/*
 * Angelic Retreat
 * Copyright (C) 2008 G. Christensen
 * 
 * Angelic Retreat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Angelic Retreat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include "boost/shared_ptr.hpp"

#include <atlwin.h>
#include <atlddx.h>

#include "resource.h"

#include "settings.h"

#include "OptionDialog.h"
#include "MachineWrapper.h"

#include "GeneralOptionsDlg.h"
#include "RestrictionOptionsDlg.h"

class CSettingsDlg: 
	public CDialogImpl<CSettingsDlg>,
	public CWinDataExchange<CSettingsDlg>
{
public:

	static const int IDD = IDD_SETTINGS;

	CSettingsDlg(setting_manager_t &settings, machine_vector_t &machines);

	bool GetLockSettings();
	bool Applied();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	void OnSettingGroupSelect(UINT uCode, int nCtrlID, HWND hwndCtrl);

	BEGIN_DDX_MAP(CSettingsDlg)
		DDX_CHECK(IDC_BLOCK_SETTINGS, m_lockSettings)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CSettingsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDOK, OnSettingsOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnSettingsCancel)
		COMMAND_ID_HANDLER_EX(IDC_APPLY, OnSettingsApply)
		COMMAND_HANDLER_EX(IDC_SETTING_LIST, LBN_SELCHANGE, OnSettingGroupSelect)
		ALT_MSG_MAP(1) // Settnig listbox
	END_MSG_MAP()

private:

	setting_manager_t &m_settings;
	machine_vector_t &m_machines;

	CButton m_wndOKBtn, m_wndCancelBtn;
	CContainedWindowT<CListBox> m_wndSettingList;

	struct OPTION_DIALOG_STRUCT
	{
		COptionDialog *dlg;
	};

	std::vector<OPTION_DIALOG_STRUCT> m_optionDialogs;

	HWND m_hTopWindow;

	bool m_lockSettings;
	bool m_applied;

	boost::shared_ptr<CGeneralOptionsDlg> m_generalOptions;
	boost::shared_ptr<COptionDialog> m_restrictionOptions;

	void PushOptionsDialog(COptionDialog *dlg);
	COptionDialog *PopOptionsDialog();

	void saveSettings(setting_manager_t &settings);
};

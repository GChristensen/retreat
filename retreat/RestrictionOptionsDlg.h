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

#include <atlddx.h>

#include "resource.h"

#include "OptionDialog.h"
#include "MachineWrapper.h"

#define ICON_BK_COLOR RGB(255, 0, 255)

class CRestrictionOptionsDlg: 
	public COptionDialog, 
	public CWinDataExchange<CRestrictionOptionsDlg>
{
public:
	CRestrictionOptionsDlg();

	void read(SettingTable *settings);
	void write(SettingTable *settings);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnRestrictProcessSelect(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnAddProcess(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnDeleteProcess(WPARAM wParam, LPARAM lParam, HWND wnd);

	BEGIN_MSG_MAP_EX(CRestrictionOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_ADD_PROCESS, OnAddProcess)
		COMMAND_ID_HANDLER_EX(IDC_DELETE_PROCESS, OnDeleteProcess)
		COMMAND_HANDLER_EX(IDC_RESTRICT_PROCESSES, LBN_SELCHANGE, \
			OnRestrictProcessSelect)
		ALT_MSG_MAP(1)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CRestrictionOptionsDlg)
		DDX_UINT(IDC_ACTIVITY_CHECK_MIN, \
			m_restrictions.CheckForUserActivityBeforeMin)
	END_DDX_MAP()

private:

	CButton m_wndAddBtn;
	CButton m_wndDeleteBtn;

	GLOBAL_RESTRICTIONS m_restrictions;

	CContainedWindowT<CListBox> m_restrictionList;
	CUpDownCtrl m_spinBtn;
};

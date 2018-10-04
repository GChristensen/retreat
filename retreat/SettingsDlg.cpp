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

#include "stdafx.h"

#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

#include <atlcrack.h>

#include "SettingsDlg.h"

CSettingsDlg::CSettingsDlg(setting_manager_t &settings, machine_vector_t &machines):
m_settings(settings),
m_machines(machines),
m_wndSettingList(this, 1),
m_lockSettings(false),
m_applied(false)
{
	m_optionDialogs.reserve(3);

	m_generalOptions = 
		boost::shared_ptr<CGeneralOptionsDlg>(new CGeneralOptionsDlg());

	m_generalOptions->SetOptionsCategoryName(
		(LPCTSTR)WTL::CString(MAKEINTRESOURCE(IDS_GENERAL_OPTIONS_GROUP_NAME)));

	m_restrictionOptions = 
		boost::shared_ptr<COptionDialog>(new CRestrictionOptionsDlg());

	m_restrictionOptions->SetOptionsCategoryName(
		(LPCTSTR)WTL::CString(MAKEINTRESOURCE(IDS_RESTRICTION_OPTIONS_GROUP_NAME)));
}

void CSettingsDlg::PushOptionsDialog(COptionDialog *dlg)
{
	OPTION_DIALOG_STRUCT dlg_struct = {dlg, };

	m_optionDialogs.push_back(dlg_struct);
}

COptionDialog *CSettingsDlg::PopOptionsDialog()
{
	COptionDialog *result = NULL;

	if (!m_optionDialogs.empty())
	{
		result = m_optionDialogs.back().dlg;
		m_optionDialogs.pop_back();
	}

	return result;
}

bool CSettingsDlg::GetLockSettings()
{
	return m_lockSettings;
}

LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	// general settings
	m_generalOptions->Create(m_hWnd);

	tstring language = 
		SettingManagerAdapter(m_settings, SM_GENERAL_SECTION)
			.getStringSetting(SM_LANGUAGE);

	m_generalOptions->SetLanguage(language);

	// global restrictions
	m_restrictionOptions->Create(m_hWnd);

	m_restrictionOptions->read(
		SettingManagerAdapter(m_settings, RM_RESTRICTIONS_SECTION));

	// main settings dialog initialization
	CenterWindow();

	m_wndOKBtn.Attach(GetDlgItem(IDOK));
	m_wndCancelBtn.Attach(GetDlgItem(IDCANCEL));
	m_wndSettingList.SubclassWindow(GetDlgItem(IDC_SETTING_LIST));

	CRect rc_origin, rc_parent;

	// get top left coordinates for child setting dialogs
	::GetWindowRect(GetDlgItem(IDC_CUSTOM_PARENT), &rc_origin);
	GetClientRect(&rc_parent);
	ClientToScreen(&rc_parent);

	int x = rc_origin.left - rc_parent.left;
	int y = rc_origin.top - rc_parent.top;
	
	// push general options
#ifndef ENSO_LIBRARY
	PushOptionsDialog(m_generalOptions.get());
#endif

	// push machines option dialogs
	for (size_t i = 0; i < m_machines.size(); ++i)
	{
		SettingsUIFactory *uiFactory = m_machines[i]->getUIFactory();
		COptionDialog *dlg = NULL;

		if (uiFactory != NULL)
		{
			dlg = uiFactory->getUI(m_hWnd);
			if (dlg != NULL)
				dlg->read(SettingManagerAdapter(m_settings, m_machines[i]->name()));
		}
		
		PushOptionsDialog(dlg);
	}

	// push restrictions
	PushOptionsDialog(m_restrictionOptions.get());

	foreach (OPTION_DIALOG_STRUCT &dlg_struct, m_optionDialogs)
	{
		m_wndSettingList.AddString(
			dlg_struct.dlg->GetOptionsCategoryName().c_str());

		dlg_struct.dlg->SetWindowPos(0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!m_optionDialogs.empty())
	{
		m_hTopWindow = m_optionDialogs[0].dlg->m_hWnd;
		::ShowWindow(m_hTopWindow, SW_SHOW);
	}

	m_wndSettingList.SetCurSel(0);

	return 0;
}

LRESULT CSettingsDlg::OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	foreach (OPTION_DIALOG_STRUCT &dlg_struct, m_optionDialogs)
	{
		dlg_struct.dlg->SetAccepted(true);
	}

	DoDataExchange(TRUE);

	saveSettings(m_settings);

	EndDialog(IDOK);
	return 0;
}

LRESULT CSettingsDlg::OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CSettingsDlg::OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_applied = true;
	saveSettings(m_settings);
	return 0;
}

LRESULT CSettingsDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	EndDialog(IDCANCEL);
	return 0;
}

void CSettingsDlg::OnSettingGroupSelect(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	int id = m_wndSettingList.GetCurSel();

	if (id >= 0 && id < (int)m_optionDialogs.size())
	{	
		COptionDialog *dlg = m_optionDialogs[id].dlg;

		if (m_hTopWindow != dlg->m_hWnd)
		{
			HWND prev = m_hTopWindow;
			m_hTopWindow = dlg->m_hWnd;

			::ShowWindow(m_hTopWindow, SW_SHOW);
			::ShowWindow(prev, SW_HIDE);
		}
	}
}

LRESULT CSettingsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	COptionDialog *dlg;

	// global restrictions
	dlg = PopOptionsDialog();
	dlg->DestroyWindow();

	// utilize machine dialogs
	for (size_t i = m_machines.size() - 1; i <= 0; --i)
	{
		dlg = PopOptionsDialog();
		
		SettingsUIFactory *uiFactory = m_machines[i]->getUIFactory();

		if (uiFactory != NULL)
		{
			uiFactory->utilizeUI(dlg);
		}
	}
	
#ifndef ENSO_LIBRARY
	// general options
	dlg = PopOptionsDialog();
	dlg->DestroyWindow();
#endif

	ATLTRACE(_T("settings cleaned up\n"));
	return 0;
}

void CSettingsDlg::saveSettings(setting_manager_t &settings)
{
	std::vector<OPTION_DIALOG_STRUCT>::iterator dlg_it;
	
	dlg_it = m_optionDialogs.begin();

#ifndef ENSO_LIBRARY
	// general options
	(dlg_it++)->dlg->write(SettingManagerAdapter(settings, SM_GENERAL_SECTION));
#endif

	for (size_t i = 0; i < m_machines.size(); ++i, ++dlg_it)
	{
		COptionDialog *dlg = dlg_it->dlg;

		if (dlg != NULL)
		{
			dlg->write(SettingManagerAdapter(settings, m_machines[i]->name()));
		}
	}

	// global restrictions
	dlg_it->dlg->write(SettingManagerAdapter(settings, RM_RESTRICTIONS_SECTION));
}

bool CSettingsDlg::Applied()
{
	return m_applied;
}
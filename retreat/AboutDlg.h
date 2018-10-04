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

#include <atlctrls.h>
#include <atlctrlx.h>

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum {IDD = IDD_ABOUT};

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(0);

		m_wndLimeLink.SubclassWindow(GetDlgItem(IDC_LIME_LINK));
		m_wndLimeLink.ModifyStyle(WS_TABSTOP, 0);
		m_wndLimeLink.SetHyperLink(_T("http://gchristensen.github.io/retreat"));
		m_wndLimeLink.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER);


		m_wndMindfulLink.SubclassWindow(GetDlgItem(IDC_MINDFUL_ORG));
		m_wndMindfulLink.ModifyStyle(WS_TABSTOP, 0);
		m_wndMindfulLink.SetHyperLink(_T("https://www.mindful.org"));
		m_wndMindfulLink.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER);

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

private:

	CHyperLink m_wndLimeLink;
	CHyperLink m_wndMindfulLink;

};

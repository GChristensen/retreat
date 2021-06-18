module;

#include "stdafx.h"
#include "resource.h"

export module AboutDlg;

export class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		//COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:

	CHyperLink linkGitHub;
	CHyperLink linkPatreon;
};

module :private;

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(0);

	linkGitHub.SubclassWindow(GetDlgItem(IDC_GITHUB_LINK));
	linkGitHub.ModifyStyle(WS_TABSTOP, 0);
	linkGitHub.SetHyperLink(_T("https://gchristensen.github.io/retreat"));
	linkGitHub.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER);

	linkPatreon.SubclassWindow(GetDlgItem(IDC_DONATE));
	linkPatreon.ModifyStyle(WS_TABSTOP, 0);
	linkPatreon.SetHyperLink(_T("https://patreon.com/gchristnsn"));
	linkPatreon.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER);

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}



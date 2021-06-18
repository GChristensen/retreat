module;

#include "stdafx.h"

#include "resource.h"

export module MainOptionsDlg;

import <memory>;
import <vector>;

import system;
import Settings;
import SectionDlg;
import GeneralOptionsSection;
import ScheduleOptionsSection;
import RestrictionOptionsSection;

export class CMainOptionsDlg :
	public CDialogImpl<CMainOptionsDlg>,
	public CWinDataExchange<CMainOptionsDlg>
{
public:

	static const int IDD = IDD_MAIN_OPTIONS;

	CMainOptionsDlg(Settings &settings);

	bool IsApplied();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	void OnSettingGroupSelect(UINT uCode, int nCtrlID, HWND hwndCtrl);

	BEGIN_DDX_MAP(CMainOptionsDlg)
		//DDX_CHECK(IDC_BLOCK_SETTINGS, m_lockSettings)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CMainOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDOK, OnSettingsOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnSettingsCancel)
		COMMAND_ID_HANDLER_EX(IDC_APPLY, OnSettingsApply)
		COMMAND_HANDLER_EX(IDC_SETTING_GROUPS, LBN_SELCHANGE, OnSettingGroupSelect)
		ALT_MSG_MAP(1) // settnig groups listbox
	END_MSG_MAP()

private:

	Settings &m_settings;

	CButton m_wndOKBtn, m_wndCancelBtn;
	CContainedWindowT<CListBox> m_wndSectionList;

	std::vector<SectionPtr> m_optionSections;

	HWND m_hTopWindow;

	bool m_applied;

	CHyperLink m_helpLink;

	void saveSettings();
};

module :private;

CMainOptionsDlg::CMainOptionsDlg(Settings &settings): 
	m_wndSectionList(this, 1),
	m_settings(settings),
	m_applied(false) {

}

LRESULT CMainOptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	CenterWindow();

#ifdef PYTHON_MODULE
	// bring window to front; useful if options were opened from an enso command
	HWND hCurWnd = ::GetForegroundWindow();
	DWORD dwMyID = ::GetCurrentThreadId();
	DWORD dwCurID = ::GetWindowThreadProcessId(hCurWnd, NULL);
	::AttachThreadInput(dwCurID, dwMyID, TRUE);
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	::SetForegroundWindow(m_hWnd);
	::SetFocus(m_hWnd);
	::SetActiveWindow(m_hWnd);
	::AttachThreadInput(dwCurID, dwMyID, FALSE);
#endif

	m_wndOKBtn.Attach(GetDlgItem(IDOK));
	m_wndCancelBtn.Attach(GetDlgItem(IDCANCEL));
	m_wndSectionList.SubclassWindow(GetDlgItem(IDC_SETTING_GROUPS));

	m_helpLink.SubclassWindow(GetDlgItem(IDC_HELP_LINK));
	m_helpLink.ModifyStyle(WS_TABSTOP, 0);
	m_helpLink.SetHyperLink((_T("file:///") + getBundledFilePath(_T("retreat.html"))).c_str());
	m_helpLink.SetHyperLinkExtendedStyle(HLINK_UNDERLINEHOVER);

	CRect rc_origin, rc_parent;

	// get top left coordinates of section placeholder
	::GetWindowRect(GetDlgItem(IDC_CUSTOM_PARENT), &rc_origin);
	GetClientRect(&rc_parent);
	ClientToScreen(&rc_parent);

	int x = rc_origin.left - rc_parent.left;
	int y = rc_origin.top - rc_parent.top;


	m_optionSections.push_back(std::make_shared<CGeneralOptionsSection>());
	m_optionSections.push_back(std::make_shared<CScheduleOptionsSection>());
	m_optionSections.push_back(std::make_shared<CRestrictionOptionsSection>());


	for (auto &section : m_optionSections) {
		section->Create(m_hWnd);
		section->read(m_settings);
		m_wndSectionList.AddString(section->GetOptionsCategoryName());
		section->SetWindowPos(0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!m_optionSections.empty()) {
		m_hTopWindow = m_optionSections[0]->m_hWnd;
		::ShowWindow(m_hTopWindow, SW_SHOW);
	}

	m_wndSectionList.SetCurSel(0);

	return 0;
}

LRESULT CMainOptionsDlg::OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	for (auto &section : m_optionSections)
		section->SetAccepted(true);

	DoDataExchange(TRUE);

	saveSettings();

	EndDialog(IDOK);
	return 0;
}

LRESULT CMainOptionsDlg::OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainOptionsDlg::OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_applied = true;
	saveSettings();
	return 0;
}

LRESULT CMainOptionsDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	EndDialog(IDCANCEL);
	return 0;
}

void CMainOptionsDlg::OnSettingGroupSelect(UINT uCode, int nCtrlID, HWND hwndCtrl)
{
	int id = m_wndSectionList.GetCurSel();

	if (id >= 0 && id < (int)m_optionSections.size())
	{
		SectionPtr &dlg = m_optionSections[id];

		if (m_hTopWindow != dlg->m_hWnd)
		{
			HWND prev = m_hTopWindow;
			m_hTopWindow = dlg->m_hWnd;

			::ShowWindow(m_hTopWindow, SW_SHOW);
			::ShowWindow(prev, SW_HIDE);
		}
	}
}

LRESULT CMainOptionsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	for (auto &section : m_optionSections)
		section->DestroyWindow();

	return 0;
}

void CMainOptionsDlg::saveSettings()
{
	for (auto &section : m_optionSections)
		section->write(m_settings);

	m_settings.save();
}

bool CMainOptionsDlg::IsApplied()
{
	return m_applied;
}
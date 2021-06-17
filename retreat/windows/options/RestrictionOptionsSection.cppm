module;

#include "stdafx.h"
#include "resource.h"

export module RestrictionOptionsSection;

import <format>;

import Settings;
import SectionDlg;

export class CRestrictionOptionsSection :
	public CSectionDlg,
	public CWinDataExchange<CRestrictionOptionsSection>
{
public:
	CRestrictionOptionsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnProcessSelect(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnAddProcess(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnDeleteProcess(WPARAM wParam, LPARAM lParam, HWND wnd);

	BEGIN_MSG_MAP_EX(CRestrictionOptionsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_ADD_PROCESS, OnAddProcess)
		COMMAND_ID_HANDLER_EX(IDC_DELETE_PROCESS, OnDeleteProcess)
		ALT_MSG_MAP(1)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CRestrictionOptionsSection)
		DDX_UINT(IDC_ACTIVITY_CHECK_MIN, m_checkForUserActivityBeforeMin)
		DDX_CHECK(IDC_USE_RESTRICTIONS, m_useRestrictionList)
	END_DDX_MAP()

private:

	CButton m_wndAddBtn;
	CButton m_wndDeleteBtn;

	int m_checkForUserActivityBeforeMin;
	bool m_useRestrictionList;

	CContainedWindowT<CListBox> m_restrictionList;
	CUpDownCtrl m_spinBtn;
};

module :private;


CRestrictionOptionsSection::CRestrictionOptionsSection() :
	CSectionDlg(IDD_RESTRICTIONS),
	m_restrictionList(this, 1)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_RESTRICTION_OPTIONS_SECTION_NAME)));
}

void CRestrictionOptionsSection::read(Settings &settings)
{
	auto processes = settings.getSectionValues(Settings::PROCESSES);

	for (auto &process : processes)
		m_restrictionList.AddString(process.c_str());
		
	m_checkForUserActivityBeforeMin = settings.getInt(Settings::MONITORING_INACTIVITY, Settings::DEFAULT_MONITORING_INACTIVITY);
	m_useRestrictionList = settings.getBoolean(Settings::MONITORING_INPUT, Settings::DEFAULT_MONITORING_INPUT)
		|| settings.getBoolean(Settings::MONITORING_PROCESSES, Settings::DEFAULT_MONITORING_PROCESSES);

	m_spinBtn.SetRange(0, 30);
	m_spinBtn.SetPos(m_checkForUserActivityBeforeMin);

	DoDataExchange(FALSE);
}

void CRestrictionOptionsSection::write(Settings &settings)
{
	DoDataExchange(TRUE);

	settings.setInt(Settings::MONITORING_INACTIVITY, m_checkForUserActivityBeforeMin);
	settings.setBoolean(Settings::MONITORING_INPUT, m_useRestrictionList);
	settings.setBoolean(Settings::MONITORING_PROCESSES, m_useRestrictionList);

	settings.clearSection(Settings::PROCESSES);

	int count = m_restrictionList.GetCount();

	for (int i = 0; i < count; ++i) {
		CString text;
		m_restrictionList.GetText(i, text);
		settings.setString(Settings::PROCESSES + std::format(_T("{:02}"), i), (LPCTSTR)text);
	}
}

LRESULT CRestrictionOptionsSection::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	m_wndAddBtn.Attach(GetDlgItem(IDC_ADD_PROCESS));
	m_wndDeleteBtn.Attach(GetDlgItem(IDC_DELETE_PROCESS));

	m_restrictionList.SubclassWindow(GetDlgItem(IDC_RESTRICT_PROCESSES));

	m_spinBtn.Attach(GetDlgItem(IDC_ACTIVITY_CHECK_SPIN));

	return 0;
}

LRESULT CRestrictionOptionsSection::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CRestrictionOptionsSection::OnAddProcess(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog fileDlg(TRUE);

	CString filter(MAKEINTRESOURCE(IDS_RESTRICTION_LIST_FILTER));

	int length = filter.GetLength();
	TCHAR* szBuffer = filter.GetBuffer(length);

	for (int i = 0; i < length; ++i) {
		if (szBuffer[i] == _T('|'))
			szBuffer[i] = _T('\0');
	}

	fileDlg.m_ofn.hwndOwner = GetParent();
	fileDlg.m_ofn.lpstrFilter = szBuffer;
	fileDlg.m_ofn.nFilterIndex = 1;

	if (fileDlg.DoModal() == IDOK) {
		m_restrictionList.AddString(fileDlg.m_szFileName);
		m_restrictionList.SetCurSel(m_restrictionList.GetCount() - 1);
	}

	filter.ReleaseBuffer();

	return 0;
}

LRESULT CRestrictionOptionsSection::OnDeleteProcess(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	int sel = m_restrictionList.GetCurSel();

	if (sel >= 0)
	{
		CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
		CString caption(MAKEINTRESOURCE(IDS_APP_NAME));

		if (::MessageBox(GetParent(), message, caption, MB_ICONQUESTION | MB_YESNO) == IDYES)
			m_restrictionList.DeleteString(sel);
	}

	return 0;
}
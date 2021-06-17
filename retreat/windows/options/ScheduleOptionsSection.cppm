

module;

#include "stdafx.h"
#include "resource.h"

export module ScheduleOptionsSection;

import <string>;
import <format>;

import Settings;
import SectionDlg;

#include "tstring.h"

//// CCronInputDlg ///////////////////////////////////////////////////////////

class CCronInputDlg :
	public CDialogImpl<CCronInputDlg>,
	public CWinDataExchange<CCronInputDlg>
{
public:

	static const int IDD = IDD_CRON_INPUT;

	CCronInputDlg();

	void SetCrontab(CString crontab);
	CString GetCrontab();

	void SetDuration(CString duration);
	CString GetDuration();

	void SetMessage(CString message);
	CString GetMessage();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnClear(WPARAM wParam, LPARAM lParam, HWND wnd);
	LRESULT OnOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCancel(WPARAM wParam, LPARAM lParam, HWND wnd);

	BEGIN_DDX_MAP(CCronInputDlg)
		DDX_TEXT(IDC_CRON0, m_crontab[0])
		DDX_TEXT(IDC_CRON1, m_crontab[1])
		DDX_TEXT(IDC_CRON2, m_crontab[2])
		DDX_TEXT(IDC_CRON3, m_crontab[3])
		DDX_TEXT(IDC_CRON4, m_crontab[4])
		DDX_TEXT(IDC_SCHEDULED_BREAK_DURATION, m_duration)
		DDX_TEXT(IDC_SCHEDULED_BREAK_MESSAGE, m_message)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CCronInputDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:

	CString m_crontab[5];
	CString m_duration;
	CString m_message;

};


CCronInputDlg::CCronInputDlg()
{

}

void CCronInputDlg::SetCrontab(CString crontab)
{
	int nTokenPos = 0, n = 0;
	CString strToken = crontab.Tokenize(_T(" "), nTokenPos);
	int size = sizeof(m_crontab) / sizeof(m_crontab[0]);

	while (!strToken.IsEmpty()) {
		if (n < size)
			m_crontab[n++] = strToken;
		strToken = crontab.Tokenize(_T(" "), nTokenPos);
	}
}

CString CCronInputDlg::GetCrontab()
{
	CString result;

	int size = sizeof(m_crontab) / sizeof(m_crontab[0]);

	for (int i = 0; i < size; ++i) {
		result += m_crontab[i];

		if (i < size - 1)
			result += _T(" ");
	}

	return result;
}

void CCronInputDlg::SetDuration(CString duration)
{
	m_duration = duration;
}

CString CCronInputDlg::GetDuration()
{
	return m_duration;
}

void CCronInputDlg::SetMessage(CString message)
{
	m_message = message;
}

CString CCronInputDlg::GetMessage()
{
	m_message.TrimRight();
	m_message.TrimLeft();
	return m_message;
}

LRESULT CCronInputDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	DoDataExchange(FALSE);
	CenterWindow();
	return 0;
}

LRESULT CCronInputDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CCronInputDlg::OnClear(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	m_message = _T("");
	DoDataExchange(FALSE);
	return 0;
}

LRESULT CCronInputDlg::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(TRUE);
	EndDialog(IDOK);
	return 0;
}

LRESULT CCronInputDlg::OnCancel(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	SendMessage(WM_CLOSE);
	return 0;
}

//// CScheduleOptionsSection //////////////////////////////////////////////////

export class CScheduleOptionsSection :
	public CSectionDlg,
	public CWinDataExchange<CScheduleOptionsSection>
{
public:
	CScheduleOptionsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);

	LRESULT OnAddScheduledBreak(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnDeleteScheduledBreak(WPARAM wParam, LPARAM lParam, HWND wnd);
	LRESULT OnLvDblClick(LPNMHDR hdr);

	BEGIN_MSG_MAP_EX(CAngelicBehaviourDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_ADD_CRON, OnAddScheduledBreak)
		COMMAND_ID_HANDLER_EX(IDC_DELETE_CRON, OnDeleteScheduledBreak)
		NOTIFY_HANDLER_EX(IDC_CRON_LIST, NM_DBLCLK, OnLvDblClick)
	END_MSG_MAP()

private:
	CButton m_wndAddBtn;
	CButton m_wndDeleteBtn;

	CListViewCtrl m_scheduledBreaksList;

	void insertScheduledBreak(int n, const tstring &crontab, const tstring &duration, const tstring &message);
};

module :private;


CScheduleOptionsSection::CScheduleOptionsSection() :
	CSectionDlg(IDD_SCHEDULE)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_SCHEDULE_OPTIONS_SECTION_NAME)));
}


void CScheduleOptionsSection::read(Settings &settings)
{
	auto schedules = settings.getSectionValues(Settings::CRON);

	int n = 0;
	for (auto &schedule : schedules) {
		auto parts = settings.split(schedule);

		insertScheduledBreak(
			n++,
			parts[0].c_str(), // crontab
			parts[1].c_str(), // duration
			parts[2].c_str()  // message
		);
	}
}

void CScheduleOptionsSection::write(Settings &settings)
{
	settings.clearSection(Settings::CRON);

	int count = m_scheduledBreaksList.GetItemCount();

	for (int i = 0; i < count; ++i) {
		CString crontab, duration, message;

		m_scheduledBreaksList.GetItemText(i, 1, crontab);
		m_scheduledBreaksList.GetItemText(i, 2, duration);
		m_scheduledBreaksList.GetItemText(i, 3, message);

		tstring value = settings.join({ tstring((LPCTSTR)crontab), tstring((LPCTSTR)duration), tstring((LPCTSTR)message) });
		settings.setString(Settings::CRON + std::format(_T("{:02}"), i), value);
	}
}

LRESULT CScheduleOptionsSection::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	m_wndAddBtn.Attach(GetDlgItem(IDC_ADD_CRON));
	m_wndDeleteBtn.Attach(GetDlgItem(IDC_DELETE_CRON));

	m_scheduledBreaksList.Attach(GetDlgItem(IDC_CRON_LIST));

	m_scheduledBreaksList.SetView(LV_VIEW_DETAILS);
	m_scheduledBreaksList.SetExtendedListViewStyle(
		m_scheduledBreaksList.GetExtendedListViewStyle()
		| LVS_EX_FULLROWSELECT
		| LVS_EX_GRIDLINES
	);

	// workaround for the first column text alignment
	// see MSDN on LVCOLUMN
	m_scheduledBreaksList.AddColumn(_T("Dummy"), 0);

	CString column(MAKEINTRESOURCE(IDS_SCHEDULED_BREAK_NUMBER_COLUMN));
	m_scheduledBreaksList.AddColumn(
		column, 1, -1,
		LVCF_FMT | LVCF_WIDTH | LVCF_TEXT,
		LVCFMT_RIGHT
	);

	column.LoadString(IDS_SCHEDULED_BREAK_CRONTAB_COLUMN);
	m_scheduledBreaksList.AddColumn(column, 2);

	column.LoadString(IDS_SCHEDULED_BREAK_DURATION_COLUMN);
	m_scheduledBreaksList.AddColumn(
		column, 3, -1,
		LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		LVCFMT_RIGHT
	);

	column.LoadString(IDS_SCHEDULED_BREAK_MESSAGE_COLUMN);
	m_scheduledBreaksList.AddColumn(column, 4);

	m_scheduledBreaksList.DeleteColumn(0);

	m_scheduledBreaksList.SetColumnWidth(1, 120);
	m_scheduledBreaksList.SetColumnWidth(3, 100);

	return 0;
}

LRESULT CScheduleOptionsSection::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CScheduleOptionsSection::OnAddScheduledBreak(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CCronInputDlg dlg;

	dlg.SetCrontab(_T("* * * * *"));
	dlg.SetDuration(to_tstring(Settings::DEFAULT_BREAK_DURATION).c_str());

	if (dlg.DoModal(m_hWnd) == IDOK) {
		insertScheduledBreak(
			m_scheduledBreaksList.GetItemCount(),
			(LPCTSTR)dlg.GetCrontab(),
			(LPCTSTR)dlg.GetDuration(),
			(LPCTSTR)dlg.GetMessage()
		);

		m_scheduledBreaksList.SelectItem(m_scheduledBreaksList.GetItemCount() - 1);
	}

	return 0;
}

LRESULT CScheduleOptionsSection::OnDeleteScheduledBreak(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	int sel = m_scheduledBreaksList.GetSelectedIndex();

	if (sel >= 0) {
		CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
		CString caption(MAKEINTRESOURCE(IDS_APP_NAME));

		if (::MessageBox(m_hWnd, message, caption, MB_ICONQUESTION | MB_YESNO) == IDYES) {
			m_scheduledBreaksList.DeleteItem(sel);

			int n = m_scheduledBreaksList.GetItemCount();

			for (int i = 0; i < n; ++i) {
				m_scheduledBreaksList.SetItemText(i, 0, to_tstring(i + 1).c_str());
			}
		}
	}

	return 0;
}

LRESULT CScheduleOptionsSection::OnLvDblClick(LPNMHDR hdr)
{
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)hdr;

	if (lpnmitem->iItem >= 0)
	{
		CCronInputDlg dlg;
		CString text;

		m_scheduledBreaksList.GetItemText(lpnmitem->iItem, 1, text);
		dlg.SetCrontab(text);

		m_scheduledBreaksList.GetItemText(lpnmitem->iItem, 2, text);
		dlg.SetDuration(text);

		m_scheduledBreaksList.GetItemText(lpnmitem->iItem, 3, text);
		dlg.SetMessage(text);

		if (dlg.DoModal(m_hWnd) == IDOK) {
			m_scheduledBreaksList.SetItemText(lpnmitem->iItem, 1, dlg.GetCrontab());

			m_scheduledBreaksList.SetItemText(lpnmitem->iItem, 2, dlg.GetDuration());

			m_scheduledBreaksList.SetItemText(lpnmitem->iItem, 3, dlg.GetMessage());
		}
	}

	return 0;
}

void CScheduleOptionsSection::insertScheduledBreak(int n, const tstring &crontab, const tstring &duration, const tstring &message)
{
	m_scheduledBreaksList.InsertItem(n, to_tstring(n + 1).c_str());

	m_scheduledBreaksList.SetItemText(n, 1, crontab.c_str());

	m_scheduledBreaksList.SetItemText(n, 2, duration.c_str());

	m_scheduledBreaksList.SetItemText(n, 3, message.c_str());
}

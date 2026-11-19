module;

#include "wtl.h"
#include "resource.h"

#include <shellapi.h>

#include <algorithm>
#include <format>
#include <iterator>
#include <set>
#include <string>
#include <vector>
#include "tstring.h"

export module ExceptionsSettingsSection;

import Settings;
import SectionDlg;
import SettingsControls;
import system;

static const struct { int id; int value; } IDLE_PRESETS[] = {
	{ IDC_IDLE_P5, 5 }, { IDC_IDLE_P10, 10 }, { IDC_IDLE_P15, 15 } };

static tstring fileNameOf(const tstring& path)
{
	size_t slash = path.find_last_of(_T("\\/"));
	return slash == tstring::npos ? path : path.substr(slash + 1);
}

static bool containsNoCase(const std::vector<tstring>& list, const tstring& value)
{
	return std::any_of(list.begin(), list.end(),
		[&](const tstring& item) { return !_tcsicmp(item.c_str(), value.c_str()); });
}

// Small-icon image list shared by the program lists.
static void setupProgramList(CListViewCtrl& list, bool withStatus)
{
	list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	SHFILEINFO info{};
	HIMAGELIST systemImages = (HIMAGELIST)::SHGetFileInfo(_T("C:\\"), 0, &info, sizeof(info),
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	list.SetImageList(systemImages, LVSIL_SMALL);

	CRect rc;
	list.GetClientRect(&rc);
	int width = rc.Width() - ::GetSystemMetrics(SM_CXVSCROLL);
	int statusWidth = withStatus ? width / 5 : 0;
	int nameWidth = width * 3 / 10;

	list.InsertColumn(0, CString(MAKEINTRESOURCE(IDS_PROCESS_NAME_COLUMN)), LVCFMT_LEFT, nameWidth);
	list.InsertColumn(1, CString(MAKEINTRESOURCE(IDS_PROCESS_PATH_COLUMN)), LVCFMT_LEFT, width - nameWidth - statusWidth);
	if (withStatus)
		list.InsertColumn(2, CString(MAKEINTRESOURCE(IDS_PROCESS_STATUS_COLUMN)), LVCFMT_LEFT, statusWidth);
}

static int iconIndexOf(const tstring& path)
{
	SHFILEINFO info{};
	if (::SHGetFileInfo(path.c_str(), 0, &info, sizeof(info), SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
		return info.iIcon;
	return 0;
}

//// CPickProcessDlg /////////////////////////////////////////////////////////

class CPickProcessDlg : public CDialogImpl<CPickProcessDlg>
{
public:
	static const int IDD = IDD_PICK_PROCESS;

	CPickProcessDlg(const std::vector<tstring>& exclude) : m_exclude(exclude) {}

	tstring GetSelected() const { return m_selected; }

	BEGIN_MSG_MAP(CPickProcessDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		NOTIFY_HANDLER(IDC_PROCESS_LIST, NM_DBLCLK, OnDoubleClick)
		NOTIFY_HANDLER(IDC_PROCESS_LIST, LVN_ITEMCHANGED, OnItemChanged)
	END_MSG_MAP()

private:
	std::vector<tstring> m_exclude;
	std::vector<tstring> m_paths;
	tstring m_selected;
	CListViewCtrl m_list;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());

		m_list.Attach(GetDlgItem(IDC_PROCESS_LIST));
		setupProgramList(m_list, false);

		TCHAR windowsDir[MAX_PATH] = { 0 };
		::GetWindowsDirectory(windowsDir, MAX_PATH);
		size_t windowsDirLen = _tcslen(windowsDir);

		for (const tstring& path : getRunningProcessPaths()) {
			// skip system processes and the programs that are already listed
			if (windowsDirLen && !_tcsnicmp(path.c_str(), windowsDir, windowsDirLen))
				continue;
			if (containsNoCase(m_exclude, path))
				continue;

			m_paths.push_back(path);
			int n = m_list.GetItemCount();
			int item = m_list.InsertItem(LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, n, fileNameOf(path).c_str(),
				0, 0, iconIndexOf(path), (LPARAM)(m_paths.size() - 1));
			m_list.SetItemText(item, 1, path.c_str());
		}

		GetDlgItem(IDOK).EnableWindow(FALSE);
		m_list.SetFocus();
		return FALSE;
	}

	LRESULT OnItemChanged(int, LPNMHDR, BOOL&)
	{
		GetDlgItem(IDOK).EnableWindow(m_list.GetSelectedIndex() >= 0);
		return 0;
	}

	LRESULT OnDoubleClick(int, LPNMHDR, BOOL&)
	{
		if (m_list.GetSelectedIndex() >= 0)
			PostMessage(WM_COMMAND, IDOK);
		return 0;
	}

	LRESULT OnOK(WORD, WORD, HWND, BOOL&)
	{
		int sel = m_list.GetSelectedIndex();
		if (sel < 0)
			return 0;
		m_selected = m_paths[m_list.GetItemData(sel)];
		EndDialog(IDOK);
		return 0;
	}

	LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
	{
		EndDialog(IDCANCEL);
		return 0;
	}
};

//// CExceptionsSettingsSection //////////////////////////////////////////////

export class CExceptionsSettingsSection : public CSectionDlg
{
public:
	CExceptionsSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	BEGIN_MSG_MAP(CExceptionsSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		COMMAND_ID_HANDLER(IDC_IDLE_ENABLE, OnToggle)
		COMMAND_ID_HANDLER(IDC_APPS_ENABLE, OnToggle)
		COMMAND_RANGE_HANDLER(IDC_IDLE_P5, IDC_IDLE_P15, OnIdlePreset)
		COMMAND_ID_HANDLER(IDC_ADD_PROCESS, OnAddProcess)
		COMMAND_ID_HANDLER(IDC_PICK_PROCESS, OnPickProcess)
		COMMAND_ID_HANDLER(IDC_DELETE_PROCESS, OnDeleteProcess)
		NOTIFY_HANDLER(IDC_RESTRICT_PROCESSES, LVN_KEYDOWN, OnListKeyDown)
		NOTIFY_HANDLER(IDC_RESTRICT_PROCESSES, LVN_ITEMCHANGED, OnListItemChanged)
		CHAIN_MSG_MAP(CSectionDlg)
	END_MSG_MAP()

private:
	CIdleStrip m_idleStrip;
	CTrackBarCtrl m_idleSlider;
	CListViewCtrl m_programList;
	std::vector<tstring> m_programs;

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnHScroll(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnShowWindow(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnToggle(WORD, WORD, HWND, BOOL&);
	LRESULT OnIdlePreset(WORD, WORD, HWND, BOOL&);
	LRESULT OnAddProcess(WORD, WORD, HWND, BOOL&);
	LRESULT OnPickProcess(WORD, WORD, HWND, BOOL&);
	LRESULT OnDeleteProcess(WORD, WORD, HWND, BOOL&);
	LRESULT OnListKeyDown(int, LPNMHDR, BOOL&);
	LRESULT OnListItemChanged(int, LPNMHDR, BOOL&);

	void updateIdle();
	void updateApps();
	void fillProgramList(int select = -1);
	void addProgram(const tstring& path);
};

module :private;

CExceptionsSettingsSection::CExceptionsSettingsSection() :
	CSectionDlg(IDD_EXCEPTIONS)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_EXCEPTIONS_SECTION_NAME)));
}

LRESULT CExceptionsSettingsSection::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	InitSectionHeader();
	MarkMuted(IDC_IDLE_SUMMARY);

	m_idleStrip.SubclassWindow(GetDlgItem(IDC_IDLE_STRIP));
	m_idleSlider.Attach(GetDlgItem(IDC_IDLE_SLIDER));
	m_idleSlider.SetRange(1, 30);
	m_idleSlider.SetPageSize(5);

	m_programList.Attach(GetDlgItem(IDC_RESTRICT_PROCESSES));
	setupProgramList(m_programList, true);

	return TRUE;
}

void CExceptionsSettingsSection::read(Settings &settings)
{
	m_loading = true;

	SetChecked(IDC_IDLE_ENABLE, settings.getBoolean(Settings::MONITORING_INPUT, Settings::DEFAULT_MONITORING_INPUT));
	SetChecked(IDC_APPS_ENABLE, settings.getBoolean(Settings::MONITORING_PROCESSES, Settings::DEFAULT_MONITORING_PROCESSES));

	int idle = std::clamp(settings.getInt(Settings::MONITORING_INACTIVITY, Settings::DEFAULT_MONITORING_INACTIVITY), 1, 30);
	m_idleSlider.SetPos(idle);

	m_programs.clear();
	for (auto &process : settings.getSectionValues(Settings::PROCESSES))
		m_programs.push_back(process);
	fillProgramList();

	m_loading = false;
	updateIdle();
	updateApps();
}

void CExceptionsSettingsSection::write(Settings &settings)
{
	settings.setBoolean(Settings::MONITORING_INPUT, IsChecked(IDC_IDLE_ENABLE));
	settings.setBoolean(Settings::MONITORING_PROCESSES, IsChecked(IDC_APPS_ENABLE));
	settings.setInt(Settings::MONITORING_INACTIVITY, m_idleSlider.GetPos());

	settings.clearSection(Settings::PROCESSES);
	for (size_t i = 0; i < m_programs.size(); ++i)
		settings.setString(Settings::PROCESSES + std::format(_T("{:02}"), i), m_programs[i]);
}

void CExceptionsSettingsSection::updateIdle()
{
	bool enabled = IsChecked(IDC_IDLE_ENABLE);
	int idle = m_idleSlider.GetPos();

	m_idleStrip.SetModel(idle, enabled);
	SetDlgItemText(IDC_IDLE_VALUE, std::format(_T("{} min"), idle).c_str());

	for (const auto& preset : IDLE_PRESETS) {
		SetChecked(preset.id, preset.value == idle);
		Enable(preset.id, enabled);
	}

	Enable(IDC_IDLE_SLIDER, enabled);
	Enable(IDC_IDLE_VALUE, enabled);
	Enable(IDC_IDLE_STRIP, enabled);

	tstring summary = enabled
		? std::format(_T("If there has been no keyboard or mouse input for {} min when a break is about to be announced, "
		                 "Retreat skips it: you have already stepped away."), idle)
		: tstring(_T("Retreat announces and starts every break, even if you were away."));
	SetDlgItemText(IDC_IDLE_SUMMARY, summary.c_str());
}

void CExceptionsSettingsSection::updateApps()
{
	bool enabled = IsChecked(IDC_APPS_ENABLE);
	m_programList.EnableWindow(enabled);
	Enable(IDC_PICK_PROCESS, enabled);
	Enable(IDC_ADD_PROCESS, enabled);
	Enable(IDC_DELETE_PROCESS, enabled && m_programList.GetSelectedIndex() >= 0);
}

void CExceptionsSettingsSection::fillProgramList(int select)
{
	std::vector<tstring> running = getRunningProcessPaths();

	m_programList.SetRedraw(FALSE);
	m_programList.DeleteAllItems();

	CString runningText(MAKEINTRESOURCE(IDS_RUNNING_NOW));
	CString notRunningText(MAKEINTRESOURCE(IDS_NOT_RUNNING));

	for (size_t i = 0; i < m_programs.size(); ++i) {
		const tstring& path = m_programs[i];
		int item = m_programList.InsertItem(LVIF_TEXT | LVIF_IMAGE, (int)i, fileNameOf(path).c_str(),
			0, 0, iconIndexOf(path), 0);
		m_programList.SetItemText(item, 1, path.c_str());
		m_programList.SetItemText(item, 2, containsNoCase(running, path) ? runningText : notRunningText);
	}

	if (select >= 0 && select < (int)m_programs.size()) {
		m_programList.SelectItem(select);
		m_programList.EnsureVisible(select, FALSE);
	}

	m_programList.SetRedraw(TRUE);
	m_programList.Invalidate();
}

void CExceptionsSettingsSection::addProgram(const tstring& path)
{
	if (path.empty())
		return;

	if (containsNoCase(m_programs, path)) {
		auto it = std::find_if(m_programs.begin(), m_programs.end(),
			[&](const tstring& p) { return !_tcsicmp(p.c_str(), path.c_str()); });
		fillProgramList((int)(it - m_programs.begin()));
		return;
	}

	m_programs.push_back(path);
	fillProgramList((int)m_programs.size() - 1);
	NotifyChanged();
}

LRESULT CExceptionsSettingsSection::OnHScroll(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
	if ((HWND)lParam != m_idleSlider.m_hWnd) {
		bHandled = FALSE;
		return 0;
	}
	updateIdle();
	NotifyChanged();
	return 0;
}

LRESULT CExceptionsSettingsSection::OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
	bHandled = FALSE;
	if (wParam && !m_loading)
		fillProgramList(m_programList.GetSelectedIndex());  // refresh "Running now"
	return 0;
}

LRESULT CExceptionsSettingsSection::OnToggle(WORD, WORD, HWND, BOOL&)
{
	updateIdle();
	updateApps();
	NotifyChanged();
	return 0;
}

LRESULT CExceptionsSettingsSection::OnIdlePreset(WORD, WORD id, HWND, BOOL&)
{
	for (const auto& preset : IDLE_PRESETS)
		if (preset.id == id)
			m_idleSlider.SetPos(preset.value);
	updateIdle();
	NotifyChanged();
	return 0;
}

LRESULT CExceptionsSettingsSection::OnAddProcess(WORD, WORD, HWND, BOOL&)
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

	if (fileDlg.DoModal(GetParent()) == IDOK)
		addProgram(fileDlg.m_szFileName);

	filter.ReleaseBuffer();

	return 0;
}

LRESULT CExceptionsSettingsSection::OnPickProcess(WORD, WORD, HWND, BOOL&)
{
	CPickProcessDlg dlg(m_programs);
	if (dlg.DoModal(GetParent()) == IDOK)
		addProgram(dlg.GetSelected());
	return 0;
}

LRESULT CExceptionsSettingsSection::OnDeleteProcess(WORD, WORD, HWND, BOOL&)
{
	int sel = m_programList.GetSelectedIndex();

	if (sel >= 0 && sel < (int)m_programs.size()) {
		CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
		CString caption(MAKEINTRESOURCE(IDS_APP_NAME));
		if (::MessageBox(GetParent(), message, caption, MB_ICONQUESTION | MB_YESNO) != IDYES)
			return 0;

		m_programs.erase(m_programs.begin() + sel);
		fillProgramList((std::min)(sel, (int)m_programs.size() - 1));
		updateApps();
		NotifyChanged();
	}

	return 0;
}

LRESULT CExceptionsSettingsSection::OnListKeyDown(int, LPNMHDR hdr, BOOL&)
{
	if (((LPNMLVKEYDOWN)hdr)->wVKey == VK_DELETE)
		PostMessage(WM_COMMAND, IDC_DELETE_PROCESS);
	return 0;
}

LRESULT CExceptionsSettingsSection::OnListItemChanged(int, LPNMHDR, BOOL&)
{
	updateApps();
	return 0;
}

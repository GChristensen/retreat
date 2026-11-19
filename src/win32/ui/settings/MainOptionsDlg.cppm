module;

#include "wtl.h"

#include <objidl.h>
#include <gdiplus.h>

#include "resource.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

export module MainOptionsDlg;


import system;
import Settings;
import SectionDlg;
import BreaksSettingsSection;
import BreakScreenSettingsSection;
import ScheduleSettingsSection;
import ExceptionsSettingsSection;
import BreakWindow;
import TimerWindow;

using namespace Gdiplus;

enum class SectionIcon { Breaks, Screen, Schedule, Exceptions };

export class CMainOptionsDlg :
	public CDialogImpl<CMainOptionsDlg>
{
public:

	static const int IDD = IDD_MAIN_OPTIONS;

	CMainOptionsDlg(Settings &settings);

	bool IsApplied();

	BEGIN_MSG_MAP_EX(CMainOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListBox)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_SECTION_CHANGED, OnSectionChanged)
		MESSAGE_HANDLER(WM_TRY_BREAK_SCREEN, OnTryBreakScreen)
		COMMAND_ID_HANDLER_EX(IDOK, OnSettingsOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnSettingsCancel)
		COMMAND_ID_HANDLER_EX(IDC_APPLY, OnSettingsApply)
		COMMAND_HANDLER_EX(IDC_SETTING_GROUPS, LBN_SELCHANGE, OnSettingGroupSelect)
	END_MSG_MAP()

private:

	Settings &m_settings;

	CListBox m_wndSectionList;
	std::vector<SectionPtr> m_optionSections;
	std::vector<SectionIcon> m_sectionIcons;

	HWND m_hTopWindow = NULL;
	bool m_applied = false;

	CHyperLink m_helpLink;
	CBrush m_sidebarBrush;
	CFont m_sidebarFont, m_sidebarBoldFont;
	double m_sidebarScale = 1.0;  // sidebar spacing grows with the font

	// "Try it" preview of the break screen
	std::vector<BreakWindowPtr> m_previewWindows;
	int m_previewTicks = 0;

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnCtlColorListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnSectionChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnTryBreakScreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	void OnSettingGroupSelect(UINT uCode, int nCtrlID, HWND hwndCtrl);

	void addSection(SectionPtr section, SectionIcon icon, int x, int y);
	void saveSettings();
	void stopPreview();
	void fitSidebar();
	int px(int value) const { return (int)(value * m_sidebarScale + 0.5); }
	CRect sidebarRect();
};

module :private;

static const COLORREF SIDEBAR_COLOR = RGB(0xEE, 0xF0, 0xF3);
static const COLORREF SELECTED_COLOR = RGB(0xFF, 0xFF, 0xFF);
static const COLORREF ACCENT_COLOR = RGB(0x1D, 0x4E, 0xD8);

static const UINT_PTR PREVIEW_TIMER_ID = 1;
static const int PREVIEW_TICK_MS = 100;
static const int PREVIEW_SECONDS = 10;

CMainOptionsDlg::CMainOptionsDlg(Settings &settings):
	m_settings(settings) {
}

void CMainOptionsDlg::addSection(SectionPtr section, SectionIcon icon, int x, int y)
{
	section->Create(m_hWnd);
	section->read(m_settings);
	section->EndLoading();
	section->SetWindowPos(0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_wndSectionList.AddString(section->GetOptionsCategoryName());
	m_optionSections.push_back(section);
	m_sectionIcons.push_back(icon);
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

	m_sidebarBrush.CreateSolidBrush(SIDEBAR_COLOR);

	LOGFONT lf{};
	CFontHandle(GetFont()).GetLogFont(&lf);
	m_sidebarFont.CreateFontIndirect(&lf);
	lf.lfWeight = FW_SEMIBOLD;
	m_sidebarBoldFont.CreateFontIndirect(&lf);

	// 9 pt Segoe UI is 12 px high at 100% scaling
	m_sidebarScale = (std::max)(1.0, std::abs(lf.lfHeight) / 12.0);

	m_wndSectionList.Attach(GetDlgItem(IDC_SETTING_GROUPS));

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

	addSection(std::make_shared<CBreaksSettingsSection>(), SectionIcon::Breaks, x, y);
	addSection(std::make_shared<CScheduleSettingsSection>(), SectionIcon::Schedule, x, y);
	addSection(std::make_shared<CBreakScreenSettingsSection>(), SectionIcon::Screen, x, y);
	addSection(std::make_shared<CExceptionsSettingsSection>(), SectionIcon::Exceptions, x, y);

	if (!m_optionSections.empty()) {
		m_hTopWindow = m_optionSections[0]->m_hWnd;
		::ShowWindow(m_hTopWindow, SW_SHOW);
	}

	fitSidebar();

	m_wndSectionList.SetCurSel(0);
	GetDlgItem(IDC_APPLY).EnableWindow(FALSE);

	return TRUE;
}

LRESULT CMainOptionsDlg::OnSettingsOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	stopPreview();

	for (auto &section : m_optionSections)
		section->SetAccepted(true);

	saveSettings();

	EndDialog(IDOK);
	return 0;
}

LRESULT CMainOptionsDlg::OnSettingsCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (!m_previewWindows.empty()) {
		// Esc while the preview is shown only closes the preview
		stopPreview();
		return 0;
	}

	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainOptionsDlg::OnSettingsApply(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_applied = true;
	saveSettings();
	GetDlgItem(IDC_APPLY).EnableWindow(FALSE);
	return 0;
}

LRESULT CMainOptionsDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	stopPreview();
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
	stopPreview();

	for (auto &section : m_optionSections)
		section->DestroyWindow();

	return 0;
}

CRect CMainOptionsDlg::sidebarRect()
{
	CRect client, list;
	GetClientRect(&client);
	m_wndSectionList.GetWindowRect(&list);
	ScreenToClient(&list);
	return CRect(client.left, client.top, list.right, list.bottom);
}

LRESULT CMainOptionsDlg::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	CDCHandle dc((HDC)wParam);
	CRect client;
	GetClientRect(&client);

	dc.FillRect(&client, ::GetSysColorBrush(COLOR_WINDOW));
	CRect sidebar = sidebarRect();
	dc.FillRect(&sidebar, m_sidebarBrush);
	return 1;
}

LRESULT CMainOptionsDlg::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CMainOptionsDlg::OnCtlColorListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	::SetBkColor((HDC)wParam, SIDEBAR_COLOR);
	return (LRESULT)(HBRUSH)m_sidebarBrush;
}

LRESULT CMainOptionsDlg::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != IDC_SETTING_GROUPS) {
		bHandled = FALSE;
		return FALSE;
	}

	LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
	CRect rc(0, 0, 4, 16);
	MapDialogRect(&rc);
	mis->itemHeight = rc.Height();
	return TRUE;
}

static void drawSectionIcon(Graphics& g, SectionIcon icon, RectF r, const Color& color)
{
	Pen pen(color, (std::max)(1.5f, r.Width / 10));
	pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
	pen.SetLineJoin(LineJoinRound);

	REAL x = r.X, y = r.Y, s = r.Width;
	auto P = [&](REAL px, REAL py) { return PointF(x + s * px / 24, y + s * py / 24); };

	switch (icon) {
	case SectionIcon::Breaks:
		g.DrawEllipse(&pen, x + s * 3 / 24, y + s * 3 / 24, s * 18 / 24, s * 18 / 24);
		g.DrawLine(&pen, P(12, 7), P(12, 12));
		g.DrawLine(&pen, P(12, 12), P(15, 14));
		break;
	case SectionIcon::Screen:
		g.DrawRectangle(&pen, x + s * 3 / 24, y + s * 4 / 24, s * 18 / 24, s * 13 / 24);
		g.DrawLine(&pen, P(8, 21), P(16, 21));
		g.DrawLine(&pen, P(12, 17), P(12, 21));
		break;
	case SectionIcon::Schedule:
		g.DrawRectangle(&pen, x + s * 3 / 24, y + s * 5 / 24, s * 18 / 24, s * 16 / 24);
		g.DrawLine(&pen, P(3, 10), P(21, 10));
		g.DrawLine(&pen, P(8, 3), P(8, 7));
		g.DrawLine(&pen, P(16, 3), P(16, 7));
		break;
	case SectionIcon::Exceptions: {
		GraphicsPath path;
		path.AddLine(P(12, 3), P(20, 6));
		path.AddLine(P(20, 6), P(20, 12));
		path.AddBezier(P(20, 12), P(20, 16.5f), P(16.5f, 19.5f), P(12, 21));
		path.AddBezier(P(12, 21), P(7.5f, 19.5f), P(4, 16.5f), P(4, 12));
		path.AddLine(P(4, 12), P(4, 6));
		path.CloseFigure();
		g.DrawPath(&pen, &path);
		break;
	}
	}
}

LRESULT CMainOptionsDlg::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != IDC_SETTING_GROUPS) {
		bHandled = FALSE;
		return FALSE;
	}

	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
	if (dis->itemID == (UINT)-1)
		return TRUE;

	CDCHandle dc(dis->hDC);
	CRect rc(dis->rcItem);
	bool selected = (dis->itemState & ODS_SELECTED) != 0;
	bool focused = (dis->itemState & ODS_FOCUS) != 0 && !(dis->itemState & ODS_NOFOCUSRECT);

	dc.FillRect(&rc, m_sidebarBrush);

	CRect pill = rc;
	pill.DeflateRect(px(6), px(2));

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	if (selected) {
		GraphicsPath path;
		REAL d = 8;
		RectF r((REAL)pill.left, (REAL)pill.top, (REAL)pill.Width() - 1, (REAL)pill.Height() - 1);
		path.AddArc(r.X, r.Y, d, d, 180, 90);
		path.AddArc(r.GetRight() - d, r.Y, d, d, 270, 90);
		path.AddArc(r.GetRight() - d, r.GetBottom() - d, d, d, 0, 90);
		path.AddArc(r.X, r.GetBottom() - d, d, d, 90, 90);
		path.CloseFigure();
		SolidBrush fill(Color(0xFF, GetRValue(SELECTED_COLOR), GetGValue(SELECTED_COLOR), GetBValue(SELECTED_COLOR)));
		g.FillPath(&fill, &path);
		Pen border(Color(0x30, 0x10, 0x18, 0x28), 1);
		g.DrawPath(&border, &path);
	}

	REAL iconSize = (REAL)(std::min)(pill.Height() - px(8), px(18));
	RectF iconRect((REAL)pill.left + px(10), (REAL)pill.top + (pill.Height() - iconSize) / 2, iconSize, iconSize);
	Color iconColor = selected ? Color(0xFF, GetRValue(ACCENT_COLOR), GetGValue(ACCENT_COLOR), GetBValue(ACCENT_COLOR))
	                           : Color(0xFF, 0x5B, 0x64, 0x70);
	if ((size_t)dis->itemID < m_sectionIcons.size())
		drawSectionIcon(g, m_sectionIcons[dis->itemID], iconRect, iconColor);

	CString text;
	m_wndSectionList.GetText(dis->itemID, text);

	CRect textRect = pill;
	textRect.left = (int)(iconRect.GetRight() + px(10));
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(selected ? RGB(0x1B, 0x1F, 0x24) : RGB(0x3A, 0x42, 0x4D));
	HFONT oldFont = dc.SelectFont(selected ? m_sidebarBoldFont : m_sidebarFont);
	dc.DrawText(text, -1, &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
	dc.SelectFont(oldFont);

	if (focused && ::GetFocus() == m_wndSectionList.m_hWnd)
		dc.DrawFocusRect(&pill);

	return TRUE;
}

LRESULT CMainOptionsDlg::OnSectionChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	GetDlgItem(IDC_APPLY).EnableWindow(TRUE);
	return 0;
}

LRESULT CMainOptionsDlg::OnTryBreakScreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	stopPreview();

	// the values entered in the dialog, without saving them
	Settings preview = m_settings;
	for (auto &section : m_optionSections)
		section->write(preview);

	bool fullscreen = preview.getBoolean(Settings::APPEARANCE_FULLSCREEN, Settings::DEFAULT_APPEARANCE_FULLSCREEN);
	m_previewWindows = createBreakWindows(preview, PREVIEW_SECONDS, false, fullscreen);
	m_previewTicks = 0;

	SetTimer(PREVIEW_TIMER_ID, PREVIEW_TICK_MS);
	return 0;
}

LRESULT CMainOptionsDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam != PREVIEW_TIMER_ID) {
		bHandled = FALSE;
		return 0;
	}

	++m_previewTicks;

	bool escape = (::GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
	bool clicked = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 && m_previewTicks > 5;

	if (escape || clicked || m_previewTicks >= PREVIEW_SECONDS * 1000 / PREVIEW_TICK_MS) {
		stopPreview();
		return 0;
	}

	if (m_previewTicks % (1000 / PREVIEW_TICK_MS) == 0)
		for (const BreakWindowPtr& window : m_previewWindows)
			window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);

	return 0;
}

void CMainOptionsDlg::stopPreview()
{
	if (IsWindow())
		KillTimer(PREVIEW_TIMER_ID);

	for (auto it = m_previewWindows.rbegin(); it != m_previewWindows.rend(); ++it)
		if ((*it)->IsWindow())
			(*it)->SendMessage(WM_CLOSE);

	m_previewWindows.clear();
}

// Widens the sidebar when the page names don't fit (large fonts or display
// scaling), moving everything to its right and growing the dialog.
void CMainOptionsDlg::fitSidebar()
{
	CClientDC dc(m_wndSectionList);
	HFONT oldFont = dc.SelectFont(m_sidebarBoldFont);

	int textWidth = 0;
	for (int i = 0; i < m_wndSectionList.GetCount(); ++i) {
		CString text;
		m_wndSectionList.GetText(i, text);
		CSize size;
		dc.GetTextExtent(text, text.GetLength(), &size);
		textWidth = (std::max)(textWidth, (int)size.cx);
	}
	dc.SelectFont(oldFont);

	// pill margins, icon padding, icon, gap, text, right padding
	int needed = px(6) + px(10) + px(18) + px(10) + textWidth + px(12) + px(6);

	CRect list;
	m_wndSectionList.GetWindowRect(&list);
	ScreenToClient(&list);

	int delta = needed - list.Width();
	if (delta <= 0)
		return;

	CRect client;
	GetClientRect(&client);

	for (HWND child = ::GetWindow(m_hWnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT)) {
		CRect rc;
		::GetWindowRect(child, &rc);
		ScreenToClient(&rc);

		if (child == m_wndSectionList.m_hWnd)
			rc.right += delta;  // the sidebar itself
		else if (rc.left <= client.left + 1 && rc.Height() <= 4)
			rc.right += delta;  // the separator line spans the whole dialog
		else if (rc.left >= list.right)
			rc.OffsetRect(delta, 0);  // pages and buttons right of the sidebar
		else
			continue;  // under the sidebar, e.g. the help link

		::SetWindowPos(child, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	CRect window;
	GetWindowRect(&window);
	SetWindowPos(NULL, 0, 0, window.Width() + delta, window.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	CenterWindow();
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

module;

#include "wtl.h"
#include "resource.h"

#include <memory>
#include <set>
#include <string>
#include "tstring.h"

export module SectionDlg;


import Settings;

// Posted to the main settings dialog whenever a section's values change.
export constexpr UINT WM_SECTION_CHANGED = WM_APP + 0x51;

// Base class of the pages shown in the settings window. Pages paint on the
// window color; the title gets a larger bold font and muted statics get a
// secondary text color.
export class CSectionDlg: public CDialogImpl<CSectionDlg>
{
public:

	int IDD;

	CSectionDlg(int idd) :
		IDD(idd),
		m_accepted(false)
	{
	}

	virtual ~CSectionDlg()
	{
	}

	const CString &GetOptionsCategoryName()
	{
		return m_optionsCategoryName;
	}

	void SetOptionsCategoryName(const CString &name)
	{
		m_optionsCategoryName = name;
	}

	void SetAccepted(bool accepted) { m_accepted = accepted; }
	bool Accepted() { return m_accepted; }

	// Called by the main dialog once the page has been created and read.
	void EndLoading() { m_loading = false; }

	virtual void read(Settings &settings)
	{}

	virtual void write(Settings &settings)
	{}

	BEGIN_MSG_MAP(CSectionDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_DESTROY, OnBaseDestroy)
	END_MSG_MAP()

protected:

	bool m_accepted;
	// true while the page is being created and filled, so change handlers stay quiet
	bool m_loading = true;
	CString m_optionsCategoryName;

	CFont m_titleFont;
	std::set<int> m_mutedIds;

	// Call from the page's WM_INITDIALOG handler.
	void InitSectionHeader()
	{
		CFontHandle dialogFont = GetFont();
		LOGFONT lf{};
		dialogFont.GetLogFont(&lf);
		lf.lfWeight = FW_SEMIBOLD;
		lf.lfHeight = MulDiv(lf.lfHeight, 3, 2);
		m_titleFont.CreateFontIndirect(&lf);

		CWindow title = GetDlgItem(IDC_SECTION_TITLE);
		if (title)
			title.SetFont(m_titleFont);

		MarkMuted(IDC_SECTION_DESC);
	}

	void MarkMuted(int id) { m_mutedIds.insert(id); }

	// Tell the main dialog that something changed (enables Apply).
	void NotifyChanged()
	{
		if (!m_loading && IsWindow())
			::PostMessage(GetParent(), WM_SECTION_CHANGED, 0, 0);
	}

	int GetInt(int id, int fallback = 0)
	{
		BOOL ok = FALSE;
		int value = (int)GetDlgItemInt(id, &ok, FALSE);
		return ok ? value : fallback;
	}

	void SetInt(int id, int value)
	{
		SetDlgItemInt(id, value, FALSE);
	}

	bool IsChecked(int id) { return IsDlgButtonChecked(id) == BST_CHECKED; }
	void SetChecked(int id, bool checked) { CheckDlgButton(id, checked ? BST_CHECKED : BST_UNCHECKED); }

	void Enable(int id, bool enable) { CWindow(GetDlgItem(id)).EnableWindow(enable); }

	void InitSpin(int id, int pos, int lo, int hi, int accel = 0)
	{
		CUpDownCtrl spin(GetDlgItem(id));
		spin.SetRange32(lo, hi);
		spin.SetPos32(pos);
		if (accel > 0) {
			UDACCEL steps[] = { { 0, (UINT)accel } };
			spin.SetAccel(1, steps);
		}
	}

	LRESULT OnCtlColorDlg(UINT, WPARAM, LPARAM, BOOL&)
	{
		return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
	}

	LRESULT OnCtlColorStatic(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		HDC dc = (HDC)wParam;
		int id = ::GetDlgCtrlID((HWND)lParam);

		::SetBkColor(dc, ::GetSysColor(COLOR_WINDOW));

		bool enabled = ::IsWindowEnabled((HWND)lParam) != FALSE;
		if (m_mutedIds.count(id) && enabled)
			::SetTextColor(dc, RGB(0x5B, 0x64, 0x70));
		else if (!enabled)
			::SetTextColor(dc, ::GetSysColor(COLOR_GRAYTEXT));
		else
			::SetTextColor(dc, ::GetSysColor(COLOR_WINDOWTEXT));

		return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
	}

	LRESULT OnBaseDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}
};

export using SectionPtr = std::shared_ptr<CSectionDlg>;

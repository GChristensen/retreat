// Small custom-painted controls used by the settings pages. Each one subclasses
// a static placeholder from the dialog template and draws with GDI+.
module;

#include "wtl.h"

#include <objidl.h>
#include <gdiplus.h>

#include <algorithm>
#include <bitset>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include "tstring.h"

export module SettingsControls;

import Settings;
import SettingsModel;

using namespace Gdiplus;

namespace palette {
	const Color text(0xFF, 0x1B, 0x1F, 0x24);
	const Color muted(0xFF, 0x5B, 0x64, 0x70);
	const Color track(0xFF, 0xF1, 0xF3, 0xF6);
	const Color trackBorder(0xFF, 0xB5, 0xBC, 0xC6);
	const Color work(0xFF, 0x3B, 0x6F, 0xE0);
	const Color warn(0xFF, 0xF2, 0xA9, 0x3B);
	const Color brk(0xFF, 0x0E, 0x7C, 0x6B);
	const Color brkLight(0xFF, 0xD5, 0xEE, 0xE9);
	const Color brkMid(0xFF, 0x7F, 0xBF, 0xB4);
	const Color brkDark(0xFF, 0x07, 0x3F, 0x37);
	const Color postponeLine(0xFF, 0xE0, 0xB4, 0x6A);
	const Color postponeFill(0xFF, 0xFF, 0xF5, 0xE6);
	const Color postponeText(0xFF, 0x7A, 0x4A, 0x06);
	const Color idle(0xFF, 0x8F, 0xD1, 0xC5);
	const Color white(0xFF, 0xFF, 0xFF, 0xFF);
	const Color gray1(0xFF, 0xB8, 0xBE, 0xC6);
	const Color gray2(0xFF, 0xD3, 0xD7, 0xDC);
}

static Color fromColorRef(COLORREF c, BYTE alpha = 0xFF) {
	return Color(alpha, GetRValue(c), GetGValue(c), GetBValue(c));
}

static void roundRectPath(GraphicsPath& path, const RectF& r, REAL radius) {
	REAL d = (std::min)(radius * 2, (std::min)(r.Width, r.Height));
	if (d <= 1) {
		path.AddRectangle(r);
		return;
	}
	path.AddArc(r.X, r.Y, d, d, 180, 90);
	path.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
	path.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
	path.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
	path.CloseFigure();
}

static void fillRound(Graphics& g, const RectF& r, REAL radius, const Brush& brush) {
	GraphicsPath path;
	roundRectPath(path, r, radius);
	g.FillPath(&brush, &path);
}

static void strokeRound(Graphics& g, const RectF& r, REAL radius, const Pen& pen) {
	GraphicsPath path;
	roundRectPath(path, r, radius);
	g.DrawPath(&pen, &path);
}

static REAL textWidth(Graphics& g, const Font& font, const tstring& s) {
	RectF bounds;
	g.MeasureString(s.c_str(), (INT)s.size(), &font, PointF(0, 0), &bounds);
	return bounds.Width;
}

enum class Align { Left, Center, Right };

static void drawText(Graphics& g, const Font& font, const tstring& s, const RectF& r, const Color& color,
                     Align align = Align::Left) {
	StringFormat fmt;
	fmt.SetLineAlignment(StringAlignmentCenter);
	fmt.SetAlignment(align == Align::Left ? StringAlignmentNear
	                 : align == Align::Center ? StringAlignmentCenter : StringAlignmentFar);
	fmt.SetTrimming(StringTrimmingNone);
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);
	SolidBrush brush(color);
	g.DrawString(s.c_str(), (INT)s.size(), &font, r, &fmt, &brush);
}

// Draws a text only if it fits into the rectangle.
static void drawTextIfFits(Graphics& g, const Font& font, const tstring& s, const RectF& r,
                           const Color& color, Align align = Align::Center) {
	if (textWidth(g, font, s) + 6 <= r.Width)
		drawText(g, font, s, r, color, align);
}

struct PaintFonts {
	Font& normal;
	Font& caption;
	Font& bold;
};

// Base: double-buffered painting into a subclassed static.
template <class T>
class CPaintedControl : public CWindowImpl<T> {
public:
	BEGIN_MSG_MAP(CPaintedControl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
	END_MSG_MAP()

	void Refresh() {
		if (this->IsWindow())
			this->Invalidate(FALSE);
	}

protected:
	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) { return 1; }

	LRESULT OnEnable(UINT, WPARAM, LPARAM, BOOL&) {
		Refresh();
		return 0;
	}

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
		CPaintDC paintDC(this->m_hWnd);
		CRect rc;
		this->GetClientRect(&rc);

		CDC memDC;
		memDC.CreateCompatibleDC(paintDC);
		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(paintDC, rc.Width(), rc.Height());
		HBITMAP oldBitmap = memDC.SelectBitmap(bitmap);

		memDC.FillRect(&rc, ::GetSysColorBrush(COLOR_WINDOW));

		{
			Graphics g(memDC);
			g.SetSmoothingMode(SmoothingModeAntiAlias);
			g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			HFONT hFont = CWindow(this->GetParent()).GetFont();
			Font font(memDC, hFont);

			LOGFONT lf{};
			::GetObject(hFont, sizeof(lf), &lf);
			lf.lfHeight = MulDiv(lf.lfHeight, 85, 100);
			CFont smallHFont;
			smallHFont.CreateFontIndirect(&lf);
			Font captionFont(memDC, smallHFont);

			lf.lfHeight = MulDiv(lf.lfHeight * 100 / 85, 100, 100);
			lf.lfWeight = FW_SEMIBOLD;
			CFont boldHFont;
			boldHFont.CreateFontIndirect(&lf);
			Font boldFont(memDC, boldHFont);

			PaintFonts fonts{ font, captionFont, boldFont };
			static_cast<T*>(this)->Draw(g, RectF((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height()),
			                           fonts, this->IsWindowEnabled() != FALSE);
		}

		paintDC.BitBlt(0, 0, rc.Width(), rc.Height(), memDC, 0, 0, SRCCOPY);
		memDC.SelectBitmap(oldBitmap);
		return 0;
	}

public:
	using Fonts = PaintFonts;
};

//// CCycleBar ///////////////////////////////////////////////////////////////

export struct CycleModel {
	int periodMin = 60;
	int breakMin = 10;
	int alertMin = 1;
	int delayCount = 2;
	int delayMin = 3;
	bool alignToClock = false;
	bool enabled = true;
};

export class CCycleBar : public CPaintedControl<CCycleBar> {
public:
	DECLARE_WND_SUPERCLASS(_T("RetreatCycleBar"), _T("Static"))

	void SetModel(const CycleModel& model) {
		m_model = model;
		Refresh();
	}

	void Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled);

private:
	CycleModel m_model;
};

void CCycleBar::Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled) {
	const bool on = m_model.enabled && windowEnabled;
	CycleTiming t = clampCycle(m_model.periodMin, m_model.breakMin, m_model.alertMin);
	const int post = (std::max)(0, m_model.delayCount) * (std::max)(0, m_model.delayMin);
	const int scale = t.periodMin + post;

	auto X = [&](int minutes) { return rc.X + rc.Width * minutes / scale; };
	const REAL gap = 2;

	REAL row1H, row2Top = 0, row2H = 0, ticksTop;
	if (post > 0) {
		row1H = std::round(rc.Height * 0.40f);
		row2Top = rc.Y + row1H + std::round(rc.Height * 0.07f);
		row2H = std::round(rc.Height * 0.25f);
		ticksTop = row2Top + row2H + 2;
	}
	else {
		row1H = std::round(rc.Height * 0.58f);
		ticksTop = rc.Y + row1H + 4;
	}

	Color cWork = on ? palette::work : palette::gray1;
	Color cWarn = on ? palette::warn : palette::gray2;
	Color cBreak = on ? palette::brk : palette::gray1;

	// row 1: work (with the warning at its tail), break, next cycle
	RectF work(X(0), rc.Y, X(t.workMin()) - X(0) - gap / 2, row1H);
	{
		GraphicsPath workPath;
		roundRectPath(workPath, work, 4);
		SolidBrush b(cWork);
		g.FillPath(&b, &workPath);

		Region clip(&workPath);
		g.SetClip(&clip);
		RectF alert(X(t.alertStartMin()), rc.Y, X(t.workMin()) - X(t.alertStartMin()), row1H);
		if (alert.Width < 4) {
			alert.X = work.GetRight() - 4;
			alert.Width = 4;
		}
		SolidBrush wb(cWarn);
		g.FillRectangle(&wb, alert);
		g.ResetClip();

		tstring label = _T("Work ") + to_tstring(t.workMin()) + _T(" min");
		RectF labelRect(work.X + 6, work.Y, work.Width - 12 - alert.Width, work.Height);
		if (textWidth(g, fonts.bold, label) <= labelRect.Width)
			drawText(g, fonts.bold, label, labelRect, palette::white);
	}

	RectF brk(X(t.workMin()) + gap / 2, rc.Y, X(t.periodMin) - X(t.workMin()) - gap, row1H);
	{
		SolidBrush b(cBreak);
		fillRound(g, brk, 4, b);
		tstring label = _T("Break ") + to_tstring(t.breakMin);
		if (textWidth(g, fonts.bold, label) + 6 <= brk.Width)
			drawText(g, fonts.bold, label, brk, palette::white, Align::Center);
		else
			drawTextIfFits(g, fonts.bold, to_tstring(t.breakMin), brk, palette::white);
	}

	if (post > 0) {
		RectF next(X(t.periodMin) + gap / 2, rc.Y + 0.5f, X(scale) - X(t.periodMin) - gap / 2 - 0.5f, row1H - 1);
		SolidBrush fill(palette::track);
		fillRound(g, next, 4, fill);
		Pen pen(palette::trackBorder, 1);
		pen.SetDashStyle(DashStyleDash);
		strokeRound(g, next, 4, pen);
		drawTextIfFits(g, fonts.caption, _T("next cycle"), next, palette::muted);

		// row 2: the postponed break spilling into the next cycle
		tstring times = m_model.delayCount == 1 ? _T("once") : m_model.delayCount == 2 ? _T("twice")
			: to_tstring(m_model.delayCount) + _T(" times");
		tstring label = _T("If postponed ") + times + _T(":");
		RectF labelRect(rc.X, row2Top, X(t.workMin()) - rc.X - 6, row2H);
		if (textWidth(g, fonts.caption, label) <= labelRect.Width)
			drawText(g, fonts.caption, label, labelRect, palette::muted, Align::Right);

		RectF postponed(X(t.workMin()) + gap / 2, row2Top, X(t.workMin() + post) - X(t.workMin()) - gap, row2H);
		HatchBrush hatch(HatchStyleWideUpwardDiagonal, on ? palette::postponeLine : palette::gray2,
		                 on ? palette::postponeFill : palette::white);
		fillRound(g, postponed, 3, hatch);
		Pen postponePen(on ? palette::postponeLine : palette::gray2, 1);
		strokeRound(g, postponed, 3, postponePen);
		tstring plus = _T("+") + to_tstring(post) + _T(" min");
		if (textWidth(g, fonts.caption, plus) + 6 <= postponed.Width) {
			SolidBrush back(on ? palette::postponeFill : palette::white);
			RectF tb = postponed;
			REAL w = textWidth(g, fonts.caption, plus) + 4;
			tb.X += (tb.Width - w) / 2;
			tb.Width = w;
			tb.Y += 2;
			tb.Height -= 4;
			g.FillRectangle(&back, tb);
			drawText(g, fonts.caption, plus, postponed, on ? palette::postponeText : palette::muted, Align::Center);
		}

		RectF ghost(X(t.workMin() + post) + gap / 2, row2Top, X(scale) - X(t.workMin() + post) - gap / 2, row2H);
		SolidBrush ghostFill(on ? palette::brkLight : palette::track);
		fillRound(g, ghost, 3, ghostFill);
		Pen ghostPen(cBreak, 1);
		strokeRound(g, ghost, 3, ghostPen);
		drawTextIfFits(g, fonts.caption, _T("Break"), ghost, on ? palette::brkDark : palette::muted);
	}

	// tick labels
	auto clock = [&](int minutes) { return formatClock((9 * 60 + minutes) / 60, (9 * 60 + minutes) % 60); };
	struct Tick { int minutes; tstring label; bool atEnd; };
	std::vector<Tick> ticks;
	ticks.push_back({ 0, m_model.alignToClock ? clock(0) : tstring(_T("start")), false });
	ticks.push_back({ t.workMin(), m_model.alignToClock ? _T("break ") + clock(t.workMin())
	                                 : _T("break at ") + to_tstring(t.workMin()) + _T(" min"), false });
	ticks.push_back({ t.periodMin, m_model.alignToClock ? _T("next ") + clock(t.periodMin)
	                                : _T("cycle ends at ") + to_tstring(t.periodMin), post == 0 });
	if (post > 0)
		ticks.push_back({ scale, _T("+") + to_tstring(post), true });

	REAL lastRight = -1e9f;
	REAL ticksH = rc.GetBottom() - ticksTop;
	Pen tickPen(palette::trackBorder, 1);
	for (size_t i = 0; i < ticks.size(); ++i) {
		const Tick& tick = ticks[i];
		REAL w = textWidth(g, fonts.caption, tick.label);
		REAL x = X(tick.minutes);
		REAL left = tick.atEnd ? rc.GetRight() - w : (i == 0 ? x : x + 3);
		if (left < lastRight + 4 || left + w > rc.GetRight() + 1)
			continue;
		if (i > 0 && !tick.atEnd)
			g.DrawLine(&tickPen, x, ticksTop + 1, x, ticksTop + ticksH - 2);
		drawText(g, fonts.caption, tick.label, RectF(left, ticksTop, w + 2, ticksH), palette::muted);
		lastRight = left + w;
	}
}

//// CIdleStrip //////////////////////////////////////////////////////////////

export class CIdleStrip : public CPaintedControl<CIdleStrip> {
public:
	DECLARE_WND_SUPERCLASS(_T("RetreatIdleStrip"), _T("Static"))

	void SetModel(int idleMinutes, bool enabled) {
		m_minutes = idleMinutes;
		m_enabled = enabled;
		Refresh();
	}

	void Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled) {
		const bool on = m_enabled && windowEnabled;
		const int span = (std::max)(30, m_minutes + 5);
		REAL trackH = std::round(rc.Height * 0.6f);
		RectF track(rc.X, rc.Y, rc.Width, trackH);

		SolidBrush trackBrush(palette::track);
		fillRound(g, track, 4, trackBrush);

		auto X = [&](REAL minutes) { return rc.X + rc.Width * minutes / span; };
		REAL idleStart = (REAL)(span - m_minutes);

		// keyboard/mouse activity before the idle window
		static const REAL activity[] = { 1, 2.5f, 3, 5, 6.5f, 7, 9, 10.5f, 11, 12.5f, 14, 15, 16.5f, 17, 18.5f,
		                                 19.5f, 21, 22, 23.5f, 24.5f, 26, 27, 28.5f, 30, 31.5f, 33 };
		SolidBrush activityBrush(on ? palette::work : palette::gray1);
		for (REAL m : activity) {
			REAL pos = m * span / 34.0f;
			if (pos >= idleStart - 0.3f)
				break;
			g.FillRectangle(&activityBrush, RectF(X(pos), track.Y + trackH * 0.22f, 2.5f, trackH * 0.56f));
		}

		RectF zone(X(idleStart), track.Y, track.GetRight() - X(idleStart) - 3, trackH);
		SolidBrush zoneBrush(on ? palette::idle : palette::gray2);
		g.FillRectangle(&zoneBrush, zone);
		Pen zonePen(on ? palette::brk : palette::gray1, 1);
		g.DrawLine(&zonePen, zone.X, zone.Y, zone.X, zone.GetBottom());
		drawTextIfFits(g, fonts.caption, _T("no input for ") + to_tstring(m_minutes) + _T(" min"), zone,
		               on ? palette::brkDark : palette::muted);

		SolidBrush dueBrush(palette::text);
		g.FillRectangle(&dueBrush, RectF(track.GetRight() - 3, track.Y, 3, trackH));

		RectF labels(rc.X, track.GetBottom() + 1, rc.Width, rc.Height - trackH - 1);
		drawText(g, fonts.caption, to_tstring(span) + _T(" min before"), labels, palette::muted, Align::Left);
		drawText(g, fonts.caption, _T("warning due"), labels, palette::muted, Align::Right);
	}

private:
	int m_minutes = 10;
	bool m_enabled = true;
};

//// CWeekStrip //////////////////////////////////////////////////////////////

export constexpr WORD WSN_SELECT = 0x0100;

export struct WeekItem {
	std::bitset<7> days;
	int hour = 0;
	int minute = 0;
	int durationMin = 0;
};

export class CWeekStrip : public CPaintedControl<CWeekStrip> {
public:
	DECLARE_WND_SUPERCLASS(_T("RetreatWeekStrip"), _T("Static"))

	BEGIN_MSG_MAP(CWeekStrip)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		CHAIN_MSG_MAP(CPaintedControl<CWeekStrip>)
	END_MSG_MAP()

	// Items that cannot be shown on the strip (custom schedules) are passed
	// as std::nullopt-like entries with no days set.
	void SetItems(const std::vector<WeekItem>& items, int selected) {
		m_items = items;
		m_selected = selected;
		Refresh();
	}

	int GetClickedItem() const { return m_clicked; }

	void Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled);

private:
	std::vector<WeekItem> m_items;
	int m_selected = -1;
	int m_clicked = -1;

	struct Hit { RectF rect; int index; };
	std::vector<Hit> m_hits;

	LRESULT OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&) {
		REAL x = (REAL)GET_X_LPARAM(lParam), y = (REAL)GET_Y_LPARAM(lParam);
		for (const Hit& hit : m_hits) {
			RectF r = hit.rect;
			r.Inflate(2, 2);
			if (r.Contains(x, y)) {
				m_clicked = hit.index;
				GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), WSN_SELECT), (LPARAM)m_hWnd);
				break;
			}
		}
		return 0;
	}

	LRESULT OnSetCursor(UINT, WPARAM, LPARAM, BOOL& bHandled) {
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		for (const Hit& hit : m_hits) {
			if (hit.rect.Contains((REAL)pt.x, (REAL)pt.y)) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return TRUE;
			}
		}
		bHandled = FALSE;
		return 0;
	}
};

void CWeekStrip::Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled) {
	static const TCHAR* dayNames[] = { _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat"), _T("Sun") };
	const int START = 6 * 60, SPAN = 18 * 60;

	m_hits.clear();

	REAL labelW = textWidth(g, fonts.caption, _T("Wed")) + 8;
	REAL headerH = rc.Height / 8.5f;
	REAL rowH = (rc.Height - headerH) / 7;
	RectF area(rc.X + labelW, rc.Y, rc.Width - labelW, rc.Height);

	auto X = [&](int minutes) { return area.X + area.Width * (minutes - START) / SPAN; };

	for (int h = 6; h <= 24; h += 3) {
		tstring label = to_tstring(h == 24 ? 0 : h) + _T(":00");
		REAL w = textWidth(g, fonts.caption, label);
		REAL x = X(h * 60);
		REAL left = h == 6 ? x : h == 24 ? area.GetRight() - w : x - w / 2;
		drawText(g, fonts.caption, label, RectF(left, rc.Y, w + 2, headerH), palette::muted);
	}

	for (int d = 0; d < 7; ++d) {
		REAL top = rc.Y + headerH + d * rowH;
		drawText(g, fonts.caption, dayNames[d], RectF(rc.X, top, labelW, rowH), palette::muted);

		RectF track(area.X, top + rowH * 0.15f, area.Width, rowH * 0.7f);
		SolidBrush trackBrush(palette::track);
		fillRound(g, track, 3, trackBrush);

		for (size_t i = 0; i < m_items.size(); ++i) {
			const WeekItem& item = m_items[i];
			if (!item.days.test(d))
				continue;

			int start = item.hour * 60 + item.minute;
			int from = (std::clamp)(start, START, START + SPAN);
			int to = (std::clamp)(start + (std::max)(1, item.durationMin), START, START + SPAN);

			RectF block(X(from), track.Y, (std::max)(4.0f, X(to) - X(from)), track.Height);
			if (block.GetRight() > track.GetRight())
				block.X = track.GetRight() - block.Width;

			bool selected = windowEnabled && (int)i == m_selected;
			SolidBrush brush(!windowEnabled ? Color(0xFF, 0xC4, 0xC9, 0xD0) : selected ? palette::brk : palette::brkMid);
			fillRound(g, block, 2, brush);
			if (selected) {
				Pen pen(palette::brkDark, 1.5f);
				strokeRound(g, block, 2, pen);
			}
			if (windowEnabled)
				m_hits.push_back({ block, (int)i });
		}
	}
}

//// CBreakPreview ///////////////////////////////////////////////////////////

export struct PreviewModel {
	bool fullscreen = true;
	bool useImages = false;
	std::shared_ptr<Bitmap> image;  // a sample image from the folder, if any
	bool stretch = true;
	COLORREF backgroundColor = RGB(255, 255, 255);
	int alpha = 128;                // 0..255
	bool transparentRegions = false;
	bool showTimer = true;
	COLORREF timerColor = RGB(0xEF, 0x1B, 0x1B);
	TimerPosition timerPosition = TimerPosition::TopLeft;
	int timerSize = 32;             // points, full screen
};

// WM_COMMAND notification code: the preview was clicked to move the countdown.
export constexpr WORD BPN_TIMER_POSITION = 0x0101;

export class CBreakPreview : public CPaintedControl<CBreakPreview> {
public:
	DECLARE_WND_SUPERCLASS(_T("RetreatBreakPreview"), _T("Static"))

	void SetModel(const PreviewModel& model) {
		m_model = model;
		Refresh();
	}

	// True when the real break screen will fall back to full-screen stars.
	bool IsStarFallback() const {
		return !m_model.fullscreen && !(m_model.useImages && m_model.image);
	}

	void Draw(Graphics& g, RectF rc, Fonts& fonts, bool windowEnabled);

	TimerPosition GetClickedPosition() const { return m_clicked; }

	BEGIN_MSG_MAP(CBreakPreview)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		CHAIN_MSG_MAP(CPaintedControl<CBreakPreview>)
	END_MSG_MAP()

private:
	PreviewModel m_model;
	RectF m_overlay;                // the break-screen area of the last paint
	TimerPosition m_clicked = TimerPosition::TopLeft;

	bool canClick() const { return m_model.showTimer && m_overlay.Width > 0 && IsWindowEnabled(); }

	// Corners by quadrant; the middle third of the screen means the center.
	TimerPosition positionAt(REAL x, REAL y) const {
		REAL fx = (x - m_overlay.X) / m_overlay.Width, fy = (y - m_overlay.Y) / m_overlay.Height;
		if (fx > 1.0f / 3 && fx < 2.0f / 3 && fy > 1.0f / 3 && fy < 2.0f / 3)
			return TimerPosition::Center;
		bool right = fx >= 0.5f, bottom = fy >= 0.5f;
		return bottom ? (right ? TimerPosition::BottomRight : TimerPosition::BottomLeft)
		              : (right ? TimerPosition::TopRight : TimerPosition::TopLeft);
	}

	LRESULT OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&) {
		REAL x = (REAL)GET_X_LPARAM(lParam), y = (REAL)GET_Y_LPARAM(lParam);
		if (canClick() && m_overlay.Contains(x, y)) {
			m_clicked = positionAt(x, y);
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BPN_TIMER_POSITION), (LPARAM)m_hWnd);
		}
		return 0;
	}

	LRESULT OnSetCursor(UINT, WPARAM, LPARAM, BOOL& bHandled) {
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		if (canClick() && m_overlay.Contains((REAL)pt.x, (REAL)pt.y)) {
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return TRUE;
		}
		bHandled = FALSE;
		return 0;
	}
};

static void fillStar(Graphics& g, REAL cx, REAL cy, REAL r, REAL angleDeg, const Color& color) {
	PointF pts[10];
	const REAL pi = 3.14159265f;
	for (int i = 0; i < 10; ++i) {
		REAL radius = (i % 2 == 0) ? r : r * 0.42f;
		REAL a = (angleDeg - 90 + i * 36) * pi / 180;
		pts[i] = PointF(cx + radius * std::cos(a), cy + radius * std::sin(a));
	}
	SolidBrush brush(color);
	g.FillPolygon(&brush, pts, 10);
}

void CBreakPreview::Draw(Graphics& g, RectF rc, Fonts& fonts, bool) {
	// monitor
	REAL standH = (std::max)(6.0f, rc.Height * 0.08f);
	RectF bezel(rc.X, rc.Y, rc.Width, rc.Height - standH);
	SolidBrush bezelBrush(Color(0xFF, 0x1F, 0x24, 0x2B));
	fillRound(g, bezel, 5, bezelBrush);
	g.FillRectangle(&bezelBrush, RectF(rc.X + rc.Width * 0.42f, bezel.GetBottom(), rc.Width * 0.16f, standH * 0.6f));
	g.FillRectangle(&bezelBrush, RectF(rc.X + rc.Width * 0.34f, rc.GetBottom() - standH * 0.4f, rc.Width * 0.32f, standH * 0.4f));

	RectF screen = bezel;
	screen.Inflate(-5, -5);

	// desktop
	SolidBrush desktop(Color(0xFF, 0x3E, 0x5C, 0x7A));
	g.FillRectangle(&desktop, screen);
	SolidBrush win1(Color(0xFF, 0xE9, 0xED, 0xF2)), win1t(Color(0xFF, 0xC9, 0xD1, 0xDB));
	SolidBrush win2(Color(0xFF, 0xF7, 0xF8, 0xFA)), win2t(Color(0xFF, 0xD4, 0xDA, 0xE2));
	RectF w1(screen.X + screen.Width * 0.06f, screen.Y + screen.Height * 0.08f, screen.Width * 0.48f, screen.Height * 0.5f);
	RectF w2(screen.X + screen.Width * 0.4f, screen.Y + screen.Height * 0.32f, screen.Width * 0.46f, screen.Height * 0.46f);
	g.FillRectangle(&win1, w1);
	g.FillRectangle(&win1t, RectF(w1.X, w1.Y, w1.Width, screen.Height * 0.06f));
	g.FillRectangle(&win2, w2);
	g.FillRectangle(&win2t, RectF(w2.X, w2.Y, w2.Width, screen.Height * 0.06f));
	SolidBrush taskbar(Color(0xFF, 0x1F, 0x29, 0x37));
	g.FillRectangle(&taskbar, RectF(screen.X, screen.GetBottom() - screen.Height * 0.08f, screen.Width, screen.Height * 0.08f));

	const bool showImage = m_model.useImages && m_model.image;
	const bool fullscreen = m_model.fullscreen || !showImage;
	const BYTE alpha = (BYTE)(std::clamp)(m_model.alpha, 0, 255);

	RectF overlay = screen;
	if (!fullscreen) {
		overlay = RectF(screen.X + screen.Width * 0.2f, screen.Y + screen.Height * 0.16f,
		                screen.Width * 0.6f, screen.Height * 0.62f);
	}

	const int screenW = (std::max)(1, ::GetSystemMetrics(SM_CXSCREEN));
	const REAL k = screen.Width / screenW;

	GraphicsState state = g.Save();
	g.SetClip(overlay);

	if (showImage) {
		Bitmap& image = *m_model.image;
		SolidBrush canvas(fromColorRef(m_model.backgroundColor, alpha));
		g.FillRectangle(&canvas, overlay);

		REAL iw = (REAL)image.GetWidth(), ih = (REAL)image.GetHeight();
		// fit keeping the aspect ratio; original size is only reduced when too large
		REAL fit = (std::min)(overlay.Width / iw, overlay.Height / ih);
		REAL scale = m_model.stretch ? fit : (std::min)(fit, k);
		RectF dst(overlay.X + (overlay.Width - iw * scale) / 2, overlay.Y + (overlay.Height - ih * scale) / 2,
		          iw * scale, ih * scale);

		ColorMatrix cm = { 1, 0, 0, 0, 0,
		                   0, 1, 0, 0, 0,
		                   0, 0, 1, 0, 0,
		                   0, 0, 0, alpha / 255.0f, 0,
		                   0, 0, 0, 0, 1 };
		ImageAttributes attributes;
		attributes.SetColorMatrix(&cm);
		if (m_model.transparentRegions) {
			Color key;
			image.GetPixel(0, 0, &key);
			attributes.SetColorKey(key, key);
		}
		g.DrawImage(&image, dst, 0, 0, iw, ih, UnitPixel, &attributes);
	}
	else {
		// random stars on a see-through background, as in Shapes.cppm
		static const struct { REAL x, y, r; int color; int angle; } stars[] = {
			{ 0.08f, 0.62f, 0.040f, 0, 10 }, { 0.18f, 0.28f, 0.025f, 1, 40 }, { 0.27f, 0.80f, 0.034f, 2, 70 },
			{ 0.35f, 0.47f, 0.046f, 3, 15 }, { 0.45f, 0.16f, 0.028f, 4, 50 }, { 0.52f, 0.70f, 0.052f, 5, 80 },
			{ 0.60f, 0.34f, 0.030f, 6, 20 }, { 0.66f, 0.86f, 0.024f, 0, 60 }, { 0.72f, 0.14f, 0.040f, 2, 5 },
			{ 0.78f, 0.56f, 0.034f, 1, 30 }, { 0.86f, 0.28f, 0.048f, 3, 65 }, { 0.92f, 0.76f, 0.028f, 4, 45 },
			{ 0.14f, 0.92f, 0.030f, 5, 25 }, { 0.42f, 0.92f, 0.022f, 6, 75 }, { 0.95f, 0.46f, 0.036f, 0, 35 },
			{ 0.24f, 0.08f, 0.022f, 1, 55 },
		};
		static const COLORREF colors[] = { RGB(31, 176, 255), RGB(141, 255, 65), RGB(252, 75, 105), RGB(189, 161, 255),
		                                   RGB(243, 159, 58), RGB(250, 255, 91), RGB(255, 173, 207) };
		for (const auto& s : stars)
			fillStar(g, overlay.X + overlay.Width * s.x, overlay.Y + overlay.Height * s.y,
			         overlay.Width * s.r * 1.4f, (REAL)s.angle, fromColorRef(colors[s.color], alpha));
	}

	m_overlay = overlay;

	if (m_model.showTimer) {
		int points = fullscreen ? m_model.timerSize : windowedTimerSize(m_model.timerSize);
		// slightly larger than true scale so that it stays readable in the thumbnail
		REAL size = (std::max)(6.0f, points * k * 1.6f);
		FontFamily family(DEFAULT_TIMER_FONT_FACE);
		Font timerFont(&family, size, FontStyleBoldItalic, UnitPixel);

		RectF textBox;
		g.MeasureString(L"09:42", 5, &timerFont, PointF(0, 0), &textBox);

		TimerOrigin origin = timerOrigin(m_model.timerPosition,
			(int)overlay.Width, (int)overlay.Height, (int)textBox.Width, (int)textBox.Height,
			(int)((fullscreen ? DEFAULT_TIMER_X : DEFAULT_TIMER_X_WINDOWED) * k),
			(int)((fullscreen ? DEFAULT_TIMER_Y : DEFAULT_TIMER_Y_WINDOWED) * k));

		SolidBrush timerBrush(fromColorRef(m_model.timerColor, alpha));
		g.DrawString(L"09:42", 5, &timerFont, PointF(overlay.X + origin.x, overlay.Y + origin.y), &timerBrush);
	}

	g.Restore(state);

	if (!fullscreen) {
		Pen frame(Color(0x60, 0, 0, 0), 1);
		g.DrawRectangle(&frame, overlay);
	}
}

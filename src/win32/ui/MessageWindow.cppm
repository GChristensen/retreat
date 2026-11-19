// The message of a scheduled break, shown over the break screen for the length
// of the break: an opaque white pill with semicircular ends, centered on the
// primary display. It never takes focus and lets clicks through.
module;

#include "wtl.h"
#include <gdiplus.h>

#include <algorithm>
#include <cmath>
#include <string>
#include "tstring.h"

export module MessageWindow;

import Settings;

// Shows the message, replacing the one already shown. When clearCenterPx is
// non-zero, a band of that height through the middle of the display is kept
// free (for a centered break timer) and the message goes below it.
export void showMessageWindow(const tstring& text, COLORREF textColor, int fontSizePt,
                              int clearCenterPx = 0);
export void closeMessageWindow();

module :private;

using namespace Gdiplus;

namespace {

using MessageWindowTraits = CWinTraits<WS_POPUP,
	WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT>;

class CMessageWindow: public CWindowImpl<CMessageWindow, CWindow, MessageWindowTraits>
{
public:
	DECLARE_WND_CLASS_EX(_T("EnsoRetreatMessageWindow"), 0, NULL)

	BEGIN_MSG_MAP(CMessageWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	END_MSG_MAP()

	// the window currently shown, if any
	static CMessageWindow* current;

	bool Render(const tstring& text, COLORREF textColor, int fontSizePt, int clearCenterPx);

	// like the break screen, it cannot be closed from outside
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
	{
		return 0;
	}

	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&)
	{
		return MA_NOACTIVATE;
	}

	virtual void OnFinalMessage(HWND) override
	{
		if (current == this)
			current = nullptr;

		delete this;
	}
};

CMessageWindow* CMessageWindow::current = nullptr;

// a rectangle with semicircles in place of its left and right sides
void addPill(GraphicsPath& path, REAL x, REAL y, REAL width, REAL height)
{
	path.AddArc(x, y, height, height, 90, 180);
	path.AddArc(x + width - height, y, height, height, 270, 180);
	path.CloseFigure();
}

bool CMessageWindow::Render(const tstring& text, COLORREF textColor, int fontSizePt, int clearCenterPx)
{
	CRect workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); // primary display

	HDC screenDC = ::GetDC(NULL);
	int ppi = GetDeviceCaps(screenDC, LOGPIXELSY);

	FontFamily family(DEFAULT_TIMER_FONT_FACE);
	Font font(&family, (REAL)MulDiv(fontSizePt, ppi, 72), FontStyleBoldItalic, UnitPixel);

	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);

	// measure the text, wrapping it to most of the display width
	Bitmap probe(1, 1, PixelFormat32bppPARGB);
	Graphics measure(&probe);
	measure.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

	REAL lineHeight = font.GetHeight(&measure);
	REAL paddingY = lineHeight * 0.35f;
	REAL maxTextWidth = (std::max)(lineHeight, workArea.Width() * 0.8f - lineHeight - 2 * paddingY);

	SizeF textSize;
	measure.MeasureString(text.c_str(), (INT)text.size(), &font,
		SizeF(maxTextWidth, (REAL)workArea.Height()), &format, &textSize);

	// the semicircles take half of the height on each side
	int height = (int)std::ceil(textSize.Height + 2 * paddingY);
	int textWidth = (int)std::ceil(textSize.Width) + 1;
	int width = textWidth + height;

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height; // top-down, as GDI+ expects
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits = nullptr;
	HBITMAP dib = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);

	if (!dib) {
		::ReleaseDC(NULL, screenDC);
		return false;
	}

	{
		Bitmap canvas(width, height, width * 4, PixelFormat32bppPARGB, (BYTE*)bits);
		Graphics g(&canvas);
		g.SetSmoothingMode(SmoothingModeAntiAlias);
		g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
		g.Clear(Color(0, 0, 0, 0));

		GraphicsPath pill;
		addPill(pill, 0, 0, (REAL)width, (REAL)height);
		SolidBrush background(Color(255, 255, 255, 255));
		g.FillPath(&background, &pill);

		// a hairline outline keeps the pill visible over light backgrounds
		GraphicsPath outline;
		addPill(outline, 0.5f, 0.5f, width - 1.0f, height - 1.0f);
		Pen border(Color(255, 210, 210, 210), 1.0f);
		g.DrawPath(&border, &outline);

		SolidBrush textBrush(Color(255, GetRValue(textColor), GetGValue(textColor), GetBValue(textColor)));
		RectF layout(height / 2.0f, 0, (REAL)textWidth, (REAL)height);
		g.DrawString(text.c_str(), (INT)text.size(), &font, layout, &format, &textBrush);
	}

	HDC memDC = CreateCompatibleDC(screenDC);
	HGDIOBJ oldBitmap = SelectObject(memDC, dib);

	int centerY = workArea.top + workArea.Height() / 2;
	int top = clearCenterPx > 0
		? centerY + clearCenterPx / 2 + (int)paddingY
		: centerY - height / 2;

	POINT position = { workArea.left + (workArea.Width() - width) / 2, top };
	SIZE size = { width, height };
	POINT origin = { 0, 0 };
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

	BOOL updated = ::UpdateLayeredWindow(m_hWnd, screenDC, &position, &size, memDC, &origin,
		0, &blend, ULW_ALPHA);

	SelectObject(memDC, oldBitmap);
	DeleteDC(memDC);
	DeleteObject(dib);
	::ReleaseDC(NULL, screenDC);

	return updated != FALSE;
}

} // namespace

void showMessageWindow(const tstring& text, COLORREF textColor, int fontSizePt, int clearCenterPx)
{
	closeMessageWindow();

	auto* window = new CMessageWindow();

	CRect initialRect(0, 0, 1, 1);

	if (!window->Create(NULL, initialRect)) {
		delete window;
		return;
	}

	CMessageWindow::current = window;

	if (!window->Render(text, textColor, fontSizePt, clearCenterPx)) {
		window->DestroyWindow(); // deletes itself
		return;
	}

	window->ShowWindow(SW_SHOWNOACTIVATE);

	// above the break screen, which has just taken the foreground
	window->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void closeMessageWindow()
{
	if (CMessageWindow::current)
		CMessageWindow::current->DestroyWindow(); // deletes itself
}

// A quote with a picture, shown over the break screen for the length of the
// break: the card of QuoteCard.cppm, centered on the primary display. Like the
// message window, it never takes focus and lets clicks through.
module;

#include "wtl.h"
#include <objidl.h>
#include <gdiplus.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include "tstring.h"

export module QuoteWindow;

import Settings;
import SettingsModel;
import QuoteCard;

// Picks the next quote if quotes are enabled and lays out its card, centered
// on the primary display, replacing the quote already shown; the window stays
// hidden until showPreparedQuoteWindow(). `advance` remembers the quote, so the
// next break shows another one; the caller saves the settings. When
// timerAbovePx is non-zero, a band of that height is kept free above the card
// (for a centered break timer, placed there by the caller).
// Returns the card in screen coordinates, or an empty rectangle if none.
export CRect prepareQuoteWindow(Settings& settings, bool advance, int timerAbovePx = 0);

// Shows the prepared quote above the break screen.
export void showPreparedQuoteWindow();

export void closeQuoteWindow();

// Makes the shown quote more or less opaque (numpad +/- on the break screen,
// to peek at the desktop). Not saved: every quote is shown fully opaque.
export void alterQuoteWindowAlpha(int delta);

// The quote look from the settings, at full-screen size for the given display.
export QuoteLook quoteLookFromSettings(Settings& settings, int screenW, int screenH, int ppi);

module :private;

using namespace Gdiplus;

namespace {

using QuoteWindowTraits = CWinTraits<WS_POPUP,
	WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT>;

class CQuoteWindow: public CWindowImpl<CQuoteWindow, CWindow, QuoteWindowTraits>
{
public:
	DECLARE_WND_CLASS_EX(_T("EnsoRetreatQuoteWindow"), 0, NULL)

	BEGIN_MSG_MAP(CQuoteWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	END_MSG_MAP()

	// the window currently shown, if any
	static CQuoteWindow* current;

	// returns the card in screen coordinates, or an empty rectangle on failure
	CRect Render(const Quote& quote, QuoteLook look, int timerAbovePx);

	// changes the opacity of the whole window, keeping its content
	void AlterAlpha(int delta);

	BYTE alpha = 255;

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

CQuoteWindow* CQuoteWindow::current = nullptr;

CRect CQuoteWindow::Render(const Quote& quote, QuoteLook look, int timerAbovePx)
{
	CRect workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); // primary display

	int margin = (int)(24 * look.unit);
	// a centered card leaves as much room below it as the timer takes above
	int available = workArea.Height() - 2 * margin - 2 * (std::max)(0, timerAbovePx);

	SizeF size = measureQuoteCard(quote, look);

	// a long quote or a short display: make the picture smaller, then the card
	if (size.Height > available && look.imageHeight > 0) {
		look.imageHeight = (std::max)(0.0f, look.imageHeight - (size.Height - available));
		if (look.imageHeight < 48 * look.unit)
			look.imageHeight = 0;
		size = measureQuoteCard(quote, look);
	}

	if (size.Width < 1 || size.Height < 1)
		return CRect();

	int width = (int)std::ceil(size.Width);
	int height = (int)std::ceil(size.Height);

	HDC screenDC = ::GetDC(NULL);

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
		return CRect();
	}

	{
		Bitmap canvas(width, height, width * 4, PixelFormat32bppPARGB, (BYTE*)bits);
		Graphics g(&canvas);
		g.Clear(Color(0, 0, 0, 0));
		drawQuoteCard(g, PointF(0, 0), quote, look);
	}

	HDC memDC = CreateCompatibleDC(screenDC);
	HGDIOBJ oldBitmap = SelectObject(memDC, dib);

	int centerY = workArea.top + workArea.Height() / 2;
	int top = centerY - height / 2;
	top = (std::max)((int)workArea.top, (std::min)(top, (int)workArea.bottom - height));

	POINT position = { workArea.left + (workArea.Width() - width) / 2, top };
	SIZE windowSize = { width, height };
	POINT origin = { 0, 0 };
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };

	BOOL updated = ::UpdateLayeredWindow(m_hWnd, screenDC, &position, &windowSize, memDC, &origin,
		0, &blend, ULW_ALPHA);

	SelectObject(memDC, oldBitmap);
	DeleteDC(memDC);
	DeleteObject(dib);
	::ReleaseDC(NULL, screenDC);

	return updated ? CRect(position, windowSize) : CRect();
}

void CQuoteWindow::AlterAlpha(int delta)
{
	alpha = (BYTE)std::clamp((int)alpha + delta, 0, 255);

	// without a source DC only the blend changes; the bitmap stays
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
	::UpdateLayeredWindow(m_hWnd, NULL, NULL, NULL, NULL, NULL, 0, &blend, ULW_ALPHA);
}

} // namespace

QuoteLook quoteLookFromSettings(Settings& settings, int screenW, int screenH, int ppi)
{
	QuoteLook look;
	look.face = settings.getString(Settings::QUOTES_FONT_FACE, DEFAULT_QUOTES_FONT_FACE);
	look.italic = settings.getBoolean(Settings::QUOTES_FONT_ITALIC, Settings::DEFAULT_QUOTES_FONT_ITALIC);
	look.color = (COLORREF)settings.getInt(Settings::QUOTES_TEXT_COLOR, Settings::DEFAULT_QUOTES_TEXT_COLOR);
	look.image = toQuoteImage(settings.getInt(Settings::QUOTES_IMAGE, Settings::DEFAULT_QUOTES_IMAGE));

	int sizePt = std::clamp(settings.getInt(Settings::QUOTES_FONT_SIZE, Settings::DEFAULT_QUOTES_FONT_SIZE),
		MIN_QUOTE_SIZE, MAX_QUOTE_SIZE);

	look.unit = ppi / 96.0f;
	look.fontPx = sizePt * ppi / 72.0f;
	look.maxTextWidth = (std::min)(screenW * 0.6f, 900 * look.unit);
	look.imageHeight = (std::min)(screenH * 0.3f, 300 * look.unit);
	return look;
}

CRect prepareQuoteWindow(Settings& settings, bool advance, int timerAbovePx)
{
	closeQuoteWindow();

	if (!settings.getBoolean(Settings::QUOTES_ENABLE, Settings::DEFAULT_QUOTES_ENABLE))
		return CRect();

	std::vector<tstring> lines = loadQuoteLines(settings);

	Quote quote;
	if (!lines.empty()) {
		int last = settings.getInt(Settings::QUOTES_LAST_INDEX, -1);
		bool sequential = settings.getBoolean(Settings::QUOTES_SEQUENTIAL, Settings::DEFAULT_QUOTES_SEQUENTIAL);
		int index = nextQuoteIndex((int)lines.size(), last, sequential, (unsigned)std::rand());

		quote = parseQuote(lines[index]);

		if (advance)
			settings.setInt(Settings::QUOTES_LAST_INDEX, index);
	}

	CRect workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	HDC screenDC = ::GetDC(NULL);
	int ppi = GetDeviceCaps(screenDC, LOGPIXELSY);
	::ReleaseDC(NULL, screenDC);

	QuoteLook look = quoteLookFromSettings(settings, workArea.Width(), workArea.Height(), ppi);

	std::unique_ptr<Bitmap> thinkerImage;
	if (look.image == QuoteImage::Thinker) {
		thinkerImage = loadThinkerImage();
		look.thinkerImage = thinkerImage.get();
	}

	std::unique_ptr<Bitmap> customImage;
	if (look.image == QuoteImage::Custom) {
		tstring file = settings.getString(Settings::QUOTES_IMAGE_FILE, _T(""));
		if (!file.empty()) {
			customImage.reset(Bitmap::FromFile(file.c_str()));
			if (customImage && customImage->GetLastStatus() != Ok)
				customImage.reset();
		}
		look.customImage = customImage.get();
	}

	auto* window = new CQuoteWindow();

	CRect initialRect(0, 0, 1, 1);

	if (!window->Create(NULL, initialRect)) {
		delete window;
		return CRect();
	}

	CQuoteWindow::current = window;

	// positions the hidden window
	CRect card = window->Render(quote, look, timerAbovePx);

	if (card.IsRectEmpty()) {
		window->DestroyWindow(); // deletes itself
		return CRect();
	}

	return card;
}

void showPreparedQuoteWindow()
{
	CQuoteWindow* window = CQuoteWindow::current;

	if (!window)
		return;

	window->ShowWindow(SW_SHOWNOACTIVATE);

	// above the break screen, which has just taken the foreground
	window->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void closeQuoteWindow()
{
	if (CQuoteWindow::current)
		CQuoteWindow::current->DestroyWindow(); // deletes itself
}

void alterQuoteWindowAlpha(int delta)
{
	if (CQuoteWindow::current)
		CQuoteWindow::current->AlterAlpha(delta);
}

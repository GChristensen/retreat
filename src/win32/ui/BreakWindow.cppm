// Builds the break-screen windows (one per monitor in full-screen mode) from
// the settings. Used by the lock state and by the settings dialog's
// "Try it" preview.
module;

#include "wtl.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "tstring.h"

export module BreakWindow;

import system;
import Settings;
import TimerWindow;
import SettingsModel;

export using BreakWindowPtr = std::shared_ptr<CTimerWindow>;

// Image folder that the break screen will use, or an empty string for the
// default random stars.
export tstring getBreakImageDirectory(Settings& settings);

// True when quotes replace the break picture with a plain dark backdrop.
export bool usePlainQuoteBackdrop(Settings& settings);

// Height of the band through the middle of the primary display that a
// centered break timer needs, or 0 when the timer is elsewhere or hidden.
export int centerTimerClearancePx(Settings& settings, bool fullscreen);

// Creates and shows the break windows. `fullscreen` is updated when the
// windowed mode falls back to full screen (no image to show).
export std::vector<BreakWindowPtr> createBreakWindows(Settings& settings, int durationSec,
                                                      bool grabInput, bool& fullscreen);

module :private;

namespace {

const COLORREF PLAIN_BACKDROP_COLOR = RGB(0x10, 0x13, 0x18);

struct BuildContext {
	Settings& settings;
	int durationSec;
	bool grabInput;
	bool fullscreen;
	std::vector<BreakWindowPtr> windows;
};

BreakWindowPtr createTimerWindow(BuildContext& ctx, CRect* pRect, bool primary)
{
	Settings& settings = ctx.settings;

	CRect workArea, *pWorkArea = nullptr;

	if (!ctx.fullscreen) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		pWorkArea = &workArea;
	}

	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, pRect, pWorkArea);

	bool transparent = settings.getBoolean(Settings::APPEARANCE_TRANSPARENT,
		Settings::DEFAULT_APPEARANCE_TRANSPARENT);

	bool plainBackdrop = usePlainQuoteBackdrop(settings);
	tstring imageDir = plainBackdrop ? tstring() : getBreakImageDirectory(settings);
	bool backgroundLoaded = false;

	if (!imageDir.empty()) {
		tstring backgroundImagePath = getRandomFile(imageDir,
			std::vector<tstring>{ _T("jp*"), _T("bmp"), _T("gif"), _T("png") });

		if (!backgroundImagePath.empty())
			backgroundLoaded = pTimerWnd->LoadBackground(
				backgroundImagePath.c_str(),
				settings.getBoolean(Settings::APPEARANCE_STRETCH_IMAGES,
					Settings::DEFAULT_APPEARANCE_STRETCH_IMAGES),
				settings.getInt(Settings::APPEARANCE_BACKGROUND_COLOR,
					Settings::DEFAULT_APPEARANCE_BACKGROUND_COLOR)
			);
	}

	if (backgroundLoaded) {
		if (transparent)
			pTimerWnd->SetTransparentColorFromImage();
	}
	else {
		if (!ctx.fullscreen) {
			ctx.fullscreen = true;
			pTimerWnd->Close();
			pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, pRect, nullptr);
		}

		if (plainBackdrop)
			pTimerWnd->PlainBackground(PLAIN_BACKDROP_COLOR);
		else
			pTimerWnd->DefaultBackground();
	}

	pTimerWnd->SetAlpha(settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL,
		Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL));

	bool showTimer = settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER,
		Settings::DEFAULT_APPEARANCE_SHOW_TIMER);

	if (primary && showTimer) {
		// no color key on a plain backdrop, so the text may be smoothed
		bool antialias = (backgroundLoaded && !transparent) || plainBackdrop;

		COLORREF timerTextColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR,
			Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

		int timerSize = std::clamp(settings.getInt(Settings::APPEARANCE_TIMER_SIZE,
			Settings::DEFAULT_APPEARANCE_TIMER_SIZE), MIN_TIMER_SIZE, MAX_TIMER_SIZE);

		pTimerWnd->SetTimerProperties(
			DEFAULT_TIMER_FONT_FACE,
			ctx.fullscreen ? timerSize : windowedTimerSize(timerSize),
			DEFAULT_CHARSET,
			timerTextColor,
			antialias,
			true,
			true
		);

		pTimerWnd->SetTimerAnchor(
			toTimerPosition(settings.getInt(Settings::APPEARANCE_TIMER_POSITION,
				Settings::DEFAULT_APPEARANCE_TIMER_POSITION)),
			ctx.fullscreen ? DEFAULT_TIMER_X : DEFAULT_TIMER_X_WINDOWED,
			ctx.fullscreen ? DEFAULT_TIMER_Y : DEFAULT_TIMER_Y_WINDOWED);
		pTimerWnd->SetTimer(ctx.durationSec - 1);
	}
	else
		pTimerWnd->SetShowTimer(false);

	pTimerWnd->CenterWindowOnWorkArea();
	pTimerWnd->ShowWindow(SW_SHOW);

	if (ctx.grabInput)
		pTimerWnd->GrabUserInput();

	// we should call this after window has been shown to reduce blinking
	pTimerWnd->ApplyLayeredWindowAttributes();

	return pTimerWnd;
}

BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC, LPRECT lprcMonitor, LPARAM dwData)
{
	BuildContext* ctx = (BuildContext*)dwData;

	MONITORINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);

	if (GetMonitorInfo(hMonitor, &info)) {
		bool primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;

		if (!ctx->fullscreen && !primary)
			return TRUE;

		CRect rect(*lprcMonitor);
		ctx->windows.push_back(createTimerWindow(*ctx, &rect, primary));
	}

	return TRUE;
}

} // namespace

tstring getBreakImageDirectory(Settings& settings)
{
	tstring imageDir = settings.getString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T(""));

	if (!settings.getBoolean(Settings::APPEARANCE_USE_IMAGES, !imageDir.empty()))
		return tstring();

	return imageDir;
}

bool usePlainQuoteBackdrop(Settings& settings)
{
	return settings.getBoolean(Settings::QUOTES_ENABLE, Settings::DEFAULT_QUOTES_ENABLE)
		&& settings.getBoolean(Settings::QUOTES_PLAIN_BACKDROP, Settings::DEFAULT_QUOTES_PLAIN_BACKDROP);
}

int centerTimerClearancePx(Settings& settings, bool fullscreen)
{
	bool showTimer = settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER,
		Settings::DEFAULT_APPEARANCE_SHOW_TIMER);
	TimerPosition position = toTimerPosition(settings.getInt(Settings::APPEARANCE_TIMER_POSITION,
		Settings::DEFAULT_APPEARANCE_TIMER_POSITION));

	if (!showTimer || position != TimerPosition::Center)
		return 0;

	int timerSize = std::clamp(settings.getInt(Settings::APPEARANCE_TIMER_SIZE,
		Settings::DEFAULT_APPEARANCE_TIMER_SIZE), MIN_TIMER_SIZE, MAX_TIMER_SIZE);
	int fontSize = fullscreen ? timerSize : windowedTimerSize(timerSize);

	CClientDC screenDC(NULL);
	return MulDiv(fontSize, screenDC.GetDeviceCaps(LOGPIXELSY), 72) * 6 / 5;
}

std::vector<BreakWindowPtr> createBreakWindows(Settings& settings, int durationSec, bool grabInput, bool& fullscreen)
{
	BuildContext ctx{ settings, durationSec, grabInput, fullscreen, {} };

	EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)&ctx);

	fullscreen = ctx.fullscreen;
	return std::move(ctx.windows);
}

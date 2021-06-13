module;

#include "stdafx.h"

export module LockWindowAdapter;

import <memory>;
import <vector>;

#include "debug.h"

import Settings;
import TimerWindow;
import StateWindowAdapter;

using WindowPtr = std::shared_ptr<CTimerWindow>;

export class LockWindowAdapter: public StateWindowAdapter {
public:
    LockWindowAdapter(Settings& settings);
    virtual void onTimer() override;
	virtual ~LockWindowAdapter();

private:
	bool fullscreen;
	int breakDurationSec;
	std::vector<WindowPtr> windows;

	auto createTimerWindow(CRect* rect, bool primary);
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
};

module :private;

LockWindowAdapter::LockWindowAdapter(Settings& settings) {
	fullscreen = settings.getBoolean(Settings::APPEARANCE_FULLSCREEN, Settings::DEFAULT_APPEARANCE_FULLSCREEN);
	breakDurationSec = settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);

    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
}

LockWindowAdapter::~LockWindowAdapter() {
	for (const WindowPtr &window : windows)
		window->SendMessage(WM_CLOSE);
}

void LockWindowAdapter::onTimer() {
	for (const WindowPtr &window : windows)
		window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);
}

auto LockWindowAdapter::createTimerWindow(CRect* pRect, bool primary) {

//	int mode = appearance(machine).Mode;
//	m_hideCursor = mode == AM_FULLSCREEN_MODE;

	CRect workArea, *pWorkArea = nullptr;

	if (!fullscreen) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		pWorkArea = &workArea;
	}

	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, pRect, pWorkArea);

//	tstring imageDir = appearance(machine).ImageFolder[mode];
//	tstring backgroundImagePath;
//
//	if (!imageDir.empty())
//	{
//		SystemHelper::getInstance()->
//			getArbitraryFilePath(imageDir, _T("jp*"), _T("bmp"), _T("gif"), backgroundImagePath);
//	}
//
//	bool backgroundLoaded =
//		pAngelicWnd->LoadBackground(
//			backgroundImagePath.c_str(),
//			appearance(machine).StretchImage,
//			appearance(machine).BackgroundColor
//		);
//
//	if (backgroundLoaded)
//	{
//		if (appearance(machine).UseTransparency)
//		{
//			COLORREF transparent = appearance(machine).TransparentColor;
//
//			if (transparent == AM_DEFAULT_TRANSPARENT_COLOR)
//			{
//				pAngelicWnd->SetTransparentColorFromImageLeftUpperPixel();
//			}
//			else
//			{
//				pAngelicWnd->SetTransparentColor(transparent);
//			}
//		}
//	}
//	else
//	{
//		if (mode != AM_FULLSCREEN_MODE)
//		{
//			mode = AM_FULLSCREEN_MODE;
//			pAngelicWnd->SendMessage(WM_CLOSE);
//			pAngelicWnd = angelic_wnd_ptr_t(new CAngelicWnd(0, NULL, NULL));
//		}
//
//		pAngelicWnd->SpecialBackground(CAngelicWnd::STARS);
//		m_hideCursor = false;
//	}
//
//	pAngelicWnd->SetAlpha(appearance(machine).TransparencyLevel);
//
//	if (primary && appearance(machine).ShowTimer)
//	{
//		AM_APPEARANCE_SETTINGS::TEXT_SETTINGS& text =
//			appearance(machine).TimerFontSettings[mode];
//
//		bool antialias =
//			backgroundLoaded && !appearance(machine).UseTransparency;
//
//		int breakLengthMin =
//			boost::any_cast<int>(getCell(machine, BREAK_DURATION_CELL));
//
//		pAngelicWnd->SetTimerProperties(
//			text.Face.c_str(),
//			DEFAULT_CHARSET,
//			text.Size,
//			text.Color,
//			antialias,
//			text.Bold,
//			text.Italic
//		);
//
//		pAngelicWnd->SetTimerPos(text.x, text.y);
//		pAngelicWnd->SetTimer(breakLengthMin * 60);
//	}
//	else
//	{
//		pAngelicWnd->SetShowTimer(false);
//	}
//
//#ifndef _DEBUG
//	if (!m_locked)
//	{
//		SystemHelper::getInstance()->lockUserInput(
//			getParentWindow(machine),
//			behaviour(machine).MagicWords,
//			ID_USER_UNLOCK_COMMAND
//		);
//
//		if (m_hideCursor)
//		{
//			ShowCursor(FALSE);
//		}
//
//		m_locked = true;
//	}
//#endif
//

	pTimerWnd->DefaultBackground();

	pTimerWnd->SetTimerProperties(
		_T("Arial"),
		DEFAULT_CHARSET,
		32,
		RGB(239, 27, 27),
		false,
		true,
		true
	);

	
	pTimerWnd->SetTimerPos(160, 140);
	pTimerWnd->SetTimer(breakDurationSec);
	pTimerWnd->SetAlpha(128);

	pTimerWnd->CenterWindowOnWorkArea();
	pTimerWnd->ShowWindow(SW_SHOW);

	if (primary)
		pTimerWnd->GrabUserInput();

	// we should call this after window has been shown to reduce blinking
	pTimerWnd->ApplyLayeredWindowAttributes();

	//pTimerWnd->DoEvents();

	return pTimerWnd;
}

BOOL CALLBACK LockWindowAdapter::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	LockWindowAdapter* adapter = (LockWindowAdapter *)dwData;

	MONITORINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);

	if (GetMonitorInfo(hMonitor, &info)) {
		bool primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
		
		if (!adapter->fullscreen && !primary)
			return TRUE;

		CRect rect(*lprcMonitor);
		adapter->windows.push_back(adapter->createTimerWindow(&rect, primary));
	}

	return TRUE;
}
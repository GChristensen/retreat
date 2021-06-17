module;

#include "stdafx.h"

export module LockWindowAdapter;

import <string>;
import <memory>;
import <vector>;

#include "debug.h"
#include "tstring.h"

import system;
import Settings;
import InputLock;
import TimerWindow;
import StateMachine;
import StateWindowAdapter;

using WindowPtr = std::shared_ptr<CTimerWindow>;

export class LockWindowAdapter: public StateWindowAdapter {
public:
    LockWindowAdapter(StateMachine& stateMachine);
    virtual void onTimer() override;
	virtual ~LockWindowAdapter();

private:
	bool fullscreen;
	Settings &settings;
	InputLock inputLock;
	bool settingsChanged = false;

	int breakDurationSec;

	std::vector<WindowPtr> windows;

	auto createTimerWindow(CRect* rect, bool primary);
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
		LPRECT lprcMonitor, LPARAM dwData);
};

module :private;

LockWindowAdapter::LockWindowAdapter(StateMachine& stateMachine): 
	settings(stateMachine.getSettings()),
	inputLock(stateMachine) {

	breakDurationSec = stateMachine.getBreakDuration();

	if (stateMachine.getBreakDurationOverride())
		breakDurationSec = stateMachine.getBreakDurationOverride();

	fullscreen = settings.getBoolean(Settings::APPEARANCE_FULLSCREEN, 
		Settings::DEFAULT_APPEARANCE_FULLSCREEN);

	inputLock.setFullScreenMode(fullscreen);

	inputLock.onNumpadPlus = [this]() {
		for (const WindowPtr& window: this->windows)
			window->AlterAlpha(10);

		if (!this->windows.empty())
			this->settings.setInt(Settings::APPEARANCE_OPACITY_LEVEL, 
				this->windows[0]->GetAlpha());

		this->settingsChanged = true;
	};

	inputLock.onNumpadMinus = [this]() {
		for (const WindowPtr& window : this->windows)
			window->AlterAlpha(-10);

		if (!this->windows.empty())
			this->settings.setInt(Settings::APPEARANCE_OPACITY_LEVEL,
				this->windows[0]->GetAlpha());

		this->settingsChanged = true;
	};

	inputLock.lock();

    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
}

LockWindowAdapter::~LockWindowAdapter() {
	if (settingsChanged)
		settings.save();

	for (const WindowPtr &window : windows)
		window->SendMessage(WM_CLOSE);

	if (settings.getBoolean(Settings::SOUNDS_ENABLE, Settings::DEFAULT_SOUNDS_ENABLE)) {
		tstring audioDir = settings.getString(Settings::SOUNDS_AUDIO_DIRECTORY, _T(""));

		if (!audioDir.empty()) {
			tstring audioFilePath = getRandomFile(audioDir,
				std::vector<tstring>{ _T("wav"), _T("mp3"), _T("wma"), _T("flac") });

			if (!audioFilePath.empty())
				playAudioFile(audioFilePath);
			else
				playSystemSound();
		}
		else
			playSystemSound();
	}
}

void LockWindowAdapter::onTimer() {
	for (const WindowPtr &window : windows)
		window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);
}

auto LockWindowAdapter::createTimerWindow(CRect* pRect, bool primary) {

	CRect workArea, *pWorkArea = nullptr;

	if (!fullscreen) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		pWorkArea = &workArea;
	}

	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, pRect, pWorkArea);

	bool transparent = settings.getBoolean(Settings::APPEARANCE_TRANSPARENT, 
		Settings::DEFAULT_APPEARANCE_TRANSPARENT);

	tstring imageDir = settings.getString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T(""));
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
		if (!fullscreen) {
			fullscreen = true;
			pTimerWnd->SendMessage(WM_CLOSE);
			pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, pRect, nullptr);
		}

		pTimerWnd->DefaultBackground();
	}

	pTimerWnd->SetAlpha(settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL, 
		Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL));

	bool showTimer = settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER, 
		Settings::DEFAULT_APPEARANCE_SHOW_TIMER);

	if (primary && showTimer) {
		bool antialias = backgroundLoaded && !transparent;

		COLORREF timerTextColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, 
			Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);
		
		pTimerWnd->SetTimerProperties(
			DEFAULT_TIMER_FONT_FACE,
			fullscreen? DEFAULT_TIMER_FONT_SIZE: DEFAULT_TIMER_FONT_SIZE - 10,
			DEFAULT_CHARSET,
			timerTextColor,
			antialias,
			true,
			true
		);
		
		pTimerWnd->SetTimerPos(fullscreen? DEFAULT_TIMER_X: DEFAULT_TIMER_X_WINDOWED, 
			fullscreen? DEFAULT_TIMER_Y: DEFAULT_TIMER_Y_WINDOWED);
		pTimerWnd->SetTimer(breakDurationSec);
	}
	else
		pTimerWnd->SetShowTimer(false);

	pTimerWnd->CenterWindowOnWorkArea();
	pTimerWnd->ShowWindow(SW_SHOW);

	pTimerWnd->GrabUserInput();

	// we should call this after window has been shown to reduce blinking
	pTimerWnd->ApplyLayeredWindowAttributes();

	return pTimerWnd;
}

BOOL CALLBACK LockWindowAdapter::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
		LPRECT lprcMonitor, LPARAM dwData) {

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
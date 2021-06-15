module;

#include "stdafx.h"

export module AlertWindowAdapter;

import <memory>;
import <vector>;
import <map>;

#include "debug.h"

import Settings;
import TimerWindow;
import StateMachine;
import StateWindowAdapter;

using WindowPtr = std::shared_ptr<CTimerWindow>;

export class AlertWindowAdapter: public StateWindowAdapter {
public:
    AlertWindowAdapter(StateMachine &stateMachine);
    virtual void onTimer() override;
	virtual ~AlertWindowAdapter();

private:
	int alertDurationSec;
	Settings &settings;

	std::vector<WindowPtr> windows;

	bool isThereForbiddenProcesses();

	auto createTimerWindow(CRect* rect);
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
		LPRECT lprcMonitor, LPARAM dwData);
};

module :private;

import system;

AlertWindowAdapter::AlertWindowAdapter(StateMachine &stateMachine): 
	settings(stateMachine.getSettings()) {

	bool checkForProcesses = settings.getBoolean(Settings::MONITORING_PROCESSES,
		Settings::DEFAULT_MONITORING_PROCESSES);

	if (checkForProcesses && isThereForbiddenProcesses())
		throw StateForbiddenException();

	alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, 
		Settings::DEFAULT_ALERT_DURATION);

    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
}

AlertWindowAdapter::~AlertWindowAdapter() {
	for (const WindowPtr &window : windows)
		window->SendMessage(WM_CLOSE);
}

bool AlertWindowAdapter::isThereForbiddenProcesses() {
	std::vector<tstring> forbiddenProcesses;
	settings.getSectionValues(Settings::PROCESSES, forbiddenProcesses);

	return isProcessRunning(forbiddenProcesses);
}

void AlertWindowAdapter::onTimer() {
	for (const WindowPtr &window : windows)
		window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);
}

auto AlertWindowAdapter::createTimerWindow(CRect* pRect) {
	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, nullptr, pRect);

	COLORREF timerTextColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, 
		Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

	pTimerWnd->SetTimerProperties(
		DEFAULT_TIMER_FONT_FACE,
		DEFAULT_TIMER_FONT_SIZE,
		DEFAULT_CHARSET,
		timerTextColor,
		false,
		true,
		true
	);

	pTimerWnd->SetTimer(alertDurationSec, true);
	pTimerWnd->PlaceWindowOnWorkArea(DEFAULT_TIMER_X, DEFAULT_TIMER_Y);
	pTimerWnd->SetAlpha(settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL, 
		Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL));

	pTimerWnd->ApplyLayeredWindowAttributes();
	pTimerWnd->ShowWindow(SW_SHOWNOACTIVATE);

	return pTimerWnd;
}

BOOL CALLBACK AlertWindowAdapter::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
		LPRECT lprcMonitor, LPARAM dwData) {

	AlertWindowAdapter* adapter = (AlertWindowAdapter *)dwData;

	CRect rect(*lprcMonitor);

	adapter->windows.push_back(adapter->createTimerWindow(&rect));

	return TRUE;
}
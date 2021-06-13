module;

#include "stdafx.h"

export module AlertWindowAdapter;

import <memory>;
import <vector>;

import Settings;
import TimerWindow;
import StateWindowAdapter;

using WindowPtr = std::shared_ptr<CTimerWindow>;

export class AlertWindowAdapter: public StateWindowAdapter {
public:
    AlertWindowAdapter(Settings& settings);
    virtual void onTimer() override;
	virtual ~AlertWindowAdapter();

private:
	int alertDurationSec;
	std::vector<WindowPtr> windows;

	auto createTimerWindow(CRect* rect);
	static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
};

module :private;

AlertWindowAdapter::AlertWindowAdapter(Settings& settings) {
	alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);

    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)this);
}

AlertWindowAdapter::~AlertWindowAdapter() {
	for (const WindowPtr &window : windows)
		window->SendMessage(WM_CLOSE);
}

void AlertWindowAdapter::onTimer() {
	for (const WindowPtr &window : windows)
		window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);
}

auto AlertWindowAdapter::createTimerWindow(CRect* pRect) {
	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, nullptr, pRect);

	pTimerWnd->SetTimerProperties(
		_T("Arial"),
		DEFAULT_CHARSET,
		32,
		RGB(239, 27, 27),
		false,
		true,
		true
	);

	pTimerWnd->SetTimer(alertDurationSec, true);
	pTimerWnd->PlaceWindowOnWorkArea(160, 140);
	pTimerWnd->SetAlpha(128);

	pTimerWnd->ApplyLayeredWindowAttributes();
	pTimerWnd->ShowWindow(SW_SHOWNOACTIVATE);

	return pTimerWnd;
}

BOOL CALLBACK AlertWindowAdapter::monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	AlertWindowAdapter* adapter = (AlertWindowAdapter *)dwData;

	CRect rect(*lprcMonitor);

	adapter->windows.push_back(adapter->createTimerWindow(&rect));

	return TRUE;
}
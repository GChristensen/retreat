module;

#include "wtl.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include "debug.h"

export module AlertWindowAdapter;



import Settings;
import SettingsModel;
import TimerWindow;
import StateMachine;
import StateWindowAdapter;

using WindowPtr = std::shared_ptr<CTimerWindow>;

// Creates and shows the pre-break warning (one countdown per monitor). Used by
// the alert state and by the settings dialog's "Try alert" preview.
export std::vector<WindowPtr> createAlertWindows(Settings& settings, int durationSec);

export class AlertWindowAdapter: public StateWindowAdapter {
public:
    AlertWindowAdapter(StateMachine &stateMachine);
    virtual void onTimer() override;
	virtual ~AlertWindowAdapter();

private:
	Settings &settings;

	std::vector<WindowPtr> windows;

	bool isThereForbiddenProcesses();
};

module :private;

import system;

namespace {

struct AlertContext {
	Settings& settings;
	int durationSec;
	std::vector<WindowPtr> windows;
};

WindowPtr createAlertWindow(AlertContext& ctx, CRect* pRect) {
	Settings& settings = ctx.settings;

	// the warning must never take focus away from what the user is doing
	auto pTimerWnd = std::make_shared<CTimerWindow>((HWND)0, nullptr, pRect, WS_EX_NOACTIVATE);

	COLORREF timerTextColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR,
		Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

	// the same size as the full-screen break timer; the star scales with the text
	int timerSize = std::clamp(settings.getInt(Settings::APPEARANCE_TIMER_SIZE,
		Settings::DEFAULT_APPEARANCE_TIMER_SIZE), MIN_TIMER_SIZE, MAX_TIMER_SIZE);

	pTimerWnd->SetTimerProperties(
		DEFAULT_TIMER_FONT_FACE,
		timerSize,
		DEFAULT_CHARSET,
		timerTextColor,
		false,
		true,
		true
	);

	pTimerWnd->SetTimer(ctx.durationSec - 1, true);
	// where the full-screen break timer will appear
	pTimerWnd->AnchorWindowOnWorkArea(
		toTimerPosition(settings.getInt(Settings::APPEARANCE_TIMER_POSITION,
			Settings::DEFAULT_APPEARANCE_TIMER_POSITION)),
		DEFAULT_TIMER_X, DEFAULT_TIMER_Y);
	pTimerWnd->SetAlpha(settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL,
		Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL));

	pTimerWnd->ApplyLayeredWindowAttributes();
	pTimerWnd->ShowWindow(SW_SHOWNOACTIVATE);

	return pTimerWnd;
}

BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	AlertContext* ctx = (AlertContext*)dwData;

	// the work area keeps the bottom positions clear of the taskbar
	MONITORINFO info = { sizeof(info) };
	CRect rect(GetMonitorInfo(hMonitor, &info) ? info.rcWork : *lprcMonitor);

	ctx->windows.push_back(createAlertWindow(*ctx, &rect));

	return TRUE;
}

}

std::vector<WindowPtr> createAlertWindows(Settings& settings, int durationSec) {
	AlertContext ctx{ settings, durationSec, {} };

	EnumDisplayMonitors(NULL, NULL, monitorEnumProc, (LPARAM)&ctx);

	return std::move(ctx.windows);
}

AlertWindowAdapter::AlertWindowAdapter(StateMachine &stateMachine):
	settings(stateMachine.getSettings()) {

	bool checkForProcesses = settings.getBoolean(Settings::MONITORING_PROCESSES,
		Settings::DEFAULT_MONITORING_PROCESSES);

	if (checkForProcesses && isThereForbiddenProcesses())
		throw StateForbiddenException();

	int alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION,
		Settings::DEFAULT_ALERT_DURATION);

	windows = createAlertWindows(settings, alertDurationSec);
}

AlertWindowAdapter::~AlertWindowAdapter() {
	for (const WindowPtr &window : windows)
		window->Close();
}

bool AlertWindowAdapter::isThereForbiddenProcesses() {
	auto forbiddenProcesses = settings.getSectionValues(Settings::PROCESSES);

	return isProcessRunning(forbiddenProcesses);
}

void AlertWindowAdapter::onTimer() {
	for (const WindowPtr &window : windows)
		window->PostMessage(CTimerWindow::WM_TIMER_WND_NOTIFY);
}

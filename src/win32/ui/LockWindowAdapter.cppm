module;

#include "wtl.h"

#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include "debug.h"
#include "tstring.h"

export module LockWindowAdapter;



import system;
import Settings;
import InputLock;
import TimerWindow;
import BreakWindow;
import StateLocked;
import MessageWindow;
import SettingsModel;
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

	void showBreakMessage();
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

	windows = createBreakWindows(settings, breakDurationSec, true, fullscreen);

	showBreakMessage();
}

// the message of a scheduled break, drawn over the break screen like the timer
void LockWindowAdapter::showBreakMessage() {
	const tstring& message = StateLocked::getMessageText();

	if (message.empty())
		return;

	COLORREF textColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR,
		Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

	int timerSize = std::clamp(settings.getInt(Settings::APPEARANCE_TIMER_SIZE,
		Settings::DEFAULT_APPEARANCE_TIMER_SIZE), MIN_TIMER_SIZE, MAX_TIMER_SIZE);

	// `fullscreen` already reflects a fallback from the windowed mode
	int fontSize = fullscreen ? timerSize : windowedTimerSize(timerSize);

	bool showTimer = settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER,
		Settings::DEFAULT_APPEARANCE_SHOW_TIMER);
	TimerPosition timerPosition = toTimerPosition(settings.getInt(Settings::APPEARANCE_TIMER_POSITION,
		Settings::DEFAULT_APPEARANCE_TIMER_POSITION));

	// keep clear of a timer in the middle of the screen
	int clearCenterPx = 0;

	if (showTimer && timerPosition == TimerPosition::Center) {
		CClientDC screenDC(NULL);
		clearCenterPx = MulDiv(fontSize, screenDC.GetDeviceCaps(LOGPIXELSY), 72) * 6 / 5;
	}

	showMessageWindow(message, textColor, fontSize, clearCenterPx);
}

LockWindowAdapter::~LockWindowAdapter() {
	closeMessageWindow();

	if (settingsChanged)
		settings.save();

	std::reverse(windows.begin(), windows.end());

	for (const WindowPtr &window : windows)
		window->Close();

	if (settings.getBoolean(_T("midnight.wake"), false)) {
		SYSTEMTIME time;
		GetLocalTime(&time);

		if (time.wHour == 23 && time.wMinute == 59 || time.wHour == 0 && time.wMinute == 0)
			SetThreadExecutionState(ES_DISPLAY_REQUIRED);
	}

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

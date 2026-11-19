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
import QuoteWindow;
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

	// returns the bottom of the quote card, or 0
	int showBreakQuote();
	void showBreakMessage(int belowY);
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

	showBreakMessage(showBreakQuote());
}

int LockWindowAdapter::showBreakQuote() {
	if (!settings.getBoolean(Settings::QUOTES_ENABLE, Settings::DEFAULT_QUOTES_ENABLE))
		return 0;

	// remembers the quote shown, so the next break shows another one
	settingsChanged = true;

	return showQuoteWindow(settings, true, centerTimerClearancePx(settings, fullscreen));
}

// the message of a scheduled break, drawn over the break screen like the timer
void LockWindowAdapter::showBreakMessage(int belowY) {
	const tstring& message = StateLocked::getMessageText();

	if (message.empty())
		return;

	COLORREF textColor = settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR,
		Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR);

	int timerSize = std::clamp(settings.getInt(Settings::APPEARANCE_TIMER_SIZE,
		Settings::DEFAULT_APPEARANCE_TIMER_SIZE), MIN_TIMER_SIZE, MAX_TIMER_SIZE);

	// `fullscreen` already reflects a fallback from the windowed mode
	int fontSize = fullscreen ? timerSize : windowedTimerSize(timerSize);

	// keep clear of a timer in the middle of the screen, and of the quote
	showMessageWindow(message, textColor, fontSize, centerTimerClearancePx(settings, fullscreen), belowY);
}

LockWindowAdapter::~LockWindowAdapter() {
	closeMessageWindow();
	closeQuoteWindow();

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

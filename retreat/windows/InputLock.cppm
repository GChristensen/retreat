module;

#include "stdafx.h"

#include <cctype>

export module InputLock;

import <string>;
import <algorithm>;
import <functional>;

import Settings;
import StateMachine;

#include "debug.h"
#include "tstring.h"

export class InputLock {
public:
	InputLock(StateMachine &stateMachine);
	~InputLock();

	void lock();

	static std::function<void()> onNumpadPlus;
	static std::function<void()> onNumpadMinus;

	void setFullScreenMode(bool fullscreen);

private:
	bool fullscreen = false;

	static StateMachine *pStateMachine;

	static tstring unlockPhrase;
	static unsigned int unlockCounter;

	static HHOOK hKeyboardHook;
	static HHOOK hMouseHook;

	static LRESULT CALLBACK LowLevelKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
};

module :private;

StateMachine *InputLock::pStateMachine;

tstring InputLock::unlockPhrase;
unsigned int InputLock::unlockCounter;

HHOOK InputLock::hKeyboardHook;
HHOOK InputLock::hMouseHook;

std::function<void()> InputLock::onNumpadPlus;
std::function<void()> InputLock::onNumpadMinus;

InputLock::InputLock(StateMachine& stateMachine) {
	pStateMachine = &stateMachine;
	Settings &settings = stateMachine.getSettings();

	unlockPhrase = settings.getString(_T("unlock.phrase"), _T(""));

	std::transform(
		unlockPhrase.begin(),
		unlockPhrase.end(),
		unlockPhrase.begin(),
		toupper
	);

	unlockCounter = 0;
}

InputLock::~InputLock() {
	if (hMouseHook)
		UnhookWindowsHookEx(hMouseHook);

	if (hKeyboardHook)
		UnhookWindowsHookEx(hKeyboardHook);

	if (fullscreen)
		ShowCursor(TRUE);

	hMouseHook = 0;
	hKeyboardHook = 0;

	onNumpadPlus = nullptr;
	onNumpadMinus = nullptr;
}

void InputLock::setFullScreenMode(bool fullscreen) {
	this->fullscreen = fullscreen;
}

void InputLock::lock() {
	if (fullscreen)
		ShowCursor(FALSE);

	hKeyboardHook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		LowLevelKeyboardHookProc,
		(HINSTANCE)pStateMachine->getAppInstance(),
		0
	);

	hMouseHook = SetWindowsHookEx(
		WH_MOUSE_LL,
		LowLevelMouseHookProc,
		(HINSTANCE)pStateMachine->getAppInstance(),
		0
	);
}

LRESULT CALLBACK InputLock::LowLevelKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	PKBDLLHOOKSTRUCT p;

	if (nCode == HC_ACTION) {
		p = (PKBDLLHOOKSTRUCT)lParam;

		if (p->vkCode == VK_ADD && wParam == WM_KEYDOWN) {
			if (onNumpadPlus)
				onNumpadPlus();
			return 1;
		}
		else if (p->vkCode == VK_SUBTRACT && wParam == WM_KEYDOWN) {
			if (onNumpadMinus)
				onNumpadMinus();
			return 1;
		}
		else if (!unlockPhrase.empty() && wParam == WM_KEYDOWN) {
			if ((p->vkCode >= 0x30 && p->vkCode <= 0x39)
				|| (p->vkCode >= 0x41 && p->vkCode <= 0x5A)
				|| p->vkCode == VK_SPACE) {

				DWORD posCode = (DWORD)unlockPhrase.at(unlockCounter);

				if (posCode == p->vkCode) {
					unlockCounter += 1;
				}
				else {
					unlockCounter = 0;
				}

				if (unlockCounter == unlockPhrase.length()) {
					unlockCounter = 0;
					pStateMachine->setIdle();
				}
			}
			
#ifndef _DEBUG
			return 1;
#else			
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
#endif
		}
		else {
			return 1;
#ifndef _DEBUG
			return 1;
#else			
			return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
#endif
		}

	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


LRESULT CALLBACK InputLock::LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode >= 0) {
		if (wParam == WM_MOUSEMOVE || wParam == WM_MOUSEWHEEL) {
			return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
		}
	}

#ifndef _DEBUG
	return 1;
#else			
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
#endif
}

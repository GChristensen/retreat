module;

#include "stdafx.h"

export module InputMonitor;

import Monitor;

export class InputMonitor: public Monitor {
public:

	InputMonitor(void *hInstance);
	virtual ~InputMonitor();

	virtual bool isSatisfied() { return inputPresents; }

private:
	static bool inputPresents;

	static HHOOK hKeyboardHook;
	static HHOOK hMouseHook;

	static LRESULT CALLBACK LowLevelKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
};

module :private;

bool InputMonitor::inputPresents;

HHOOK InputMonitor::hKeyboardHook;
HHOOK InputMonitor::hMouseHook;

InputMonitor::InputMonitor(void *hInstance) {
	inputPresents = false;

	hKeyboardHook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		LowLevelKeyboardHookProc,
		(HINSTANCE)hInstance,
		0
	);

	hMouseHook = SetWindowsHookEx(
		WH_MOUSE_LL,
		LowLevelMouseHookProc,
		(HINSTANCE)hInstance,
		0
	);
}

InputMonitor::~InputMonitor() {
	if (hMouseHook)
		UnhookWindowsHookEx(hMouseHook);

	if (hKeyboardHook)
		UnhookWindowsHookEx(hKeyboardHook);

	hMouseHook = 0;
	hKeyboardHook = 0;
}

LRESULT CALLBACK InputMonitor::LowLevelKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION)
		inputPresents = true;

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


LRESULT CALLBACK InputMonitor::LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (wParam == WM_LBUTTONDOWN
			|| wParam == WM_RBUTTONDOWN
			|| wParam == WM_MOUSEMOVE
			|| wParam == WM_MOUSEWHEEL) {
			inputPresents = true;
		}
	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}


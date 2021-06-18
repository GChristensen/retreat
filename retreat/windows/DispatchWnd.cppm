module;

#include "stdafx.h"
#include "resource.h"

#include "tstring.h"

#include "retreat_pymodule.h"

export module DispatchWnd;

import <memory>;

import Controller;
import TrayIconImpl;

#include "debug.h"

#define WM_USER_TIMER (WM_USER + 1)
#define WM_GET_LOCK_STATE (WM_USER + 10)

typedef CWinTraits<WS_BORDER | WS_SYSMENU> DispatchTraits;

export class CDispatchWnd : public CWindowImpl<CDispatchWnd, CWindow, DispatchTraits>,
	public CTrayIconImpl<CDispatchWnd>
{
public:

	CDispatchWnd(HINSTANCE hInstance);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetLockState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnQueryEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DECLARE_WND_CLASS(_T("DispatchWindow"))

	BEGIN_MSG_MAP(CDispatchWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_USER_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_GET_LOCK_STATE, OnGetLockState)
		MESSAGE_HANDLER(msgTaskbarCreated, OnTaskBarCreated)
		MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
		//MESSAGE_HANDLER(WM_QUERYENDSESSION, OnQueryEndSession)
		CHAIN_MSG_MAP(CTrayIconImpl<CDispatchWnd>)
	END_MSG_MAP()


	bool IsMenuDisabled();
	void PrepareContextMenu(HMENU hMenu);

protected:

	Controller controller;

	HINSTANCE hInstance;

	int menuDisabled = 0;
	int msgTaskbarCreated;

	void showTrayIcon();
	void enableMenu();
	void disableMenu();
	void setMenuItemText(HMENU hMenu, int id, int resource);

	HANDLE timer = NULL;
	HANDLE timerEvent = NULL;
	HANDLE timerThread = NULL;
	static unsigned WINAPI timerThreadProc(void *param);
	void startTimer(int period);
	void stopTimer();
};

module :private;

import system;
import AboutDlg;
import MainOptionsDlg;

const int TIMER_PERIOD = 1000;

CDispatchWnd::CDispatchWnd(HINSTANCE hInstance): controller(hInstance)
{
	this->hInstance = hInstance;
}

LRESULT CDispatchWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	controller.reset();

	// show icon on Explorer restart by receiving TaskbarCreated message
	msgTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

	showTrayIcon();
	SetDefaultItem(-1);

	startTimer(TIMER_PERIOD);

	return 0;
}

LRESULT CDispatchWnd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	unsigned command = LOWORD(wParam);

	switch (command)
	{
	case ID_MENU_DISABLE:
		controller.enable(controller.canEnable());
		break;
	case ID_MENU_DELAY:
		controller.delay();
		break;
	case ID_MENU_SKIP:
		controller.skip();
		break;
	case ID_MENU_TAKEBREAK:
		controller.lock();
		break;
	case ID_MENU_OPTIONS:
		if (!menuDisabled)
		{
			disableMenu();

			CMainOptionsDlg optionsDlg(controller.getSettings());
			if (optionsDlg.DoModal(m_hWnd) == IDOK || optionsDlg.IsApplied())
				controller.reset();

			enableMenu();
		}
		break;
	case ID_MENU_ABOUT:
		if (!menuDisabled)
		{
			disableMenu();

			CAboutDlg aboutDlg;
			aboutDlg.DoModal();

			enableMenu();
		}
		break;
	case ID_MENU_EXIT:
		PostMessage(WM_CLOSE);
		break;
	default:
		break;
	}

	return 0;
}

LRESULT CDispatchWnd::OnGetLockState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return !controller.canExit();
}

LRESULT CDispatchWnd::OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	switch (wParam)
	{
	case PBT_APMSUSPEND:
		controller.stop();
		break;
	case PBT_APMRESUMEAUTOMATIC:
		controller.resume();
		break;
	}

	return 0;
}

//LRESULT CDispatchWnd::OnQueryEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
//{
//	
//	return TRUE;
//}

LRESULT CDispatchWnd::OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	showTrayIcon();

	return 0;
}

LRESULT CDispatchWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	stopTimer();

	RemoveIcon();
	DestroyWindow();
	return 0;
}

LRESULT CDispatchWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PostQuitMessage(0);
	return 0;
}

// context menu ///////////////////////////////////////////////////////////////

void CDispatchWnd::showTrayIcon()
{
	int icon_id = IDI_TRAY;

#ifdef PYTHON_MODULE
	switch (getPythonShowIcon()) {
	case ICON_HIDE:
		return;
	case ICON_SHOW:
		break;
	case ICON_SHOW_AMETHYST:
		icon_id = IDI_TRAY_AMETHYST;
	}
#endif

	// set tray icon
	HICON hIcon = (HICON)LoadImage(
		ModuleHelper::GetModuleInstance(),
		MAKEINTRESOURCE(icon_id),
		IMAGE_ICON, 16, 16, LR_SHARED
	);

	CString appName;
	appName.LoadString(IDS_APP_NAME);

	InstallIcon(appName, hIcon, IDR_SHELL_MENU);
}

void CDispatchWnd::enableMenu()
{
	menuDisabled -= 1;
}

void CDispatchWnd::disableMenu()
{
	menuDisabled += 1;
}

// this function is used by the tray icon helper class
bool CDispatchWnd::IsMenuDisabled()
{
	return menuDisabled > 0;
}

// this function is used by the tray icon helper class
void CDispatchWnd::PrepareContextMenu(HMENU hMenu)
{
	if (!controller.canDisable())
		EnableMenuItem(hMenu, ID_MENU_DISABLE, MF_DISABLED | MF_GRAYED);

	if (controller.canEnable())
		setMenuItemText(hMenu, ID_MENU_DISABLE, IDS_ENABLE_MENU_ITEM_NAME);

	if (!controller.canDelay()) 
		EnableMenuItem(hMenu, ID_MENU_DELAY, MF_DISABLED | MF_GRAYED);

	if (!controller.canExit()) {
		EnableMenuItem(hMenu, ID_MENU_OPTIONS, MF_DISABLED | MF_GRAYED);
		EnableMenuItem(hMenu, ID_MENU_EXIT, MF_DISABLED | MF_GRAYED);
	}

	if (!controller.canSkip())
		EnableMenuItem(hMenu, ID_MENU_SKIP, MF_DISABLED | MF_GRAYED);
}

void CDispatchWnd::setMenuItemText(HMENU hMenu, int id, int resource)
{
	MENUITEMINFO itemInfo = { sizeof(MENUITEMINFO), MIIM_STRING | MIIM_FTYPE, MFT_STRING, };

	CString itemText(MAKEINTRESOURCE(resource));

	itemInfo.dwTypeData = const_cast<TCHAR*>((const TCHAR*)itemText);

	SetMenuItemInfo(hMenu, id, FALSE, &itemInfo);
}

// timer thread ///////////////////////////////////////////////////////////////

void CDispatchWnd::startTimer(int period) 
{
	timer = CreateWaitableTimer(NULL, FALSE, NULL);

	LARGE_INTEGER timerExpires;
	timerExpires.QuadPart = Int32x32To64(-10000, period);
	SetWaitableTimer(timer, &timerExpires, period, NULL, NULL, FALSE);

	timerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	timerThread = (HANDLE)_beginthreadex(NULL, 0, timerThreadProc, (void*)this, 0, NULL);
}

void CDispatchWnd::stopTimer() 
{
	SetEvent(timerEvent);
	CancelWaitableTimer(timer);
	WaitForSingleObject(timerThread, TIMER_PERIOD + 1000);

	CloseHandle(timer);
	CloseHandle(timerEvent);
	CloseHandle(timerThread);
}


unsigned WINAPI CDispatchWnd::timerThreadProc(void* param) 
{
	CDispatchWnd* pThis = static_cast<CDispatchWnd*>(param);
	HANDLE handles[2] = { pThis->timer, pThis->timerEvent };

	while (true) 
	{
		if (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0) 
		{
			pThis->PostMessage(WM_USER_TIMER, 0, 0);
		}
		else
			break;
	}

	return 0;
}

LRESULT CDispatchWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	controller.onTimer();

	return 0;
}
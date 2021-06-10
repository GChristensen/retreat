module;

#include "stdafx.h"
#include "resource.h"

export module DispatchWnd;

import TrayIconImpl;

typedef CWinTraits<WS_BORDER | WS_SYSMENU> DispatchTraits;

export class CDispatchWnd : public CWindowImpl<CDispatchWnd, CWindow, DispatchTraits>,
	public CTrayIconImpl<CDispatchWnd>
{
public:

	CDispatchWnd();

	void PrepareContextMenu(HMENU hMenu);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	//LRESULT OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	//LRESULT OnQueryEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DECLARE_WND_CLASS(_T("DispatchWindow"))

	BEGIN_MSG_MAP(CDispatchWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(msgTaskbarCreated, OnTaskBarCreated)
		//MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
		//MESSAGE_HANDLER(WM_QUERYENDSESSION, OnQueryEndSession)
		CHAIN_MSG_MAP(CTrayIconImpl<CDispatchWnd>)
	END_MSG_MAP()


	bool isMenuDisabled();
	void enableMenu();
	void disableMenu();


protected:

	int menuDisabled = 0;
	int msgTaskbarCreated;

	void showTrayIcon();
};

module :private;

import AboutDlg;

CDispatchWnd::CDispatchWnd()
{
}

LRESULT CDispatchWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_tzset();

	// initialize random number generator
	srand((unsigned)time(NULL));

	// show icon on Explorer restart by receiving TaskbarCreated message
	msgTaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

	showTrayIcon();
	SetDefaultItem(-1);

	return 0;
}

LRESULT CDispatchWnd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	unsigned command = LOWORD(wParam);

	switch (command)
	{
	case ID_MENU_EXIT:
		PostMessage(WM_CLOSE);
		break;
	case ID_MENU_OPTIONS:
		//		showOptions();
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
	default:
		// delegate to state machine
		break;
	}

	return 0;
}

LRESULT CDispatchWnd::OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	showTrayIcon();

	return 0;
}

LRESULT CDispatchWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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
bool CDispatchWnd::isMenuDisabled()
{
	return menuDisabled > 0;
}

// this function is used by the tray icon helper class
void CDispatchWnd::PrepareContextMenu(HMENU hMenu)
{
}

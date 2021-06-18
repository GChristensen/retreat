// retreat.cpp : main source file for retreat.exe
//

#include "stdafx.h"
#include <gdiplus.h>

#include "dbgcout.h"
#include "LimitSingleInstance.h"

import DispatchWnd;

CAppModule _Module;

HWND hDispatchWnd;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CDispatchWnd wndMain(_Module.GetModuleInstance());

	if (wndMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	hDispatchWnd = wndMain.m_hWnd;

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int entry_win32(HINSTANCE hInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
#ifdef _DEBUG
#	define GLOBAL_MUTEX_NAME _T("EnsoRetreatGlobalMutex_Debug")
#else
#	define GLOBAL_MUTEX_NAME _T("EnsoRetreatGlobalMutex")
#endif

	LimitSingleInstance singleInstance(GLOBAL_MUTEX_NAME);

	if (singleInstance.IsAnotherInstanceRunning())
		return 0;

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR					 gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

#ifdef _DEBUG
	initdebug();
#endif

	hRes = _Module.Init(NULL, hInstance);

	_Module.SetResourceInstance(hInstance);

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	Gdiplus::GdiplusShutdown(gdiplusToken);
	::CoUninitialize();

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	return entry_win32(hInstance, lpstrCmdLine, nCmdShow);
}
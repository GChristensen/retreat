/*
 * Angelic Retreat
 * Copyright (C) 2008 G. Christensen
 * 
 * Angelic Retreat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Angelic Retreat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "SystemHelper.h"

#include <cctype> 
#include <cassert>
#include <algorithm>

#include <vector>

#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

#include <psapi.h>
#include <shlobj.h>
#include <atlmisc.h>

#ifndef NODXSDK
#	include <dshow.h>
#	pragma comment(lib, "amstrmid.lib")
#else
#	pragma comment(lib, "winmm.lib")
#endif

SystemHelper *SystemHelper::m_pInstance = NULL;

bool SystemHelper::m_bActivityPresents = false;

HHOOK SystemHelper::m_hKeyMonitoring = NULL;
HHOOK SystemHelper::m_hMouseMonitoring = NULL;

HHOOK SystemHelper::m_hKeyboardHook = NULL;
HHOOK SystemHelper::m_hMouseHook = NULL; 
HWND SystemHelper::m_hTargetWnd = NULL;
tstring SystemHelper::m_triggerPhrase;
int SystemHelper::m_triggerPhraseCtr = 0;
bool SystemHelper::m_userCanInterrupt = false;
unsigned short SystemHelper::m_triggerCommandID = 0;

SystemHelper::SystemHelper():
m_monitoringRequestCtr(0),
m_hAppInstance(NULL),
m_nowPlaying(false)
{
}

void SystemHelper::releaseInstance()
{
	delete m_pInstance;
	m_pInstance = NULL;
}

SystemHelper::~SystemHelper()
{
	if (m_hKeyboardHook != NULL)
	{
		UnhookWindowsHookEx(m_hKeyboardHook);
	}

	if (m_hMouseHook != NULL)
	{
		UnhookWindowsHookEx(m_hMouseHook);
	}
	 
	if (m_hKeyMonitoring != NULL)
	{
		UnhookWindowsHookEx(m_hKeyMonitoring);
	}

	if (m_hMouseMonitoring != NULL)
	{
		UnhookWindowsHookEx(m_hMouseMonitoring);
	}

	if (m_nowPlaying)
	{
#ifndef NODXSDK

		m_notifyWnd.DestroyWindow();
		cleanupDirectShow();

#endif
	}
}

SystemHelper *SystemHelper::getInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new SystemHelper;
	}

	return m_pInstance;
}

bool SystemHelper::isXP()
{
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	return (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)
		|| (osvi.dwMajorVersion > 5);
}

LRESULT CALLBACK SystemHelper::LowLevelMonitoringMouseHookProc
	(
	 int nCode, 
	 WPARAM wParam, 
	 LPARAM lParam
	)
{

	if (nCode >= 0)
	{
		if (wParam == WM_LBUTTONDOWN
			|| wParam == WM_RBUTTONDOWN 
			|| wParam == WM_MOUSEMOVE
			|| wParam == WM_MOUSEWHEEL)
		{
			m_bActivityPresents = true;
		}
	}

	return CallNextHookEx(m_hMouseMonitoring, nCode, wParam, lParam); 
} 

LRESULT CALLBACK SystemHelper::LowLevelMonintoringKeyboardProc
	(
	 int nCode, 
	 WPARAM wParam, 
	 LPARAM lParam
	)
{
	if (nCode == HC_ACTION) 
	{
		m_bActivityPresents = true;
	}

	return CallNextHookEx(m_hKeyMonitoring, nCode, wParam, lParam);
}

LRESULT CALLBACK SystemHelper::LowLevelKeyboardProc
	(
	 int nCode, 
	 WPARAM wParam, 
	 LPARAM lParam
	)
{
	PKBDLLHOOKSTRUCT p;

	if (nCode == HC_ACTION) 
	{
		p = (PKBDLLHOOKSTRUCT)lParam;

		bool control = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
		bool alt = (p->flags & LLKHF_ALTDOWN) != 0;

		if ( // return if following shortcuts have been pressed
				(p->vkCode == VK_TAB && alt)
				// ALT+ESC
				|| (p->vkCode == VK_ESCAPE && alt)    
				// CTRL+ESC
				|| ((p->vkCode == VK_ESCAPE) && control)
				// CTRL+TAB
				|| ((p->vkCode == VK_TAB) && control)
				// CTRL+SHIFT+ESC
				|| ((p->vkCode == VK_ESCAPE) && control && shift)
		   )
		{
		   return 1;
		}
		else if (control || shift || alt || p->vkCode == VK_ADD
						 || p->vkCode == VK_SUBTRACT)
		{ // only shortcuts are allowed, therefore user can control applications
		  // with global hotkeys
			return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam);
		}
		else if	(m_userCanInterrupt && m_hTargetWnd && wParam == WM_KEYDOWN)
		{
			if (   (p->vkCode >= 0x30 && p->vkCode <= 0x39)
				|| (p->vkCode >= 0x41 && p->vkCode <= 0x5A) 
				|| p->vkCode == VK_SPACE) 
			{	
				int triggerLength = m_triggerPhrase.length();
				
				DWORD posCode = static_cast<DWORD>(
					m_triggerPhrase.at(m_triggerPhraseCtr));

				if (posCode == p->vkCode)
				{
					m_triggerPhraseCtr += 1;
				}
				else
				{
					m_triggerPhraseCtr = 0;
				}

				if (m_triggerPhraseCtr == triggerLength)
				{
					::SendMessage(
						m_hTargetWnd, WM_COMMAND, m_triggerCommandID , NULL);

					m_triggerPhraseCtr = 0;
				}
			}

			return 1;
		}
		else 
		{
			return 1;
		}
		
	}

	return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK SystemHelper::LowLevelMouseHookProc
	(
	 int nCode, 
	 WPARAM wParam, 
	 LPARAM lParam
	)
{

	if (nCode >= 0)
	{
		if (wParam == WM_MOUSEMOVE
			|| wParam == WM_MOUSEWHEEL)
		{
			return CallNextHookEx(m_hMouseMonitoring, nCode, wParam, lParam); 
		}
	}

	return 1; 
} 

void SystemHelper::setAppInstanceHandle(HINSTANCE hInstance)
{
	m_hAppInstance = hInstance;
}

// this will work because we only need to determine if user didn't show any
// activity
int SystemHelper::startActivityMonitoring()
{
	assert(m_hAppInstance);

	if (m_monitoringRequestCtr == 0)
	{
		m_bActivityPresents = false;

		m_hKeyMonitoring = SetWindowsHookEx(
			WH_KEYBOARD_LL, 
			LowLevelMonintoringKeyboardProc, 
			m_hAppInstance, 
			0
			);

		m_hMouseMonitoring = SetWindowsHookEx(
			WH_MOUSE_LL, 
			LowLevelMonitoringMouseHookProc, 
			m_hAppInstance, 
			0
			);
	}

	m_monitoringRequestCtr += 1;
	
	return 1;
}

int SystemHelper::finishActivityMonitoring()
{
	if (m_monitoringRequestCtr == 0)
	{
		return -1;
	}

	m_monitoringRequestCtr =- 1;

	if (m_monitoringRequestCtr == 0)
	{
		UnhookWindowsHookEx(m_hMouseMonitoring);
		m_hMouseMonitoring = NULL;

		UnhookWindowsHookEx(m_hKeyMonitoring);
		m_hKeyMonitoring = NULL;
	}
	
	return m_bActivityPresents? 1: 0;
}

int SystemHelper::lockUserInput
	(
	 HWND targetWnd, 
	 const tstring &triggerPhrase,
	 unsigned short triggerCommand
	)
{
	assert(m_hAppInstance);

	if (m_hKeyboardHook == NULL && m_hMouseHook == NULL)
	{
		m_hTargetWnd = targetWnd;
		m_triggerPhraseCtr = 0;

		m_triggerPhrase = triggerPhrase;
		m_userCanInterrupt = m_triggerPhrase.length() > 0;
		m_triggerCommandID = triggerCommand;

		std::transform(
			m_triggerPhrase.begin(), 
			m_triggerPhrase.end(), 
			m_triggerPhrase.begin(), 
			std::toupper
			);

		m_hKeyboardHook = SetWindowsHookEx(
			WH_KEYBOARD_LL, 
			LowLevelKeyboardProc, 
			m_hAppInstance, 
			0
			);

		m_hMouseHook = SetWindowsHookEx(
			WH_MOUSE_LL, 
			LowLevelMouseHookProc, 
			m_hAppInstance, 
			0
			);

		return 1;
	}

	return 0;
}

int SystemHelper::unlockUserInput()
{
	if (m_hKeyboardHook != NULL && m_hMouseHook != NULL)
	{
		UnhookWindowsHookEx(m_hMouseHook);
		UnhookWindowsHookEx(m_hKeyboardHook);

		m_hKeyboardHook = NULL;
		m_hMouseHook = NULL;

		return 1;
	}

	return 0;
}

bool SystemHelper::executableInMemory(const std::vector<tstring> &executables)
{
	HANDLE hHeap = GetProcessHeap();

	DWORD dwError;
	DWORD cbReturned;
	DWORD cbAlloc = 128;
	DWORD * pdwIds = NULL;

	bool result = false;

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	DWORD dwSystemId = (osvi.dwMajorVersion < 5) ? 2 : 8;

	// get active processes info
	do
	{
		cbAlloc *= 2;

		if (pdwIds != NULL)
			HeapFree(hHeap, 0, pdwIds);

		pdwIds = (DWORD *)HeapAlloc(hHeap, 0, cbAlloc);
		if (pdwIds == NULL)
		{
			return SetLastError(ERROR_NOT_ENOUGH_MEMORY), false;
		}

		if (!EnumProcesses(pdwIds, cbAlloc, &cbReturned))
		{
			dwError = GetLastError();

			HeapFree(hHeap, 0, pdwIds);
			return SetLastError(dwError), false;
		}
	}
	while (cbReturned == cbAlloc);

	// convert executable file names to upper case
	std::vector<tstring> 
		executablesUpcase(executables.begin(), executables.end());
	
	foreach(tstring &exe, executablesUpcase)
	{
		std::transform(exe.begin(), exe.end(), exe.begin(), std::toupper);
	}

	// enum processes
	for (DWORD i = 0; i < cbReturned / sizeof(DWORD); i++)
	{
		BOOL bContinue = TRUE;
		DWORD dwProcessId = pdwIds[i];

		if (dwProcessId == 0)
		{
			// "Idle" process
		}
		else if (dwProcessId == dwSystemId)
		{
			// "System" process
		}
		else
		{
			HANDLE hProcess;
			TCHAR szModulePath[MAX_PATH];

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, dwProcessId);

			if (hProcess != NULL)
			{
				GetModuleFileNameEx(hProcess, NULL, szModulePath, MAX_PATH);
			}

			CloseHandle(hProcess);

			tstring modulePathUpcase = szModulePath;
			std::transform(
				modulePathUpcase.begin(), 
				modulePathUpcase.end(), 
				modulePathUpcase.begin(), 
				std::toupper
				);

			foreach(tstring &exe, executablesUpcase)
			{
				if (modulePathUpcase == exe)
				{
					result = true;
					bContinue = FALSE;
					break;
				}
			}
		}

		if (!bContinue)
			break;
	}

	HeapFree(hHeap, 0, pdwIds);

	return result;
}

void SystemHelper::minimizeWorkingSet()
{
	HANDLE hProc = OpenProcess(PROCESS_SET_QUOTA, FALSE, GetCurrentProcessId());
	SetProcessWorkingSetSize(hProc, -1, -1);
	CloseHandle(hProc);
}



void SystemHelper::searchForFiles(const tstring &dir, 
								  const tstring &ext,
								  std::vector<CString> &files)
{
	CString pattern = (dir + _T("\\*.") + ext).c_str();
	CFindFile finder;

	if (finder.FindFile(pattern))
	{
		do
		{
			if (finder.IsDots() || finder.IsDirectory())
			{
				continue;
			}

			files.push_back(finder.GetFilePath());
		} while (finder.FindNextFile());
	}

	finder.Close();
}

void SystemHelper::getArbitraryFilePath
	(
	 const tstring &dir, 
	 const tstring &ext, 
	 const tstring &ext2,
	 const tstring &ext3,
	 tstring &path
	)
{
	using WTL::CString;

	std::vector<CString> files;

	searchForFiles(dir, ext, files);

	if (!ext2.empty())
		searchForFiles(dir, ext2, files);
	
	if (!ext3.empty())
		searchForFiles(dir, ext3, files);

	if (!files.empty())
	{
		unsigned int n = rand() % files.size();
		path = (const TCHAR *)files[n];
	}
}

tstring SystemHelper::getModuleDir()
{
	TCHAR module_dir[MAX_PATH];

	GetModuleFileName(m_hAppInstance, module_dir, MAX_PATH);

	TCHAR *p = _tcsrchr(module_dir, _T('\\'));

	*(p + 1) = 0;

	return module_dir;
}

tstring SystemHelper::getSettingsForCurrentUser
	(
	 const tstring &app_dir, 
	 const tstring &file
	)
{
	TCHAR path[MAX_PATH];

	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	tstring settings = path;
	
	if (settings[settings.length() - 1] != _T('\\'))
		settings.append(_T("\\"));

	settings.append(app_dir);
	settings.append(_T("\\"));

	// check whether application directory exists
	if (::GetFileAttributes(settings.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectory(settings.c_str(), NULL);
	}

	settings.append(file);
	return settings;
}

void SystemHelper::getLanguages
	(
	 const tstring &dir, 
	 std::map<tstring, tstring> &languages
	)
{
	languages.clear();

	tstring resource_pattern = dir;
	resource_pattern.append(_T("\\*.dll"));

	using WTL::CString;

	CFindFile finder;

	if (finder.FindFile(resource_pattern.c_str()))
	{
		do
		{
			HMODULE library = ::LoadLibrary(finder.GetFilePath());

			TCHAR lang_name[128];

			memset(lang_name, 0, sizeof(lang_name));

			::LoadString(
				library, 1, 
				lang_name, sizeof(lang_name) / sizeof(lang_name[0] - 1)
				);

			if (_tcslen(lang_name))
			{
				languages[(LPCTSTR)finder.GetFileName()] = lang_name;
			}

			::FreeLibrary(library);

		} while (finder.FindNextFile());
	}

	finder.Close();
}

HRESULT SystemHelper::playSound(const tstring &file)
{
#ifndef NODXSDK

	// and no multithreading of course
	// function should not be called when sound is still playing
	if (m_nowPlaying)
	{
		return E_FAIL;
	}

	m_nowPlaying = true;

	HRESULT h;

	// create Graph Buider
	h = CoCreateInstance(
			CLSID_FilterGraph, NULL,
			CLSCTX_INPROC, IID_IGraphBuilder,
			(void **)&m_pGraphBuilder
			);

	if (FAILED(h))
	{
		m_nowPlaying = false;
		return h;
	}

	// get the IMediaControl Interface
	m_pGraphBuilder->QueryInterface(
		IID_IMediaControl, (void **)&m_pMediaControl);

	// get the IMediaEventEx Interface
	m_pGraphBuilder->QueryInterface(
		IID_IMediaEventEx, (void **)&m_pMediaEventEx);

	IBaseFilter	*pSource;

	// add the new source filter to the graph
	h = m_pGraphBuilder->AddSourceFilter(file.c_str(), file.c_str(), &pSource);

	if (FAILED(h))
	{
		cleanupDirectShow();
		return h;
	}

	IPin *pPin = NULL;

	// get the first output pin of the new source filter
	// audio sources typically have only one output pin, 
	// so for most audio cases finding any output pin is sufficient
	h = pSource->FindPin(L"Output", &pPin); 
	pSource->Release();

	if (FAILED(h))
	{
		cleanupDirectShow();
		return h;
	}

	// we have the new output pin, render it
	h = m_pGraphBuilder->Render(pPin);

	if (FAILED(h))
	{
		cleanupDirectShow();
		return h;
	}

	pPin->Release();

	// create notify window
	m_notifyWnd.Create(this);

	// set message sent when playing ends
	h = m_pMediaEventEx->SetNotifyWindow(
		(OAHWND)m_notifyWnd.m_hWnd, MEDIA_NOTIFY_MSG, 0);

	if (FAILED(h))
	{
		m_notifyWnd.DestroyWindow();
		cleanupDirectShow();
		return h;
	}

	// start the graph
	m_pMediaControl->Run();

	return S_OK;

#else

	// PlaySound can only play WAV files
	return PlaySound(file.c_str(), 0, SND_FILENAME | SND_ASYNC)? S_OK : E_FAIL;

#endif
}

#ifndef NODXSDK

void SystemHelper::cleanupDirectShow()
{

	if (m_pMediaControl != NULL)
	{
		m_pMediaControl->Stop();
		m_pMediaControl->Release();
	}
	
	if (m_pMediaEventEx != NULL)
	{
		m_pMediaEventEx->Release();	
	}

	if (m_pGraphBuilder != NULL)
	{
		m_pGraphBuilder->Release();
	}
	
	m_nowPlaying = false;
}

LRESULT SystemHelper::CMediaNotifyWnd::OnPlayEnd
	(
	 UINT uMsg, 
	 WPARAM wParam, 
	 LPARAM lParam, 
	 BOOL &
	)
{
	long evCode, param1, param2;

	while (SUCCEEDED(
		m_helper->m_pMediaEventEx->GetEvent(&evCode, &param1, &param2, 0)))
	{
		m_helper->m_pMediaEventEx->FreeEventParams(evCode, param1, param2);

		switch (evCode) 
		{
		case EC_COMPLETE:
			// playback complete
			m_helper->cleanupDirectShow();

			DestroyWindow();

			m_helper = NULL;

			return 0;
		}
	}
	
	return 0;
}

#endif
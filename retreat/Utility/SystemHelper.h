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

#pragma once

#include <vector>
#include <map>

#include "windows.h"
#include "../tstring.h"

#ifndef NODXSDK

interface IGraphBuilder;
interface IMediaControl;
interface IMediaEventEx;

#endif

class SystemHelper
{
public:

	static SystemHelper *getInstance();
	static void releaseInstance();
	~SystemHelper();

	void setAppInstanceHandle(HINSTANCE hInstance);

	bool isXP();

	int startActivityMonitoring();
	int finishActivityMonitoring();

	int lockUserInput
		(
		 HWND targetWnd, 
		 const tstring &triggerPhrase,
		 unsigned short triggerCommand
		);
	int unlockUserInput();

	bool executableInMemory(const std::vector<tstring> &executables);

	void minimizeWorkingSet();

	void getArbitraryFilePath
		(
		const tstring &dir, 
		const tstring &ext, 
		const tstring &ext2,
		const tstring &ext3,
		tstring &path
		);

	tstring getModuleDir();

	tstring getSettingsForCurrentUser
		(
		 const tstring &app_dir, 
		 const tstring &file
		);

	void getLanguages
		(
		 const tstring &dir,
		 std::map<tstring, tstring> &languages
		);

	HRESULT playSound(const tstring &file);

private:

	SystemHelper();

	static SystemHelper *m_pInstance;
	HINSTANCE m_hAppInstance;

	int m_monitoringRequestCtr;

	static HHOOK m_hKeyMonitoring;
	static HHOOK m_hMouseMonitoring;

	static bool m_bActivityPresents;

	static HHOOK m_hKeyboardHook;
	static HHOOK m_hMouseHook; 

	static HWND m_hTargetWnd;
	static tstring m_triggerPhrase;
	static int m_triggerPhraseCtr;
	static bool m_userCanInterrupt;
	static unsigned short m_triggerCommandID;

	static LRESULT CALLBACK LowLevelMonitoringMouseHookProc
		(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMonintoringKeyboardProc
		(int nCode, WPARAM wParam, LPARAM lParam) ;

	static LRESULT CALLBACK LowLevelKeyboardProc
		(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseHookProc
		(int nCode, WPARAM wParam, LPARAM lParam);

	bool m_nowPlaying;

#ifndef NODXSDK

	static const int MEDIA_NOTIFY_MSG = WM_APP + 1;

	void cleanupDirectShow();

	class CMediaNotifyWnd: 
		public CWindowImpl<CMediaNotifyWnd, CWindow, CNullTraits>
	{
	public:

		void Create(SystemHelper *instance)
		{
			m_helper = instance;

			CWindowImpl<CMediaNotifyWnd, CWindow, CNullTraits>::Create(0);
		}

		BEGIN_MSG_MAP(CMediaNotifyWnd)
			MESSAGE_HANDLER(SystemHelper::MEDIA_NOTIFY_MSG, OnPlayEnd)
		END_MSG_MAP()

		LRESULT OnPlayEnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);

	private:
		
		SystemHelper *m_helper;
	};

	CMediaNotifyWnd m_notifyWnd;

	IGraphBuilder   *m_pGraphBuilder;
	IMediaControl   *m_pMediaControl;
	IMediaEventEx   *m_pMediaEventEx;

#endif

	void searchForFiles(const tstring &dir, 
						const tstring &ext,
						std::vector<WTL::CString> &files);

};

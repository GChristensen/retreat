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
#include "../resource.h"

#include "../scheduler/scheduler.h"
#include "../utility/SystemHelper.h"

#include "LockState.h"
#include "IdleState.h"

#include "boost/foreach.hpp"
#define foreach BOOST_FOREACH

#define LOCK_TASK_NAME _T("AngelicMachine:Lock")


LockState::angelic_wnd_ptr_t
LockState::createAngelicWindow(AngelicMachine *machine, CRect *rect, bool primary)
{
 	int mode = appearance(machine).Mode;
	m_hideCursor = mode == AM_FULLSCREEN_MODE;

	angelic_wnd_ptr_t pAngelicWnd = 
			angelic_wnd_ptr_t(
				new CAngelicWnd(
					0,
					rect, 
					(mode > AM_FULLSCREEN_MODE)
						? const_cast<CRect *>(&machine->getWorkArea())
						: NULL
				)
			);

	tstring imageDir = appearance(machine).ImageFolder[mode];
	tstring backgroundImagePath;

	if (!imageDir.empty())
	{
		SystemHelper::getInstance()->
			getArbitraryFilePath(imageDir, _T("jp*"), _T("bmp"), _T("gif"), backgroundImagePath);
	}
	
	bool backgroundLoaded = 
		pAngelicWnd->LoadBackground(
			backgroundImagePath.c_str(), 
			appearance(machine).StretchImage,
			appearance(machine).BackgroundColor
			);

	if (backgroundLoaded)
	{
		if (appearance(machine).UseTransparency)
		{
			COLORREF transparent = appearance(machine).TransparentColor;

			if (transparent == AM_DEFAULT_TRANSPARENT_COLOR)
			{
				pAngelicWnd->SetTransparentColorFromImageLeftUpperPixel();
			}
			else
			{
				pAngelicWnd->SetTransparentColor(transparent);
			}
		}
	}
	else
	{
		if (mode != AM_FULLSCREEN_MODE)
		{
			mode = AM_FULLSCREEN_MODE;
			pAngelicWnd->SendMessage(WM_CLOSE);
			pAngelicWnd = angelic_wnd_ptr_t(new CAngelicWnd(0, NULL, NULL));
		}

		pAngelicWnd->SpecialBackground(CAngelicWnd::STARS);
		m_hideCursor = false;
	}

	pAngelicWnd->SetAlpha(appearance(machine).TransparencyLevel);	

	if (primary && appearance(machine).ShowTimer)
	{
		AM_APPEARANCE_SETTINGS::TEXT_SETTINGS &text =
			appearance(machine).TimerFontSettings[mode];

		bool antialias = 
			backgroundLoaded && !appearance(machine).UseTransparency;

		int breakLengthMin = 
			boost::any_cast<int>(getCell(machine, BREAK_DURATION_CELL));

		pAngelicWnd->SetTimerProperties(
			text.Face.c_str(),
			DEFAULT_CHARSET,
			text.Size,
			text.Color,
			antialias,
			text.Bold,
			text.Italic
			);
	
		pAngelicWnd->SetTimerPos(text.x, text.y);
		pAngelicWnd->SetTimer(breakLengthMin * 60);
	}
	else 
	{
		pAngelicWnd->SetShowTimer(false);
	}

#ifndef _DEBUG
	if (!m_locked)
	{
		SystemHelper::getInstance()->lockUserInput(
			getParentWindow(machine),
			behaviour(machine).MagicWords,
			ID_USER_UNLOCK_COMMAND
			);

		if (m_hideCursor)
		{
			ShowCursor(FALSE);
		}

		m_locked = true;
	}
#endif

	pAngelicWnd->CenterWindowOnWorkArea();
	pAngelicWnd->ShowWindow(SW_SHOW);

	
	pAngelicWnd->GrabUserInput();

	// window will not be visible before SetLayeredWindowAttributes call
	// we should call this after window has been shown to reduce blinking
	pAngelicWnd->ApplyLayeredWindowAttributes();

	pAngelicWnd->DoEvents();

	return pAngelicWnd;
}

BOOL CALLBACK LockState::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
										 LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);

	if (GetMonitorInfo(hMonitor, &info))
	{
		bool primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
		LockState *state = (LockState *)dwData;

		if (!primary && state->appearance(state->m_contextMachine).Mode
			!= AM_FULLSCREEN_MODE)
			return TRUE;

		CRect rect(*lprcMonitor);
		state->m_pAngelicWnds.push_back(state->
			createAngelicWindow(state->m_contextMachine, &rect, primary));

		if (primary)
		{
			state->m_pPrimaryWindow = state->m_pAngelicWnds.back();
		}
	}

	return TRUE;
}

LockState::LockState(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	m_locked = false;
	m_contextMachine = machine;

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);
	
	int breakLengthMin = 
		boost::any_cast<int>(getCell(machine, BREAK_DURATION_CELL));

	scheduler->addSchedule(
		new MeanderSchedule(
		LOCK_TASK_NAME,
		breakLengthMin * 60 + 1,
		Event(AngelicMachine::TIMER_ANGELIC_WND_NOTIFY),
		Event(AngelicMachine::TIMER_RETURN_IDLE_STATE)
		));

	ATLTRACE("Angelic Machine: lock\n");
}

LockState::~LockState()
{
}

void LockState::userEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event
	)
{
	switch (event)
	{
	case AngelicMachine::USER_UNLOCK:
		{
			terminate(machine, scheduler);
			setState(machine, new IdleState(machine, scheduler));
		}
		break;
	}
}

void LockState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long data
	)
{
	switch (event)
	{
	case AngelicMachine::TIMER_ANGELIC_WND_NOTIFY:
		{
			m_pPrimaryWindow->PostMessage(CAngelicWnd::WM_ANGELIC_WND_NOTIFY);
		}
		break;
	case AngelicMachine::TIMER_RETURN_IDLE_STATE:
		{
			cleanup(machine, scheduler);
			
			if (behaviour(machine).BeepAfterBreak)
			{
				tstring sound;
				SystemHelper::getInstance()->getArbitraryFilePath(
					appearance(machine).SoundFolder, _T("*"), _T(""), _T(""), sound);

				ATLTRACE(_T("Playing file: %s\n"), sound.c_str());

				if (!sound.empty())
				{
					if (FAILED(SystemHelper::getInstance()->playSound(sound)))
					{
						MessageBeep(MB_ICONASTERISK);
					}
				}
				else 
				{
					MessageBeep(MB_ICONASTERISK);
				}
			}
			
			if (isCellSet(machine, TIMED_BREAK_MESSAGE))
			{
				tstring message = boost::any_cast<tstring>(getCell(machine, 
					TIMED_BREAK_MESSAGE));

				if (message != _T(""))
				{
					PostMessage(getParentWindow(machine), WM_DISPLAY_GLOBAL_MESSAGE, 
						(WPARAM)_tcsdup(message.c_str()), 0);
				}
			}

			setCell(machine, TIMED_BREAK_MESSAGE, tstring(_T("")));

			setState(machine, new IdleState(machine, scheduler));
		}
		break;
	}
}

void LockState::terminate(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	scheduler->chancelSchedule(LOCK_TASK_NAME);
	
	cleanup(machine, scheduler);
}

void LockState::cleanup(AngelicMachine *machine, SimpleScheduler *scheduler)
{
#ifndef _DEBUG
	if (m_hideCursor)
	{
		ShowCursor(TRUE);
	}
	SystemHelper::getInstance()->unlockUserInput();

#endif

	//m_pPrimaryWindow->ReturnUserInput();

	appearance(machine).TransparencyLevel = m_pPrimaryWindow->GetAlpha();

	foreach(angelic_wnd_ptr_t &wnd, m_pAngelicWnds)
		wnd->SendMessage(WM_CLOSE);

	// fix sticky keys
	keybd_event(VK_LCONTROL, 0, 0, 0);
	keybd_event(VK_LCONTROL, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_RCONTROL, 0, 0, 0);
	keybd_event(VK_RCONTROL, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LSHIFT, 0, 0, 0);
	keybd_event(VK_LSHIFT, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_RSHIFT, 0, 0, 0);
	keybd_event(VK_RSHIFT, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_LMENU, 0, 0, 0);
	keybd_event(VK_LMENU, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_RMENU, 0, 0, 0);
	keybd_event(VK_RMENU, 0, KEYEVENTF_KEYUP, 0);
}
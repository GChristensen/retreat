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
#include "AlertState.h"
#include "DelayState.h"
#include "LockState.h"
#include "IdleState.h"

#include "resource.h"

#include "../scheduler/scheduler.h"

#define ALERT_TASK_NAME _T("AngelicMachine:Alert")
#define ALERT_WINDOW_BACKGROUND RGB(254, 0, 254)

AlertState::AlertState(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	m_delayEvent = NULL;
	m_skipEvent = NULL;

	m_contextMachine = machine;

	SYSTEMTIME now;
	::GetLocalTime(&now);

	if (behaviour(machine).CurrentDay != now.wDay)
	{
		behaviour(machine).CurrentDay = now.wDay;
		setCell(machine, SKIP_BREAK_FLAG_CELL, true);
		m_skipEvent = new GlobalEvent(L"@@__ANGELIC_RETREAT_SKIP_EVENT__@@", this, machine, scheduler,
			ID_MENU_SKIP, behaviour(machine).AlertBeforeMin * 60 * 1000);
	}

	m_delayEvent = new GlobalEvent(L"@@__ANGELIC_RETREAT_DELAY_EVENT__@@", this, machine, scheduler,
		ID_MENU_DELAY, behaviour(machine).AlertBeforeMin * 60 * 1000);

	m_alertBeforeSec = behaviour(machine).AlertBeforeSec;
	
	if (m_alertBeforeSec == 0)
	{
		m_alertBeforeSec = behaviour(machine).AlertBeforeMin * 60;
	}

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)this);

	scheduler->addSchedule(
		new MeanderSchedule(
				ALERT_TASK_NAME,
				m_alertBeforeSec,
				Event(AngelicMachine::TIMER_ANGELIC_WND_NOTIFY),
				Event(AngelicMachine::TIMER_INITIATE_LOCK)
				));

	ATLTRACE("Angelic Machine: alert\n");
}

AlertState::~AlertState()
{
	if (m_skipEvent)
		delete m_skipEvent;
	delete m_delayEvent;
}

AlertState::angelic_wnd_ptr_t
AlertState::createAngelicWindow(AngelicMachine *machine, CRect *rect)
{
	angelic_wnd_ptr_t pAngelicWnd = 
		angelic_wnd_ptr_t(new CAngelicWnd(0, NULL, rect));

	AM_APPEARANCE_SETTINGS::TEXT_SETTINGS &text =
		appearance(machine).TimerFontSettings[AM_ALERT_MODE];

	pAngelicWnd->SetTimerProperties(
		text.Face.c_str(),
		DEFAULT_CHARSET,
		text.Size,
		text.Color,
		false,
		text.Bold,
		text.Italic
		);

	pAngelicWnd->SetTimer(m_alertBeforeSec - 1);
	pAngelicWnd->PlaceWindowOnWorkArea(text.x, text.y);
	pAngelicWnd->SetAlpha(appearance(machine).TransparencyLevel);

	pAngelicWnd->ApplyLayeredWindowAttributes();
	pAngelicWnd->ShowWindow(SW_SHOWNOACTIVATE);

	return pAngelicWnd;
}

BOOL CALLBACK AlertState::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
										 LPRECT lprcMonitor, LPARAM dwData)
{
	AlertState *state = (AlertState *)dwData;

	CRect rect(*lprcMonitor);
	state->m_pAngelicWnds.push_back(state->
		createAngelicWindow(state->m_contextMachine, &rect));

	return TRUE;
}

void AlertState::userEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event
	)
{
	switch (event)
	{
	case AngelicMachine::USER_DELAY:
		{
			int delayCount = 
				boost::any_cast<int>(getCell(machine, DELAY_COUNTER_CELL));

			if (delayCount < behaviour(machine).DelayLimit)
			{
				setCell(machine, DELAY_COUNTER_CELL, ++delayCount);

				terminate(machine, scheduler);
				setState(machine, new DelayState(machine, scheduler));
			}
		}
		break;
		case AngelicMachine::USER_SKIP:
		{
			bool allowSkip = 
				boost::any_cast<bool>(getCell(machine, SKIP_BREAK_FLAG_CELL));

			if (allowSkip)
			{
				setCell(machine, SKIP_BREAK_FLAG_CELL, false);

				terminate(machine, scheduler);
				setState(machine, new IdleState(machine, scheduler));
			}
		}
		break;
	}
}

void AlertState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long
	)
{
	switch (event)
	{
	case AngelicMachine::TIMER_ANGELIC_WND_NOTIFY:
		{
			foreach(angelic_wnd_ptr_t wnd, m_pAngelicWnds)
				wnd->PostMessage(CAngelicWnd::WM_ANGELIC_WND_NOTIFY);
		}
		break;
	case AngelicMachine::TIMER_INITIATE_LOCK:
		{
			foreach(angelic_wnd_ptr_t wnd, m_pAngelicWnds)
				wnd->SendMessage(WM_CLOSE);
			setState(machine, new LockState(machine, scheduler));
		}
		break;
	}

}

void AlertState::terminate(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	scheduler->chancelSchedule(ALERT_TASK_NAME);
	foreach(angelic_wnd_ptr_t wnd, m_pAngelicWnds)
		wnd->SendMessage(WM_CLOSE);
}

AbstractMachine::flag_t AlertState::flags(AngelicMachine *machine)
{
	AbstractMachine::flag_t flags = 0;

	int delayCount = 
		boost::any_cast<int>(getCell(machine, DELAY_COUNTER_CELL));

	if (delayCount < behaviour(machine).DelayLimit)
	{
		flags |= AngelicMachine::USER_DELAY_FLAG;
	}

	bool allowSkip = 
		boost::any_cast<bool>(getCell(machine, SKIP_BREAK_FLAG_CELL));

	if (allowSkip)
	{
		flags |= AngelicMachine::USER_SKIP_FLAG;
	}

	return flags;
}
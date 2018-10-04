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
#include "AngelicMachine.h"
#include "IdleState.h"
#include "AlertState.h"

#ifdef _DEBUG
#	include "LockState.h"
#endif

#include "../scheduler/scheduler.h"

#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

#define PERIODIC_TASK_NAME _T("AngelicMachine:PeriodicBreaks")
#define TIMED_TASK_NAME _T("AngelicMachine:TimedBreaks")

AngelicMachine::AngelicMachine(const tstring &name, HWND parent):
AbstractMachine(name),
m_parentWindow(parent),
m_pState(NULL)
{
}

AngelicMachine::~AngelicMachine()
{
	ATLTRACE("Angelic Machine has gone\n");
}

void AngelicMachine::readSettings(SettingTable *settings)
{
	m_behaviourSettings.read(settings);
	m_appearanceSettings.read(settings);
	m_timedBreaksData.read(settings);
}

void AngelicMachine::writeSettings(SettingTable *settings)
{
	m_behaviourSettings.write(settings);
	m_appearanceSettings.write(settings);
}

void AngelicMachine::start(SimpleScheduler *scheduler)
{
	if (m_pState == NULL)
	{
		if (m_behaviourSettings.UsePeriodicBreaks)
		{
			QueueSchedule *periodicBreaks = new QueueSchedule(
				tstring(PERIODIC_TASK_NAME),
				m_behaviourSettings.PeriodLengthMin * 60,
				m_behaviourSettings.PeriodLengthMin
				);

			if (m_behaviourSettings.CheckForUserActivityBeforeMin > 0)
			{
				Event startUserListening(TIMER_START_LISTENING_USER);
				startUserListening.setPoint(  
					m_behaviourSettings.PeriodLengthMin 
					- m_behaviourSettings.BreakLengthMin
					- m_behaviourSettings.AlertBeforeMin
					- m_behaviourSettings.CheckForUserActivityBeforeMin
					);
				periodicBreaks->addEvent(startUserListening);
			}

			Event startAlert(TIMER_START_ALERT);
			startAlert.setPoint(  
				m_behaviourSettings.PeriodLengthMin 
				- m_behaviourSettings.BreakLengthMin
				- m_behaviourSettings.AlertBeforeMin
				);
			periodicBreaks->addEvent(startAlert);

			if (m_behaviourSettings.SynchronizeWithSystemTime)
			{
				SYSTEMTIME now;
				::GetLocalTime(&now);

				periodicBreaks->adjust(now.wMinute * 60 + now.wSecond);	
			}

			scheduler->addSchedule(periodicBreaks);
		}

		size_t nTimedBreaks = m_timedBreaksData.breaks.size();

		if (m_behaviourSettings.UseTimedBreaks && nTimedBreaks > 0)
		{
			CronSchedule *timedBreaks = 
				new CronSchedule(TIMED_TASK_NAME);

			int valid = 0;
			for (size_t i = 0; i < nTimedBreaks; ++i)
			{
				try
				{
					timedBreaks->addCrontab(
						m_timedBreaksData.breaks[i].crontab,
						m_timedBreaksData.breaks[i].message,
						TIMER_INITIATE_TIMED_LOCK,
						i
						);
				}
				catch (CronSchedule::bad_crontab &)
				{
					continue; // ignore invalid crontabs
				}
				
				valid += 1;
			}

			if (valid > 0)
			{
				scheduler->addSchedule(timedBreaks);
			}
			else
			{
				delete timedBreaks;
			}
		}

		m_pState = new IdleState(this, scheduler);
		//m_pState = new AlertState(this, scheduler);

		//IdleState(this, scheduler);
		//m_pState = new LockState(this, scheduler);
	}
}

void AngelicMachine::suspend(SimpleScheduler *scheduler, bool suspend)
{
	if (suspend)
	{
		terminate(scheduler);
		ATLTRACE(_T("Angelic Machine: suspended\n"));
	}
	else
	{
		start(scheduler);
		ATLTRACE(_T("Angelic Machine: resumed\n"));
	}
}

void AngelicMachine::userEvent(SimpleScheduler *scheduler, int event)
{
	if (m_pState != NULL)
	{
		m_pState->userEvent(this, scheduler, event);
	}
}

void AngelicMachine::timerEvent(SimpleScheduler *scheduler, int event, long data)
{
	if (m_pState != NULL)
	{
		m_pState->timerEvent(this, scheduler, event, data);
	}
}

void AngelicMachine::terminate(SimpleScheduler *scheduler)
{
	if (m_pState != NULL)
	{
		scheduler->chancelSchedule(PERIODIC_TASK_NAME);

		m_pState->terminate(this, scheduler);

		delete m_pState;
		m_pState = NULL;
	}
}

AbstractMachine::flag_t AngelicMachine::flags()
{
	if (m_pState != NULL)
	{
		return m_pState->flags(this);
	}

	return USER_TERMINATE_FLAG;
}

bool AngelicMachine::allowRestrictions(int event)
{
	if (m_pState != NULL)
	{
		return m_pState->allowRestrictions(this, event);
	}

	return false;
}

void AngelicMachine::setWorkArea(const CRect &workArea)
{
	m_workArea = workArea;
}

const CRect &AngelicMachine::getWorkArea()
{
	return m_workArea;
}
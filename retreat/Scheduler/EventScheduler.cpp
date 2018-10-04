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
#include "EventScheduler.h"

#include <time.h>
#include <process.h>

#include <list>

using namespace std;
using namespace boost;

// EventScheduler

///////////////////////////////////////////////////////////////////////////////

// timer thread
unsigned __stdcall EventScheduler::timerThread(void *param)
{
	EventScheduler *pThis = static_cast<EventScheduler *>(param);

	pThis->startTimer(pThis->m_hTimer, EVENT_SCHEDULER_TIMER_PERIOD);
	HANDLE handles[2] = {pThis->m_hTimer, pThis->m_hTimerEvent};

	while (true)
	{
		if (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
		{	
			time_t current_time;
			time(&current_time);
			
			EnterCriticalSection(&pThis->m_scheduleCS); // <<<

			callback_map_t::const_iterator callback_it = pThis->m_callbacks.begin();

			// iterate callbacks
			for (; callback_it != pThis->m_callbacks.end(); ++callback_it)
			{
				list<tstring> to_delete;			
				schedule_map_ptr_t p_schedule_map = callback_it->second;
				schedule_map_t::iterator schedule_it = p_schedule_map->begin();

				// iterate schedules
				for (; schedule_it != p_schedule_map->end(); ++schedule_it)
				{
					Event event;
					schedule_ptr_t &p_schedule = schedule_it->second;

					if (p_schedule->isReady(&current_time, event))
					{
						callback_it->first->eventRaised(&current_time, event);

						if (p_schedule->isDisposable()) // collect disposable schedules
						{
							to_delete.push_back(p_schedule->getName());
						}
					}
				}

				// remove disposable schedules
				list<tstring>::iterator disposable_it = to_delete.begin();
				for (; disposable_it != to_delete.end(); ++disposable_it)
				{
					p_schedule_map->erase(*disposable_it);
				}
			}

			LeaveCriticalSection(&pThis->m_scheduleCS); // >>>
		}
		else 
			break;
	}

	return 0;
}

HANDLE EventScheduler::createTimer()
{
	HANDLE timer = NULL;

	timer = CreateWaitableTimer(NULL, FALSE, NULL);

	if (!timer)
	{
		return NULL;
	}

	return timer;
}

void EventScheduler::startTimer(HANDLE timer, long period)
{
	LARGE_INTEGER timerExpires;

	timerExpires.QuadPart = Int32x32To64(-10000, period);

	SetWaitableTimer(
		timer,
		&timerExpires,
		period,
		NULL,
		NULL,
		FALSE);
}

void EventScheduler::stopTimer(HANDLE timer)
{
	CancelWaitableTimer(timer);
}

void EventScheduler::freeTimer(HANDLE timer)
{
	CloseHandle(timer);
}

///////////////////////////////////////////////////////////////////////////////

EventScheduler::EventScheduler(): 
m_hTimerThread(NULL)
{
	InitializeCriticalSection(&m_scheduleCS);

	m_hTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTimer = createTimer();
}

EventScheduler::~EventScheduler()
{
	stopScheduling();
	freeTimer(m_hTimer);
	CloseHandle(m_hTimerEvent);
	DeleteCriticalSection(&m_scheduleCS);
}

bool EventScheduler::startScheduling()
{
	if (!m_hTimerThread)
	{
		m_hTimerThread = 
			(HANDLE)_beginthreadex(NULL, 0, timerThread, (void *)this, 0, NULL);
		
		return true;
	}
	
	return false;
}

void EventScheduler::stopScheduling()
{
	if (m_hTimerThread)
	{
		// stop the timer and wait while timer thread finishes
		stopTimer(m_hTimer);
		SetEvent(m_hTimerEvent);
		WaitForSingleObject(m_hTimerThread, INFINITE);
		CloseHandle(m_hTimerThread);
		m_hTimerThread = NULL;
	}
}

bool EventScheduler::addSchedule(SchedulerCallback *callback, Schedule *schedule)
{
	EnterCriticalSection(&m_scheduleCS); // <<<

	callback_map_t::iterator callback_it = m_callbacks.find(callback);

	// if there are no such callback, insert it
	if (callback_it == m_callbacks.end())
	{
		callback_pair_t callback_pair;
		schedule_map_ptr_t p_schedule_map(new schedule_map_t);

		callback_pair.first = callback;
		callback_pair.second = p_schedule_map;

		std::pair<callback_map_t::iterator, bool> it_pair = 
			m_callbacks.insert(callback_pair);
		callback_it = it_pair.first;
	}

	// add schedule
	schedule_map_ptr_t p_schedule_map = callback_it->second;

	schedule_pair_t schedule_pair;
	schedule_pair.first = schedule->getName();
	// create new instance of schedule and copy contains from parameter
	// and wrap it into smart pointer
	schedule_pair.second = schedule_ptr_t(schedule);

	p_schedule_map->insert(schedule_pair);

	LeaveCriticalSection(&m_scheduleCS); // >>>

	return true;
}

void EventScheduler::adjustSchedules(SchedulerCallback *callback, int value)
{
	class AdjustAction
	{
	public:
		int value_;

		AdjustAction(int value): value_(value) {}
		void operator()(SchedulerCallback *callback, schedule_ptr_t &schedule)
		{
			if (schedule->isAdjustable())
			{
				schedule->adjust(value_);
			}
		}
	} action(value);

	EnterCriticalSection(&m_scheduleCS); // <<<
	iterateSchedules(m_callbacks, action);
	LeaveCriticalSection(&m_scheduleCS); // >>>
}

// remove schedule from callback container
void EventScheduler::chancelSchedule(SchedulerCallback *callback, const tstring &name)
{
	EnterCriticalSection(&m_scheduleCS); // <<<
	
	callback_map_t::iterator callback_it = m_callbacks.find(callback);

	if (callback_it != m_callbacks.end())
	{
		schedule_map_ptr_t p_schedule_map = callback_it->second;

		p_schedule_map->erase(name);
	}

	LeaveCriticalSection(&m_scheduleCS); // >>>
}

void EventScheduler::detachCallback(SchedulerCallback *callback)
{
	EnterCriticalSection(&m_scheduleCS); // <<<

	m_callbacks.erase(callback);

	LeaveCriticalSection(&m_scheduleCS); // >>>
}
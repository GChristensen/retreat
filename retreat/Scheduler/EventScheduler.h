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

#include <windows.h>

#include <map>
#include <boost/shared_ptr.hpp>

#include "tstring.h"
#include "Schedule.h"
#include "SchedulerCallback.h"

#define EVENT_SCHEDULER_TIMER_PERIOD 1000

class EventScheduler
{
public:
	EventScheduler();
	~EventScheduler();

	bool startScheduling();
	void stopScheduling();

	// currently faith of callback isn't managed
	// TODO?
	bool addSchedule(SchedulerCallback *callback, Schedule *schedule);

	void adjustSchedules(SchedulerCallback *callback, int value);

	void chancelSchedule(SchedulerCallback *callback, const tstring &name);
	void detachCallback(SchedulerCallback *callback);

protected:

	typedef boost::shared_ptr<Schedule> schedule_ptr_t;
	typedef std::pair<tstring, schedule_ptr_t> schedule_pair_t;
	typedef std::map<tstring, schedule_ptr_t> schedule_map_t;

	typedef boost::shared_ptr<schedule_map_t> schedule_map_ptr_t;
	typedef std::pair<SchedulerCallback *, schedule_map_ptr_t> callback_pair_t;
	typedef std::map<SchedulerCallback *, schedule_map_ptr_t> callback_map_t;

	callback_map_t m_callbacks;

	template <class T>
	static void iterateSchedules(const callback_map_t &callbacks, T &action)
	{
		callback_map_t::const_iterator callback_it = callbacks.begin();

		for (; callback_it != callbacks.end(); ++callback_it)
		{
			schedule_map_ptr_t p_schedule_map = callback_it->second;
			schedule_map_t::iterator schedule_it = p_schedule_map->begin();

			for (; schedule_it != p_schedule_map->end(); ++schedule_it)
			{
				action(callback_it->first, schedule_it->second);
			}
		}
	}

	// timer related stuff
	HANDLE m_hTimer;
	HANDLE m_hTimerEvent;
	HANDLE m_hTimerThread;

	HANDLE createTimer();
	void startTimer(HANDLE timer, long period);
	void stopTimer(HANDLE timer);
	void freeTimer(HANDLE timer);
	static unsigned __stdcall timerThread(void *param);

	CRITICAL_SECTION m_scheduleCS;

};
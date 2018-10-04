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

#include "windows.h"

#include "EventScheduler.h"

// class wraps EventScheduler to simplify dealing with callbacks
// client sees no callbacks, instead messages will be sent to specified window
// when event fires

class MessageScheduler: public EventScheduler
{
	static const int EVENT_SIMPLE_SCHEDULER_CONTROL_WM = WM_USER + 328;
public:

	static const int WM_DISPATCH_EVENT = EVENT_SIMPLE_SCHEDULER_CONTROL_WM + 1;

	MessageScheduler(HWND targetWindow);
	~MessageScheduler();

	bool addSchedule(int dispatchIndex, Schedule *schedule);
	void chancelSchedule(int dispatchIndex, const tstring &name);

	void adjustSchedules(int dispatchIndex, int value);
	void releaseSchedules(int dispatchIndex);

protected:

	HWND m_targetWindow;

	class DispatchCallback: public SchedulerCallback
	{
	public:
		DispatchCallback(int dispatchIndex, HWND targetWindow): 
		  m_dispatchIndex(dispatchIndex),
		  m_targetWnd(targetWindow)
		  {}

		virtual void eventRaised(const time_t* now, const Event &event)
		{
			DWORD dwDispInfo = MAKEWPARAM(event.getID(), m_dispatchIndex);

			::PostMessage(m_targetWnd, WM_DISPATCH_EVENT, dwDispInfo,
				event.getData());
		}

	private:
		int m_dispatchIndex;
		HWND m_targetWnd;
	};

	typedef boost::shared_ptr<DispatchCallback> disp_callback_ptr_t;
	typedef std::pair<int, disp_callback_ptr_t> disp_callback_pair_t;
	typedef std::map<int, disp_callback_ptr_t> disp_callback_map_t;

	disp_callback_map_t m_dispIndexCallbackMap;

	template <class T>
	bool addSchedule(SchedulerCallback *callback, T &schedule);
	void adjustSchedules(SchedulerCallback *callback, int value);
	void chancelSchedule(SchedulerCallback *callback, const tstring &name);
	void detachCallback(SchedulerCallback *callback);
};


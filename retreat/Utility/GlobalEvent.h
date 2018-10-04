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
#include "../AngelicMachine/AngelicMachine.h"
#include "../Scheduler/SimpleScheduler.h"

#define EVENT_SCHEDULER_TIMER_PERIOD 1000

class GlobalEvent
{
public:
	GlobalEvent(const tstring &event_name, AngelicState *state, AngelicMachine *machine, 
		SimpleScheduler *scheduler, int event, DWORD timeout);
	~GlobalEvent();

	void cancel();
protected:
	AngelicState *m_state;
	AngelicMachine *m_machine;
	SimpleScheduler *m_scheduler;
	int m_event;
	DWORD m_timeout;

	HANDLE m_hEventThread;
	HANDLE m_hGlobalEvent;
	HANDLE m_hInterruptEvent;
	static unsigned __stdcall eventThread(void *param);
};
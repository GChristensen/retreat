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

#include "../scheduler/scheduler.h"
#include "../utility/SystemHelper.h"

#include "IdleState.h"
#include "AlertState.h"
#include "ListenActivityState.h"

#define LISTEN_ACTIVITY_TASK_NAME _T("AngelicMachine:ListenActivity")

ListenActivityState::ListenActivityState
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler
	)
{
	SystemHelper::getInstance()->startActivityMonitoring();

	int aletrBeforeMin = behaviour(machine).AlertBeforeMin;

	QueueSchedule *endListening = new QueueSchedule(
		tstring(LISTEN_ACTIVITY_TASK_NAME),
		aletrBeforeMin * 60,
		aletrBeforeMin,
		true,
		false
		);

	Event event(AngelicMachine::TIMER_FINISH_ACTIVITY_LISTENING);
	event.setPoint(aletrBeforeMin);
	endListening->addEvent(event);

	scheduler->addSchedule(endListening);

	ATLTRACE("Angelic machine: listening activity\n");
}

ListenActivityState::~ListenActivityState()
{
}

void ListenActivityState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long
	)
{
	switch (event)
	{
	case AngelicMachine::TIMER_FINISH_ACTIVITY_LISTENING:
		{
			if (SystemHelper::getInstance()->finishActivityMonitoring() == 1)
			// activity presents
			{
				setState(machine, new AlertState(machine, scheduler));
			}
			else
			// no user activity
			{
				setState(machine, new IdleState(machine, scheduler));
			}
		}
		break;
	}
}

void ListenActivityState::terminate
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler
	)
{
	scheduler->chancelSchedule(LISTEN_ACTIVITY_TASK_NAME);
	SystemHelper::getInstance()->finishActivityMonitoring();

	ATLTRACE("Angelic Machine: terminate listening\n");
}
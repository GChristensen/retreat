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
#include "DelayState.h"
#include "AlertState.h"

#include "../scheduler/scheduler.h"

#define DELAY_TASK_NAME _T("AngelicMachine:Delay")

DelayState::DelayState(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	int delayTimeMin = behaviour(machine).DelayTimeMin;

	QueueSchedule *interruptDelay = new QueueSchedule(
		tstring(DELAY_TASK_NAME),
		delayTimeMin * 60,
		delayTimeMin,
		true,
		false
		);

	Event event(AngelicMachine::TIMER_INITIATE_LOCK);
	event.setPoint(delayTimeMin);
	interruptDelay->addEvent(event);
	
	scheduler->addSchedule(interruptDelay);

	ATLTRACE("Angelic Machine: delayed\n");
}

DelayState::~DelayState()
{
}

void DelayState::userEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event
	)
{

}

void DelayState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long
	)
{
	switch (event)
	{
	case AngelicMachine::TIMER_INITIATE_LOCK:
		setState(machine, new AlertState(machine, scheduler));
		break;
	}
}

void DelayState::terminate(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	scheduler->chancelSchedule(DELAY_TASK_NAME);
}

AbstractMachine::flag_t DelayState::flags(AngelicMachine *machine)
{
	return 0;
}
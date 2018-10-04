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
#include "DisableState.h"
#include "IdleState.h"

#include "../scheduler/scheduler.h"
#include "../utility/SystemHelper.h"

#define DISABLE_TASK_NAME _T("AngelicMachine:Enable")

DisableState::DisableState(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	int enableAfterMin = behaviour(machine).AutomaticallyEnableAfterMin;

	QueueSchedule *enableSchedule = new QueueSchedule(
		DISABLE_TASK_NAME,
		enableAfterMin * 60,
		enableAfterMin,
		true,
		false
		);

	Event event(AngelicMachine::TIMER_RETURN_IDLE_STATE);
	event.setPoint(enableAfterMin);
	enableSchedule->addEvent(event);

	scheduler->addSchedule(enableSchedule);

	SystemHelper::getInstance()->minimizeWorkingSet();

	ATLTRACE("Angelic Machine: disabled\n");
}

DisableState::~DisableState()
{
}

void DisableState::userEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event
	)
{
	switch (event)
	{
	case AngelicMachine::USER_DISABLE:
		{
			terminate(machine, scheduler);
			setState(machine, new IdleState(machine, scheduler));
		}
		break;
	}
}

void DisableState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long
	)
{
	switch (event)
	{
	case AngelicMachine::TIMER_RETURN_IDLE_STATE:
		{
			setState(machine, new IdleState(machine, scheduler));
		}
		break;
	}
}

void DisableState::terminate(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	scheduler->chancelSchedule(DISABLE_TASK_NAME);
}

AbstractMachine::flag_t DisableState::flags(AngelicMachine *machine)
{
	return 
		  AngelicState::flags(machine) 
		& ~AngelicMachine::USER_LOCK_FLAG
		| AngelicMachine::STATE_DISABLED_FLAG;
}
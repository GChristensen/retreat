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

#include "IdleState.h"
#include "AlertState.h"
#include "ListenActivityState.h"
#include "DisableState.h"
#include "LockState.h"

#include "../utility/SystemHelper.h"

IdleState::IdleState()
{
	ATLTRACE("Angelic Machine Idle ancestor\n");
}

IdleState::IdleState(AngelicMachine *machine, SimpleScheduler *scheduler)
{
	setCell(machine, DELAY_COUNTER_CELL, (int)0);
	setCell(machine, BREAK_DURATION_CELL, (int)1);

	if (!isCellSet(machine, SKIP_BREAK_FLAG_CELL))
	{
		setCell(machine, SKIP_BREAK_FLAG_CELL, false);
	}

	SystemHelper::getInstance()->minimizeWorkingSet();

	ATLTRACE("Angelic Machine: idle\n");
}

IdleState::~IdleState()
{
}

void IdleState::userEvent
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
			setState(machine, new DisableState(machine, scheduler));
		}
		break;
	case AngelicMachine::USER_LOCK:
		{
			terminate(machine, scheduler);
			setCell(machine, BREAK_DURATION_CELL, 
				behaviour(machine).BreakLengthMin);
			setState(machine, new LockState(machine, scheduler));
		}
		break;
	}
}

void IdleState::timerEvent
	(
	 AngelicMachine *machine, 
	 SimpleScheduler *scheduler, 
	 int event,
	 long data
	)
{
	bool timedBreak = false;

	switch (event)
	{
	case AngelicMachine::TIMER_START_LISTENING_USER:
		{
			setCell(machine, BREAK_DURATION_CELL, 
				behaviour(machine).BreakLengthMin);
			setState(machine, new ListenActivityState(machine, scheduler));
		}
		break;
	case AngelicMachine::TIMER_INITIATE_TIMED_LOCK:
		{
			timedBreak = true;
			setCell(machine, BREAK_DURATION_CELL,
				timed_breaks(machine).breaks[data].duration);
			setCell(machine, TIMED_BREAK_MESSAGE,
				timed_breaks(machine).breaks[data].message);
		}
	case AngelicMachine::TIMER_START_ALERT:
		{
			if (!timedBreak)
			{
				setCell(machine, BREAK_DURATION_CELL, 
					behaviour(machine).BreakLengthMin);
			}
			setState(machine, new AlertState(machine, scheduler));
		}
		break;
	}
}

void IdleState::terminate(AngelicMachine *machine, SimpleScheduler *scheduler)
{
}

bool IdleState::allowRestrictions(AngelicMachine *machine, int event)
{
	return behaviour(machine).UseRestrictionList;
}
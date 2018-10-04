
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
#include "MeanderSchedule.h"

MeanderSchedule::MeanderSchedule
	(tstring name, int ticks, Event &regular, Event &last):
Schedule(name, false, false),
m_regularEvent(regular),
m_lastEvent(last), 
m_ticks(ticks)
{
}

MeanderSchedule::~MeanderSchedule()
{
}

void MeanderSchedule::adjust(int seconds)
{
}

bool MeanderSchedule::isReady(const time_t* now, Event &event)
{
	m_ticks -= 1;

	if (m_ticks == 0)
	{
		event = m_lastEvent;
	}
	else if (m_ticks > 0)
	{
		event = m_regularEvent;
	}
	else
	{
		return false;
	}

	return true;
}

bool MeanderSchedule::isDisposable()
{
	return m_ticks == 0;
}

bool MeanderSchedule::isAdjustable() 
{
	return false;
}
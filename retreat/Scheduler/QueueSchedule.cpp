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

#include <cassert>

#include "QueueSchedule.h"

QueueSchedule::QueueSchedule()
{
	m_points = 0;
	m_pointValue = 0;
	m_counter = 0;
	m_evetsRaised = 0;
}

QueueSchedule::QueueSchedule
	(
	 const tstring &name,
	 int interval,
	 int points,
	 bool disposable,
	 bool adjustable
	):
Schedule(name, disposable, adjustable)
{
	// adjustable task can not be disposable
	assert(!(disposable && adjustable));

	if (points == 0)
	{
		throw _T("Point amount should be greater than zero");
	}

	m_points = points;

	// period duration in seconds
	m_periodDuration = interval;
	// point value in seconds
	m_pointValue = interval / points;

	// actually there will be no tick with 0 index, 
	// tics with indexes 1..points are valid
	m_counter = 0;
	m_evetsRaised = 0;
}

QueueSchedule::~QueueSchedule()
{

}

void QueueSchedule::adjust(int seconds)
{
	m_counter = seconds;
}

void QueueSchedule::addEvent(Event &event)
{
	m_events.push_back(event);
}

// currently if there are several events with same point value
// only firstly inserted event will be selected
bool QueueSchedule::isReady(const time_t* now, Event &event)
{	
	// imagine - we have interval duration = 3 sec and only one point
	// first and only point should fire when 3 seconds have passed from
	// previous tick
	// but if segment will be eq 0 tick potentially also could fire, 
	// we should prohibit this
	int segment = (m_counter % m_periodDuration) + 1;

	m_counter += 1;

	if (segment % m_pointValue == 0)
	{
		std::vector<Event>::iterator event_it = m_events.begin();
		for (; event_it != m_events.end(); ++event_it)
		{
			if (event_it->havePoint(segment / m_pointValue))
			{
				event = *event_it;
				m_evetsRaised += 1;
				return true;
			}
		}
	}

	return false;
}

bool QueueSchedule::isDisposable()
{
	return m_isDisposable && m_evetsRaised >= m_events.size();
}
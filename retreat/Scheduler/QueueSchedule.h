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

#include <vector>

#include "Schedule.h"

// how queue schedule works

// client must specify period length in seconds trough constructor *interval* 
// parameter
// *points* parameter specifies amount of ticks (points) by period 
// will be divided

// for example, if 3600 period length and 60 ticks were specified
// then one hour period will be divided by 60 ticks (1 point value = 
// 3600/60 = 60 sec ie event could fire every minute)

// client can add several events to schedule 
// different points (ticks) could be specified for each event
// appropriate event will be raised on corresponding tick 

class QueueSchedule: public Schedule
{
public:

	QueueSchedule();
	QueueSchedule(
		const tstring &name,
		int interval,
		int points,
		bool disposable = false,
		bool adjustable = true
		);
	~QueueSchedule();

	virtual void QueueSchedule::adjust(int seconds);
	virtual bool isReady(const time_t* now, Event &event);
	virtual bool isDisposable();

	void addEvent(Event &event);

private:

	int m_points;

	long m_counter;
	long m_pointValue;
	long m_periodDuration;

	std::vector<Event> m_events;

	unsigned m_evetsRaised;
};

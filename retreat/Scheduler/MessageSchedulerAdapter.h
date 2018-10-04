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

#include "SimpleScheduler.h"
#include "QueueSchedule.h"
#include "MeanderSchedule.h"

class MessageSchedulerAdapter: public SimpleScheduler
{
public:
	MessageSchedulerAdapter
		(
		 MessageScheduler &messageScheduler, 
		 int dispatchIndex
		 ):
	m_messageScheduler(messageScheduler),
	m_dispatchIndex(dispatchIndex)
	{}

	operator SimpleScheduler *()
	{
		return this;
	}

	bool addSchedule(Schedule *schedule)
	{
		return m_messageScheduler.addSchedule(m_dispatchIndex, schedule);
	}

	void chancelSchedule(const tstring &name)
	{
		m_messageScheduler.chancelSchedule(m_dispatchIndex, name);
	}

	void adjustSchedules(int value)
	{
		m_messageScheduler.adjustSchedules(m_dispatchIndex, value);
	}

	void releaseSchedules()
	{
		m_messageScheduler.releaseSchedules(m_dispatchIndex);
	}

private:

	MessageScheduler &m_messageScheduler;
	int m_dispatchIndex;

};
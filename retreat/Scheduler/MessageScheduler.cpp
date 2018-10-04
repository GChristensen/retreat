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
#include "MessageScheduler.h"

MessageScheduler::MessageScheduler(HWND targetWindow):
EventScheduler(),
m_targetWindow(targetWindow)
{
}

MessageScheduler::~MessageScheduler()
{
}

bool MessageScheduler::addSchedule(int dispatchIndex, Schedule *schedule)
{
	disp_callback_map_t::iterator callback_it = 
		m_dispIndexCallbackMap.find(dispatchIndex);

	// if there is no such callback, insert it
	if (callback_it == m_dispIndexCallbackMap.end())
	{
		disp_callback_pair_t callback_pair;
		disp_callback_ptr_t p_disp_callback(
			new DispatchCallback(dispatchIndex, m_targetWindow));

		callback_pair.first = dispatchIndex;
		callback_pair.second = p_disp_callback;

		std::pair<disp_callback_map_t::iterator, bool> it_pair = 
			m_dispIndexCallbackMap.insert(callback_pair);
		callback_it = it_pair.first;
	}

	return EventScheduler::addSchedule(callback_it->second.get(), schedule);
}

void MessageScheduler::chancelSchedule
	(
	 int dispatchIndex,
	 const tstring &name
	)
{
	disp_callback_map_t::iterator callback_it = 
		m_dispIndexCallbackMap.find(dispatchIndex);

	if (callback_it != m_dispIndexCallbackMap.end())
	{
		EventScheduler::chancelSchedule(callback_it->second.get(), name);
	}
}

void MessageScheduler::adjustSchedules(int dispatchIndex, int value)
{
	disp_callback_map_t::iterator callback_it = 
		m_dispIndexCallbackMap.find(dispatchIndex);

	if (callback_it != m_dispIndexCallbackMap.end())
	{
		EventScheduler::adjustSchedules(callback_it->second.get(), value);
	}
}

void MessageScheduler::releaseSchedules(int dispatchIndex)
{
	disp_callback_map_t::iterator callback_it = 
		m_dispIndexCallbackMap.find(dispatchIndex);

	if (callback_it != m_dispIndexCallbackMap.end())
	{
		EventScheduler::detachCallback(callback_it->second.get());
	}

	m_dispIndexCallbackMap.erase(callback_it);
}
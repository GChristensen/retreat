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

//#include <set>
#include <cassert>

class Event
{
public:
	Event(): m_eventID(0), m_data(0) {}
	Event(int id): m_eventID(id), m_data(0) {}
	Event(int id, long data): m_eventID(id), m_data(data) {}

	int getID() const {return m_eventID;}

	void setData(long data) {m_data = data;}
	long getData() const {return m_data;}

	//void addPoint(long point)
	void setPoint(long point) 
	{
		assert(point != 0);

		//m_points.insert(point);
		m_point = point;
	}

	bool havePoint(long point)
	{
		//std::set<long>::iterator point_it = m_points.find(point);
		//return point_it != m_points.end();
		return m_point == point;
	}

private:

	int m_eventID;
	long m_data;
	
	// several points are not used currently, set gives overhead therefore
	//std::set<long> m_points; 

	long m_point;

};
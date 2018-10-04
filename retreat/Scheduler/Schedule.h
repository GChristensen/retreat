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

#include "tstring.h"
#include "Event.h"

class Schedule
{
public:

	Schedule(): m_isDisposable(false), m_isAdjustable(true) {}
	Schedule(const tstring &name, bool disposable, bool adjustable): 
			m_name(name), m_isDisposable(disposable), m_isAdjustable(adjustable) 
			{}
	
	tstring &getName() {return m_name;}

	virtual void adjust(int seconds) = 0;
	virtual bool isReady(const time_t* now, Event &event) = 0;
	
	virtual bool isDisposable() {return m_isDisposable;}
	virtual bool isAdjustable() {return m_isAdjustable;}

protected:

	tstring m_name;
	
	bool m_isDisposable;
	bool m_isAdjustable;
};
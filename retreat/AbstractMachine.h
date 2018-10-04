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
#include "SettingTable.h"
#include "scheduler/SimpleScheduler.h"

class AbstractMachine
{
public:

	AbstractMachine(const tstring &name):
	m_machineName(name)
	{}

	virtual ~AbstractMachine() {}

	const tstring &name()
	{
		return m_machineName;
	}

	virtual void readSettings(SettingTable *settings) = 0;
	virtual void writeSettings(SettingTable *settings) = 0;

	virtual void start(SimpleScheduler *scheduler) = 0;
	virtual void suspend(SimpleScheduler *scheduler, bool suspend) = 0;

	virtual void userEvent(SimpleScheduler *scheduler, int event) = 0;
	virtual void timerEvent(SimpleScheduler *scheduler, int event, long data) = 0;
	virtual void terminate(SimpleScheduler *scheduler) = 0;

	typedef unsigned long flag_t;

	virtual flag_t flags() = 0;
	
	// true indicates that specified *event* should obey global restrictions
	// if such restrictions are present, event will be suppressed
	// and machine will not change it's state
	virtual bool allowRestrictions(int event) = 0;

protected:
	
	tstring m_machineName;
};
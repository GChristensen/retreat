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

#include <map>

#include "boost/any.hpp"

#include <atlmisc.h>

#include "../AbstractMachine.h"
#include "../scheduler/SimpleScheduler.h"

#include "AngelicSettings.h"

#define WM_DISPLAY_GLOBAL_MESSAGE WM_USER + 400

class AngelicState;

class AngelicMachine: public AbstractMachine
{
	friend class AngelicState;
public:
	AngelicMachine(const tstring &name, HWND parent);
	virtual ~AngelicMachine();

	enum STATE_TRANSITIONS
	{
		USER_LOCK,
		USER_SKIP, 
		USER_DELAY,
		USER_UNLOCK,
		USER_DISABLE,
		TIMER_VOID_EVENT,
		TIMER_START_ALERT,
		TIMER_INITIATE_LOCK,
		TIMER_RETURN_IDLE_STATE,
		TIMER_ANGELIC_WND_NOTIFY,
		TIMER_INITIATE_TIMED_LOCK,
		TIMER_START_LISTENING_USER,
		TIMER_FINISH_ACTIVITY_LISTENING
	};

	static const flag_t USER_DISABLE_FLAG	= 0x1;
	static const flag_t USER_DELAY_FLAG		= 0x2;
	static const flag_t USER_SKIP_FLAG		= 0x4;
	static const flag_t USER_LOCK_FLAG		= 0x8;
	static const flag_t USER_OPTIONS_FLAG	= 0x10;
	static const flag_t USER_TERMINATE_FLAG	= 0x20;
	static const flag_t STATE_DISABLED_FLAG	= 0x40;

	HWND m_parentWindow;
	
	virtual void readSettings(SettingTable *settings);
	virtual void writeSettings(SettingTable *settings);

	virtual void start(SimpleScheduler *scheduler);
	virtual void suspend(SimpleScheduler *scheduler, bool suspend);

	virtual void userEvent(SimpleScheduler *scheduler, int event);
	virtual void timerEvent(SimpleScheduler *scheduler, int event, long data);
	virtual void terminate(SimpleScheduler *scheduler);

	virtual flag_t flags();

	virtual bool allowRestrictions(int event);

	void setWorkArea(const CRect &workArea);
	const CRect &getWorkArea();

private:

	// global storage for states associated with machine instance
	typedef std::map<int, boost::any> cell_map_t;
	cell_map_t m_cellMap;

	bool isCellSet(int cell)
	{
		return m_cellMap.find(cell) != m_cellMap.end();
	}

	boost::any &getCell(int cell)
	{
		return m_cellMap[cell];
	}

	void setCell(int cell, const boost::any &value)
	{
		m_cellMap[cell] = value;
	}

	void setState(AngelicState *newState)
	{
		m_pState = newState;	
	}

	AngelicState *m_pState;
	CRect m_workArea;

	AM_BEHAVIOUR_SETTINGS m_behaviourSettings;
	AM_APPEARANCE_SETTINGS m_appearanceSettings;
	AM_TIMED_BREAKS m_timedBreaksData;
};

class AngelicState
{
public:

	virtual ~AngelicState()
	{}

	virtual void userEvent(
		AngelicMachine *machine, SimpleScheduler *scheduler, int event) = 0;

	virtual void timerEvent(
		AngelicMachine *machine, SimpleScheduler *scheduler, int event, 
		long data) = 0;

	virtual void terminate(
		AngelicMachine *machine, SimpleScheduler *scheduler) = 0;

	virtual AbstractMachine::flag_t flags(AngelicMachine *machine)
	{
		return 
			  AngelicMachine::USER_DISABLE_FLAG
		//	| AngelicMachine::USER_DELAY_FLAG
		//	| AngelicMachine::USER_SKIP_FLAG
			| AngelicMachine::USER_LOCK_FLAG	
			| AngelicMachine::USER_OPTIONS_FLAG
			| AngelicMachine::USER_TERMINATE_FLAG;
	}

	virtual bool allowRestrictions(AngelicMachine *machine, int event)
	{
		return false;
	}

protected:

	enum CELLS
	{
		DELAY_COUNTER_CELL,
		BREAK_DURATION_CELL,
		SKIP_BREAK_FLAG_CELL,
		TIMED_BREAK_MESSAGE
	};

	bool isCellSet(AngelicMachine *machine, int cell)
	{
		return machine->isCellSet(cell);
	}

	boost::any &getCell(AngelicMachine *machine, int cell)
	{
		return machine->getCell(cell);
	}

	void setCell(AngelicMachine *machine, int cell, const boost::any &value)
	{
		machine->setCell(cell, value);
	}

	AM_BEHAVIOUR_SETTINGS &behaviour(AngelicMachine *machine)
	{
		return machine->m_behaviourSettings;
	}

	AM_APPEARANCE_SETTINGS &appearance(AngelicMachine *machine)
	{
		return machine->m_appearanceSettings;
	}

	AM_TIMED_BREAKS &timed_breaks(AngelicMachine *machine)
	{
		return machine->m_timedBreaksData;
	}

	// lethal method !
	void setState(AngelicMachine *machine, AngelicState *state)
	{
		machine->setState(state);
		delete this;
	}

	HWND getParentWindow(AngelicMachine *machine)
	{
		return machine->m_parentWindow;
	}

};

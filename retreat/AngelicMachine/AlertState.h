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

#include "boost/shared_ptr.hpp"

#include "AngelicMachine.h"
#include "AngelicWnd.h"

#include "../Utility/GlobalEvent.h"

class AlertState :public AngelicState
{
public:
	AlertState(AngelicMachine *machine, SimpleScheduler *scheduler);
	virtual ~AlertState();

	virtual void userEvent(
		AngelicMachine *machine, SimpleScheduler *scheduler, int event);

	virtual void timerEvent(
		AngelicMachine *machine, SimpleScheduler *scheduler, int event, long);

	virtual void terminate(
		AngelicMachine *machine, SimpleScheduler *scheduler);

	virtual AbstractMachine::flag_t flags(AngelicMachine *machine);

	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
										 LPRECT lprcMonitor, LPARAM dwData);

private:
	
	typedef boost::shared_ptr<CAngelicWnd> angelic_wnd_ptr_t;
	std::vector<angelic_wnd_ptr_t> m_pAngelicWnds;

	AngelicMachine *m_contextMachine;
	int m_alertBeforeSec;

	angelic_wnd_ptr_t
	createAngelicWindow(AngelicMachine *machine, CRect *rect);

	GlobalEvent *m_delayEvent;
	GlobalEvent *m_skipEvent;

};

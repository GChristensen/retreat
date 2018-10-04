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
#include <map>

#include "boost/shared_ptr.hpp"

#include "SettingTable.h"
#include "AbstractMachine.h"
#include "SettingsUIFactory.h"

#define RM_RESTRICTIONS_SECTION		_T("Restrictions")
#define RM_PROCESS_LIST_PREFIX		_T("ProcessList")
#define RM_PROCESS_LIST_LENGTH		_T("Length")
#define RM_PROCESS_LIST_ITEM		_T("item")

#define RM_BEHAVIOUR_PREFIX			_T("Behaviour")
#define RM_CHECK_FOR_ACTIVITY_BEFORE_MIN	\
	_T("BeQuietIfNoActivityDetectedBeforeMin")

#define rm_behaviour(name) \
	(tstring(RM_BEHAVIOUR_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define rm_proclist(name) \
	(tstring(RM_PROCESS_LIST_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

/* 
  To keep things enough clear and unified we need one more abstraction layer.
  MachineWrapper will aggregate AbstractMachine and perform translation
  of UI commands to machine user events, maintain global restrictions
  and UI items availability through machine flags.
*/

struct GLOBAL_RESTRICTIONS
{
	void read(SettingTable *settings);
	void write(SettingTable *settings);

	std::vector<tstring> processList;
	int CheckForUserActivityBeforeMin;
};

class MachineWrapper
{
public:

	MachineWrapper(AbstractMachine *machine, SettingsUIFactory *factory);
	~MachineWrapper();

	static void readGlobalRestrictions(SettingTable *settings);
	static bool checkGlobalRestrictions();

	void addUserCommand
		(
		 int command,
		 int event,
		 AbstractMachine::flag_t flag
		);
	bool commandAvaliable(int command);

	SettingsUIFactory *getUIFactory();

	const tstring &name();
	void readSettings(SettingTable *settings);
	void writeSettings(SettingTable *settings);

	void start(SimpleScheduler *scheduler);
	void suspend(SimpleScheduler *scheduler, bool suspend);

	void userEvent(SimpleScheduler *scheduler, int command);
	void timerEvent(SimpleScheduler *scheduler, int event, long data);
	void terminate(SimpleScheduler *scheduler);

	AbstractMachine::flag_t flags();

private:
	
	static GLOBAL_RESTRICTIONS g_restrictions;

	typedef boost::shared_ptr<AbstractMachine> abstract_machine_ptr_t;
	abstract_machine_ptr_t m_pMachine;

	struct COMMAND_IDENTITY
	{
		COMMAND_IDENTITY(): event(-1), flag(0)
		{}

		COMMAND_IDENTITY(int event_, AbstractMachine::flag_t flag_):
		event(event_), flag(flag_)
		{}

		int event;
		AbstractMachine::flag_t flag;
	};

	typedef std::map<int, COMMAND_IDENTITY> user_command_map_t;
	user_command_map_t m_userCommands;

	SettingsUIFactory *m_uiFactory;
};

typedef boost::shared_ptr<MachineWrapper> machinewp_ptr_t;
typedef std::vector<machinewp_ptr_t> machine_vector_t;
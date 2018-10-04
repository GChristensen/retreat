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

#include "stdafx.h"
#include "MachineWrapper.h"

#include <cassert>

#include "boost/lexical_cast.hpp"

#include "utility/SystemHelper.h"

GLOBAL_RESTRICTIONS MachineWrapper::g_restrictions;

void GLOBAL_RESTRICTIONS::read(SettingTable *settings)
{
	using boost::lexical_cast;

	CheckForUserActivityBeforeMin = 
		settings->getIntSetting(
		rm_behaviour(RM_CHECK_FOR_ACTIVITY_BEFORE_MIN));

	processList.clear();

	size_t procListLength = 
		settings->getIntSetting(rm_proclist(RM_PROCESS_LIST_LENGTH));

	processList.reserve(procListLength);

	for (size_t i = 0; i < procListLength; ++i)
	{
		tstring listItem = 
			rm_proclist(RM_PROCESS_LIST_ITEM) + lexical_cast<tstring>(i);

		tstring listItemValue = settings->getStringSetting(listItem);

		if (listItemValue.length() > 0)
		{
			processList.push_back(listItemValue);
		}
	}
}

void GLOBAL_RESTRICTIONS::write(SettingTable *settings)
{
	using boost::lexical_cast;

	settings->setIntSetting(
		rm_behaviour(RM_CHECK_FOR_ACTIVITY_BEFORE_MIN),
		CheckForUserActivityBeforeMin
		);

	settings->setIntSetting(
		rm_proclist(RM_PROCESS_LIST_LENGTH), processList.size());

	for (size_t i = 0; i < processList.size(); ++i)
	{
		tstring listItem = 
			rm_proclist(RM_PROCESS_LIST_ITEM) + lexical_cast<tstring>(i);

		settings->setStringSetting(listItem, processList[i]);
	}
}

MachineWrapper::MachineWrapper(AbstractMachine *machine, SettingsUIFactory *factory)
{
	m_pMachine = abstract_machine_ptr_t(machine);
	m_uiFactory = factory;
}

MachineWrapper::~MachineWrapper()
{
	delete m_uiFactory;
}

void MachineWrapper::readGlobalRestrictions(SettingTable *settings)
{
	g_restrictions.read(settings);
}

bool MachineWrapper::checkGlobalRestrictions()
{
	return SystemHelper::getInstance()->
		executableInMemory(g_restrictions.processList);
}

void MachineWrapper::addUserCommand(int command, int event, AbstractMachine::flag_t flag)
{
	m_userCommands[command] = COMMAND_IDENTITY(event, flag);
}

bool MachineWrapper::commandAvaliable(int command)
{
	user_command_map_t::iterator ci = m_userCommands.find(command);

	if (ci != m_userCommands.end())
	{
		assert(m_pMachine);
		return (m_pMachine->flags() & ci->second.flag) != 0;
	}

	return true;
}

SettingsUIFactory *MachineWrapper::getUIFactory()
{
	assert(m_uiFactory);
	return m_uiFactory;
}

const tstring & MachineWrapper::name()
{
	assert(m_pMachine);
	return m_pMachine->name();
}

void MachineWrapper::readSettings(SettingTable *settings)
{
	assert(m_pMachine);
	return m_pMachine->readSettings(settings);
}

void MachineWrapper::writeSettings(SettingTable *settings)
{
	assert(m_pMachine);
	return m_pMachine->writeSettings(settings);
}

void MachineWrapper::start(SimpleScheduler *scheduler)
{
	assert(m_pMachine);
	return m_pMachine->start(scheduler);
}

void MachineWrapper::suspend(SimpleScheduler *scheduler, bool suspend)
{
	assert(m_pMachine);
	return m_pMachine->suspend(scheduler, suspend);
}

void MachineWrapper::userEvent(SimpleScheduler *scheduler, int command)
{
	user_command_map_t::iterator ci = m_userCommands.find(command);

	if (ci != m_userCommands.end())
	{
		assert(m_pMachine);
		m_pMachine->userEvent(scheduler, ci->second.event);
	}
}

void MachineWrapper::timerEvent(SimpleScheduler *scheduler, int event, long data)
{
	assert(m_pMachine);

	if (m_pMachine->allowRestrictions(event))
	{
		if (checkGlobalRestrictions())
		{
			return;
		}
	}

	m_pMachine->timerEvent(scheduler, event, data);
}

void MachineWrapper::terminate(SimpleScheduler *scheduler)
{
	assert(m_pMachine);
	m_pMachine->terminate(scheduler);
}

AbstractMachine::flag_t MachineWrapper::flags()
{
	assert(m_pMachine);
	return m_pMachine->flags();
}

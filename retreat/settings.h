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

#include "SettingTable.h"
#include "SettingManager.h"

#define SM_GENERAL_SECTION		_T("General")
#define SM_LANGUAGE				_T("Language")
#define SM_LOCK					_T("Lock")

template<class stringT>
class tstring_to_char
{
public:
	const TCHAR *operator()(const stringT &s)
	{
		return s.c_str();
	}
};

typedef SettingManager<tstring, tstring_to_char<tstring> > setting_manager_t;

class SettingManagerAdapter: public SettingTable
{
public:
	SettingManagerAdapter(setting_manager_t &settings, const tstring &section):
	m_settings(settings),
	m_section(section)
	{
	}

	operator SettingTable *()
	{
		return this;
	}

	virtual int getIntSetting(const tstring &name);

	virtual tstring getStringSetting(const tstring &name)
	{
		if (!m_settings.isSettingExists(m_section, name))
		{
			m_settings.insertSetting(m_section, name, _T(""));
			m_settings.loadSetting(m_section, name);
		}
		return m_settings.get<tstring>(m_section, name);
	}

	virtual void setIntSetting(const tstring &name, int setting)
	{
		if (!m_settings.isSettingExists(m_section, name))
		{
			m_settings.insertSetting(m_section, name, 0);
		}
		m_settings.set<int>(m_section, name, setting);
	}

	virtual void setStringSetting(const tstring &name, const tstring &setting)
	{
		if (!m_settings.isSettingExists(m_section, name))
		{
			m_settings.insertSetting(m_section, name, _T(""));
		}
		m_settings.set<tstring>(m_section, name, setting);
	}

private:

	setting_manager_t &m_settings;
	tstring m_section;
};

void InitializeSettingManager(setting_manager_t &settings);
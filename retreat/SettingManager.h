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

#include "boost\any.hpp"
#include "boost\smart_ptr.hpp"
#include "windows.h"

#include <map>

template<class stringT, class string_to_charF>
class SettingManager;

template<class stringT>
class dumb_string_to_char
{
public:
	const TCHAR *operator()(const stringT &s)
	{
		return s;
	}
};

template<class stringT, class string_to_charF = dumb_string_to_char<stringT> >
class SettingInsertionHelper
{
public:

	SettingInsertionHelper
			(SettingManager<stringT, string_to_charF> *parent_, 
			 const stringT &section_): 
	section(section_),
	parent(parent_)
	{
	}

	SettingInsertionHelper<stringT, string_to_charF> &
	operator()
			(stringT settingName, stringT stringValue)
	{

		parent->insertSetting(section, settingName, stringValue);
		return *this;
	}

	SettingInsertionHelper<stringT, string_to_charF> &
	operator()
			(stringT settingName, int intValue, bool hexFlag = false)
	{

		parent->insertSetting(section, settingName, intValue);
		return *this;
	}

private:
	stringT section;
	SettingManager<stringT, string_to_charF> *parent;
};

template<class stringT, class string_to_charF = dumb_string_to_char<stringT> >
class SettingManager
{
public:

	typedef std::map<stringT, boost::any> setting_map;
	typedef typename setting_map::value_type setting_pair;

	SettingManager()
	{
	}

	SettingManager(const stringT &settingFile):
		m_settingFile(settingFile)
	{
	}

	~SettingManager(void)
	{
	}

	void setSettingsFile(const stringT &settingFile)
	{
		m_settingFile = settingFile;
	}

	void insertSetting(const stringT &section, const stringT &settingName, 
								 const stringT &stringValue)
	{
		setting_map_ptr settings = create_or_find_setting_map(m_sectionMap, section);
		
		settings->insert(setting_pair(settingName, 
							  boost::any(stringValue))
						  );
	}

	void insertSetting(const stringT &section, const stringT &settingName, 
							  int intValue, bool hexFlag = false)
	{
		setting_map_ptr settings = create_or_find_setting_map(m_sectionMap, section);

		if (hexFlag)
		{
			settings->insert(setting_pair(settingName, 
								boost::any(int_hex(intValue)))
							  );
		}
		else
		{
			settings->insert(setting_pair(settingName, 
								boost::any(intValue))
							  );
		}
	}

	SettingInsertionHelper<stringT, string_to_charF> insertSettings(const stringT &section)
	{
		SettingInsertionHelper<stringT, string_to_charF> insertion_helper(this, section);
		return insertion_helper;
	}

	void loadSettings()
	{
		section_map::iterator section = m_sectionMap.begin();
		setting_map::iterator setting;

		for(; section != m_sectionMap.end(); ++section)
		{
			setting = section->second->begin();
			
			for(; setting != section->second->end(); ++setting)
			{
				setting_pair pair;
				read_setting(m_settingFile, section->first, 
					setting->first, setting->second);
			}
		}
	}

	void loadSetting(const stringT &section, const stringT &setting)
	{
		section_map::iterator section_it = m_sectionMap.find(section);

		if (section_it != m_sectionMap.end())
		{
			setting_map_ptr settings = section_it->second;

			setting_map::iterator settings_it = settings->find(setting);

			if (settings_it != settings->end())
			{
				read_setting(m_settingFile, section, 
					setting, settings_it->second);
			}
		}
	}

	void loadSection(const stringT &section)
	{
		setting_map_ptr settings = create_or_find_setting_map(m_sectionMap, section);

		settings->clear();

		TCHAR *lpData;
		size_t nSize = SECTION_BUFFER_SIZE;
		DWORD dwResult; 

		string_to_charF to_char;

		do 
		{
			lpData = new TCHAR[nSize];

			dwResult = GetPrivateProfileSection(
				to_char(section),
				lpData,
				(DWORD)nSize,
				to_char(m_settingFile)
				);

			if (dwResult == nSize - 2)
			{
				delete [] lpData;

				nSize *= 2;
				lpData = new TCHAR[nSize];
			}
		}
		while (dwResult == nSize - 2);

		int pairLen;	
		TCHAR *pPair = lpData;

		while (true)
		{
			pairLen = (int)_tcslen(pPair);

			if (!pairLen)
				break;

			TCHAR* pEq = _tcschr(pPair, _T('='));

			if (pEq)
			{
				*pEq = 0;

				settings->insert(setting_pair(pPair, boost::any(stringT(pEq + 1))));
			}

			pPair += pairLen + 1;
		}

		delete [] lpData;
	}

	template<class F>
	F iterateSection(const stringT &section, F f)
	{
		section_map::iterator section_it = m_sectionMap.find(section);
		
		if (section_it != m_sectionMap.end())
		{
			setting_map_ptr p_setting_map = section_it->second;
			
			std::for_each(p_setting_map->begin(), p_setting_map->end(), f);
		}

		return f;
	}

	bool isSettingExists(const stringT &section, const stringT &setting)
	{
		section_map::iterator section_it = m_sectionMap.find(section);
		
		if (section_it != m_sectionMap.end())
		{
			setting_map_ptr settings = section_it->second;

			return settings->find(setting) != settings->end();
		}

		return false;
	}

	void saveSettings()
	{ 	
		section_map::iterator section = m_sectionMap.begin();
		setting_map::iterator setting;

		for(; section != m_sectionMap.end(); ++section)
		{
			setting = section->second->begin();
			
			for(; setting != section->second->end(); ++setting)
			{
				write_setting(m_settingFile, section->first, 
					setting->first, setting->second);
			}
		}
	}

	void saveSetting(const stringT &section, const stringT &name)
	{
		const boost::any &value = find_setting(section, name);
		
		assert(value.type() != typeid(void));

		write_setting(m_settingFile, section, name, value);
	}

	template<class X>
	const X get(const stringT &section, const stringT &name)
	{
		const boost::any &value = find_setting(section, name);
		
		assert(value.type() == typeid(X));

		if (value.type() == typeid(X))
			return boost::any_cast<X>(value);

		return X();
	}

	template<class X>
	const X get(const stringT &name)
	{
		section_map::iterator section_it = m_sectionMap.begin();
		
		for(; section_it != m_sectionMap.end(); ++section_it)
		{
			setting_map_ptr p_setting_map = section_it->second;
			
			setting_map::iterator setting = p_setting_map->find(name);

			if (setting != p_setting_map->end())
			{
				//_RPT1(1, "actual setting type is %s", setting->second.type().name());
				assert(setting->second.type() == typeid(X));
				
				if (setting->second.type() == typeid(X))
					return boost::any_cast<X>(setting->second);
			}
		}
		
		return X();
	}

	template<class X>
	void set(const stringT &section, const stringT &name, const X &value)
	{
		section_map::iterator section_it = m_sectionMap.find(section);
		
		if (section_it != m_sectionMap.end())
		{
			setting_map_ptr p_setting_map = section_it->second;
			
			setting_map::iterator setting = p_setting_map->find(name);

			if (setting != p_setting_map->end())
			{			
				assert(setting->second.type() == typeid(X));

				if (setting->second.type() == typeid(X))
					setting->second = value;
			}
		}
	}

private:

	typedef boost::shared_ptr<setting_map> setting_map_ptr;
	typedef std::map<stringT, setting_map_ptr> section_map;
	typedef typename section_map::value_type section_pair;

	static const int INT_BUFFER_SIZE = 64;
	static const int STR_BUFFER_SIZE = 500;
	static const int SECTION_BUFFER_SIZE = 4046;

	// hex representation of int, nothing but type
	class int_hex
	{
	public:
		int_hex(int value): m_value(value) {}
		int_hex(const int_hex &that) {operator=(that);}

		operator int () {return m_value;}
		int_hex &operator = (const int_hex &that) 
		{
			this->m_value = that.m_value;
			return *this;
		}
	private:
		int m_value;
	};

	// find section map by name or create new if not found
	setting_map_ptr create_or_find_setting_map(
		section_map &sections,
		const stringT &section)
	{
		section_map::iterator sec_it = sections.find(section);
		
		if (sec_it != sections.end())
		{
			return sec_it->second;
		}
		else
		{
			setting_map_ptr new_settings(new setting_map);
			sections.insert(section_pair(section, new_settings));
			return new_settings;
		}

		return setting_map_ptr();
	}

	// write setting to file
	void write_setting(
		const stringT &file,
		const stringT &section, 
		const stringT &name,
		const boost::any &value)
	{

		string_to_charF to_char;
		
		if (value.type() == typeid(int))
		{
			TCHAR buff[INT_BUFFER_SIZE];

			_itot_s(boost::any_cast<int>(value), buff, 
				sizeof(buff) / sizeof(TCHAR), 10
				);

			WritePrivateProfileString(
				to_char(section), 
				to_char(name), 
				buff, 
				to_char(file)
				);
		}
		else if (value.type() == typeid(int_hex))
		{
			TCHAR buff[INT_BUFFER_SIZE];

			_stprintf_s(buff, sizeof(buff) / sizeof(TCHAR), _T("0x%X"), 
				(int)boost::any_cast<int_hex>(value)
				);

			WritePrivateProfileString(
				to_char(section), 
				to_char(name), 
				buff, 
				to_char(file)
				);
		}
		else if (value.type() == typeid(stringT))
		{
			WritePrivateProfileString(
				to_char(section), 
				to_char(name), 
				to_char(boost::any_cast<stringT>(value)), 
				to_char(file)
				);
		}

	}

	// read setting from file
	void read_setting(
		const stringT &file,
		const stringT &section, 
		const stringT &name,
		boost::any &value)
	{

		string_to_charF to_char;

		if (value.type() == typeid(int))
		{
			value = (int)GetPrivateProfileInt(
				to_char(section), 
				to_char(name), 
				boost::any_cast<int>(value), 
				to_char(file)
				);
		}
		else if (value.type() == typeid(int_hex))
		{
			value = int_hex(
				(int)GetPrivateProfileInt(
					to_char(section), 
					to_char(name), 
					(int)boost::any_cast<int_hex>(value), 
					to_char(file)
					)
				);
		}
		else if (value.type() == typeid(stringT))
		{
			TCHAR buff[STR_BUFFER_SIZE];

			GetPrivateProfileString(
				to_char(section), 
				to_char(name), 
				to_char(boost::any_cast<stringT>(value)),
				buff,
				sizeof(buff) / sizeof(TCHAR), 
				to_char(file)
				);

			value = stringT(buff);
		}

	}

	const boost::any &find_setting(const stringT &section, const stringT &name)
	{
		section_map::iterator section_it = m_sectionMap.find(section);
		
		if (section_it != m_sectionMap.end())
		{
			setting_map_ptr p_setting_map = section_it->second;
			
			setting_map::iterator setting = p_setting_map->find(name);

			if (setting != p_setting_map->end())
			{			
				return setting->second;
			}
		}

		return setting_stub;
	}

	boost::any setting_stub;

	stringT m_settingFile;
	
	section_map m_sectionMap;
};



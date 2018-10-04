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

#include "settings.h"
#include "AngelicMachine/AngelicSettings.h"
#include "MachineWrapper.h"

// insert static settings
void InitializeSettingManager(setting_manager_t &settings)
{
	settings.insertSettings(SM_GENERAL_SECTION)
		(SM_LANGUAGE, _T("english"))
		(SM_LOCK, 0)
		;

	settings.insertSettings(ANGELIC_MACHINE_NAME)
		(am_behaviour(AM_PERIOD_LENGTH_MIN), AM_DEFAULT_PERIOD_LENGTH)
		(am_behaviour(AM_BREAK_LENGTH_MIN), AM_DEFAULT_BREAK_LENGTH)
		(am_behaviour(AM_DELAY_TIME_MIN), AM_DEFAULT_DELAY_LENGTH)
		(am_behaviour(AM_ALERT_BEFORE_MIN), AM_DEFAULT_ALERT_LENGTH)
		(am_behaviour(AM_ALERT_BEFORE_SEC), 0)
		(am_behaviour(AM_AUTOMATICALLY_ENABLE_AFTER_MIN), 
			AM_DEFAULT_DISABLE_SPAN)
		(am_behaviour(AM_SYNCHRONIZE_WITH_SYSTEM_TIME), 1)
		(am_behaviour(AM_USE_RESTRICTION_LIST), 0)
		(am_behaviour(AM_DELAY_LIMIT), AM_DEFAULT_DELAY_LIMIT)
		(am_behaviour(AM_BEEP_AFTER_BREAK), 0)
		(am_behaviour(AM_CURRENT_DAY), 0)
		(am_behaviour(AM_MAGIC_WORDS), _T(""))
		(am_behaviour(AM_USE_PERIODIC_BREAKS), 1)
		(am_behaviour(AM_USE_TIMED_BREAKS), 0)

		(am_appearance(AM_PROGRAM_MODE), 0)
		(am_appearance(AM_BACKGROUND_COLOR), AM_DEFAULT_BACKGROUND_COLOR)
		(am_appearance(AM_TRANSPARENCY_LEVEL), AM_DEFAULT_TRANSPARENCY_LEVEL)
		(am_appearance(AM_USE_TRANSPARENCY), 0)
		(am_appearance(AM_TRANSPARENT_COLOR), AM_DEFAULT_TRANSPARENT_COLOR)
		(am_appearance(AM_STRETCH_IMAGE), 0)
		(am_appearance(AM_SHOW_TIMER), 1)
		(am_appearance(am_fullscreen(AM_IMAGE_FOLDER)), _T(""))
		(am_appearance(am_windowed(AM_IMAGE_FOLDER)), _T(""))
		(am_appearance(AM_SOUND_FOLDER), _T(""))

		(am_appearance(am_alert(AM_TIMER_FONT_FACE)), AM_DEFAULT_FONT_FACE)
		(am_appearance(am_alert(AM_TIMER_FONT_SIZE)), AM_DEFAULT_FONT_SIZE)
		(am_appearance(am_alert(AM_TIMER_FONT_ITALIC)), 1)
		(am_appearance(am_alert(AM_TIMER_FONT_BOLD)), 1)
		(am_appearance(am_alert(AM_TIMER_X)), 160)
		(am_appearance(am_alert(AM_TIMER_Y)), 140)
		(am_appearance(am_alert(AM_TIMER_TEXT_COLOR)), AM_DEFAULT_FONT_COLOR)

		(am_appearance(am_fullscreen(AM_TIMER_FONT_FACE)), AM_DEFAULT_FONT_FACE)
		(am_appearance(am_fullscreen(AM_TIMER_FONT_SIZE)), AM_DEFAULT_FONT_SIZE)
		(am_appearance(am_fullscreen(AM_TIMER_FONT_ITALIC)), 1)
		(am_appearance(am_fullscreen(AM_TIMER_FONT_BOLD)), 1)
		(am_appearance(am_fullscreen(AM_TIMER_X)), 160)
		(am_appearance(am_fullscreen(AM_TIMER_Y)), 140)
		(am_appearance(am_fullscreen(AM_TIMER_TEXT_COLOR)),	
			AM_DEFAULT_FONT_COLOR)

		(am_appearance(am_windowed(AM_TIMER_FONT_FACE)), AM_DEFAULT_FONT_FACE)
		(am_appearance(am_windowed(AM_TIMER_FONT_SIZE)), 22)
		(am_appearance(am_windowed(AM_TIMER_FONT_ITALIC)), 1)
		(am_appearance(am_windowed(AM_TIMER_FONT_BOLD)), 1)
		(am_appearance(am_windowed(AM_TIMER_X)), 25)
		(am_appearance(am_windowed(AM_TIMER_Y)), 25)
		(am_appearance(am_windowed(AM_TIMER_TEXT_COLOR)), AM_DEFAULT_FONT_COLOR)
		
		(am_timed(AM_LENGTH), 0)
		;

	settings.insertSettings(RM_RESTRICTIONS_SECTION)
		(rm_behaviour(RM_CHECK_FOR_ACTIVITY_BEFORE_MIN), 0)
		(rm_proclist(RM_PROCESS_LIST_LENGTH), 0)
		;
}

int SettingManagerAdapter::getIntSetting(const tstring &name)
{
	// actually workaround for Angelic Machine, but this global setting
	// is avaliable always through CheckForUserActivityBeforeMin name
	static tstring check(am_behaviour(_T("CheckForUserActivityBeforeMin")));

	if (name == check)
	{
		return m_settings.get<int>(
			RM_RESTRICTIONS_SECTION,
			rm_behaviour(RM_CHECK_FOR_ACTIVITY_BEFORE_MIN)
			);
	}

	if (!m_settings.isSettingExists(m_section, name))
	{
		m_settings.insertSetting(m_section, name, 0);
		m_settings.loadSetting(m_section, name);
	}
	return m_settings.get<int>(m_section, name);
}
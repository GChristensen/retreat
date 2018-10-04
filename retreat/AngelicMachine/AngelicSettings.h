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

#include "boost/lexical_cast.hpp"

#include "../SettingTable.h"

// due to lack of metaprogramming...
// currently simple read settings by name
// but anyway these settings are completely independent from setting manager

// hypothetically these settings can be stored in xml, only change of separator
// is necessary

#define ANGELIC_MACHINE_NAME				_T("AngelicMachine")

#define AM_PERIOD_LENGTH_MIN				_T("PeriodLengthMin")
#define AM_BREAK_LENGTH_MIN					_T("BreakLengthMin")
#define AM_DELAY_TIME_MIN					_T("DelayTimeMin")
#define AM_ALERT_BEFORE_MIN					_T("AlertBeforeMin")
#define AM_ALERT_BEFORE_SEC					_T("AlertBeforeSec")
#define AM_CHECK_FOR_ACTIVITY_BEFORE_MIN	_T("CheckForUserActivityBeforeMin")
#define AM_AUTOMATICALLY_ENABLE_AFTER_MIN	_T("AutomaticallyEnableAfterMin")
#define AM_SYNCHRONIZE_WITH_SYSTEM_TIME		_T("SynchronizeWithSystemTime")
#define AM_USE_RESTRICTION_LIST				_T("AllowRestrictions")
#define AM_DELAY_LIMIT						_T("DelayLimit")
#define AM_BEEP_AFTER_BREAK					_T("BeepAfterBreak")
#define AM_CURRENT_DAY						_T("CurrentDay")
#define AM_MAGIC_WORDS						_T("MagicWords")
#define AM_USE_PERIODIC_BREAKS				_T("UsePeriodicBreaks")
#define AM_USE_TIMED_BREAKS					_T("UseTimedBreaks")

#define AM_PROGRAM_MODE						_T("Mode")
#define AM_BACKGROUND_COLOR					_T("BackgroundColor")
#define AM_TRANSPARENCY_LEVEL				_T("TransparencyLevel")
#define AM_USE_TRANSPARENCY					_T("UseTransparency")
#define AM_TRANSPARENT_COLOR				_T("TransparentColor")
#define AM_IMAGE_FOLDER						_T("ImageFolder")
#define AM_STRETCH_IMAGE					_T("StretchImage")
#define AM_SHOW_TIMER						_T("ShowTimer")
#define AM_SOUND_FOLDER						_T("SoundFolder")

#define AM_TIMER_FONT_FACE					_T("TimerFont")
#define AM_TIMER_FONT_SIZE					_T("TimerFontSize")
#define AM_TIMER_FONT_ITALIC				_T("TimerFontItalic")
#define AM_TIMER_FONT_BOLD					_T("TimerFontBold")
#define AM_TIMER_X							_T("TimerX")
#define AM_TIMER_Y							_T("TimerY")
#define AM_TIMER_TEXT_COLOR					_T("TimerTextColor")

#define AM_LENGTH							_T("Length")
#define AM_ITEM								_T("item")

#define AM_CRONTAB							_T("Crontab")

#define AM_FULLSCREEN_MODE					0
#define AM_WINDOWED_MODE					1
#define AM_ALERT_MODE						2

#define AM_BEHAVIOUR_PREFIX					_T("Behaviour")
#define AM_APPEARANCE_PREFIX				_T("Appearance")
#define AM_ALERT_MODE_PREFIX				_T("Alert")
#define AM_FULLSCREEN_MODE_PREFIX			_T("Fullscreen")
#define AM_WINDOWED_MODE_PREFIX				_T("Windowed")
#define AM_TIMED_BREAK_LIST_PREFIX			_T("TimedBreaks")
#define AM_TIMED_BREAK_MSG					_T("TimedBreakMsg")

#define am_behaviour(name) \
	(tstring(AM_BEHAVIOUR_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_appearance(name) \
	(tstring(AM_APPEARANCE_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_alert(name) \
	(tstring(AM_ALERT_MODE_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_fullscreen(name) \
	(tstring(AM_FULLSCREEN_MODE_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_windowed(name) \
	(tstring(AM_WINDOWED_MODE_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_timed(name) \
	(tstring(AM_TIMED_BREAK_LIST_PREFIX) + tstring(ST_PATH_SEPARATOR) + name)

#define am_separate(name1, name2) \
	(name1 + tstring(ST_PATH_SEPARATOR) + name2)

#define AM_DEFAULT_PERIOD_LENGTH		60
#define AM_DEFAULT_BREAK_LENGTH			10
#define AM_DEFAULT_DELAY_LENGTH			3
#define AM_DEFAULT_ALERT_LENGTH			1
#define AM_DEFAULT_DISABLE_SPAN			120
#define AM_DEFAULT_DELAY_LIMIT			2
#define AM_DEFAULT_FONT_FACE			_T("Arial")
#define AM_DEFAULT_FONT_SIZE			32
#define AM_DEFAULT_FONT_COLOR			RGB(239, 27, 27)
#define AM_DEFAULT_BACKGROUND_COLOR		RGB(255, 255, 255)
#define AM_DEFAULT_TRANSPARENT_COLOR	0xFF000000
#define AM_DEFAULT_TRANSPARENCY_LEVEL	128

struct AM_BEHAVIOUR_SETTINGS
{
	bool UsePeriodicBreaks;
	bool UseTimedBreaks;
	int PeriodLengthMin;
	int BreakLengthMin;
	int DelayTimeMin;
	int AlertBeforeMin;
	int AlertBeforeSec;
	int CheckForUserActivityBeforeMin;
	int AutomaticallyEnableAfterMin;
	bool SynchronizeWithSystemTime;
	bool UseRestrictionList;
	int DelayLimit;
	bool BeepAfterBreak;
	int CurrentDay;
	tstring MagicWords;

	void read(SettingTable *settings)
	{
		UsePeriodicBreaks 
			= getIntBehaiviorSetting(settings, AM_USE_PERIODIC_BREAKS) != 0;
		
		UseTimedBreaks
			= true;//getIntBehaiviorSetting(settings, AM_USE_TIMED_BREAKS) != 0;

		PeriodLengthMin = getIntBehaiviorSetting(settings, AM_PERIOD_LENGTH_MIN);

		// period length could not be less or equal to zero
		if (PeriodLengthMin <= 0)
		{
			PeriodLengthMin = AM_DEFAULT_PERIOD_LENGTH;
		}

		BreakLengthMin = getIntBehaiviorSetting(settings, AM_BREAK_LENGTH_MIN);

		if (BreakLengthMin <= 0 || BreakLengthMin > PeriodLengthMin)
		{
			BreakLengthMin = 1;
		}

		DelayTimeMin = getIntBehaiviorSetting(settings, AM_DELAY_TIME_MIN);

		if (DelayTimeMin > PeriodLengthMin - BreakLengthMin)
		{
			DelayTimeMin = AM_DEFAULT_DELAY_LENGTH;
		}

		AlertBeforeMin = getIntBehaiviorSetting(settings, AM_ALERT_BEFORE_MIN);

		if (   AlertBeforeMin <= 0 
			|| AlertBeforeMin > PeriodLengthMin - BreakLengthMin - DelayTimeMin)
		{
			AlertBeforeMin = 1;
		}
		
		AlertBeforeSec = getIntBehaiviorSetting(settings, AM_ALERT_BEFORE_SEC);

		CheckForUserActivityBeforeMin = getIntBehaiviorSetting(
			settings, AM_CHECK_FOR_ACTIVITY_BEFORE_MIN);

		if (CheckForUserActivityBeforeMin >
			PeriodLengthMin - BreakLengthMin - DelayTimeMin - AlertBeforeMin)
		{
			CheckForUserActivityBeforeMin = 0;
		}

		AutomaticallyEnableAfterMin = getIntBehaiviorSetting(
			settings, AM_AUTOMATICALLY_ENABLE_AFTER_MIN);

		SynchronizeWithSystemTime = getIntBehaiviorSetting(
			settings, AM_SYNCHRONIZE_WITH_SYSTEM_TIME) != 0;

		UseRestrictionList = 
			getIntBehaiviorSetting(settings, AM_USE_RESTRICTION_LIST) != 0;

		DelayLimit = getIntBehaiviorSetting(settings, AM_DELAY_LIMIT);

		BeepAfterBreak = 
			getIntBehaiviorSetting(settings, AM_BEEP_AFTER_BREAK) != 0;

		CurrentDay = getIntBehaiviorSetting(settings, AM_CURRENT_DAY);
		MagicWords = getStringBehaiviorSetting(settings, AM_MAGIC_WORDS);
	}

	void write(SettingTable *settings)
	{
		setIntBehaviourSetting(settings, AM_CURRENT_DAY, CurrentDay);

		setIntBehaviourSetting(settings, AM_SYNCHRONIZE_WITH_SYSTEM_TIME,
			(int)SynchronizeWithSystemTime);

		setIntBehaviourSetting(settings, AM_USE_PERIODIC_BREAKS,
			(int)UsePeriodicBreaks);

		setIntBehaviourSetting(settings, AM_BEEP_AFTER_BREAK,
			(int)BeepAfterBreak);

		setIntBehaviourSetting(settings, AM_USE_RESTRICTION_LIST,
			(int)UseRestrictionList);

		setIntBehaviourSetting(settings, AM_PERIOD_LENGTH_MIN, PeriodLengthMin);
		setIntBehaviourSetting(settings, AM_BREAK_LENGTH_MIN, BreakLengthMin);
		setIntBehaviourSetting(settings, AM_ALERT_BEFORE_MIN, AlertBeforeMin);
		setIntBehaviourSetting(settings, AM_DELAY_TIME_MIN, DelayTimeMin);
		setIntBehaviourSetting(settings, AM_DELAY_LIMIT, DelayLimit);
	}

private:

	int getIntBehaiviorSetting(SettingTable *settings, tstring name)
	{
		return settings->getIntSetting(am_behaviour(name));
	}

	tstring getStringBehaiviorSetting(SettingTable *settings, tstring name)
	{
		return settings->getStringSetting(am_behaviour(name));
	}

	void setIntBehaviourSetting(SettingTable *settings, 
		const tstring &name, int setting)
	{
		settings->setIntSetting(am_behaviour(name), setting);
	}
};

struct AM_APPEARANCE_SETTINGS
{
	int Mode;
	unsigned long BackgroundColor;
	unsigned char TransparencyLevel;
	bool UseTransparency;
	unsigned long TransparentColor;
	bool StretchImage;
	bool ShowTimer;
	tstring ImageFolder[2];
	tstring SoundFolder;

	struct TEXT_SETTINGS
	{
		tstring Face;
		int	Size;
		bool Italic;
		bool Bold;
		unsigned long Color;
		int x;
		int y;
	} TimerFontSettings[3];

	void read(SettingTable *settings)
	{
		BackgroundColor = 
			static_cast<unsigned long>(
				getIntAppearanceSetting(settings, AM_BACKGROUND_COLOR)
				);

		TransparencyLevel = 
			getIntAppearanceSetting(settings, AM_TRANSPARENCY_LEVEL);

		Mode = getIntAppearanceSetting(settings, AM_PROGRAM_MODE);

		if (Mode != 0) Mode = 1;

		UseTransparency = 
			getIntAppearanceSetting(settings, AM_USE_TRANSPARENCY) != 0;

		TransparentColor = 
			static_cast<unsigned long>(
				getIntAppearanceSetting(settings, AM_TRANSPARENT_COLOR)
				);

		StretchImage = getIntAppearanceSetting(settings, AM_STRETCH_IMAGE) != 0;
		ShowTimer = getIntAppearanceSetting(settings, AM_SHOW_TIMER) != 0;

		ImageFolder[AM_FULLSCREEN_MODE] = 
			getStringAppearanceSetting(settings, am_fullscreen(AM_IMAGE_FOLDER));

		ImageFolder[AM_WINDOWED_MODE] = 
			getStringAppearanceSetting(settings, am_windowed(AM_IMAGE_FOLDER));

		SoundFolder = getStringAppearanceSetting(settings, AM_SOUND_FOLDER);

		getTextSettngs(settings, TimerFontSettings[AM_FULLSCREEN_MODE], 
			AM_FULLSCREEN_MODE_PREFIX);

		getTextSettngs(settings, TimerFontSettings[AM_WINDOWED_MODE],
			AM_WINDOWED_MODE_PREFIX);

		getTextSettngs(settings, TimerFontSettings[AM_ALERT_MODE], 
			AM_ALERT_MODE_PREFIX);
	}

	void write(SettingTable *settings)
	{
		setIntAppearanceSetting(settings, AM_PROGRAM_MODE, Mode);
		setIntAppearanceSetting(settings, AM_BACKGROUND_COLOR, BackgroundColor);
		setIntAppearanceSetting(settings, AM_TRANSPARENT_COLOR, TransparentColor);
		
		setIntTextSetting(
			settings, 
			AM_FULLSCREEN_MODE_PREFIX,
			AM_TIMER_TEXT_COLOR,
			TimerFontSettings[AM_FULLSCREEN_MODE].Color
			);

		setIntTextSetting(
			settings, 
			AM_WINDOWED_MODE_PREFIX, 
			AM_TIMER_TEXT_COLOR,
			TimerFontSettings[AM_WINDOWED_MODE].Color
			);
		
		setIntTextSetting(
			settings, 
			AM_ALERT_MODE_PREFIX, 
			AM_TIMER_TEXT_COLOR,
			TimerFontSettings[AM_ALERT_MODE].Color
			);
		
		setIntAppearanceSetting(settings, AM_TRANSPARENCY_LEVEL, TransparencyLevel);
		setIntAppearanceSetting(settings, AM_USE_TRANSPARENCY, (int)UseTransparency);
		setIntAppearanceSetting(settings, AM_SHOW_TIMER, (int)ShowTimer);

		setStringTextSetting(
			settings, AM_FULLSCREEN_MODE_PREFIX, AM_IMAGE_FOLDER, ImageFolder[0]);

		setStringTextSetting(
			settings, AM_WINDOWED_MODE_PREFIX, AM_IMAGE_FOLDER, ImageFolder[1]);

		setStringAppearanceSetting(settings, AM_SOUND_FOLDER, SoundFolder);

		setIntAppearanceSetting(settings, AM_STRETCH_IMAGE, (int)StretchImage);
	}

private:

	int getIntAppearanceSetting(SettingTable *settings, const tstring &name)
	{
		return settings->getIntSetting(am_appearance(name));
	}

	void setIntAppearanceSetting(SettingTable *settings, 
		const tstring &name, int setting)
	{
		settings->setIntSetting(am_appearance(name), setting);
	}

	tstring getStringAppearanceSetting
		(
		 SettingTable *settings, 
		 const tstring &name
		)
	{
		return settings->getStringSetting(am_appearance(name));
	}

	void setStringAppearanceSetting
		(
		 SettingTable *settings, 
		 const tstring &name,
		 const tstring &setting
		)
	{
		settings->setStringSetting(am_appearance(name), setting);
	}

	int getIntTextSetting
		(
		 SettingTable *settings, 
		 const tstring &mode,
		 const tstring &name
		)
	{
		return getIntAppearanceSetting(settings, mode + ST_PATH_SEPARATOR + name);
	}

	void setIntTextSetting
		(
		 SettingTable *settings, 
		 const tstring &mode,
		 const tstring &name,
		 int setting
		)
	{
		setIntAppearanceSetting(settings, mode + ST_PATH_SEPARATOR + name, setting);
	}

	tstring getStringTextSetting
		(
		 SettingTable *settings, 
		 const tstring &mode,
		 const tstring &name
		)
	{
		return getStringAppearanceSetting(
			settings, mode + ST_PATH_SEPARATOR + name);
	}

	void setStringTextSetting
		(
		SettingTable *settings, 
		const tstring &mode,
		const tstring &name,
		const tstring &setting
		)
	{
		setStringAppearanceSetting(settings, mode + ST_PATH_SEPARATOR + name, setting);
	}

	void getTextSettngs
		(
		 SettingTable *settings, 
		 TEXT_SETTINGS &textSettings, 
		 const tstring &mode
		)
	{
		textSettings.Face = 
			getStringTextSetting(settings, mode, AM_TIMER_FONT_FACE);

		if (textSettings.Face.length() == 0)
		{
			textSettings.Face = AM_DEFAULT_FONT_FACE;
		}

		textSettings.Size = 
			getIntTextSetting(settings, mode, AM_TIMER_FONT_SIZE);

		if (textSettings.Size == 0)
		{
			textSettings.Size = AM_DEFAULT_FONT_SIZE;
		}

		textSettings.Italic = 
			getIntTextSetting(settings, mode, AM_TIMER_FONT_ITALIC) != 0;

		textSettings.Bold = 
			getIntTextSetting(settings, mode, AM_TIMER_FONT_BOLD) != 0;

		textSettings.x = getIntTextSetting(settings, mode, AM_TIMER_X);
		textSettings.y = getIntTextSetting(settings, mode, AM_TIMER_Y);

		textSettings.Color = 
			static_cast<unsigned long>(
				getIntTextSetting(settings, mode, AM_TIMER_TEXT_COLOR)
			);
	}
};

struct AM_TIMED_BREAKS
{
	struct TIMED_BREAK_DATA
	{
		tstring crontab;
		int duration;
		tstring message;
	};

	typedef std::vector<TIMED_BREAK_DATA> cron_breaks_array_t;
	cron_breaks_array_t breaks;

	void read(SettingTable *settings)
	{
		using boost::lexical_cast;
		using boost::bad_lexical_cast;

		breaks.clear();

		int breakListLength = 
			settings->getIntSetting(am_timed(AM_LENGTH));

		breaks.reserve(breakListLength);

		for (int i = 0; i < breakListLength; ++i)
		{
			tstring listItem = AM_ITEM + lexical_cast<tstring>(i);

			TIMED_BREAK_DATA data;

			data.crontab = 
				settings->getStringSetting(
					am_timed(am_separate(listItem, AM_CRONTAB)));

			data.duration = 
				settings->getIntSetting(
					am_timed(am_separate(listItem, AM_BREAK_LENGTH_MIN)));

			data.message = 
				settings->getStringSetting(
					am_timed(am_separate(listItem, AM_TIMED_BREAK_MSG)));

			breaks.push_back(data);
		}
	}

	void write(SettingTable *settings)
	{
		using boost::lexical_cast;

		settings->setIntSetting(am_timed(AM_LENGTH), breaks.size());

		for (size_t i = 0; i < breaks.size(); ++i)
		{
			tstring listItem = 
				am_timed(AM_ITEM) + lexical_cast<tstring>(i);

			settings->setStringSetting(
				am_separate(listItem, AM_CRONTAB), breaks[i].crontab);

			settings->setIntSetting(
				am_separate(listItem, AM_BREAK_LENGTH_MIN), breaks[i].duration);

			settings->setStringSetting(
				am_separate(listItem, AM_TIMED_BREAK_MSG), breaks[i].message);
		}
	}
};
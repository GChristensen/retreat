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
#include "CronTab.h"

#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"
#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

CronTab::CRON_VALUE const CronTab::m_cronRanges[] =
{
	{0, 59},
	{0, 23},
	{1, 31},
	{1, 12},
	{0, 7}
};

CronTab::CronTab(tstring crontab) :
m_crontab(5),
m_invalid(false)
{
	bool continuous = true;

	// parse crontab, here is assumed that crontab is always syntactically
	// valid
	std::vector<tstring> string_crontab_items;
	split(crontab, _T(" "), string_crontab_items);

	for (size_t i = 0; i < string_crontab_items.size() && i < 5; ++i)
	{
		if (string_crontab_items[i] == _T("*"))
		{
			continue;
		}

		continuous = false;

		std::vector<tstring> string_crontab_item;
		split(string_crontab_items[i], _T(","), string_crontab_item);

		foreach(tstring &value, string_crontab_item)
		{
			std::vector<tstring> string_crontab_value;
			split(value, _T("-"), string_crontab_value);

			if (string_crontab_value.size() > 1)
			{
				CRON_VALUE cron_value = {-1, -1};

				try
				{
					cron_value.v1 = 
						boost::lexical_cast<int>(string_crontab_value[0]);

					cron_value.v2 = 
						boost::lexical_cast<int>(string_crontab_value[1]);
				}
				catch (boost::bad_lexical_cast &)
				{
				}

				if (   cron_value.v1 < m_cronRanges[i].v1 
					|| cron_value.v1 > m_cronRanges[i].v2
					|| cron_value.v2 < m_cronRanges[i].v1 
					|| cron_value.v2 > m_cronRanges[i].v2
					|| cron_value.v1 > cron_value.v2)
				{
					m_invalid = true;
					break;
				}

				m_crontab[i].push_back(cron_value);
			}
			else if (string_crontab_value.size() == 1)
			{
				CRON_VALUE cron_value = {-1, -1};

				try
				{
					cron_value.v1 = 
						boost::lexical_cast<int>(string_crontab_value[0]);
				}
				catch (boost::bad_lexical_cast &)
				{
				}

				if (   cron_value.v1 < m_cronRanges[i].v1 
					|| cron_value.v1 > m_cronRanges[i].v2)
				{
					m_invalid = true;
					break;
				}

				m_crontab[i].push_back(cron_value);
			}
		}

		if (m_invalid)
		{
			break;
		}
	}

	if (continuous)
	{
		m_invalid = true;
	}

	return;
}

bool CronTab::includes(const tm &time)
{
	if (m_invalid)
	{
		return false;
	}

	bool result = true;

	int tm_values[] = 
	{time.tm_min, time.tm_hour, time.tm_mday, time.tm_mon};

	// at first check day of week, 7 - also Sunday
	foreach (CRON_VALUE &value, m_crontab[4])
	{ 
		if (value.v2 >= 0)
		{
			result = time.tm_wday >= value.v1 && time.tm_wday <= value.v2
				|| time.tm_wday == 0 && value.v2 == 7;
		}
		else
		{
			result = time.tm_wday == value.v1
				|| time.tm_wday == 0 && value.v1 == 7;
		}

		if (result)
		{
			break;
		}
	}

	if (!result)
	{
		return false;
	}

	for (int i = 0; i < 4; ++i)
	{
		foreach (CRON_VALUE &value, m_crontab[i])
		{
			if (value.v2 >= 0)
			{
				result = tm_values[i] >= value.v1 
					&& tm_values[i] <= value.v2;
			}
			else
			{
				result = tm_values[i] == value.v1;
			}

			if (result)
			{
				break;
			}
		}

		if (!result)
		{
			break;
		}
	}

	return result;
}

void CronTab::split(const tstring &in, const TCHAR *separator, std::vector<tstring> &out)
{
	typedef boost::tokenizer
		<
		boost::char_separator<TCHAR>, 
		tstring::const_iterator,
		tstring
		> tokenizer;

	boost::char_separator<TCHAR> separator_char(separator);
	tokenizer tokens(in, separator_char);

	copy(tokens.begin(), tokens.end(), 
		std::insert_iterator<std::vector<tstring> >(out, out.begin()));
}
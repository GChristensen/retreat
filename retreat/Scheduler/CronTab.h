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

#include <ctime>

#include <vector>

class CronTab
{
public:

	CronTab(tstring crontab);

	bool includes(const tm &time);
	bool valid() { return !m_invalid; }

private:

	// if v2 is -1, corresponding value is single value, else value is range
	struct CRON_VALUE
	{
		int v1;
		int v2;
	};

	static const CRON_VALUE m_cronRanges[];

	typedef std::vector<CRON_VALUE> cron_value_array_t;
	
	// if cron_value_array_t is empty, item is undefined ("*")
	typedef std::vector<cron_value_array_t> ctrontab_t;

	ctrontab_t m_crontab;
	bool m_invalid;

	void split
		(
		 const tstring &in, 
		 const TCHAR *separator, 
		 std::vector<tstring> &out
		);
};
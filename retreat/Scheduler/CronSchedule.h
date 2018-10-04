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
#include "Schedule.h"
#include "CronTab.h"

#include <vector>
#include <exception>

#include "boost/shared_ptr.hpp"

class CronSchedule :
	public Schedule
{
public:

	class bad_crontab: public std::exception {};

	CronSchedule(const tstring &name, bool disposable = false);
	virtual ~CronSchedule();

	void addCrontab(const tstring &crontab, const tstring &message, 
		int event, long data);  // throw (bad_crontab);
	
	virtual void adjust(int seconds) {}
	virtual bool isReady(const time_t* now, Event &event);

private:

	typedef boost::shared_ptr<CronTab> crontab_ptr_t;	

	struct CRON_DATA
	{
		crontab_ptr_t pCronTab;
		tstring message;
		int event;
		long data;
	};

	typedef std::vector<CRON_DATA> cron_table_t;
	cron_table_t m_crontTable;
};

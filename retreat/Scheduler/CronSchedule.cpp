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
#include "CronSchedule.h"

CronSchedule::CronSchedule
(const tstring &name, bool disposable):
Schedule(name, disposable, false)
{
}

CronSchedule::~CronSchedule()
{
}

void CronSchedule::addCrontab(const tstring &crontab, const tstring &message, 
							  int event, long data)
{
	CRON_DATA cron_data;

	cron_data.pCronTab = crontab_ptr_t(new CronTab(crontab));

	if (!cron_data.pCronTab->valid())
	{
		throw bad_crontab();
	}

	cron_data.message = message;
	cron_data.event = event;
	cron_data.data = data;

	m_crontTable.push_back(cron_data);
}

bool CronSchedule::isReady(const time_t* now, Event &event)
{
	if (!(*now % 60))
	{
		tm *local_time = localtime(now);
		cron_table_t::iterator crontab_it = m_crontTable.begin();

		while (crontab_it != m_crontTable.end())
		{
			if (crontab_it->pCronTab->includes(*local_time))
			{
				
				event = Event(crontab_it->event, crontab_it->data);
				return true;
			}

			++crontab_it;
		}
	}

	return false;
}
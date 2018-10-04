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

#define ST_PATH_SEPARATOR	_T(":")

class SettingTable
{
public:
	virtual int getIntSetting(const tstring &name) = 0;
	virtual tstring getStringSetting(const tstring &name) = 0;

	virtual void setIntSetting(const tstring &name, int setting) = 0;
	virtual void setStringSetting(const tstring &name, const tstring &setting) = 0;
};
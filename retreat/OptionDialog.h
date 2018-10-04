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

#include <atlstr.h>
#include <atlwin.h>
#include <atlcrack.h>

#include "SettingTable.h"

class COptionDialog: 
	public CDialogImpl<COptionDialog>
{
public:

	int IDD;

	COptionDialog(int idd):
	IDD(idd),
	m_accepted(false)
	{
	}

	virtual ~COptionDialog()
	{
	}

	const tstring &GetOptionsCategoryName()
	{
		return m_optionsCategoryName;
	}

	void SetOptionsCategoryName(const tstring &name)
	{
		m_optionsCategoryName = name;
	}

	void SetAccepted(bool accepted) {m_accepted = accepted;}
	bool Accepted() {return m_accepted;}

	virtual void read(SettingTable *settings)
	{}

	virtual void write(SettingTable *settings)
	{}

	DECLARE_EMPTY_MSG_MAP()
	
protected:

	bool m_accepted;
	tstring m_optionsCategoryName;

};

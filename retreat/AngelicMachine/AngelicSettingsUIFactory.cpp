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

#include <cassert>

#include <atlmisc.h>
#include "../resource.h"

#include "AngelicSettingsUIFactory.h"
#include "AngelicOptionsDlg.h"

AngelicSettingsUIFactory::AngelicSettingsUIFactory()
{
}

AngelicSettingsUIFactory::~AngelicSettingsUIFactory()
{
}

COptionDialog *AngelicSettingsUIFactory::getUI(HWND parent)
{
	CAngelicOptionsDlg *result = new CAngelicOptionsDlg();
	
	result->Create(parent);

	WTL::CString group(MAKEINTRESOURCE(IDS_ANGELIC_OPTIONS_GROUP_NAME));
	result->SetOptionsCategoryName((LPCTSTR)group);

	return result;
}

void AngelicSettingsUIFactory::utilizeUI(COptionDialog *ui)
{
	assert(ui);

	if (ui != NULL)
	{
		ui->DestroyWindow();
		delete ui;
	}
}
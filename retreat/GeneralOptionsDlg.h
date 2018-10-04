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

#include "OptionDialog.h"

#include <map>

class CGeneralOptionsDlg: public COptionDialog
{
public:

	CGeneralOptionsDlg();

	virtual void write(SettingTable *settings);
	
	tstring GetLanguage();
	void SetLanguage(const tstring &language);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);

	BEGIN_MSG_MAP_EX(CGeneralOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		ALT_MSG_MAP(1)
	END_MSG_MAP()

private:

	std::map<tstring, tstring> m_lang_map;
	CContainedWindowT<CComboBox> m_wndLanguageList;

};

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
#include "resource.h"

#include "GeneralOptionsDlg.h"

#include <algorithm>

#include "settings.h"

#include "utility/SystemHelper.h"

CGeneralOptionsDlg::CGeneralOptionsDlg() :
COptionDialog(IDD_GENERAL_OPTIONS),
m_wndLanguageList(this, 1)
{
}

void CGeneralOptionsDlg::write(SettingTable *settings)
{
	settings->setStringSetting(SM_LANGUAGE, GetLanguage());
}

void CGeneralOptionsDlg::SetLanguage(const tstring &language)
{
	m_wndLanguageList.AddString(_T("English"));

	tstring lang_dir = SystemHelper::getInstance()->getModuleDir();
	lang_dir.append(_T("\\language"));

	SystemHelper::getInstance()->getLanguages(lang_dir, m_lang_map);

	int sel = 0;

	std::map<tstring, tstring>::iterator lang_it;
	for (lang_it = m_lang_map.begin(); lang_it != m_lang_map.end(); ++lang_it)
	{
		m_wndLanguageList.AddString(lang_it->second.c_str());

		if (lang_it->first == language)
		{
			sel = std::distance(m_lang_map.begin(), lang_it) + 1;
		}
	}

	m_wndLanguageList.SetCurSel(sel);
}

tstring CGeneralOptionsDlg::GetLanguage()
{
	tstring result = _T("english");

	int sel = m_wndLanguageList.GetCurSel();

	if (sel > 0)
	{
		std::map<tstring, tstring>::iterator lang_it = m_lang_map.begin();
		std::advance(lang_it, sel - 1);

		result = lang_it->first;
	}

	return result;
}

LRESULT CGeneralOptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_wndLanguageList.SubclassWindow(GetDlgItem(IDC_LANGUAGE));
	return 0;
}
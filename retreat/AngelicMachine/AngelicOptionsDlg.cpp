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

#include "AngelicOptionsDlg.h"

#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"
#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH

#include <atldlgs.h>

//// CTimedBreakDlg ////////////////////////////////////////////////////////////

CTimedBreakDlg::CTimedBreakDlg():
m_duration(0)
{

}

void CTimedBreakDlg::SetCrontab(tstring crontab)
{
	typedef boost::tokenizer
		<
		boost::char_separator<TCHAR>, 
		tstring::const_iterator,
		tstring
		> tokenizer;

	boost::char_separator<TCHAR> separator_char(_T(" "));
	tokenizer tokens(crontab, separator_char);

	int i = 0;
	foreach (tstring s, tokens)
	{
		m_crontab[i] = s.c_str();

		if (++i == sizeof(m_crontab) / sizeof(m_crontab[0])) 
			break;
	}
}

tstring CTimedBreakDlg::GetCrontab()
{
	tstring result;

	int n = sizeof(m_crontab) / sizeof(m_crontab[0]);

	for (int i = 0; i < n; ++i)
	{
		result.append((LPCTSTR)m_crontab[i]);

		if (i < n - 1)
		{
			result.append(_T(" "));
		}
	}

	return result;
}

void CTimedBreakDlg::SetDuration(unsigned duration)
{
	m_duration = duration;
}

unsigned CTimedBreakDlg::GetDuration()
{
	return m_duration;
}

void CTimedBreakDlg::SetMessage(WTL::CString message)
{
	m_message = message;
}

tstring CTimedBreakDlg::GetMessage()
{
	m_message.TrimRight();
	m_message.TrimLeft();
	return (LPCTSTR)m_message;
}

LRESULT CTimedBreakDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	DoDataExchange(FALSE);
	CenterWindow();
	return 0;
}

LRESULT CTimedBreakDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT CTimedBreakDlg::OnClear(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	m_message = _T("");
	DoDataExchange(FALSE);
	return 0;
}

LRESULT CTimedBreakDlg::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(TRUE);
	EndDialog(IDOK);
	return 0;
}

LRESULT CTimedBreakDlg::OnCancel(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	SendMessage(WM_CLOSE);
	return 0;
}

//// CAngelicBehaviourDlg //////////////////////////////////////////////////////

CAngelicBehaviourDlg::CAngelicBehaviourDlg(HWND hBase):
m_hBase(hBase)
{
}

void CAngelicBehaviourDlg::read(SettingTable *settings)
{
	m_behaviour.read(settings);
	m_bDisablePeriodicBreaks = !m_behaviour.UsePeriodicBreaks;

	DoDataExchange(FALSE);

	initSpin(GetDlgItem(IDC_PERIOD_SPIN), m_behaviour.PeriodLengthMin, 1);
	initSpin(GetDlgItem(IDC_BREAK_SPIN), m_behaviour.BreakLengthMin, 1);
	initSpin(GetDlgItem(IDC_ALERT_SPIN), m_behaviour.AlertBeforeMin, 1);
	initSpin(GetDlgItem(IDC_DELAY_SPIN), m_behaviour.DelayTimeMin, 1);
	initSpin(GetDlgItem(IDC_DELAY_LIMIT_SPIN), m_behaviour.DelayLimit, 0, 5);
}

void CAngelicBehaviourDlg::write(SettingTable *settings)
{
	DoDataExchange(TRUE);

	m_behaviour.UsePeriodicBreaks = !m_bDisablePeriodicBreaks;
	m_behaviour.write(settings);
}

LRESULT CAngelicBehaviourDlg::
		OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	return 0;
}

LRESULT CAngelicBehaviourDlg::
		OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	return 0;
}

void CAngelicBehaviourDlg::
	 initSpin(HWND wnd, int pos, int min /*= 0*/, int max /*= UD_MAXVAL*/)
{
	::SendMessage(wnd, UDM_SETRANGE, 0, MAKELPARAM(max, min));
	::SendMessage(wnd, UDM_SETPOS, 0, MAKELPARAM(pos, 0));
}

//// CAngelicScheduleDlg //////////////////////////////////////////////////////

CAngelicScheduleDlg::CAngelicScheduleDlg(HWND hBase):
m_hBase(hBase)
{
}

void CAngelicScheduleDlg::read(SettingTable *settings)
{
	m_timedBreaks.read(settings);

	for (size_t i = 0; i < m_timedBreaks.breaks.size(); ++i)
	{
		insertTimedBreak(
			i, 
			m_timedBreaks.breaks[i].crontab,
			m_timedBreaks.breaks[i].duration,
			m_timedBreaks.breaks[i].message
			);
	}
}

void CAngelicScheduleDlg::write(SettingTable *settings)
{
	int n = m_timedBreaksList.GetItemCount();
	m_timedBreaks.breaks.clear();
	m_timedBreaks.breaks.resize(n);

	for (int i = 0; i < n; ++i)
	{
		using boost::lexical_cast;
		using boost::bad_lexical_cast;

		WTL::CString text;

		m_timedBreaksList.GetItemText(i, 1, text);
		m_timedBreaks.breaks[i].crontab = (LPCTSTR)text;

		m_timedBreaksList.GetItemText(i, 2, text);

		try
		{
			m_timedBreaks.breaks[i].duration = 
				lexical_cast<unsigned>((LPCTSTR)text);
		}
		catch (bad_lexical_cast &)
		{
		}

		m_timedBreaksList.GetItemText(i, 3, text);
		m_timedBreaks.breaks[i].message = (LPCTSTR)text;
	}

	m_timedBreaks.write(settings);
}

LRESULT CAngelicScheduleDlg::
		OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{	
	m_wndAddBtn.Attach(GetDlgItem(IDC_ADD_CRON));
	m_wndDeleteBtn.Attach(GetDlgItem(IDC_DELETE_CRON));

	m_timedBreaksList.Attach(GetDlgItem(IDC_TIMED_BREAKS_LIST));

	m_timedBreaksList.SetView(LV_VIEW_DETAILS);
	m_timedBreaksList.SetExtendedListViewStyle(
		  m_timedBreaksList.GetExtendedListViewStyle() 
		| LVS_EX_FULLROWSELECT 
		| LVS_EX_GRIDLINES
		);

	// workaround for first column text alignment
	// see MSDN on LVCOLUMN
	m_timedBreaksList.AddColumn(_T("Dummy"), 0); 

	WTL::CString column(MAKEINTRESOURCE(IDS_TIMED_BREAK_NUMBER_COLUMN));
	m_timedBreaksList.AddColumn(
		column, 1, -1,
		LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ,
		LVCFMT_RIGHT
		);

	column.LoadString(IDS_TIMED_BREAK_CRONTAB_COLUMN);
	m_timedBreaksList.AddColumn(column, 2);

	column.LoadString(IDS_TIMED_BREAK_DURATION_COLUMN);
	m_timedBreaksList.AddColumn(
		column, 3, -1, 
		LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
		LVCFMT_RIGHT
		);

	column.LoadString(IDS_TIMED_BREAK_MESSAGE_COLUMN);
	m_timedBreaksList.AddColumn(column, 4);

	m_timedBreaksList.DeleteColumn(0);

	m_timedBreaksList.SetColumnWidth(1, 100);
	m_timedBreaksList.SetColumnWidth(3, 100);

	return 0;
}

LRESULT CAngelicScheduleDlg::
		OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	return 0;
}

LRESULT CAngelicScheduleDlg::
		OnAddTimedBreak(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CTimedBreakDlg dlg;

	dlg.SetCrontab(_T("* * * * *"));
	dlg.SetDuration(AM_DEFAULT_BREAK_LENGTH);

	if (dlg.DoModal(GetContainer()) == IDOK)
	{
		insertTimedBreak(
			m_timedBreaksList.GetItemCount(), 
			dlg.GetCrontab(),
			dlg.GetDuration(),
			dlg.GetMessage()
			);

		m_timedBreaksList.SelectItem(m_timedBreaksList.GetItemCount() - 1);
	}

	return 0;
}

LRESULT CAngelicScheduleDlg::
		OnDeleteTimedBreak(WPARAM wParam, LPARAM lParam, HWND wnd)
{
	int sel = m_timedBreaksList.GetSelectedIndex();

	if (sel >= 0)
	{
		WTL::CString message(MAKEINTRESOURCE(IDS_CONFIRM_DELETION_MESSAGE));
		WTL::CString caption(MAKEINTRESOURCE(IDS_APP_NAME));

		GetContainer();


		if (::MessageBox(
			GetContainer(), message, caption, MB_ICONQUESTION | MB_YESNO)
			== IDYES)
		{
			using boost::lexical_cast;

			m_timedBreaksList.DeleteItem(sel);

			int n = m_timedBreaksList.GetItemCount();

			for (int i = 0; i < n; ++i)
			{
				m_timedBreaksList.SetItemText(
					i, 0, lexical_cast<tstring>(i + 1).c_str());
			}
		}
	}

	return 0;
}

LRESULT CAngelicScheduleDlg::OnLvDblClick(LPNMHDR hdr)
{
	LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)hdr;

	if (lpnmitem->iItem >= 0)
	{		
		using boost::lexical_cast;
		using boost::bad_lexical_cast;

		CTimedBreakDlg dlg;
		WTL::CString text;

		m_timedBreaksList.GetItemText(lpnmitem->iItem, 1, text);
		dlg.SetCrontab((LPCTSTR)text);

		m_timedBreaksList.GetItemText(lpnmitem->iItem, 2, text);

		try
		{
			dlg.SetDuration(lexical_cast<unsigned>((LPCTSTR)text));
		}
		catch(bad_lexical_cast &)
		{
		}

		m_timedBreaksList.GetItemText(lpnmitem->iItem, 3, text);
		dlg.SetMessage((LPCTSTR)text);

		if (dlg.DoModal(GetContainer()) == IDOK)
		{
			m_timedBreaksList.SetItemText(
				lpnmitem->iItem, 1, dlg.GetCrontab().c_str());

			m_timedBreaksList.SetItemText(
				lpnmitem->iItem, 2, 
				lexical_cast<tstring>(dlg.GetDuration()).c_str()
				);

			m_timedBreaksList.SetItemText(
				lpnmitem->iItem, 3, dlg.GetMessage().c_str());
		}
	}

	return 0;
}

void CAngelicScheduleDlg::
		insertTimedBreak(int n, const tstring &crontab, unsigned duration, 
		const tstring &message)
{
	using boost::lexical_cast;
	m_timedBreaksList.InsertItem(n, lexical_cast<tstring>(n + 1).c_str());
	m_timedBreaksList.SetItemText(n, 1, crontab.c_str());

	m_timedBreaksList.SetItemText(
		n, 2, lexical_cast<tstring>(duration).c_str());

	m_timedBreaksList.SetItemText(n, 3, message.c_str());
}

//// CAngelicAppearanceDlg /////////////////////////////////////////////////////

CAngelicAppearanceDlg::CAngelicAppearanceDlg(HWND hBase):
m_hBase(hBase)
{
}

void CAngelicAppearanceDlg::read(SettingTable *settings)
{
	m_appearance.read(settings);
	m_hideTimer = !m_appearance.ShowTimer;

	int modeIndex = 0;

	if (m_appearance.Mode == AM_FULLSCREEN_MODE && m_appearance.UseTransparency)
	{
		modeIndex = 1;
	}
	else if (m_appearance.Mode == AM_WINDOWED_MODE && !m_appearance.UseTransparency)
	{
		modeIndex = 2;
	}
	else if (m_appearance.Mode == AM_WINDOWED_MODE && m_appearance.UseTransparency)
	{
		modeIndex = 3;
	}

	m_wndModeList.SetCurSel(modeIndex);

	HWND hOpacitySpin = GetDlgItem(IDC_OPACITY_SPIN);
	::SendMessage(hOpacitySpin, UDM_SETRANGE, 0, MAKELPARAM(255, 0));
	::SendMessage(hOpacitySpin, UDM_SETPOS, 0, m_appearance.TransparencyLevel);

	m_imageFolder = m_appearance.ImageFolder[m_appearance.Mode].c_str();

	m_soundFolder = m_appearance.SoundFolder.c_str();

	m_wndBackgroundColor.SetColor(m_appearance.BackgroundColor);

	if (m_appearance.TransparentColor != AM_DEFAULT_TRANSPARENT_COLOR)
	{
		m_wndTransparentColor.SetColor(m_appearance.TransparentColor);
	}

	m_wndTimerTextColor.SetColor(m_appearance.TimerFontSettings[0].Color);

	DoDataExchange(FALSE);
}

void CAngelicAppearanceDlg::write(SettingTable *settings)
{
	DoDataExchange(TRUE);

	m_appearance.ShowTimer = !m_hideTimer;

	int modeIndex = m_wndModeList.GetCurSel();

	if (modeIndex == 0)
	{
		m_appearance.Mode = AM_FULLSCREEN_MODE;
		m_appearance.UseTransparency = false;
	}
	else if (modeIndex == 1)
	{
		m_appearance.Mode = AM_FULLSCREEN_MODE;
		m_appearance.UseTransparency = true;
	}
	else if (modeIndex == 2)
	{
		m_appearance.Mode = AM_WINDOWED_MODE;
		m_appearance.UseTransparency = false;
	}
	else if (modeIndex == 3)
	{
		m_appearance.Mode = AM_WINDOWED_MODE;
		m_appearance.UseTransparency = true;
	}

	m_appearance.ImageFolder[m_appearance.Mode] = (LPCTSTR)m_imageFolder;

	m_appearance.SoundFolder = (LPCTSTR)m_soundFolder;

	m_appearance.BackgroundColor = m_wndBackgroundColor.GetColor();

	if (m_wndTransparentColor.GetColor() == CLR_DEFAULT)
	{
		m_appearance.TransparentColor = AM_DEFAULT_TRANSPARENT_COLOR;
	}
	else
	{
		m_appearance.TransparentColor = m_wndTransparentColor.GetColor();
	}

	for (int i = 0; i < 3; ++i)
	{
		m_appearance.TimerFontSettings[i].Color = 
			m_wndTimerTextColor.GetColor();
	}

	m_appearance.write(settings);
}

LRESULT CAngelicAppearanceDlg::
		OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_wndModeList.Attach(GetDlgItem(IDC_ANGELIC_MODE_LIST));

	m_wndModeList.AddString(
		WTL::CString(MAKEINTRESOURCE(IDS_FULLSCREEN_MODE_ITEM)));
	m_wndModeList.AddString(
		WTL::CString(MAKEINTRESOURCE(IDS_TRANSPARENT_FULLSCREEN_MODE_ITEM)));
	m_wndModeList.AddString(
		WTL::CString(MAKEINTRESOURCE(IDS_WINDOWED_MODE_ITEM)));
	m_wndModeList.AddString(
		WTL::CString(MAKEINTRESOURCE(IDS_TRANSPARENT_WINDOWED_MODE_ITEM)));

	m_wndModeList.SetCurSel(0);

	m_wndBackgroundColor.SubclassWindow(GetDlgItem(IDC_BACKGROUND_COLOR));
	m_wndTransparentColor.SubclassWindow(GetDlgItem(IDC_TRANSPARENT_COLOR));
	m_wndTimerTextColor.SubclassWindow(GetDlgItem(IDC_TIMER_TEXT_COLOR));

	WTL::CString defaultText(MAKEINTRESOURCE(IDS_DEFAULT_COLOR_TEXT));
	WTL::CString customText(MAKEINTRESOURCE(IDS_CUSTOM_COLOR_TEXT));

	m_wndBackgroundColor.SetDefaultText(defaultText);
	m_wndBackgroundColor.SetCustomText(customText);
	m_wndTransparentColor.SetDefaultText(defaultText);
	m_wndTransparentColor.SetCustomText(customText);
	m_wndTimerTextColor.SetDefaultText(defaultText);
	m_wndTimerTextColor.SetCustomText(customText);

	m_wndTransparentColor.SetDefaultColor(RGB(128, 128, 128));

	return 0;
}

LRESULT CAngelicAppearanceDlg::
		OnImageFolderChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_wndModeList.GetCurSel() < 2)
	{
		DoDataExchange(TRUE, IDC_IMAGE_FOLDER);
		m_appearance.ImageFolder[0] = (LPCTSTR)m_imageFolder;
	}
	else
	{
		DoDataExchange(TRUE, IDC_IMAGE_FOLDER);
		m_appearance.ImageFolder[1] = (LPCTSTR)m_imageFolder;
	}

	return 0;
}

LRESULT CAngelicAppearanceDlg::
		OnModeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_wndModeList.GetCurSel() < 2)
	{
		m_imageFolder = m_appearance.ImageFolder[0].c_str();
		DoDataExchange(FALSE, IDC_IMAGE_FOLDER);
	}
	else
	{
		m_imageFolder = m_appearance.ImageFolder[1].c_str();
		DoDataExchange(FALSE, IDC_IMAGE_FOLDER);
	}

	return 0;
}

LRESULT CAngelicAppearanceDlg::
		OnBrowseImageFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	WTL::CString title(MAKEINTRESOURCE(IDS_BROWSE_IMAGE_FOLDER_TITLE));

	CFolderDialog dlg(NULL, title);

	DoDataExchange(TRUE, IDC_IMAGE_FOLDER);
	dlg.SetInitialFolder(m_imageFolder);

	if (dlg.DoModal(GetContainer()) == IDOK)
	{
		m_imageFolder = dlg.GetFolderPath();
		DoDataExchange(FALSE, IDC_IMAGE_FOLDER);
	}

	return 0;
}

LRESULT CAngelicAppearanceDlg::
		OnBrowseSoundFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	WTL::CString title(MAKEINTRESOURCE(IDS_BROWSE_SOUND_FOLDER_TITLE));

	CFolderDialog dlg(NULL, title);

	DoDataExchange(TRUE, IDC_SOUND_FOLDER);
	dlg.SetInitialFolder(m_soundFolder);

	if (dlg.DoModal(GetContainer()) == IDOK)
	{
		m_soundFolder = dlg.GetFolderPath();
		DoDataExchange(FALSE, IDC_SOUND_FOLDER);
	}

	return 0;
}

//// CAngelicOptionsDlg ////////////////////////////////////////////////////////

CAngelicOptionsDlg::CAngelicOptionsDlg():
COptionDialog(IDD_ANGELIC_OPTIONS),
m_angelicBehaviour(NULL),
m_angelicSchedule(NULL),
m_angelicAppearance(NULL)
{
}

void CAngelicOptionsDlg::read(SettingTable *settings)
{
	ATLASSERT(m_angelicBehaviour.IsWindow());
	m_angelicBehaviour.read(settings);

	ATLASSERT(m_angelicSchedule.IsWindow());
	m_angelicSchedule.read(settings);

	ATLASSERT(m_angelicAppearance.IsWindow());
	m_angelicAppearance.read(settings);
}

void CAngelicOptionsDlg::write(SettingTable *settings)
{
	ATLASSERT(m_angelicBehaviour.IsWindow());
	m_angelicBehaviour.write(settings);

	ATLASSERT(m_angelicSchedule.IsWindow());
	m_angelicSchedule.write(settings);

	ATLASSERT(m_angelicAppearance.IsWindow());
	m_angelicAppearance.write(settings);
}

LRESULT CAngelicOptionsDlg::
		OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	CRect rect;
	GetClientRect(&rect);
	m_tabView.Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE);

	m_angelicBehaviour.Create(m_tabView.m_hWnd);
	m_angelicSchedule.Create(m_tabView.m_hWnd);
	m_angelicAppearance.Create(m_tabView.m_hWnd);

	m_angelicBehaviour.SetBase(GetParent());
	m_angelicSchedule.SetBase(GetParent());
	m_angelicAppearance.SetBase(GetParent());

	m_tabView.AddTab(WTL::CString(MAKEINTRESOURCE(IDS_BEHAVIOUR_TAB)), 
		m_angelicBehaviour);
	m_tabView.AddTab(WTL::CString(MAKEINTRESOURCE(IDS_SCHEDULE_TAB)), 
		m_angelicSchedule);
	m_tabView.AddTab(WTL::CString(MAKEINTRESOURCE(IDS_APPEARANCE_TAB)), 
		m_angelicAppearance);

	m_tabView.SetActiveTab(0);

	return 0;
}

LRESULT CAngelicOptionsDlg::
		OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	m_tabView.RemoveAllTabs();
	m_angelicBehaviour.DestroyWindow();
	m_angelicSchedule.DestroyWindow();
	m_angelicAppearance.DestroyWindow();
	return 0;
}
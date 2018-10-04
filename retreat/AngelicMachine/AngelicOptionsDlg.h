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

#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlddx.h>

#include "../WTLTabViewCtrl.h"
#include "../ColorButton.h"

#include "../OptionDialog.h"
#include "../resource.h"

#include "AngelicSettings.h"

#define ICON_BK_COLOR RGB(255, 0, 255)

// small dialog for crontab input

class CTimedBreakDlg: 
	public CDialogImpl<CTimedBreakDlg>,
	public CWinDataExchange<CTimedBreakDlg>
{
public:

	static const int IDD = IDD_TIMED_BREAK;

	CTimedBreakDlg();

	void SetCrontab(tstring crontab);
	tstring GetCrontab();

	void SetDuration(unsigned duration);
	unsigned GetDuration();

	void SetMessage(WTL::CString message);
	tstring GetMessage();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnClear(WPARAM wParam, LPARAM lParam, HWND wnd);
	LRESULT OnOK(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCancel(WPARAM wParam, LPARAM lParam, HWND wnd);

	BEGIN_DDX_MAP(CTimedBreakDlg)
		DDX_TEXT(IDC_CRON0, m_crontab[0])
		DDX_TEXT(IDC_CRON1, m_crontab[1])
		DDX_TEXT(IDC_CRON2, m_crontab[2])
		DDX_TEXT(IDC_CRON3, m_crontab[3])
		DDX_TEXT(IDC_CRON4, m_crontab[4])
		DDX_UINT(IDC_TIMED_BREAK_DURATION, m_duration)
		DDX_TEXT(IDC_TIMED_BREAK_MESSAGE, m_message)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CTimedBreakDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:
	
	WTL::CString m_crontab[5];
	unsigned m_duration;
	WTL::CString m_message;

};

///////////////////////////////////////////////////////////////////////////////

class CAngelicBehaviourDlg: 
	public CDialogImpl<CAngelicBehaviourDlg>,
	public CWinDataExchange<CAngelicBehaviourDlg>
{
public:

	static const int IDD = IDD_ANGELIC_BEHAVIOUR;

	CAngelicBehaviourDlg(HWND hBase);

	void read(SettingTable *settings);
	void write(SettingTable *settings);
	
	void SetBase(HWND hBase)
	{
		m_hBase = hBase;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);

	BEGIN_MSG_MAP_EX(CAngelicBehaviourDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CAngelicBehaviourDlg)
		DDX_UINT(IDC_PERIOD, m_behaviour.PeriodLengthMin)
		DDX_UINT(IDC_BREAK, m_behaviour.BreakLengthMin)
		DDX_UINT(IDC_ALERT, m_behaviour.AlertBeforeMin)
		DDX_UINT(IDC_DELAY, m_behaviour.DelayTimeMin)
		DDX_UINT(IDC_DELAY_LIMIT, m_behaviour.DelayLimit)
		DDX_CHECK(IDC_SYNC, m_behaviour.SynchronizeWithSystemTime)
		DDX_CHECK(IDC_BEEP, m_behaviour.BeepAfterBreak)
		DDX_CHECK(IDC_USE_RESTRICTIONS, m_behaviour.UseRestrictionList)
		DDX_CHECK(IDC_PERIODIC_OFF, m_bDisablePeriodicBreaks)
	END_DDX_MAP()

private:

	AM_BEHAVIOUR_SETTINGS m_behaviour;

	bool m_bDisablePeriodicBreaks;

	void initSpin(HWND wnd, int pos, int min = 0, int max = UD_MAXVAL);

	HWND m_hBase;

	HWND GetContainer()
	{
		// tab control -> CAngelicOptionsDlg -> CSettingsDlg
		return m_hBase;
	}

};

///////////////////////////////////////////////////////////////////////////////

class CAngelicScheduleDlg: 
	public CDialogImpl<CAngelicScheduleDlg>
{
public:

	static const int IDD = IDD_ANGELIC_SCHEDULE;

	CAngelicScheduleDlg(HWND hBase);

	void read(SettingTable *settings);
	void write(SettingTable *settings);
		
	void SetBase(HWND hBase)
	{
		m_hBase = hBase;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnAddTimedBreak(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnDeleteTimedBreak(WPARAM wParam, LPARAM lParam, HWND wnd);
	LRESULT OnLvDblClick(LPNMHDR hdr);

	BEGIN_MSG_MAP_EX(CAngelicBehaviourDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_ADD_CRON, OnAddTimedBreak)
		COMMAND_ID_HANDLER_EX(IDC_DELETE_CRON, OnDeleteTimedBreak)
		NOTIFY_HANDLER_EX(IDC_TIMED_BREAKS_LIST, NM_DBLCLK, OnLvDblClick)
	END_MSG_MAP()

private:

	AM_TIMED_BREAKS m_timedBreaks;

	CButton m_wndAddBtn;
	CButton m_wndDeleteBtn;

	CListViewCtrl m_timedBreaksList;

	void insertTimedBreak(int n, const tstring &crontab, unsigned duration, 
		const tstring &message);

	HWND m_hBase;

	HWND GetContainer()
	{
		// tab control -> CAngelicOptionsDlg -> CSettingsDlg
		return  m_hBase;
	}

};

///////////////////////////////////////////////////////////////////////////////

class CAngelicAppearanceDlg: 
	public CDialogImpl<CAngelicAppearanceDlg>,
	public CWinDataExchange<CAngelicAppearanceDlg>
{
public:

	static const int IDD = IDD_ANGELIC_APPEARANCE;

	CAngelicAppearanceDlg(HWND hBase);

	void read(SettingTable *settings);
	void write(SettingTable *settings);

	void SetBase(HWND hBase)
	{
		m_hBase = hBase;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnImageFolderChange(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnModeChange(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnBrowseImageFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnBrowseSoundFolder(UINT uNotifyCode, int nID, CWindow wndCtl);

	BEGIN_DDX_MAP(CAngelicAppearanceDlg)
		DDX_UINT(IDC_OPACITY_LEVEL, m_appearance.TransparencyLevel)
		DDX_CHECK(IDC_HIDE_TIMER, m_hideTimer)
		DDX_CHECK(IDC_STRETCH_IMAGE, m_appearance.StretchImage)
		DDX_TEXT(IDC_SOUND_FOLDER, m_soundFolder)
		DDX_TEXT(IDC_IMAGE_FOLDER, m_imageFolder)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CAngelicAppearanceDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_IMAGE_FOLDER, OnBrowseImageFolder)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_SOUND_FOLDER, OnBrowseSoundFolder)
		COMMAND_HANDLER_EX(IDC_IMAGE_FOLDER, EN_CHANGE, OnImageFolderChange)
		COMMAND_HANDLER_EX(IDC_ANGELIC_MODE_LIST, CBN_SELCHANGE, OnModeChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

private:

	AM_APPEARANCE_SETTINGS m_appearance;
	bool m_hideTimer;

	CComboBox m_wndModeList;

	CColorButton m_wndBackgroundColor;
	CColorButton m_wndTransparentColor;
	CColorButton m_wndTimerTextColor;

	WTL::CString m_imageFolder;
	WTL::CString m_soundFolder;

	HWND m_hBase;

	HWND GetContainer()
	{
		// tab control -> CAngelicOptionsDlg -> CSettingsDlg
		return m_hBase;
	}
};

///////////////////////////////////////////////////////////////////////////////

class CAngelicOptionsDlg: 
	public COptionDialog
{
public:
	
	CAngelicOptionsDlg();

	void read(SettingTable *settings);
	void write(SettingTable *settings);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);

	BEGIN_MSG_MAP_EX(CAngelicOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

private:

	CWTLTabViewCtrl m_tabView;
	CAngelicBehaviourDlg m_angelicBehaviour;
	CAngelicScheduleDlg m_angelicSchedule;
	CAngelicAppearanceDlg m_angelicAppearance;
};
module;

#include "stdafx.h"
#include "resource.h"

#include "ColorButton.h"
#include "WTLTabViewCtrl.h"

export module GeneralSettingsSection;

import <string>;

import Settings;
import SectionDlg;

#include "debug.h"
#include "tstring.h"

//// CBehaviorSettingsDlg //////////////////////////////////////////////////////

class CBehaviorSettingsDlg :
	public CDialogImpl<CBehaviorSettingsDlg>,
	public CWinDataExchange<CBehaviorSettingsDlg>
{
public:

	static const int IDD = IDD_BEHAVIOUR_OPTIONS;

	CBehaviorSettingsDlg(HWND hBase);

	void read(Settings &settings);
	void write(Settings &settings);

	void SetBase(HWND hBase) { m_hBase = hBase; }

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);

	BEGIN_MSG_MAP_EX(CBehaviorSettingsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CBehaviorSettingsDlg)
		DDX_UINT(IDC_PERIOD, m_periodLengthMin)
		DDX_UINT(IDC_BREAK, m_breakLengthMin)
		DDX_UINT(IDC_ALERT, m_alertBeforeMin)
		DDX_UINT(IDC_DELAY, m_delayTimeMin)
		DDX_UINT(IDC_DELAY_LIMIT, m_delayLimit)
		DDX_UINT(IDC_DISABLE_FOR, m_suspendForHrs)
		DDX_CHECK(IDC_SYNC, m_synchronizeWithSystemTime)
		DDX_CHECK(IDC_BEEP, m_beepAfterBreak)
		DDX_CHECK(IDC_PERIODIC_OFF, m_disablePeriodicBreaks)
	END_DDX_MAP()

private:

	int m_periodLengthMin;
	int m_breakLengthMin;
	int m_delayTimeMin;
	int m_delayLimit;
	int m_alertBeforeMin;
	int m_suspendForHrs;
	bool m_synchronizeWithSystemTime;
	bool m_beepAfterBreak;
	int m_disablePeriodicBreaks;

	void initSpin(HWND wnd, int pos, int min = 0, int max = UD_MAXVAL);

	HWND m_hBase;

	HWND GetContainer() { return m_hBase; }
};


CBehaviorSettingsDlg::CBehaviorSettingsDlg(HWND hBase) :
	m_hBase(hBase)
{
}

void CBehaviorSettingsDlg::read(Settings &settings)
{
	m_periodLengthMin = settings.getInt(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION);
	m_breakLengthMin = settings.getInt(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);
	m_delayTimeMin = settings.getInt(Settings::DELAY_DURATION, Settings::DEFAULT_DELAY_DURATION);
	m_delayLimit = settings.getInt(Settings::DELAY_AMOUNT, Settings::DEFAULT_DELAY_AMOUNT);
	m_alertBeforeMin = settings.getInt(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);
	m_suspendForHrs = settings.getInt(Settings::SUSPENDED_DURATION, Settings::DEFAULT_SUSPENDED_DURATION);
	m_synchronizeWithSystemTime = !settings.getBoolean(Settings::PERIOD_FROM_LAUNCH, Settings::DEFAULT_PERIOD_FROM_LAUNCH);
	m_beepAfterBreak = settings.getBoolean(Settings::SOUNDS_ENABLE, Settings::DEFAULT_SOUNDS_ENABLE);
	m_disablePeriodicBreaks = !settings.getBoolean(Settings::PERIOD_ENABLE, Settings::DEFAULT_PERIOD_ENABLE);

	DoDataExchange(FALSE);

	initSpin(GetDlgItem(IDC_PERIOD_SPIN), m_periodLengthMin, 1);
	initSpin(GetDlgItem(IDC_BREAK_SPIN), m_breakLengthMin, 1);
	initSpin(GetDlgItem(IDC_ALERT_SPIN), m_alertBeforeMin, 1);
	initSpin(GetDlgItem(IDC_DELAY_SPIN), m_delayTimeMin, 1);
	initSpin(GetDlgItem(IDC_DELAY_LIMIT_SPIN), m_delayLimit, 0, 5);
	initSpin(GetDlgItem(IDC_DISABLE_FOR), m_suspendForHrs, 1, 24);
}

void CBehaviorSettingsDlg::write(Settings &settings)
{
	DoDataExchange(TRUE);

	settings.setInt(Settings::PERIOD_DURATION, m_periodLengthMin);
	settings.setInt(Settings::BREAK_DURATION, m_breakLengthMin);
	settings.setInt(Settings::DELAY_DURATION, m_delayTimeMin);
	settings.setInt(Settings::DELAY_AMOUNT, m_delayLimit);
	settings.setInt(Settings::ALERT_DURATION, m_alertBeforeMin);
	settings.setInt(Settings::SUSPENDED_DURATION, m_suspendForHrs);
	settings.setBoolean(Settings::PERIOD_FROM_LAUNCH, !m_synchronizeWithSystemTime);
	settings.setBoolean(Settings::SOUNDS_ENABLE, m_beepAfterBreak);
	settings.setBoolean(Settings::PERIOD_ENABLE, !m_disablePeriodicBreaks);
}

LRESULT CBehaviorSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

LRESULT CBehaviorSettingsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return 0;
}

void CBehaviorSettingsDlg::initSpin(HWND wnd, int pos, int min /*= 0*/, int max /*= UD_MAXVAL*/)
{
	::SendMessage(wnd, UDM_SETRANGE, 0, MAKELPARAM(max, min));
	::SendMessage(wnd, UDM_SETPOS, 0, MAKELPARAM(pos, 0));
}

//// CAppearanceOptionsDlg ////////////////////////////////////////////////////

class CAppearanceOptionsDlg :
	public CDialogImpl<CAppearanceOptionsDlg>,
	public CWinDataExchange<CAppearanceOptionsDlg>
{
public:

	static const int IDD = IDD_APPEARANCE_OPTIONS;

	CAppearanceOptionsDlg(HWND hBase);

	void read(Settings &settings);
	void write(Settings &settings);

	void SetBase(HWND hBase)
	{
		m_hBase = hBase;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnBrowseImageFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnBrowseSoundFolder(UINT uNotifyCode, int nID, CWindow wndCtl);

	BEGIN_DDX_MAP(CAppearanceOptionsDlg)
		DDX_CHECK(IDC_TRANSPARENT_REGIONS, m_transparentRegions)
		DDX_UINT(IDC_OPACITY_LEVEL, m_transparencyLevel)
		DDX_CHECK(IDC_SHOW_TIMER, m_showTimer)
		DDX_CHECK(IDC_STRETCH_IMAGE, m_stretchImage)
		DDX_TEXT(IDC_SOUND_FOLDER, m_soundFolder)
		DDX_TEXT(IDC_IMAGE_FOLDER, m_imageFolder)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CAppearanceOptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_IMAGE_FOLDER, OnBrowseImageFolder)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_SOUND_FOLDER, OnBrowseSoundFolder)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

private:

	bool m_transparentRegions;
	int m_transparencyLevel;
	bool m_stretchImage;
	bool m_showTimer;

	CComboBox m_wndModeList;

	CColorButton m_wndBackgroundColor;
	CColorButton m_wndTransparentColor;
	CColorButton m_wndTimerTextColor;

	CString m_imageFolder;
	CString m_soundFolder;

	HWND m_hBase;

	HWND GetContainer() { return m_hBase; }
};

CAppearanceOptionsDlg::CAppearanceOptionsDlg(HWND hBase) :
	m_hBase(hBase)
{
}

void CAppearanceOptionsDlg::read(Settings &settings)
{
	int modeIndex = 0;

	if (!settings.getBoolean(Settings::APPEARANCE_FULLSCREEN, Settings::DEFAULT_APPEARANCE_FULLSCREEN))
		modeIndex = 1;

	m_wndModeList.SetCurSel(modeIndex);

	m_transparencyLevel = settings.getInt(Settings::APPEARANCE_OPACITY_LEVEL, Settings::DEFAULT_APPEARANCE_OPACITY_LEVEL);

	HWND hOpacitySpin = GetDlgItem(IDC_OPACITY_SPIN);
	::SendMessage(hOpacitySpin, UDM_SETRANGE, 0, MAKELPARAM(255, 0));
	::SendMessage(hOpacitySpin, UDM_SETPOS, 0, m_transparencyLevel);

	m_imageFolder = settings.getString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T("")).c_str();

	m_soundFolder = settings.getString(Settings::SOUNDS_AUDIO_DIRECTORY, _T("")).c_str();

	m_wndBackgroundColor.SetColor(settings.getInt(Settings::APPEARANCE_BACKGROUND_COLOR, Settings::DEFAULT_APPEARANCE_BACKGROUND_COLOR));

	m_transparentRegions = settings.getBoolean(Settings::APPEARANCE_TRANSPARENT, Settings::DEFAULT_APPEARANCE_TRANSPARENT);

	m_showTimer = settings.getBoolean(Settings::APPEARANCE_SHOW_TIMER, Settings::DEFAULT_APPEARANCE_SHOW_TIMER);
	m_wndTimerTextColor.SetColor(settings.getInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, Settings::DEFAULT_APPEARANCE_TIMER_TEXT_COLOR));

	DoDataExchange(FALSE);
}

void CAppearanceOptionsDlg::write(Settings &settings)
{
	DoDataExchange(TRUE);

	settings.setBoolean(Settings::APPEARANCE_FULLSCREEN, !m_wndModeList.GetCurSel());

	settings.setInt(Settings::APPEARANCE_OPACITY_LEVEL, m_transparencyLevel);

	settings.setString(Settings::APPEARANCE_IMAGE_DIRECTORY, (LPCTSTR)m_imageFolder);

	settings.setString(Settings::SOUNDS_AUDIO_DIRECTORY, (LPCTSTR)m_soundFolder);

	settings.setInt(Settings::APPEARANCE_BACKGROUND_COLOR, m_wndBackgroundColor.GetColor());

	settings.setBoolean(Settings::APPEARANCE_TRANSPARENT, m_transparentRegions);

	settings.setBoolean(Settings::APPEARANCE_SHOW_TIMER, m_showTimer);

	settings.setInt(Settings::APPEARANCE_TIMER_TEXT_COLOR, m_wndTimerTextColor.GetColor());
}

LRESULT CAppearanceOptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	m_wndModeList.Attach(GetDlgItem(IDC_WINDOW_MODE_LIST));

	m_wndModeList.AddString(CString(MAKEINTRESOURCE(IDS_FULLSCREEN_MODE_ITEM)));
	m_wndModeList.AddString(CString(MAKEINTRESOURCE(IDS_WINDOWED_MODE_ITEM)));
	
	m_wndModeList.SetCurSel(0);

	m_wndBackgroundColor.SubclassWindow(GetDlgItem(IDC_BACKGROUND_COLOR));
	m_wndTimerTextColor.SubclassWindow(GetDlgItem(IDC_TIMER_TEXT_COLOR));

	CString defaultText(MAKEINTRESOURCE(IDS_DEFAULT_COLOR_TEXT));
	CString customText(MAKEINTRESOURCE(IDS_CUSTOM_COLOR_TEXT));

	m_wndBackgroundColor.SetDefaultText(defaultText);
	m_wndBackgroundColor.SetCustomText(customText);
	m_wndTransparentColor.SetDefaultText(defaultText);
	m_wndTransparentColor.SetCustomText(customText);
	m_wndTimerTextColor.SetDefaultText(defaultText);
	m_wndTimerTextColor.SetCustomText(customText);

	m_wndTransparentColor.SetDefaultColor(RGB(128, 128, 128));

	return 0;
}

LRESULT CAppearanceOptionsDlg::OnBrowseImageFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CString title(MAKEINTRESOURCE(IDS_BROWSE_IMAGE_FOLDER_TITLE));

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

LRESULT CAppearanceOptionsDlg::OnBrowseSoundFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CString title(MAKEINTRESOURCE(IDS_BROWSE_SOUND_FOLDER_TITLE));

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

//// CGeneralSettingsSection ///////////////////////////////////////////////////

export class CGeneralSettingsSection: public CSectionDlg
{
public:

	CGeneralSettingsSection();

	virtual void read(Settings &settings) override;
	virtual void write(Settings &settings) override;

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&);

	BEGIN_MSG_MAP_EX(CGeneralSettingsSection)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

private:

	CWTLTabViewCtrl m_tabView;
	CBehaviorSettingsDlg m_behaviourOptionsDlg;
	CAppearanceOptionsDlg m_appearanceOptionsDlg;
};


CGeneralSettingsSection::CGeneralSettingsSection():
	CSectionDlg(IDD_GENERAL_OPTIONS),
	m_behaviourOptionsDlg(NULL),
	m_appearanceOptionsDlg(NULL)
{
	SetOptionsCategoryName(CString(MAKEINTRESOURCE(IDS_GENERAL_OPTIONS_SECTION_NAME)));
}

void CGeneralSettingsSection::read(Settings &settings)
{
	m_behaviourOptionsDlg.read(settings);
	m_appearanceOptionsDlg.read(settings);
}

void CGeneralSettingsSection::write(Settings &settings)
{
	m_behaviourOptionsDlg.write(settings);
	m_appearanceOptionsDlg.write(settings);
}

LRESULT CGeneralSettingsSection::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	CRect rect;
	GetClientRect(&rect);
	m_tabView.Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE);

	m_behaviourOptionsDlg.Create(m_tabView.m_hWnd);
	m_appearanceOptionsDlg.Create(m_tabView.m_hWnd);
	
	m_behaviourOptionsDlg.SetBase(GetParent());
	m_appearanceOptionsDlg.SetBase(GetParent());

	m_tabView.AddTab(CString(MAKEINTRESOURCE(IDS_BEHAVIOUR_TAB)), m_behaviourOptionsDlg);
	m_tabView.AddTab(CString(MAKEINTRESOURCE(IDS_APPEARANCE_TAB)), m_appearanceOptionsDlg);

	m_tabView.SetActiveTab(0);

	return 0;
}

LRESULT CGeneralSettingsSection::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	m_tabView.RemoveAllTabs();
	m_behaviourOptionsDlg.DestroyWindow();
	m_appearanceOptionsDlg.DestroyWindow();

	return 0;
}
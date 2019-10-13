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

#ifdef ENSO_LIBRARY
#	include <Python.h>
#endif

#include "DispatchWnd.h"

#include <algorithm>

#include "boost/shared_ptr.hpp"
#include "boost/foreach.hpp"

#define foreach BOOST_FOREACH


#include "boost/lexical_cast.hpp"

#include "AboutDlg.h"

#include "Utility/SystemHelper.h"
#include "AngelicMachine/AngelicSettingsUIFactory.h"

CDispatchWnd::CDispatchWnd() :
m_disableMenu(0),
m_hResourceLib(NULL),
m_aboutRunning(false),
m_unlockOptions(false)
{
	// commands which menu item state could be disabled/enabled by machines
	m_alterableCommands.reserve(6);
	m_alterableCommands.push_back(ID_MENU_DISABLE);
	m_alterableCommands.push_back(ID_MENU_DELAY);
	m_alterableCommands.push_back(ID_MENU_SKIP);
	m_alterableCommands.push_back(ID_MENU_TAKEBREAK);
	m_alterableCommands.push_back(ID_MENU_OPTIONS);
	m_alterableCommands.push_back(ID_MENU_EXIT);
}

void CDispatchWnd::showTrayIcon()
{
	int icon_id = IDI_TRAY;

#ifdef ENSO_LIBRARY
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();
	PyObject *config = PyImport_ImportModule("enso.config");
	PyObject *dict = PyModule_GetDict(config);
	PyObject *theme = PyDict_GetItem(dict, PyUnicode_FromString("COLOR_THEME"));
	
	int show_icon_i = 0;
	if (PyDict_Contains(dict, PyUnicode_FromString("RETREAT_SHOW_ICON"))) {
		PyObject *show_icon = PyDict_GetItem(dict, PyUnicode_FromString("RETREAT_SHOW_ICON"));
		show_icon_i = PyObject_IsTrue(show_icon);
	}

	const char *theme_s = PyUnicode_AsUTF8(theme);
	Py_DecRef(config);
	PyGILState_Release(gstate);

	if (show_icon_i == -1 || !show_icon_i)
		return;

	if (!strcmp(theme_s, "amethyst"))
		icon_id = IDI_TRAY_AMETHYST;
#endif

	// set tray icon
	HICON hIcon = (HICON)LoadImage(
		ModuleHelper::GetModuleInstance(), 
		MAKEINTRESOURCE(icon_id), 
		IMAGE_ICON, 16, 16, LR_SHARED
		);

	WTL::CString appName;
	appName.LoadString(IDS_APP_NAME);

	InstallIcon(appName, hIcon, IDR_SHELL_MENU);
}

LRESULT CDispatchWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_tzset();

	// initialize random number generator
	srand((unsigned)time(NULL));

	m_taskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

	// get settings
	::InitializeSettingManager(m_settingManager);

	tstring settings_dir;

#ifdef ENSO_LIBRARY
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyObject* config = PyImport_ImportModule("enso.config");
	PyObject* dict = PyModule_GetDict(config);
	PyObject* enso_user_dir = PyDict_GetItem(dict, PyUnicode_FromString("ENSO_USER_DIR"));

	USES_CONVERSION;
	settings_dir = A2T(PyUnicode_AsUTF8(enso_user_dir));
	settings_dir.append(_T("/retreat.cfg"));

	Py_DecRef(config);
	PyGILState_Release(gstate);
#else
	CAtlString s;
	s.LoadStringW(IDS_APP_NAME);

	settings_dir = SystemHelper::getInstance()->getSettingsForCurrentUser(
		s.GetString(), CFG_FILE
	);
#endif

	m_settingManager.setSettingsFile(settings_dir);


	m_settingManager.loadSettings();

	tstring language = 
		SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
		.getStringSetting(SM_LANGUAGE);

	loadResourceLib(language);

	m_lockOptions = 
		SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
		.getIntSetting(SM_LOCK);

	MachineWrapper::readGlobalRestrictions(
		SettingManagerAdapter(m_settingManager, RM_RESTRICTIONS_SECTION));

	// start scheduler
	m_pScheduler = message_scheduler_ptr_t(new MessageScheduler(m_hWnd));
	m_pScheduler->startScheduling();

	CRect workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	// Angelic machine
	// custom machines could be added by the same way
	AngelicMachine *pAngelicMachine = 
		new AngelicMachine(ANGELIC_MACHINE_NAME, m_hWnd);

	pAngelicMachine->setWorkArea(workArea);

	machinewp_ptr_t pAngelicMachineWrapper = 
		machinewp_ptr_t(
			new MachineWrapper(
				pAngelicMachine, 
				new AngelicSettingsUIFactory()
			)
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_DISABLE, 
		AngelicMachine::USER_DISABLE, 
		AngelicMachine::USER_DISABLE_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_DELAY, 
		AngelicMachine::USER_DELAY, 
		AngelicMachine::USER_DELAY_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_SKIP, 
		AngelicMachine::USER_SKIP, 
		AngelicMachine::USER_SKIP_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_TAKEBREAK, 
		AngelicMachine::USER_LOCK, 
		AngelicMachine::USER_LOCK_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_OPTIONS, 
		-1,
		AngelicMachine::USER_OPTIONS_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_MENU_EXIT, 
		-1,
		AngelicMachine::USER_TERMINATE_FLAG
		);

	pAngelicMachineWrapper->addUserCommand(
		ID_USER_UNLOCK_COMMAND, 
		AngelicMachine::USER_UNLOCK,
		0
		);

	m_machines.push_back(pAngelicMachineWrapper);
	ANGELIC_MACHINE_INDEX = m_machines.size() - 1;

	showTrayIcon();
	SetDefaultItem(-1);

	startMachines(m_machines);

	return 0;
}

LRESULT CDispatchWnd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	unsigned command = LOWORD(wParam);

	machinewp_ptr_t angelicMachine = m_machines[ANGELIC_MACHINE_INDEX];

	if (!angelicMachine->commandAvaliable(command))
	{
		return 0;
	}

	switch (command)
	{
	case ID_MENU_EXIT:
		if (m_lockOptions)
		{
			m_unlockOptions = true;

			WTL::CString msg(MAKEINTRESOURCE(IDS_UNLOCK_OPTIONS_MESSAGE));
			WTL::CString title(MAKEINTRESOURCE(IDS_APP_NAME));

			::MessageBox(0, msg, title, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			PostMessage(WM_CLOSE);
		}
		break;
	case ID_MENU_OPTIONS:
		showOptions();
		break;
	case ID_MENU_ABOUT:
		if (!m_aboutRunning)
		{
			disableMenu();
			m_aboutRunning = true;

			CAboutDlg aboutDlg;
			aboutDlg.DoModal();

			m_aboutRunning = false;
			enableMenu();

		}
		break;
	default:
		{
			for (size_t i = 0; i < m_machines.size(); ++i)
			{
				m_machines[i]->userEvent(
					MessageSchedulerAdapter(*m_pScheduler, i), command);
			}
		}
	}

	return 0;
}

LRESULT CDispatchWnd::OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	showTrayIcon();

	return 0;
}


LRESULT CDispatchWnd::OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	switch (wParam)
	{
	case PBT_APMSUSPEND:
		suspendMachines(m_machines, true);
		m_settingManager.saveSettings();
		break;
	case PBT_APMRESUMECRITICAL:
		stopMachines(m_machines, false);
		m_settingManager.loadSettings();
		
		Sleep(1000);
		startMachines(m_machines);
		break;
	case PBT_APMRESUMESUSPEND:
		Sleep(1000);
		suspendMachines(m_machines, false);
		break;
	}

	return 0;
}

LRESULT CDispatchWnd::OnQueryEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	if (m_unlockOptions && m_lockOptions == 1)
	{
		SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
			.setIntSetting(SM_LOCK, 0);

		m_settingManager.saveSetting(SM_GENERAL_SECTION, SM_LOCK);
	}

	return TRUE;
}

LRESULT CDispatchWnd::OnSchedulerMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	//ATLTRACE("Got event: 0x%X\n", (int)wParam);

	int dispatchIndex = HIWORD(wParam);
	int event = LOWORD(wParam);

	m_machines[dispatchIndex]->timerEvent(
		MessageSchedulerAdapter(*m_pScheduler, dispatchIndex), event, 
		lParam);

	return 0;
}

LRESULT CDispatchWnd::OnDisplayGlobalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	CAtlString s;
	s.LoadStringW(IDS_APP_NAME);

	MessageBox((LPCTSTR)wParam, s.GetString(), MB_OK | MB_ICONINFORMATION);
	free((void *)wParam);

	return 0;
}


LRESULT CDispatchWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	stopMachines(m_machines);

	m_pScheduler->stopScheduling();
	m_settingManager.saveSettings();

	RemoveIcon();
	DestroyWindow();
	return 0;
}

LRESULT CDispatchWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_hResourceLib != NULL)
	{
		::FreeLibrary(m_hResourceLib);
	}

	PostQuitMessage(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////

void CDispatchWnd::enableMenu()
{
	m_disableMenu -= 1;
}

void CDispatchWnd::disableMenu()
{
	m_disableMenu += 1;
}

// this function is used by tray icon helper class
bool CDispatchWnd::IsMenuDisabled()
{
	return m_disableMenu > 0;
}

bool CDispatchWnd::IsLocked()
{
	machinewp_ptr_t angelicMachine = m_machines[ANGELIC_MACHINE_INDEX];

	return IsMenuDisabled() || !angelicMachine->commandAvaliable(ID_MENU_EXIT);
}

// this function is used by tray icon helper class
void CDispatchWnd::PrepareContextMenu(HMENU hMenu)
{
	// angelic machine always exists
	machinewp_ptr_t angelicMachine = m_machines[ANGELIC_MACHINE_INDEX];

	if (angelicMachine->flags() & AngelicMachine::STATE_DISABLED_FLAG)
	{
		setMenuItemText(hMenu, ID_MENU_DISABLE, IDS_ENABLE_MENU_ITEM_NAME);
	}

	foreach(int command, m_alterableCommands)
	{
		if (!angelicMachine->commandAvaliable(command))
		{
			EnableMenuItem(hMenu, command, MF_DISABLED | MF_GRAYED);
		}
	}

	if (m_lockOptions)
	{
		EnableMenuItem(hMenu, ID_MENU_OPTIONS, MF_DISABLED | MF_GRAYED);
		setMenuItemText(hMenu, ID_MENU_EXIT, IDS_UNLOCK_OPTIONS_ITEM);
	}
}

///////////////////////////////////////////////////////////////////////////

void CDispatchWnd::startMachines(machine_vector_t &machines)
{
	for (size_t i = 0; i < machines.size(); ++i)
	{
		machines[i]->readSettings(
			SettingManagerAdapter(m_settingManager, machines[i]->name())
			);

		machines[i]->start(MessageSchedulerAdapter(*m_pScheduler, i));
	}
}

void CDispatchWnd::stopMachines(machine_vector_t &machines, bool save /*= true*/)
{
	for (unsigned i = 0; i < machines.size(); ++i)
	{
		if (save)
		{
			machines[i]->writeSettings(
				SettingManagerAdapter(m_settingManager, machines[i]->name()));
		}

		machines[i]->terminate(MessageSchedulerAdapter(*m_pScheduler, i));
	}
}

void CDispatchWnd::suspendMachines(machine_vector_t &machines, bool suspend)
{
	for (unsigned i = 0; i < machines.size(); ++i)
	{
		if (suspend)
		{
			machines[i]->writeSettings(
				SettingManagerAdapter(m_settingManager, machines[i]->name()));
		}

		machines[i]->suspend(
			MessageSchedulerAdapter(*m_pScheduler, i), suspend);
	}
}

void CDispatchWnd::setMenuItemText(HMENU hMenu, int id, int resource)
{
	MENUITEMINFO itemInfo = 
	{sizeof(MENUITEMINFO), MIIM_STRING | MIIM_FTYPE, MFT_STRING, };

	WTL::CString itemText(MAKEINTRESOURCE(resource));

	itemInfo.dwTypeData = const_cast<TCHAR *>((const TCHAR *)itemText);

	SetMenuItemInfo(hMenu, id, FALSE, &itemInfo);
}

void CDispatchWnd::showOptions()
{
	disableMenu();

	tstring language = 
		SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
			.getStringSetting(SM_LANGUAGE);

	// create settings dialog (full of side effects on setting manager)
	// machines should be untouched
	CSettingsDlg settingsDlg(m_settingManager, m_machines);

	if (settingsDlg.DoModal(m_hWnd) == IDOK || settingsDlg.Applied())
	{
		// angelic machine always exists
		machinewp_ptr_t angelicMachine = m_machines[ANGELIC_MACHINE_INDEX];

		if (angelicMachine->flags() & AngelicMachine::USER_OPTIONS_FLAG)
		// Angelic Machine allows options change
		{
			stopMachines(m_machines, false);

			MachineWrapper::readGlobalRestrictions(
				SettingManagerAdapter(
					m_settingManager, 
					RM_RESTRICTIONS_SECTION
				));

			tstring new_language = 
				SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
					.getStringSetting(SM_LANGUAGE);

			if (new_language != language)
			{
				loadResourceLib(new_language);
			}

			if (settingsDlg.GetLockSettings())
			{
				m_lockOptions = 1;

				SettingManagerAdapter(m_settingManager, SM_GENERAL_SECTION)
					.setIntSetting(SM_LOCK, m_lockOptions);
			}

			m_settingManager.saveSettings();

			// machines will read settings before start
			startMachines(m_machines);
		}
		else
		{
			MessageBeep(MB_ICONWARNING);
		}
	}

	enableMenu();
}

void CDispatchWnd::loadResourceLib(const tstring &name)
{
	if (m_hResourceLib != NULL)
	{
		::FreeLibrary(m_hResourceLib);
	}

	tstring lib_name = SystemHelper::getInstance()->getModuleDir();
	lib_name.append(_T("\\language\\"));
	lib_name.append(name);

	m_hResourceLib = ::LoadLibrary(lib_name.c_str());

	if (m_hResourceLib != NULL)
	{
		_Module.SetResourceInstance(m_hResourceLib);
	}
	else
	{
		_Module.SetResourceInstance(::GetModuleHandle(NULL));
	}
}
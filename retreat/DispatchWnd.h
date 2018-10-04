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

#include <vector>

#include "settings.h"
#include "trayiconimpl.h"

#include "SettingsDlg.h"

#include "MachineWrapper.h"
#include "AngelicMachine/AngelicMachine.h"

#include "scheduler/scheduler.h"

#define CFG_FILE _T("retreat.cfg")

#define DISPATCH_WND_CLASS _T("DispatchWindow")

typedef CWinTraits<WS_BORDER | WS_SYSMENU> DispatchTraits;

class CDispatchWnd: public CWindowImpl<CDispatchWnd, CWindow, DispatchTraits>,
					public CTrayIconImpl<CDispatchWnd>
{
public:

	CDispatchWnd();

	bool IsMenuDisabled();
	void enableMenu();
	void disableMenu();

	bool IsLocked();

	void PrepareContextMenu(HMENU hMenu);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTaskBarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnPowerBroadcast(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnQueryEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnSchedulerMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnDisplayGlobalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DECLARE_WND_CLASS(DISPATCH_WND_CLASS)

	BEGIN_MSG_MAP(CDispatchWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(m_taskbarCreated, OnTaskBarCreated);
		MESSAGE_HANDLER(WM_POWERBROADCAST, OnPowerBroadcast)
		MESSAGE_HANDLER(WM_QUERYENDSESSION, OnQueryEndSession)
		MESSAGE_HANDLER(WM_DISPLAY_GLOBAL_MESSAGE, OnDisplayGlobalMessage)
		MESSAGE_HANDLER(
			MessageScheduler::WM_DISPATCH_EVENT, 
			OnSchedulerMessage
			)
		CHAIN_MSG_MAP(CTrayIconImpl<CDispatchWnd>)
	END_MSG_MAP()

protected:

	setting_manager_t m_settingManager;

	typedef boost::shared_ptr<MessageScheduler> message_scheduler_ptr_t;
	message_scheduler_ptr_t m_pScheduler;

	machine_vector_t m_machines;
	
	int ANGELIC_MACHINE_INDEX;

	std::vector<int> m_alterableCommands;

	int m_disableMenu;
	int m_lockOptions;
	bool m_unlockOptions;

	int m_taskbarCreated;

	HMODULE m_hResourceLib;

	bool m_aboutRunning;

	void startMachines(machine_vector_t &machines);
	void stopMachines(machine_vector_t &machines, bool save = true);
	void suspendMachines(machine_vector_t &machines, bool suspend);

	void setMenuItemText(HMENU hMenu, int id, int resource);

	void showOptions();
	void showTrayIcon();
	void loadResourceLib(const tstring &name);

};
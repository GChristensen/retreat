/*
 * Angelic Retreat
 * Copyright (C) 2017 G. Christensen
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

#include "StdAfx.h"
#include "GlobalEvent.h"

#include <process.h>

using namespace std;
using namespace boost;

GlobalEvent::GlobalEvent(const tstring & event_name, AngelicState * state, AngelicMachine * machine,
	SimpleScheduler * scheduler, int event, DWORD timeout):
	m_state(state), m_machine(machine), m_scheduler(scheduler), m_event(event), m_timeout(timeout)
{
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &sd;

	m_hGlobalEvent = CreateEvent(&sa, FALSE, FALSE, event_name.c_str());
	m_hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_hEventThread =
			(HANDLE)_beginthreadex(NULL, 0, eventThread, (void *)this, 0, NULL);

}

GlobalEvent::~GlobalEvent()
{
	cancel();
	WaitForSingleObject(m_hEventThread, 500);
	CloseHandle(m_hGlobalEvent);
	CloseHandle(m_hInterruptEvent);
	CloseHandle(m_hEventThread);
}

void GlobalEvent::cancel()
{
	SetEvent(m_hInterruptEvent);
}

unsigned GlobalEvent::eventThread(void * param)
{
	GlobalEvent *global_event = (GlobalEvent *)param;

	HANDLE events[] = { global_event->m_hGlobalEvent, global_event->m_hInterruptEvent };
	DWORD event_fired = WaitForMultipleObjects(2, events, FALSE, global_event->m_timeout + 1000);

	if (event_fired == WAIT_OBJECT_0) {
		
		PostMessage(global_event->m_machine->m_parentWindow, WM_COMMAND, global_event->m_event, 0);
//		global_event->m_state->userEvent(global_event->m_machine, global_event->m_scheduler,
//			global_event->m_event);
	}

	return 0;
}

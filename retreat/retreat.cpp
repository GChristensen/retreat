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


#include <time.h>

#include "resource.h"

#include "DispatchWnd.h"

#include "utility/SystemHelper.h"

#include "LimitSingleInstance.h"

CAppModule _Module;

CDispatchWnd wndMain;

int Run(LPTSTR, HINSTANCE hInstance, int nCmdShow)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	SystemHelper::getInstance()->setAppInstanceHandle(hInstance);

	if(wndMain.Create(0 /*HWND_MESSAGE*/) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	
	int nRet = theLoop.Run();

	SystemHelper::releaseInstance();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	CLimitSingleInstance limit(_T("%AngelicRetreat%"));

	if (limit.IsAnotherInstanceRunning())
	{
		return 0;
	}

	HRESULT hRes;


	hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#ifndef ENSO_LIBRARY
	ATLASSERT(SUCCEEDED(hRes));
#endif 

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_TAB_CLASSES | ICC_STANDARD_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, hInstance, nCmdShow);

	_Module.Term();

	::CoUninitialize();

	return nRet;
}

#ifdef ENSO_LIBRARY

HINSTANCE hDllInst = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	//if (fdwReason == DLL_PROCESS_ATTACH) {
		hDllInst = hinstDLL;
		_Module.SetResourceInstance(hinstDLL);
	//}

	return TRUE;
}

DWORD WINAPI pyenso_thread(LPVOID lpParam) {
	return _tWinMain(hDllInst, 0, 0, 0);
}

static PyObject *
pyenso_retreat_start(PyObject *self, PyObject *args) {
	HANDLE hThread = CreateThread(NULL, 0, pyenso_thread, 0, 0, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_stop(PyObject *self) {
	wndMain.PostMessage(WM_CLOSE, 0, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_take_break(PyObject *self) {
	wndMain.PostMessage(WM_COMMAND, ID_MENU_TAKEBREAK, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_delay(PyObject *self) {
	wndMain.PostMessage(WM_COMMAND, ID_MENU_DELAY, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_skip(PyObject *self) {
	wndMain.PostMessage(WM_COMMAND, ID_MENU_SKIP, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_options(PyObject *self) {
	wndMain.PostMessage(WM_COMMAND, ID_MENU_OPTIONS, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_about(PyObject *self) {
	wndMain.PostMessage(WM_COMMAND, ID_MENU_ABOUT, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
pyenso_retreat_is_locked(PyObject *self) {
	return PyBool_FromLong((long)wndMain.IsLocked());
}


static PyMethodDef enso_functions[] = {
	{"start", (PyCFunction)pyenso_retreat_start, METH_VARARGS},
	{"stop", (PyCFunction)pyenso_retreat_stop, METH_NOARGS},
	{"take_break", (PyCFunction)pyenso_retreat_take_break, METH_NOARGS},
	{"delay", (PyCFunction)pyenso_retreat_delay, METH_NOARGS},
	{"skip", (PyCFunction)pyenso_retreat_skip, METH_NOARGS},
	{"options", (PyCFunction)pyenso_retreat_options, METH_NOARGS},
	{"about", (PyCFunction)pyenso_retreat_about, METH_NOARGS},
//	{"version", (PyCFunction)pyenso_retreat_version, METH_NOARGS},
	{"is_locked", (PyCFunction)pyenso_retreat_is_locked, METH_NOARGS},
	{NULL, NULL, 0, NULL},
};

struct module_state {
	PyObject *error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static PyObject *
error_out(PyObject *m) {
	struct module_state *st = GETSTATE(m);
	PyErr_SetString(st->error, "something bad happened");
	return NULL;
}

static int enso_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int enso_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static struct PyModuleDef moduledef = {
		PyModuleDef_HEAD_INIT,
		"enso.contrib._retreat",
		NULL,
		sizeof(struct module_state),
		enso_functions,
		NULL,
		enso_traverse,
		enso_clear,
		NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit__retreat(void)
{
	PyObject *m = PyModule_Create(&moduledef);

	if (m == NULL)
		INITERROR;

	return m;
}
#endif
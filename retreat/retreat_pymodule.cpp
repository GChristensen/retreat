#ifdef PYTHON_MODULE

#include <string>

#include "stdafx.h"
#include <Python.h>

#include "resource.h"

#include "tstring.h"

#define WM_GET_LOCK_STATE (WM_USER + 10)

extern HWND hDispatchWnd;
int entry_win32(HINSTANCE hInstance, LPTSTR lpstrCmdLine, int nCmdShow);

HINSTANCE hDllInst = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	hDllInst = hinstDLL;
	
	return TRUE;
}

DWORD WINAPI pyenso_thread(LPVOID lpParam) {
	return entry_win32(hDllInst, 0, 0);
}

static PyObject*
pyenso_retreat_start(PyObject* self, PyObject* args) {
	HANDLE hThread = CreateThread(NULL, 0, pyenso_thread, 0, 0, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_stop(PyObject* self) {
	PostMessage(hDispatchWnd, WM_CLOSE, 0, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_take_break(PyObject* self) {
	PostMessage(hDispatchWnd, WM_COMMAND, ID_MENU_TAKEBREAK, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_delay(PyObject* self) {
	PostMessage(hDispatchWnd, WM_COMMAND, ID_MENU_DELAY, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_skip(PyObject* self) {
	PostMessage(hDispatchWnd, WM_COMMAND, ID_MENU_SKIP, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_options(PyObject* self) {
	PostMessage(hDispatchWnd, WM_COMMAND, ID_MENU_OPTIONS, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_about(PyObject* self) {
	PostMessage(hDispatchWnd, WM_COMMAND, ID_MENU_ABOUT, 0);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject*
pyenso_retreat_is_locked(PyObject* self) {
	return PyBool_FromLong((long)SendMessage(hDispatchWnd, WM_GET_LOCK_STATE, 0, 0));
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
	PyObject* error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static PyObject*
error_out(PyObject* m) {
	struct module_state* st = GETSTATE(m);
	PyErr_SetString(st->error, "something bad happened");
	return NULL;
}

static int enso_traverse(PyObject* m, visitproc visit, void* arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int enso_clear(PyObject* m) {
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
	PyObject* m = PyModule_Create(&moduledef);

	if (m == NULL)
		INITERROR;

	return m;
}

//// Utility Functions ////////////////////////////////////////////////////////

int getPythonShowIcon() {
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();
	PyObject* config = PyImport_ImportModule("enso.config");
	PyObject* dict = PyModule_GetDict(config);
	PyObject* theme = PyDict_GetItem(dict, PyUnicode_FromString("COLOR_THEME"));

	int show_icon_i = 0;
	if (PyDict_Contains(dict, PyUnicode_FromString("RETREAT_SHOW_ICON"))) {
		PyObject* show_icon = PyDict_GetItem(dict, PyUnicode_FromString("RETREAT_SHOW_ICON"));
		show_icon_i = PyObject_IsTrue(show_icon);
	}

	const char* theme_s = PyUnicode_AsUTF8(theme);
	Py_DecRef(config);
	PyGILState_Release(gstate);

	if (show_icon_i == -1 || !show_icon_i)
		return 2;

	return !strcmp(theme_s, "amethyst");
}

tstring getPythonConfigFilePath(const tstring &configFile) {
	tstring result;

	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyObject* config = PyImport_ImportModule("enso.config");
	PyObject* dict = PyModule_GetDict(config);
	PyObject* enso_user_dir = PyDict_GetItem(dict, PyUnicode_FromString("ENSO_USER_DIR"));

	USES_CONVERSION;
	result = A2T(PyUnicode_AsUTF8(enso_user_dir));
	result.append(_T("\\"));
	result.append(configFile);

	Py_DecRef(config);
	PyGILState_Release(gstate);

	return result;
}


#endif
module; 

#include "wtl.h"
#include <tlhelp32.h>
#include <pathcch.h>
#include <shlobj.h>
#include <dshow.h>

#include "resource.h"
#include "python_module.h"

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include "debug.h"
#include "tstring.h"

module system;



const TCHAR* SETTINGS_DIR = _T("Enso Retreat");

tstring getBundledFilePath(const tstring& fileName)
{
	TCHAR moduleDir[MAX_PATH];

	GetModuleFileName(_Module.GetModuleInstance(), moduleDir, MAX_PATH);

	TCHAR* p = _tcsrchr(moduleDir, _T('\\'));

	*(p + 1) = 0;

	tstring result = moduleDir;
	result.append(fileName);

	return result;
}


tstring getConfigFilePath(const tstring& configFile) {
#ifdef PYTHON_MODULE
	// inside Enso the config lives in the Enso user directory
	return getPythonConfigFilePath(configFile);
#else
	TCHAR appDataDirPath[MAX_PATH];

	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataDirPath);

	tstring configFilePath = appDataDirPath;

	if (!configFilePath.ends_with(_T("\\")))
		configFilePath.append(_T("\\"));

	configFilePath.append(SETTINGS_DIR);
	configFilePath.append(_T("\\"));

	// check whether application directory exists
	if (::GetFileAttributes(configFilePath.c_str()) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(configFilePath.c_str(), NULL);

	configFilePath.append(configFile);
	return configFilePath;
#endif
}

void searchForFiles(const tstring &dir, const tstring &ext, std::vector<tstring> &files)
{
	tstring pattern = dir + _T("\\*.") + ext;
	CFindFile finder;

	if (finder.FindFile(pattern.c_str())) {
		do {
			if (finder.IsDots() || finder.IsDirectory())
				continue;

			files.emplace_back((LPCTSTR)finder.GetFilePath());
		} while (finder.FindNextFile());
	}

	finder.Close();
}

tstring getRandomFile(const tstring& dir, const std::vector<tstring>& exts) {
	std::vector<tstring> files;

	for (const tstring& ext : exts)
		searchForFiles(dir, ext, files);

	if (files.empty())
		return _T("");

	return files[rand() % files.size()];
}

void playAudioFile(tstring &filePath) {
	auto pFilePath = std::make_shared<tstring>(filePath);

	auto playProc = [pFilePath]() {
		HRESULT hr = ::CoInitialize(NULL);

		IGraphBuilder* pGraph = NULL;
		IMediaControl* pControl = NULL;
		IMediaEvent* pEvent = NULL;

		hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&pGraph);

		if (FAILED(hr)) {
			DBGLOG(_T("ERROR - Could not create the Filter Graph Manager."));
			return;
		}

		hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
		hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);

		hr = pGraph->RenderFile(pFilePath->c_str(), NULL);
		if (SUCCEEDED(hr)) {
			hr = pControl->Run();
			if (SUCCEEDED(hr)) {
				long evCode;
				pEvent->WaitForCompletion(INFINITE, &evCode);
			}
		}

		pEvent->Release();
		pControl->Release();
		pGraph->Release();

		::CoUninitialize();
	};

	std::thread playThread(playProc);
	playThread.detach();
}

void playSystemSound() {
	MessageBeep(MB_ICONASTERISK);
}

std::vector<tstring> getRunningProcessPaths() {
	std::vector<tstring> result;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return result;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &entry)) {
		do {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
			if (hProcess != NULL) {
				TCHAR processPath[MAX_PATH], canonizedPath[MAX_PATH] = { 0, };
				DWORD pathSize = MAX_PATH;
				if (QueryFullProcessImageName(hProcess, 0, processPath, &pathSize)
						&& S_OK == PathCchCanonicalize(canonizedPath, MAX_PATH, processPath))
					result.push_back(canonizedPath);
				CloseHandle(hProcess);
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);

	auto lessNoCase = [](const tstring& a, const tstring& b) { return _tcsicmp(a.c_str(), b.c_str()) < 0; };
	auto equalNoCase = [](const tstring& a, const tstring& b) { return _tcsicmp(a.c_str(), b.c_str()) == 0; };

	std::sort(result.begin(), result.end(), lessNoCase);
	result.erase(std::unique(result.begin(), result.end(), equalNoCase), result.end());

	return result;
}

bool isProcessRunning(std::vector<tstring> &processes) {
	if (processes.empty())
		return false;

	for (const tstring& running : getRunningProcessPaths())
		for (const tstring& process : processes)
			if (!_tcsicmp(process.c_str(), running.c_str()))
				return true;

	return false;
}
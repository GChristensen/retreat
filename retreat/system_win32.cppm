module; 

#include "stdafx.h"
#include <tlhelp32.h>
#include <pathcch.h>
#pragma comment(lib,"Pathcch")
#include <shlobj.h>
#include <dshow.h>
#pragma comment(lib,"Strmiids")

export module system:win32;

import <memory>;
import <vector>;
import <string>;
import <thread>;
import <chrono>;

#include "debug.h"
#include "tstring.h"

const TCHAR* SETTINGS_DIR = _T("Enso Retreat");

export tstring getConfigFilePath(const tstring& configFile) {
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
}

export void searchForFiles(const tstring &dir, const tstring &ext, std::vector<tstring> &files)
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

export tstring getRandomFile(const tstring& dir, const std::vector<tstring>& exts) {
	std::vector<tstring> files;

	for (const tstring& ext : exts)
		searchForFiles(dir, ext, files);

	if (files.empty())
		return _T("");

	return files[rand() % files.size()];
}

export void playAudioFile(tstring &filePath) {
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

export void displayMessage(tstring& text) {
	auto pText = std::make_shared<tstring>(text);

	auto displayProc = [pText]() {
		MessageBox(0, pText->c_str(), _T("Enso Retreat"), MB_OK);
	};

	std::thread displayThread(displayProc);
	displayThread.detach();
}


export bool isProcessRunning(std::vector<tstring> &processes) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	auto checkProcess = [&processes](PROCESSENTRY32 &entry) { 
		bool matches = false;
	
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, entry.th32ProcessID);
		if (hProcess != NULL) {
			TCHAR processPath[MAX_PATH], canonizedPath[MAX_PATH] = { 0, };
			DWORD pathSize = MAX_PATH;
			if (QueryFullProcessImageName(hProcess, NULL, processPath, &pathSize)) {
				if (S_OK == PathCchCanonicalize(canonizedPath, MAX_PATH, processPath)) {
					for (auto& process : processes)
						if (!_tcsicmp(process.c_str(), canonizedPath)) {
							matches = true;
							break;
						}
				}
			}
			CloseHandle(hProcess);
		}
	
		return matches;
	};

	bool running = false;
	if (Process32First(snapshot, &entry) && !checkProcess(entry))
		while (Process32Next(snapshot, &entry)) {
			running = checkProcess(entry);

			if (running)
				break;
		}
	else {
		running = true;
	}

	CloseHandle(snapshot);
	return running;
}
module; 

#include "stdafx.h"
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

export tstring getConfigFilePath(const tstring& appDir, const tstring& configFile) {
	TCHAR appDataDirPath[MAX_PATH];

	::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataDirPath);

	tstring configFilePath = appDataDirPath;

	if (!configFilePath.ends_with(_T("\\")))
		configFilePath.append(_T("\\"));

	configFilePath.append(appDir);
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
			DBGLOG("ERROR - Could not create the Filter Graph Manager.");
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

	using namespace std::chrono_literals;
	std::thread playThread(playProc);
	playThread.detach();
}
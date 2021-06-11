module; 

#include <windows.h>
#include <shlobj.h>
#include <string>

#include "tstring.h"

export module system:win32;

//import <string>;

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
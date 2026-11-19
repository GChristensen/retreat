module;

#include <string>
#include <vector>

#include "tstring.h"

// Platform services used by the core. Only declarations live here; each platform
// provides a module implementation unit (`module system;`) with the bodies,
// e.g. src/win32/system_win32.cpp.
export module system;

export tstring getBundledFilePath(const tstring& fileName);
export tstring getConfigFilePath(const tstring& configFile);
export void searchForFiles(const tstring& dir, const tstring& ext, std::vector<tstring>& files);
export tstring getRandomFile(const tstring& dir, const std::vector<tstring>& exts);
export void playAudioFile(tstring& filePath);
export void playSystemSound();
export bool isProcessRunning(std::vector<tstring>& processes);
// Canonical full paths of the running executables, deduplicated and sorted.
export std::vector<tstring> getRunningProcessPaths();

// Headless stand-in for the platform `system` module (see src/core/system.cppm).
module;

#include <filesystem>
#include <string>
#include <vector>

#include "tstring.h"

module system;

tstring getBundledFilePath(const tstring& fileName) { return fileName; }

tstring getConfigFilePath(const tstring& configFile) {
    return (std::filesystem::temp_directory_path() / "retreat_core_smoke" / configFile).native();
}

void searchForFiles(const tstring&, const tstring&, std::vector<tstring>&) {}
tstring getRandomFile(const tstring&, const std::vector<tstring>&) { return tstring(); }
void playAudioFile(tstring&) {}
void playSystemSound() {}
bool isProcessRunning(std::vector<tstring>&) { return false; }
std::vector<tstring> getRunningProcessPaths() { return {}; }

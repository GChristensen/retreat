module;

#include <windows.h>

#include <ctime>
#include <string>
#include "tstring.h"

// Keeps the latest StateSnapshot in the configuration directory (state.dat), so that a
// restarted application can continue a break. Written atomically: a temporary file
// replaces the old one.
export module StateStore;

import StateSnapshot;


export class StateStore {
public:
    StateSnapshot load();
    bool save(const StateSnapshot& snapshot);

    // when the computer was last started
    static time_t bootTime();

private:
    const tstring& path();
};

module :private;

import system;

#ifdef _DEBUG
static const TCHAR* STATE_FILE_NAME = _T("state_debug.dat");  // debug durations are seconds
#else
static const TCHAR* STATE_FILE_NAME = _T("state.dat");
#endif

const tstring& StateStore::path() {
    // resolved once: in retreatlib.pyd this asks Python for the Enso user directory
    static tstring file = getConfigFilePath(STATE_FILE_NAME);
    return file;
}

StateSnapshot StateStore::load() {
    StateSnapshot snapshot = StateSnapshot::invalid();

    HANDLE file = CreateFile(path().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return snapshot;

    StateSnapshot read;
    DWORD size = 0;
    if (ReadFile(file, &read, sizeof(read), &size, nullptr) && size == sizeof(read) && read.isValid())
        snapshot = read;

    CloseHandle(file);
    return snapshot;
}

bool StateStore::save(const StateSnapshot& snapshot) {
    tstring temp = path() + _T(".tmp");

    HANDLE file = CreateFile(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    DWORD written = 0;
    bool ok = WriteFile(file, &snapshot, sizeof(snapshot), &written, nullptr) && written == sizeof(snapshot);
    CloseHandle(file);

    if (ok)
        ok = MoveFileEx(temp.c_str(), path().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;

    if (!ok)
        DeleteFile(temp.c_str());

    return ok;
}

time_t StateStore::bootTime() {
    return time(nullptr) - (time_t)(GetTickCount64() / 1000);
}

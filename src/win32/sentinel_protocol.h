// Shared by retreat.exe / retreatlib.pyd (SentinelClient) and sentinel.exe.
// Plain Win32, no WTL or C++ modules, so that the sentinel stays tiny.
#pragma once

#include <windows.h>
#include <shellapi.h>
#include <cstdint>
#include <cwchar>

namespace sentinel {

#ifdef _DEBUG
#   define SENTINEL_NAME_SUFFIX L"_Debug"
#else
#   define SENTINEL_NAME_SUFFIX L""
#endif

constexpr const wchar_t* EXE_NAME = L"sentinel.exe";
constexpr const wchar_t* MUTEX_NAME = L"Local\\EnsoRetreat.Sentinel" SENTINEL_NAME_SUFFIX;
constexpr const wchar_t* QUIT_EVENT_NAME = L"Local\\EnsoRetreat.Sentinel.Quit" SENTINEL_NAME_SUFFIX;
constexpr const wchar_t* PIPE_PREFIX = L"\\\\.\\pipe\\EnsoRetreat.Sentinel" SENTINEL_NAME_SUFFIX L".";

// sentinel.exe exits with this code when it stops on purpose; the main app then does
// not start it again
constexpr DWORD EXIT_STANDDOWN = 0x52540001;
// retreat.exe exits with this code when another instance is already running; the
// sentinel then waits for that instance to introduce itself instead of relaunching
constexpr DWORD EXIT_ALREADY_RUNNING = 0x52540002;

constexpr DWORD MAX_IMAGE = 1024;
constexpr DWORD MAX_COMMAND_LINE = 8192;
constexpr DWORD MAX_SNAPSHOT = 1024;

enum MessageType : std::uint32_t {
    MSG_HELLO = 1,          // main → sentinel: who to watch and how to restart it
    MSG_SNAPSHOT,           // main → sentinel, and the reply to MSG_GET_SNAPSHOT
    MSG_GET_SNAPSHOT,       // main → sentinel
    MSG_GOODBYE,            // main → sentinel: exiting on purpose, do not restart
};

enum GoodbyeReason : std::uint32_t {
    GOODBYE_EXIT = 1,
    GOODBYE_SESSION_END,
    GOODBYE_RESTART_DISABLED,
};

struct Header {
    std::uint32_t type;
};

struct Hello {
    Header header{ MSG_HELLO };
    std::uint32_t pid = 0;
    wchar_t image[MAX_IMAGE] = {};
    wchar_t currentDirectory[MAX_IMAGE] = {};
    wchar_t commandLine[MAX_COMMAND_LINE] = {};
};

// The state itself is opaque to the sentinel; it only needs to know whether a break
// is being enforced (then it refuses to quit).
struct Snapshot {
    Header header{ MSG_SNAPSHOT };
    std::uint32_t enforcing = 0;
    std::uint32_t size = 0;
    std::uint8_t data[MAX_SNAPSHOT] = {};
};

struct GetSnapshot {
    Header header{ MSG_GET_SNAPSHOT };
};

struct Goodbye {
    Header header{ MSG_GOODBYE };
    std::uint32_t reason = GOODBYE_EXIT;
};

constexpr DWORD MAX_MESSAGE = sizeof(Hello);

inline void pipeName(wchar_t* buffer, size_t size) {
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    swprintf_s(buffer, size, L"%s%lu", PIPE_PREFIX, session);
}

inline void copyString(wchar_t* to, size_t size, const wchar_t* from) {
    wcsncpy_s(to, size, from ? from : L"", _TRUNCATE);
}

// The arguments part of a command line: argv[0] (quoted or not) and the blanks after
// it skipped, as CommandLineToArgvW does it.
inline const wchar_t* commandLineArguments(const wchar_t* commandLine) {
    const wchar_t* p = commandLine ? commandLine : L"";

    if (*p == L'"') {
        for (++p; *p && *p != L'"'; ++p);
        if (*p)
            ++p;
    }
    else
        for (; *p && *p != L' ' && *p != L'\t'; ++p);

    for (; *p == L' ' || *p == L'\t'; ++p);
    return p;
}

// CreateProcess refuses executables whose manifest asks for uiAccess (Enso runs in
// pythonu.exe, signed with uiAccess="true") with ERROR_ELEVATION_REQUIRED; those have
// to go through ShellExecute, which hands them to the AppInfo service. The window is
// hidden as by Enso's launcher (pythonu.exe is a console program).
inline bool launchViaShell(const wchar_t* image, const wchar_t* commandLine,
                           const wchar_t* directory, PROCESS_INFORMATION* pi) {
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
    sei.lpVerb = L"open";
    sei.lpFile = image;
    sei.lpParameters = commandLineArguments(commandLine);
    sei.lpDirectory = directory;
    sei.nShow = SW_HIDE;

    if (!ShellExecuteExW(&sei) || !sei.hProcess)
        return false;

    *pi = {};
    pi->hProcess = sei.hProcess;
    pi->dwProcessId = GetProcessId(sei.hProcess);
    return true;
}

// Starts a process as a child of Explorer, so that "End process tree" on one of the
// pair does not take the other down. Falls back to a plain child process, and to
// ShellExecute for uiAccess executables. pi->hThread may be null.
inline bool launchDetached(const wchar_t* image, const wchar_t* commandLine,
                           const wchar_t* currentDirectory, PROCESS_INFORMATION* pi) {
    wchar_t buffer[MAX_COMMAND_LINE];     // CreateProcessW may modify the command line
    copyString(buffer, MAX_COMMAND_LINE, commandLine);

    const wchar_t* directory = currentDirectory && *currentDirectory ? currentDirectory : nullptr;

    HANDLE explorer = nullptr;
    DWORD explorerPid = 0;
    if (HWND shell = GetShellWindow())
        GetWindowThreadProcessId(shell, &explorerPid);
    if (explorerPid)
        explorer = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, explorerPid);

    if (explorer) {
        SIZE_T attributeSize = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeSize);

        auto attributes = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attributeSize);
        bool launched = false;

        if (attributes && InitializeProcThreadAttributeList(attributes, 1, 0, &attributeSize)) {
            if (UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
                                          &explorer, sizeof(explorer), nullptr, nullptr)) {
                STARTUPINFOEXW si = {};
                si.StartupInfo.cb = sizeof(si);
                si.lpAttributeList = attributes;

                launched = CreateProcessW(image, buffer, nullptr, nullptr, FALSE,
                                          EXTENDED_STARTUPINFO_PRESENT, nullptr, directory,
                                          &si.StartupInfo, pi) != FALSE;
            }
            DeleteProcThreadAttributeList(attributes);
        }

        if (attributes)
            HeapFree(GetProcessHeap(), 0, attributes);
        CloseHandle(explorer);

        if (launched)
            return true;

        if (GetLastError() == ERROR_ELEVATION_REQUIRED)
            return launchViaShell(image, commandLine, directory, pi);

        copyString(buffer, MAX_COMMAND_LINE, commandLine);
    }

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    if (CreateProcessW(image, buffer, nullptr, nullptr, FALSE, 0, nullptr, directory, &si, pi))
        return true;

    return GetLastError() == ERROR_ELEVATION_REQUIRED
        && launchViaShell(image, commandLine, directory, pi);
}

// Restart throttling: more than `burst` restarts within `window` ms back off
// exponentially, from 5 s up to 5 min.
class Backoff {
public:
    // milliseconds to wait before the next restart
    DWORD next() {
        ULONGLONG now = GetTickCount64();

        if (now - windowStart > WINDOW_MS) {
            windowStart = now;
            count = 0;
            delay = 0;
        }

        if (++count <= BURST)
            return 0;

        delay = delay ? (delay * 2 > MAX_DELAY_MS ? MAX_DELAY_MS : delay * 2) : MIN_DELAY_MS;
        windowStart = now;  // keep backing off while restarts keep failing
        return delay;
    }

private:
    static constexpr ULONGLONG WINDOW_MS = 60 * 1000;
    static constexpr int BURST = 3;
    static constexpr DWORD MIN_DELAY_MS = 5 * 1000;
    static constexpr DWORD MAX_DELAY_MS = 5 * 60 * 1000;

    ULONGLONG windowStart = 0;
    int count = 0;
    DWORD delay = 0;
};

// Message-mode pipe helpers (blocking handles).
inline bool writeMessage(HANDLE pipe, const void* message, DWORD size) {
    DWORD written = 0;
    return WriteFile(pipe, message, size, &written, nullptr) && written == size;
}

} // namespace sentinel

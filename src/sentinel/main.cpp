// sentinel.exe: restarts Enso Retreat (retreat.exe, or Enso when Retreat runs as
// retreatlib.pyd) when it is terminated. It has no UI and knows nothing about breaks:
// the restarted application restores its state from the snapshot kept here and on disk.
//
// The main application starts the sentinel and restarts it in turn if it dies; see
// src/win32/SentinelClient.cppm. Protocol: src/win32/sentinel_protocol.h.
//
//   sentinel.exe          run (normally started by the main application)
//   sentinel.exe --quit   ask a running sentinel to stop (refused during a break)

#include <windows.h>

#include <cstring>
#include <cwchar>

#include "sentinel_protocol.h"

using namespace sentinel;

namespace {

// how long to wait for an application to introduce itself before giving up
constexpr ULONGLONG HELLO_TIMEOUT_MS = 30 * 1000;

CRITICAL_SECTION lock;

// guarded by `lock`
Hello hello;
bool haveHello = false;
Snapshot lastSnapshot;

HANDLE helloEvent;      // a client sent HELLO
HANDLE goodbyeEvent;    // a client sent GOODBYE
HANDLE quitEvent;       // sentinel.exe --quit

bool sessionEnding = false;

struct Lock {
    Lock() { EnterCriticalSection(&lock); }
    ~Lock() { LeaveCriticalSection(&lock); }
};

// pipe /////////////////////////////////////////////////////////////////////////////

DWORD WINAPI clientThread(LPVOID param) {
    HANDLE pipe = (HANDLE)param;

    static thread_local Hello buffer;   // largest message

    for (;;) {
        DWORD read = 0;
        if (!ReadFile(pipe, &buffer, sizeof(buffer), &read, nullptr) || read < sizeof(Header))
            break;

        switch (buffer.header.type) {
        case MSG_HELLO:
            if (read == sizeof(Hello)) {
                {
                    Lock guard;
                    hello = buffer;
                    hello.image[MAX_IMAGE - 1] = 0;
                    hello.currentDirectory[MAX_IMAGE - 1] = 0;
                    hello.commandLine[MAX_COMMAND_LINE - 1] = 0;
                    haveHello = true;
                }
                SetEvent(helloEvent);
            }
            break;

        case MSG_SNAPSHOT:
            if (read == sizeof(Snapshot)) {
                Lock guard;
                std::memcpy(&lastSnapshot, &buffer, sizeof(Snapshot));
            }
            break;

        case MSG_GET_SNAPSHOT: {
            Snapshot reply;
            {
                Lock guard;
                reply = lastSnapshot;
            }
            reply.header.type = MSG_SNAPSHOT;
            writeMessage(pipe, &reply, sizeof(reply));
            break;
        }

        case MSG_GOODBYE:
            SetEvent(goodbyeEvent);
            break;
        }
    }

    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
    return 0;
}

DWORD WINAPI pipeThread(LPVOID) {
    wchar_t name[256];
    pipeName(name, 256);

    for (;;) {
        HANDLE pipe = CreateNamedPipeW(name, PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            PIPE_UNLIMITED_INSTANCES, 64 * 1024, 64 * 1024, 0, nullptr);

        if (pipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        if (ConnectNamedPipe(pipe, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
            HANDLE thread = CreateThread(nullptr, 0, clientThread, pipe, 0, nullptr);
            if (thread)
                CloseHandle(thread);
            else
                CloseHandle(pipe);
        }
        else
            CloseHandle(pipe);
    }
}

// session end ////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_QUERYENDSESSION:
        sessionEnding = true;
        return TRUE;
    case WM_ENDSESSION:
        if (wParam)
            ExitProcess(EXIT_STANDDOWN);
        sessionEnding = false;     // shutdown was cancelled
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

void createSessionWindow(HINSTANCE instance) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"EnsoRetreatSentinel";
    RegisterClassW(&wc);

    // a hidden top-level window: message-only windows do not get session broadcasts
    CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
}

bool shouldStandDown() {
    return sessionEnding || GetSystemMetrics(SM_SHUTTINGDOWN);
}

// --quit /////////////////////////////////////////////////////////////////////////////

int quit() {
    HANDLE event = OpenEventW(EVENT_MODIFY_STATE, FALSE, QUIT_EVENT_NAME);
    if (!event)
        return 0;   // not running

    SetEvent(event);
    CloseHandle(event);

    for (int i = 0; i < 30; ++i) {
        HANDLE mutex = OpenMutexW(SYNCHRONIZE, FALSE, MUTEX_NAME);
        if (!mutex)
            return 0;
        CloseHandle(mutex);
        Sleep(100);
    }

    return 1;       // refused (a break is under way) or not responding
}

// main loop //////////////////////////////////////////////////////////////////////////

int run() {
    HANDLE process = nullptr;               // the application being watched
    DWORD processId = 0;
    ULONGLONG waitingSince = GetTickCount64();  // waiting for a HELLO (no process)
    ULONGLONG relaunchAt = 0;               // pending restart (0 = none)
    Backoff backoff;

    for (;;) {
        HANDLE handles[4] = { quitEvent, goodbyeEvent, helloEvent };
        DWORD count = 3;
        if (process)
            handles[count++] = process;

        DWORD timeout = INFINITE;
        ULONGLONG now = GetTickCount64();
        if (relaunchAt)
            timeout = relaunchAt > now ? (DWORD)(relaunchAt - now) : 0;
        else if (!process)
            timeout = waitingSince + HELLO_TIMEOUT_MS > now ? (DWORD)(waitingSince + HELLO_TIMEOUT_MS - now) : 0;

        DWORD result = MsgWaitForMultipleObjects(count, handles, FALSE, timeout, QS_ALLINPUT);

        if (result == WAIT_OBJECT_0) {                  // --quit
            Lock guard;
            if (!lastSnapshot.enforcing)
                return EXIT_STANDDOWN;
        }
        else if (result == WAIT_OBJECT_0 + 1) {         // GOODBYE
            return EXIT_STANDDOWN;
        }
        else if (result == WAIT_OBJECT_0 + 2) {         // HELLO
            DWORD pid;
            {
                Lock guard;
                pid = hello.pid;
            }

            if (pid != processId) {
                HANDLE opened = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
                if (opened) {
                    if (process)
                        CloseHandle(process);
                    process = opened;
                    processId = pid;
                    relaunchAt = 0;
                }
            }
        }
        else if (process && result == WAIT_OBJECT_0 + 3) {  // the application is gone
            DWORD exitCode = 0;
            GetExitCodeProcess(process, &exitCode);
            CloseHandle(process);
            process = nullptr;
            processId = 0;

            if (shouldStandDown())
                return EXIT_STANDDOWN;

            if (exitCode == EXIT_ALREADY_RUNNING) {
                // another instance is running; it will introduce itself
                waitingSince = GetTickCount64();
                continue;
            }

            bool known;
            {
                Lock guard;
                known = haveHello;
            }
            if (!known)
                return EXIT_STANDDOWN;

            relaunchAt = GetTickCount64() + backoff.next();
        }
        else if (result == WAIT_OBJECT_0 + count) {     // window messages
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        else if (result == WAIT_TIMEOUT) {
            if (relaunchAt) {
                relaunchAt = 0;

                if (shouldStandDown())
                    return EXIT_STANDDOWN;

                Hello target;
                {
                    Lock guard;
                    target = hello;
                }

                PROCESS_INFORMATION pi = {};
                if (launchDetached(target.image, target.commandLine, target.currentDirectory, &pi)) {
                    if (pi.hThread)
                        CloseHandle(pi.hThread);
                    process = pi.hProcess;
                    processId = pi.dwProcessId;
                }
                else
                    relaunchAt = GetTickCount64() + backoff.next();
            }
            else if (!process)
                return EXIT_STANDDOWN;      // nobody to watch
        }
        else if (result == WAIT_FAILED) {
            Sleep(100);
        }
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR commandLine, int) {
    if (commandLine && wcsstr(commandLine, L"--quit"))
        return quit();

    HANDLE mutex = CreateMutexW(nullptr, FALSE, MUTEX_NAME);
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS)
        return EXIT_ALREADY_RUNNING;

    InitializeCriticalSection(&lock);
    helloEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    goodbyeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    quitEvent = CreateEventW(nullptr, FALSE, FALSE, QUIT_EVENT_NAME);

    // be told about the end of the session before the application is
    SetProcessShutdownParameters(0x3FF, SHUTDOWN_NORETRY);
    createSessionWindow(instance);

    HANDLE thread = CreateThread(nullptr, 0, pipeThread, nullptr, 0, nullptr);
    if (thread)
        CloseHandle(thread);

    int result = run();

    CloseHandle(mutex);
    return result;
}

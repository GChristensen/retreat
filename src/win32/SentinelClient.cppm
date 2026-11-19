module;

#include <windows.h>

#include <cstring>
#include <mutex>
#include <string>
#include "tstring.h"

#include "sentinel_protocol.h"

// The main application's side of the sentinel pair (see src/sentinel/main.cpp).
// A worker thread owns the pipe, starts sentinel.exe when it is not running and
// starts it again when it is terminated, so the UI thread (and, in retreatlib.pyd,
// Python) never waits on it.
export module SentinelClient;

import StateSnapshot;


export class SentinelClient {
public:
    ~SentinelClient();

    // connect to the sentinel, starting it if necessary
    void start();
    bool isRunning() const { return thread != nullptr; }

    // the snapshot held by an already running sentinel (left by the previous run);
    // invalid if there was none
    StateSnapshot initialSnapshot(DWORD timeoutMs);

    // hand the current state to the sentinel (no I/O on the calling thread)
    void post(const StateSnapshot& snapshot);

    // tell the sentinel to stop without restarting the application, then disconnect
    void goodbye(sentinel::GoodbyeReason reason);

private:
    HANDLE thread = nullptr;
    HANDLE stopEvent = nullptr;
    HANDLE postEvent = nullptr;
    HANDLE initialEvent = nullptr;

    std::mutex lock;
    sentinel::Snapshot latest;      // guarded by lock
    bool havePost = false;          // guarded by lock
    StateSnapshot initial = StateSnapshot::invalid();   // written before initialEvent is set
    sentinel::GoodbyeReason goodbyeReason = (sentinel::GoodbyeReason)0;

    sentinel::Hello hello;
    std::wstring sentinelPath;
    std::wstring sentinelDirectory;

    static DWORD WINAPI threadProc(LPVOID param);
    void run();
    HANDLE connect();
    HANDLE launch();
    bool fetchSnapshot(HANDLE pipe);
    bool sendLatest(HANDLE pipe);
    void stopThread();
};

module :private;

import system;

using namespace sentinel;

SentinelClient::~SentinelClient() {
    stopThread();
}

void SentinelClient::start() {
    if (thread)
        return;

    hello = Hello();
    hello.pid = GetCurrentProcessId();
    GetModuleFileNameW(nullptr, hello.image, MAX_IMAGE);
    GetCurrentDirectoryW(MAX_IMAGE, hello.currentDirectory);
    copyString(hello.commandLine, MAX_COMMAND_LINE, GetCommandLineW());

    // next to retreat.exe / retreatlib.pyd
    sentinelPath = getBundledFilePath(EXE_NAME);
    sentinelDirectory = sentinelPath.substr(0, sentinelPath.find_last_of(L'\\'));

    stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    postEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    initialEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    goodbyeReason = (GoodbyeReason)0;

    thread = CreateThread(nullptr, 0, threadProc, this, 0, nullptr);
}

StateSnapshot SentinelClient::initialSnapshot(DWORD timeoutMs) {
    if (!thread || WaitForSingleObject(initialEvent, timeoutMs) != WAIT_OBJECT_0)
        return StateSnapshot::invalid();

    return initial;
}

void SentinelClient::post(const StateSnapshot& snapshot) {
    if (!thread)
        return;

    static_assert(sizeof(StateSnapshot) <= MAX_SNAPSHOT);

    {
        std::lock_guard guard(lock);
        latest.enforcing = snapshot.kind == StateKind::Alert
            || snapshot.kind == StateKind::Delay
            || snapshot.kind == StateKind::Locked;
        latest.size = sizeof(StateSnapshot);
        std::memcpy(latest.data, &snapshot, sizeof(StateSnapshot));
        havePost = true;
    }

    SetEvent(postEvent);
}

void SentinelClient::goodbye(GoodbyeReason reason) {
    if (!thread)
        return;

    goodbyeReason = reason;
    stopThread();
}

void SentinelClient::stopThread() {
    if (!thread)
        return;

    SetEvent(stopEvent);
    WaitForSingleObject(thread, 2000);

    CloseHandle(thread);
    CloseHandle(stopEvent);
    CloseHandle(postEvent);
    CloseHandle(initialEvent);
    thread = stopEvent = postEvent = initialEvent = nullptr;
}

DWORD WINAPI SentinelClient::threadProc(LPVOID param) {
    static_cast<SentinelClient*>(param)->run();
    return 0;
}

HANDLE SentinelClient::connect() {
    wchar_t name[256];
    pipeName(name, 256);

    for (int attempt = 0; attempt < 3; ++attempt) {
        HANDLE pipe = CreateFileW(name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (pipe != INVALID_HANDLE_VALUE) {
            DWORD mode = PIPE_READMODE_MESSAGE;
            SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr);
            return pipe;
        }

        if (GetLastError() != ERROR_PIPE_BUSY || !WaitNamedPipeW(name, 500))
            break;
    }

    return nullptr;
}

HANDLE SentinelClient::launch() {
    std::wstring commandLine = L"\"" + sentinelPath + L"\"";

    PROCESS_INFORMATION pi = {};
    if (!launchDetached(sentinelPath.c_str(), commandLine.c_str(), sentinelDirectory.c_str(), &pi))
        return nullptr;

    if (pi.hThread)
        CloseHandle(pi.hThread);
    return pi.hProcess;
}

bool SentinelClient::fetchSnapshot(HANDLE pipe) {
    GetSnapshot request;
    if (!writeMessage(pipe, &request, sizeof(request)))
        return false;

    Snapshot reply;
    DWORD read = 0;
    if (!ReadFile(pipe, &reply, sizeof(reply), &read, nullptr) || read != sizeof(reply))
        return false;

    if (reply.size == sizeof(StateSnapshot)) {
        StateSnapshot snapshot;
        std::memcpy(&snapshot, reply.data, sizeof(StateSnapshot));
        if (snapshot.isValid())
            initial = snapshot;
    }

    return true;
}

bool SentinelClient::sendLatest(HANDLE pipe) {
    Snapshot message;
    {
        std::lock_guard guard(lock);
        if (!havePost)
            return true;
        message = latest;
    }

    return writeMessage(pipe, &message, sizeof(message));
}

void SentinelClient::run() {
    Backoff backoff;
    bool first = true;

    auto initialDone = [&]() {
        if (first) {
            first = false;
            SetEvent(initialEvent);
        }
    };

    // wait, but return true if asked to stop
    auto pause = [&](DWORD ms) {
        return WaitForSingleObject(stopEvent, ms) == WAIT_OBJECT_0;
    };

    for (;;) {
        HANDLE pipe = connect();
        HANDLE sentinelProcess = nullptr;

        if (pipe) {
            if (first)
                fetchSnapshot(pipe);
            initialDone();

            ULONG serverPid = 0;
            if (GetNamedPipeServerProcessId(pipe, &serverPid))
                sentinelProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, serverPid);
        }
        else {
            initialDone();      // nothing was running, so nothing to restore from it

            if (WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0)
                return;

            sentinelProcess = launch();

            // wait for its pipe
            for (int i = 0; i < 50 && !pipe; ++i) {
                if (pause(100))
                    break;
                pipe = connect();
            }

            if (!pipe) {
                if (sentinelProcess)
                    CloseHandle(sentinelProcess);
                if (pause(backoff.next() + 1000))
                    return;
                continue;
            }
        }

        bool connected = writeMessage(pipe, &hello, sizeof(hello)) && sendLatest(pipe);
        bool standDown = false;

        while (connected) {
            HANDLE handles[3] = { stopEvent, postEvent, sentinelProcess };
            DWORD count = sentinelProcess ? 3 : 2;

            DWORD result = WaitForMultipleObjects(count, handles, FALSE, INFINITE);

            if (result == WAIT_OBJECT_0) {
                if (goodbyeReason) {
                    Goodbye message;
                    message.reason = goodbyeReason;
                    writeMessage(pipe, &message, sizeof(message));
                }
                CloseHandle(pipe);
                if (sentinelProcess)
                    CloseHandle(sentinelProcess);
                return;
            }
            else if (result == WAIT_OBJECT_0 + 1) {
                connected = sendLatest(pipe);
            }
            else if (result == WAIT_OBJECT_0 + 2) {
                DWORD exitCode = 0;
                GetExitCodeProcess(sentinelProcess, &exitCode);
                standDown = exitCode == EXIT_STANDDOWN;
                connected = false;
            }
            else
                connected = false;
        }

        CloseHandle(pipe);
        if (sentinelProcess)
            CloseHandle(sentinelProcess);

        if (standDown)
            return;             // stopped on purpose (--quit or GOODBYE)

        if (pause(backoff.next()))
            return;
    }
}

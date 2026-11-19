module;

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>

#include "tstring.h"

// A fixed-size, trivially copyable picture of the state machine. It is written to
// disk and sent to the sentinel as raw bytes, so it must stay a POD; bump VERSION
// whenever the layout changes.
export module StateSnapshot;


export enum class StateKind : std::int32_t {
    Idle = 0,
    Alert,
    Delay,
    Locked,
    Suspended,
};

export struct StateSnapshot {
    static constexpr std::uint32_t MAGIC = 0x52545353;    // 'RTSS'
    static constexpr std::uint32_t VERSION = 1;
    static constexpr std::size_t MESSAGE_LENGTH = 256;

    std::uint32_t magic = MAGIC;
    std::uint32_t version = VERSION;
    std::uint64_t seq = 0;

    std::int64_t writtenAt = 0;         // wall-clock time of the snapshot
    StateKind kind = StateKind::Idle;
    std::int32_t cleanExit = 0;         // the app quit normally while nothing was enforced
    std::int32_t offline = 0;           // written on suspend / session end: the time until the
                                        // next start was spent away from the computer
    std::int32_t reserved = 0;

    std::int64_t deadline = 0;          // absolute end of the current state (0 if none)
    std::int64_t periodStartTime = 0;   // PeriodicEvent::startTime

    std::int32_t delaysRequested = 0;
    std::int32_t lockDurationOverride = 0;  // break length set by a cron event, seconds

    TCHAR messageText[MESSAGE_LENGTH] = {}; // shown during a cron break

    bool isValid() const {
        return magic == MAGIC && version == VERSION;
    }

    void setMessageText(const tstring& text) {
        std::size_t n = text.size() < MESSAGE_LENGTH - 1 ? text.size() : MESSAGE_LENGTH - 1;
        std::memcpy(messageText, text.data(), n * sizeof(TCHAR));
        messageText[n] = 0;
    }

    tstring getMessageText() const {
        TCHAR copy[MESSAGE_LENGTH];
        std::memcpy(copy, messageText, sizeof(copy));
        copy[MESSAGE_LENGTH - 1] = 0;
        return tstring(copy);
    }

    // which of two snapshots is more recent
    static const StateSnapshot& newest(const StateSnapshot& a, const StateSnapshot& b) {
        if (!a.isValid()) return b;
        if (!b.isValid()) return a;
        if (a.writtenAt != b.writtenAt) return a.writtenAt > b.writtenAt ? a : b;
        return a.seq >= b.seq ? a : b;
    }

    static StateSnapshot invalid() {
        StateSnapshot s;
        s.magic = 0;
        return s;
    }
};

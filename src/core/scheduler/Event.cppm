module;

#include <ctime>

#include <memory>
#include <optional>
#include <string>
#include "tstring.h"

export module Event;


// An alert that should have started while the application was not running.
export struct MissedAlert {
    time_t alertStart = 0;      // when the alert window began
    int breakDurationSec = 0;   // length of the break that follows the alert, 0 = default
    tstring messageText;        // shown when the break ends (cron events)
};

export class Event {
public:
    virtual ~Event() = default;

    virtual bool isMonitoring(time_t time) { return false; }
    virtual bool isAlert(time_t time) { return false; }

    // The earliest alert of this event that starts in (from, to], if any.
    virtual std::optional<MissedAlert> alertBetween(time_t from, time_t to) { return std::nullopt; }

    virtual void debug() { }
};

export using EventPtr = std::shared_ptr<Event>;

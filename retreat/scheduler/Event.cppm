module;

#include <ctime>

export module Event;

import <memory>;

export class Event {
public:
    virtual bool isMonitoring(time_t time) { return false; }
    virtual bool isAlert(time_t time) { return false; }

    virtual void debug() { }
};

export using EventPtr = std::shared_ptr<Event>;
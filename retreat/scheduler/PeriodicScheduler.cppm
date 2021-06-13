module;

#include <ctime>
#include <cstdlib>

export module PeriodicScheduler;

import Scheduler;
import Settings;
import StateMachine;

#include "debug.h"

export class PeriodicScheduler: public Scheduler {
public:
    PeriodicScheduler(Settings &settings);

    virtual void schedule(StateMachine &machine) override;

private:
    static std::time_t startTime;

    int counter = 0;
    int timeToEventSec;
    int eventBoundarySec;
    
    bool fromLaunchTime;
    int periodDurationSec;
    int breakDurationSec;
    int alertDurationSec;
};

module :private;

time_t PeriodicScheduler::startTime = 0;

PeriodicScheduler::PeriodicScheduler(Settings &settings) {
    fromLaunchTime = settings.getBoolean(Settings::PERIOD_FROM_LAUNCH, Settings::DEFAULT_PERIOD_FROM_LAUNCH);
    periodDurationSec = settings.getMinutesInSec(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION);
    breakDurationSec = settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);
    alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);

    eventBoundarySec = breakDurationSec + alertDurationSec;
    timeToEventSec = periodDurationSec - eventBoundarySec;

    if (eventBoundarySec > periodDurationSec)
        periodDurationSec += eventBoundarySec;

    if (!startTime) {
        startTime = time(nullptr);
        if (!fromLaunchTime)
            startTime -= startTime % 3600;
    }
}

void PeriodicScheduler::schedule(StateMachine &machine) {
    time_t currentTime = time(nullptr);
    int elapsed = currentTime - startTime;

    if (elapsed % periodDurationSec == timeToEventSec) {
        machine.setAlert();
    }

    DBGLOG(elapsed);
}
module;

#include <ctime>

export module PeriodicEvent;

import Event;
import Settings;

#include "debug.h"

export class PeriodicEvent: public Event {
public:
    PeriodicEvent(Settings &settings);

    virtual bool isMonitoring(time_t time) override;
    virtual bool isAlert(time_t time) override;

    static void reset(Settings &settings);

private:
    static std::time_t startTime;

    int timeToAlertSec;
    int alertBoundarySec;

    int timeToMonitoringSec;
    int monitoringBoundarySec;

    bool monitoringEnabled;
    int userInactivitySec;

    bool fromLaunchTime;
    int periodDurationSec;
    int breakDurationSec;
    int alertDurationSec;
};

module :private;

time_t PeriodicEvent::startTime = 0;

PeriodicEvent::PeriodicEvent(Settings &settings) {
    periodDurationSec = settings.getMinutesInSec(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION);
    breakDurationSec = settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);
    alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);

    monitoringEnabled = settings.getBoolean(Settings::MONITORING_INPUT, Settings::DEFAULT_MONITORING_INPUT);
    userInactivitySec = settings.getMinutesInSec(Settings::MONITORING_INACTIVITY, Settings::DEFAULT_MONITORING_INACTIVITY);

    alertBoundarySec = breakDurationSec + alertDurationSec;
    monitoringBoundarySec = monitoringEnabled? alertBoundarySec + userInactivitySec: 0;
    timeToAlertSec = periodDurationSec - alertBoundarySec;
    timeToMonitoringSec = periodDurationSec - monitoringBoundarySec;

    if ((monitoringEnabled? monitoringBoundarySec: alertBoundarySec) > periodDurationSec)
        periodDurationSec += monitoringEnabled? monitoringBoundarySec: alertBoundarySec;

    if (!startTime)
        reset(settings);
}

void PeriodicEvent::reset(Settings &settings) {
    bool fromLaunchTime = settings.getBoolean(Settings::PERIOD_FROM_LAUNCH, Settings::DEFAULT_PERIOD_FROM_LAUNCH);
 
    startTime = time(nullptr);
    if (!fromLaunchTime)
        startTime -= startTime % 3600;
   
}

bool PeriodicEvent::isMonitoring(time_t time) {
    int elapsed = time - startTime;

    if (monitoringEnabled && elapsed % periodDurationSec == timeToMonitoringSec)
        return true;

    return false;
}

bool PeriodicEvent::isAlert(time_t time) {
    int elapsed = time - startTime;

    DBGLOG(elapsed << _T(" (") << elapsed % periodDurationSec << _T(") | " << timeToAlertSec));

    if (elapsed % periodDurationSec == timeToAlertSec)
        return true;

    return false;
}

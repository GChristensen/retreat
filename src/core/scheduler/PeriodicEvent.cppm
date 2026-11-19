module;

#include <ctime>

#include <optional>
#include <string>
#include "tstring.h"
#include "debug.h"

export module PeriodicEvent;

import Event;
import Settings;


export class PeriodicEvent: public Event {
public:
    PeriodicEvent(Settings &settings);

    virtual bool isMonitoring(time_t time) override;
    virtual bool isAlert(time_t time) override;
    virtual std::optional<MissedAlert> alertBetween(time_t from, time_t to) override;

    static void resetStartTime(Settings &settings);
    static time_t getStartTime() { return startTime; }
    static void setStartTime(time_t time) { startTime = time; }

private:
    static std::time_t startTime;

    int timeToAlertSec;
    int alertBoundarySec;
    bool alertEventSet = false;

    int timeToMonitoringSec;
    int monitoringBoundarySec;
    bool monitoringEventSet = false;

    bool monitoringEnabled;
    int userInactivitySec;

    bool fromLaunchTime;
    int periodDurationSec;
    int breakDurationSec;
    int alertDurationSec;

};

module :private;

const int TRANSITION_TOLERANCE_SEC = 3;

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
        resetStartTime(settings);
}

void PeriodicEvent::resetStartTime(Settings &settings) {
    bool fromLaunchTime = settings.getBoolean(Settings::PERIOD_FROM_LAUNCH, Settings::DEFAULT_PERIOD_FROM_LAUNCH);
 
    startTime = time(nullptr);

    if (!fromLaunchTime) {
        tm localTime;
        localtime_s(&localTime, &startTime);
        
        localTime.tm_sec = 0;
        localTime.tm_min = 0;

        startTime = mktime(&localTime);
    }
}

bool PeriodicEvent::isMonitoring(time_t time) {
    if (!monitoringEnabled || monitoringBoundarySec == alertBoundarySec)
        return false;

    int elapsed = time - startTime;

    int timePoint = elapsed % periodDurationSec;
    if (!monitoringEventSet && timePoint >= timeToMonitoringSec && timePoint < (timeToMonitoringSec + TRANSITION_TOLERANCE_SEC)) {
        monitoringEventSet = true;
        return true;
    }
    else if (monitoringEventSet && timePoint >= (timeToMonitoringSec + TRANSITION_TOLERANCE_SEC))
        monitoringEventSet = false;

    return false;
}

bool PeriodicEvent::isAlert(time_t time) {
    int elapsed = time - startTime;

    DBGLOG(elapsed << _T(" (") << elapsed % periodDurationSec << _T(") | " << timeToAlertSec));

    int timePoint = elapsed % periodDurationSec;

    if (!alertEventSet && timePoint >= timeToAlertSec && timePoint < (timeToAlertSec + TRANSITION_TOLERANCE_SEC)) {
        alertEventSet = true;
        return true;
    }
    else if (alertEventSet && timePoint >= (timeToAlertSec + TRANSITION_TOLERANCE_SEC))
        alertEventSet = false;

    return false;
}

std::optional<MissedAlert> PeriodicEvent::alertBetween(time_t from, time_t to) {
    if (periodDurationSec <= 0 || to <= from)
        return std::nullopt;

    // alerts start at startTime + k * period + timeToAlert; find the first one after `from`
    long long offset = (long long)(from - startTime) - timeToAlertSec;
    long long k = offset >= 0 ? offset / periodDurationSec : -((-offset + periodDurationSec - 1) / periodDurationSec);
    time_t alertStart = startTime + (time_t)((k + 1) * periodDurationSec) + timeToAlertSec;

    if (alertStart <= from)
        alertStart += periodDurationSec;

    if (alertStart > to)
        return std::nullopt;

    return MissedAlert{ alertStart, 0, tstring() };
}

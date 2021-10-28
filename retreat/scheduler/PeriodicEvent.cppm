module;

#include <ctime>

// !!! dependency on windows in generic code
#include <windows.h>
#include <tchar.h>
// !!!

export module PeriodicEvent;

// >>> for debug
#include "tstring.h"

import <format>;
import <thread>;
// <<< for debug


import Event;
import Settings;

#include "debug.h"

export class PeriodicEvent: public Event {
public:
    PeriodicEvent(Settings &settings);

    virtual bool isMonitoring(time_t time) override;
    virtual bool isAlert(time_t time) override;

    virtual void debug() override;

    static void resetStartTime(Settings &settings);

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

    int alertCount = 0;
    int roughAlertCount = 0;
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

    if (timePoint == timeToAlertSec)
        roughAlertCount += 1;

    // 3 second tolerance gap to compensate running in a GUI thread which may supend and load libraries
    if (!alertEventSet && timePoint >= timeToAlertSec && timePoint < (timeToAlertSec + TRANSITION_TOLERANCE_SEC)) {
        alertEventSet = true;

        alertCount += 1;

        if (roughAlertCount != alertCount)
            MessageBox(0, _T("Incorrect alert behavior!"), _T("Enso Retreat"), MB_ICONERROR);

        return true;
    }
    else if (alertEventSet && timePoint == timeToAlertSec) {
        MessageBox(0, _T("Incorrect alertEvent behavior!"), _T("Enso Retreat"), MB_ICONERROR);
    }
    else if (alertEventSet && timePoint >= (timeToAlertSec + TRANSITION_TOLERANCE_SEC))
        alertEventSet = false;

    return false;
}

void PeriodicEvent::debug() {
    auto debugProc = [this]() {
        tstring dbg = std::format(_T("timeToAlertSec: {:d}\nalertEventSet: {:d}\nalertCount: {:d}"), this->timeToAlertSec, this->alertEventSet, this->alertCount);
        ::MessageBox(0, dbg.c_str(), _T(""), 0);
    };

    std::thread debugThread(debugProc);
    debugThread.detach();
}

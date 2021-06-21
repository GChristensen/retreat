module;

#include <ctime>
#include "cron.h"

export module CronEvent;

import <regex>;
import <string>;

import Event;
import Settings;
import StateLocked;

#include "tstring.h"
#include "debug.h"

export class CronEvent: public Event {
public:
    CronEvent(Settings &settings, tstring &eventString);

	bool isMalformed() { return malformed; }

    virtual bool isMonitoring(time_t time) override;
    virtual bool isAlert(time_t time) override;

private:

	int breakDurationSec;
	int alertDurationSec;

	tstring messageText;

	int alertBoundarySec;
	int monitoringBoundarySec;

    bool monitoringEnabled;
    int userInactivitySec;

	int currentMinuteAlert = -1;
	int currentMinuteMonitoring = -1;

	bool malformed = false;

    auto split(tstring &s, tstring separator);

	cron::cronexpr cronExpr;
};

module :private;

auto CronEvent::split(tstring &s, tstring separator) {
    tregex reg(separator);
    tregex_token_iterator iter(s.begin(), s.end(), reg, -1);

    return std::vector<tstring>(iter, tregex_token_iterator());
}

CronEvent::CronEvent(Settings& settings, tstring &eventString) {
	auto eventTokens = split(eventString, _T("\\|"));

	try {
		cronExpr = cron::make_cron(eventTokens[0]);
	}
	catch (std::exception &) {
		malformed = true;
		return;
	}

	alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);

	breakDurationSec = -1;

	if (eventTokens.size() > 1) {
		try {
			breakDurationSec = std::stoi(eventTokens[1]) * DBG_SECONDS;
		}
		catch (std::exception&) {
		}
	}

	if (breakDurationSec < 0)
		breakDurationSec = Settings::DEFAULT_BREAK_DURATION;

	if (eventTokens.size() > 2)
		messageText = eventTokens[2];

    monitoringEnabled = settings.getBoolean(Settings::MONITORING_INPUT, Settings::DEFAULT_MONITORING_INPUT);
    userInactivitySec = settings.getMinutesInSec(Settings::MONITORING_INACTIVITY, Settings::DEFAULT_MONITORING_INACTIVITY);

	alertBoundarySec = alertDurationSec;
	monitoringBoundarySec = monitoringEnabled ? alertBoundarySec + userInactivitySec : 0;
}

bool CronEvent::isMonitoring(time_t time) {
	if (!monitoringEnabled || monitoringBoundarySec == alertBoundarySec)
		return false;

	tm localTime;
	time_t withBoundary = time + monitoringBoundarySec;
	localtime_s(&localTime, &withBoundary);

	if (currentMinuteMonitoring != localTime.tm_min) {
		currentMinuteMonitoring = localTime.tm_min;
		if (cron::cron_has(cronExpr, localTime))
			return true;
	}

    return false;
}

bool CronEvent::isAlert(time_t time) {
	tm localTime;
	time_t withBoundary = time + alertBoundarySec;
	
	localtime_s(&localTime, &withBoundary);
	
	if (currentMinuteAlert != localTime.tm_min) {
		currentMinuteAlert = localTime.tm_min;
		
		if (cron::cron_has(cronExpr, localTime)) {
			StateLocked::setMessageText(messageText);
			StateLocked::setLockDurationOverride(breakDurationSec);
			return true;
		}
	}

    return false;
}


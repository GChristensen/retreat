module;

#include <ctime>

export module CronEvent;

import <tuple>;
import <regex>;
import <string>;
import <algorithm>;

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

	int periodDurationSec;
	int breakDurationSec;
	int alertDurationSec;

	tstring messageText;

	int alertBoundarySec;
	int monitoringBoundarySec;

    bool monitoringEnabled;
    int userInactivitySec;

	int currentMinuteAlert = -1;
	int currentMinuteMonitoring = -1;

	// is a range if the both members are not equal -1
	struct CronValue {
		int v1;
		int v2;

		bool empty() { return v1 == -1 && v2 == -1; }
	};

	// an element of the vector considered a cron "*" if it is empty
	std::vector<std::vector<CronValue>> cronItems;

	static const CronValue CRON_RANGES[];

	bool malformed = false;

    auto split(tstring &s, tstring separator);

    void parseCron(tstring &cron);
	bool matches(const tm &time);
};

module :private;

CronEvent::CronValue const CronEvent::CRON_RANGES[] = {
	{0, 59},
	{0, 23},
	{1, 31},
	{1, 12},
	{0, 7}
};

auto CronEvent::split(tstring &s, tstring separator) {
    tregex reg(separator);
    tregex_token_iterator iter(s.begin(), s.end(), reg, -1);

    return std::vector<tstring>(iter, tregex_token_iterator());
}

CronEvent::CronEvent(Settings& settings, tstring &eventString): cronItems(5) {
	auto eventTokens = split(eventString, _T("\\|"));
	parseCron(eventTokens[0]);

	periodDurationSec = settings.getMinutesInSec(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION);
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

	alertBoundarySec = breakDurationSec + alertDurationSec;
	monitoringBoundarySec = monitoringEnabled ? alertBoundarySec + userInactivitySec : 0;
}

bool CronEvent::isMonitoring(time_t time) {
	tm localTime;
	time_t withBoundary = time + monitoringBoundarySec;
	localtime_s(&localTime, &withBoundary);

	if (currentMinuteMonitoring != localTime.tm_min) {
		currentMinuteMonitoring = localTime.tm_min;
		if (this->matches(localTime))
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
		
		if (this->matches(localTime)) {
			StateLocked::setMessageText(messageText);
			StateLocked::setLockDurationOverride(breakDurationSec);
			return true;
		}
	}

    return false;
}

void CronEvent::parseCron(tstring& cron) {
	auto cronItemStrings = split(cron, _T(" "));

	for (std::size_t i = 0; i < cronItemStrings.size() && i < 5; ++i) {
		if (cronItemStrings[i] == _T("*"))
			continue;

		auto enumeration = split(cronItemStrings[i], _T(","));

		for (auto& value : enumeration) {
			auto range = split(value, _T("-"));

			if (range.size() > 1) {
				CronValue cronValue = { -1, -1 };

				try {
					cronValue.v1 = std::stoi(range[0]);
					cronValue.v2 = std::stoi(range[1]);
				}
				catch (std::exception&) {
				}

				if (cronValue.v1 < CRON_RANGES[i].v1
					|| cronValue.v1 > CRON_RANGES[i].v2
					|| cronValue.v2 < CRON_RANGES[i].v1
					|| cronValue.v2 > CRON_RANGES[i].v2
					|| cronValue.v1 > cronValue.v2) {
					malformed = true;
					break;
				}

				cronItems[i].emplace_back(cronValue);
			}
			else if (range.size() == 1) {
				CronValue cronValue = { -1, -1 };

				try {
					cronValue.v1 = std::stoi(range[0]);
				}
				catch (std::exception&) {
				}

				if (cronValue.v1 < CRON_RANGES[i].v1
					|| cronValue.v1 > CRON_RANGES[i].v2) {
					malformed = true;
					break;
				}

				cronItems[i].emplace_back(cronValue);
			}
		}

		if (malformed)
			break;
	}

	if (!malformed) {
		bool empty = true;

		for (std::vector<CronValue>& values : cronItems) {
			if (!values.empty()) {
				empty = false;
				break;
			}
		}

		malformed = empty;
	}
}

bool CronEvent::matches(const tm &time) {
	if (malformed)
		return false;

	bool result = true;

	int tm_values[] = { time.tm_min, time.tm_hour, time.tm_mday, time.tm_mon };

	// at first check the day of week, 7 - also Sunday
	for (CronValue &value : cronItems[4]) {
		if (value.v2 >= 0)
			result = time.tm_wday >= value.v1 && time.tm_wday <= value.v2
				|| time.tm_wday == 0 && value.v2 == 7;
		else
			result = time.tm_wday == value.v1 || time.tm_wday == 0 && value.v1 == 7;

		if (result)
			break;
	}

	if (!result)
		return false;

	for (int i = 0; i < 4; ++i) {
		for (CronValue &value : cronItems[i]) {
			if (value.v2 >= 0)
				result = tm_values[i] >= value.v1 && tm_values[i] <= value.v2;
			else
				result = tm_values[i] == value.v1;
			
			if (result)
				break;
		}

		if (!result)
			break;
	}

	return result;
}
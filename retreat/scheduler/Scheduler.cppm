module;

#include <ctime>
#include <cstdlib>

export module Scheduler;

import <memory>;
import <vector>;

import Event;
import Settings;
import StateMachine;
import PeriodicEvent;
import CronEvent;

#include "debug.h"

export class Scheduler {
public:
    Scheduler(Settings& settings);
    void reset(Settings& settings);

    void addEvent(EventPtr);

    void schedule(StateMachine& machine);

private:
    std::vector<EventPtr> events;

};

module :private;

Scheduler::Scheduler(Settings &settings) {
    reset(settings);
}

void Scheduler::reset(Settings &settings) {
    events.clear();

    if (settings.getBoolean(Settings::PERIOD_ENABLE, Settings::DEFAULT_PERIOD_ENABLE))
        addEvent(std::make_shared<PeriodicEvent>(settings));

    auto cronStrings = settings.getSectionValues(Settings::CRON);

    for (auto& cronString : cronStrings) {
        std::shared_ptr<CronEvent> cronEvent;
        try {
            cronEvent = std::make_shared<CronEvent>(settings, cronString);
        }
        catch (std::exception&) {
        }

        if (cronEvent != nullptr && !cronEvent->isMalformed())
            addEvent(cronEvent);
    }
}

void Scheduler::addEvent(EventPtr event) {
    events.emplace_back(event);
}

void Scheduler::schedule(StateMachine& machine) {
    time_t currentTime = time(nullptr);

    for (auto &event: events) {
        if (event->isAlert(currentTime)) {
            machine.setAlert();
            return;
        }
        else if (event->isMonitoring(currentTime)) {
            machine.setMonitoring();
            return;
        }
    }

}
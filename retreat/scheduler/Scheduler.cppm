module;

#include <ctime>
#include <cstdlib>

export module Scheduler;

import <vector>;

import Event;
import Settings;
import StateMachine;

#include "debug.h"

export class Scheduler {
public:
    Scheduler(Settings& settings);

    void addEvent(EventPtr);

    void schedule(StateMachine& machine);

private:
    std::vector<EventPtr> events;

};

module :private;

Scheduler::Scheduler(Settings& settings) {

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
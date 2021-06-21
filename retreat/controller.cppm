module;

#include <ctime>
#include <cstdlib>

export module Controller;

import <memory>;
import <exception>;

import Settings;
import Scheduler;
import StateMachine;
import StateMachineImpl;
import PeriodicEvent;

#include "tstring.h"

export class Controller {
public: 

    Controller(void *appInstance);

    void onTimer();

    bool isAlert() { return stateMachine.isAlert(); }
    bool isSuspended() { return stateMachine.isSuspended(); }

    bool canDisable() { return stateMachine.canDisable(); }
    bool canEnable() { return stateMachine.isSuspended(); }
    bool canDelay() { return stateMachine.canDelay(); }
    bool canExit() { return stateMachine.canExit(); }
    bool canSkip() { return stateMachine.canSkip(); }

    void reset();
    void stop();
    void resume();

    void enable(bool enable);
    void delay() { stateMachine.setDelay(); }
    void skip() { stateMachine.setIdle(true); }
    void lock() { stateMachine.setLocked(); }

    Settings &getSettings() { return settings; }

private:
    void *appInstance;
    Settings settings;
    StateMachine stateMachine;
    Scheduler scheduler;
};


module :private;

Controller::Controller(void *appInstance): 
    appInstance(appInstance),
    stateMachine(settings, appInstance),
    scheduler(settings) {

    _tzset();
    srand((unsigned)time(nullptr));
}

void Controller::reset() {
    stateMachine.reset(settings);
    scheduler.reset(settings);
}

void Controller::stop() {
    stateMachine.setHalted();
}

void Controller::resume() {
    PeriodicEvent::resetStartTime(settings);
    stateMachine.setIdle();
}

void Controller::onTimer() {
    if (!scheduler.schedule(stateMachine))
        stateMachine.onTimer();
}

void Controller::enable(bool enable) {
    if (enable && stateMachine.isSuspended())
        stateMachine.setIdle();
    else if (!enable)
        stateMachine.setSuspended();
}

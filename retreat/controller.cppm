module;

#include <ctime>
#include <cstdlib>

export module Controller;

import <memory>;

import Settings;
import Scheduler;
import StateMachine;
import StateMachineImpl;
import PeriodicEvent;

export class Controller {
public: 

    Controller(void *appInstance);

    void onTimer();

    bool canDisable() { return stateMachine->canDisable(); }
    bool canEnable() { return stateMachine->isSuspended(); }
    bool canDelay() { return stateMachine->canDelay(); }
    bool canExit() { return stateMachine->canExit(); }
    bool canSkip() { return stateMachine->canSkip(); }

    void reset(std::shared_ptr<Settings> settings);
    void stop();
    void resume();

    void enable(bool enable);
    void delay() { stateMachine->setDelay(); }
    void skip() { stateMachine->setIdle(true); }
    void lock() { stateMachine->setLocked(); }

private:
    void *appInstance;
    std::shared_ptr<Settings> settings;
    std::shared_ptr<Scheduler> scheduler;
    std::shared_ptr<StateMachine> stateMachine;
};


module :private;

Controller::Controller(void *appInstance): appInstance(appInstance) {
    _tzset();
    srand((unsigned)time(nullptr));
}

void Controller::reset(std::shared_ptr<Settings> settings) {
    this->settings = settings;

    stateMachine = std::make_shared<StateMachine>(*settings, appInstance);
    scheduler = std::make_shared<Scheduler>(*settings);
    
    scheduler->addEvent(std::make_shared<PeriodicEvent>(*settings));
}

void Controller::stop() {
    stateMachine->setHalted();
}

void Controller::resume() {
    PeriodicEvent::reset(*settings);
    stateMachine->setIdle();
}

void Controller::onTimer() {
    if (scheduler != nullptr)
        scheduler->schedule(*stateMachine);

    if (stateMachine != nullptr)
        stateMachine->onTimer();
}

void Controller::enable(bool enable) {
    if (enable && stateMachine->isSuspended())
        stateMachine->setIdle();
    else if (!enable)
        stateMachine->setSuspended();
}

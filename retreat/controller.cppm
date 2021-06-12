module;

#include <ctime>
#include <cstdlib>

export module Controller;

import <memory>;

import Settings;
import Scheduler;
import SchedulerFactory;
import StateMachine;
import StateMachineImpl;

export class Controller {
public: 

    Controller();

    void updateSettings(std::shared_ptr<Settings> settings);

    void onTimer();

    bool canEnable() { return stateMachine->isSuspended(); }
    bool canDelay() { return stateMachine->canDelay(); }
    bool canExit() { return stateMachine->canExit(); }
    bool canSkip() { return stateMachine->canSkip(); }

    void enable(bool enable);
    void delay() { stateMachine->setDelay(); }
    void skip() { stateMachine->setIdle(true); }
    void lock() { stateMachine->setLocked(); }

private:
    std::shared_ptr<Settings> settings;
    std::shared_ptr<Scheduler> scheduler;
    std::shared_ptr<StateMachine> stateMachine;
};


module :private;

Controller::Controller() {
    _tzset();
    srand((unsigned)time(nullptr));
}

void Controller::updateSettings(std::shared_ptr<Settings> settings) {
    this->settings = settings;

    scheduler = SchedulerFactory::createScheduler(*settings);
    stateMachine = std::make_shared<StateMachine>(settings);
}

void Controller::onTimer() {
    if (scheduler != nullptr)
        scheduler->schedule(*stateMachine);

    stateMachine->onTimer();
}

void Controller::enable(bool enable) {
    if (enable && stateMachine->isSuspended())
        stateMachine->setIdle();
    else if (!enable)
        stateMachine->setSuspended();
}

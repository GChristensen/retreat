module;

#include <ctime>
#include <cstdlib>

export module Controller;

import <memory>;

import Settings;
import Scheduler;
import SchedulerFactory;
import StateMachine;

export class Controller {
public: 

    Controller();

    void updateSettings(std::shared_ptr<Settings> settings);

    void onTimer();

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
    stateMachine = std::make_shared<StateMachine>();
}

void Controller::onTimer() {
    if (scheduler != nullptr)
        scheduler->schedule(*stateMachine);
}

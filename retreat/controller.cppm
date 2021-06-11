module;

#include <time.h>

export module controller;

import settings;

export class Controller {
public: 

    Controller();

    void updateSettings(const Settings& settings);

    void onTimer();

private:
    Settings settings;
};


module :private;

Controller::Controller() {
    _tzset();
    srand((unsigned)time(0));
}

void Controller::updateSettings(const Settings& settings) {
    this->settings = settings;
}

void Controller::onTimer() {
}

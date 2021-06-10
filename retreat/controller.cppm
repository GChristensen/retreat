module;

export module controller;

import std.core;

import settings;

export class Controller {
public: 

    Controller(const Settings &settings);

private:
    Settings settings;
};


module :private;

Controller::Controller(const Settings& settings): settings(std::move(settings)) {

}


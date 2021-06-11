export module Scheduler;

import <memory>;

import Settings;
import StateMachine;

export class Scheduler {
public:
    virtual void schedule(StateMachine &machine) = 0;
};
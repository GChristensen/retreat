export module Scheduler;

import StateMachine;

export class Scheduler {
public:
    virtual void schedule(StateMachine &machine) = 0;
};
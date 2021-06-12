export module CronScheduler;

import Settings;
import Scheduler;
import StateMachine;

#include "tstring.h"

export class CronScheduler: public Scheduler {
public:
    CronScheduler(Settings &settings);

    virtual void schedule(StateMachine &machine) override;
};

module :private;

CronScheduler::CronScheduler(Settings &settings) {

}

void CronScheduler::schedule(StateMachine &machine) {
    
}

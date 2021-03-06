export module StateSuspended;

import State;
import StateMachine;

#include "debug.h"

export class StateSuspended : public State {
public:
    StateSuspended(StateMachine &stateMachine);

    virtual bool isSuspended() { return true; };

    virtual bool canAlert() { return false; };

    virtual void onTimer() override;

private:
    int counter;

    StateMachine &stateMachine;
};

module :private;

StateSuspended::StateSuspended(StateMachine& stateMachine): stateMachine(stateMachine) {
    counter = stateMachine.getSuspendedDuration();

    DBGLOG(_T("suspended"));
}

void StateSuspended::onTimer() {
    counter -= 1;

    if (!counter)
        stateMachine.setIdle();
}

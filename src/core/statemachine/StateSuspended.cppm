module;

#include "debug.h"

export module StateSuspended;

import State;
import StateMachine;
import StateSnapshot;


export class StateSuspended : public State {
public:
    StateSuspended(StateMachine &stateMachine, int remainingSec = 0);

    virtual bool isSuspended() { return true; };

    virtual bool canAlert() { return false; };

    virtual void onTimer() override;

    virtual StateKind kind() override { return StateKind::Suspended; }
    virtual int remainingSec() override { return counter; }

private:
    int counter;

    StateMachine &stateMachine;
};

module :private;

StateSuspended::StateSuspended(StateMachine& stateMachine, int remainingSec): stateMachine(stateMachine) {
    counter = remainingSec > 0 ? remainingSec : stateMachine.getSuspendedDuration();

    DBGLOG(_T("suspended"));
}

void StateSuspended::onTimer() {
    counter -= 1;

    if (!counter)
        stateMachine.setIdle();
}

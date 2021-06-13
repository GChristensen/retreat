export module StateLocked;

import State;
import StateDelay;

import StateMachine;
import LockWindowAdapter;

#include "debug.h"

export class StateLocked: public State {
public:
    StateLocked(StateMachine &stateMachine);

    virtual void onTimer() override;
    
private:
    int counter;

    StateMachine &stateMachine;

    std::unique_ptr<LockWindowAdapter> lockWindow;
};

module :private;

StateLocked::StateLocked(StateMachine& stateMachine): 
    stateMachine(stateMachine),
    lockWindow(std::make_unique<LockWindowAdapter>(*stateMachine.getSettings())) {
    
    counter = stateMachine.getBreakDuration();
    StateDelay::resetDelays();

    DBGLOG(_T("locked"));
}

void StateLocked::onTimer() {
    counter -= 1;

    lockWindow->onTimer();

    if (counter < 0)
        stateMachine.setIdle();
}

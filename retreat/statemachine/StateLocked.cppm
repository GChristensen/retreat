export module StateLocked;

import State;
import StateDelay;

import StateMachine;
import StateWindowAdapter;
import StateWindowFactory;

#include "debug.h"

export class StateLocked: public State {
public:
    StateLocked(StateMachine &stateMachine);

    virtual bool canAlert() { return false; };

    virtual void onTimer() override;
    
private:
    int counter;

    StateMachine &stateMachine;

    StateWindowPtr lockWindow;
};

module :private;

StateLocked::StateLocked(StateMachine& stateMachine): 
    stateMachine(stateMachine),
    lockWindow(StateWindowFactory::createStateWindow(
        StateWindowFactory::STATE_WINDOW_LOCK, stateMachine)) {
    
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

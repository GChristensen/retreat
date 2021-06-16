export module StateDelay;

import State;
import StateMachine;

#include "debug.h"
#include "tstring.h"

export class StateDelay : public State {
public:
    StateDelay(StateMachine& stateMachine);

    virtual void onTimer() override;

    virtual bool canExit() { return false; };

    static bool delaysAvailable(int maxAmount) {
        return delaysRequested < maxAmount;
    }

    static void reset() { delaysRequested = 0; }

private:
    int counter;

    static int delaysRequested;

    StateMachine& stateMachine;
};

module :private;

int StateDelay::delaysRequested = 0;

StateDelay::StateDelay(StateMachine& stateMachine) : stateMachine(stateMachine) {
    counter = stateMachine.getDelayDuration();

    delaysRequested += 1;

    DBGLOG(_T("delay"));
}

void StateDelay::onTimer() {
    counter -= 1;

    if (!counter) {
        if (delaysAvailable(stateMachine.getDelayAmount()))
            stateMachine.setAlert(); 
        else
            stateMachine.setLocked();
    }
}

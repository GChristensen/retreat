module;

#include "debug.h"
#include "tstring.h"

export module StateDelay;

import StateSnapshot;

import State;
import StateMachine;


export class StateDelay : public State {
public:
    StateDelay(StateMachine& stateMachine);
    // restored from a snapshot: does not count as a new delay
    StateDelay(StateMachine& stateMachine, int remainingSec);

    virtual void onTimer() override;

    virtual bool canExit() { return false; };

    static bool delaysAvailable(int maxAmount) {
        return delaysRequested < maxAmount;
    }

    static void reset() { delaysRequested = 0; }
    static int getDelaysRequested() { return delaysRequested; }
    static void setDelaysRequested(int amount) { delaysRequested = amount; }

    virtual StateKind kind() override { return StateKind::Delay; }
    virtual int remainingSec() override { return counter; }

private:
    int counter;

    static int delaysRequested;

    StateMachine &stateMachine;
};

module :private;

int StateDelay::delaysRequested = 0;

StateDelay::StateDelay(StateMachine& stateMachine) : stateMachine(stateMachine) {
    counter = stateMachine.getDelayDuration();

    delaysRequested += 1;

    DBGLOG(_T("delay"));
}

StateDelay::StateDelay(StateMachine& stateMachine, int remainingSec) : stateMachine(stateMachine) {
    counter = remainingSec > 0 ? remainingSec : 1;

    DBGLOG(_T("delay (restored)"));
}

void StateDelay::onTimer() {
    counter -= 1;

    if (!counter)
        stateMachine.setAlert(); 
        
}

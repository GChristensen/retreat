export module StateLocked;

import State;
import StateDelay;

import StateMachine;

#include "debug.h"

export class StateLocked: public State {
public:
    StateLocked(StateMachine &stateMachine);

    virtual void onTimer() override;
    
private:
    int counter;

    StateMachine &stateMachine;
};

module :private;

StateLocked::StateLocked(StateMachine& stateMachine): stateMachine(stateMachine) {
    counter = stateMachine.getBreakDuration();
    StateDelay::resetDelays();

    tcout << "locked" << std::endl;
}

void StateLocked::onTimer() {
    counter -= 1;

    if (!counter)
        stateMachine.setIdle();
}

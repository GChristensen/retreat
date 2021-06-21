export module StateLocked;

import State;
import StateDelay;

import StateMachine;
import StateWindowAdapter;
import StateWindowFactory;

#include "debug.h"
#include "tstring.h"

export class StateLocked: public State {
public:
    StateLocked(StateMachine &stateMachine);

    virtual bool canAlert() { return false; };

    virtual void onTimer() override;

    // used to set break length from cron events
    static void setLockDurationOverride(int duration);
    static void setMessageText(tstring text);
    static void reset();
    
private:
    int counter;
    static int lockDurationOverride;
    static tstring messageText;

    StateMachine &stateMachine;

    StateWindowPtr lockWindow;
};

module :private;

import system;

int StateLocked::lockDurationOverride = 0;
tstring StateLocked::messageText;

StateLocked::StateLocked(StateMachine& stateMachine): 
    stateMachine(stateMachine) {
    
    stateMachine.setBreakDurationOverride(lockDurationOverride);

    if (lockDurationOverride)
        counter = lockDurationOverride;
    else
        counter = stateMachine.getBreakDuration();

    lockWindow = StateWindowFactory::createStateWindow(StateWindowFactory::STATE_WINDOW_LOCK, stateMachine);

    DBGLOG(_T("locked"));
}

void StateLocked::onTimer() {
    counter -= 1;

    lockWindow->onTimer();

    if (!counter) {
        if (!messageText.empty())
            displayMessage(messageText);

        stateMachine.setIdle();
    }
}

void StateLocked::setLockDurationOverride(int duration) {
    lockDurationOverride = duration;
}

void StateLocked::setMessageText(tstring text) {
    messageText = text;
}

void StateLocked::reset() {
    lockDurationOverride = 0;
    messageText = _T("");
}
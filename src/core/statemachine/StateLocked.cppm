module;

#include <string>
#include "debug.h"
#include "tstring.h"

export module StateLocked;

import State;
import StateDelay;
import StateSnapshot;

import StateMachine;
import StateWindowAdapter;
import StateWindowFactory;



export class StateLocked: public State {
public:
    // remainingSec > 0: restored from a snapshot, lock for that long
    StateLocked(StateMachine &stateMachine, int remainingSec = 0);

    virtual bool canAlert() { return false; };

    virtual void onTimer() override;

    virtual StateKind kind() override { return StateKind::Locked; }
    virtual int remainingSec() override { return counter; }

    // used to set break length from cron events
    static void setLockDurationOverride(int duration);
    static void setMessageText(tstring text) {
        messageText = text;
    }
    static void reset();

    static int getLockDurationOverride() { return lockDurationOverride; }
    static const tstring& getMessageText() { return messageText; }
    
private:
    int counter;
    static int lockDurationOverride;
    static tstring messageText;

    StateMachine &stateMachine;

    StateWindowPtr lockWindow;
};

module :private;

int StateLocked::lockDurationOverride = 0;
tstring StateLocked::messageText;

StateLocked::StateLocked(StateMachine& stateMachine, int remainingSec): 
    stateMachine(stateMachine) {
    
    // the lock window reads the break length from the override
    stateMachine.setBreakDurationOverride(remainingSec > 0 ? remainingSec : lockDurationOverride);

    if (remainingSec > 0)
        counter = remainingSec;
    else if (lockDurationOverride)
        counter = lockDurationOverride;
    else
        counter = stateMachine.getBreakDuration();

    lockWindow = StateWindowFactory::createStateWindow(StateWindowFactory::STATE_WINDOW_LOCK, stateMachine);

    DBGLOG(_T("locked"));
}

void StateLocked::onTimer() {
    counter -= 1;

    lockWindow->onTimer();

    if (!counter)
        stateMachine.setIdle();
}

void StateLocked::setLockDurationOverride(int duration) {
    lockDurationOverride = duration;
}

//void StateLocked::setMessageText(tstring text) {
//    messageText = text;
//}

void StateLocked::reset() {
    lockDurationOverride = 0;
    messageText = _T("");
}
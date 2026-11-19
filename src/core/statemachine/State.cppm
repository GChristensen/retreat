module;

#include <memory>

export module State;

import StateSnapshot;


export class State {
public:
    virtual bool isMonitoring() { return false; };
    virtual bool isSuspended() { return false; };
    virtual bool isHalted() { return false; };
    virtual bool isAlert() { return false; };
    virtual bool isIdle() { return false; };

    virtual bool canDisable() { return false; };
    virtual bool canAlert() { return true; };
    virtual bool canDelay() { return false; };
    virtual bool canExit() { return true; }; 
    virtual bool canSkip() { return false; };

    virtual void onTimer() {};

    // for snapshots: what to restore and how many seconds are left in this state
    virtual StateKind kind() { return StateKind::Idle; }
    virtual int remainingSec() { return 0; }
};

export using StatePtr = std::shared_ptr<State>;
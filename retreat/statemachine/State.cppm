export module State;

import <memory>;

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
};

export using StatePtr = std::shared_ptr<State>;
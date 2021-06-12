export module State;

import <memory>;

export class State {
public:
    virtual void onTimer() {};
    virtual bool isSuspended() { return false; };
    virtual bool canDisable() { return false; };
    virtual bool canDelay() { return false; };
    virtual bool canExit() { return true; }; 
    virtual bool canSkip() { return false; };
};

export using StatePtr = std::shared_ptr<State>;
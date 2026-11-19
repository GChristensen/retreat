module;

#include "debug.h"

export module StateIdle;

import State;
import StateDelay;
import StateLocked;


export class StateIdle: public State {
public:
    StateIdle();

    virtual bool canDisable() override { return true; };  
    virtual bool isIdle() override { return true; };
};

module :private;

StateIdle::StateIdle() {
    StateLocked::reset();
    StateDelay::reset();
    
    DBGLOG(_T("idle"));
}

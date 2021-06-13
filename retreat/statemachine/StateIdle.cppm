export module StateIdle;

import State;

#include "debug.h"

export class StateIdle: public State {
public:
    StateIdle();

    virtual bool canDisable() override { return true; };    
};

module :private;

StateIdle::StateIdle() {
    DBGLOG(_T("idle"));
}

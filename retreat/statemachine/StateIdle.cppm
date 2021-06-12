export module StateIdle;

import State;

#include "debug.h"

export class StateIdle: public State {
public:

    StateIdle();
    
};

module :private;

StateIdle::StateIdle() {
    tcout << "idle" << std::endl;
}

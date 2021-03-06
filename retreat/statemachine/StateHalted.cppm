export module StateHalted;

import State;

#include "debug.h"

export class StateHalted: public State {
public:
    StateHalted();

    virtual bool isHalted() { return true; };

    virtual bool canAlert() { return false; };
};

module :private;

StateHalted::StateHalted() {
    DBGLOG(_T("stop"));
}

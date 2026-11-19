module;

#include <memory>

export module StateWindowFactory;


import StateMachine;
import StateWindowAdapter;

export class StateWindowFactory {
public:
    enum StateWindowType {
        STATE_WINDOW_ALERT,
        STATE_WINDOW_LOCK
    };

    static StateWindowPtr createStateWindow(StateWindowType type, StateMachine &stateMachine);
};

// createX() is implemented per platform, e.g. src/win32/factories_win32.cpp

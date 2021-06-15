export module StateWindowFactory;

import <memory>;

import StateMachine;
import StateWindowAdapter;
import AlertWindowAdapter;
import LockWindowAdapter;

export class StateWindowFactory {
public:
    enum StateWindowType {
        STATE_WINDOW_ALERT,
        STATE_WINDOW_LOCK
    };

    static StateWindowPtr createStateWindow(StateWindowType type, StateMachine &stateMachine);
};

module :private;

StateWindowPtr StateWindowFactory::createStateWindow(StateWindowType type, StateMachine &stateMachine) {
    switch (type) {
    case STATE_WINDOW_ALERT:
        return std::make_shared<AlertWindowAdapter>(stateMachine);
    case STATE_WINDOW_LOCK:
        return std::make_shared<LockWindowAdapter>(stateMachine);
    }

    return nullptr;
}
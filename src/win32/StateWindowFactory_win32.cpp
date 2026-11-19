module;

#include <memory>

module StateWindowFactory;

import StateMachine;
import StateWindowAdapter;
import AlertWindowAdapter;
import LockWindowAdapter;

StateWindowPtr StateWindowFactory::createStateWindow(StateWindowType type, StateMachine &stateMachine) {
    switch (type) {
    case STATE_WINDOW_ALERT:
        return std::make_shared<AlertWindowAdapter>(stateMachine);
    case STATE_WINDOW_LOCK:
        return std::make_shared<LockWindowAdapter>(stateMachine);
    }

    return nullptr;
}

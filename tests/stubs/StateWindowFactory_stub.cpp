module;

#include <memory>

module StateWindowFactory;

import StateMachine;
import StateWindowAdapter;

// No windows in tests: every state gets an inert adapter.
StateWindowPtr StateWindowFactory::createStateWindow(StateWindowType, StateMachine&) {
    return std::make_shared<StateWindowAdapter>();
}

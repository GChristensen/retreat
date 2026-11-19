module;

#include <memory>

module MonitorFactory;

import Monitor;
import StateMachine;
import InputMonitor;

MonitorPtr MonitorFactory::createMonitor(MonitorType type, StateMachine &stateMachine) {
    switch (type) {
    case INPUT_MONITOR:
        return std::make_shared<InputMonitor>(stateMachine.getAppInstance());
    }

    return nullptr;
}

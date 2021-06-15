export module MonitorFactory;

import <memory>;

import Monitor;
import InputMonitor;
import StateMachine;

export class MonitorFactory {
public:
    enum MonitorType {
        INPUT_MONITOR
    };

    static MonitorPtr createMonitor(MonitorType type, StateMachine &stateMachine);
};

module :private;

MonitorPtr MonitorFactory::createMonitor(MonitorType type, StateMachine &stateMachine) {
    switch (type) {
    case INPUT_MONITOR:
        return std::make_shared<InputMonitor>(stateMachine.getAppInstance());
    }

    return nullptr;
}
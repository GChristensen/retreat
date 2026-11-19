module;

#include <memory>

module MonitorFactory;

import Monitor;
import StateMachine;

namespace {
    class AlwaysSatisfied : public Monitor {
    public:
        bool isSatisfied() override { return true; }
    };
}

MonitorPtr MonitorFactory::createMonitor(MonitorType, StateMachine&) {
    return std::make_shared<AlwaysSatisfied>();
}

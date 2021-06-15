export module StateMonitoring;

import State;
import StateMachine;
import Monitor;
import MonitorFactory;

import "debug.h";

export class StateMonitoring: public State {
public:
    StateMonitoring(StateMachine &stateMachine);

    virtual bool isMonitoring() override { return true; };

    virtual bool canAlert() override { return inputMonitor->isSatisfied(); }

private:
    MonitorPtr inputMonitor;

    StateMachine& stateMachine;
};

module :private;

StateMonitoring::StateMonitoring(StateMachine& stateMachine): stateMachine(stateMachine) {
    inputMonitor = MonitorFactory::createMonitor(MonitorFactory::INPUT_MONITOR, stateMachine);
    DBGLOG(_T("monitoring"));
}


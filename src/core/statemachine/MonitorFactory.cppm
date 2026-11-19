module;

#include <memory>

export module MonitorFactory;


import Monitor;
import StateMachine;

export class MonitorFactory {
public:
    enum MonitorType {
        INPUT_MONITOR
    };

    static MonitorPtr createMonitor(MonitorType type, StateMachine &stateMachine);
};

// createX() is implemented per platform, e.g. src/win32/factories_win32.cpp

// May need to be compiled manually before StateMachineImpl

export module StateMachine;

import <memory>;
import <exception>;

import Settings;

import State;

export class StateMachine {
public:
    StateMachine(Settings &settings, void *appInstance);
    void reset(Settings& settings);

    void setIdle(bool skip = false);
    void setAlert();
    void setDelay();
    void setLocked();
    void setHalted();
    void setSuspended();
    void setMonitoring();

    void onTimer();

    bool isMonitoring();
    bool isSuspended();
    bool isHalted();
    bool isAlert();

    bool canDisable();
    bool canDelay();
    bool canExit();
    bool canSkip();

    int getSuspendedDuration() { return suspendedDurationSec; }
    void setBreakDurationOverride(int duration) { breakDurationOverrideSec = duration; }
    int getBreakDurationOverride() { return breakDurationOverrideSec; }
    int getBreakDuration() { return breakDurationSec; }
    int getAlertDuration() { return alertDurationSec; }
    int getDelayDuration() { return delayDurationSec; }
    int getDelayAmount() { return delayAmount; }

    Settings &getSettings() { return settings; }
    void *getAppInstance() { return appInstance; }

private:
    std::shared_ptr<State> state;

    Settings &settings;
    void *appInstance;

    int suspendedDurationSec;
    int breakDurationOverrideSec;
    int breakDurationSec;
    int alertDurationSec;
    int delayDurationSec;
    int delayAmount;
};

export using StateMachinePtr = std::shared_ptr<StateMachine>;

export class StateForbiddenException: public std::exception {
public:
    virtual char const* what() const { return "State forbidden."; }
};
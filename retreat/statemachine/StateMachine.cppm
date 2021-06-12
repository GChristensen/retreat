// May need to be compiled manually before StateMachineImpl

export module StateMachine;

import <memory>;

import Settings;

import State;

export class StateMachine {
public:
    StateMachine();
    StateMachine(SettingsPtr settings);

    void setIdle(bool skip = false);
    void setAlert();
    void setDelay();
    void setLocked();
    void setSuspended();

    void onTimer();

    bool isSuspended();
    bool canDisable();
    bool canDelay();
    bool canExit();
    bool canSkip();

    int getSuspendedDuration() { return suspendedDurationSec; }
    int getBreakDuration() { return breakDurationSec; }
    int getAlertDuration() { return alertDurationSec; }
    int getDelayDuration() { return delayDurationSec; }
    int getDelayAmount() { return delayAmount; }

    SettingsPtr &getSettings() { return settings; }

private:
    std::shared_ptr<State> state;

    SettingsPtr settings;

    int suspendedDurationSec;
    int breakDurationSec;
    int alertDurationSec;
    int delayDurationSec;
    int delayAmount;
};

export using StateMachinePtr = std::shared_ptr<StateMachine>;

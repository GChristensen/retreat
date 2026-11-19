module;

#include <ctime>
#include <cstdlib>

#include <memory>
#include "debug.h"

export module StateAlert;

import StateSnapshot;

import State;
import StateDelay;
import StateMachine;
import StateWindowAdapter;
import StateWindowFactory;


export class StateAlert: public State {
public:
    StateAlert(StateMachine& stateMachine);

    virtual void onTimer() override;
    virtual bool canDelay() override {
        return StateDelay::delaysAvailable(stateMachine.getDelayAmount());
    }

    virtual bool isAlert() override { return true; };

    virtual bool canAlert() override { return false; };
    virtual bool canExit() override { return false; }
    virtual bool canSkip() override { return skippable; }

    virtual StateKind kind() override { return StateKind::Alert; }
    virtual int remainingSec() override { return counter; }

    static int getCurrentDay();
    static int setSkipExpended();
    static bool setSkippable(int date, bool expended);

private:
    int counter;

    static int currentDay;
    static int skippable;

    StateMachine &stateMachine;

    StateWindowPtr alertWindow;
};

module :private;

int StateAlert::skippable = true;
int StateAlert::currentDay = 0;

int StateAlert::getCurrentDay() {
    tm local;
    time_t t = time(nullptr);
    localtime_s(&local, &t);
    return local.tm_mday;
}

int StateAlert::setSkipExpended() {
    skippable = false;
    return currentDay;
}

bool StateAlert::setSkippable(int date, bool expended) {
    if (date == getCurrentDay() && expended) {
        skippable = false;
        currentDay = date;
    }

    return skippable;
}

StateAlert::StateAlert(StateMachine &stateMachine): stateMachine(stateMachine) {

    counter = stateMachine.getAlertDuration();

    int currentDay = getCurrentDay();

    if (this->currentDay != currentDay) {
        this->currentDay = currentDay;
        skippable = true;
    }

    alertWindow = StateWindowFactory::createStateWindow(StateWindowFactory::STATE_WINDOW_ALERT, stateMachine);

    DBGLOG(_T("alert"));
}

void StateAlert::onTimer() {
    counter -= 1;

    alertWindow->onTimer();

    if (!counter)     
        stateMachine.setLocked();
}

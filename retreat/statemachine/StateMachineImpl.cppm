export module StateMachineImpl;

import <string>;

#include "tstring.h"

import StateMachine;

import StateIdle;
import StateAlert;
import StateDelay;
import StateLocked;
import StateSuspended;

StateMachine::StateMachine() {

}

StateMachine::StateMachine(SettingsPtr settings) {
    this->settings = settings;
    state = std::make_shared<StateIdle>();

    suspendedDurationSec = settings->getInt(Settings::SUSPENDED_DURATION, Settings::DEFAULT_SUSPENDED_DURATION) /** 60*/;
    breakDurationSec = settings->getInt(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION) /** 60*/;
    alertDurationSec = settings->getInt(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION) /** 60*/;
    delayDurationSec = settings->getInt(Settings::DELAY_DURATION, Settings::DEFAULT_DELAY_DURATION) /** 60*/;
    delayAmount = settings->getInt(Settings::DELAY_AMOUNT, Settings::DEFAULT_DELAY_AMOUNT);

    int skipDate = settings->getInt(Settings::SKIP_DATE, 0);
    int skipExpended = settings->getBoolean(Settings::SKIP_EXPENDED, false);

    bool skippable = StateAlert::setSkippable(skipDate, skipExpended);

    if (skippable && skipExpended) {
        this->settings->setString(Settings::SKIP_EXPENDED, _T("false"));
        this->settings->save();
    }
}

void StateMachine::setIdle(bool skip) {
    if (skip) {
        int currentDay = StateAlert::setSkipExpended();

        this->settings->setString(Settings::SKIP_DATE, to_tstring(currentDay));
        this->settings->setString(Settings::SKIP_EXPENDED, _T("true"));

        this->settings->save();
    }
    state = std::make_shared<StateIdle>();
}

void StateMachine::setAlert() {
    if (!state->isSuspended())
        state = std::make_shared<StateAlert>(*this);
}

void StateMachine::setDelay() {
    if (state->canDelay())
        state = std::make_shared<StateDelay>(*this);
}

void StateMachine::setLocked() {
    state = std::make_shared<StateLocked>(*this);
}

void StateMachine::setSuspended() {
    state = std::make_shared<StateSuspended>(*this);
}

void StateMachine::onTimer() {
    state->onTimer();
}

bool StateMachine::isSuspended() {
    return state->isSuspended();
}

bool StateMachine::canDisable() { 
    return state->canDisable();
};

bool StateMachine::canDelay() {
    return state->canDelay();
}

bool StateMachine::canExit() {
    return state->canExit();
}

bool StateMachine::canSkip() {
    return state->canSkip();
}

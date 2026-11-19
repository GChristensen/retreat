module;

#include <ctime>
#include <memory>
#include <string>
#include "tstring.h"
#include "debug.h"

export module StateMachineImpl;




import StateMachine;

import StateIdle;
import StateAlert;
import StateDelay;
import StateLocked;
import StateHalted;
import StateSuspended;
import StateMonitoring;
import StateSnapshot;
import StateRestore;

StateMachine::StateMachine(Settings &settings, void* appInstance): settings(settings), appInstance(appInstance) {

    reset(settings);
    state = std::make_shared<StateIdle>();
}

void StateMachine::reset(Settings &settings) {
    suspendedDurationSec = settings.getHoursInSec(Settings::SUSPENDED_DURATION, Settings::DEFAULT_SUSPENDED_DURATION);
    breakDurationSec = settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);
    alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);
    delayDurationSec = settings.getMinutesInSec(Settings::DELAY_DURATION, Settings::DEFAULT_DELAY_DURATION);
    delayAmount = settings.getInt(Settings::DELAY_AMOUNT, Settings::DEFAULT_DELAY_AMOUNT);

    int skipDate = settings.getInt(Settings::SKIP_DATE, 0);
    int skipExpended = settings.getBoolean(Settings::SKIP_EXPENDED, false);

    bool skippable = StateAlert::setSkippable(skipDate, skipExpended);

    if (skippable && skipExpended) {
        this->settings.setString(Settings::SKIP_EXPENDED, _T("false"));
        this->settings.save();
    }
}

void StateMachine::setIdle(bool skip) {
    if (skip && state->canSkip()) {
        int currentDay = StateAlert::setSkipExpended();

        settings.setInt(Settings::SKIP_DATE, currentDay);
        settings.setString(Settings::SKIP_EXPENDED, _T("true"));

        settings.save();
    }
    else if (skip && !state->canSkip())
        return;

    state = std::make_shared<StateIdle>();
}

void StateMachine::setHalted() {
    state = std::make_shared<StateHalted>();
}


void StateMachine::setAlert() {
    if (state->canAlert()) {
        try { 
            state = std::make_shared<StateAlert>(*this); 
        }
        catch (StateForbiddenException &) {
        }
    }

    if (state->isMonitoring())
        setIdle();
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

void StateMachine::setMonitoring() {
    if (state->isIdle())
        state = std::make_shared<StateMonitoring>(*this);
}

StateSnapshot StateMachine::snapshot(time_t now) {
    StateSnapshot s;

    s.kind = state->kind();

    int remaining = state->remainingSec();
    s.deadline = remaining > 0 ? (std::int64_t)(now + remaining) : 0;

    s.delaysRequested = StateDelay::getDelaysRequested();
    s.lockDurationOverride = StateLocked::getLockDurationOverride();
    s.setMessageText(StateLocked::getMessageText());

    return s;
}

StateKind StateMachine::kind() {
    return state->kind();
}

void StateMachine::apply(const RestorePlan& plan) {
    switch (plan.action) {
    case RestoreAction::Fresh:
    case RestoreAction::Idle:
        state = std::make_shared<StateIdle>();
        break;

    case RestoreAction::Delay:
        state = std::make_shared<StateIdle>();      // clears leftovers of the previous run
        StateDelay::setDelaysRequested(plan.delaysRequested);
        StateLocked::setLockDurationOverride(plan.lockDurationOverride);
        StateLocked::setMessageText(plan.messageText);
        state = std::make_shared<StateDelay>(*this, plan.remainingSec);
        break;

    case RestoreAction::Locked:
        state = std::make_shared<StateIdle>();
        StateDelay::setDelaysRequested(plan.delaysRequested);
        StateLocked::setLockDurationOverride(plan.lockDurationOverride);
        StateLocked::setMessageText(plan.messageText);
        state = std::make_shared<StateLocked>(*this, plan.remainingSec);
        break;

    case RestoreAction::Suspended:
        state = std::make_shared<StateIdle>();
        state = std::make_shared<StateSuspended>(*this, plan.remainingSec);
        break;
    }
}

void StateMachine::onTimer() {
    state->onTimer();
}

bool StateMachine::isMonitoring() {
    return state->isMonitoring();
}

bool StateMachine::isSuspended() {
    return state->isSuspended();
}

bool StateMachine::isHalted() {
    return state->isHalted();
}

bool StateMachine::isAlert() {
    return state->isAlert();
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

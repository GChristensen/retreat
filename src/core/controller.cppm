module;

#include <ctime>
#include <cstdlib>
#include <cstdint>

#include <memory>
#include <exception>
#include "tstring.h"

export module Controller;


import Settings;
import Scheduler;
import StateMachine;
import StateMachineImpl;
import StateSnapshot;
import StateRestore;
import PeriodicEvent;


export class Controller {
public: 

    Controller(void *appInstance);

    void onTimer();

    bool isAlert() { return stateMachine.isAlert(); }
    bool isSuspended() { return stateMachine.isSuspended(); }

    bool canDisable() { return stateMachine.canDisable(); }
    bool canEnable() { return stateMachine.isSuspended(); }
    bool canDelay() { return stateMachine.canDelay(); }
    bool canExit() { return stateMachine.canExit(); }
    bool canSkip() { return stateMachine.canSkip(); }

    void reset();
    void stop();
    // after the computer wakes up; `suspended` was taken (offline) when it went to sleep
    void resume(const StateSnapshot& suspended);

    void debug();

    void enable(bool enable);
    void delay() { stateMachine.setDelay(); }
    void skip() { stateMachine.setIdle(true); }
    void lock() { stateMachine.setLocked(); }

    StateKind kind() { return stateMachine.kind(); }

    // offline: the computer is going to sleep or the session ends;
    // cleanExit: the application quits normally
    StateSnapshot snapshot(bool offline = false, bool cleanExit = false);
    // continue from a snapshot left by a previous run (see StateRestore)
    RestorePlan restore(const StateSnapshot& snapshot, time_t bootTime);

    Settings &getSettings() { return settings; }

private:
    void *appInstance;
    Settings settings;
    StateMachine stateMachine;
    Scheduler scheduler;
    std::uint64_t seq = 0;

    RestoreContext restoreContext(time_t bootTime);
};


module :private;

Controller::Controller(void *appInstance): 
    appInstance(appInstance),
    stateMachine(settings, appInstance),
    scheduler(settings) {

    _tzset();
    srand((unsigned)time(nullptr));
}

void Controller::reset() {
    stateMachine.reset(settings);
    scheduler.reset(settings);
}

void Controller::stop() {
    stateMachine.setHalted();
}

void Controller::resume(const StateSnapshot& suspended) {
    restore(suspended, 0);
}

void Controller::debug() {
    scheduler.debug();
}

void Controller::onTimer() {
    scheduler.schedule(stateMachine);
}

void Controller::enable(bool enable) {
    if (enable && stateMachine.isSuspended())
        stateMachine.setIdle();
    else if (!enable)
        stateMachine.setSuspended();
}

StateSnapshot Controller::snapshot(bool offline, bool cleanExit) {
    time_t now = time(nullptr);

    StateSnapshot s = stateMachine.snapshot(now);
    s.seq = ++seq;
    s.writtenAt = now;
    s.periodStartTime = PeriodicEvent::getStartTime();
    s.offline = offline;
    s.cleanExit = cleanExit;

    return s;
}

RestoreContext Controller::restoreContext(time_t bootTime) {
    RestoreContext context;

    context.breakDurationSec = settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION);
    context.alertDurationSec = settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);
    context.staleSec = settings.getBoolean(Settings::PERIOD_ENABLE, Settings::DEFAULT_PERIOD_ENABLE)
        ? settings.getMinutesInSec(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION)
        : 60 * 60;
    context.bootTime = bootTime;
    context.missedAlert = [this](time_t from, time_t to) { return scheduler.missedAlert(from, to); };

    return context;
}

RestorePlan Controller::restore(const StateSnapshot& snapshot, time_t bootTime) {
    time_t now = time(nullptr);

    if (snapshot.isValid() && snapshot.seq > seq)
        seq = snapshot.seq;

    // the period start decides when scheduled alerts were due, so set it before planning
    if (snapshot.isValid() && !snapshot.cleanExit && snapshot.periodStartTime)
        PeriodicEvent::setStartTime((time_t)snapshot.periodStartTime);

    RestorePlan plan = planRestore(snapshot, now, restoreContext(bootTime));

    if (plan.action == RestoreAction::Fresh)
        PeriodicEvent::resetStartTime(settings);

    stateMachine.apply(plan);

    return plan;
}

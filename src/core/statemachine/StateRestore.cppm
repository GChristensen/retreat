module;

#include <algorithm>
#include <ctime>
#include <functional>
#include <optional>
#include <string>

#include "tstring.h"

// Decides what to do with a saved StateSnapshot when the application starts again or
// the computer wakes up. Pure logic, no side effects: see StateMachine::apply().
//
// Two situations are distinguished:
//  - offline: the snapshot was written on suspend or at the end of the session, or the
//    computer was restarted since. The time in between was spent away from the
//    computer, so it counts as break time.
//  - terminated: the application was killed while the computer was in use. A break that
//    was announced or under way resumes; one that was skipped by being dead is imposed
//    in full (penalty).
export module StateRestore;

import Event;
import StateSnapshot;


export struct RestoreContext {
    int breakDurationSec = 0;
    int alertDurationSec = 0;
    int staleSec = 0;           // a terminated snapshot older than this is ignored
    time_t bootTime = 0;        // when the computer was started (0 if unknown)

    // the earliest scheduled alert that starts in (from, to]
    std::function<std::optional<MissedAlert>(time_t from, time_t to)> missedAlert;
};

export enum class RestoreAction {
    Fresh,      // start as if launched for the first time
    Idle,       // idle, but keep the schedule
    Delay,
    Locked,
    Suspended,
};

export struct RestorePlan {
    RestoreAction action = RestoreAction::Fresh;
    int remainingSec = 0;           // Delay, Locked, Suspended
    time_t periodStartTime = 0;     // everything except Fresh
    int delaysRequested = 0;
    int lockDurationOverride = 0;   // full break length set by a cron event, 0 = default
    tstring messageText;            // shown during the break
    bool penalty = false;           // Locked imposed because a break was evaded
};

export RestorePlan planRestore(const StateSnapshot& snapshot, time_t now, const RestoreContext& context);


module :private;

static int fullBreak(int overrideSec, const RestoreContext& context) {
    return overrideSec > 0 ? overrideSec : context.breakDurationSec;
}

static RestorePlan locked(RestorePlan plan, int sec, bool penalty) {
    plan.action = RestoreAction::Locked;
    plan.remainingSec = sec;
    plan.penalty = penalty;
    return plan;
}

static RestorePlan withAction(RestorePlan plan, RestoreAction action, int remainingSec = 0) {
    plan.action = action;
    plan.remainingSec = remainingSec;
    return plan;
}

// break that may still be under way after an absence; Fresh if it is over
static RestorePlan resumeBreak(RestorePlan plan, time_t breakEnd, int fullSec, time_t now) {
    if (now < breakEnd)
        return locked(plan, (int)std::min<time_t>(fullSec, breakEnd - now), false);

    return withAction(plan, RestoreAction::Fresh);
}

static RestorePlan planOffline(const StateSnapshot& s, RestorePlan plan, time_t now, time_t gap,
                               const RestoreContext& context) {
    time_t remaining = s.deadline - now;
    int full = fullBreak(s.lockDurationOverride, context);

    switch (s.kind) {
    case StateKind::Locked:
        return remaining > 0 ? locked(plan, (int)remaining, false) : withAction(plan, RestoreAction::Fresh);

    case StateKind::Alert:
        // the break would have started when the alert ended
        return resumeBreak(plan, s.deadline + full, full, now);

    case StateKind::Delay:
        if (remaining > 0)
            return withAction(plan, RestoreAction::Delay, (int)remaining);
        return resumeBreak(plan, s.deadline + context.alertDurationSec + full, full, now);

    case StateKind::Suspended:
        return remaining > 0 ? withAction(plan, RestoreAction::Suspended, (int)remaining)
                             : withAction(plan, RestoreAction::Fresh);

    case StateKind::Idle:
    default:
        if (context.missedAlert) {
            // only an alert whose break could still be running is of interest
            time_t from = std::max<time_t>((time_t)s.writtenAt,
                now - context.alertDurationSec - context.breakDurationSec);
            if (auto missed = context.missedAlert(from, now)) {
                int missedFull = fullBreak(missed->breakDurationSec, context);
                plan.messageText = missed->messageText;
                plan.lockDurationOverride = missed->breakDurationSec;
                RestorePlan result = resumeBreak(plan,
                    missed->alertStart + context.alertDurationSec + missedFull, missedFull, now);
                if (result.action == RestoreAction::Locked)
                    return result;
            }
        }

        return gap >= context.breakDurationSec ? withAction(plan, RestoreAction::Fresh)
                                               : withAction(plan, RestoreAction::Idle);
    }
}

static RestorePlan planTerminated(const StateSnapshot& s, RestorePlan plan, time_t now, time_t gap,
                                  const RestoreContext& context) {
    if (context.staleSec > 0 && gap > context.staleSec)
        return withAction(plan, RestoreAction::Fresh);

    time_t remaining = s.deadline - now;
    int full = fullBreak(s.lockDurationOverride, context);

    // alerts missed while the application was dead (from `since` on) are imposed in full
    auto idle = [&](time_t since) {
        if (context.missedAlert) {
            if (auto missed = context.missedAlert(since, now)) {
                plan.messageText = missed->messageText;
                plan.lockDurationOverride = missed->breakDurationSec;
                return locked(plan, fullBreak(missed->breakDurationSec, context), true);
            }
        }
        return withAction(plan, RestoreAction::Idle);
    };

    switch (s.kind) {
    case StateKind::Locked:
        return remaining > 0 ? locked(plan, (int)remaining, false) : locked(plan, full, true);

    case StateKind::Alert:
        return locked(plan, full, true);

    case StateKind::Delay:
        return remaining > 0 ? withAction(plan, RestoreAction::Delay, (int)remaining)
                             : locked(plan, full, true);

    case StateKind::Suspended:
        if (remaining > 0)
            return withAction(plan, RestoreAction::Suspended, (int)remaining);
        return idle((time_t)s.deadline);

    case StateKind::Idle:
    default:
        return idle((time_t)s.writtenAt);
    }
}

RestorePlan planRestore(const StateSnapshot& snapshot, time_t now, const RestoreContext& context) {
    RestorePlan plan;

    if (!snapshot.isValid() || snapshot.cleanExit)
        return plan;

    plan.periodStartTime = (time_t)snapshot.periodStartTime;
    plan.delaysRequested = snapshot.delaysRequested;
    plan.lockDurationOverride = snapshot.lockDurationOverride;
    plan.messageText = snapshot.getMessageText();

    time_t gap = std::max<time_t>(0, now - (time_t)snapshot.writtenAt);
    bool offline = snapshot.offline || (context.bootTime && context.bootTime > snapshot.writtenAt);

    return offline ? planOffline(snapshot, plan, now, gap, context)
                   : planTerminated(snapshot, plan, now, gap, context);
}

// Smoke test for the portable core: builds and runs on Windows, Linux and macOS
// against the stub platform in stubs/.
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "tstring.h"
#include "scheduler/cron.h"

import Settings;
import SettingsModel;
import StateMachine;
import StateMachineImpl;
import StateSnapshot;
import StateRestore;
import Event;
import PeriodicEvent;

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n",                \
                         __FILE__, __LINE__, #cond);                         \
            ++failures;                                                      \
        }                                                                    \
    } while (0)

static void testCron() {
    // Five fields (no seconds): minute hour day-of-month month day-of-week.
    auto cex = cron::make_cron(_T("30 9 * * MON-FRI"));

    std::tm start{};
    start.tm_year = 2026 - 1900;
    start.tm_mon = 10;      // November
    start.tm_mday = 21;     // Saturday
    start.tm_hour = 12;
    start.tm_isdst = -1;

    std::tm next = cron::cron_next(cex, start);
    CHECK(next.tm_mday == 23);  // Monday
    CHECK(next.tm_hour == 9);
    CHECK(next.tm_min == 30);
    CHECK(next.tm_sec == 0);

    bool threw = false;
    try { cron::make_cron(_T("not a cron expression")); }
    catch (const cron::bad_cronexpr&) { threw = true; }
    CHECK(threw);
}

static void testSettingsRoundTrip() {
    auto dir = std::filesystem::temp_directory_path() / "retreat_core_smoke";
    std::filesystem::create_directories(dir);
    tstring file = (dir / "roundtrip.cfg").native();

    Settings out;
    out.setInt(Settings::BREAK_DURATION, 7);
    out.setBoolean(Settings::SOUNDS_ENABLE, true);
    out.setString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T("images"));
    out.save(file);

    Settings in;
    in.load(file);
    CHECK(in.getInt(Settings::BREAK_DURATION, 0) == 7);
    CHECK(in.getBoolean(Settings::SOUNDS_ENABLE, false));
    CHECK(in.getString(Settings::APPEARANCE_IMAGE_DIRECTORY, _T("")) == _T("images"));

    // non-ASCII text survives, and does not cut off the values after it
    Settings unicode;
    unicode.setString(_T("quote_list.q0000"), _T("“Rest is not idleness.” — John Lubbock"));
    unicode.setInt(_T("zzz.after"), 5);
    unicode.save(file);

    Settings unicodeIn;
    unicodeIn.load(file);
    CHECK(unicodeIn.getString(_T("quote_list.q0000"), _T("")) == _T("“Rest is not idleness.” — John Lubbock"));
    CHECK(unicodeIn.getInt(_T("zzz.after"), 0) == 5);

    // a file edited by hand in a legacy code page is still read to the end
    {
        std::ofstream legacy(std::filesystem::path(file), std::ios::binary);
        legacy << "[breaks]" << (char)10 << "message=caf" << (char)0xE9 << (char)10
               << "duration=9" << (char)10;
    }
    Settings legacyIn;
    legacyIn.load(file);
    CHECK(legacyIn.getInt(Settings::BREAK_DURATION, 0) == 9);
    CHECK(legacyIn.getString(_T("breaks.message"), _T("")).size() == 4);

    // a byte order mark added by an editor is skipped
    {
        std::ofstream bom(std::filesystem::path(file), std::ios::binary);
        bom << (char)0xEF << (char)0xBB << (char)0xBF << "[breaks]" << (char)10 << "duration=8" << (char)10;
    }
    Settings bomIn;
    bomIn.load(file);
    CHECK(bomIn.getInt(Settings::BREAK_DURATION, 0) == 8);

    // characters outside the basic plane survive
    Settings emoji;
    emoji.setString(_T("quote_list.q0000"), _T("Zen 🌸"));
    emoji.save(file);
    Settings emojiIn;
    emojiIn.load(file);
    CHECK(emojiIn.getString(_T("quote_list.q0000"), _T("")) == _T("Zen 🌸"));

    // quotes: built-in until saved, then exactly what was saved, in order
    Settings quotes;
    quotes.save(file);
    quotes.load(file);
    CHECK(loadQuoteLines(quotes) == defaultQuoteLines());

    std::vector<tstring> mine;
    for (int i = 0; i < 12; ++i)
        mine.push_back(_T("Quote ") + to_tstring(i) + _T(" — Author"));
    saveQuoteLines(quotes, mine);
    quotes.save(file);

    Settings quotesIn;
    quotesIn.load(file);
    CHECK(loadQuoteLines(quotesIn) == mine);

    saveQuoteLines(quotesIn, {});
    CHECK(loadQuoteLines(quotesIn).empty());

    std::filesystem::remove_all(dir);
}

static void testStateMachine() {
    Settings settings;
    StateMachine sm(settings, nullptr);

    CHECK(!sm.isAlert());
    sm.setAlert();
    CHECK(sm.isAlert());
    CHECK(sm.canDelay());
    sm.setDelay();
    CHECK(!sm.isAlert());
    sm.setSuspended();
    CHECK(sm.isSuspended());
    sm.setIdle();
    CHECK(!sm.isSuspended());
}

static void testSimpleSchedule() {
    auto weekdays = parseSimpleSchedule(_T("30 12 * * 1-5"));
    CHECK(weekdays.has_value());
    if (weekdays) {
        CHECK(weekdays->minute == 30);
        CHECK(weekdays->hour == 12);
        CHECK(isWeekdays(weekdays->days));
        CHECK(toCron(*weekdays) == _T("30 12 * * 1-5"));
        CHECK(describeSchedule(*weekdays) == _T("Weekdays at 12:30"));
    }

    auto friday = parseSimpleSchedule(_T("0 17 * * 5"));
    CHECK(friday && friday->days.count() == 1 && friday->days.test(4));
    CHECK(friday && toCron(*friday) == _T("0 17 * * 5"));

    auto daily = parseSimpleSchedule(_T("30 21 * * *"));
    CHECK(daily && isEveryDay(daily->days));
    CHECK(daily && toCron(*daily) == _T("30 21 * * *"));

    auto sunday0 = parseSimpleSchedule(_T("0 9 * * 0"));
    auto sunday7 = parseSimpleSchedule(_T("0 9 * * 7"));
    CHECK(sunday0 && sunday7 && *sunday0 == *sunday7 && sunday0->days.test(6));
    CHECK(sunday0 && toCron(*sunday0) == _T("0 9 * * 0"));

    auto names = parseSimpleSchedule(_T("30 9 * * MON-FRI"));
    CHECK(names && isWeekdays(names->days));

    auto weekend = parseSimpleSchedule(_T("0 10 * * 6,0"));
    CHECK(weekend && isWeekend(weekend->days));
    CHECK(weekend && toCron(*weekend) == _T("0 10 * * 0,6"));
    CHECK(weekend && describeDays(weekend->days) == _T("Weekends"));

    auto monToSat = parseSimpleSchedule(_T("0 15 * * 1,2,3,4,5,6"));
    CHECK(monToSat && toCron(*monToSat) == _T("0 15 * * 1-6"));
    CHECK(monToSat && describeDays(monToSat->days) == _T("Mon–Sat"));

    auto scattered = parseSimpleSchedule(_T("0 15 * * 1,3,4"));
    CHECK(scattered && toCron(*scattered) == _T("0 15 * * 1,3,4"));
    CHECK(scattered && describeDays(scattered->days) == _T("Mon, Wed, Thu"));

    CHECK(!parseSimpleSchedule(_T("*/15 * * * *")));
    CHECK(!parseSimpleSchedule(_T("0 9 1 * *")));
    CHECK(!parseSimpleSchedule(_T("0 9-17 * * *")));
    CHECK(!parseSimpleSchedule(_T("0 9 * 1 *")));
    CHECK(!parseSimpleSchedule(_T("60 9 * * *")));
    CHECK(!parseSimpleSchedule(_T("0 9 * *")));
}

static void testTimerOrigin() {
    // 1920x1080 screen, 100x40 text, full-screen margins
    auto tl = timerOrigin(TimerPosition::TopLeft, 1920, 1080, 100, 40, 160, 140);
    CHECK(tl.x == 160 && tl.y == 140);
    auto tr = timerOrigin(TimerPosition::TopRight, 1920, 1080, 100, 40, 160, 140);
    CHECK(tr.x == 1920 - 160 - 100 && tr.y == 140);
    auto bl = timerOrigin(TimerPosition::BottomLeft, 1920, 1080, 100, 40, 160, 140);
    CHECK(bl.x == 160 && bl.y == 1080 - 140 - 40);
    auto br = timerOrigin(TimerPosition::BottomRight, 1920, 1080, 100, 40, 160, 140);
    CHECK(br.x == 1660 && br.y == 900);
    auto c = timerOrigin(TimerPosition::Center, 1920, 1080, 100, 40, 160, 140);
    CHECK(c.x == 910 && c.y == 520);

    // a small window keeps the text inside
    auto small = timerOrigin(TimerPosition::BottomRight, 120, 50, 100, 40, 25, 25);
    CHECK(small.x == 0 && small.y == 0);
    auto big = timerOrigin(TimerPosition::TopLeft, 80, 30, 100, 40, 25, 25);
    CHECK(big.x == 0 && big.y == 0);

    CHECK(toTimerPosition(3) == TimerPosition::BottomRight);
    CHECK(toTimerPosition(9) == TimerPosition::TopLeft);
    CHECK(toTimerPosition(-1) == TimerPosition::TopLeft);

    CHECK(windowedTimerSize(32) == 22);
    CHECK(windowedTimerSize(72) == 50);
    CHECK(windowedTimerSize(12) == MIN_TIMER_SIZE);
}

static void testQuotes() {
    auto q = parseQuote(_T("  Rest is not idleness. — John Lubbock  "));
    CHECK(q.text == _T("Rest is not idleness.") && q.author == _T("John Lubbock"));

    // the last dash wins, and plain hyphens inside words do not count
    q = parseQuote(_T("Well-being - a state of mind - Anonymous"));
    CHECK(q.text == _T("Well-being - a state of mind") && q.author == _T("Anonymous"));

    q = parseQuote(_T("No author here"));
    CHECK(q.text == _T("No author here") && q.author.empty());
    q = parseQuote(_T("Trailing dash -- "));
    CHECK(q.text == _T("Trailing dash") && q.author.empty());
    q = parseQuote(_T("Text – en dash"));
    CHECK(q.text == _T("Text") && q.author == _T("en dash"));

    auto lines = splitQuoteLines(_T("one\r\n\r\n  two  \nthree\n"));
    CHECK(lines.size() == 3 && lines[0] == _T("one") && lines[1] == _T("two") && lines[2] == _T("three"));
    CHECK(joinQuoteLines(lines) == _T("one\r\ntwo\r\nthree"));
    CHECK(splitQuoteLines(_T(" \r\n ")).empty());

    for (const tstring& line : defaultQuoteLines())
        CHECK(!parseQuote(line).author.empty());

    CHECK(nextQuoteIndex(0, -1, false, 7) == -1);
    CHECK(nextQuoteIndex(1, 0, false, 7) == 0);
    CHECK(nextQuoteIndex(3, -1, true, 7) == 0);
    CHECK(nextQuoteIndex(3, 0, true, 7) == 1);
    CHECK(nextQuoteIndex(3, 2, true, 7) == 0);
    CHECK(nextQuoteIndex(3, 9, true, 7) == 0);
    for (unsigned r = 0; r < 20; ++r) {
        int i = nextQuoteIndex(4, 2, false, r);
        CHECK(i >= 0 && i < 4 && i != 2);
    }
    CHECK(nextQuoteIndex(4, -1, false, 6) == 2);

    CHECK(toQuoteImage(1) == QuoteImage::Custom);
    CHECK(toQuoteImage(5) == QuoteImage::Thinker);
}

static void testClampCycle() {
    auto normal = clampCycle(60, 10, 1);
    CHECK(normal.periodMin == 60 && normal.breakMin == 10 && normal.alertMin == 1);
    CHECK(normal.workMin() == 50 && normal.alertStartMin() == 49);

    auto longBreak = clampCycle(30, 40, 1);
    CHECK(longBreak.breakMin == 30 - MIN_WORK_MIN);

    auto longAlert = clampCycle(20, 10, 15);
    CHECK(longAlert.alertMin == longAlert.workMin() - 1);
    CHECK(longAlert.alertStartMin() >= 1);

    auto tiny = clampCycle(1, 0, 0);
    CHECK(tiny.periodMin == MIN_PERIOD_MIN && tiny.breakMin == 1 && tiny.alertMin == 1);
}


static StateSnapshot snap(StateKind kind, time_t writtenAt, time_t deadline) {
    StateSnapshot s;
    s.kind = kind;
    s.writtenAt = writtenAt;
    s.deadline = deadline;
    s.periodStartTime = 1000;
    s.delaysRequested = 1;
    return s;
}

static RestoreContext context(std::optional<MissedAlert> missed = std::nullopt) {
    RestoreContext c;
    c.breakDurationSec = 600;
    c.alertDurationSec = 60;
    c.staleSec = 3600;
    c.missedAlert = [missed](time_t from, time_t to) -> std::optional<MissedAlert> {
        if (missed && missed->alertStart > from && missed->alertStart <= to)
            return missed;
        return std::nullopt;
    };
    return c;
}

static void testRestoreTerminated() {
    const time_t t = 1000000;
    auto c = context();

    // invalid / clean exit: fresh start
    CHECK(planRestore(StateSnapshot::invalid(), t, c).action == RestoreAction::Fresh);
    auto clean = snap(StateKind::Locked, t - 1, t + 100);
    clean.cleanExit = 1;
    CHECK(planRestore(clean, t, c).action == RestoreAction::Fresh);

    // locked: the rest of the break, or the whole break if it was outlived
    auto p = planRestore(snap(StateKind::Locked, t - 1, t + 100), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 100 && !p.penalty);
    CHECK(p.periodStartTime == 1000 && p.delaysRequested == 1);
    p = planRestore(snap(StateKind::Locked, t - 200, t - 100), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 600 && p.penalty);

    // alert: always the whole break
    p = planRestore(snap(StateKind::Alert, t - 1, t + 30), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 600 && p.penalty);

    // a cron break keeps its own length and message
    auto cron = snap(StateKind::Alert, t - 1, t + 30);
    cron.lockDurationOverride = 300;
    cron.setMessageText(_T("lunch"));
    p = planRestore(cron, t, c);
    CHECK(p.remainingSec == 300 && p.lockDurationOverride == 300 && p.messageText == _T("lunch"));

    // delay: continue it, or the whole break once it ran out
    p = planRestore(snap(StateKind::Delay, t - 1, t + 50), t, c);
    CHECK(p.action == RestoreAction::Delay && p.remainingSec == 50);
    p = planRestore(snap(StateKind::Delay, t - 100, t - 10), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 600 && p.penalty);

    // suspended
    p = planRestore(snap(StateKind::Suspended, t - 1, t + 500), t, c);
    CHECK(p.action == RestoreAction::Suspended && p.remainingSec == 500);
    p = planRestore(snap(StateKind::Suspended, t - 100, t - 10), t, c);
    CHECK(p.action == RestoreAction::Idle);

    // idle: keep the schedule; an alert that came due while dead is imposed in full
    p = planRestore(snap(StateKind::Idle, t - 5, 0), t, c);
    CHECK(p.action == RestoreAction::Idle && p.periodStartTime == 1000);
    p = planRestore(snap(StateKind::Idle, t - 5, 0), t, context(MissedAlert{ t - 2, 0, _T("") }));
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 600 && p.penalty);
    p = planRestore(snap(StateKind::Idle, t - 5, 0), t, context(MissedAlert{ t - 2, 120, _T("cron") }));
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 120 && p.messageText == _T("cron"));

    // dead for longer than a period: start over
    CHECK(planRestore(snap(StateKind::Alert, t - 4000, t - 3970), t, c).action == RestoreAction::Fresh);
}

static void testRestoreOffline() {
    const time_t t = 1000000;
    auto c = context();

    auto offline = [&](StateKind kind, time_t writtenAt, time_t deadline) {
        auto s = snap(kind, writtenAt, deadline);
        s.offline = 1;
        return s;
    };

    // time away counts as break
    auto p = planRestore(offline(StateKind::Locked, t - 100, t + 100), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 100 && !p.penalty);
    CHECK(planRestore(offline(StateKind::Locked, t - 300, t - 100), t, c).action == RestoreAction::Fresh);

    // alert ended 100 s ago: the break started then
    p = planRestore(offline(StateKind::Alert, t - 130, t - 100), t, c);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 500 && !p.penalty);
    CHECK(planRestore(offline(StateKind::Alert, t - 1000, t - 990), t, c).action == RestoreAction::Fresh);

    p = planRestore(offline(StateKind::Delay, t - 10, t + 20), t, c);
    CHECK(p.action == RestoreAction::Delay && p.remainingSec == 20);

    // idle: a short absence keeps the schedule, a long one starts over
    CHECK(planRestore(offline(StateKind::Idle, t - 100, 0), t, c).action == RestoreAction::Idle);
    CHECK(planRestore(offline(StateKind::Idle, t - 700, 0), t, c).action == RestoreAction::Fresh);
    // ... unless a break is still under way: alert 100 s ago, break ends at +560
    p = planRestore(offline(StateKind::Idle, t - 700, 0), t, context(MissedAlert{ t - 100, 0, _T("") }));
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 560 && !p.penalty);

    // a reboot since the snapshot also means offline
    auto rebooted = context();
    rebooted.bootTime = t - 50;
    p = planRestore(snap(StateKind::Alert, t - 130, t - 100), t, rebooted);
    CHECK(p.action == RestoreAction::Locked && p.remainingSec == 500 && !p.penalty);
}

static void testPeriodicAlertBetween() {
    Settings settings;
    PeriodicEvent event(settings);

    int period = settings.getMinutesInSec(Settings::PERIOD_DURATION, Settings::DEFAULT_PERIOD_DURATION);
    int toAlert = period
        - settings.getMinutesInSec(Settings::BREAK_DURATION, Settings::DEFAULT_BREAK_DURATION)
        - settings.getMinutesInSec(Settings::ALERT_DURATION, Settings::DEFAULT_ALERT_DURATION);

    const time_t start = 1000000;
    PeriodicEvent::setStartTime(start);

    auto a = event.alertBetween(start, start + period);
    CHECK(a && a->alertStart == start + toAlert);
    CHECK(!event.alertBetween(start, start + toAlert - 1));
    CHECK(!event.alertBetween(start + toAlert, start + period));     // `from` is exclusive
    a = event.alertBetween(start + toAlert, start + 2 * period);
    CHECK(a && a->alertStart == start + period + toAlert);
    a = event.alertBetween(start + 5 * period + toAlert - 1, start + 5 * period + toAlert);
    CHECK(a && a->alertStart == start + 5 * period + toAlert);
}

static void testSnapshotApply() {
    Settings settings;
    StateMachine sm(settings, nullptr);

    sm.setAlert();
    sm.setDelay();
    auto s = sm.snapshot(1000);
    CHECK(s.kind == StateKind::Delay && s.deadline > 1000 && s.delaysRequested == 1);

    RestorePlan plan;
    plan.action = RestoreAction::Locked;
    plan.remainingSec = 42;
    plan.delaysRequested = 2;
    sm.apply(plan);
    CHECK(sm.kind() == StateKind::Locked);
    s = sm.snapshot(1000);
    CHECK(s.deadline == 1042 && s.delaysRequested == 2);

    plan.action = RestoreAction::Delay;
    plan.remainingSec = 7;
    sm.apply(plan);
    CHECK(sm.kind() == StateKind::Delay && sm.snapshot(0).delaysRequested == 2);

    plan.action = RestoreAction::Fresh;
    sm.apply(plan);
    CHECK(sm.kind() == StateKind::Idle);
}

template <typename F>
static void run(const char* name, F test) {
    try {
        test();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s: uncaught exception: %s\n", name, e.what());
        ++failures;
    } catch (...) {
        std::fprintf(stderr, "%s: uncaught non-std exception\n", name);
        ++failures;
    }
}

int main() {
    run("testCron", testCron);
    run("testSettingsRoundTrip", testSettingsRoundTrip);
    run("testStateMachine", testStateMachine);
    run("testSimpleSchedule", testSimpleSchedule);
    run("testClampCycle", testClampCycle);
    run("testTimerOrigin", testTimerOrigin);
    run("testQuotes", testQuotes);
    run("testRestoreTerminated", testRestoreTerminated);
    run("testRestoreOffline", testRestoreOffline);
    run("testPeriodicAlertBetween", testPeriodicAlertBetween);
    run("testSnapshotApply", testSnapshotApply);

    if (failures)
        std::fprintf(stderr, "%d check(s) failed\n", failures);
    else
        std::puts("core smoke test passed");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

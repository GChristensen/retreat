// Smoke test for the portable core: builds and runs on Windows, Linux and macOS
// against the stub platform in stubs/.
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>

#include "tstring.h"
#include "scheduler/cron.h"

import Settings;
import SettingsModel;
import StateMachine;
import StateMachineImpl;

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

    if (failures)
        std::fprintf(stderr, "%d check(s) failed\n", failures);
    else
        std::puts("core smoke test passed");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

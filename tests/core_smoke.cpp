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

    if (failures)
        std::fprintf(stderr, "%d check(s) failed\n", failures);
    else
        std::puts("core smoke test passed");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

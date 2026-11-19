// Pure helpers behind the settings UI: the visual scheduled-break editor and
// the break-cycle visualisation. No platform dependencies, unit-tested in
// tests/core_smoke.cpp.
module;

#include <algorithm>
#include <bitset>
#include <cwctype>
#include <optional>
#include <string>
#include <vector>
#include "tstring.h"

export module SettingsModel;

// Days are indexed Monday = 0 ... Sunday = 6.
export struct SimpleSchedule {
    int minute = 0;
    int hour = 0;
    std::bitset<7> days;

    bool operator==(const SimpleSchedule&) const = default;
};

export std::optional<SimpleSchedule> parseSimpleSchedule(const tstring& cron);
export tstring toCron(const SimpleSchedule& schedule);
export bool isEveryDay(const std::bitset<7>& days);
export bool isWeekdays(const std::bitset<7>& days);
export bool isWeekend(const std::bitset<7>& days);
export tstring describeDays(const std::bitset<7>& days);
export tstring describeSchedule(const SimpleSchedule& schedule);
export tstring formatClock(int hour, int minute);

// One periodic-break cycle, in minutes. The break takes the last `breakMin`
// minutes of the `periodMin` cycle; the warning precedes the break.
export struct CycleTiming {
    int periodMin;
    int breakMin;
    int alertMin;

    int workMin() const { return periodMin - breakMin; }
    int alertStartMin() const { return periodMin - breakMin - alertMin; }
};

export constexpr int MIN_PERIOD_MIN = 10;
export constexpr int MAX_PERIOD_MIN = 600;
export constexpr int MIN_WORK_MIN = 5;

// Keeps the values consistent: break leaves at least MIN_WORK_MIN of work,
// and the warning fits inside the work part.
export CycleTiming clampCycle(int periodMin, int breakMin, int alertMin);

// Where the break countdown is drawn. Stored as an int in
// appearance.timer_position.
export enum class TimerPosition { TopLeft = 0, TopRight, BottomLeft, BottomRight, Center };

export TimerPosition toTimerPosition(int value);

// Countdown sizes offered in the settings (points, full-screen mode).
export constexpr int TIMER_SIZES[] = { 24, 32, 48, 72 };
export constexpr int MIN_TIMER_SIZE = 12;
export constexpr int MAX_TIMER_SIZE = 200;

// Font size used in windowed mode, where the window is only as large as the image.
export int windowedTimerSize(int fullscreenSize);

export struct TimerOrigin { int x; int y; };

// Top-left corner of the countdown text inside an area of areaW x areaH.
// Corners keep marginX/marginY from the edges; the result never leaves the area.
export TimerOrigin timerOrigin(TimerPosition position, int areaW, int areaH,
                               int textW, int textH, int marginX, int marginY);

// The picture above a quote on the break screen. Stored as an int in
// quotes.image.
export enum class QuoteImage { Thinker = 0, Custom, None };

export QuoteImage toQuoteImage(int value);

// Quote sizes offered in the settings (points, full-screen mode).
export constexpr int QUOTE_SIZES[] = { 20, 26, 32, 40 };
export constexpr int MIN_QUOTE_SIZE = 10;
export constexpr int MAX_QUOTE_SIZE = 120;

export struct Quote {
    tstring text;
    tstring author;

    bool operator==(const Quote&) const = default;
};

// One quote per line: "Rest is not idleness. — John Lubbock". The author
// follows the last dash surrounded by spaces (an em or en dash, "--" or "-").
export Quote parseQuote(const tstring& line);

// Splits the text of the quote editor into lines, trimmed, without empty ones.
export std::vector<tstring> splitQuoteLines(const tstring& text);

// Joins quote lines for the editor (Windows line breaks).
export tstring joinQuoteLines(const std::vector<tstring>& lines);

// Quotes used until the user saves their own.
export std::vector<tstring> defaultQuoteLines();

// Index of the quote for the next break. `last` is the one shown last time
// (-1 if none); `random` is any non-negative random number. In random order
// the same quote is not shown twice in a row when there is a choice.
export int nextQuoteIndex(int count, int last, bool sequential, unsigned random);

module :private;

namespace {

std::vector<tstring> splitWhitespace(const tstring& s) {
    std::vector<tstring> parts;
    tstring current;
    for (TCHAR c : s) {
        if (c == _T(' ') || c == _T('\t')) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        }
        else
            current += c;
    }
    if (!current.empty())
        parts.push_back(current);
    return parts;
}

std::vector<tstring> split(const tstring& s, TCHAR sep) {
    std::vector<tstring> parts;
    tstring current;
    for (TCHAR c : s) {
        if (c == sep) {
            parts.push_back(current);
            current.clear();
        }
        else
            current += c;
    }
    parts.push_back(current);
    return parts;
}

std::optional<int> parseNumber(const tstring& s, int lo, int hi) {
    if (s.empty() || s.size() > 3)
        return std::nullopt;
    int value = 0;
    for (TCHAR c : s) {
        if (c < _T('0') || c > _T('9'))
            return std::nullopt;
        value = value * 10 + (c - _T('0'));
    }
    if (value < lo || value > hi)
        return std::nullopt;
    return value;
}

// Cron day of week: 0 or 7 = Sunday, 1 = Monday ... 6 = Saturday.
std::optional<int> parseCronDay(const tstring& s) {
    static const TCHAR* names[] = { _T("SUN"), _T("MON"), _T("TUE"), _T("WED"), _T("THU"), _T("FRI"), _T("SAT") };

    tstring upper;
    for (TCHAR c : s)
        upper += (TCHAR)std::towupper((wint_t)c);

    for (int i = 0; i < 7; ++i)
        if (upper == names[i])
            return i;

    return parseNumber(s, 0, 7);
}

int cronDayToIndex(int cronDay) {
    // Sunday (0 or 7) -> 6, Monday (1) -> 0
    return (cronDay + 6) % 7;
}

int indexToCronDay(int index) {
    return (index + 1) % 7;
}

std::optional<std::bitset<7>> parseDays(const tstring& field) {
    std::bitset<7> days;

    if (field == _T("*") || field == _T("?")) {
        days.set();
        return days;
    }

    for (const tstring& item : split(field, _T(','))) {
        auto range = split(item, _T('-'));

        if (range.size() == 1) {
            auto day = parseCronDay(range[0]);
            if (!day)
                return std::nullopt;
            days.set(cronDayToIndex(*day));
        }
        else if (range.size() == 2) {
            auto from = parseCronDay(range[0]);
            auto to = parseCronDay(range[1]);
            if (!from || !to || *from > *to)
                return std::nullopt;
            for (int d = *from; d <= *to; ++d)
                days.set(cronDayToIndex(d));
        }
        else
            return std::nullopt;
    }

    if (days.none())
        return std::nullopt;

    return days;
}

const TCHAR* DAY_NAMES[] = { _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat"), _T("Sun") };

bool isBlank(TCHAR c) {
    return c == _T(' ') || c == _T('\t') || c == _T('\r') || c == _T('\n') || c == _T('\u00A0');
}

tstring trim(const tstring& s) {
    size_t from = 0, to = s.size();
    while (from < to && isBlank(s[from]))
        ++from;
    while (to > from && isBlank(s[to - 1]))
        --to;
    return s.substr(from, to - from);
}

} // namespace

std::optional<SimpleSchedule> parseSimpleSchedule(const tstring& cron) {
    auto fields = splitWhitespace(cron);

    if (fields.size() != 5)
        return std::nullopt;

    auto minute = parseNumber(fields[0], 0, 59);
    auto hour = parseNumber(fields[1], 0, 23);

    if (!minute || !hour)
        return std::nullopt;

    if (fields[2] != _T("*") && fields[2] != _T("?"))
        return std::nullopt;

    if (fields[3] != _T("*"))
        return std::nullopt;

    auto days = parseDays(fields[4]);
    if (!days)
        return std::nullopt;

    SimpleSchedule schedule;
    schedule.minute = *minute;
    schedule.hour = *hour;
    schedule.days = *days;
    return schedule;
}

bool isEveryDay(const std::bitset<7>& days) {
    return days.all();
}

bool isWeekdays(const std::bitset<7>& days) {
    return days == std::bitset<7>(0b0011111);
}

bool isWeekend(const std::bitset<7>& days) {
    return days == std::bitset<7>(0b1100000);
}

// Joins sorted values, collapsing runs of three or more into "first<dash>last".
template <typename Format>
tstring joinRuns(const std::vector<int>& values, const tstring& separator, const tstring& dash, Format format) {
    tstring result;
    for (size_t i = 0; i < values.size();) {
        size_t j = i;
        while (j + 1 < values.size() && values[j + 1] == values[j] + 1)
            ++j;

        if (!result.empty())
            result += separator;

        if (j - i >= 2) {
            result += format(values[i]) + dash + format(values[j]);
            i = j + 1;
        }
        else {
            result += format(values[i]);
            ++i;
        }
    }
    return result;
}

tstring toCron(const SimpleSchedule& schedule) {
    tstring dow;

    if (isEveryDay(schedule.days) || schedule.days.none())
        dow = _T("*");
    else if (isWeekdays(schedule.days))
        dow = _T("1-5");
    else {
        std::vector<int> cronDays;
        for (int i = 0; i < 7; ++i)
            if (schedule.days.test(i))
                cronDays.push_back(indexToCronDay(i));
        std::sort(cronDays.begin(), cronDays.end());

        dow = joinRuns(cronDays, _T(","), _T("-"),
            [](int d) { return to_tstring(d); });
    }

    return to_tstring(schedule.minute) + _T(" ") + to_tstring(schedule.hour) + _T(" * * ") + dow;
}

tstring describeDays(const std::bitset<7>& days) {
    if (isEveryDay(days))
        return _T("Every day");
    if (days.none())
        return _T("Never");
    if (isWeekdays(days))
        return _T("Weekdays");
    if (isWeekend(days))
        return _T("Weekends");

    std::vector<int> indexes;
    for (int i = 0; i < 7; ++i)
        if (days.test(i))
            indexes.push_back(i);

    return joinRuns(indexes, _T(", "), _T("\u2013"),
        [](int d) { return tstring(DAY_NAMES[d]); });
}

tstring formatClock(int hour, int minute) {
    tstring result = to_tstring(hour) + _T(":");
    if (minute < 10)
        result += _T("0");
    return result + to_tstring(minute);
}

tstring describeSchedule(const SimpleSchedule& schedule) {
    return describeDays(schedule.days) + _T(" at ") + formatClock(schedule.hour, schedule.minute);
}

CycleTiming clampCycle(int periodMin, int breakMin, int alertMin) {
    CycleTiming t;
    t.periodMin = std::clamp(periodMin, MIN_PERIOD_MIN, MAX_PERIOD_MIN);
    t.breakMin = std::clamp(breakMin, 1, t.periodMin - MIN_WORK_MIN);
    t.alertMin = std::clamp(alertMin, 1, std::max(1, t.workMin() - 1));
    return t;
}

TimerPosition toTimerPosition(int value) {
    if (value < (int)TimerPosition::TopLeft || value > (int)TimerPosition::Center)
        return TimerPosition::TopLeft;
    return (TimerPosition)value;
}

int windowedTimerSize(int fullscreenSize) {
    // the original windowed timer was 10 pt smaller than the 32 pt full-screen one
    return std::max(MIN_TIMER_SIZE, (fullscreenSize * 22 + 16) / 32);
}

TimerOrigin timerOrigin(TimerPosition position, int areaW, int areaH,
                        int textW, int textH, int marginX, int marginY) {
    int left = marginX, top = marginY;
    int right = areaW - marginX - textW, bottom = areaH - marginY - textH;

    TimerOrigin origin{ left, top };

    switch (position) {
    case TimerPosition::TopLeft:     origin = { left, top }; break;
    case TimerPosition::TopRight:    origin = { right, top }; break;
    case TimerPosition::BottomLeft:  origin = { left, bottom }; break;
    case TimerPosition::BottomRight: origin = { right, bottom }; break;
    case TimerPosition::Center:      origin = { (areaW - textW) / 2, (areaH - textH) / 2 }; break;
    }

    origin.x = std::clamp(origin.x, 0, std::max(0, areaW - textW));
    origin.y = std::clamp(origin.y, 0, std::max(0, areaH - textH));
    return origin;
}

QuoteImage toQuoteImage(int value) {
    if (value < (int)QuoteImage::Thinker || value > (int)QuoteImage::None)
        return QuoteImage::Thinker;
    return (QuoteImage)value;
}

Quote parseQuote(const tstring& line) {
    static const TCHAR* separators[] = { _T(" \u2014 "), _T(" \u2013 "), _T(" -- "), _T(" - ") };

    tstring text = trim(line);

    // a dash with nothing after it
    for (const TCHAR* separator : separators) {
        tstring dangling = tstring(separator).substr(0, tstring(separator).size() - 1);
        if (text.size() > dangling.size() && text.ends_with(dangling))
            return { trim(text.substr(0, text.size() - dangling.size())), tstring() };
    }

    size_t best = tstring::npos, bestLength = 0;
    for (const TCHAR* separator : separators) {
        tstring sep(separator);
        size_t pos = text.rfind(sep);
        if (pos != tstring::npos && (best == tstring::npos || pos > best)) {
            best = pos;
            bestLength = sep.size();
        }
    }

    // no dash, or nothing but the author
    if (best == tstring::npos || best == 0)
        return { text, tstring() };

    return { trim(text.substr(0, best)), trim(text.substr(best + bestLength)) };
}

std::vector<tstring> splitQuoteLines(const tstring& text) {
    std::vector<tstring> lines;
    for (const tstring& line : split(text, _T('\n'))) {
        tstring trimmed = trim(line);
        if (!trimmed.empty())
            lines.push_back(trimmed);
    }
    return lines;
}

tstring joinQuoteLines(const std::vector<tstring>& lines) {
    tstring result;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0)
            result += _T("\r\n");
        result += lines[i];
    }
    return result;
}

std::vector<tstring> defaultQuoteLines() {
    return {
        _T("Very little is needed to make a happy life; it is all within yourself, in your way of thinking. \u2014 Marcus Aurelius"),
        _T("Rest is not idleness. \u2014 John Lubbock"),
        _T("Nature does not hurry, yet everything is accomplished. \u2014 Lao Tzu"),
        _T("Sitting quietly, doing nothing, spring comes, and the grass grows by itself. \u2014 Zen saying"),
        _T("The unexamined life is not worth living. \u2014 Socrates"),
        _T("It is not that we have a short time to live, but that we waste a lot of it. \u2014 Seneca"),
    };
}

int nextQuoteIndex(int count, int last, bool sequential, unsigned random) {
    if (count <= 0)
        return -1;
    if (count == 1)
        return 0;

    if (sequential)
        return (last < 0 || last >= count - 1) ? 0 : last + 1;

    if (last < 0 || last >= count)
        return (int)(random % (unsigned)count);

    // any quote but the last one
    int pick = (int)(random % (unsigned)(count - 1));
    return pick >= last ? pick + 1 : pick;
}

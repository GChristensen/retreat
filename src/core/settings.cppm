module;

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <regex>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include "tstring.h"
#include "debug.h"

export module Settings;




const TCHAR *CONFIG_FILE_NAME = _T("retreat.cfg");

export const TCHAR *DEFAULT_TIMER_FONT_FACE = _T("Arial");
export const int DEFAULT_TIMER_FONT_SIZE = 32;
export const int DEFAULT_TIMER_X_WINDOWED = 25;
export const int DEFAULT_TIMER_Y_WINDOWED = 25;
export const int DEFAULT_TIMER_X = 160;
export const int DEFAULT_TIMER_Y = 140;

export const TCHAR *DEFAULT_QUOTES_FONT_FACE = _T("Georgia");

export class Settings {
public:
    static constexpr const TCHAR *PERIOD_ENABLE = _T("periods.enable");
    static constexpr const TCHAR *PERIOD_DURATION = _T("periods.duration");
    static constexpr const TCHAR *PERIOD_FROM_LAUNCH = _T("periods.from_launch_time");

    static constexpr const TCHAR *BREAK_DURATION = _T("breaks.duration");

    static constexpr const TCHAR* ALERT_DURATION = _T("alerts.duration");

    static constexpr const TCHAR *DELAY_AMOUNT = _T("delays.amount");
    static constexpr const TCHAR *DELAY_DURATION = _T("delays.duration");

    static constexpr const TCHAR *SUSPENDED_DURATION = _T("suspended.duration");

    static constexpr const TCHAR *SKIP_DATE = _T("skip.date");
    static constexpr const TCHAR *SKIP_EXPENDED = _T("skip.expended");

    static constexpr const TCHAR *APPEARANCE_FULLSCREEN = _T("appearance.fullscreen");
    static constexpr const TCHAR *APPEARANCE_TRANSPARENT = _T("appearance.transparent");
    static constexpr const TCHAR *APPEARANCE_OPACITY_LEVEL = _T("appearance.opacity_level");
    static constexpr const TCHAR *APPEARANCE_BACKGROUND_COLOR = _T("appearance.background_color");
    static constexpr const TCHAR *APPEARANCE_IMAGE_DIRECTORY = _T("appearance.image_directory");
    static constexpr const TCHAR *APPEARANCE_USE_IMAGES = _T("appearance.use_images");
    static constexpr const TCHAR *APPEARANCE_TIMER_POSITION = _T("appearance.timer_position");
    static constexpr const TCHAR *APPEARANCE_TIMER_SIZE = _T("appearance.timer_size");
    static constexpr const TCHAR *APPEARANCE_STRETCH_IMAGES = _T("appearance.stretch_images");
    static constexpr const TCHAR *APPEARANCE_SHOW_TIMER = _T("appearance.show_timer");
    static constexpr const TCHAR *APPEARANCE_TIMER_TEXT_COLOR = _T("appearance.timer_text_color");

    static constexpr const TCHAR *QUOTES_ENABLE = _T("quotes.enable");
    static constexpr const TCHAR *QUOTES_PLAIN_BACKDROP = _T("quotes.plain_backdrop");
    static constexpr const TCHAR *QUOTES_FONT_FACE = _T("quotes.font_face");
    static constexpr const TCHAR *QUOTES_FONT_ITALIC = _T("quotes.font_italic");
    static constexpr const TCHAR *QUOTES_FONT_SIZE = _T("quotes.font_size");
    static constexpr const TCHAR *QUOTES_TEXT_COLOR = _T("quotes.text_color");
    static constexpr const TCHAR *QUOTES_IMAGE = _T("quotes.image");
    static constexpr const TCHAR *QUOTES_IMAGE_FILE = _T("quotes.image_file");
    static constexpr const TCHAR *QUOTES_SEQUENTIAL = _T("quotes.sequential");
    static constexpr const TCHAR *QUOTES_LAST_INDEX = _T("quotes.last_index");
    // number of entries in QUOTE_LIST; absent until the quotes are first saved,
    // which means the built-in ones are used
    static constexpr const TCHAR *QUOTES_COUNT = _T("quotes.count");
    // not under "quotes.": every value in that section is a quote
    static constexpr const TCHAR *QUOTE_LIST = _T("quote_list.");

    static constexpr const TCHAR *SOUNDS_ENABLE = _T("sounds.enable");
    static constexpr const TCHAR *SOUNDS_AUDIO_DIRECTORY = _T("sounds.audio_directory");

    static constexpr const TCHAR *MONITORING_INPUT = _T("monitoring.input");
    static constexpr const TCHAR *MONITORING_PROCESSES = _T("monitoring.processes");
    static constexpr const TCHAR *MONITORING_INACTIVITY = _T("monitoring.inactivity");

    static constexpr const TCHAR* PROCESSES = _T("processes.");
    static constexpr const TCHAR* CRON = _T("cron.");
    // not under "cron.": every value in that section is a break
    static constexpr const TCHAR* SCHEDULE_ENABLE = _T("schedule.enable");

    static constexpr const TCHAR* RESTART_ON_TERMINATION = _T("behavior.restart_on_termination");

    static constexpr const bool DEFAULT_PERIOD_ENABLE = true;
    static constexpr const bool DEFAULT_SCHEDULE_ENABLE = true;
    static constexpr const int DEFAULT_PERIOD_DURATION = 60;
    static constexpr const bool DEFAULT_PERIOD_FROM_LAUNCH = false;

    static constexpr const int DEFAULT_BREAK_DURATION = 10;

    static constexpr const int DEFAULT_ALERT_DURATION = 1;

    static constexpr const int DEFAULT_DELAY_AMOUNT = 2;
    static constexpr const int DEFAULT_DELAY_DURATION = 3;
    
    static constexpr const int DEFAULT_SUSPENDED_DURATION = 2;

    static constexpr const bool DEFAULT_BEHAVIOR_BEEP = false;
    static constexpr const bool DEFAULT_RESTART_ON_TERMINATION = true;

    static constexpr const bool DEFAULT_APPEARANCE_FULLSCREEN = true;
    static constexpr const bool DEFAULT_APPEARANCE_TRANSPARENT = false;
    static constexpr const int DEFAULT_APPEARANCE_OPACITY_LEVEL = 128;
    static constexpr const int DEFAULT_APPEARANCE_BACKGROUND_COLOR = 0xFFFFFF;
    static constexpr const bool DEFAULT_APPEARANCE_STRETCH_IMAGES = true;
    static constexpr const bool DEFAULT_APPEARANCE_SHOW_TIMER = true;
    static constexpr const int DEFAULT_APPEARANCE_TIMER_TEXT_COLOR = 0x1B1BEF;
    static constexpr const int DEFAULT_APPEARANCE_TIMER_POSITION = 0;   // TimerPosition::TopLeft
    static constexpr const int DEFAULT_APPEARANCE_TIMER_SIZE = 32;      // points, full screen

    static constexpr const bool DEFAULT_QUOTES_ENABLE = false;
    static constexpr const bool DEFAULT_QUOTES_PLAIN_BACKDROP = false;
    static constexpr const bool DEFAULT_QUOTES_FONT_ITALIC = true;
    static constexpr const int DEFAULT_QUOTES_FONT_SIZE = 26;           // points, full screen
    static constexpr const int DEFAULT_QUOTES_TEXT_COLOR = 0xD6E6EF;    // warm white, RGB(EF, E6, D6)
    static constexpr const int DEFAULT_QUOTES_IMAGE = 0;                // QuoteImage::Thinker
    static constexpr const bool DEFAULT_QUOTES_SEQUENTIAL = false;

    static constexpr const bool DEFAULT_SOUNDS_ENABLE = false;

    static constexpr const bool DEFAULT_MONITORING_INPUT = false;
    static constexpr const bool DEFAULT_MONITORING_PROCESSES = false;
    static constexpr const int DEFAULT_MONITORING_INACTIVITY = 10;

    Settings();

    void load(const tstring& file);
    void save(const tstring& file);
    void save();

    tstring getString(const tstring path, const tstring defaultValue);
    bool getBoolean(const tstring path, bool defaultValue);
    int getInt(const tstring path, int defaultValue);
    int getMinutesInSec(const tstring path, int defaultValue);
    int getHoursInSec(const tstring path, int defaultValue);

    auto getSection(tstring name);
    auto getSectionValues(tstring name);
    void clearSection(tstring name);
    
    void setString(const tstring path, const tstring value);
    void setBoolean(const tstring path, bool value);
    void setInt(const tstring path, int value);

    auto split(tstring &value);
    tstring join(const std::vector<tstring> &list);

private:
    tstring file;
    std::map<tstring, tstring> values;

    void readConfig(const tstring& file, std::map<tstring, tstring>& values);
    void writeConfig(const tstring& file, const std::map<tstring, tstring>& values);

    void handlePreviousVersions();
};

export using SettingsPtr = std::shared_ptr<Settings>;

// The quotes shown on the break screen, one "text — author" line each; the
// built-in ones until the user has saved a list.
export std::vector<tstring> loadQuoteLines(Settings& settings);
export void saveQuoteLines(Settings& settings, const std::vector<tstring>& lines);

module :private;

import system;
import SettingsModel;

// The file is UTF-8, so that quotes and break messages may hold any text.

static void appendCodePoint(std::wstring& out, uint32_t cp) {
    if (sizeof(wchar_t) == 2 && cp > 0xFFFF) {
        cp -= 0x10000;
        out += (wchar_t)(0xD800 + (cp >> 10));
        out += (wchar_t)(0xDC00 + (cp & 0x3FF));
    }
    else
        out += (wchar_t)cp;
}

// Strict decoding; nothing if the bytes are not UTF-8. A byte order mark is skipped.
static std::optional<std::wstring> fromUtf8(const std::string& bytes) {
    std::wstring out;
    size_t i = 0, n = bytes.size();

    if (n >= 3 && (uint8_t)bytes[0] == 0xEF && (uint8_t)bytes[1] == 0xBB && (uint8_t)bytes[2] == 0xBF)
        i = 3;

    while (i < n) {
        uint8_t c = (uint8_t)bytes[i];
        uint32_t cp;
        size_t extra;

        if (c < 0x80) { cp = c; extra = 0; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; extra = 1; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; extra = 2; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; extra = 3; }
        else return std::nullopt;

        if (i + extra >= n && extra > 0)
            return std::nullopt;

        for (size_t k = 1; k <= extra; ++k) {
            uint8_t cc = (uint8_t)bytes[i + k];
            if ((cc & 0xC0) != 0x80)
                return std::nullopt;
            cp = (cp << 6) | (cc & 0x3F);
        }

        bool overlong = (extra == 1 && cp < 0x80) || (extra == 2 && cp < 0x800) || (extra == 3 && cp < 0x10000);
        if (overlong || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return std::nullopt;

        appendCodePoint(out, cp);
        i += extra + 1;
    }

    return out;
}

static std::string toUtf8(const std::wstring& text) {
    std::string out;

    for (size_t i = 0; i < text.size(); ++i) {
        uint32_t cp = (uint32_t)text[i];

        if (sizeof(wchar_t) == 2 && cp >= 0xD800 && cp <= 0xDBFF && i + 1 < text.size()) {
            uint32_t low = (uint32_t)text[i + 1];
            if (low >= 0xDC00 && low <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                ++i;
            }
        }

        if (cp < 0x80)
            out += (char)cp;
        else if (cp < 0x800) {
            out += (char)(0xC0 | (cp >> 6));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000) {
            out += (char)(0xE0 | (cp >> 12));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else {
            out += (char)(0xF0 | (cp >> 18));
            out += (char)(0x80 | ((cp >> 12) & 0x3F));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
    }

    return out;
}

Settings::Settings() {
    file = getConfigFilePath(CONFIG_FILE_NAME);

    load(file);
    handlePreviousVersions();
}

void Settings::readConfig(const tstring& file, std::map<tstring, tstring>& values) {
    tptree ptree;
    std::ifstream iconfig(std::filesystem::path(file), std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(iconfig)), std::istreambuf_iterator<char>());

    // not UTF-8: a file edited by hand in a legacy code page, read byte by byte as before
    std::optional<std::wstring> text = fromUtf8(bytes);
    if (!text) {
        text.emplace();
        for (char c : bytes)
            *text += (wchar_t)(uint8_t)c;
    }

    tistringstream iconfigText(*text);
    read_ini(iconfigText, ptree);

    values.clear();

    for (auto& [section, settings] : ptree) {
        for (auto& [setting, value] : settings) {
            values[section + _T(".") + setting] = value.data();
        }
    }
}

void Settings::writeConfig(const tstring& file, const std::map<tstring, tstring>& values) {
    tptree ptree;

    for (auto& [path, value] : values)
        ptree.put(path, value);

    tostringstream oconfigText;
    write_ini(oconfigText, ptree);

    std::ofstream oconfig(std::filesystem::path(file), std::ios::binary);
    std::string bytes = toUtf8(oconfigText.str());
    oconfig.write(bytes.data(), (std::streamsize)bytes.size());
}

void Settings::load(const tstring& file) {
    readConfig(file, values);
}

void Settings::save() {
    writeConfig(file, values);
}

void Settings::save(const tstring& file) {
    writeConfig(file, values);
}

tstring Settings::getString(const tstring path, const tstring defaultValue) {
    if (values.find(path) == values.end() || values[path].empty())
        return defaultValue;

    return values[path];
}

bool Settings::getBoolean(const tstring path, bool defaultValue) {
    if (values.find(path) == values.end() || values[path].empty())
        return defaultValue;

    return !values[path].compare(_T("true"));
}

int Settings::getInt(const tstring path, int defaultValue) {
    if (values.find(path) == values.end() || values[path].empty())
        return defaultValue;

    int result;
    
    try {
        result = std::stoi(values[path]);
    }
    catch (std::exception &) {
        return defaultValue;
    }

    return result;
}

int Settings::getMinutesInSec(const tstring path, int defaultValue) {
    int result = getInt(std::move(path), defaultValue);

    return result * DBG_SECONDS;
}

int Settings::getHoursInSec(const tstring path, int defaultValue) {
    int result = getInt(std::move(path), defaultValue);

    return result * DBG_SECONDS * 60;
}

auto Settings::getSection(tstring name) {
    std::vector<std::pair<tstring, tstring>> contents;

    for (auto &vpair : values)
        if (vpair.first.starts_with(name))
            contents.emplace_back(vpair);

    return contents;
}

auto Settings::getSectionValues(tstring name) {
    std::vector<tstring> contents;

    for (auto &vpair : values)
        if (vpair.first.starts_with(name))
            contents.emplace_back(vpair.second);

    return contents;
}

void Settings::clearSection(tstring name) {
    std::erase_if(values, [&name](const auto& item) { return item.first.starts_with(name); });
}

void Settings::setString(const tstring path, const tstring value) {
    values[path] = value;
}

void Settings::setBoolean(const tstring path, bool value) {
    values[path] = value ? _T("true") : _T("false");
}

void Settings::setInt(const tstring path, int value) {
    values[path] = to_tstring(value);
}

auto Settings::split(tstring &value) {
    tregex reg(_T("\\|"));
    tregex_token_iterator iter(value.begin(), value.end(), reg, -1);

    return std::vector<tstring>(iter, tregex_token_iterator());
}

tstring Settings::join(const std::vector<tstring>& list) {
    tstring result;
    
    int n = list.size() - 1;
    for (auto &elem : list) {
        result.append(elem);
        if (n-- > 0)
            result.append(_T("|"));
    }

    return result;
}

void Settings::handlePreviousVersions() {
    std::map<tstring, tstring>::iterator iter;

    if (iter = values.find(_T("General.Behaviour:PeriodLengthMin")); iter != values.end()) {
        setString(PERIOD_DURATION, (*iter).second);

        if (iter = values.find(_T("General.Behaviour:BreakLengthMin")); iter != values.end())
            setString(BREAK_DURATION, (*iter).second);

        if (iter = values.find(_T("AngelicMachine.Appearance:Windowed:ImageFolder")); iter != values.end())
            setString(APPEARANCE_IMAGE_DIRECTORY, (*iter).second);

        if (iter = values.find(_T("AngelicMachine.Appearance:TransparencyLevel")); iter != values.end())
            setString(APPEARANCE_OPACITY_LEVEL, (*iter).second);

        std::erase_if(values, [](const auto& item) {
            return any_of(item.first.begin(), item.first.end(), [](TCHAR c) { return c >= 65 && c <= 90; });
        });

        save();
    }
}

std::vector<tstring> loadQuoteLines(Settings& settings) {
    if (settings.getInt(Settings::QUOTES_COUNT, -1) < 0)
        return defaultQuoteLines();

    // keys are zero-padded, so the map keeps them in order
    return settings.getSectionValues(Settings::QUOTE_LIST);
}

void saveQuoteLines(Settings& settings, const std::vector<tstring>& lines) {
    settings.clearSection(Settings::QUOTE_LIST);

    for (size_t i = 0; i < lines.size(); ++i) {
        tstring number = to_tstring(i);
        number.insert(0, number.size() < 4 ? 4 - number.size() : 0, _T('0'));
        settings.setString(tstring(Settings::QUOTE_LIST) + _T("q") + number, lines[i]);
    }

    settings.setInt(Settings::QUOTES_COUNT, (int)lines.size());
}

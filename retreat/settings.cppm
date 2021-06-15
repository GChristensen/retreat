export module Settings;

import <map>;
import <memory>;
import <string>;
import <utility>;

import "config.h";

#include "tstring.h"
#include "debug.h"

const TCHAR *CONFIG_FILE_NAME = _T("retreat.conf");

export const TCHAR *DEFAULT_TIMER_FONT_FACE = _T("Arial");
export const int DEFAULT_TIMER_FONT_SIZE = 32;
export const int DEFAULT_TIMER_X_WINDOWED = 25;
export const int DEFAULT_TIMER_Y_WINDOWED = 25;
export const int DEFAULT_TIMER_X = 160;
export const int DEFAULT_TIMER_Y = 140;

export class Settings {
public:
    static constexpr const TCHAR *ERIOD_ENABLE = _T("periods.enable");
    static constexpr const TCHAR *PERIOD_DURATION = _T("periods.duration");
    static constexpr const TCHAR *PERIOD_FROM_LAUNCH = _T("periods.from_launch_time");

    static constexpr const TCHAR *BREAK_DURATION = _T("breaks.duration");
    static constexpr const TCHAR *DELAY_AMOUNT = _T("delays.amount");
    static constexpr const TCHAR *DELAY_DURATION = _T("delays.duration");
    static constexpr const TCHAR *ALERT_DURATION = _T("alerts.duration");
    static constexpr const TCHAR *SUSPENDED_DURATION = _T("suspended.duration");

    static constexpr const TCHAR *SKIP_DATE = _T("skip.date");
    static constexpr const TCHAR *SKIP_EXPENDED = _T("skip.expended");

    static constexpr const TCHAR *APPEARANCE_FULLSCREEN = _T("appearance.fullscreen");
    static constexpr const TCHAR *APPEARANCE_TRANSPARENT = _T("appearance.transparent");
    static constexpr const TCHAR *APPEARANCE_OPACITY_LEVEL = _T("appearance.opacity_level");
    static constexpr const TCHAR *APPEARANCE_BACKGROUND_COLOR = _T("appearance.background_color");
    static constexpr const TCHAR *APPEARANCE_IMAGE_DIRECTORY = _T("appearance.image_directory");
    static constexpr const TCHAR *APPEARANCE_STRETCH_IMAGES = _T("appearance.stretch_images");
    static constexpr const TCHAR *APPEARANCE_SHOW_TIMER = _T("appearance.show_timer");
    static constexpr const TCHAR *APPEARANCE_TIMER_TEXT_COLOR = _T("appearance.timer_text_color");

    static constexpr const TCHAR *SOUNDS_ENABLE = _T("sounds.enable");
    static constexpr const TCHAR *SOUNDS_AUDIO_DIRECTORY = _T("sounds.audio_directory");

    static constexpr const TCHAR *MONITORING_INPUT = _T("monitoring.input");
    static constexpr const TCHAR *MONITORING_PROCESSES = _T("monitoring.processes");
    static constexpr const TCHAR *MONITORING_INACTIVITY = _T("monitoring.inactivity");

    static constexpr const TCHAR* PROCESSES = _T("processes.");


    static constexpr const bool DEFAULT_PERIOD_ENABLE = true;
    static constexpr const int DEFAULT_PERIOD_DURATION = 60;
    static constexpr const bool DEFAULT_PERIOD_FROM_LAUNCH = false;

    static constexpr const int DEFAULT_BREAK_DURATION = 10;
    static constexpr const int DEFAULT_DELAY_AMOUNT = 2;
    static constexpr const int DEFAULT_DELAY_DURATION = 3;
    static constexpr const int DEFAULT_ALERT_DURATION = 1;
    static constexpr const int DEFAULT_SUSPENDED_DURATION = 120;

    static constexpr const bool DEFAULT_BEHAVIOR_BEEP = false;

    static constexpr const bool DEFAULT_APPEARANCE_FULLSCREEN = true;
    static constexpr const bool DEFAULT_APPEARANCE_TRANSPARENT = false;
    static constexpr const int DEFAULT_APPEARANCE_OPACITY_LEVEL = 128;
    static constexpr const int DEFAULT_APPEARANCE_BACKGROUND_COLOR = 0xFFFFFF;
    static constexpr const bool DEFAULT_APPEARANCE_STRETCH_IMAGES = true;
    static constexpr const bool DEFAULT_APPEARANCE_SHOW_TIMER = true;
    static constexpr const int DEFAULT_APPEARANCE_TIMER_TEXT_COLOR = 0x1B1BEF;

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

    void getSection(tstring name, std::vector<std::pair<tstring, tstring>> &contents);
    void getSectionValues(tstring name, std::vector<tstring> &values);

    void setString(const tstring path, const tstring value);

private:
    tstring file;
    std::map<tstring, tstring> values;
};

export using SettingsPtr = std::shared_ptr<Settings>;

module :private;

import system;

Settings::Settings() {
    file = getConfigFilePath(CONFIG_FILE_NAME);
    load(file);
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

    return result  *DBG_SECONDS;
}

int Settings::getHoursInSec(const tstring path, int defaultValue) {
    int result = getInt(std::move(path), defaultValue);

    return result  *DBG_SECONDS  *60;
}

void Settings::getSection(tstring name, std::vector<std::pair<tstring, tstring>> &contents) {
    contents.clear();

    for (auto &vpair : values)
        if (vpair.first.starts_with(name))
            contents.emplace_back(vpair);
}

void Settings::getSectionValues(tstring name, std::vector<tstring> &contents) {
    contents.clear();

    for (auto &vpair : values)
        if (vpair.first.starts_with(name))
            contents.emplace_back(vpair.second);
}

void Settings::setString(const tstring path, const tstring value) {
    values[path] = value;
}
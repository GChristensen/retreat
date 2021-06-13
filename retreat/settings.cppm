export module Settings;

import <map>;
import <memory>;
import <string>;
import <utility>;

import "config.h";

#include "tstring.h"
#include "debug.h"


export class Settings {
public:
    static constexpr const TCHAR *PERIOD_DURATION = _T("periods.duration");
    static constexpr const TCHAR *PERIOD_FROM_LAUNCH = _T("periods.from_launch_time");

    static constexpr const TCHAR *BREAK_DURATION = _T("breaks.duration");
    static constexpr const TCHAR *DELAY_AMOUNT = _T("delays.amount");
    static constexpr const TCHAR *DELAY_DURATION = _T("delays.duration");
    static constexpr const TCHAR *ALERT_DURATION = _T("alerts.duration");
    static constexpr const TCHAR *SUSPENDED_DURATION = _T("suspended.duration");

    static constexpr const TCHAR *SKIP_DATE = _T("skip.date");
    static constexpr const TCHAR *SKIP_EXPENDED = _T("skip.expended");

    static constexpr const TCHAR *BEHAVIOR_BEEP = _T("behavior.beep");

    static constexpr const int DEFAULT_PERIOD_DURATION = 60;
    static constexpr const bool DEFAULT_PERIOD_FROM_LAUNCH = false;

    static constexpr const int DEFAULT_BREAK_DURATION = 10;
    static constexpr const int DEFAULT_DELAY_AMOUNT = 2;
    static constexpr const int DEFAULT_DELAY_DURATION = 3;
    static constexpr const int DEFAULT_ALERT_DURATION = 1;
    static constexpr const int DEFAULT_SUSPENDED_DURATION = 120;

    static constexpr const bool DEFAULT_BEHAVIOR_BEEP = false;


    Settings();
    Settings(const tstring& file);

    void load(const tstring& file);
    void save(const tstring& file);
    void save();

    tstring getString(const tstring path, const tstring defaultValue);
    bool getBoolean(const tstring path, bool defaultValue);
    int getInt(const tstring path, int defaultValue);
    int getMinutesInSec(const tstring path, int defaultValue);
    int getHoursInSec(const tstring path, int defaultValue);

    void setString(const tstring path, const tstring value);

private:
    tstring file;
    std::map<tstring, tstring> values;
};

export using SettingsPtr = std::shared_ptr<Settings>;

module :private;

Settings::Settings() {
    
}

Settings::Settings(const tstring &file): file(file) {
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
    if (values.find(path) == values.end())
        return defaultValue;

    return values[path];
}

bool Settings::getBoolean(const tstring path, bool defaultValue) {
    if (values.find(path) == values.end())
        return defaultValue;

    return !values[path].compare(_T("true"));
}

int Settings::getInt(const tstring path, int defaultValue) {
    if (values.find(path) == values.end())
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

void Settings::setString(const tstring path, const tstring value) {
    values[path] = value;
}
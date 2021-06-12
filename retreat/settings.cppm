export module Settings;

import <map>;
import <memory>;
import <string>;

import "config.h";

#include "tstring.h"

export class Settings {
public:
    static const TCHAR *PERIOD_DURATION;
    static const TCHAR *PERIOD_FROM_LAUNCH;

    static const TCHAR *BREAK_DURATION;
    static const TCHAR *DELAY_AMOUNT;
    static const TCHAR *DELAY_DURATION;
    static const TCHAR *ALERT_DURATION;
    static const TCHAR* SUSPENDED_DURATION;

    static const TCHAR* SKIP_DATE;
    static const TCHAR* SKIP_EXPENDED;

    static const TCHAR *BEHAVIOR_BEEP;


    Settings();
    Settings(const tstring& file);

    void load(const tstring& file);
    void save(const tstring& file);
    void save();

    tstring getString(const tstring path, const tstring defaultValue);
    bool getBoolean(const tstring path, bool defaultValue);
    int getInt(const tstring path, int defaultValue);

    void setString(const tstring path, const tstring value);

private:
    tstring file;
    std::map<tstring, tstring> values;
};

export using SettingsPtr = std::shared_ptr<Settings>;

module :private;

const TCHAR *Settings::PERIOD_DURATION = _T("periods.duration");
const TCHAR *Settings::PERIOD_FROM_LAUNCH = _T("periods.from_launch_time");

const TCHAR *Settings::BREAK_DURATION = _T("breaks.duration");
const TCHAR *Settings::DELAY_AMOUNT = _T("delays.amount");
const TCHAR *Settings::DELAY_DURATION = _T("delays.duration");
const TCHAR *Settings::ALERT_DURATION = _T("alerts.duration");
const TCHAR *Settings::SUSPENDED_DURATION = _T("suspended.duration");

const TCHAR *Settings::SKIP_DATE = _T("skip.date");
const TCHAR *Settings::SKIP_EXPENDED = _T("skip.expended");

const TCHAR *Settings::BEHAVIOR_BEEP = _T("behavior.beep");

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

void Settings::setString(const tstring path, const tstring value) {
    values[path] = value;
}
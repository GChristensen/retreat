export module settings;

import <string>;
import <unordered_map>;

import "config.h";

#include "tstring.h"

export class Settings {
public:
    static const TCHAR *PERIOD_DURATION;
    static const TCHAR *PERIOD_FROM_LAUNCH;

    static const TCHAR *BREAK_DURATION;
    static const TCHAR *BREAK_DELAYS;
    static const TCHAR *BREAK_DELAY;
    static const TCHAR *BREAK_ALERT;

    static const TCHAR *BEHAVIOR_BEEP;


    Settings();
    Settings(const tstring& file);

    void load(const tstring& file);
    void save(const tstring& file);

    tstring getString(const tstring path, const tstring defaultValue);
    bool getBoolean(const tstring path, bool defaultValue);
    int getInt(const tstring path, int defaultValue);

    void setString(const tstring path, const tstring value);

private:
    std::unordered_map<tstring, tstring> values;
};

module :private;

const TCHAR *Settings::PERIOD_DURATION = _T("periods.duration");
const TCHAR *Settings::PERIOD_FROM_LAUNCH = _T("periods.from_launch_time");

const TCHAR *Settings::BREAK_DURATION = _T("breaks.duration");
const TCHAR *Settings::BREAK_DELAYS = _T("breaks.delays");
const TCHAR *Settings::BREAK_DELAY = _T("breaks.delay");
const TCHAR *Settings::BREAK_ALERT = _T("breaks.alert");

const TCHAR *Settings::BEHAVIOR_BEEP = _T("behavior.beep");

Settings::Settings() {
    
}

Settings::Settings(const tstring &file) {
    load(file);
}

void Settings::load(const tstring& file) {
    readConfig(file, values);
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
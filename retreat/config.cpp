#include <map>
#include <string>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "tstring.h"

using namespace boost::property_tree;

void readConfig(const tstring &file, std::map<tstring, tstring> &values) {
    wptree ptree;
    tifstream iconfig(file);
    read_ini(iconfig, ptree);

    values.clear();

    for (auto &[section, settings] : ptree) {
        for (auto &[setting, value] : settings) {
            values[section + _T(".") + setting] = value.data();
        }
    }
}

void writeConfig(const tstring& file, const std::map<tstring, tstring> &values) {
    wptree ptree;

    for (auto& [path, value] : values)
        ptree.put(path, value);

    tofstream oconfig(file);
    write_ini(oconfig, ptree);
}
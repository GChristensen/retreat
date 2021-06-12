#pragma once

#include <map>
#include <string>

#include "tstring.h"

void readConfig(const tstring& file, std::map<tstring, tstring> &values);
void writeConfig(const tstring& file, const std::map<tstring, tstring> &values);

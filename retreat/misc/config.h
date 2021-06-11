#pragma once

#include <unordered_map>

#include "tstring.h"

void readConfig(const tstring& file, std::unordered_map<tstring, tstring> &values);
void writeConfig(const tstring& file, const std::unordered_map<tstring, tstring> &values);

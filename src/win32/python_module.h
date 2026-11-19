#pragma once

#ifdef PYTHON_MODULE

#include <string>

#include "tstring.h"

#define ICON_SHOW 0

#define ICON_SHOW_AMETHYST 1

#define ICON_HIDE 2

int getPythonShowIcon();

tstring getPythonConfigFilePath(const tstring& configFile);

#endif
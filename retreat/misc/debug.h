#ifdef _DEBUG

import <iostream>;
#include "tstring.h"
#define DBGLOG(x) tcout << x << std::endl;

#define DBG_SECONDS 1

#else

#define DBGLOG(x)

#defin DBG_SECONDS 60

#endif
    
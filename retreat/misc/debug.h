#ifdef _DEBUG

import <chrono>;
import <iostream>;
#include "tstring.h"
#define DBGLOG(x) tcout << x << std::endl;
#define DBGTIME auto ___dbgt1 = std::chrono::high_resolution_clock::now();
#define DBGTIMEEND auto ___dbgt2 = std::chrono::high_resolution_clock::now(); \
            DBGLOG(_T("time: ") << std::chrono::duration_cast<std::chrono::milliseconds>(___dbgt2 - ___dbgt1));

#define DBG_SECONDS 1

#else

#define DBGLOG(x)
#define DBGTIME
#define DBGTIMEEND

#defin DBG_SECONDS 60

#endif
    
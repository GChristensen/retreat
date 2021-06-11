#include <string>
#include <sstream>
#include <iostream>

#include <windows.h>

#include "tstring.h"

class dbg_stream_for_cout : public tstringbuf {
public:
    ~dbg_stream_for_cout() { sync(); }
    int sync()
    {
        ::OutputDebugString(str().c_str());
        str(tstring()); // Clear the string buffer
        return 0;
    }
};

dbg_stream_for_cout g_dbg_stream_for_cout;

void initdbgcout() {
    tcout.rdbuf(&g_dbg_stream_for_cout);
}

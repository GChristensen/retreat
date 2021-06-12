#include <string>
#include <sstream>
#include <iostream>

#include <windows.h>

#include "tstring.h"

class DbgStream : public tstringbuf {
public:
    ~DbgStream() { sync(); }
    int sync()
    {
        ::OutputDebugString(str().c_str());
        str(tstring()); // Clear the string buffer
        return 0;
    }
};

DbgStream dbgStream;

void initdbgcout() {
    tcout.rdbuf(&dbgStream);
}

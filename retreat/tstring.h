#ifndef __TSTRING__
#define __TSTRING__

#include <string>

#ifdef _UNICODE
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif

#endif // __TSTRING__
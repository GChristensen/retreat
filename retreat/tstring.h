#ifndef __TSTRING_H__
#define __TSTRING_H__

#ifdef UNICODE
#   ifndef _T
#       define _T(x) L ## x
#   endif
#   ifndef TCHAR
#       define TCHAR wchar_t
#   endif
#   define tstring std::wstring
#   define to_tstring std::to_wstring
#   define tstring_view std::wstring_view
#   define tstringbuf std::wstringbuf
#   define tistringstream std::wistringstream
#   define tostringstream std::wostringstream
#   define tifstream std::wifstream
#   define tofstream std::wofstream
#   define tcout std::wcout
#   define tregex std::wregex
#   define tregex_token_iterator std::wsregex_token_iterator
#else
#   ifndef _T
#       define _T(x) x
#   endif
#   ifndef TCHAR
#       define TCHAR char
#   endif
#   define tstring std::string
#   define to_tstring std::to_string
#   define tstring_view std::string_view
#   define tstringbuf std::stringbuf
#   define tifstream std::ifstream
#   define tistringstream std::istringstream
#   define tostringstream std::ostringstream
#   define tofstream std::ofstream
#   define tcout std::cout
#   define tregex std::regex
#   define tregex_token_iterator std::sregex_token_iterator
#endif

#endif // __TSTRING_H__

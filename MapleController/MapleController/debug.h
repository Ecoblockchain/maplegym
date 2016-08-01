#pragma once

#define STRSAFE_NO_DEPRECATE
#include <windows.h>
#include <strsafe.h>

#define _SafeStringVFormatW(buff, fmt, args) SUCCEEDED(StringCchVPrintfW(buff, _countof(buff), fmt, args))
#define _SafeStringVFormatA(buff, fmt, args) SUCCEEDED(StringCchVPrintfA(buff, _countof(buff), fmt, args))

VOID LogW(LPCWSTR lpcwszFormat, ...);
VOID LogA(LPCSTR lpcszFormat, ...);

#ifdef _UNICODE
  #define Log LogW
#else
  #define Log LogA
#endif
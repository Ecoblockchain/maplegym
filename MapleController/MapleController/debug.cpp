#include "debug.h"

// keep huge buffers off the stack
__declspec(thread) WCHAR wszLogBuffer[0x100];
__declspec(thread) CHAR szLogBuffer[0x100];

VOID LogW(LPCWSTR lpcwszFormat, ...)
{
  va_list pArguments;

  va_start(pArguments, lpcwszFormat);
  if (_SafeStringVFormatW(wszLogBuffer, lpcwszFormat, pArguments))
    OutputDebugStringW(wszLogBuffer);
  va_end(pArguments);
}

VOID LogA(LPCSTR lpcszFormat, ...)
{
  va_list pArguments;

  va_start(pArguments, lpcszFormat);
  if (_SafeStringVFormatA(szLogBuffer, lpcszFormat, pArguments))
    OutputDebugStringA(szLogBuffer);
  va_end(pArguments);
}
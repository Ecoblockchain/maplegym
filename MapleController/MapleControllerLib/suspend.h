#pragma once

#include <windows.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

// force unicode
#ifdef SE_DEBUG_NAME
  #undef SE_DEBUG_NAME
#endif
#define SE_DEBUG_NAME L"SeDebugPrivilege"

BOOL InitSuspender(VOID);
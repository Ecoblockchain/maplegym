#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <strsafe.h>
#include <shlwapi.h>
#include "main.h"

#pragma comment(lib, "shlwapi")

#define SafeStrcpyW(dest, src) SUCCEEDED(StringCchCopyW(dest, _countof(dest), src))
#define SafeStrcatW(dest, src) SUCCEEDED(StringCchCatW(dest, _countof(dest), src))

extern HANDLE hMapleProcess;
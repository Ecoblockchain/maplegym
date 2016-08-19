#include "suspend.h"

LONG (NTAPI * NtSuspendProcess)(HANDLE ProcessHandle) = NULL;
LONG (NTAPI * NtResumeProcess)(HANDLE ProcessHandle)  = NULL;

BOOL InitNtFunctions(VOID)
{
  HMODULE hModule;

  hModule = GetModuleHandleW(L"ntdll.dll");
  if (hModule == NULL)
    return FALSE;

  NtSuspendProcess = (decltype(NtSuspendProcess))GetProcAddress(hModule, "NtSuspendProcess");
  if (NtSuspendProcess == NULL)
    return FALSE;

  NtResumeProcess = (decltype(NtResumeProcess))GetProcAddress(hModule, "NtResumeProcess");
  if (NtResumeProcess == NULL)
    return FALSE;

  return TRUE;
}

BOOL EnableDebugPrivilege(VOID)
{
  HANDLE           hToken = NULL;
  TOKEN_PRIVILEGES priv;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    return FALSE;

  if (!LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid))
    return FALSE;

  priv.PrivilegeCount           = 1;
  priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  return AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(priv), NULL, NULL);
}

BOOL SuspendMapleStory(BOOL fSuspend)
{
  static BOOL bSuspended = FALSE;

  if (bSuspended == fSuspend)
    return FALSE;

  if (hMapleProcess == NULL)
    if ((fSuspend ? NtSuspendProcess : NtResumeProcess)(hMapleProcess) == STATUS_SUCCESS)
      bSuspended = fSuspend;

  return (bSuspended == fSuspend);
}
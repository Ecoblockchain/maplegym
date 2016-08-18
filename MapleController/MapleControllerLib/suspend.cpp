#include "suspend.h"

LONG (NTAPI * NtSuspendProcess)(HANDLE ProcessHandle) = NULL;
LONG (NTAPI * NtResumeProcess)(HANDLE ProcessHandle) = NULL;

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
  HANDLE            hToken = NULL;
  LUID              luid;
  TOKEN_PRIVILEGES  priv;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    return FALSE;

  if (!LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid))
    return FALSE;

  priv.PrivilegeCount           = 1;
  priv.Privileges[0].Luid       = luid;
  priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  return AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
}

BOOL InitSuspender(VOID)
{
  return (InitNtFunctions() && EnableDebugPrivilege());
}

BOOL SuspendMapleStory(DWORD dwProcessId, BOOL fSuspend)
{
  static BOOL   bSuspended = FALSE;
  /****/ HANDLE hProcess = NULL;

  if (fSuspend == bSuspended)
    return FALSE;

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
  if (hProcess != NULL)
  {
    if ((fSuspend ? NtSuspendProcess : NtResumeProcess)(hProcess) == STATUS_SUCCESS)
      bSuspended = fSuspend;
    CloseHandle(hProcess);
  }

  return (bSuspended == fSuspend);
}
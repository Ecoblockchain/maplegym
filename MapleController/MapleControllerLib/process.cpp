#include "process.h"

HANDLE hMapleProcess = NULL;

HMODULE LoadModuleExW(HANDLE hProcess, LPCWSTR lpcszDll)
{
  DWORD   cdwBytes = sizeof(WCHAR) * (lstrlenW(lpcszDll) + 1), junk;
  LPVOID  lpvBase;
  HANDLE  hThread;
  HMODULE hRET = NULL;

  lpvBase = VirtualAllocEx(hProcess, NULL, cdwBytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (lpvBase != NULL)
  {
    if (WriteProcessMemory(hProcess, lpvBase, lpcszDll, cdwBytes, &junk))
    {
      hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, lpvBase, 0, NULL);
      if (hThread != NULL)
      {
        if (WaitForSingleObject(hThread, INFINITE) == WAIT_OBJECT_0)
          GetExitCodeThread(hThread, (LPDWORD)&hRET);
        CloseHandle(hThread);
      }
    }
    VirtualFreeEx(hProcess, lpvBase, cdwBytes, MEM_DECOMMIT);
  }
  return hRET;
}

// keep large buffers off stack
static WCHAR wszPathBuffer[MAX_PATH];

BOOL LoadLocalModuleW(HANDLE hProcess, LPCWSTR lpcszDll)
{
  if (GetModuleFileNameW(hModule, wszPathBuffer, MAX_PATH) > 0)
    if (PathRemoveFileSpecW(wszPathBuffer))
      if (PathAppendW(wszPathBuffer, lpcszDll))
        if (LoadModuleExW(hProcess, wszPathBuffer) != NULL)
          return TRUE;
  return FALSE;
}

#define LoadController(x) LoadLocalModuleW(x, L"MapleController.dll")
#define LoadWindowMode(x) LoadLocalModuleW(x, L"ForceWindowMode.dll")

BOOL RunSuspendedW(LPCWSTR lpcszPath, LPPROCESS_INFORMATION lpProcessInfo)
{
  STARTUPINFOW si = {0};
  LPWSTR       lpszCopy;
  DWORD        cdwLength = lstrlenW(lpcszPath) + 1;
  BOOL         bRET = FALSE;

  si.cb = sizeof(si);

  lpszCopy = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * cdwLength);
  if (lpszCopy != NULL)
  {
    if (SUCCEEDED(StringCchCopyW(lpszCopy, cdwLength, lpcszPath)))
      if (CreateProcessW(NULL, lpszCopy, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, lpProcessInfo))
        bRET = TRUE;
    HeapFree(GetProcessHeap(), 0, lpszCopy);
  }
  return bRET;
}

BOOL CompareProcessFile(DWORD dwProcessID, LPCWSTR lpcszPath)
{
  BOOL            bRET = FALSE;
  HANDLE          hSnapshot;
  MODULEENTRY32W  ModuleEntry;
  
  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    ModuleEntry.dwSize = sizeof(ModuleEntry);
    if (Module32FirstW(hSnapshot, &ModuleEntry))
      if (lstrcmpiW(lpcszPath, ModuleEntry.szExePath) == 0)
        bRET = TRUE;
    CloseHandle(hSnapshot);
  }
  return bRET;
}

BOOL TerminateProcessById(DWORD dwProcessID)
{
  HANDLE hProcess;
  BOOL   bRET = FALSE;

  hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
  if (hProcess != NULL)
  {
    if (TerminateProcess(hProcess, EXIT_SUCCESS))
      bRET = TRUE;
    CloseHandle(hProcess);
  }
  return bRET;
}

BOOL TerminateProcessByPath(LPCWSTR lpcszPath)
{
  HANDLE          hSnapshot;
  PROCESSENTRY32W ProcessEntry;
  BOOL            bRET = FALSE;

  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE)
  {
    ProcessEntry.dwSize = sizeof(ProcessEntry);
    if (Process32FirstW(hSnapshot, &ProcessEntry))
    {
      bRET = TRUE;
      do {
        if (CompareProcessFile(ProcessEntry.th32ProcessID, lpcszPath))
          if (!TerminateProcessById(ProcessEntry.th32ProcessID))
            bRET = FALSE;
      } while (bRET && Process32NextW(hSnapshot, &ProcessEntry));
    }
    CloseHandle(hSnapshot);
  }
  return bRET;
}

BOOL RunMapleStoryW(LPCWSTR lpcszPath)
{
  PROCESS_INFORMATION ProcessInfo;

  if (!TerminateProcessByPath(lpcszPath))
    return FALSE;

  hMapleProcess = NULL;

  if (RunSuspendedW(lpcszPath, &ProcessInfo))
  {
    if (LoadController(ProcessInfo.hProcess))
      if (LoadWindowMode(ProcessInfo.hProcess))
        if (ResumeThread(ProcessInfo.hThread) != (DWORD)-1)
          hMapleProcess = ProcessInfo.hProcess;
    CloseHandle(ProcessInfo.hThread);
  }

  if (hMapleProcess == NULL)
  {
    TerminateProcess(ProcessInfo.hProcess, EXIT_FAILURE);
    CloseHandle(ProcessInfo.hProcess);
  }

  return (hMapleProcess != NULL);
}

BOOL TerminateMapleStory(VOID)
{
  BOOL bRET = TerminateProcess(hMapleProcess, EXIT_SUCCESS);

  CloseHandle(hMapleProcess);
  hMapleProcess = NULL;
  return bRET;
}


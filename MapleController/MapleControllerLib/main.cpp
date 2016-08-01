#include "main.h"

HMODULE hModule;

#define FindMaple() FindWindowW(L"MapleStoryClass", L"MapleStory")

HWND FindMapleWindow(VOID)
{
  HWND hWnd;

  for (hWnd = FindMaple(); hWnd == NULL; hWnd = FindMaple())
    Sleep(500);
  return hWnd;
}

HWND GetMapleWindow(VOID)
{
  static HWND hWnd = NULL;

  if (hWnd == NULL)
    hWnd = FindMapleWindow();
  return hWnd;
}

BOOL APIENTRY DllMain(HINSTANCE _hModule, DWORD fdwReason, LPVOID)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
  {
    hModule = _hModule;
    DisableThreadLibraryCalls(hModule);
  }
  return TRUE;
}
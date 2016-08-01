#include <windows.h>
#include "debug.h"
#include "hookman.hpp"
#include "hooks.hpp"

HookMan* pHooks = new HookMan();

typedef struct _MAPLEOBJ {
  PVOID*    ppPointer;
  ULONG_PTR ulAddress;
} MAPLEOBJ, *LPMAPLEOBJ;

static const MAPLEOBJ c_mapleobjs[] = {
  {(PVOID*)&_CInputSystem_IsKeyPressed, 0x004FE23C},
  {(PVOID*)&_CClientSocket_SendPacket,  0x0046AE92},
  {(PVOID*)&_CLogo_UpdateLogo,          0x0055F197},
  {(PVOID*)&_CLogo_ForcedEnd,           0x0055EDA3},
  {(PVOID*)&_CUIStatusBar_CUIStatusBar, 0x006AA070},
  {(PVOID*)&_CUIStatusBar_Draw,         0x006B1144},
  {(PVOID*)&_CUIWnd_CUIWnd,             0x006E8988},
  {(PVOID*)&_CWnd_CreateWnd,            0x0075C153},
  /**/
  {(PVOID*)&ppCClientSocket,            0x00892BF8},
};

static const HOOKINFO c_hooks[] =
{
  // winapi hooks
  {(PVOID*)&_CreateWindowExA,           CreateWindowExAHook},
  {(PVOID*)&_GetFocus,                  GetFocusHook},
  {(PVOID*)&_DialogBoxParamA,           DialogBoxParamAHook},
  {(PVOID*)&_ShowWindow,                ShowWindowHook},
  {(PVOID*)&_SetWindowPos,              SetWindowPosHook},

  // maplestory hooks
  {(PVOID*)&_CInputSystem_IsKeyPressed, CInputSystem_IsKeyPressed},
  {(PVOID*)&_CLogo_UpdateLogo,          CLogo_UpdateLogo},
  {(PVOID*)&_CUIStatusBar_CUIStatusBar, CUIStatusBar_CUIStatusBar},
  {(PVOID*)&_CUIStatusBar_Draw,         CUIStatusBar_Draw},
  {(PVOID*)&_CUIWnd_CUIWnd,             CUIWnd_CUIWnd},
  {(PVOID*)&_CWnd_CreateWnd,            CWnd_CreateWnd},
};

VOID InitMapleObjects(VOID)
{
  for (MAPLEOBJ mapleobj : c_mapleobjs)
    *(PULONG_PTR)mapleobj.ppPointer = mapleobj.ulAddress;
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    InitMapleObjects();
    if (pHooks->AddHooks(c_hooks, _countof(c_hooks)))
      if (pHooks->Toggle(TRUE))
        if (DisableThreadLibraryCalls(hModule))
          break;
    return FALSE;

  case DLL_PROCESS_DETACH:
    pHooks->Toggle(FALSE);
    break;
  }
  return TRUE;
}
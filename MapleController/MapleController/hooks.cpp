#include "hooks.hpp"
#include "hookman.hpp"
#include "debug.h"

INT_PTR (WINAPI * _DialogBoxParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam) = DialogBoxParamA;
HWND (WINAPI * _CreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = CreateWindowExA;
HWND (WINAPI * _GetFocus)(VOID) = GetFocus;
BOOL (WINAPI * _ShowWindow)(HWND hWnd, INT nCmdShow) = ShowWindow;
BOOL (WINAPI * _SetWindowPos)(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags) = SetWindowPos;

VOID (MAPLECALL * _CLogo_UpdateLogo)(CLogo* pCLogo, DWORD dwEDX) = NULL;
VOID (MAPLECALL * _CLogo_ForcedEnd)(CLogo* pCLogo, DWORD dwEDX) = NULL;
VOID (MAPLECALL * _CClientSocket_SendPacket)(LPVOID pCClientSocket, DWORD dwEDX, COutPacket* pPacket) = NULL;
BOOL (MAPLECALL * _CInputSystem_IsKeyPressed)(LPVOID pThis, DWORD dwEDX, INT nVK) = NULL;

VOID (MAPLECALL * _CUIWnd_CUIWnd)(LPVOID pCUIWnd, DWORD dwEDX, INT nUIType, INT nCloseType, INT nCloseX, INT nCloseY, BOOL bBackground, INT nBackgroundX, INT nBackgroundY) = NULL;
VOID (MAPLECALL * _CWnd_CreateWnd)(LPVOID pCWnd, DWORD dwEDX, INT l, INT t, INT w, INT h, INT z, BOOL bScreenCoord, PVOID pData, BOOL bSetFocus) = NULL;
VOID (MAPLECALL * _CUIStatusBar_CUIStatusBar)(LPVOID pCUIStatusBar, DWORD dwEDX) = NULL;
VOID (MAPLECALL * _CUIStatusBar_Draw)(LPVOID pCUIStatusBar, DWORD dwEDX, LPRECT lpRect) = NULL;

PVOID*  ppCClientSocket = NULL;
PVOID   pCInput;
PDWORD  pdwEXPTable;

static const BYTE bMovementKeys[] = {
  VK_NULL,    // stay put
  VK_UP,      // move up
  VK_RIGHT,   // move right
  VK_DOWN,    // move down
  VK_LEFT,    // move left
};

static const BYTE bCommandKeys[] = {
  VK_NULL,    // do nothing
  VK_CONTROL, // attack
  'Z',        // loot
  VK_MENU,    // jump
};

#define IsCommandKey(x) contains(bCommandKeys, x)

static INT      nKeyOverride = 0;
static HWND     hWndMaple = NULL;
static BOOL     bLastCommand = VK_NULL;
extern HookMan* pHooks;

BOOL CompareClassNameA(LPCSTR lpcszClass, LPCSTR lpcszString)
{
  if (lpcszClass != NULL)
    if (HIWORD(lpcszClass) != 0)
      if (lstrcmpiA(lpcszClass, lpcszString) == 0)
        return TRUE;
  return FALSE;
}

// hook maplestory's IsKeyPressed check to fake keypresses
BOOL MAPLECALL CInputSystem_IsKeyPressed(LPVOID pThis, DWORD dwEDX, INT nVK)
{
  pCInput = pThis;

  if (!contains(bMovementKeys, (BYTE)nVK) && !contains(bCommandKeys, (BYTE)nVK))
    return FALSE;

  return (nVK == nKeyOverride) || _CInputSystem_IsKeyPressed(pThis, dwEDX, nVK);
}

UINT GetCurrentMovement(VOID)
{
  for (UINT i = 1; i < _countof(bMovementKeys); i++)
    if (_CInputSystem_IsKeyPressed(pCInput, 0, bMovementKeys[i]))
      return i;
  return 0;
}

UINT GetCommandIndex(WORD wKey)
{
  for (UINT i = 1; i < _countof(bCommandKeys); i++)
    if (bCommandKeys[i] == wKey)
      return i;
  return 0;
}

UINT GetLastCommand(VOID)
{
  if (_CInputSystem_IsKeyPressed(pCInput, 0, VK_MENU))
    return GetCommandIndex(VK_MENU);

  for (UINT i = 1; i < _countof(bCommandKeys); i++)
    if (bCommandKeys[i] == bLastCommand)
      return i;

  return 0;
}

DWORD EncodeLastAction(VOID)
{
  int m = GetCurrentMovement();
  int c = GetLastCommand();

  bLastCommand = VK_NULL;
  return (m * _countof(bCommandKeys)) + c;
}

LRESULT CALLBACK MapleWindowHook(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
  switch (uMessage)
  {
  case WM_CREATE:
  case WM_ACTIVATE:
  case WM_NCACTIVATE:
		_SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    break;

  case WM_CONTROLPLAYER:
    if (wParam < _countof(bMovementKeys) && lParam < _countof(bCommandKeys))
    {
      nKeyOverride = bMovementKeys[wParam];
      uMessage     = WM_KEYDOWN;
      wParam       = bCommandKeys[lParam];
      lParam       = MapVirtualKeyW(bCommandKeys[lParam], MAPVK_VK_TO_VSC) << 16;
      break;
    }
    return 0;

  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:
    return 0;

  case WM_GETLASTACTION:
    return (LRESULT)EncodeLastAction();

  case WM_KEYDOWN:
    if (IsCommandKey(wParam) || wParam == VK_ESCAPE)
    {
      if (wParam != VK_ESCAPE)
        bLastCommand = (BYTE)wParam;
      break;
    }
    return 0;
  }
  return DefSubclassProc(hWnd, uMessage, wParam, lParam);
}

// instead of showing errors, just exit
INT_PTR WINAPI DialogBoxParamAHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
  UNREFERENCED_PARAMETER(hInstance);
  UNREFERENCED_PARAMETER(lpTemplateName);
  UNREFERENCED_PARAMETER(hWndParent);
  UNREFERENCED_PARAMETER(lpDialogFunc);
  UNREFERENCED_PARAMETER(dwInitParam);

  TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
  return EXIT_FAILURE;
}

// destroy the ad at the end & hook the main window
HWND WINAPI CreateWindowExAHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
  if (CompareClassNameA(lpClassName, "NexonADBallon"))
    return NULL;
  
  BOOL bIsMain = CompareClassNameA(lpClassName, "MapleStoryClass");
  if (bIsMain)
    dwStyle &= ~WS_VISIBLE;

  HWND hWnd = _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

  if (bIsMain)
  {
    hWndMaple = hWnd;
    if (!SetWindowSubclass(hWnd, MapleWindowHook, ID_SUBCLASS, 0))
      return DestroyWindow(hWnd), NULL;
  }

  return hWnd;
}

// tell maplestory its own window is always focused
HWND WINAPI GetFocusHook(VOID)
{
  if (nKeyOverride != 0)
    if (hWndMaple != NULL)
      return hWndMaple;
  return _GetFocus();
}

// don't let maplestory call ShowWindow on itself
BOOL WINAPI ShowWindowHook(HWND hWnd, INT nCmdShow)
{
  if (hWnd == hWndMaple)
    return FALSE;
  return _ShowWindow(hWnd, nCmdShow);
}

// don't let maplestory call SetWindowPos on itself
BOOL WINAPI SetWindowPosHook(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags)
{
  if (hWnd == hWndMaple)
    return FALSE;
  return _SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

// COutPacket(0x001b).EncodeString(username)
// the server doesn't check the rest of the packet (like the password)
static const BYTE c_lpbLoginPacket[] = {0x1b, 0x00, 0x01, 0x00, 'x'}; 

BOOL LoginUser(VOID)
{
  COutPacket* pPacket;
  BOOL        bRET = FALSE;

  pPacket = (COutPacket*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COutPacket));
  if (pPacket != NULL)
  {
    pPacket->m_aSendBuff = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(BYTE) * _countof(c_lpbLoginPacket));
    if (pPacket->m_aSendBuff != NULL)
    {
      CopyMemory(pPacket->m_aSendBuff, c_lpbLoginPacket, _countof(c_lpbLoginPacket));

      pPacket->m_uOffset = _countof(c_lpbLoginPacket);
      _CClientSocket_SendPacket(*ppCClientSocket, 0, pPacket);
      bRET = TRUE;

      HeapFree(GetProcessHeap(), 0, pPacket->m_aSendBuff);
    }
    HeapFree(GetProcessHeap(), 0, pPacket);
  }
  return bRET;
}

// skip the logo & immediately login automatically after
VOID MAPLECALL CLogo_UpdateLogo(CLogo* pCLogo, DWORD dwEDX)
{
  UNREFERENCED_PARAMETER(dwEDX);

  pCLogo->m_bVideoMode = 1;
  _CLogo_ForcedEnd(pCLogo, 0);

  LoginUser();
}

static std::set<LPVOID> distractions;

// add `CUIWnd`s to distractions
VOID MAPLECALL CUIWnd_CUIWnd(LPVOID pCUIWnd, DWORD dwEDX, INT nUIType, INT nCloseType, INT nCloseX, INT nCloseY, BOOL bBackground, INT nBackgroundX, INT nBackgroundY)
{
  distractions.insert(pCUIWnd);

  _CUIWnd_CUIWnd(pCUIWnd, dwEDX, nUIType, nCloseType, nCloseX, nCloseY, bBackground, nBackgroundX, nBackgroundY);
}

// add CUIStatusBar to distractions
VOID MAPLECALL CUIStatusBar_CUIStatusBar(LPVOID pCUIStatusBar, DWORD dwEDX)
{
  distractions.insert(pCUIStatusBar);

  _CUIStatusBar_CUIStatusBar(pCUIStatusBar, dwEDX);
}

// move all distractions offscreen when they're created
VOID MAPLECALL CWnd_CreateWnd(LPVOID pCWnd, DWORD dwEDX, INT l, INT t, INT w, INT h, INT z, BOOL bScreenCoord, PVOID pData, BOOL bSetFocus)
{
  if (contains(distractions, pCWnd))
  {
    l = 9999;
    t = 9999;
    // todo: remove pCWnd from distractions?
    // idk if CreateWnd is called more than once for each CWnd
  }
  _CWnd_CreateWnd(pCWnd, dwEDX, l, t, w, h, z, bScreenCoord, pData, bSetFocus);
}

// don't redraw the status bar
VOID MAPLECALL CUIStatusBar_Draw(LPVOID pCUIStatusBar, DWORD dwEDX, LPRECT lpRect)
{
  UNREFERENCED_PARAMETER(pCUIStatusBar);
  UNREFERENCED_PARAMETER(dwEDX);
  UNREFERENCED_PARAMETER(lpRect);

  // pass
}
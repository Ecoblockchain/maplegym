#pragma once

#include <windows.h>
#include <commctrl.h>
#include <set>
#include <algorithm>
#pragma comment(lib, "comctl32")

#define WM_CONTROLPLAYER  (WM_USER + 0xBEEF)
#define WM_GETLASTACTION  (WM_USER + 0xBEEF + 1)

#define ID_SUBCLASS       0xDEADBEEF
#define IDT_RECORD        0xBEEF
#define VK_NULL           0

#define contains(xs, x) (std::find(std::begin(xs), std::end(xs), x) != std::end(xs))

#define MAPLECALL __fastcall

// ==========
// structures
// ==========

#pragma pack(push, 1)

struct CLogo
{
  BYTE _useless[44];
  BOOL m_bVideoMode;
};

struct COutPacket {
  BOOL    m_bLoopback;
  LPBYTE  m_aSendBuff;
  DWORD   m_uOffset;
  BOOL    m_bIsEncryptedByShanda;
};

#pragma pack(pop)

// ========
// poINTers
// ========

extern HWND (WINAPI * _CreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
extern INT_PTR (WINAPI * _DialogBoxParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
extern HWND (WINAPI * _GetFocus)(VOID);
extern BOOL (WINAPI * _ShowWindow)(HWND hWnd, INT nCmdShow);
extern BOOL (WINAPI * _SetWindowPos)(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags);

extern BOOL (MAPLECALL * _CInputSystem_IsKeyPressed)(LPVOID pCInputSystem, DWORD dwEDX, INT nVK);
extern VOID (MAPLECALL * _CLogo_UpdateLogo)(CLogo* pCLogo, DWORD dwEDX);
extern VOID (MAPLECALL * _CLogo_ForcedEnd)(CLogo* pCLogo, DWORD dwEDX);
extern VOID (MAPLECALL * _CClientSocket_SendPacket)(LPVOID pCClientSocket, DWORD dwEDX, COutPacket* pPacket);
extern VOID (MAPLECALL * _CUIWnd_CUIWnd)(LPVOID pCUIWnd, DWORD dwEDX, INT nUIType, INT nCloseType, INT nCloseX, INT nCloseY, BOOL bBackground, INT nBackgroundX, INT nBackgroundY);
extern VOID (MAPLECALL * _CWnd_CreateWnd)(LPVOID pCWnd, DWORD dwEDX, INT l, INT t, INT w, INT h, INT z, BOOL bScreenCoord, PVOID pData, BOOL bSetFocus);
extern VOID (MAPLECALL * _CUIStatusBar_CUIStatusBar)(LPVOID pCUIStatusBar, DWORD dwEDX);
extern VOID (MAPLECALL * _CUIStatusBar_Draw)(LPVOID pCUIStatusBar, DWORD dwEDX, LPRECT lpRect);

extern PVOID* ppCClientSocket;
extern PDWORD pdwEXPTable;

// =====
// hooks
// =====

HWND WINAPI CreateWindowExAHook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, INT x, INT y, INT nWidth, INT nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
INT_PTR WINAPI DialogBoxParamAHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND WINAPI GetFocusHook(VOID);
BOOL WINAPI ShowWindowHook(HWND hWnd, INT nCmdShow);
BOOL WINAPI SetWindowPosHook(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags);

BOOL MAPLECALL CInputSystem_IsKeyPressed(LPVOID pCInputSystem, DWORD dwEDX, INT nVK);
VOID MAPLECALL CLogo_UpdateLogo(CLogo* pCLogo, DWORD dwEDX);
VOID MAPLECALL CUIWnd_CUIWnd(LPVOID pCUIWnd, DWORD dwEDX, INT nUIType, INT nCloseType, INT nCloseX, INT nCloseY, BOOL bBackground, INT nBackgroundX, INT nBackgroundY);
VOID MAPLECALL CWnd_CreateWnd(LPVOID pCWnd, DWORD dwEDX, INT l, INT t, INT w, INT h, INT z, BOOL bScreenCoord, PVOID pData, BOOL bSetFocus);
VOID MAPLECALL CUIStatusBar_CUIStatusBar(LPVOID pCUIStatusBar, DWORD dwEDX);
VOID MAPLECALL CUIStatusBar_Draw(LPVOID pCUIStatusBar, DWORD dwEDX, LPRECT lpRect);
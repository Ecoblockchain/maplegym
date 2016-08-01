#include <windows.h>
#include "main.h"

#define VK_NULL 0

#define WM_CONTROLPLAYER  (WM_USER + 0xBEEF)
#define WM_GETLASTACTION  (WM_USER + 0xBEEF + 1)

#define MOVEMENT_COUNT 5  // {nop, up, right, down, left}
#define COMMAND_COUNT  4  // {nop, attack, loot, jump}

BOOL DecodeAction(WORD wAction, LPWORD lpwMovement, LPWORD lpwCommand)
{
  lpwMovement[0] = wAction / COMMAND_COUNT;
  if (lpwMovement[0] >= MOVEMENT_COUNT)
    return FALSE;

  lpwCommand[0] = wAction % COMMAND_COUNT;
  return TRUE;
}

BOOL DoAction(WORD wAction)
{
  WORD wMovement, wCommand;
  HWND hMaple = GetMapleWindow();

  if (DecodeAction(wAction, &wMovement, &wCommand))
    if (PostMessageW(hMaple, WM_CONTROLPLAYER, wMovement, wCommand))
      return TRUE;
  return FALSE;
}

DWORD GetLastAction(VOID)
{
  return (DWORD)SendMessage(GetMapleWindow(), WM_GETLASTACTION, 0, 0);
}

VOID ShowMapleWindow(VOID)
{
  HWND hWnd = GetMapleWindow();

  ShowWindow(hWnd, SW_SHOW);

  // idk which is the right one
  SetActiveWindow(hWnd);
  SetForegroundWindow(hWnd);
  SetFocus(hWnd);
}
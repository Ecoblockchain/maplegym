#pragma once

#include <windows.h>
#include <map>
#include "detours.h"
#pragma  comment(lib, "detours")

typedef struct _HOOKINFO {
  PVOID* ppTarget;
  PVOID  pDetour;
} HOOKINFO, *LPHOOKINFO;

typedef const HOOKINFO* LPCHOOKINFO;
typedef std::map<PVOID*, PVOID> HookMap;
typedef std::pair<PVOID*, PVOID> HookEntry;
typedef LONG (WINAPI * DETOUR_FUNCTION)(PVOID* ppTarget, PVOID pDetour);

class HookMan
{
private:
  HookMap m_hooks;
  BOOL    m_fInstalled;

public:
  HookMan(VOID);
  BOOL AddHooks(LPCHOOKINFO lpcHooks, DWORD cdwCount);
  VOID AddToHookMap(HookMap* pHookMap, LPCHOOKINFO lpcHooks, DWORD cdwCount);
  LONG RunDetours(BOOL fSet, HookMap hooks);
  BOOL ToggleHooks(BOOL fSet, HookMap hooks);
  BOOL Toggle(BOOL fSet);
};
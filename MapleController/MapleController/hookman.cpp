#include "hookman.hpp"

HookMan::HookMan(VOID)
{
  m_fInstalled = FALSE;
}

VOID HookMan::AddToHookMap(HookMap* pHookMap, LPCHOOKINFO lpcHooks, DWORD cdwCount)
{
  for (DWORD i = 0; i < cdwCount; i++)
    pHookMap->insert(HookEntry(lpcHooks[i].ppTarget, lpcHooks[i].pDetour));
}

BOOL HookMan::AddHooks(LPCHOOKINFO lpcHooks, DWORD cdwCount)
{
  AddToHookMap(&m_hooks, lpcHooks, cdwCount);

  if (m_fInstalled)
  {
    HookMap hmap;

    AddToHookMap(&hmap, lpcHooks, cdwCount);
    return ToggleHooks(TRUE, hmap);
  }

  return TRUE;
}

LONG HookMan::RunDetours(BOOL fSet, HookMap hooks)
{
  DETOUR_FUNCTION DetourFunction = (fSet ? DetourAttach : DetourDetach);
  LONG            lResult;

  for (HookEntry hook : hooks)
  {
    lResult = DetourFunction(hook.first, hook.second);
    if (lResult != NO_ERROR)
      return lResult;
  }
  return NO_ERROR;
}

BOOL HookMan::ToggleHooks(BOOL fSet, HookMap hooks)
{
  BOOL bRET = FALSE;

  if (DetourTransactionBegin() == NO_ERROR)
  {
    if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
      if (RunDetours(fSet, hooks) == NO_ERROR)
        if (DetourTransactionCommit() == NO_ERROR)
          bRET = TRUE;

    DetourTransactionAbort();
  }

  return bRET;
}

BOOL HookMan::Toggle(BOOL fSet)
{
  if (m_fInstalled == fSet)
    return FALSE;

  if (ToggleHooks(fSet, m_hooks))
  {
    m_fInstalled = fSet;
    return TRUE;
  }
  return FALSE;
}
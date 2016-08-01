#include "hookman.hpp"

HookMan::HookMan(VOID)
{
  m_fInstalled = FALSE;
}

BOOL HookMan::AddHooks(LPCHOOKINFO lpcHooks, DWORD cdwCount)
{
  for (DWORD i = 0; i < cdwCount; i++)
    m_hooks.insert(HookEntry(lpcHooks[i].ppTarget, lpcHooks[i].pDetour));

  if (m_fInstalled)
  {
    HookMap hookmap;

    for (DWORD i = 0; i < cdwCount; i++)
      hookmap.insert(HookEntry(lpcHooks[i].ppTarget, lpcHooks[i].pDetour));
    return ToggleHooks(TRUE, hookmap);
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
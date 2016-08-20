// force window mode

#include <windows.h>
#include "detours.h"
#include "d3d8.h"
#pragma  comment(lib, "detours")
#pragma  comment(lib, "d3d8.lib")

BOOL SetHook(BOOL fSet, PVOID* ppTarget, PVOID pDetour)
{
  BOOL bRET = FALSE;

  if (DetourTransactionBegin() == NO_ERROR)
  {
    if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
      if ((fSet ? DetourAttach : DetourDetach)(ppTarget, pDetour) == NO_ERROR)
        if (DetourTransactionCommit() == NO_ERROR)
          bRET = TRUE;
    DetourTransactionAbort();
  }
  return bRET;
}

#define AttachHook(x, y) SetHook(TRUE, x, y)
#define DetachHook(x, y) SetHook(FALSE, x, y)

#define VTABLE_CREATEDEVICE 15

PVOID GetCreateDevicePointer(VOID)
{
  IDirect3D8* pObject;
  PVOID       pvRET = NULL;

  pObject = Direct3DCreate8(D3D_SDK_VERSION);
  if (pObject != NULL)
  {
    pvRET = (*(PVOID**)pObject)[VTABLE_CREATEDEVICE];
    pObject->Release();
  }

  return pvRET;
}

HRESULT (WINAPI * _CreateDevice)(IDirect3D8* This, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) = NULL;

#define InitCreateDevicePointer() ((_CreateDevice = (decltype(_CreateDevice))GetCreateDevicePointer()) != NULL)

HRESULT WINAPI CreateDevice(IDirect3D8* This, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pParams, IDirect3DDevice8** ppReturnedDeviceInterface)
{
  D3DDISPLAYMODE mode;

  This->GetAdapterDisplayMode(0, &mode);
  pParams->BackBufferFormat = mode.Format;
  pParams->hDeviceWindow = NULL;
  pParams->Windowed = TRUE;
  pParams->FullScreen_RefreshRateInHz = 0;
  pParams->FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

  return _CreateDevice(This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pParams, ppReturnedDeviceInterface);
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    if (InitCreateDevicePointer())
      if (AttachHook((PVOID*)&_CreateDevice, CreateDevice))
        if (DisableThreadLibraryCalls(hModule))
          break;
    return FALSE;
  case DLL_PROCESS_DETACH:
    DetachHook((PVOID*)&_CreateDevice, CreateDevice);
    break;
  }
  return TRUE;
}
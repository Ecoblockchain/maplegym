#include <windows.h>
#include "main.h"
#include "d3d8.h"
#pragma  comment(lib, "d3d8")

LPDIRECT3DDEVICE8 lpDirectDevice = NULL;
D3DDISPLAYMODE    d3dDisplayMode;

BOOL InitDirectX(VOID)
{
  LPDIRECT3D8           lpDirect3D = NULL;
  D3DPRESENT_PARAMETERS d3dpp;
  HWND                  hWnd = GetMapleWindow();

  lpDirect3D = Direct3DCreate8(D3D_SDK_VERSION);
  if (lpDirect3D == NULL)
    return FALSE;

  if (FAILED(lpDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dDisplayMode)))
    return FALSE;

  ZeroMemory(&d3dpp, sizeof(d3dpp));

  d3dpp.Windowed                        = TRUE;
  d3dpp.Flags                           = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  d3dpp.BackBufferFormat                = d3dDisplayMode.Format;
  d3dpp.BackBufferHeight                = d3dDisplayMode.Height;
  d3dpp.BackBufferWidth                 = d3dDisplayMode.Width;
  d3dpp.MultiSampleType                 = D3DMULTISAMPLE_NONE;
  d3dpp.SwapEffect                      = D3DSWAPEFFECT_COPY;
  d3dpp.hDeviceWindow                   = hWnd;
  d3dpp.FullScreen_RefreshRateInHz      = 0;
  d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

  return SUCCEEDED(lpDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpDirectDevice));
}

#pragma pack(push, 1)

typedef struct _D3ARGB {
  BYTE b;
  BYTE g;
  BYTE r;
  BYTE a;
} D3ARGB, *LPD3ARGB;

#pragma pack(pop)

inline VOID CopyPixels(D3DLOCKED_RECT* pRect, LPBYTE lpbDest, UINT nWidth, UINT nHeight)
{
  UINT     x, y, i = 0;
  LPD3ARGB lpRow;

  for (y = 0; y < nHeight; y++)
  {
    lpRow = (LPD3ARGB)((LPBYTE)pRect->pBits + (y * pRect->Pitch));
    for (x = 0; x < nWidth; x++)
    {
      lpbDest[i++] = lpRow[x].r;
      lpbDest[i++] = lpRow[x].g;
      lpbDest[i++] = lpRow[x].b;
    }
  }
}

BOOL GetMapleScreen(LPBYTE lpbOutput)
{
  IDirect3DSurface8* pSurface;
  D3DLOCKED_RECT     rect;

  if (lpDirectDevice == NULL)
    if (!InitDirectX())
      return FALSE;

  if (FAILED(lpDirectDevice->CreateImageSurface(d3dDisplayMode.Width, d3dDisplayMode.Height, D3DFMT_A8R8G8B8, &pSurface)))
    return FALSE;

  if (FAILED(lpDirectDevice->GetFrontBuffer(pSurface)))
    return FALSE;

  if (SUCCEEDED(pSurface->LockRect(&rect, NULL, D3DLOCK_READONLY)))
  {
    CopyPixels(&rect, lpbOutput, 800, 600);
    pSurface->UnlockRect();
    return TRUE;
  }
  return FALSE;
}
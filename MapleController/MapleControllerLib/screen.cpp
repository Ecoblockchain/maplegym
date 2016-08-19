#include "screen.hpp"

static Screenshotter* pScreenshotter = new Screenshotter();

Screenshotter::Screenshotter(VOID)
{
  m_lpDevice  = NULL;
  m_dwWidth   = 0;
  m_dwHeight  = 0;
}

BOOL Screenshotter::CreateDevice(LPDIRECT3D8 lpDirect3D, HWND hWnd, const D3DDISPLAYMODE* pcMode)
{
  D3DPRESENT_PARAMETERS params;

  ZeroMemory(&params, sizeof(D3DPRESENT_PARAMETERS));

  params.hDeviceWindow                   = hWnd;
  params.Windowed                        = TRUE;

  params.Flags                           = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  params.MultiSampleType                 = D3DMULTISAMPLE_NONE;
  params.SwapEffect                      = D3DSWAPEFFECT_COPY;

  params.BackBufferFormat                = pcMode->Format;
  params.BackBufferHeight                = pcMode->Height;
  params.BackBufferWidth                 = pcMode->Width;

  params.FullScreen_RefreshRateInHz      = 0;
  params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

  return SUCCEEDED(lpDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &m_lpDevice));
}

BOOL Screenshotter::InitDirectX(HWND hWnd)
{
  LPDIRECT3D8    lpDirect3D = NULL;
  D3DDISPLAYMODE d3dMode;

  lpDirect3D = Direct3DCreate8(D3D_SDK_VERSION);
  if (lpDirect3D != NULL)
  {
    if (SUCCEEDED(lpDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dMode)))
    {
      m_dwWidth = d3dMode.Width;
      m_dwHeight = d3dMode.Height;
      return CreateDevice(lpDirect3D, hWnd, &d3dMode);
    }
  }
  return FALSE;
}

VOID Screenshotter::CopyPixels(D3DLOCKED_RECT* pRect, LPBYTE lpbDest, UINT nWidth, UINT nHeight)
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

BOOL Screenshotter::Capture(HWND hWnd, LPBYTE lpbOutput)
{
  IDirect3DSurface8* pSurface;
  D3DLOCKED_RECT     d3dRect;
  BOOL               bRET = FALSE;

  OutputDebugStringW(L"ss1");
  if (m_lpDevice == NULL && !InitDirectX(hWnd))
    return FALSE;
  OutputDebugStringW(L"ss2");

  if (SUCCEEDED(m_lpDevice->CreateImageSurface(m_dwWidth, m_dwHeight, D3DFMT_A8R8G8B8, &pSurface)))
  {
    OutputDebugStringW(L"ss2");
    if (SUCCEEDED(m_lpDevice->GetFrontBuffer(pSurface)))
    {
      OutputDebugStringW(L"ss3");
      if (SUCCEEDED(pSurface->LockRect(&d3dRect, NULL, D3DLOCK_READONLY)))
      {
        OutputDebugStringW(L"ss4");
        CopyPixels(&d3dRect, lpbOutput, 800, 600);
        bRET = TRUE;
        pSurface->UnlockRect();
      }
    }
    pSurface->Release();
  }
  return bRET;
}

BOOL GetMapleScreen(LPBYTE lpbOutput)
{
  return pScreenshotter->Capture(GetMapleWindow(), lpbOutput);
}
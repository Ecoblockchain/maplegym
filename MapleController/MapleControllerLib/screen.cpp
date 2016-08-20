#include "screen.hpp"

static Camera* pCamera = new Camera();

Camera::Camera(VOID)
{
  m_lpDevice  = NULL;
  m_dwWidth   = 0;
  m_dwHeight  = 0;
}

HRESULT Camera::CreateDevice(LPDIRECT3D8 lpDirect3D, HWND hWnd, const D3DDISPLAYMODE* pcMode)
{
  D3DPRESENT_PARAMETERS params = {0};

  params.hDeviceWindow    = hWnd;
  params.Windowed         = TRUE;
  params.SwapEffect       = D3DSWAPEFFECT_COPY;
  params.BackBufferCount  = 1;
  params.BackBufferFormat = pcMode->Format;
  params.BackBufferHeight = pcMode->Height;
  params.BackBufferWidth  = pcMode->Width;

  return lpDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &m_lpDevice);
}

BOOL Camera::InitDirectX(HWND hWnd)
{
  LPDIRECT3D8    lpDirect3D = NULL;
  D3DDISPLAYMODE d3dMode;

  lpDirect3D = Direct3DCreate8(D3D_SDK_VERSION);
  if (lpDirect3D != NULL)
    if (SUCCEEDED(lpDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dMode)))
      if (SUCCEEDED(CreateDevice(lpDirect3D, hWnd, &d3dMode)))
      {
        m_dwWidth = d3dMode.Width;
        m_dwHeight = d3dMode.Height;
        return TRUE;
      }

  return FALSE;
}

VOID Camera::CopyPixels(D3DLOCKED_RECT* pRect, LPBYTE lpbDest, UINT nWidth, UINT nHeight)
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

BOOL Camera::Capture(HWND hWnd, LPBYTE lpbOutput)
{
  IDirect3DSurface8* pSurface = NULL;
  D3DLOCKED_RECT     d3dRect;
  BOOL               bRET = FALSE;

  if (m_lpDevice != NULL || InitDirectX(hWnd))
    if (SUCCEEDED(m_lpDevice->CreateImageSurface(m_dwWidth, m_dwHeight, D3DFMT_A8R8G8B8, &pSurface)))
      if (SUCCEEDED(m_lpDevice->GetFrontBuffer(pSurface)))
        if (SUCCEEDED(pSurface->LockRect(&d3dRect, NULL, D3DLOCK_READONLY)))
          bRET = TRUE;

  if (bRET)
  {
    CopyPixels(&d3dRect, lpbOutput, 800, 600);
    pSurface->UnlockRect();
  }

  if (pSurface != NULL)
    pSurface->Release();

  return bRET;
}

BOOL GetMapleScreen(LPBYTE lpbOutput)
{
  return pCamera->Capture(GetMapleWindow(), lpbOutput);
}
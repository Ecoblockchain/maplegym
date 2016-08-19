#pragma once

#include <windows.h>
#include "main.h"
#include "d3d8.h"
#pragma  comment(lib, "d3d8")

#pragma pack(push, 1)

typedef struct _D3ARGB {
  BYTE b;
  BYTE g;
  BYTE r;
  BYTE a;
} D3ARGB, *LPD3ARGB;

#pragma pack(pop)

class Screenshotter
{
private:
  LPDIRECT3DDEVICE8 m_lpDevice;
  DWORD             m_dwWidth;
  DWORD             m_dwHeight;

public:
  Screenshotter(VOID);

  BOOL InitDirectX(HWND hWnd);
  VOID CopyPixels(D3DLOCKED_RECT* pRect, LPBYTE lpbDest, UINT nWidth, UINT nHeight);
  BOOL CreateDevice(LPDIRECT3D8 lpDirect3D, HWND hWnd, const D3DDISPLAYMODE* pcMode);
  BOOL Capture(HWND hWnd, LPBYTE lpbOutput);
};
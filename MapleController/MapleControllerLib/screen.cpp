#include <windows.h>
#include "main.h"

HBITMAP GetBitmap(HDC hCompatible, INT nWidth, INT nHeight, LPBYTE* ppBuffer)
{
  BITMAPINFO  bi;
  HBITMAP     hBitmap;

  ZeroMemory(&bi, sizeof(bi));

  bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
  bi.bmiHeader.biWidth       = nWidth;
  bi.bmiHeader.biHeight      = nHeight;
  bi.bmiHeader.biPlanes      = 1;
  bi.bmiHeader.biBitCount    = 24;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage   = nWidth * nHeight * 3;

  hBitmap = CreateDIBSection(hCompatible, &bi, DIB_RGB_COLORS, (PVOID*)ppBuffer, NULL, 0);
  return ((int)hBitmap == ERROR_INVALID_PARAMETER ? NULL : hBitmap);
}

BOOL GetMapleScreen(LPBYTE lpbOutput)
{
  HDC     hDC, hCompat;
  HBITMAP hBitmap;
  LPBYTE  lpbBuffer;
  BOOL    bRET = FALSE;
  HWND    hMaple = GetMapleWindow();

  hDC = GetDC(hMaple);
  if (hDC != NULL)
  {
    hCompat = CreateCompatibleDC(hDC);
    if (hCompat != NULL)
    {
      hBitmap = GetBitmap(hCompat, 800, 600, &lpbBuffer);
      if (hBitmap != NULL)
      {
        CopyMemory(lpbOutput, lpbBuffer, 800 * 600 * 3);
        bRET = TRUE;

        DeleteObject(hBitmap);
      }
      DeleteDC(hCompat);
    }
    ReleaseDC(hMaple, hDC);
  }
  return bRET;
}

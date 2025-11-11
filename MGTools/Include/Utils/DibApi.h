// dibapi.h
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef _INC_DIBAPI
#define _INC_DIBAPI

#include "stdafx.h"
#include "MGToolsExport.h"

#define COLOR_SCALE_RED   0
#define COLOR_SCALE_GREEN 1
#define COLOR_SCALE_BLUE  2
#define COLOR_SCALE_GRAY  3
#define COLOR_MESH_SCALE  4

#define PAL_COLORS    200

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* Print Area selection */
#define PW_WINDOW        1
#define PW_CLIENT        2

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

/* Function prototypes */
BOOL      WINAPI  PaintDIB (HDC,int, int, LPRECT, HDIB, LPRECT, CPalette* pPal);
BOOL      WINAPI  CreateDIBPalette(HDIB hDIB, CPalette* cPal);
LPSTR     WINAPI  FindDIBBits (LPSTR lpbi);
DWORD     WINAPI  DIBWidth (LPSTR lpDIB);
DWORD     WINAPI  DIBHeight (LPSTR lpDIB);
WORD      WINAPI  PaletteSize (LPSTR lpbi);
WORD      WINAPI  DIBNumColors (LPSTR lpbi);
HGLOBAL   MGTOOLS_API WINAPI  CopyHandle (HGLOBAL h);
HGLOBAL	  MGTOOLS_API CopyHandle32 (LPCVOID lpMem, DWORD Size);

BOOL      WINAPI  TransformDIBits(HDIB hDib);
BOOL      MGTOOLS_API WINAPI  SaveDIB (HDIB hDib, CFile& file);
HDIB      MGTOOLS_API WINAPI  ReadDIBFile(CFile& file);

HDIB MGTOOLS_API BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
HDIB MGTOOLS_API CopyScreenToDIB(LPRECT lpRect);
HBITMAP MGTOOLS_API CopyWindowToBitmap(HWND hWnd, WORD fPrintArea);
HBITMAP MGTOOLS_API CopyScreenToBitmap(LPRECT lpRect);
HPALETTE MGTOOLS_API GetSystemPalette(void);
int PalEntriesOnDevice(HDC hDC);
HPALETTE MGTOOLS_API CreateColorScalePalette(HDC hDC, int nColor);
void MGTOOLS_API GetPaletteColor(HPALETTE hP, int ind, float* pCol, int* pICol=0);

#endif //!_INC_DIBAPI

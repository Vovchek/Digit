
//
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
//
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
//
//  Author:		AAB
//  Description:	SECImage ABC implementation
//  Created:		Tue Aug 29 15:07:58 1995
//
//  CHANGELOG:
//   MSW       9/30/95   Code Review
//
// Future enhancements:
// fill in other bit colors
// change to cFile
// fill in member fxns for DC stuff
// Rearrange member functions to match header
// with operations/attributes at top and implementation
// stuff like padding towards the bottom out of the way.


// stdafx.h for SEC includes secall.h
#include "..\..\stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif

#ifndef __SECIMAGE_H__
#include "..\..\Include\Image\secimage.h"
#endif

#ifndef __SECDIB_H__
#include "..\..\Include\Image\secdib.h"
#endif

#ifndef _INC_WINDOWSX
#include <windowsx.h>
#endif

#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

#define ABS(x) ( (x)>0?(x):(-1*x))

#ifdef WIN32
typedef LOGPALETTE* LPLOGPALETTE;
#else
typedef LOGPALETTE FAR* LPLOGPALETTE;
#endif


//
//	SECImage - Constructor
//
//
//	Initialize member variables; null out data pointers
//
//
//

//@doc SECImage
//@mfunc Constructs a SECImage object.
//@xref <c SECImage>
SECImage::SECImage()
{

	HDC hDC = ::GetDC(NULL);
	// CDC sysDC;
	// HDC hDC = sysDC.GetSafeHdc();
	m_nBitPlanes = GetDeviceCaps(hDC, PLANES);
	m_nSysBitsPerPixel = m_nSrcBitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL);

	// determine number of system colors
	switch (m_nSysBitsPerPixel) {
		case 1:
			m_wSysColors = (WORD)(1 << m_nBitPlanes);
			break;
		case 2:
			m_wSysColors = 4;
			break;
		case 4:
			m_wSysColors = 16;
			break;
		case 8:
			m_wSysColors = 256;
			break;
		default:
			m_wSysColors = 0;
			m_nSysBitsPerPixel = 0;
			break;
		}
	::ReleaseDC(NULL, hDC);

	m_pPalette		= NULL;
	m_pFile			= NULL;
	m_lpBMI			= NULL;
	m_lpCache		= NULL;
	m_lpSrcBits		= NULL;

	m_dwWidth		= 0;
	m_dwHeight		= 0;
	m_dwCachePos	= 0;
	m_dwCacheSize	= 0;
	m_dwCacheMax	= 0;
	m_dwError		= 0;
	m_dwPadWidth		= 0;
	m_nSrcBitsPerPixel	= 0;
	m_wColors			= 0;

	m_bIsPadded		= FALSE;
	m_bUseHalftone	= FALSE;
	m_bSwapOnWrite	= FALSE; // some monochrome images need to be swapped when written
}

//
//	SECImage - Destructor
//
//
//
//
//
//

//@doc SECImage
//@mfunc SECImage destructor
//@xref <c SECImage>
SECImage::~SECImage()
{
    // Make sure all member data that might have been allocated is freed.
    if (m_lpSrcBits){
		//DWORD dwlen = GlobalSize(GlobalPtrHandle(m_lpSrcBits));
		GlobalFreePtr(m_lpSrcBits);
		m_lpSrcBits = NULL;
    }

	if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
		m_lpBMI = NULL;
	}

	if (m_pPalette)	{
		delete m_pPalette;
		m_pPalette = NULL;
	}

	OnCleanupAllocFiles();
}

//
//
//	CopyImage
//
//	Takes two SECImage types and duplicates the contents of
// 	pSrc into pDest.   If pDest has any previous data stored,
//	it will be overwritten and freed.  This is a "full" copy,
//	in that duplicates will be made of all member data, including
//	the full pixel array of the source.
//
//@doc SECImage
//@mfunc Creates a duplicate of the current image.
//@rdesc Nonzero if successful; otherwise 0.
//@parm SECImage * | pSrc | Source image to be used.
//@comm Makes a full copy of the source image.  Memory is allocated
// for the image data; if not enough is available, an error value is returned.
//@xref <c SECImage> <mf SECImage::ConvertImage>
BOOL SECImage::CopyImage(SECImage *pSrc)
{
    DWORD dwBufSize;
    ASSERT_VALID(pSrc);


    // This member function does not create new space for the class
    // instance itself.

    if (m_lpSrcBits){
		GlobalFreePtr(m_lpSrcBits);
		m_lpSrcBits = NULL;
    }

    if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
		m_lpBMI = NULL;
	}

    // Before we make the easy assignments, attempt to copy over
    // the full pixel array if it exists.
    dwBufSize = GlobalSize(GlobalPtrHandle(pSrc->m_lpSrcBits));
#ifdef WIN32
    m_lpSrcBits = (LPBYTE)GlobalAllocPtr(GHND, dwBufSize);
#else
    m_lpSrcBits = (BYTE huge *)GlobalAllocPtr(GHND, dwBufSize);
#endif
    if (!m_lpSrcBits)
	{
		TRACE(_T("SECImage::CopyImage failed due to lack of memory\n"));
		goto copy_image_fail;
	}
    _fmemcpy(m_lpSrcBits, pSrc->m_lpSrcBits, (size_t)dwBufSize);

    // Now do the same for the colormap if it exists..
    dwBufSize = GlobalSize(GlobalPtrHandle(pSrc->m_lpBMI));
    m_lpBMI= (LPBITMAPINFO)GlobalAllocPtr(GHND, dwBufSize);
    if (!m_lpBMI)
	{
		TRACE(_T("SECImage::CopyImage could not allocate color map\n"));
		goto copy_image_fail;
	}
    _fmemcpy(m_lpBMI, pSrc->m_lpBMI, (size_t)dwBufSize);

    m_lpRGB = m_lpBMI->bmiColors;

	m_dwWidth			= pSrc->m_dwWidth;
	m_dwHeight			= pSrc->m_dwHeight;
	m_dwPadWidth		= pSrc->m_dwPadWidth;
	m_dwBitCount		= pSrc->m_dwBitCount;
	m_nBitHolder		= pSrc->m_nBitHolder;
	m_nSrcBitsPerPixel	= pSrc->m_nSrcBitsPerPixel;
	m_nBitPlanes		= pSrc->m_nBitPlanes;
	m_wColors			= pSrc->m_wColors;
	m_wSysColors		= pSrc->m_wSysColors;

	m_bIsPadded			= pSrc->m_bIsPadded;
	m_bUseHalftone		= pSrc->m_bUseHalftone;
	m_bSwapOnWrite		= pSrc->m_bSwapOnWrite;

    // Keep the cache info reset to unused state
    FreeCache();

    // Create palette entry for Destination image
    CreatePalette();

	// make sure new image is valid
	ASSERT_VALID(this);
    return TRUE;

 copy_image_fail:

    if (m_lpSrcBits){
		GlobalFreePtr(m_lpSrcBits);
		m_lpSrcBits = NULL;
    }

    if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
		m_lpSrcBits = NULL;
    }

    return FALSE;
}

//
//
//	ConvertImage
//
//	Takes two SECImage types and moves the base class data from
//  the source to the destination.   More importantly, member
//  pointers are simply copied rather than duplicated (as is the
//  case in SECImage::CopyImage).
//
//  NOTE: IF FUNCTION IS SUCCESSFUL, THE SOURCE IMAGE IS INVALID!
//
//@doc SECImage
//@mfunc Converts from one derived image class to another.
//@rdesc Nonzero if successful; otherwise 0.
//@parm SECImage * | pSrc | Source image to convert from.
//@comm Converts the image stored in the derived image source to the
// format of the current instance.
//@devnote If this function is successful, the source image is
// invalid.
//@xref <c SECImage> <mf SECImage::CopyImage>
BOOL SECImage::ConvertImage(SECImage *pSrc)
{
    ASSERT_VALID(pSrc);

    // This member function does not create new space for the class
    // instance of the destination.

    m_lpSrcBits			= pSrc->m_lpSrcBits;
	m_lpBMI				= pSrc->m_lpBMI;
    m_lpRGB				= m_lpBMI->bmiColors;
    m_pPalette			= pSrc->m_pPalette;

	m_dwWidth			= pSrc->m_dwWidth;
	m_dwHeight			= pSrc->m_dwHeight;
	m_dwPadWidth		= pSrc->m_dwPadWidth;
	m_dwBitCount		= pSrc->m_dwBitCount;
	m_nBitHolder		= pSrc->m_nBitHolder;
	m_nSrcBitsPerPixel	= pSrc->m_nSrcBitsPerPixel;
	m_nBitPlanes		= pSrc->m_nBitPlanes;
	m_wColors			= pSrc->m_wColors;
	m_wSysColors		= pSrc->m_wSysColors;

	m_bIsPadded			= pSrc->m_bIsPadded;
	m_bUseHalftone		= pSrc->m_bUseHalftone;
	m_bSwapOnWrite		= pSrc->m_bSwapOnWrite;

	// preserve the cache information
    m_dwCacheMax		= pSrc->m_dwCacheMax;
	m_dwCacheSize		= pSrc->m_dwCacheSize;
	m_dwCachePos		= pSrc->m_dwCachePos;
	m_lpCache			= pSrc->m_lpCache;

	// source is invalid; null out pointers so
	// the buffers will not get deleted when the source
	// object is destroyed
	pSrc->m_lpSrcBits	= NULL;
	pSrc->m_lpBMI		= NULL;
	pSrc->m_lpRGB		= NULL;
	pSrc->m_pPalette	= NULL;

    return TRUE;
}

//@doc SECImage
//@mfunc Returns the width of the current image.
//@rdesc The width in pixels of the image.
//@comm Returns 0 if no image is loaded.
//@xref <c SECImage> <mf SECImage::dwGetHeight>
DWORD SECImage::dwGetWidth()
{
    return (DWORD)m_dwWidth;
}

//@doc SECImage
//@mfunc Returns the height of the current image.
//@rdesc The height in pixels of the image.
//@comm Returns 0 if no image is loaded.
//@xref <c SECImage> <mf SECImage::dwGetWidth>
DWORD SECImage::dwGetHeight()
{
    return (DWORD)m_dwHeight;
}

DWORD SECImage::PubCalcPadding(DWORD dwBitsPerPixel, DWORD dwPixels)
{
	return CalcPadding( dwBitsPerPixel, dwPixels);
}

DWORD SECImage::CalcPadding (DWORD dwBitsPerPixel, DWORD dwPixels)
{
    DWORD dwBits, dwPadBits;

    dwBits    = dwBitsPerPixel * dwPixels;

    if ((dwBits % 32) == 0)
        // already DWORD aligned, no padding needed
        return 0;
    else
        dwPadBits = 32 - (dwBits % 32);

    return (dwPadBits / 8 + (((dwPadBits % 8) > 0) ? 1 : 0));
}

DWORD SECImage::PubLastByte(DWORD dwBitsPerPixel, DWORD dwPixels)
{
  return LastByte(dwBitsPerPixel, dwPixels);
}

DWORD SECImage::LastByte(DWORD dwBitsPerPixel, DWORD dwPixels)
{
    DWORD dwBits, extraBits, numBytes;

    dwBits = dwBitsPerPixel * dwPixels;
        numBytes  = dwBits / 8;

        extraBits = dwBits - numBytes * 8;
        if((extraBits % 8) > 0)
                numBytes++;

    return (numBytes);
}

DWORD SECImage::PubCalcBytesPerLine(DWORD dwBitsPerPixel, DWORD dwWidth)
{
	return CalcBytesPerLine(dwBitsPerPixel, dwWidth);
}

DWORD SECImage::CalcBytesPerLine(DWORD dwBitsPerPixel, DWORD dwWidth) {
    DWORD dwBits, dwPadBits;

    dwBits    = dwBitsPerPixel * dwWidth;

    if ((dwBits % 32) == 0)
        return (dwBits/8);  // already DWORD aligned, no padding needed
    else
        dwPadBits = 32 - (dwBits % 32);

    return (dwBits/8 + dwPadBits/8 + (((dwPadBits % 8) > 0) ? 1 : 0));
}

//
//	PadBits
//
//
//	Pad the image byte rows on DWORD boundaries if needed.
//	UnpadBits does the reverse.
//
BOOL SECImage::PubPadBits()
{
	return PadBits();
}
BOOL SECImage::PadBits()
{
    // dwAdjust used when bits per pixel spreads over more than 1 byte
    DWORD dwOffset = 0, dwPadOffset=0;
    m_dwPadWidth = PADWIDTH(m_dwWidth);

    // check for conditions where we don't need to pad
	if (m_bIsPadded)
	return TRUE;

	dwPadOffset = CalcBytesPerLine(m_nSrcBitsPerPixel, m_dwWidth);
	dwOffset = LastByte(m_nSrcBitsPerPixel, m_dwWidth);

	if (dwPadOffset == dwOffset)
		return TRUE;

    //AAB LPBYTE lptemp = (LPBYTE)GlobalAllocPtr(GHND, m_dwWidth*dwAdjust);
	LPBYTE lptemp = (LPBYTE) GlobalAllocPtr(GHND, dwPadOffset);
    if(!lptemp)
	{
		TRACE(_T("SECImage::Padbits could not allocate row of width %d..\n"), m_dwWidth);
		return FALSE;
	}

    // enough space has already been allocated for the bit array to
    // include the padding, so we just need to shift rows around.
    // This will pad each "row" on a DWORD alignment.

    for (DWORD row=m_dwHeight-1; row>0; row--)
	{
	    _fmemcpy((LPBYTE)lptemp, (LPBYTE)(m_lpSrcBits + (row*dwOffset)),
			 (size_t)(dwOffset));
	    //AAB _fmemcpy((LPBYTE)(m_lpSrcBits + (row*PADWIDTH(dwPadOffset))),
	    _fmemcpy((LPBYTE)(m_lpSrcBits + (row*dwPadOffset)),
			 lptemp, (size_t)(dwOffset));
	}
    GlobalFreePtr(lptemp);

    // set the pad flag
    m_bIsPadded = 1;
    return (TRUE);
}

//
//	UnPadBits
//
//	See PadBits
//
BOOL SECImage::PubUnPadBits()
{
	return UnPadBits();
}

BOOL SECImage::UnPadBits()
{
	DWORD dwOffset = 0, dwPadOffset = 0;

    if (!m_bIsPadded)
	return TRUE;

	dwPadOffset = CalcBytesPerLine(m_nSrcBitsPerPixel, m_dwWidth);
	dwOffset = LastByte(m_nSrcBitsPerPixel, m_dwWidth);

    LPBYTE lptemp = (LPBYTE)GlobalAllocPtr(GHND, dwOffset);
    if(!lptemp)
	{
		TRACE(_T("SECImage::UnpadBits could not allocate row of width %d..\n"), m_dwWidth);
		return FALSE;
	}

    // enough space has already been allocated for the bit array to
    // include the padding, so we just need to shift rows around.
    for (DWORD row=1; row<m_dwHeight; row++)
	{
		_fmemcpy(lptemp, (LPBYTE)(m_lpSrcBits + row*(dwPadOffset)),
		     (size_t)(dwOffset));
	    _fmemcpy((LPBYTE)(m_lpSrcBits + (row*dwOffset)), lptemp,
		     (size_t)(dwOffset));
	}

    GlobalFreePtr(lptemp);
    m_bIsPadded = 0;

    return (TRUE);
}

//@doc SECImage
//@mfunc Called before an image is saved.
//@rdesc Nonzero if saving should proceed, otherwise 0.
//@comm Override this method to provide verification before
// an image is saved.
//@xref <c SECImage> <mf SECImage::SaveImage> <mf SECImage::DoSaveImage> <mf SECImage::PostSaveImage>
BOOL SECImage::PreSaveImage()
{
    // Nothing to do by default
    return TRUE;
}

//@doc SECImage
//@mfunc Called after an image is saved.
//@rdesc Nonzero if saving is considered successful, otherwise 0.
//@comm Override this method to provide verification after
// an image is saved.  If this method returns FALSE,
// <mf SECImage::SaveImage> will also return FALSE.
//@xref <c SECImage> <mf SECImage::SaveImage> <mf SECImage::DoSaveImage> <mf SECImage::PostSaveImage>
BOOL SECImage::PostSaveImage()
{
    // Nothing to do by default
    return TRUE;
}

void SECImage::OnCleanupAllocFiles() {
	CFile* pFile;
	POSITION pos=m_listFileAlloc.GetHeadPosition();
	while(pos) {
		pFile=(CFile *)m_listFileAlloc.GetNext(pos);
		delete pFile;
		}
	m_listFileAlloc.RemoveAll();
}

//@doc SECImage
//@mfunc Saves the current image to a file in the format of the derived image class.
//@syntax BOOL SECImage::SaveImage(CFile* pFile);
//@syntax BOOL SECImage::SaveImage(LPCTSTR lpszFileName);
//@rdesc Nonzero if successful; otherwise 0.
//@parm CFile* | pFile | File to save image to.
//@parm LPCTSTR | lpszFileName |  Filename to save image to.
//@comm Saves the current image data to a file in the format of the derived image class.
//
//Conversion to derived image class file format is performed by the derived class.
//@xref <c SECImage> <mf SECImage::DoSaveImage> <mf SECImage::PreSaveImage> <mf SECImage::PostSaveImage>
BOOL SECImage::SaveImage(CFile* pFile)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pFile);

    m_pFile = pFile;

    if (!PreSaveImage())
	return FALSE;

    // Call the virtual image saving function.
    if (!DoSaveImage(pFile))
	return FALSE;

    if (!PostSaveImage())
	return FALSE;

    return TRUE;
}

BOOL SECImage::SaveImage(LPCTSTR lpszFileName)
{
    ASSERT(lpszFileName != 0);
    ASSERT_VALID(this);

    CFile* pFile;
    BOOL rtn;

    if (m_pFile) {
		pFile = m_pFile;
		if (!pFile->Open(lpszFileName, CFile::modeCreate|CFile::modeWrite)) {
	    	TRACE(_T("Failed to open file %s for writing.\n"), lpszFileName);
	    	return FALSE;
			}
    	}
    else {
		pFile = new CFile(lpszFileName, CFile::modeCreate|CFile::modeWrite);
		m_listFileAlloc.AddTail(pFile);		// add for memory cleanup
		}

    ASSERT_VALID(pFile);

    rtn = SaveImage(pFile);
    pFile->Close();

    return rtn;
}

//@doc SECImage
//@mfunc Called before an image is loaded.
//@rdesc Nonzero if loading is to proceed, otherwise 0.
//@comm Override this method to provide checking before
// an image is loaded.
//@xref <c SECImage> <mf SECImage::LoadImage> <mf SECImage::DoLoadImage> <mf SECImage::PostLoadImage>
BOOL SECImage::PreLoadImage()
{
    // Make sure previous color map info is freed.  It is up to the
    // the derived image class to populate the m_lpBMI member, so
    // m_lpBMI should be valid immediately following the ReadImage()
    if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
		m_lpBMI = NULL;
	}

    if (m_lpSrcBits){
		//DWORD dwlen = GlobalSize(GlobalPtrHandle(m_lpSrcBits));
		GlobalFreePtr(m_lpSrcBits);
		m_lpSrcBits = NULL;
    }

	if (m_pPalette)
	{
		delete m_pPalette;
		m_pPalette = NULL;
	}

	m_dwWidth			= 0;
	m_dwHeight			= 0;
	m_dwError			= 0;
	m_dwPadWidth		= 0;
	m_nSrcBitsPerPixel	= 0;
	m_wColors			= 0;

	m_bIsPadded			= FALSE;

    // Keep the cache info reset to unused state
    FreeCache();

    return TRUE;
}

//@doc SECImage
//@mfunc Called after an image is loaded.
//@rdesc Nonzero if loading is considered successful, otherwise 0.
//@comm Override this method to provide checking after
// an image is loaded.  If this method returns FALSE,
// <mf SECImage::LoadImage> will also return FALSE.
//@xref <c SECImage> <mf SECImage::LoadImage> <mf SECImage::DoLoadImage> <mf SECImage::PreLoadImage>
BOOL SECImage::PostLoadImage()
{
    // If a colormap exists, it is now loaded and a palette can be created.
    CreatePalette();

    // Make sure the image is correctly padded on DWORD boundary; which
    // is required for the DIB format.
    m_dwPadWidth = PADWIDTH(m_dwWidth);
    PadBits();

	// check if the image resolution does not match the current display;
	// if so then set the m_bUseHalftone flag
    if( m_nSysBitsPerPixel )
        if (m_nSrcBitsPerPixel != m_nSysBitsPerPixel)
            m_bUseHalftone = TRUE;


    // Check if structure is already allocated.  It is up to the derived
    // image classes to populate the m_lpBMI BITMAPINFOHEADER structure,
    // however, if for some reason they failed in that endeavour, then this
    // last ditch case will attempt to allocate/fill m_lpBMI.
    if (m_lpBMI) {
	return TRUE;
    }

    // Take our best shot at implementing a BitmapInfo structure for this image
    if (!m_lpBMI)
	m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, sizeof(BITMAPINFOHEADER) +
					       m_wColors*sizeof(RGBQUAD));
    if (!m_lpBMI)
	{
		TRACE(_T("SECImage::LoadImage could not allocate colormap with %d colors\n"), m_wColors);
		return FALSE;
	}
    m_lpBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_lpBMI->bmiHeader.biWidth = m_dwWidth;
    m_lpBMI->bmiHeader.biHeight = m_dwHeight;
    m_lpBMI->bmiHeader.biPlanes = (WORD)m_nBitPlanes;
    m_lpBMI->bmiHeader.biBitCount =  (WORD)m_nSrcBitsPerPixel;
    m_lpBMI->bmiHeader.biCompression = BI_RGB;
//    m_lpBMI->bmiHeader.biSizeImage = m_dwPadWidth *  m_dwHeight;
    // Memory alloc issue.
	int nWidth = 0;
	int nBits = m_dwWidth * m_lpBMI->bmiHeader.biBitCount;

	// Align to DWORD & Convert to byte
	if (nBits % 32) {
		nWidth = (nBits/32+1) * 4 /* =(32/8) */;
	} else {
		nWidth = nBits / 8;
	}

    m_lpBMI->bmiHeader.biSizeImage = nWidth *  m_dwHeight;

    return TRUE;
}

//@doc SECImage
//@mfunc Loads image data from an image file.
//@syntax BOOL LoadImage(LPCTSTR lpszFileName);
//@syntax BOOL LoadImage(CFile* pFile);
//@rdesc Nonzero if successful; otherwise 0.
//@parm LPCTSTR | lpszFileName |  Filename to load image from.
//@parm CFile* | pFile | File to load image from.
//@comm Loads an image from a file.
//
// The derived image subclasses will actually convert the image data
// from the native image format to the intermediate format used by SECImage.
//
// The derived image classes allocate memory to store the image data.
// If not enough memory is available to hold the image, LoadImage will
// return an error condition.
//
// If the loaded image uses a format of 8 bits per pixel or less, LoadImage
// will read a colormap from the image and create a CPalette stored as member
// data m_pPalette.  If the image uses 24 bits per pixel or more, no palette
// is created.
//
// When loaded, image data is automatically converted to an intermediate format
// associated with SECImage.
//@xref <c SECImage> <mf SECImage::SaveImage> <mf SECImage::ConvertImage> <mf SECImage::CopyImage>
BOOL SECImage::LoadImage(CFile* pFile)
{
    ASSERT_VALID(pFile);

    m_pFile = pFile;

	// if this instance contains valid image data,
	// transfer it to a temporary image object.
	// If the load fails, or is cancelled, it can
	// then be restored before returning
	SECDib* pImg = NULL;
	if ((m_lpSrcBits) && (m_dwWidth > 0) && (m_dwHeight > 0) && (m_lpBMI != 0)) {
		pImg = new SECDib;
		if (pImg)
			pImg->ConvertImage(this);
	}

    if (!PreLoadImage()) {
		if (pImg)
			ConvertImage(pImg);
		return FALSE;
	}

    // Call the virtual image loading function.  The derived class
    // will convert to the correct intermediate format (DIB for now).
    if (!DoLoadImage(pFile)) {
		if (pImg)
			ConvertImage(pImg);
		return FALSE;
	}

    if (!PostLoadImage()) {
		if (pImg)
			ConvertImage(pImg);
		return FALSE;
	}

	if (pImg) {
		delete pImg;
		pImg = NULL;
	}

    return TRUE;
}

BOOL SECImage::LoadImage(LPCTSTR lpszFileName)
{
    CFile *pFile;
    BOOL rtn;

    if (m_pFile) {
		pFile = m_pFile;
		pFile->Open(lpszFileName, CFile::modeRead);
    	}
    else {
		pFile = new CFile(lpszFileName, CFile::modeRead);
		m_listFileAlloc.AddTail(pFile);		// add for memory cleanup
		}

    ASSERT_VALID(pFile);
    rtn = LoadImage(pFile);
    pFile->Close();

    return rtn;
}

//
//	MakeBitmap
//
//
//
//
//
//
//@doc SECImage
//@mfunc Creates a CBitmap from the current image data.
//@syntax CBitmap* SECImage::MakeBitmap(CDC *pDC, const TCHAR * lpszFileName)
//@syntax CBitmap* SECImage::MakeBitmap(CDC *pDC)
//@rdesc Returns a pointer to a CBitmap if successful; otherwise 0.
//@parm CDC * | pDC  | Device context to create a new bitmap for.
//@parm  const TCHAR * | lpszFileName | Name of the file to create a new bitmap for.
//@comm Creates a CBitmap object compatible with the device context
// specified by pDC, using the image data as the source for the bitmap data.
//@devnote When you are finished using the CBitmap object created by MakeBitmap,
// make sure the bitmap is selected out of the device context, then delete
// the CBitmap object.
//@xref <c SECImage> <mf SECImage::CreateFromBitmap>
CBitmap* SECImage::MakeBitmap(CDC *pDC, const TCHAR *lpszFileName)
{

	ASSERT_VALID(pDC);
	ASSERT(lpszFileName != 0);

    if (!LoadImage(lpszFileName))
		return NULL;

    CBitmap* pBitmap = new CBitmap;
    pBitmap->CreateBitmap((int)m_dwWidth, (int)m_dwHeight, m_nBitPlanes,
			  m_nSrcBitsPerPixel, m_lpSrcBits);

    pDC; //UNUSED
    return pBitmap;
}



//
//	CreateFromBitmap
//
//	Fills in an SECImage instance with DIB info gathered from a
//	passed in DC and CBitmap.  Invalidates any previous data in
//	the instance.
//
//@doc SECImage
//@mfunc Fills in an SECImage instance with DIB info.
//@rdesc Nonzero if successful; otherwise 0;
//@parm CDC * | pDC | Pointer to device context.
//@parm  CBitmap * | pSrcBitmap | Pointer to source bitmap.
//@comm Fills in an SECImage instance with DIB info gathered from a
//	passed in DC and CBitmap.  Invalidates any previous data in
//	the instance.
//@xref <c SECImage> <mf SECImage::MakeBitmap>
BOOL SECImage::CreateFromBitmap(CDC *pDC, CBitmap *pSrcBitmap)
{
	ASSERT_VALID(pSrcBitmap);
	ASSERT_VALID(pDC);

	BITMAP bmHdr;

	// Get the pSrcBitmap info
	pSrcBitmap->GetObject(sizeof(BITMAP), &bmHdr);

	// Reallocate space for the image data
	if (m_lpSrcBits)
		GlobalFreePtr(m_lpSrcBits);

	DWORD dwWidth;
	if (bmHdr.bmBitsPixel > 8)
		dwWidth = PADWIDTH(bmHdr.bmWidth * 3);
	else
		dwWidth = PADWIDTH(bmHdr.bmWidth);

#ifndef WIN32
	long lTest = (long)(dwWidth)*(long)bmHdr.bmHeight;
	m_lpSrcBits = (BYTE huge *)GlobalAllocPtr(GHND, lTest);
#else
	m_lpSrcBits = (LPBYTE)GlobalAllocPtr(GHND, dwWidth*bmHdr.bmHeight);
#endif
	if (!m_lpSrcBits)
	{
		TRACE(_T("SECImage::CreateFromBitmap could not allocate data storage\n"));
		return FALSE;
	}

	// Set the appropriate number of colors base on BITMAP structure info
	switch (bmHdr.bmBitsPixel)
	{
		case 1 :
			m_wColors = (WORD)(1 << bmHdr.bmPlanes);
			break;
		case 4 :
			m_wColors = 16;
			break;
		case 8 :
			m_wColors = 256;
			break;
		default :
			m_wColors = 0;
			break;
	}

	// Re-allocate and populate BITMAPINFO structure
	if (m_lpBMI)
		GlobalFreePtr(m_lpBMI);

	m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND,
		sizeof(BITMAPINFOHEADER) + m_wColors*sizeof(RGBQUAD) );
	if (!m_lpBMI)
	{
		if (m_lpSrcBits)
		{
			GlobalFreePtr(m_lpSrcBits);
			m_lpSrcBits = NULL;
		}
		TRACE(_T("SECImage::CreateFromBitmap could not allocate BITMAPINFO struct\n"));
		return FALSE;
	}

	// Populate BITMAPINFO header info
	m_lpBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_dwWidth = m_lpBMI->bmiHeader.biWidth = bmHdr.bmWidth;
	m_dwPadWidth = PADWIDTH(m_dwWidth);
	m_dwHeight = m_lpBMI->bmiHeader.biHeight = bmHdr.bmHeight;
	m_lpBMI->bmiHeader.biPlanes = bmHdr.bmPlanes;


	if (bmHdr.bmBitsPixel > 8)
		m_lpBMI->bmiHeader.biBitCount = 24;
	else
		{
		m_lpBMI->bmiHeader.biBitCount = (USHORT)(bmHdr.bmBitsPixel * bmHdr.bmPlanes );
		m_lpBMI->bmiHeader.biPlanes = bmHdr.bmPlanes = 1 ;
		}
	m_nSrcBitsPerPixel = m_lpBMI->bmiHeader.biBitCount;

	m_lpBMI->bmiHeader.biCompression = BI_RGB;
	m_lpBMI->bmiHeader.biSizeImage = ((((bmHdr.bmWidth * bmHdr.bmBitsPixel) + 31) & ~31) >> 3) * bmHdr.bmHeight;
;
	m_lpBMI->bmiHeader.biXPelsPerMeter = 0;
	m_lpBMI->bmiHeader.biYPelsPerMeter = 0;
	m_lpBMI->bmiHeader.biClrUsed = 0;
	m_lpBMI->bmiHeader.biClrImportant = 0;
	m_lpRGB = (LPRGBQUAD)(m_lpBMI->bmiColors);

	// Now actually get the bits
	int test = ::GetDIBits(pDC->GetSafeHdc(), (HBITMAP)pSrcBitmap->GetSafeHandle(),
	 	0, (WORD)bmHdr.bmHeight, m_lpSrcBits, m_lpBMI, DIB_RGB_COLORS);


	// check that we scanned in the correct number of bitmap lines
	if (test != (int)bmHdr.bmHeight)
	{
		TRACE(_T("SECImage::CreateFromBitmap call to GetDIBits did not return full number of requested scan lines\n"));
	}
	CreatePalette();
	m_bIsPadded = TRUE;

	// Fill out DIB header info
	return TRUE;
}


//
//	MakeBitmap, assumes bitmap's already read in.
//
//
//	Note: up to the caller to free this CBitmap *
//
//	MSW - I created this because I already read in and
//      just wanted a CBitmap...
//
CBitmap* SECImage::MakeBitmap(CDC *pDC)
{
    ASSERT_VALID(pDC);

    CBitmap* pBitmap = new CBitmap();
	CBitmap* pBitmap2;
	CDC *pNewDC = new CDC();
	pNewDC->CreateCompatibleDC(pDC);
#ifdef WIN32
	pBitmap->CreateCompatibleBitmap(pDC,
		m_dwWidth, m_dwHeight);
#else
	pBitmap->CreateCompatibleBitmap(pDC,
		(int)m_dwWidth, (int)m_dwHeight);
#endif
	pBitmap2 = pNewDC->SelectObject(pBitmap);

#ifdef WIN32
    ::StretchDIBits(
		pNewDC->GetSafeHdc(),
		0,0,
		m_dwWidth, m_dwHeight,
		0,0,
		m_dwWidth, m_dwHeight,
		m_lpSrcBits,
		m_lpBMI, DIB_RGB_COLORS,
		SRCCOPY);
#else
    ::StretchDIBits(
		pNewDC->GetSafeHdc(),
		0,0,
		(int)m_dwWidth, (int)m_dwHeight,
		0,0,
		(int)m_dwWidth, (int)m_dwHeight,
		m_lpSrcBits,
		m_lpBMI, DIB_RGB_COLORS,
		SRCCOPY);
#endif

	pBitmap2 = (CBitmap *)pNewDC->SelectObject(pBitmap2);
	pNewDC->DeleteDC();
	delete pNewDC;
	return pBitmap2;
    // return pBitmap;

}

//
//	StretchDIBits
//
//	Encapsulates the ::StretchDIBits() call.  If m_bUseHalftone is specified,
//	the function auto-detects whether a display has less color resolution
//	than the current bitmap, and if so, uses a halftone paint.
//
//@doc SECImage
//@mfunc Encapsulates the ::StretchDIBits() call.
//@rdesc If successful, returns the number of scan lines copied.
//@parm CDC * | pDC |  Pointer to device context.
//@parm  int | XDest |  Specifies the x-coordinate, in logical units, of
// the upper-left corner of the destination rectangle.
//@parm  int | YDest | Specifies the y-coordinate, in logical units, of
// the upper-left corner of the destination rectangle.
//@parm  int | cxDest | Specifies the width, in logical units, of the
// destination rectangle
//@parm  int | cyDest | Specifies the height, in logical units, of the
// destination rectangle.
//@parm  int | XSrc  | Specifies the x-coordinate, in pixels, of the
// source rectangle in the DIB
//@parm  int | YSrc | Specifies the y-coordinate, in pixels, of the source
// rectangle in the DIB.
//@parm  int | cxSrc | Specifies the width, in pixels, of the source
// rectangle in the DIB.
//@parm  int | cySrc  | Specifies the height, in pixels, of the source
// rectangle in the DIB.
//@parm  const void FAR* | lpvBits | Pointer to the DIB bits, which are
// stored as an array of bytes.
//@parm  LPBITMAPINFO | lpbmi | Pointer to a BITMAPINFO structure that
// contains information about the DIB.
//@parm  UINT | fuColorUse | Specifies whether the bmiColors member of
// the BITMAPINFO structure was provided and, if so, whether bmiColors
// contains explicit red, green, blue (RGB) values or indexes.
//@parm  DWORD | fdwRop | Specifies how the source pixels, the destination
// device context's current brush, and the destination pixels are to be
// combined to form the new image.
//@comm  If m_bUseHalftone is specified,
//	the function auto-detects whether a display has less color resolution
//	than the current bitmap, and if so, uses a halftone paint.
//@xref <c SECImage>
int SECImage::StretchDIBits(CDC *pDC, int XDest, int YDest, int cxDest, int cyDest,
							int XSrc, int YSrc, int cxSrc, int cySrc, const void FAR* lpvBits,
							LPBITMAPINFO lpbmi, UINT fuColorUse, DWORD fdwRop)
{
#ifdef WIN32
	int nOldStretchBltMode = 0;
	CPalette *palHalftone, *palOrig;
	int nLines;
	HDC hSafeDC = pDC->GetSafeHdc();

	ASSERT_VALID(pDC);
	ASSERT(lpbmi != 0);

	// Determine whether we just call into the standard StretchDIBits
	if (!m_bUseHalftone) //  || m_nSrcBitsPerPixel == m_wSysColors)
	return ::StretchDIBits(hSafeDC, XDest, YDest, cxDest, cyDest,
		XSrc, YSrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, fdwRop);

	// Create the halftone palette
	palHalftone = new CPalette();
	if (!palHalftone->CreateHalftonePalette(pDC))
		return 0;
	palOrig = pDC->SelectPalette(palHalftone, FALSE);
	if (!palOrig)
	{
		delete palHalftone;
		return 0;
	}
	pDC->RealizePalette();


	// Initialize HALFTONE to the DC
	if(pDC->m_hAttribDC != NULL)
	{
		nOldStretchBltMode = pDC->GetStretchBltMode();
		pDC->SetStretchBltMode(HALFTONE);
	}

	nLines = ::StretchDIBits(hSafeDC, XDest, YDest, cxDest, cyDest,
		XSrc, YSrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, fdwRop);
	//DWORD test = GetLastError();

	// Restore original DC state
	if(pDC->m_hAttribDC != NULL)
		pDC->SetStretchBltMode(nOldStretchBltMode);
	pDC->SelectPalette(palOrig, FALSE);
	delete palHalftone;
	return nLines;
#else

	HDC hSafeDC = pDC->GetSafeHdc();

	ASSERT_VALID(pDC);
	ASSERT(lpbmi != 0);

	// Just call standard StretchDIBits for WIN16
	return ::StretchDIBits(hSafeDC, XDest, YDest, cxDest, cyDest,
		XSrc, YSrc, cxSrc, cySrc, lpvBits, lpbmi, fuColorUse, fdwRop);
#endif
}


//
//	CreatePalette
//
//	Creates m_pPalette based on the
//	color table.
//
//
//@doc SECImage
//@mfunc Creates the image palette.
//@rdesc Nonzero if successful; otherwise 0.
//@xref <c SECImage>
BOOL SECImage::CreatePalette()
{

    // DWORD dwColors = m_wColors;
    DWORD dwColors, dw;
    if ( m_lpBMI->bmiHeader.biClrUsed)
		dwColors = m_lpBMI->bmiHeader.biClrUsed;
	else
    	dwColors = m_wColors;

    // If 24 bpp, then all color information is stored with image data
    if (m_nSrcBitsPerPixel == 24)
	return TRUE;

    // Create a LOGPALETTE structure and convert the colormap entries
    // to this format.   LOGPALETTE is then used for palette creation.
    LPLOGPALETTE pPal = (LPLOGPALETTE)GlobalAllocPtr(GHND, sizeof(LOGPALETTE) +
						     ((m_lpBMI->bmiHeader.biClrUsed>m_wColors)?m_lpBMI->bmiHeader.biClrUsed:m_wColors)
						      	* sizeof(PALETTEENTRY));

    if (!pPal)
	{
		TRACE(_T("SECImage::CreatePalette failed allocation of %d colors\n"), dwColors);
		return (NULL);
	}
	//DWORD dw = 0;
    pPal->palVersion = 0x300;
    pPal->palNumEntries = (WORD) ((m_lpBMI->bmiHeader.biClrUsed>m_wColors)?m_lpBMI->bmiHeader.biClrUsed:m_wColors);
    for (dw=0; dw<dwColors; dw++)
	{
	BYTE red, green, blue;
	    red = pPal->palPalEntry[dw].peRed = m_lpRGB[dw].rgbRed;
	    green = pPal->palPalEntry[dw].peGreen = m_lpRGB[dw].rgbGreen;
	    blue = pPal->palPalEntry[dw].peBlue = m_lpRGB[dw].rgbBlue;
	    pPal->palPalEntry[dw].peFlags = 0;
	}
	for (;dw<m_wColors;dw++)
	{
	 	pPal->palPalEntry[dw].peRed = 0;
	    pPal->palPalEntry[dw].peGreen = 0;
	    pPal->palPalEntry[dw].peBlue = 0;
	    pPal->palPalEntry[dw].peFlags = 0;
	}

    // delete any existing palette
    if (m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}

    // create away...
    m_pPalette = new CPalette;
    BOOL bResult = m_pPalette->CreatePalette(pPal);
    if (pPal)
    	GlobalFreePtr(pPal);
    return bResult;
}



//
//	NearestColor -
//
//
//	Not implemented for now; will be used in future...
//
#ifdef ORIG
WORD SECImage::NearestColor(PALETTEENTRY *pEntries, PALETTEENTRY palEntry)
{
    DWORD dwTotalDiff = (255L * 255L) * 3L;
    DWORD dwDiffRed, dwDiffGreen, dwDiffBlue;
    DWORD dwDelta = 0;
    WORD wClosestMatch = 0;

    for (WORD wColor=0; wColor<m_wSysColors; wColor++) {
	if (palEntry.peRed > pEntries[wColor].peRed)
	    dwDiffRed = palEntry.peRed - pEntries[wColor].peRed;
	else dwDiffRed = pEntries[wColor].peRed - palEntry.peRed;
	if (palEntry.peGreen > pEntries[wColor].peGreen)
	    dwDiffGreen= palEntry.peGreen - pEntries[wColor].peGreen;
	else dwDiffGreen = pEntries[wColor].peGreen - palEntry.peGreen;
	if (palEntry.peBlue> pEntries[wColor].peBlue)
	    dwDiffBlue= palEntry.peBlue - pEntries[wColor].peBlue;
	else dwDiffBlue = pEntries[wColor].peBlue - palEntry.peBlue;

	dwDelta = dwDiffRed * dwDiffRed +
	    dwDiffGreen * dwDiffGreen +
	    dwDiffBlue * dwDiffBlue;
	if (!dwDelta)
	    return (wColor);
	if (dwDelta < dwTotalDiff){
	    dwTotalDiff = dwDelta;
	    wClosestMatch = wColor;
	}
    }
    return wClosestMatch;

}
#endif

//
//	NumColors -
//
//      Calculate number of colors in bitmap based on bit count
//
//@doc SECImage
//@mfunc Calculates number of colors in bitmap based on bit count.
//@rdesc The number of colors in the bitmap.
//@xref <c SECImage>
WORD SECImage::NumColors()
{
    ASSERT(m_lpBMI!=0);

    switch(m_lpBMI->bmiHeader.biBitCount)
	{
	case 1:
	    return 2;
	case 4:
	    return 16;
	case 8:
	    return 256;
	default:
	    return 0;
	}
}

//
//	NumBytes
//
//  	Calculate number of bytes associated in the entire array,
//      based on bits per pixel.
//
//@doc SECImage
//@mfunc Calculates number of bytes based on bits per pixel.
//@rdesc The number of bits per pixel.
//@xref <c SECImage>
DWORD SECImage::NumBytes()
{
    ASSERT(m_lpBMI != 0);


    DWORD rv = PADWIDTH((m_lpBMI->bmiHeader.biWidth)*((DWORD)m_lpBMI->bmiHeader.biBitCount)) *
	m_lpBMI->bmiHeader.biHeight;
    return rv;
}

// 	Cache functions:
//
//	These are used as internal file buffers for image compression
//	and decompression routines.   These speed up the load and save
//	times by buffering all the compression schemes which many times
//	write out data a byte at a time.
//
//	Useful variables:
//	m_lpCache : the cache itself
//	m_dwCacheSize :	size of currently loaded cache in bytes
//	m_dwCacheMax : maximum size of cache
//	m_dwCachePos : current position of the cache pointer.
//
//

//	LoadCache
//
//	Loads an internal image file cache into the buffer.
// 	returns the number of bytes read in.
//
DWORD SECImage::LoadCache()
{
    ASSERT_VALID(m_pFile);
	ASSERT(m_lpCache!=0);
	ASSERT(m_dwCacheMax>0);

    m_dwCacheSize = m_pFile->Read(m_lpCache, m_dwCacheMax);
    m_dwCachePos = 0;
    return m_dwCacheSize;
}

//
//	InitCache
//
//	Allocates memory for cache; sets whether cache is in
//	read or write mode
//	SEC_READ_CACHE == read mode
//	SEC_WRITE_CACHE == write mode
//
BOOL SECImage::InitCache(DWORD dwBufSize, WORD wMode)
{
    // Free up cache if it exists already
	ASSERT_VALID(m_pFile);

    if (m_lpCache){
	GlobalFreePtr(m_lpCache);
	m_lpCache = NULL;
	}

    if (!m_pFile)
	return FALSE;

    m_dwCacheMax = dwBufSize;
#ifdef WIN32
    m_lpCache = (LPBYTE)GlobalAllocPtr(GHND, m_dwCacheMax);
#else
    m_lpCache = (BYTE huge *)GlobalAllocPtr(GHND, m_dwCacheMax);
#endif
    // abort out in case allocation was unsuccessful
    if (!m_lpCache) {
		TRACE(_T("SECImage::InitCache could not allocate cache space\n"));
		m_dwCacheMax = 0;
		return FALSE;
    }
    m_dwCacheSize = m_dwCachePos = 0;

    if (wMode==SEC_READ_CACHE)
	LoadCache();

    return TRUE;
}


// Retrieve actual data from the file cache buffer to the provided
// memory location.   Caller must be responsible for enough space being
// available in destination buffer.
// Should only be called from READ_CACHE state
DWORD SECImage::GetCache(void *pBuf, DWORD dwBytes)
{
    ASSERT(pBuf!=0);
	ASSERT_VALID(m_pFile);
	ASSERT(dwBytes>0);

    DWORD dwCheckBufLoad, dwByteCount=dwBytes;

    // If cache has not been loaded...
    if (!m_dwCacheMax)
	return m_pFile->Read(pBuf, dwBytes);

    // Check if current buffer has enough data for request
    if (dwBytes < m_dwCacheSize - m_dwCachePos)
	{
	    _fmemcpy(pBuf, (m_lpCache+m_dwCachePos), (size_t)(dwBytes));
	    m_dwCachePos += dwBytes;
	    return (dwBytes);
	}
    // Otherwise we have to cycle through the buffer via a "LoadCache".
    while (dwByteCount)
	{
	    // copy any remaining cache data
	    _fmemcpy(pBuf, (m_lpCache+m_dwCachePos), (size_t)(m_dwCacheSize-m_dwCachePos));
	    // decrement the number of bytes to be read
	    dwByteCount -= m_dwCacheSize-m_dwCachePos;
	    // reload the cache
	    dwCheckBufLoad = LoadCache();
	    if (!dwCheckBufLoad)
		return (dwBytes-dwByteCount);
	}
    return dwBytes;
}


// Fill in cache buffer with data bytes; if you "overfill" it, then
// write out the data to the file and reset the cache pointers.
// Should only be called from WRITE_CACHE state
DWORD SECImage::PutCache(void *pBuf, DWORD dwBytes)
{
    ASSERT_VALID(m_pFile);
	ASSERT(pBuf!=0);
	ASSERT(dwBytes>0);

    DWORD dwpBufPos = 0;
    if (!m_dwCacheMax){
	m_pFile->Write(pBuf, dwBytes);
	return dwBytes;
    }

    while (dwBytes)
	{
	    // Just copy dwBytes worth of data into buffer if the buffer
	    // has enough space
	    if (m_dwCachePos + dwBytes < m_dwCacheMax)
		{
#ifdef WIN32
			_fmemcpy((m_lpCache+m_dwCachePos), (LPBYTE)pBuf+dwpBufPos, (size_t)(dwBytes));
#else
			// _fmemcpy((m_lpCache+m_dwCachePos), (BYTE *)(pBuf+dwpBufPos), (size_t)(dwBytes));
		    memcpy((m_lpCache+m_dwCachePos), ((BYTE *)pBuf+dwpBufPos), (size_t)dwBytes);
#endif
		    m_dwCachePos += dwBytes;
		    return (dwpBufPos + dwBytes);
		}

	    // otherwise fill out the rest of the buffer, and then write
	    // out to file.
	   else
	       {
		   // copy into whatever space is left in the cache and update
		   // position values
#ifdef WIN32
		   // memcpy((m_lpCache+m_dwCachePos), ((BYTE *)pBuf + dwpBufPos), (size_t)(m_dwCacheMax-m_dwCachePos));
		   _fmemcpy((m_lpCache+m_dwCachePos), (LPBYTE)pBuf+dwpBufPos, (size_t)(m_dwCacheMax-m_dwCachePos));
#else
		   // _fmemcpy((m_lpCache+m_dwCachePos), (BYTE *)pBuf+dwpBufPos, (size_t)(m_dwCacheMax-m_dwCachePos));
		   memcpy((m_lpCache+m_dwCachePos), (BYTE *)pBuf+dwpBufPos, (size_t)(m_dwCacheMax-m_dwCachePos));
#endif
		   dwpBufPos += (m_dwCacheMax-m_dwCachePos);
		   dwBytes -= (m_dwCacheMax-m_dwCachePos);
		   // m_dwCachePos = m_dwCacheMax-m_dwCachePos;
		   // m_dwCachePos = m_dwCacheMax - dwpBufPos;
		   m_dwCachePos = m_dwCacheMax - dwpBufPos + 1;

		   // Write out the current cache
		   FlushCache();
	       }
	}
    return (dwpBufPos + dwBytes);
}

//
//	FlushCache
//
//	Writes out the current contents of the image cache, and resets
//	the cache position variables
//
void SECImage::FlushCache()
{
    ASSERT_VALID(m_pFile);

    if (m_dwCachePos>0)
	{
	    m_pFile->Write(m_lpCache, (m_dwCachePos));
	    m_dwCachePos = 0;
	}
}

//
//	FreeCache
//
//	Free up the cache buffer and reset the position values to their
//	uninitialized state.
//
void SECImage::FreeCache()
{
    if (m_lpCache){
	GlobalFreePtr(m_lpCache);
	m_lpCache = NULL;
	}
    m_dwCachePos = 0;
    m_dwCacheSize = 0;
    m_dwCacheMax = 0;
}

//
//  LoadDib
//
//	This is put in secimage since our base format is in DIB format.  Note, the
//  derived SECDib class also reuses this code in it's SECLoadImage routine.
//  If in the future the base format for SECImage should change, this original
//  DIB specific code should be copied down to the derived SECDib class.
BOOL SECImage::SECLoadDib(CFile *pFile)
{
    ASSERT_VALID(pFile);

    WORD wBytes = 0;
	int cutCompress = 0;
    LPBITMAPFILEHEADER lpbmfHdr = (LPBITMAPFILEHEADER)GlobalAllocPtr(GHND, sizeof(BITMAPFILEHEADER));
    if (!lpbmfHdr)
	{
		TRACE(_T("SECImage::SECLoadDib could not allocate BITMAPFILEHEADER space\n"));
		goto sec_dib_load_err;
	}


    wBytes = (WORD)pFile->Read(lpbmfHdr, sizeof(BITMAPFILEHEADER));

    if (lpbmfHdr->bfType != 0x4d42)
	{
	    return FALSE;
	}

    //AAB REALLOC BUG m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER));
	m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER) + 256*sizeof(RGBQUAD));
    if (!m_lpBMI)
	{
		TRACE(_T("SECImage::LoadDib could not allocate space for BITMAPINFO\n"));
		goto sec_dib_load_err;
	}

    // Read in bitmap header info
    wBytes = (WORD)pFile->Read(m_lpBMI, (UINT)(lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER)));

	if(m_lpBMI->bmiHeader.biXPelsPerMeter != 0 || m_lpBMI->bmiHeader.biYPelsPerMeter != 0){
	  m_lpBMI->bmiHeader.biXPelsPerMeter = 0;
	  m_lpBMI->bmiHeader.biYPelsPerMeter = 0;
	  cutCompress = 2;
	}

    // fill in some member data
    m_bIsPadded = TRUE; // DIB files already padded...
    m_dwWidth = (DWORD) m_lpBMI->bmiHeader.biWidth;
    m_dwPadWidth = PADWIDTH(m_dwWidth);
    m_dwHeight = (DWORD) m_lpBMI->bmiHeader.biHeight;

	m_nSrcBitsPerPixel = m_lpBMI->bmiHeader.biBitCount;

	if(m_nSrcBitsPerPixel < 8)
		return FALSE;

    switch(m_lpBMI->bmiHeader.biBitCount)
	{
	case 1:
	    m_wColors = 2;
	    break;
	case 4 :
	    m_wColors = 16;
	    break;
	case 8 :
	    m_wColors = 256;
	    break;
	default :
	    m_wColors = 0;
	}

    m_lpRGB=m_lpBMI->bmiColors;
	//AAB REALLOC BUG m_lpBMI = (LPBITMAPINFO)GlobalReAllocPtr(m_lpBMI, sizeof(BITMAPINFO) + m_wColors*sizeof(RGBQUAD),GMEM_MOVEABLE);

    // Now read in the actual bitmap data from the DIB
    if (m_lpSrcBits){
	GlobalFreePtr(m_lpSrcBits);
	m_lpSrcBits = NULL;
    }

    m_lpSrcBits = (LPBYTE)GlobalAllocPtr( GHND, (m_dwPadWidth * m_dwHeight* m_lpBMI->bmiHeader.biBitCount / 8-cutCompress));


    if (!m_lpSrcBits)
	{
		TRACE(_T("SECImage::LoadDIB could not allocate image data space\n"));
		goto sec_dib_load_err;
	}

    pFile->Read(m_lpSrcBits,
			(DWORD) (pFile->GetLength() - lpbmfHdr->bfOffBits-cutCompress));

    if (lpbmfHdr)
		GlobalFreePtr(lpbmfHdr);
    return (TRUE);

// Error handling; free up any allocated memory and return error condition.
 sec_dib_load_err:
    if (lpbmfHdr)
		GlobalFreePtr(lpbmfHdr);
    if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
	m_lpBMI = NULL;
	}
    if (m_lpSrcBits){
	    GlobalFreePtr(m_lpSrcBits);
	m_lpSrcBits = NULL;
	}
    return (FALSE);
}

BOOL SECImage::SECLoadDibArchive(CArchive& ar)
{
    ASSERT(!ar.IsStoring());

	UINT uBufSize;
    LPBITMAPFILEHEADER lpbmfHdr = (LPBITMAPFILEHEADER)GlobalAllocPtr(GHND, sizeof(BITMAPFILEHEADER));
    if (!lpbmfHdr)
	{
		TRACE(_T("SECImage::SECLoadDib could not allocate BITMAPFILEHEADER space\n"));
		goto sec_dib_ar_load_err;
	}

    ar.Read(lpbmfHdr, sizeof(BITMAPFILEHEADER));
    if (lpbmfHdr->bfType != 0x4d42)
	{
	    return FALSE;
	}

    //AAB REALLOC BUG m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER));
	m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER) + 256*sizeof(RGBQUAD));
    if (!m_lpBMI)
	{
		TRACE(_T("SECImage::LoadDib could not allocate space for BITMAPINFO\n"));
		goto sec_dib_ar_load_err;
	}

    // Read in bitmap header info
    ar.Read(m_lpBMI, (UINT)(lpbmfHdr->bfOffBits-sizeof(BITMAPFILEHEADER)));
    // fill in some member data
    m_bIsPadded = TRUE; // DIB files already padded...
    m_dwWidth = (DWORD) m_lpBMI->bmiHeader.biWidth;
    m_dwPadWidth = PADWIDTH(m_dwWidth);
    m_dwHeight = (DWORD) m_lpBMI->bmiHeader.biHeight;

	m_nSrcBitsPerPixel = m_lpBMI->bmiHeader.biBitCount;
    switch(m_lpBMI->bmiHeader.biBitCount)
	{
	case 1:
	    m_wColors = 2;
	    break;
	case 4 :
	    m_wColors = 16;
	    break;
	case 8 :
	    m_wColors = 256;
	    break;
	default :
	    m_wColors = 0;
	}

    m_lpRGB=m_lpBMI->bmiColors;
	//AAB REALLOC BUG m_lpBMI = (LPBITMAPINFO)GlobalReAllocPtr(m_lpBMI, sizeof(BITMAPINFO) + m_wColors*sizeof(RGBQUAD),GMEM_MOVEABLE);

    // Now read in the actual bitmap data from the DIB
    if (m_lpSrcBits){
	GlobalFreePtr(m_lpSrcBits);
	m_lpSrcBits = NULL;
    }

	uBufSize=(UINT)((m_dwPadWidth*m_dwHeight*m_lpBMI->bmiHeader.biBitCount)/8);
    m_lpSrcBits = (LPBYTE)GlobalAllocPtr(GHND,uBufSize);

    if (!m_lpSrcBits)
	{
		TRACE(_T("SECImage::LoadDIB could not allocate image data space\n"));
		goto sec_dib_ar_load_err;
	}

#ifdef WIN32
    ar.Read(m_lpSrcBits,(DWORD) (uBufSize));
#else
    ar.Read(m_lpSrcBits,uBufSize);
#endif

    if (lpbmfHdr)
		GlobalFreePtr(lpbmfHdr);
    return (TRUE);

// Error handling; free up any allocated memory and return error condition.
 sec_dib_ar_load_err:
    if (lpbmfHdr)
		GlobalFreePtr(lpbmfHdr);
    if (m_lpBMI){
		GlobalFreePtr(m_lpBMI);
	m_lpBMI = NULL;
	}
    if (m_lpSrcBits){
	    GlobalFreePtr(m_lpSrcBits);
	m_lpSrcBits = NULL;
	}
    return (FALSE);
}

//
//	Serialize
//
//	Ok to implement here even though we're an abstract class;
//  we just don't use IMPLEMENT_SERIAL macro (see MS
//  Knowledge Base Q103983)
//
void SECImage::Serialize(CArchive& ar)
{
    // Write out base class info to archive
    CObject::Serialize(ar);

    BITMAPFILEHEADER bmfHdr;
    if (ar.IsStoring())
	{
	    // Make sure bitmap data is in padded format
	    PadBits();
	    bmfHdr.bfType = DIB_HEADER_MARKER;
	    // initialize to BitmapInfo size
	    DWORD dwImageSize= m_lpBMI->bmiHeader.biSize;
	    // Add in palette size
	    WORD wColors = NumColors();
	    WORD wPaletteSize = (WORD)(wColors*sizeof(RGBQUAD));
	    dwImageSize+=wPaletteSize;

	    // Add in size of actual bit array
	    dwImageSize+=PADWIDTH((m_dwWidth)*(DWORD)(m_lpBMI->bmiHeader.biBitCount/8)) * m_dwHeight;
	    m_lpBMI->bmiHeader.biSizeImage = 0;
	    bmfHdr.bfSize = dwImageSize + sizeof(BITMAPFILEHEADER);
	    bmfHdr.bfReserved1 = 0;
	    bmfHdr.bfReserved2 = 0;
	    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + m_lpBMI->bmiHeader.biSize + wPaletteSize;
	    ar.Write(&bmfHdr, sizeof(BITMAPFILEHEADER));
	    ar.Write(m_lpBMI, sizeof(BITMAPINFO) + (wColors-1)*sizeof(RGBQUAD));
		// ar.Write(m_lpSrcBits, (UINT)(m_dwPadWidth*m_dwHeight*(m_lpBMI->bmiHeader.biBitCount/8)));
		// Fixed serialization problem for 1 bpp and 4 bpp dibs
		ar.Write(m_lpSrcBits, (UINT)((m_dwPadWidth * m_dwHeight* m_lpBMI->bmiHeader.biBitCount) / 8));
	}
    else
	{
		SECLoadDibArchive(ar);
	}
}

//
//	AssertValid
//
//
//

#ifdef _DEBUG
void SECImage::AssertValid() const
{
    CObject::AssertValid();
	ASSERT(m_lpSrcBits!=0);
	ASSERT(m_dwWidth > 0);
	ASSERT(m_dwHeight > 0);
	ASSERT(m_lpBMI != 0);
}
#endif /* _DEBUG */




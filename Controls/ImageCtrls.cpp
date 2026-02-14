#include <malloc.h>
#include <memory>

#include "stdafx.h"
#include "AppDef.h"
#include "Utils\FileUtils.h"
#include "Utils\mutils.h"
#include "ImageCtrls.h"
#include "MGTools\Include\Utils\Utils.h"
#include "MGTools\Include\Image\SecPcx.h"
#include "MGTools\Include\Image\SecJpeg.h"
#include "MGTools\Include\Image\SecGif.h"
#include "MGTools\Include\Image\SecTarga.h"
#include "MGTools\Include\Image\SecTiff.h"
//====================================================================
CImageCtrls::CImageCtrls()
{
	Init();
}

void CImageCtrls::Init()
{
	for(int i=0; i < 256; i++){
	  m_rgbPalette[i].rgbBlue =m_rgbPalette[i].rgbGreen =m_rgbPalette[i].rgbRed = i;
	  m_rgbPalette[i].rgbReserved = 0;
	}
	m_pDIB = NULL;
	PrepareNewImage();
}

void CImageCtrls::PrepareNewImage()
{
    kContrast = 1.;
    kBright = 1.;

    if(m_pDIB)
		delete m_pDIB;
    m_pDIB = NULL;
    ImageSize.cx = 0;
    ImageSize.cy = 0;
}

CImageCtrls::~CImageCtrls()
{
    if (m_pDIB != NULL)
      delete m_pDIB;
	m_pDIB = NULL;
}

CImageCtrls& CImageCtrls::operator=(const CImageCtrls& rhs)
{
  if(this==&rhs) return *this;
    kContrast = rhs.kContrast;
    kBright = rhs.kBright;
    m_pDIB = rhs.m_pDIB;
    ImageSize = rhs.ImageSize;
    for(int i=0; i < 256; i++){
       m_rgbPalette[i] = rhs.m_rgbPalette[i];
    }

    return *this;
}

CRect CImageCtrls::GetDIBRect()
{
	CRect rcDIB;
	if(ImageSize.cx == 0 && ImageSize.cy == 0)
	  rcDIB = CRect(0,0,ImageSize.cx ,ImageSize.cy);
	else
	  rcDIB = CRect(0,0,ImageSize.cx,ImageSize.cy);
    return rcDIB;
}

BOOL CImageCtrls::ConvertToDIB(CString& name)
{
    fs::path inputPath((LPCTSTR)name);

    int type = FileType(inputPath.string());
    if (type < 0)
        return FALSE;

    // Allow only known image types
    if (type < T_BMP || type > T_TIF)
        return FALSE;

    // NOTE: should always create temporary .bmp, even if an original file is .bmp already
    // This temporary .bmp is used for undo logic and wil be erazed on file close
    // Build temporary .bmp output path
    char tmpPath[_MAX_PATH];
    GetTempPath(_MAX_PATH, tmpPath);
    fs::path outPath = fs::path(tmpPath) / inputPath.stem();
    outPath.replace_extension(".bmp");

    // Prepare DIB target
    auto dib = std::make_unique<SECDib>();
    BOOL res = FALSE;

    switch (type)
    {
    case T_BMP: {
        // If already BMP, do conversion to asure the format compatibility
        auto src = std::make_unique<SECDib>();
        if (src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    case T_PCX: {
        auto src = std::make_unique<SECPcx>();
        if(src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    case T_JPG: {
        auto src = std::make_unique<SECJpeg>();
        if (src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    case T_GIF: {
        auto src = std::make_unique<SECGif>();
        if (src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    case T_TGA: {
        auto src = std::make_unique<SECTarga>();
        if (src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    case T_TIF: {
        auto src = std::make_unique<SECTiff>();
        if (src->LoadImage(name) && dib->ConvertImage(src.get()))
            res = TRUE;
        break;
    }
    default:
        return FALSE;
    }

    if (!res)
        return FALSE;

    CFile file;
    if (!file.Open((LPCTSTR)outPath.string().c_str(), CFile::modeCreate | CFile::modeWrite))
        return FALSE;

    dib->SaveImage(&file);
    file.Close();

    name = outPath.c_str();
    return TRUE;
}

BOOL CImageCtrls::ConvertToGrayScale(CDC* pDC, LPCTSTR fname)
{
  int nBitCount = m_pDIB->m_nSrcBitsPerPixel;
  if(nBitCount == 8)
      return TRUE;

  BOOL bPrePadded = m_pDIB->m_bIsPadded;
      if(bPrePadded)
        m_pDIB->PubUnPadBits();

  int nPlanes = m_pDIB->m_nBitPlanes;
  DWORD Size = m_pDIB->m_dwWidth * m_pDIB->m_dwHeight * nBitCount/8;
  DWORD newSize = m_pDIB->m_dwWidth * m_pDIB->m_dwHeight;

  RGBQUAD rgbPix;
  LPBYTE lpPixel;
  LPBYTE lpDIBBits;
  lpDIBBits = (LPBYTE)malloc(newSize*sizeof(BYTE));

  LPBITMAPINFO lpbi;
    // Fill in the BITMAPINFOHEADER
    lpbi = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbi->bmiHeader.biWidth = m_pDIB->m_dwWidth;
    lpbi->bmiHeader.biHeight = m_pDIB->m_dwHeight;
    lpbi->bmiHeader.biPlanes = 1;
    lpbi->bmiHeader.biBitCount = 8;
    lpbi->bmiHeader.biCompression = BI_RGB;
    lpbi->bmiHeader.biSizeImage = newSize;
    lpbi->bmiHeader.biXPelsPerMeter = 0;
    lpbi->bmiHeader.biYPelsPerMeter = 0;
    lpbi->bmiHeader.biClrUsed = 0;
    lpbi->bmiHeader.biClrImportant = 0;

    // Fill in the color table
    UINT uUsage = DIB_RGB_COLORS;
    memcpy( lpbi->bmiColors, m_rgbPalette, sizeof(RGBQUAD) * 256 );

    HBITMAP hBitmap = CreateDIBSection(pDC->m_hDC, lpbi, uUsage, (void **)lpDIBBits, NULL, 0 );
    CBitmap* pBitmap = CBitmap::FromHandle(hBitmap);

    SECDib* pDib = new SECDib;
    pDib->CreateFromBitmap(pDC, pBitmap);

  int n = -1;
  unsigned char Pixel=0;
  if(nBitCount == 16){
    for(DWORD idx=0; idx < Size; idx +=2){
      lpPixel = m_pDIB->m_lpSrcBits + idx;
      rgbPix.rgbBlue = (BYTE)*lpPixel;
      rgbPix.rgbGreen = (BYTE)*(lpPixel+1);
      Pixel = __max(rgbPix.rgbBlue, rgbPix.rgbGreen);
//	  Pixel = (rgbPix.rgbBlue + rgbPix.rgbGreen)/2;
      ++n;
      lpDIBBits[n] = Pixel;
    }
  }
  if(nBitCount == 24){
    for(DWORD idx=0; idx < Size; idx +=3){
      lpPixel = m_pDIB->m_lpSrcBits + idx;
      rgbPix.rgbBlue = (BYTE)*lpPixel;
      rgbPix.rgbGreen = (BYTE)*(lpPixel+1);
      rgbPix.rgbRed = (BYTE)*(lpPixel+2);
//	  Pixel = (rgbPix.rgbBlue + rgbPix.rgbGreen + rgbPix.rgbRed)/3;
      Pixel = __max(rgbPix.rgbRed, rgbPix.rgbGreen);
      Pixel = __max(Pixel, rgbPix.rgbBlue);
      ++n;
      lpDIBBits[n] = Pixel;
    }
  }
  else{
      delete pDib;
      delete [] (BYTE *)lpbi;
      free(lpDIBBits);
      ::DeleteObject((HGDIOBJ)hBitmap);
      return FALSE;
  }

    memcpy(pDib->m_lpSrcBits, lpDIBBits, newSize);
	pDib->m_bIsPadded = 0;
    pDib->SaveImage(fname);
    delete pDib;
    delete [] (BYTE *)lpbi;
    free(lpDIBBits);
    ::DeleteObject((HGDIOBJ)hBitmap);

    if(m_pDIB)
		delete m_pDIB;
    m_pDIB = new SECDib;
    m_pDIB->LoadImage(fname);
    return TRUE;
}

bool CImageCtrls::LoadImage(CString& fname)
{
    //Note, could have done new SECDIB(filename) too?
	delete m_pDIB;

    if(!ConvertToDIB(fname))
          return false;
	
    m_pDIB = new SECDib;
    if (m_pDIB->LoadImage(LPCTSTR(fname)) == FALSE){
        delete m_pDIB;
		TCHAR buffer[256];
		_stprintf(buffer,_T("Imagetst could not load DIB/BMP file: %s!"), LPCTSTR(fname));
		AfxMessageBox(buffer,MB_ICONEXCLAMATION|MB_OK);
		return false;
    }

    ImageSize.cx = (int)m_pDIB->m_dwWidth;
    ImageSize.cy = (int)m_pDIB->m_dwHeight;

	return true;
}

void CImageCtrls::SaveImage(LPCTSTR fname)
{
	if(!m_pDIB)
		return;
	auto Type = FileType(fname);
	switch(Type){
	  case T_BMP: OnFileSaveAsDib(fname); break;
	  case T_PCX: OnFileSaveAsPcx(fname); break;
	  case T_JPG: OnFileSaveAsJpeg(fname); break;
	  case T_GIF: OnFileSaveAsGif(fname); break;
	  case T_TGA: OnFileSaveAsTga(fname); break;
	  case T_TIF: OnFileSaveAsTiff(fname); break;
	}
}

void CImageCtrls::OnFileSaveAsDib(LPCTSTR fname)
{
	if(fname){
			m_pDIB->SaveImage(fname);
	}
	else{
		CFileDialog dlg(FALSE, _T("dib"), _T("*.dib"));
		if (dlg.DoModal() == IDOK)
		{
			m_pDIB->SaveImage(dlg.GetPathName());
		}
	}
}

void CImageCtrls::OnFileSaveAsGif(LPCTSTR fname)
{
	if(fname){
			SECGif *pGif = new SECGif();
			if (!pGif)
				return;
			pGif->ConvertImage(m_pDIB);
			pGif->SaveImage(fname);
			m_pDIB->ConvertImage(pGif);
			delete pGif;
	}
	else{
		CFileDialog dlg(FALSE, _T("gif"), _T("*.gif"));
		if (dlg.DoModal() == IDOK)
		{
			SECGif *pGif = new SECGif();
			if (!pGif)
				return;
			pGif->ConvertImage(m_pDIB);
			pGif->SaveImage(dlg.GetPathName());
			m_pDIB->ConvertImage(pGif);
			delete pGif;
		}
	}
}

void CImageCtrls::OnFileSaveAsJpeg(LPCTSTR fname)
{
	if(fname){
			SECJpeg *pJpeg = new SECJpeg();
			if (!pJpeg)
				return;
			pJpeg->ConvertImage(m_pDIB);
			pJpeg->SaveImage(fname);
			m_pDIB->ConvertImage(pJpeg);
			delete pJpeg;
	}
	else{
		CFileDialog dlg(FALSE, _T("jpg"), _T("*.jpg"));
		if (dlg.DoModal() == IDOK)
		{
			SECJpeg *pJpeg = new SECJpeg();
			if (!pJpeg)
				return;
			pJpeg->ConvertImage(m_pDIB);
			pJpeg->SaveImage(dlg.GetPathName());
			m_pDIB->ConvertImage(pJpeg);
			delete pJpeg;
		}
	}
}


void CImageCtrls::OnFileSaveAsPcx(LPCTSTR fname)
{
	if(fname){
			SECPcx *pPcx = new SECPcx();
			if (!pPcx)
				return;
			pPcx->ConvertImage(m_pDIB);
			pPcx->SaveImage(fname);
			m_pDIB->ConvertImage(pPcx);
			delete pPcx;
	}
	else{
		CFileDialog dlg(FALSE, _T("pcx"), _T("*.pcx"));
		if (dlg.DoModal() == IDOK)
		{
			SECPcx *pPcx = new SECPcx();
			if (!pPcx)
				return;
			pPcx->ConvertImage(m_pDIB);
			pPcx->SaveImage(dlg.GetPathName());
			m_pDIB->ConvertImage(pPcx);
			delete pPcx;
		}
	}
}

void CImageCtrls::OnFileSaveAsTga(LPCTSTR fname)
{
	if(fname){
			SECTarga *pTga= new SECTarga();
			if (!pTga)
				return;
			pTga->ConvertImage(m_pDIB);
			pTga->SaveImage(fname);
			m_pDIB->ConvertImage(pTga);
			delete pTga;
	}
	else{
		CFileDialog dlg(FALSE, _T("tga"), _T("*.tga"));
		if (dlg.DoModal() == IDOK)
		{
			SECTarga *pTga= new SECTarga();
			if (!pTga)
				return;
			pTga->ConvertImage(m_pDIB);
			pTga->SaveImage(dlg.GetPathName());
			m_pDIB->ConvertImage(pTga);
			delete pTga;
		}
	}
}

void CImageCtrls::OnFileSaveAsTiff(LPCTSTR fname)
{
	if(fname){
			SECTiff *pTiff= new SECTiff();
			if (!pTiff)
				return;
			pTiff->ConvertImage(m_pDIB);
			pTiff->SaveImage(fname);
			m_pDIB->ConvertImage(pTiff);
			delete pTiff;
	}
	else{
		CFileDialog dlg(FALSE, _T("tif"), _T("*.tif"));
		if (dlg.DoModal() == IDOK)
		{
			SECTiff *pTiff= new SECTiff();
			if (!pTiff)
				return;
			pTiff->ConvertImage(m_pDIB);
			pTiff->SaveImage(dlg.GetPathName());
			m_pDIB->ConvertImage(pTiff);
			delete pTiff;
		}
	}
}

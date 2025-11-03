// SpecFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpecFileDlg.h"
#include "MGTools\Include\Graph\GraphTools.h"
#include "MGTools\Include\Utils\Utils.h"

#include "MGTools\Include\Image\SecDib.h"
#include "MGTools\Include\Image\SecPcx.h"
#include "MGTools\Include\Image\SecJpeg.h"
#include "MGTools\Include\Image\SecGif.h"
#include "MGTools\Include\Image\SecTarga.h"
#include "MGTools\Include\Image\SecTiff.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpecialFileDialog dialog

CSpecialFileDialog::CSpecialFileDialog(BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags,
		LPCTSTR lpszFilter, CWnd* pParentWnd)
	: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
		dwFlags, lpszFilter, pParentWnd, 0, FALSE)
{
	//{{AFX_DATA_INIT(CSpecialFileDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_ofn.Flags |= OFN_ENABLETEMPLATE;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILESPECIAL);
	m_ofn.lpstrTitle = "Delete File";
	m_bDeleteAll = FALSE;
}


void CSpecialFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpecialFileDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpecialFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CSpecialFileDialog)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpecialFileDialog message handlers

BOOL CSpecialFileDialog::OnInitDialog() 
{
	BOOL bRet = CFileDialog::OnInitDialog();
/*	if (bRet == TRUE) {
		GetParent()->GetDlgItem(IDOK)->SetWindowText("Delete");
	}*/
	 
	SetTimer(1,100,0);
	return bRet;
}

void CSpecialFileDialog::OnDelete() 
{
//	m_bDeleteAll = TRUE;
	// 0x480 is the child window ID of the File Name edit control
	//  (as determined by SPYXX)
//	GetParent()->GetDlgItem(0x480)->GetWindowText(m_strFilename);
//	GetParent()->SendMessage(WM_COMMAND, IDCANCEL);
}

void CSpecialFileDialog::OnFileNameChange()
{
	CString path = GetPathName();

	if(!path.IsEmpty() && oldFilename != path){	
		
		oldFilename = path;
		LoadPicture(path);
	}
}
afx_msg LRESULT CSpecialFileDialog::OnDelayedLoadPicture(WPARAM wParam, LPARAM lParam) 
{
	if (!m_pendingPicturePath.IsEmpty())
	{
		LoadPicture(m_pendingPicturePath);
	}
	return 0;
}
void CSpecialFileDialog::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == 1) {
		KillTimer(1);
		LoadPicture(m_pendingPicturePath);
	}
	CFileDialog::OnTimer(nIDEvent);
}

void CSpecialFileDialog::LoadPicture(LPCTSTR Filename)
{
	if(!IsFileExist(Filename, FALSE)){
		Invalidate(FALSE);
		return;
	}
	
	 CClientDC dc(this);
	 HWND hParent = ::GetParent(GetSafeHwnd());
	 if (!hParent) {
		 TRACE(_T("Родительское окно не найдено.\n"));
		 return;
	 }

	 HWND hCtl = ::GetDlgItem(hParent, IDBM_PREOPEN_PICTURE);
	 if (!hCtl) {
		 TRACE(_T("IDBM_PREOPEN_PICTURE не найден через ::GetDlgItem.\n"));
		 return;
	 }

	 CWnd* pW = CWnd::FromHandle(hCtl);
	 if (pW == nullptr)
	 {
		 TRACE(_T("Контрол IDBM_PREOPEN_PICTURE не найден.\n"));
		 return;
	 }
     CRect wR; pW->GetWindowRect(wR);
     ScreenToClient(wR);

	 CString s = Filename;
	 CString ext = s.Right(3);
	 ext.MakeUpper();

     int regX = wR.Width();
     int regY = wR.Height();
	 CRect reg(wR);
	 double scale=1;
	 double rangeX, rangeY;
	 if(ext == "BMP"){
       SECDib m_pDIB;
       if(m_pDIB.LoadImage(LPCTSTR(Filename))){
        rangeX = m_pDIB.m_dwPadWidth;
        rangeY = m_pDIB.m_dwHeight;
 		    CPalette* pOldPalette = dc.SelectPalette(m_pDIB.m_pPalette, TRUE);
			GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
			int sW = (reg.Width() - regX)/2;
			int sH = (reg.Height() - regY)/2;
			reg.left += sW;
			reg.top += sH;
			reg.right = reg.left + regX;
			reg.bottom = reg.top + regY;
			dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
		    m_pDIB.m_bUseHalftone = TRUE;
			m_pDIB.StretchDIBits(&dc,
				reg.left, reg.top, reg.Width(), reg.Height(),
				0, 0, m_pDIB.m_dwPadWidth, m_pDIB.m_dwHeight,
				m_pDIB.m_lpSrcBits,
				m_pDIB.m_lpBMI, DIB_RGB_COLORS,
				SRCCOPY);
			if(pOldPalette)
				dc.SelectPalette(pOldPalette, TRUE);
		}
	 }
	 else if(ext == "PCX"){
       SECPcx* m_pDIB = new SECPcx;
       if(m_pDIB->LoadImage(LPCTSTR(Filename))){
 		CPalette* pOldPalette = dc.SelectPalette(m_pDIB->m_pPalette, TRUE);
        rangeX = m_pDIB->m_dwPadWidth;
        rangeY = m_pDIB->m_dwHeight;
        GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
        dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
	    m_pDIB->m_bUseHalftone = TRUE;
		m_pDIB->StretchDIBits(&dc,
		   reg.left, reg.top, reg.Width(), reg.Height(),
		   0, 0, m_pDIB->m_dwPadWidth, m_pDIB->m_dwHeight,
		   m_pDIB->m_lpSrcBits,
 		   m_pDIB->m_lpBMI, DIB_RGB_COLORS,
		   SRCCOPY);
		if(pOldPalette)
 		  dc.SelectPalette(pOldPalette, TRUE);
  	   }
	   delete m_pDIB;
	 }
	 else if(ext == "JPG"){
       SECJpeg* m_pDIB = new SECJpeg;
       if(m_pDIB->LoadImage(LPCTSTR(Filename))){
 		CPalette* pOldPalette = dc.SelectPalette(m_pDIB->m_pPalette, TRUE);
        rangeX = m_pDIB->m_dwPadWidth;
        rangeY = m_pDIB->m_dwHeight;
        GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
        dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
	    m_pDIB->m_bUseHalftone = TRUE;
		m_pDIB->StretchDIBits(&dc,
		   reg.left, reg.top, reg.Width(), reg.Height(),
		   0, 0, m_pDIB->m_dwPadWidth, m_pDIB->m_dwHeight,
		   m_pDIB->m_lpSrcBits,
 		   m_pDIB->m_lpBMI, DIB_RGB_COLORS,
		   SRCCOPY);
		if(pOldPalette)
 		  dc.SelectPalette(pOldPalette, TRUE);
  	   }
	   delete m_pDIB;
	 }
	 else if(ext == "GIF"){
       SECGif* m_pDIB = new SECGif;
       if(m_pDIB->LoadImage(LPCTSTR(Filename))){
 		CPalette* pOldPalette = dc.SelectPalette(m_pDIB->m_pPalette, TRUE);
        rangeX = m_pDIB->m_dwPadWidth;
        rangeY = m_pDIB->m_dwHeight;
        GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
        dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
	    m_pDIB->m_bUseHalftone = TRUE;
		m_pDIB->StretchDIBits(&dc,
		   reg.left, reg.top, reg.Width(), reg.Height(),
		   0, 0, m_pDIB->m_dwPadWidth, m_pDIB->m_dwHeight,
		   m_pDIB->m_lpSrcBits,
 		   m_pDIB->m_lpBMI, DIB_RGB_COLORS,
		   SRCCOPY);
		if(pOldPalette)
 		  dc.SelectPalette(pOldPalette, TRUE);
  	   }
	   delete m_pDIB;
	 }
	 else if(ext == "TGA"){
       SECTarga* m_pDIB = new SECTarga;
       if(m_pDIB->LoadImage(LPCTSTR(Filename))){
 		CPalette* pOldPalette = dc.SelectPalette(m_pDIB->m_pPalette, TRUE);
        rangeX = m_pDIB->m_dwPadWidth;
        rangeY = m_pDIB->m_dwHeight;
        GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
        dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
	    m_pDIB->m_bUseHalftone = TRUE;
		m_pDIB->StretchDIBits(&dc,
		   reg.left, reg.top, reg.Width(), reg.Height(),
		   0, 0, m_pDIB->m_dwPadWidth, m_pDIB->m_dwHeight,
		   m_pDIB->m_lpSrcBits,
 		   m_pDIB->m_lpBMI, DIB_RGB_COLORS,
		   SRCCOPY);
		if(pOldPalette)
 		  dc.SelectPalette(pOldPalette, TRUE);
  	   }
	   delete m_pDIB;
	 }
	 else if(ext == "TIF"){
       SECTiff* m_pDIB = new SECTiff;
       if(m_pDIB->LoadImage(LPCTSTR(Filename))){
 		CPalette* pOldPalette = dc.SelectPalette(m_pDIB->m_pPalette, TRUE);
        rangeX = m_pDIB->m_dwPadWidth;
        rangeY = m_pDIB->m_dwHeight;
        GetDCRegion(&dc, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
        dc.FillRect(&wR, &CBrush(RGB(0,0,0)));
	    m_pDIB->m_bUseHalftone = TRUE;
		m_pDIB->StretchDIBits(&dc,
		   reg.left, reg.top, reg.Width(), reg.Height(),
		   0, 0, m_pDIB->m_dwPadWidth, m_pDIB->m_dwHeight,
		   m_pDIB->m_lpSrcBits,
 		   m_pDIB->m_lpBMI, DIB_RGB_COLORS,
		   SRCCOPY);
		if(pOldPalette)
 		  dc.SelectPalette(pOldPalette, TRUE);
  	   }
	   delete m_pDIB;
	 }
}

// ZoomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ZoomDlg.h"

#include "Utils\mutils.h"
#include "ImageTempl\ImageDoc.h"
#include "ImageTempl\ImageView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog


CZoomDlg::CZoomDlg(CWnd* pParent /*=NULL*/)
	: CMDialog(CZoomDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZoomDlg)
	//}}AFX_DATA_INIT
}


void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CMDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZoomDlg)
	DDX_Control(pDX, IDCB_zoom_scale, ZoomScale);
	DDX_Control(pDX, IDS_zoom_scale, ZoomScaleCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CZoomDlg, CMDialog)
	//{{AFX_MSG_MAP(CZoomDlg)
	ON_CBN_SELCHANGE(IDCB_zoom_scale, OnSelchangeZoomScale)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
//	ON_BN_CLICKED(IDUNDO, OnUndo)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDS_zoom_scale, OnReleasedcapturezoomscale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg message handlers
void CZoomDlg::OnClose() 
{
	CMDialog::OnClose();
}

void CZoomDlg::OnOK() 
{
    CImageCtrls* pImCtrls = GetImageCtrls();
	pImCtrls->undokZoom = InkZoom;
	CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
	CString s;
    s.Format("%g", pImCtrls->kZoom);
	pDoc->AddRecordScenario(O_ZOOM, LPCTSTR(s));
	pDoc->LastOperationType = O_ZOOM;
    pImCtrls->m_pDIB->SaveImage(pDoc->GetTmpPath());
	pDoc->ReloadDocument();
	
	CMDialog::OnOK();
}

void CZoomDlg::OnCancel() 
{
	OnUndo();
	CMDialog::OnCancel();
}

void CZoomDlg::OnUndo()
{
    CImageCtrls* pImCtrls = GetImageCtrls();
	CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
	pImCtrls->kZoom = InkZoom;
	pDoc->LastOperationType = O_NO_UNDO;
    pDoc->GetView()->Invalidate(FALSE);
}

void CZoomDlg::ResetZoomScale()
{
    int iSel = ZoomScaleCtrl.GetPos();
    CString s;
    ZoomScale.GetLBText(iSel, s);
    ZoomScale.SetCurSel(iSel);
    OnSelchangeZoomScale();
}

void CZoomDlg::OnSelchangeZoomScale() 
{
    int iSel = ZoomScale.GetCurSel();
    CString s;
    ZoomScale.GetLBText(iSel, s);
    ZoomScaleCtrl.SetPos(iSel);
    double zs = atoi(LPCTSTR(s))/100.;

    CImageCtrls* pImCtrls = GetImageCtrls();
	CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
    pImCtrls->kZoom = zs;
    pDoc->GetView()->Invalidate(FALSE);
}

BOOL CZoomDlg::OnInitDialog() 
{
	CMDialog::OnInitDialog();
    CImageCtrls* pImCtrls = GetImageCtrls();
	CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
	
    int zs = (int)(pImCtrls->kZoom * 100);
    CString s;
    s.Format("%d", zs);
    int iSel = ZoomScale.FindStringExact(-1, LPCTSTR(s));
    ZoomScale.SetCurSel(iSel);

    ZoomScaleCtrl.SetRange(0, 25);
    ZoomScaleCtrl.SetPos(iSel);

	InkZoom = pImCtrls->kZoom;
    pDoc->PrepareOperationUndo(O_ZOOM);
	return TRUE;
}

void CZoomDlg::ApplyNow(UINT nIDEvent)
{
    if(UpdateData(TRUE)){
      ResetZoomScale();
      CImageCtrls* pImCtrls = GetImageCtrls();
	  CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
      pDoc->GetView()->Invalidate(FALSE);
    }
}

void CZoomDlg::OnTimer(UINT nIDEvent) 
{
    KillTimer(nIDEvent);
    ApplyNow(nIDEvent);
	CMDialog::OnTimer(nIDEvent);
}

void CZoomDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
 if(pScrollBar){
  UINT iD = pScrollBar->GetDlgCtrlID();
  if(iD == IDS_zoom_scale){
        SetTimer(1, 0, 0);
  }
 }

 CMDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CZoomDlg::OnReleasedcapturezoomscale(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

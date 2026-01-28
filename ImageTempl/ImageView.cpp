// ImageView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AppDef.h"
#include "Utils\mutils.h"
#include "BaseImageView.h"
#include "ImageDoc.h"
#include "ImageView.h"
#include "ImageFeatures\SectionFrame.h"

#include "MGTools\Include\Utils\Utils.h"

#include "ImageFeatures\ZoomDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CImageView *gr = NULL;
void Polyline(int pn, ISO_POINT *plist, double level, int ilevel)
{
  gr->SingleIsoline(pn, plist, level, ilevel);
}

/////////////////////////////////////////////////////////////////////////////
// CImageView

IMPLEMENT_DYNCREATE(CImageView, CBaseImageView)

CImageView::CImageView()
{
}

CImageView::~CImageView()
{
//    ::DeleteObject(HGDIOBJ(bkColorBrush));
}

bool CImageView::GetXPixelLine(CPoint d_P, double*& pR, double*& pF, int& nP)
{
    CImageCtrls* pImCtrls = GetImageCtrls(this);
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CPoint l_P(d_P);
	ClientToDoc(l_P);

    int iy;
	int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
	int yDIB = pImCtrls->m_pDIB->m_dwHeight;
	unsigned char* lpDIBBits = (unsigned char*)pImCtrls->m_pDIB->m_lpSrcBits;
    CRect regDIB(pImCtrls->GetDIBRect());
	CRect d_regDIB(regDIB);
	DocToClient(d_regDIB);

	iy = d_P.y;
	for(int ix=0; ix < nP; ix++){
       CPoint d_curP(ix, iy);
	   if(d_regDIB.PtInRect(d_curP)){
		  CPoint l_curP(d_curP);
		  ClientToDoc(l_curP);
          CDPoint dP(l_curP);
	      int iix = (int)dP.x;
	      int iiy = yDIB - (int)dP.y;
		  if(iix > -1 && iiy > -1 && iix < xDIB && iiy < yDIB)
            pF[ix] = (double)((int)lpDIBBits[xDIB*iiy+iix]/255.);
		  else
            pF[ix] = 0.;
		}
		else{
          pF[ix] = 0.;
		}
          pR[ix] = ix;
	   }

    return true;
}

bool CImageView::GetYPixelLine(CPoint d_P, double*& pR, double*& pF, int& nP)
{
    CImageCtrls* pImCtrls = GetImageCtrls(this);
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CPoint l_P(d_P);
	ClientToDoc(l_P);

    int ix;
	int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
	int yDIB = pImCtrls->m_pDIB->m_dwHeight;
	unsigned char* lpDIBBits = (unsigned char*)pImCtrls->m_pDIB->m_lpSrcBits;
    CRect regDIB(pImCtrls->GetDIBRect());
	CRect d_regDIB(regDIB);
	DocToClient(d_regDIB);

	int iScrollWidth = 0;
    if(GetStyle() & WS_HSCROLL)
      iScrollWidth = 15;

	ix = d_P.x;
	for(int iy=nP; iy > 0; iy--){
       CPoint d_curP(ix, iy);
	   if(d_regDIB.PtInRect(d_curP)){
		  CPoint l_curP(d_curP);
		  ClientToDoc(l_curP);
          CDPoint dP(l_curP);
	      int iix = (int)dP.x;
	      int iiy = yDIB - (int)dP.y;
		  if(iix > -1 && iiy > -1 && iix < xDIB && iiy < yDIB)
            pF[nP-iy] = (double)((int)lpDIBBits[xDIB*iiy+iix]/255.);
		  else
            pF[nP-iy] = 0.;
		}
		else{
          pF[nP-iy] = 0.;
		}
          pR[nP-iy] = nP-iy;
	   }
    return true;
}

BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
	//{{AFX_MSG_MAP(CImageView)
	ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
    ON_COMMAND(IDD_MEASURE, OnMeasure)
    ON_UPDATE_COMMAND_UI(IDD_MEASURE, OnUpdateMeasure)
    ON_COMMAND(IDD_FOTO_SECTIONS, OnFotoSections)
    ON_UPDATE_COMMAND_UI(IDD_FOTO_SECTIONS, OnUpdateFotoSections)
    ON_COMMAND(IDD_BOUNDS_EXT, OnExtBounds)
    ON_UPDATE_COMMAND_UI(IDD_BOUNDS_EXT, OnUpdateExtBounds)
    ON_COMMAND(IDD_BOUNDS_INS, OnInsBounds)
    ON_UPDATE_COMMAND_UI(IDD_BOUNDS_INS, OnUpdateInsBounds)
    ON_COMMAND(IDD_ZOOM_IMAGE, OnZoom)
    ON_UPDATE_COMMAND_UI(IDD_ZOOM_IMAGE, OnUpdateZoom)
    ON_COMMAND(IDD_UNDO, OnUndo)
    ON_UPDATE_COMMAND_UI(IDD_UNDO, OnUpdateUndo)
    ON_COMMAND(IDD_UNDO, OnUndo)
    ON_UPDATE_COMMAND_UI(IDD_UNDO, OnUpdateUndo)
    ON_COMMAND(IDD_AUTO_D, OnAutoDigit)
    ON_UPDATE_COMMAND_UI(IDD_AUTO_D, OnUpdateAutoDigit)
	ON_COMMAND(IDD_FC_MAX, OnFCMax)
    ON_UPDATE_COMMAND_UI(IDD_FC_MAX, OnUpdateFCMax)
	ON_COMMAND(IDD_FC_MIN, OnFCMin)
    ON_UPDATE_COMMAND_UI(IDD_FC_MIN, OnUpdateFCMin)
	ON_COMMAND(IDD_FC_MINMAX, OnFCMinMax)
    ON_UPDATE_COMMAND_UI(IDD_FC_MINMAX, OnUpdateFCMinMax)
	ON_COMMAND(IDD_CLEAR_D, OnClearDigit)
    ON_UPDATE_COMMAND_UI(IDD_AUTO_D, OnUpdateClearDigit)
	ON_COMMAND(IDD_CALC_APROX, OnCalcAproximation)
    ON_UPDATE_COMMAND_UI(IDD_CALC_APROX, OnUpdateCalcAproximation)
	ON_COMMAND(IDD_ADD_DOT_D, OnAddDot)
	ON_UPDATE_COMMAND_UI(IDD_ADD_DOT_D, OnUpdateAddDot)
	ON_COMMAND(IDD_DEL_DOT_D, OnRemoveDot)
	ON_UPDATE_COMMAND_UI(IDD_DEL_DOT_D, OnUpdateRemoveDot)
	ON_COMMAND(IDD_DEL_FR_D, OnRemoveFringe)
	ON_UPDATE_COMMAND_UI(IDD_DEL_FR_D, OnUpdateRemoveFringe)
	ON_COMMAND(IDD_ADD_SEC_D, OnAddZAPSection)
    ON_UPDATE_COMMAND_UI(IDD_ADD_SEC_D, OnUpdateAddSection)
	ON_COMMAND(IDD_DEL_SEC_D, OnDelZAPSection)
    ON_UPDATE_COMMAND_UI(IDD_DEL_SEC_D, OnUpdateDelSection)
	ON_COMMAND(IDD_SEC_LEFT_D, OnShiftDotLeft)
	ON_UPDATE_COMMAND_UI(IDD_SEC_LEFT_D, OnUpdateShiftDotLeft)
	ON_COMMAND(IDD_SEC_RIGHT_D, OnShiftDotRight)
	ON_UPDATE_COMMAND_UI(IDD_SEC_RIGHT_D, OnUpdateShiftDotRight)
	ON_COMMAND(IDD_RENUM_FR_D, OnRenumFringe)
	ON_UPDATE_COMMAND_UI(IDD_RENUM_FR_D, OnUpdateRenumFringe)
	ON_COMMAND(IDD_RENUM_DOT_D, OnRenumDot)
	ON_UPDATE_COMMAND_UI(IDD_RENUM_DOT_D, OnUpdateRenumDot)
	ON_COMMAND(IDD_NUM_OFF_MINUS, OnNumberMinus)
	ON_UPDATE_COMMAND_UI(IDD_NUM_OFF_MINUS, OnUpdateNumberMinus)
	ON_COMMAND(IDD_NUM_OFF_PLUS, OnNumberPlus)
	ON_UPDATE_COMMAND_UI(IDD_NUM_OFF_PLUS, OnUpdateNumberPlus)
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_SETCURSOR()
    ON_WM_SIZE()
	ON_WM_MOVE()
    ON_WM_KILLFOCUS()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageView drawing
BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= CS_BYTEALIGNCLIENT;
/*    bkColorBrush = CreateSolidBrush(RGB(0,0,0));
    cs.lpszClass = AfxRegisterWndClass(CS_OWNDC|CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
      ::LoadCursor(NULL, IDC_ARROW), bkColorBrush, NULL);
*/	
	return CBaseImageView::PreCreateWindow(cs);
}

int CImageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseImageView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CImageView::Init()
{
  CursorPos = CPoint(-1,-1);
  MeasureLine = CRect(0,0,0,0);
  m_bLinning = FALSE;
  m_Captured = FALSE;
  CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
  pMCtrls->Init();
}

void CImageView::OnInitialUpdate()
{
	CBaseImageView::OnInitialUpdate();

   CursorPos = CPoint(-1, -1);
   CImageCtrls* pImage = GetImageCtrls(this);
   CControls* pCtrls = GetControls();
   
   CRect wDIBRect = pImage->GetDIBRect();
   CFrameWnd* pFr = GetParentFrame();
   WINDOWPLACEMENT wp;
   pFr->GetWindowPlacement(&wp);
   pFr->CalcWindowRect(&wDIBRect, CWnd::adjustOutside);
   int W = wDIBRect.Width()+20;
   int H = wDIBRect.Height()+20;
   CDC* pDC = GetDC();
   int scr_W = pDC->GetDeviceCaps(HORZRES);
   int scr_H = pDC->GetDeviceCaps(VERTRES);
   BOOL needFit=FALSE;
   if(W > std::lround(scr_W*0.65)){
      W = std::lround(scr_W*0.65);
	  needFit=TRUE;
   }
   if(H > std::lround(scr_H*0.65)){
      H = std::lround(scr_H*0.65);
	  needFit=TRUE;
   }
   pFr->MoveWindow(wp.rcNormalPosition.left , wp.rcNormalPosition.top, W, H);
   if(needFit){
	   OnZoomFit();
   }
}

void CImageView::OnDraw(CDC* pDC)
{
    CControls* pCtrls = GetControls();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();

    CDC dc;
    CDC* pDrawDC = pDC;
    CBitmap bitmap;
    CBitmap* pOldBitmap;

    CRect client, rectClip;
    pDC->GetClipBox(&client);
    rectClip = client;
    pDC->LPtoDP(&rectClip);
    rectClip.InflateRect(1, 1); // avoid rounding to nothing

    if (dc.CreateCompatibleDC(pDC)){
            if (bitmap.CreateCompatibleBitmap(pDC, rectClip.Width(), rectClip.Height()))
            {
                OnPrepareDC(&dc, NULL);
                pDrawDC = &dc;
                // offset origin more because bitmap is just piece of the whole drawing
                dc.OffsetViewportOrg(-rectClip.left, -rectClip.top);
                pOldBitmap = dc.SelectObject(&bitmap);
                dc.SetBrushOrg(rectClip.left % 8, rectClip.top % 8);
                // might as well clip to the same rectangle
                dc.IntersectClipRect(client);
            }
        }
	
    DrawBackGround(pDrawDC);
    DrawImage(pDrawDC);
    DrawMeasureLine(pDrawDC);
    if(pCtrls->EnableCustomDots){
       DrawCustomDots(pDrawDC);
       DrawCurBound(pDrawDC);
    }
    else if(pDoc->Tracker.GetEnableState()){
       pDoc->Tracker.DrawTracker(pDrawDC);
       DrawCurBound(pDrawDC);
    }
    if(pDoc->IsFotoSections()){
	    DrawCrossedLines(pDrawDC);
		pDoc->ReSetSections(CursorPos);
	}
    DrawBounds(pDrawDC);
	DrawDigitInfo(pDrawDC);
	DrawAproximation(pDrawDC);
	
    if (pDrawDC != pDC){
        pDC->SetViewportOrg(0, 0);
        pDC->SetWindowOrg(0,0);
        pDC->SetMapMode(MM_TEXT);
        dc.SetViewportOrg(0, 0);
        dc.SetWindowOrg(0,0);
        dc.SetMapMode(MM_TEXT);
        pDC->BitBlt(rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
            &dc, 0, 0, SRCCOPY);
        dc.SelectObject(pOldBitmap);
    }
	if(pDoc)
    	pDoc->SetZoomToTitle();
}

void CImageView::DrawDigitInfo(CDC* pDC)
{
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, pDoc);
	pDoc->Digit.Draw(pDC, DotSide);
}

void CImageView::DrawAproximation(CDC* pDC)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsAproximation() && (pCtrls->ViewState & V_APPROXIMATION)){
	  gr = this;
	  hDC = pDC->GetSafeHdc();
	  if (pDoc->nLevel > 0)
		  pDoc->pLevel[pDoc->nLevel - 1] *= 0.999999999;
	  
	  ApproxContour(pDoc->pMatr, pDoc->nx, pDoc->ny, pDoc->pY, pDoc->pX, pDoc->nLevel, pDoc->pLevel, Polyline);
  }
}
/////////////////////////////////////////////////////////////////////////////
// CImageView diagnostics

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CBaseImageView::AssertValid();
}

void CImageView::Dump(CDumpContext& dc) const
{
	CBaseImageView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImageView message handlers
void CImageView::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
}

void CImageView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
  CControls* pCtrls = GetControls();
  CMainFrame* pFR = GetMainFrame();
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  
  if(bActivate){
	  Invalidate(FALSE);
	  pDoc->OnActivate();
	  pDoc->SetTextWndInfo();
  	  pFR->SetCurrentNumber(pDoc->Digit.CurrentNumber);
	  pFR->SetImageInfo(pDoc->Digit.Comments,pDoc->Digit.ScaleFactor,pDoc->Digit.Rotation);
  }
  else{
	  pFR->GetImageInfo(pDoc->Digit.Comments,pDoc->Digit.ScaleFactor,pDoc->Digit.Rotation);
  }
  CBaseImageView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CImageView::OnMeasure()
{
    CMainFrame* pMFr = GetMainFrame();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    CControls* pCtrls = GetControls();
    if(pCtrls->EnableOptions & I_MEASURELINE){
       DeActivateMode(I_MEASURELINE);	
	   pMFr->ShowMeasurePane(FALSE);
    }
    else{
	   pCtrls->ActiveEditMode = -1;
       DeActivateMode(I_FOTO_SECTIONS);	
       DeActivateMode(I_BOUNDS_EXT);	
       DeActivateMode(I_BOUNDS_INS);	
       pDoc->ActivateMeasure(TRUE);
	   pMFr->ShowMeasurePane(TRUE);
    }
}

void CImageView::OnUpdateMeasure(CCmdUI* pCmdUI)
{
    CControls* pCtrls = GetControls();
    pCmdUI->SetRadio(pCtrls->EnableOptions & I_MEASURELINE);
}

void CImageView::OnFotoSections()
{
  CMainFrame* pMFr = GetMainFrame();
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();
  if(!pDoc->IsFotoSections()){
    pCtrls->ActiveEditMode = -1;
    pDoc->ActivateFotoSections(TRUE);
    DeActivateMode(I_MEASURELINE);
    DeActivateMode(I_BOUNDS_EXT);
    DeActivateMode(I_BOUNDS_INS);
    pMFr->ShowMeasurePane(FALSE);
  }
}

void CImageView::OnUpdateFotoSections(CCmdUI* pCmdUI)
{
}

void CImageView::OnExtBounds()
{
    CMainFrame* pMFr = GetMainFrame();
    CControls* pCtrls = GetControls();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	
	if(pCtrls->EnableOptions & I_BOUNDS_EXT){
	   DeActivateMode(I_BOUNDS_EXT);	
	}
	else{
	   pCtrls->ActiveEditMode = -1;
	   pDoc->ActivateExtBounds(TRUE);
	   DeActivateMode(I_MEASURELINE);	
	   DeActivateMode(I_FOTO_SECTIONS);	
	   pMFr->ShowMeasurePane(FALSE);
	}
}

void CImageView::OnUpdateExtBounds(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  CDigitInfo* pD = GetDigitInfo(this);
  
  pCmdUI->SetRadio(pCtrls->EnableOptions & I_BOUNDS_EXT);
  if(pD->IsDigiting())
	  pCmdUI->Enable(FALSE);
  else
	  pCmdUI->Enable(TRUE);
}

void CImageView::OnInsBounds()
{
    CMainFrame* pMFr = GetMainFrame();
    CControls* pCtrls = GetControls();
    CBoundCtrls* pBCtrls = GetBoundCtrls();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	
	if(pCtrls->EnableOptions & I_BOUNDS_INS){
	   DeActivateMode(I_BOUNDS_INS);	
	}
	else{
	   pCtrls->ActiveEditMode = -1;
	   pDoc->ActivateInsBounds(TRUE);
	   DeActivateMode(I_MEASURELINE);	
	   DeActivateMode(I_FOTO_SECTIONS);	
	   pMFr->ShowMeasurePane(FALSE);
	}
}

void CImageView::OnUpdateInsBounds(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  CDigitInfo* pD = GetDigitInfo(this);
  CBoundCtrls* pB = GetBoundCtrls(this);

  if(pB->ExtBoundType != -1 && (pCtrls->EnableOptions & I_BOUNDS_INS))
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);

  if(pD->IsDigiting() || pB->ExtBoundType == -1)
	  pCmdUI->Enable(FALSE);
  else
	  pCmdUI->Enable(TRUE);
}

void CImageView::OnZoom()
{
    CZoomDlg D(GetParentFrame());
    D.SetNameINI(GetIniFile());
    D.SetWndCap("Zoom");
    D.DoModal();
}

void CImageView::OnUpdateZoom(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
}

BOOL CImageView::OnEraseBkgnd(CDC* pDC) 
{
    return CBaseImageView::OnEraseBkgnd(pDC);
}

void CImageView::BeginLine(CPoint P)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CString s = pDoc->GetPathName();
  CImageCtrls* pImCtrls = GetImageCtrls(this);
  CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
  CControls* pCtrls = GetControls();
  
    int step = pCtrls->MarkerSide;
    CPoint cP(P);
    CRect rP(cP.x-step, cP.y-step, cP.x+step, cP.y+step);

    CPoint P1(MeasureLine.left, MeasureLine.top);
    CPoint P2(MeasureLine.right, MeasureLine.bottom);

    PisActive = 0;
    if(rP.PtInRect(P1)){
      PisActive = 1;
	  DocToClient(P1);
	  ClientToScreen(&P1);
	  SetCursorPos(P1.x, P1.y);
    }   
    else if(rP.PtInRect(P2)){
      PisActive = 2;
	  DocToClient(P2);
	  ClientToScreen(&P2);
	  SetCursorPos(P2.x, P2.y);
    }
    else{
	  pCtrls->EnableOptions &= ~I_MEASURE_ACTIVE;	
      Invalidate(FALSE);
	  MakeLoopMessage();
	  
      MeasureLine.left = P.x;
      MeasureLine.top = P.y;
      MeasureLine.right = P.x;
      MeasureLine.bottom = P.y;
      PisActive = 2;
    }
    m_bLinning = TRUE;
    pCtrls->EnableOptions |= I_MEASURE_ACTIVE;
    pCtrls->EnableOptions |= I_MEASURE_DRAW;
}

void CImageView::EndLine(CPoint P2)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CString s = pDoc->GetPathName();
  CImageCtrls* pImCtrls = GetImageCtrls(this);
  CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
   
    if(PisActive==2){
      MeasureLine.right   = P2.x;
      MeasureLine.bottom  = P2.y;
    }
    else if(PisActive==1){
      MeasureLine.left   = P2.x;
      MeasureLine.top  = P2.y;
    }
    m_bLinning = FALSE;

    CPoint TL = MeasureLine.TopLeft();
    CPoint BR = MeasureLine.BottomRight();
    CRect rcDIB(pImCtrls->GetDIBRect());
    CRect rcDest(pImCtrls->GetDIBRect());
    pMCtrls->SetFirstPoint(rcDest, rcDIB, TL);
    pMCtrls->SetSecondPoint(rcDest, rcDIB, BR);
    pDoc->WriteMeasureCtrls();
}

void CImageView::DrawMouseMoveMeasureLine(CPoint P2)
{
   if(GetCapture() != this)
      return;

   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CImageCtrls* pImCtrls = GetImageCtrls(this);
   CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);

      COLORREF Color = InvColor;
      CPen penLine;
      penLine.CreatePen(PS_COSMETIC, 0, Color);

      CClientDC dc(this);
      OnPrepareDC(&dc);
      CPen *pOld = dc.SelectObject(&penLine);
      int orop = dc.SetROP2(R2_XORPEN);

      dc.MoveTo(MeasureLine.left,  MeasureLine.top);
      dc.LineTo(MeasureLine.right, MeasureLine.bottom);
      DrawMarker(&dc, CPoint(MeasureLine.left,  MeasureLine.top));
      DrawMarker(&dc, CPoint(MeasureLine.right, MeasureLine.bottom));

      if(PisActive==2){
        MeasureLine.right   = P2.x;
        MeasureLine.bottom  = P2.y;
      }
      else if(PisActive==1){
        MeasureLine.left   = P2.x;
        MeasureLine.top  = P2.y;
      }

      if(PisActive==1 || PisActive==2){
        dc.MoveTo(MeasureLine.left, MeasureLine.top);
        dc.LineTo(MeasureLine.right, MeasureLine.bottom);
        DrawMarker(&dc, CPoint(MeasureLine.left,  MeasureLine.top));
        DrawMarker(&dc, CPoint(MeasureLine.right, MeasureLine.bottom));
      }

      CPoint TL = MeasureLine.TopLeft();
      CPoint BR = MeasureLine.BottomRight();
      CRect rcDIB(pImCtrls->GetDIBRect());
      CRect rcDest(pImCtrls->GetDIBRect());
      pMCtrls->SetFirstPoint(rcDest, rcDIB, TL);
      pMCtrls->SetSecondPoint(rcDest, rcDIB, BR);
      pDoc->WriteMeasureCtrls();

      dc.SetROP2(orop);
      CPen* pRetPen = dc.SelectObject(pOld);
      if(pRetPen) pRetPen->DeleteObject();
}

void CImageView::DrawMeasureLine(CDC* pDC)
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CImageCtrls* pImCtrls = GetImageCtrls(this);
   CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
   CControls* pCtrls = GetControls();
   
  if(pCtrls->EnableOptions & I_MEASURE_ACTIVE){
    COLORREF Color = InvColor;
    CPen* open = NULL;
    CPen pen0;
    pen0.CreatePen(PS_COSMETIC, 0, RGB(0,0,0));
        open = pDC->SelectObject(&pen0);

    CRect rDIB(pImCtrls->GetDIBRect());
    CRect rcDest = (pImCtrls->GetDIBRect());

    int orop = pDC->SetROP2(R2_XORPEN);
    CPen pen1;
    pen1.CreatePen(PS_COSMETIC, 0, Color);
    CPen* pPen = pDC->SelectObject(&pen1);
    pPen->DeleteObject();

    CPoint cP1;
    CDPoint dP(pMCtrls->L.P1);
	cP1.x = std::lround(dP.x);
	cP1.y = std::lround(dP.y);
    DrawMarker(pDC, cP1);
    
    CPoint cP2;
    dP = pMCtrls->L.P2;
	cP2.x = std::lround(dP.x);
	cP2.y = std::lround(dP.y);
    DrawMarker(pDC, cP2);

    CPen pen2;
    pen2.CreatePen(PS_COSMETIC, 0, Color);
    pPen = pDC->SelectObject(&pen2);
    pPen->DeleteObject();

    pDC->MoveTo(cP1.x , cP1.y);
    pDC->LineTo(cP2.x , cP2.y);

    pDC->SetROP2(orop);
    if(open){
      CPen* pRetPen = pDC->SelectObject(open);
      if(pRetPen) pRetPen->DeleteObject();
    }
  } 
}

void CImageView::DrawCrossedLines(CDC* pDC)
{
    CPen pen;
    pen.CreatePen(PS_DOT, 0, InvColor);
    CPen* open = pDC->SelectObject(&pen);
    int orop = pDC->SetROP2(R2_XORPEN);
    
    CRect clientR;
    GetClientRect(clientR);
	ClientToDoc(clientR);
    CPoint P1, P2;

    if(CursorPos != CPoint(-1,-1)){
        P1.x = clientR.left; P1.y = CursorPos.y;
        P2.x = clientR.right; P2.y = CursorPos.y;
        pDC->MoveTo(P1);
        pDC->LineTo(P2);
        P1.x = CursorPos.x; P1.y = clientR.top;
        P2.x = CursorPos.x; P2.y = clientR.bottom;
        pDC->MoveTo(P1);
        pDC->LineTo(P2);
	}
	
    pDC->SetROP2(orop);
    CPen* retPen = pDC->SelectObject(open);
    if(retPen)
        retPen->DeleteObject();
}

void CImageView::DrawMouseMoveCrossedLines(CPoint P)
{
    CClientDC dc(this);
    OnPrepareDC(&dc);
    CPen pen;
    pen.CreatePen(PS_DOT, 0, InvColor);
    CPen* open = dc.SelectObject(&pen);
    int orop = dc.SetROP2(R2_XORPEN);
    
    CRect clientR;
    GetClientRect(clientR);
	ClientToDoc(clientR);
    CPoint P1, P2;

    if(CursorPos != CPoint(-1,-1)){
        P1.x = clientR.left; P1.y = CursorPos.y;
        P2.x = clientR.right; P2.y = CursorPos.y;
        dc.MoveTo(P1);
        dc.LineTo(P2);
        P1.x = CursorPos.x; P1.y = clientR.top;
        P2.x = CursorPos.x; P2.y = clientR.bottom;
        dc.MoveTo(P1);
        dc.LineTo(P2);
    }

		CursorPos = P;
		P1.x = clientR.left; P1.y = CursorPos.y;
		P2.x = clientR.right; P2.y = CursorPos.y;
		dc.MoveTo(P1);
		dc.LineTo(P2);
		P1.x = CursorPos.x; P1.y = clientR.top;
		P2.x = CursorPos.x; P2.y = clientR.bottom;
		dc.MoveTo(P1);
		dc.LineTo(P2);
		dc.SetROP2(orop);

	CPen* retPen = dc.SelectObject(open);
    if(retPen)
        retPen->DeleteObject();
}

void CImageView::BeginDragDot(CPoint P)
{
	Invalidate(FALSE);
    SetCapture();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CPoint P1;
	pDoc->GetLockedDotPos(P1);
	m_Captured = TRUE;
	DragDot(P1);
}

void CImageView::DragDot(CPoint P, BOOL newPos)
{
  if(m_Captured){
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, pDoc);
	int DotSide12 = DotSide/2;
    CClientDC dc(this);
    OnPrepareDC(&dc);

    // Save DC state so selections and mapping are restored safely
    int nSave = dc.SaveDC();

    CPen pen;
    pen.CreatePen(PS_SOLID, 0, InvColor);
    CBrush br;
    br.CreateSolidBrush(InvColor);
    
    dc.SelectObject(&pen);
    dc.SelectObject(&br);

    int orop = dc.SetROP2(R2_XORPEN);
    
    CPoint P1;
	
	pDoc->GetLockedDotPos(P1);
    dc.Ellipse(P1.x - DotSide12, P1.y - DotSide12, P1.x + DotSide12, P1.y + DotSide12);

	if(pCtrls->ViewState & V_ZAPSECTIONS){
		P.y = P1.y;
	}
	
	pDoc->SetLockedDotPos(P);
	if(newPos){
		pDoc->GetLockedDotPos(P1);
        dc.Ellipse(P1.x - DotSide12, P1.y - DotSide12, P1.x + DotSide12, P1.y + DotSide12);
    }
    dc.SetROP2(orop);

    // restore the DC (reselects previous pen/brush, clip, mapping, etc.)
    dc.RestoreDC(nSave);
  }
}

void CImageView::DropDot(CPoint P)
{
	m_Captured = FALSE;
    InvalidateRect(NULL, FALSE);
}

void CImageView::BeginDragZapSection(CPoint P)
{
//	Invalidate(FALSE);
    SetCapture();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CPoint P1, P2;
	pDoc->GetLockedZapSectionXYPos(P1, P2);
	m_Captured = TRUE;
	DragZapSection(P1);
}

void CImageView::DragZapSection(CPoint P, BOOL newPos/*TRUE*/)
{
  if(m_Captured){
    CClientDC dc(this);
    OnPrepareDC(&dc);
    CPen pen;
    pen.CreatePen(PS_SOLID, 0, InvColor);
    CPen* open = dc.SelectObject(&pen);
    int orop = dc.SetROP2(R2_XORPEN);
	
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CPoint P1, P2;
	pDoc->GetLockedZapSectionXYPos(P1, P2);
 	dc.MoveTo(P1);
	dc.LineTo(P2);
	pDoc->SetLockedZapSectionYPos(P.y);
	if(newPos){
		pDoc->GetLockedZapSectionXYPos(P1, P2);
		dc.MoveTo(P1);
		dc.LineTo(P2);
	}
    dc.SetROP2(orop);
	CPen* retPen = dc.SelectObject(open);
    if(retPen)
        retPen->DeleteObject();
  }
}

void CImageView::DropZapSection(CPoint P)
{
	m_Captured = FALSE;
    InvalidateRect(NULL, FALSE);
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CControls* pCtrls = GetControls();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CImageDoc* pActDoc = (CImageDoc*) GetWIActiveDocument();
	if(pDoc == pActDoc){
		CImageCtrls* pImCtrls = GetImageCtrls(this);
		CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
		CControls* pCtrls = GetControls();
		CPoint l_point(point);
		ClientToDoc(l_point);
		CursorPos = l_point;
		BOOL keyDown = FALSE;
		if((nFlags & MK_LBUTTON) || 
			GetAsyncKeyState(VK_LEFT)<0 || GetAsyncKeyState(VK_RIGHT)<0 ||
			GetAsyncKeyState(VK_UP)<0 || GetAsyncKeyState(VK_DOWN)<0)
		      keyDown = TRUE;
		
		if(keyDown && pDoc->IsFotoSections()){
			DrawMouseMoveCrossedLines(l_point);
			pDoc->ReSetSections(l_point);
     	    InvalidateRect(NULL,FALSE);
		}
		else if(pCtrls->EnableOptions & I_MEASURE_ACTIVE &&
			pCtrls->EnableOptions & I_MEASURE_DRAW){
			DrawMouseMoveMeasureLine(l_point);
		}
		else if((pCtrls->EnableOptions & I_BOUNDS_EXT) && pCtrls->EnableCustomDots){
			DragCustomDot(l_point);
		}
		else if((pCtrls->EnableOptions & I_BOUNDS_INS) && pCtrls->EnableCustomDots){
			DragCustomDot(l_point);
		}
		else if(pDoc->Tracker.GetDragingState()){
			DragTracker(l_point);
		}
		else if(pDoc->IsLockedDot()){
			DragDot(l_point);
		}
		else if(pDoc->IsLockedZapSection()){
			DragZapSection(l_point);
		}
	}
    CBaseImageView::OnMouseMove(nFlags, point);
}

void CImageView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    CImageCtrls* pImCtrls = GetImageCtrls(this);
    CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
    CBoundCtrls* pBCtrls = GetBoundCtrls(this);
    CControls* pCtrls = GetControls();
    
    CPoint l_point(point);
    ClientToDoc(l_point);
	CursorPos = l_point;
    if(pDoc->IsFotoSections()){
	    DrawMouseMoveCrossedLines(l_point);
		pDoc->ReSetSections(l_point);
 	    InvalidateRect(NULL, FALSE);
	}
    else if(pCtrls->EnableOptions & I_MEASURELINE){
      BeginLine(l_point);
      SetCapture();
    }
    else if((pCtrls->EnableOptions & I_BOUNDS_EXT) && pCtrls->EnableCustomDots){
      SetCustomDot(l_point);
    }
    else if((pCtrls->EnableOptions & I_BOUNDS_INS) && pCtrls->EnableCustomDots){
      SetCustomDot(l_point);
    }
    else if((pCtrls->EnableOptions & I_BOUNDS_EXT) && pCtrls->EnableTracker){
           SetCapture();
		   BeginTracker(l_point);
		   InvalidateRect(NULL, FALSE);
    }
    else if((pCtrls->EnableOptions & I_BOUNDS_INS) && pCtrls->EnableTracker){
           SetCapture();
		   BeginTracker(l_point);
		   InvalidateRect(NULL, FALSE);
    }
	else if(pCtrls->ActiveEditMode == E_ADD_DOT && pDoc->LockDot(l_point, TRUE)){
		BeginDragDot(l_point);
	}
	else if(pCtrls->ActiveEditMode == E_ADD_SECTION && pDoc->LockZapSection(l_point, TRUE)){
		BeginDragZapSection(l_point);
	}
	else{
		pDoc->OnLButDown(l_point);
	}
   CBaseImageView::OnLButtonDown(nFlags, point);
}

void CImageView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CBaseImageView::OnLButtonDblClk(nFlags, point);
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    CImageCtrls* pImCtrls = GetImageCtrls(this);
    CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
    CBoundCtrls* pBCtrls = GetBoundCtrls(this);
    CControls* pCtrls = GetControls();
    
    CPoint l_point(point);
    ClientToDoc(l_point);

    if(pDoc->IsFotoSections()){
		;
	}
    else if(pCtrls->EnableOptions & I_MEASURE_ACTIVE){
      ReleaseCapture();
      EndLine(l_point);
      pCtrls->EnableOptions &= ~I_MEASURE_DRAW;
      Invalidate(FALSE);
   }    
   else if((pCtrls->EnableOptions & I_BOUNDS_EXT) && pCtrls->EnableCustomDots){
		DropCustomDot(l_point);
   }
   else if((pCtrls->EnableOptions & I_BOUNDS_INS) && pCtrls->EnableCustomDots){
		DropCustomDot(l_point);
   }
   else if(pDoc->Tracker.GetDragingState()){
        ReleaseCapture();
		DropTracker(l_point);
		CRect R;
        if(pDoc->Tracker.GetBoundRect(R)){
			pBCtrls->CustomDots.SetSize(4);
			int W = R.Width()/2;
			int H = R.Height()/2;
			pBCtrls->CustomDots[0].x = R.left + W;
			pBCtrls->CustomDots[0].y = R.top;
			pBCtrls->CustomDots[1].x = R.right;
			pBCtrls->CustomDots[1].y = R.top + H;
			pBCtrls->CustomDots[2].x = R.left + W;
			pBCtrls->CustomDots[2].y = R.bottom;
			pBCtrls->CustomDots[3].x = R.left;
			pBCtrls->CustomDots[3].y = R.top + H;
			pBCtrls->SetCurBound(pCtrls->CurTypeBound);
        }
   }
   else if(pDoc->IsLockedDot()){
        ReleaseCapture();
		DropDot(l_point);
		pDoc->LockDot(l_point, FALSE);
   }
   else if(pDoc->IsLockedZapSection()){
        ReleaseCapture();
		DropZapSection(l_point);
		pDoc->LockZapSection(l_point, FALSE);
   }

   CBaseImageView::OnLButtonUp(nFlags, point);
}

void CImageView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    
    CPoint l_point(point);
    ClientToDoc(l_point);

	pDoc->OnRButDown(l_point);
	
    CBaseImageView::OnRButtonDown(nFlags, point);
}

void CImageView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	CBaseImageView::OnRButtonDblClk(nFlags, point);
}

void CImageView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    // TODO: Add your message handler code here and/or call default
    
    CBaseImageView::OnRButtonUp(nFlags, point);
}

BOOL CImageView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CImageDoc* pDoc = (CImageDoc*) GetDocument();
	CImageDoc* pActDoc = (CImageDoc*) GetWIActiveDocument();
	if(pDoc == pActDoc){
		CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
		CBoundCtrls* pBCtrls =  GetBoundCtrls(this);
        CControls* pCtrls = GetControls();
		if(pWnd == this && ((pCtrls->EnableOptions & I_BOUNDS_EXT)||(pCtrls->EnableOptions & I_BOUNDS_INS)) && pCtrls->EnableTracker){
            CClientDC dc(this);
            OnPrepareDC(&dc);
			if(pDoc->Tracker.SetCursor(this, &dc, nHitTest))
				return TRUE;
			else{
			  HINSTANCE inst = AfxGetResourceHandle();
			  HANDLE han = LoadImage(inst,MAKEINTRESOURCE(IDC_RECT_BOUND_CUR), IMAGE_CURSOR, 32,32, LR_SHARED);
			  SetCursor(HCURSOR(han));
			}
			return TRUE;
		}
		if(pCtrls->EnableOptions & I_MEASURELINE){
			HINSTANCE inst = AfxGetResourceHandle();
			HANDLE han = LoadImage(inst,MAKEINTRESOURCE(IDC_MEASURE_CUR), IMAGE_CURSOR, 32,32, LR_SHARED);
			SetCursor(HCURSOR(han));
			return TRUE;
		}
		if(((pCtrls->EnableOptions & I_BOUNDS_EXT)||(pCtrls->EnableOptions & I_BOUNDS_INS)) && pCtrls->EnableCustomDots){
			HINSTANCE inst = AfxGetResourceHandle();
			HANDLE han = LoadImage(inst,MAKEINTRESOURCE(IDC_MARK_BOUND_CUR), IMAGE_CURSOR, 32,32, LR_SHARED);
			SetCursor(HCURSOR(han));
			return TRUE;
		}
		if(pDoc->IsDotUnderCursor(CursorPos)){
			HINSTANCE inst = AfxGetResourceHandle();
			HANDLE han = LoadImage(inst,MAKEINTRESOURCE(IDC_SEL_DOT), IMAGE_CURSOR, 32,32, LR_SHARED);
			SetCursor(HCURSOR(han));
			return TRUE;
		}
		if(pDoc->IsZapSectionUnderCursor(CursorPos)){
			HINSTANCE inst = AfxGetResourceHandle();
			HANDLE han = LoadImage(inst,MAKEINTRESOURCE(IDC_SEL_ZAP_SEC), IMAGE_CURSOR, 32,32, LR_SHARED);
			SetCursor(HCURSOR(han));
			return TRUE;
		}
	}
    return CBaseImageView::OnSetCursor(pWnd, nHitTest, message);
}

void CImageView::OnSize(UINT nType, int cx, int cy) 
{
    CBaseImageView::OnSize(nType, cx, cy);
}

void CImageView::OnMove(int x, int y) 
{
    CBaseImageView::OnMove(x, y);
}

void CImageView::OnSetFocus(CWnd* pOldWnd) 
{
  CBaseImageView::OnSetFocus(pOldWnd);
}

void CImageView::OnKillFocus(CWnd* pNewWnd) 
{
    CBaseImageView::OnKillFocus(pNewWnd);
}

void CImageView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CursorPos = CPoint(-1, -1);
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
//	if(pDoc && pDoc->IsFotoSections())
       InvalidateRect(NULL,FALSE);
	CBaseImageView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CImageView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CursorPos = CPoint(-1, -1);
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
//	if(pDoc && pDoc->IsFotoSections())
       InvalidateRect(NULL,FALSE);
	CBaseImageView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CImageView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    CControls* pCtrls = GetControls();
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    CBoundCtrls* pBCtrls = GetBoundCtrls(this);
   CImageCtrls* pI = GetImageCtrls();
    int xDIB = pI->ImageSize.cx;
    int yDIB = pI->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pBCtrls->GetExtCorBound(pBCtrls->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
	
	if(pDoc->IsDots() && BoundR.PtInRect(CursorPos)){
	    return;
	}
	
    if(pDoc->IsFotoSections()){
	   return;
	}

   CBaseImageView::OnContextMenu(pWnd, point);  
}

void CImageView::OnUndo()
{
  CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
  pDoc->LastOperationUndo();
}

void CImageView::OnUpdateUndo(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
  CControls* pCtrls = GetControls();
  
  if(pDoc->LastOperationType == O_NO_UNDO)
    pCmdUI->Enable(FALSE);
  else
    pCmdUI->Enable(TRUE);
}


void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  CControls* pCtrls = GetControls();
  CImageDoc* pDoc = (CImageDoc*)GetDocument();

  if((pCtrls->EnableOptions & I_BOUNDS_EXT) || (pCtrls->EnableOptions & I_BOUNDS_INS)){
	   if(nChar==VK_RETURN)
		  OnApplyBound();
	   else if(nChar==VK_ESCAPE)
		  OnRemoveCurBound();
	   else if(nChar==VK_DELETE)
          OnRemoveLastBound();
  }
		
  if(pDoc->IsFotoSections()){
	CPoint cPos;
	GetCursorPos(&cPos);
	CRect clR;
	GetClientRect(clR);
	ClientToScreen(clR);
	if(clR.PtInRect(cPos)){
		if(nChar == VK_LEFT)
			cPos.x -= 1; 	
		else if(nChar == VK_RIGHT)
			cPos.x += 1; 	
		else if(nChar == VK_UP)
			cPos.y -= 1; 	
		else if(nChar == VK_DOWN)
			cPos.y += 1; 	
		SetCursorPos(cPos.x, cPos.y);
	}
  }
  pDoc->OnKeyDown(nChar, nRepCnt, nFlags);
  CBaseImageView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CImageView::OnTimer(UINT nIDEvent) 
{
    KillTimer(nIDEvent);
    m_nTimer = 0;
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
	if(pDoc->IsFotoSections()){
	   pDoc->AlignFotoSections();
	}
	CBaseImageView::OnTimer(nIDEvent);
}

void CImageView::OnAutoDigit()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   DeActivateMode(I_BOUNDS_EXT);
   DeActivateMode(I_BOUNDS_INS);
   pDoc->AutoDigit();
}

void CImageView::OnUpdateAutoDigit(CCmdUI* pCmdUI)
{
	CBoundCtrls* pB = GetBoundCtrls(this);
    CImageCtrls* pI = GetImageCtrls();
    int xDIB = pI->ImageSize.cx;
    int yDIB = pI->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
	if(BoundR.IsRectNull())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CImageView::OnClearDigit()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   pDoc->ClearDigit();
}

void CImageView::OnUpdateClearDigit(CCmdUI* pCmdUI)
{
	CBoundCtrls* pB = GetBoundCtrls(this);
    CImageCtrls* pI = GetImageCtrls();
    int xDIB = pI->ImageSize.cx;
    int yDIB = pI->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
	if(BoundR.IsRectNull())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CImageView::OnCalcAproximation()
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  pDoc->CalcAproximation();
}

void CImageView::OnUpdateCalcAproximation(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
}

void CImageView::OnFCMax()
{
  CControls* pCtrls = GetControls();
  pCtrls->FringeCenterAs = FC_MAX;
}

void CImageView::OnUpdateFCMax(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  if(pCtrls->FringeCenterAs == FC_MAX)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnFCMin()
{
  CControls* pCtrls = GetControls();
  pCtrls->FringeCenterAs = FC_MIN;
}

void CImageView::OnUpdateFCMin(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  if(pCtrls->FringeCenterAs == FC_MIN)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnFCMinMax()
{
  CControls* pCtrls = GetControls();
  pCtrls->FringeCenterAs = FC_MINMAX;
}

void CImageView::OnUpdateFCMinMax(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  if(pCtrls->FringeCenterAs == FC_MINMAX)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}


void CImageView::OnAddDot()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_ADD_DOT)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_ADD_DOT;
}

void CImageView::OnUpdateAddDot(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();
  CBoundCtrls* pB = GetBoundCtrls(this);
    CImageCtrls* pI = GetImageCtrls();
    int xDIB = pI->ImageSize.cx;
    int yDIB = pI->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);

  if(!BoundR.IsRectNull())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_ADD_DOT)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnRemoveDot()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_DELETE_DOT)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_DELETE_DOT;
}

void CImageView::OnUpdateRemoveDot(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();
  CBoundCtrls* pB = GetBoundCtrls(this);
    CImageCtrls* pI = GetImageCtrls();
    int xDIB = pI->ImageSize.cx;
    int yDIB = pI->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);

  if(!BoundR.IsRectNull())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_DELETE_DOT)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnRemoveFringe()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_DELETE_FRINGE)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_DELETE_FRINGE;
}

void CImageView::OnUpdateRemoveFringe(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_DELETE_FRINGE)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnAddZAPSection()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_ADD_SECTION)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_ADD_SECTION;
}

void CImageView::OnUpdateAddSection(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CBoundCtrls* pB = GetBoundCtrls(this);
  CImageCtrls* pIm = GetImageCtrls(this);
  CControls* pCtrls = GetControls();
    int xDIB = pIm->ImageSize.cx;
    int yDIB = pIm->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);

  if(!BoundR.IsRectNull() && pDoc->LoadedFileType != T_FRN && pIm->m_pDIB && (pCtrls->ViewState & V_ZAPSECTIONS))
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_ADD_SECTION)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);

}

void CImageView::OnDelZAPSection()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_DELETE_SECTION)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_DELETE_SECTION;
}

void CImageView::OnUpdateDelSection(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CBoundCtrls* pB = GetBoundCtrls(this);
  CControls* pCtrls = GetControls();
  CImageCtrls* pIm = GetImageCtrls(this);
    int xDIB = pIm->ImageSize.cx;
    int yDIB = pIm->ImageSize.cy;
    CRect BoundR(0,0,0,0);
	pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);

  if(!BoundR.IsRectNull() && pDoc->LoadedFileType != T_FRN && (pCtrls->ViewState & V_ZAPSECTIONS))
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_DELETE_SECTION)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);

}

void CImageView::OnShiftDotLeft()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   CControls* pCtrls = GetControls();
   pDoc->DeActivateAllMode();
   if(pCtrls->ActiveEditMode == E_SECTION_LEFT)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_SECTION_LEFT;
}

void CImageView::OnUpdateShiftDotLeft(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CBoundCtrls* pB = GetBoundCtrls(this);
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots() && pDoc->LoadedFileType != T_FRN && pDoc->Digit.IsZapSections() && (pCtrls->ViewState & V_ZAPSECTIONS))
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_SECTION_LEFT)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnShiftDotRight()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   pDoc->DeActivateAllMode();
   CControls* pCtrls = GetControls();
   if(pCtrls->ActiveEditMode == E_SECTION_RIGHT)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_SECTION_RIGHT;
}

void CImageView::OnUpdateShiftDotRight(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CBoundCtrls* pB = GetBoundCtrls(this);
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots() && pDoc->LoadedFileType != T_FRN && pDoc->Digit.IsZapSections() && (pCtrls->ViewState & V_ZAPSECTIONS))
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_SECTION_RIGHT)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnRenumFringe()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   pDoc->DeActivateAllMode();
   CControls* pCtrls = GetControls();
   if(pCtrls->ActiveEditMode == E_RENUM_FRINGE)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_RENUM_FRINGE;
}

void CImageView::OnUpdateRenumFringe(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_RENUM_FRINGE)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnRenumDot()
{
   CImageDoc* pDoc = (CImageDoc*)GetDocument();
   pDoc->DeActivateAllMode();
   CControls* pCtrls = GetControls();
   if(pCtrls->ActiveEditMode == E_RENUM_DOT)
	  pCtrls->ActiveEditMode = -1;
   else
	  pCtrls->ActiveEditMode = E_RENUM_DOT;
}

void CImageView::OnUpdateRenumDot(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ActiveEditMode == E_RENUM_DOT)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

void CImageView::OnNumberMinus()
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  pDoc->OnNumberMinus();
}

void CImageView::OnUpdateNumberMinus(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
}

void CImageView::OnNumberPlus()
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  pDoc->OnNumberPlus();
}

void CImageView::OnUpdateNumberPlus(CCmdUI* pCmdUI)
{
  CImageDoc* pDoc = (CImageDoc*)GetDocument();
  CControls* pCtrls = GetControls();

  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
}

void CImageView::SingleIsoline(int pn, ISO_POINT *plist, double level, int ilevel)
{
  if (pn < 2)
    return;

  CDC* pDC = CDC::FromHandle(hDC);
  CControls* pCtrls = GetControls();
  
  int n = pn;
  ISO_POINT *list = plist;
  long int xC;
  long int yC;
  double z = level;

    COLORREF Color;
	pCtrls->GetIndexColor(z, Color);
    CPen pen;
    pen.CreatePen(PS_DOT, 0, Color);
    CPen* open = pDC->SelectObject(&pen);

	xC = (int)list->x;
	yC = (int)list->y;
    pDC->MoveTo(xC, yC);
    list++;

    while (--n){
	  xC = (int)list->x;
	  yC = (int)list->y;
      pDC->LineTo(xC, yC);
      list++;
    }
/*
    // Add text label to contour line.
      if (pn > MINCELLS)
      {
      srand((unsigned)time(NULL));

      static char s[80];
      sprintf (s, "%d", ilevel + 1);

      int mincell = MINCELLS; //(int)((double)pn / 4.);
      int index = (int)(((double)rand() / (double)RAND_MAX) * (double)mincell);

     }
*/
   CPen* retPen = pDC->SelectObject(open);
   if(retPen) retPen->DeleteObject();
}

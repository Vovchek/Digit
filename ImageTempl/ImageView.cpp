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
#include "DigitMode\Rendering\ShapeDrawStyle.h"

#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CImageView* gr = NULL;
void Polyline(int pn, ISO_POINT* plist, double level, int ilevel)
{
	gr->SingleIsoline(pn, plist, level, ilevel);
}

// Draw bounds (apertures) in screen coordinates so they scale with image pixels
void CImageView::DrawBounds(CDC* pDC)
{
    if (!pDC) {
        return;
    }

    auto* pApertureCtrls = GetApertureCtrls(this);
    if (!pApertureCtrls) {
        return;
    }

    pApertureCtrls->DrawShapes(*pDC, m_viewTransform, nullptr, -1);
}

// Override image drawing to draw bitmap without the world transform (to get proper resampling)
void CImageView::DrawImage(CDC* pDC)
{
	CImageCtrls* pImCtrls = GetImageCtrls(this);
	SECDib* pImage = (SECDib*)pImCtrls->GetImage();
	CControls* pCtrls = GetControls();

	if (pImage && (pCtrls->ViewState & V_INTERFEROGRAM)) {
		// compute destination rect in screen coords using ViewTransform
		CRect rcDIB(pImCtrls->GetDIBRect());
		CPoint tl = m_viewTransform.WorldToScreen(CPoint2d{ (double)rcDIB.left, (double)rcDIB.top });
		CPoint br = m_viewTransform.WorldToScreen(CPoint2d{ (double)rcDIB.right, (double)rcDIB.bottom });
		CRect rcDest(tl, br);
		rcDest.NormalizeRect();

		// Temporarily disable any world transform on this DC so StretchDIBits uses high-quality scaling
		HDC hdc = pDC->GetSafeHdc();
		XFORM oldX;
		BOOL hadTransform = FALSE;
		memset(&oldX, 0, sizeof(oldX));
		if (GetWorldTransform(hdc, &oldX)) {
			// set identity transform
			XFORM id = { 1.0f,0.0f,0.0f,1.0f,0.0f,0.0f };
			SetWorldTransform(hdc, &id);
			hadTransform = TRUE;
		}

		CPalette* pOldPalette = NULL;
		if (pImage && pImage->m_pPalette)
			pOldPalette = pDC->SelectPalette(pImage->m_pPalette, TRUE);
		// Use high-quality stretching for bitmap resampling
		int prevMode = SetStretchBltMode(hdc, HALFTONE);
		pDC->SetBrushOrg(rcDest.left % 8, rcDest.top % 8);
		pImage->m_bUseHalftone = TRUE;
		int ix = rcDIB.left, iy = rcDIB.top, iw = rcDIB.Width(), ih = rcDIB.Height();
		pImage->StretchDIBits(pDC,
			rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
			ix, iy, iw, ih,
			pImage->m_lpSrcBits,
			pImage->m_lpBMI, DIB_RGB_COLORS,
			SRCCOPY);

		if (pOldPalette)
			pDC->SelectPalette(pOldPalette, TRUE);
		// restore previous stretch mode
		SetStretchBltMode(hdc, prevMode);

		// restore previous world transform
		if (hadTransform) {
			SetWorldTransform(hdc, &oldX);
		}
	}
	else {
		CRect clRect; GetClientRect(clRect);
		pDC->FillRect(&clRect, &CBrush(RGB(0, 0, 0)));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageView

// CImageView

IMPLEMENT_DYNCREATE(CImageView, CBaseImageView)

CImageView::CImageView()
{
    // Tool handlers initialized as members; full initialization happens in OnInitialUpdate
}

CImageView::~CImageView()
{
	//    ::DeleteObject(HGDIOBJ(bkColorBrush));
	delete m_boundsToolAdapter;
	delete m_fringeToolAdapter;
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
	for (int ix = 0; ix < nP; ix++) {
		CPoint d_curP(ix, iy);
		if (d_regDIB.PtInRect(d_curP)) {
			CPoint l_curP(d_curP);
			ClientToDoc(l_curP);
			CDPoint dP(l_curP);
			int iix = (int)dP.x;
			int iiy = yDIB - (int)dP.y;
			if (iix > -1 && iiy > -1 && iix < xDIB && iiy < yDIB)
				pF[ix] = (double)((int)lpDIBBits[xDIB * iiy + iix] / 255.);
			else
				pF[ix] = 0.;
		}
		else {
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
	if (GetStyle() & WS_HSCROLL)
		iScrollWidth = 15;

	ix = d_P.x;
	for (int iy = nP; iy > 0; iy--) {
		CPoint d_curP(ix, iy);
		if (d_regDIB.PtInRect(d_curP)) {
			CPoint l_curP(d_curP);
			ClientToDoc(l_curP);
			CDPoint dP(l_curP);
			int iix = (int)dP.x;
			int iiy = yDIB - (int)dP.y;
			if (iix > -1 && iiy > -1 && iix < xDIB && iiy < yDIB)
				pF[nP - iy] = (double)((int)lpDIBBits[xDIB * iiy + iix] / 255.);
			else
				pF[nP - iy] = 0.;
		}
		else {
			pF[nP - iy] = 0.;
		}
		pR[nP - iy] = nP - iy;
	}
	return true;
}

BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
	//{{AFX_MSG_MAP(CImageView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_COMMAND(IDD_MEASURE, OnMeasure)
	ON_UPDATE_COMMAND_UI(IDD_MEASURE, OnUpdateMeasure)
	ON_COMMAND(IDD_FOTO_SECTIONS, OnFotoSections)
	ON_UPDATE_COMMAND_UI(IDD_FOTO_SECTIONS, OnUpdateFotoSections)
	ON_COMMAND(IDD_ZOOM_IN, OnZoomIn)
	ON_COMMAND(IDD_ZOOM_OUT, OnZoomOut)
	ON_COMMAND(IDD_ZOOM_FIT, OnZoomFit)
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
	// undo/redo
	ON_COMMAND(IDD_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(IDD_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(IDD_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(IDD_EDIT_REDO, OnUpdateEditRedo)
	// bounds editing commands
	ON_COMMAND(ID_ADD_BOUND_CIRCLE, OnAddBoundCircle)
	ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_CIRCLE, OnUpdateAddBound)
	ON_COMMAND(ID_ADD_BOUND_ELLIPSE, OnAddBoundEllipse)
	ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_ELLIPSE, OnUpdateAddBound)
	ON_COMMAND(ID_ADD_BOUND_RECT, OnAddBoundRect)
	ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_RECT, OnUpdateAddBound)
	ON_COMMAND(ID_ADD_BOUND_POLYGON, OnAddBoundPolygon)
	ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_POLYGON, OnUpdateAddBound)
	ON_COMMAND(ID_BOUND_VISIBILITY, OnBoundVisisbility)
	ON_UPDATE_COMMAND_UI(ID_BOUND_VISIBILITY, OnUpdateBoundVisibility)
	ON_COMMAND(ID_BOUND_MODE_SELECT, OnBoundModeSelect)
	ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_SELECT, OnUpdateBoundModeSelect)
	ON_COMMAND(ID_BOUND_MODE_DELETE, OnBoundModeDelete)
	ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_DELETE, OnUpdateBoundModeDelete)	
	// fringes editing - temporary on IDD_ADD_DOT_D
	ON_COMMAND(IDD_ADD_DOT_D, OnFringesEdit)
	ON_UPDATE_COMMAND_UI(IDD_ADD_DOT_D, OnUpdateFringesEdit)
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
	m_CursorPos = CPoint(-1, -1);
	MeasureLine = CRect(0, 0, 0, 0);
	m_bLinning = FALSE;
	m_Captured = FALSE;
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
	pMCtrls->Init();
}

void CImageView::OnInitialUpdate()
{
	CBaseImageView::OnInitialUpdate();

	m_CursorPos = CPoint(-1, -1);
	CImageCtrls* pImage = GetImageCtrls(this);
	CControls* pCtrls = GetControls();

	CRect wDIBRect = pImage->GetDIBRect();
	CFrameWnd* pFr = GetParentFrame();
	WINDOWPLACEMENT wp;
	pFr->GetWindowPlacement(&wp);
	pFr->CalcWindowRect(&wDIBRect, CWnd::adjustOutside);
	int W = wDIBRect.Width() + 20;
	int H = wDIBRect.Height() + 20;
	CDC* pDC = GetDC();  // Get DC - will be released below
	int scr_W = pDC->GetDeviceCaps(HORZRES);
	int scr_H = pDC->GetDeviceCaps(VERTRES);
	ReleaseDC(pDC);
	BOOL needFit = FALSE;
	if (W > std::lround(scr_W * 0.65)) {
		W = std::lround(scr_W * 0.65);
		needFit = TRUE;
	}
	if (H > std::lround(scr_H * 0.65)) {
		H = std::lround(scr_H * 0.65);
		needFit = TRUE;
	}
	pFr->MoveWindow(wp.rcNormalPosition.left, wp.rcNormalPosition.top, W, H);
	if (needFit) {
		OnZoomFit();
	}

	CenterImageInView();

	// Initialize tooltip control for dynamic hover tooltips
	if (!m_tooltip.m_hWnd) {
		m_tooltip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
		m_tooltip.AddTool(this, _T(""));
		m_tooltip.SetMaxTipWidth(300);
		m_tooltip.Activate(TRUE);
	}
	
	// ========================================================================
	// Phase 5: Initialize Tool Input Handlers (CAD-Grade Architecture)
	// ========================================================================
	
	CImageDoc* pDoc = (CImageDoc*)CView::GetDocument(); // othewise CImageView::GetDocument() calls GetWIActiveDocument()
	m_pDoc = pDoc; // save actual document pointer
	if (pDoc) {
		// Initialize aperture controls and get centralized command dispatcher
		DigitMode::CApertureCtrls* pApertureCtrls = GetApertureCtrls(this);
		DigitMode::CommandDispatcher* pDispatcher = pApertureCtrls ? 
			&pApertureCtrls->GetCommandDispatcher() : &m_cmdDispatcher;
		
		// Initialize fringe handler
		m_fringeInputHandler.Initialize(
			&pDoc->Digit,
			&GetViewTransform(),
			pDispatcher
		);
		
		// Initialize bounds handler with aperture subsystem and image provider
		if (pImage && pApertureCtrls) {
			m_boundsInputHandler.Initialize(
				pApertureCtrls,
				pImage,
				&GetViewTransform(),
				pDispatcher
			);
		}
		
		// ====================================================================
		// Day 3: Initialize InteractionManager with Tool Adapters
		// ====================================================================
		
		// Create tool adapters wrapping existing handlers
		m_boundsToolAdapter = new DigitMode::BoundsToolAdapter(&m_boundsInputHandler);
		m_fringeToolAdapter = new DigitMode::FringeToolAdapter(&m_fringeInputHandler);
		
		// Register tools with interaction manager
		GetInteractionManager().RegisterTool(m_boundsToolAdapter);
		GetInteractionManager().RegisterTool(m_fringeToolAdapter);
		
		// Set fringe handler as initial active tool
		GetInteractionManager().SetActiveTool(m_fringeToolAdapter);
		
		// BAND-AID: Keep InputRouter working as fallback for now
		// Both systems manage the same handlers, but event routing properly checks return values
		// so events are only processed once per system (not twice)
		// TODO: Later migration - make InteractionManager the sole routing system and remove InputRouter
		// GetInputRouter().SetActiveTool(&m_fringeInputHandler);  // ← RE-ENABLED for stability
	}
}

void CImageView::CenterImageInView()
{
	CImageCtrls* pImage = GetImageCtrls(this);
	if (!pImage || !pImage->HasImage()) return;

	CRect imgRect = pImage->GetDIBRect();
	if (imgRect.IsRectEmpty()) return;

	CRect clientR;
	GetClientRect(clientR);
	if (clientR.IsRectEmpty()) return;

	// Calculate scaled image dimensions in screen space
	double scale = m_viewTransform.GetScale();
	double scaledW = imgRect.Width() * scale;
	double scaledH = imgRect.Height() * scale;

	// Center the image: position it so the scaled image is centered
	// but don't push it off-screen if it's smaller than the client area
	double offsetX = (clientR.Width() - scaledW) / 2.0;
	double offsetY = (clientR.Height() - scaledH) / 2.0;

	CPoint2d newOffset = { offsetX, offsetY };
	m_viewTransform.SetOffset(newOffset);

	Invalidate(FALSE);
}

// ============================================================================
// Window Resize and Zoom Centering
// ============================================================================

void CImageView::OnSize(UINT nType, int cx, int cy)
{
	// Call base class first to handle standard CScrollView sizing
	CBaseImageView::OnSize(nType, cx, cy);
	
	// When window is maximized or restored to normal size, center the image
	// within the new client area to keep it visible and properly positioned
	if (nType == SIZE_MAXIMIZED || nType == SIZE_RESTORED) {
		CenterImageInView();
	}
}

void CImageView::OnZoomIn()
{
	CRect clientR;
	GetClientRect(clientR);
	CPoint center(clientR.Width() / 2, clientR.Height() / 2);
	m_viewTransform.ZoomAt(center, 1.15);
	Invalidate(FALSE);
}

void CImageView::OnZoomOut()
{
	CRect clientR;
	GetClientRect(clientR);
	CPoint center(clientR.Width() / 2, clientR.Height() / 2);
	m_viewTransform.ZoomAt(center, 1.0 / 1.15);
	Invalidate(FALSE);
}

void CImageView::OnZoomFit()
{
	CImageCtrls* pImage = GetImageCtrls(this);
	CRect imgRect = pImage->GetDIBRect();
	CRect clientR;
	GetClientRect(clientR);

	m_viewTransform.ZoomToFit(imgRect, clientR);
	Invalidate(FALSE);
}

// ============================================================================
// Phase 5: Tool Activation Methods (CAD-Grade Architecture)
// ============================================================================

void CImageView::ActivateFringeTool()
{
	// Set fringe handler as active tool in BOTH systems
	// (both check return values to prevent double-processing)
	GetInteractionManager().SetActiveTool(m_fringeToolAdapter);
	//GetInputRouter().SetActiveTool(&m_fringeHandler);  // ← RE-ENABLED
	Invalidate(FALSE);
}

void CImageView::ActivateBoundsTool()
{
	// Set bounds handler as active tool in BOTH systems
	// (both check return values to prevent double-processing)
	GetInteractionManager().SetActiveTool(m_boundsToolAdapter);
	//GetInputRouter().SetActiveTool(&m_boundsInputHandler);  // ← RE-ENABLED
	Invalidate(FALSE);
}

// ============================================================================
// Tooltip Update (from BaseImageView virtual hook)
// ============================================================================

void CImageView::UpdateTooltip(const CString& tooltip)
{
	if (m_tooltip.m_hWnd && tooltip != m_lastTip) {
		m_tooltip.UpdateTipText(tooltip, this);
		m_lastTip = tooltip;
		m_tooltip.Activate(!tooltip.IsEmpty());
	}
}

// ============================================================================
// Legacy Drawing Methods
// ============================================================================

void CImageView::DrawDigitInfo(CDC* pDC)
{
	// Use single legacy drawing path: CDigitInfo::Draw expects the DC to have
	// a world transform applied so coordinates inside Draw are in image/world
	// space. We set the transform here from m_viewTransform, then call Draw.
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CControls* pCtrls = GetControls();
	if (!pDoc || !pCtrls) return;

	// Prepare HDC and save state
	HDC hdc = pDC->GetSafeHdc();
	int oldMode = SetGraphicsMode(hdc, GM_ADVANCED);
	XFORM oldX; memset(&oldX, 0, sizeof(oldX));
	bool hadOld = false;
	if (GetWorldTransform(hdc, &oldX)) hadOld = true;

	// Get current viewport origin to adjust translation when drawing into
	// an offscreen DC that has offset viewport set by OnDraw.
	POINT vp = { 0,0 };
	::GetViewportOrgEx(hdc, &vp);

	double s = m_viewTransform.GetScale();
	CPoint2d off = m_viewTransform.GetOffset();
	XFORM xform;
	xform.eM11 = (FLOAT)s; xform.eM12 = 0.0f;
	xform.eM21 = 0.0f; xform.eM22 = (FLOAT)s;
	xform.eDx = (FLOAT)(off.x - vp.x);
	xform.eDy = (FLOAT)(off.y - vp.y);
	SetWorldTransform(hdc, &xform);

	// Delegate drawing to CDigitInfo::Draw which handles extremums, dots,
	// fringes and rubber-band consistently in world coordinates.
	int DotSide = 6; pCtrls->GetCorrectDotSize(DotSide, pDoc);
	CPoint active = m_fringeInputHandler.GetInputHandler().GetActiveDot(&pDoc->Digit);
	// CursorPos is already in world coordinates (set in OnMouseMove)
	CPoint cursor = m_viewTransform.ScreenToWorld(m_CursorPos);
	bool rubber = m_fringeInputHandler.GetInputHandler().GetRubberBand(&pDoc->Digit);
	pDoc->Digit.Draw(pDC, DotSide, active, cursor, rubber);

	// Restore previous transform/state
	if (hadOld) SetWorldTransform(hdc, &oldX);
	SetGraphicsMode(hdc, oldMode);
}

void CImageView::DrawAproximation(CDC* pDC)
{
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CControls* pCtrls = GetControls();

	if (pDoc->IsAproximation() && (pCtrls->ViewState & V_APPROXIMATION)) {
		gr = this;
		hDC = pDC->GetSafeHdc();
		if (pDoc->nLevel > 0)
			pDoc->pLevel[pDoc->nLevel - 1] *= 0.999999999;

		ApproxContour(pDoc->pMatr, pDoc->nx, pDoc->ny, pDoc->pY, pDoc->pX, pDoc->nLevel, pDoc->pLevel, Polyline);
	}
}

void CImageView::OnDraw(CDC* pDC)
{
	CControls* pCtrls = GetControls();
	CImageDoc* pDoc = (CImageDoc*)GetDocument();

	// ViewTransform is used for coordinate conversions. We draw bitmap in
	// device coords (no world-transform) for proper resampling, then apply
	// a temporary GDI world-transform to the same DC for vector overlays
	// so they scale/translate correctly. xForm is computed after rectClip so
	// we can adjust translation when drawing to an offscreen bitmap.

	CDC dc;
	CDC* pDrawDC = pDC;
	CBitmap bitmap;
	CBitmap* pOldBitmap;

	CRect client, rectClip;
	pDC->GetClipBox(&client);
	rectClip = client;
	pDC->LPtoDP(&rectClip);
	rectClip.InflateRect(1, 1); // avoid rounding to nothing

	// compute world transform based on current view
	double s = m_viewTransform.GetScale();
	CPoint2d off = m_viewTransform.GetOffset();
	XFORM xForm;
	xForm.eM11 = (FLOAT)s; xForm.eM12 = 0.0f;
	xForm.eM21 = 0.0f; xForm.eM22 = (FLOAT)s;
	// default translation assumes drawing to screen DC
	xForm.eDx = (FLOAT)off.x; xForm.eDy = (FLOAT)off.y;

	if (dc.CreateCompatibleDC(pDC)) {
		if (bitmap.CreateCompatibleBitmap(pDC, rectClip.Width(), rectClip.Height()))
		{
			OnPrepareDC(&dc, NULL);
			pDrawDC = &dc;
			// offset origin more because bitmap is just piece of the whole drawing
			dc.OffsetViewportOrg(-rectClip.left, -rectClip.top);
			// When drawing into an offscreen bitmap the world transform's
			// translation must be adjusted by the viewport offset.
		 xForm.eDx = (FLOAT)(off.x - rectClip.left);
		 xForm.eDy = (FLOAT)(off.y - rectClip.top);
			pOldBitmap = dc.SelectObject(&bitmap);
			dc.SetBrushOrg(rectClip.left % 8, rectClip.top % 8);
			// might as well clip to the same rectangle
			dc.IntersectClipRect(client);
		}
	}

	DrawBackGround(pDrawDC);
	DrawImage(pDrawDC);

	// Draw vector overlays in screen (device) coordinates. Do not apply
	// a GDI world-transform here - Draw* functions convert world points
	// to screen via m_viewTransform.WorldToScreen when needed.

	DrawMeasureLine(pDrawDC);
	if (pCtrls->EnableCustomDots) {
		DrawCustomDots(pDrawDC);
		DrawCurBound(pDrawDC);
	}
	else if (pDoc->Tracker.GetEnableState()) {
		pDoc->Tracker.DrawTracker(pDrawDC);
		DrawCurBound(pDrawDC);
	}
	if (pDoc->IsFotoSections()) {
		DrawCrossedLines(pDrawDC);
		pDoc->ReSetSections(m_CursorPos);
	}
	DrawBounds(pDrawDC);
	DrawDigitInfo(pDrawDC);
	DrawAproximation(pDrawDC);
	
	// Draw selection box rubber-band if active (Navigate mode)
	// m_drag points are in world coordinates, pass viewTransform for screen conversion
	using namespace DigitMode;
	if (m_fringeInputHandler.GetInputHandler().GetEditMode() == FringeEditMode::Navigate) {
		m_fringeInputHandler.GetInputHandler().DrawSelectionBox(pDrawDC, &m_viewTransform);
	}
	// After drawing committed shapes:
	if (m_boundsInputHandler.GetBoundsHandler().IsDrafting()) {
		const auto& boundsHandler = m_boundsInputHandler.GetBoundsHandler();

		// Draw vertex markers (X marks) for draft points
		const auto* draft = boundsHandler.GetDraft();
		if (draft) {
			const auto& points = draft->perimeterPoints;
			CPen markerPen(PS_SOLID, 1, RGB(255, 255, 0));  // Yellow X marks
			CPen* oldPen = pDrawDC->SelectObject(&markerPen);

			const int markerSize = 4;  // pixels
			for (const auto& worldPt : points) {
				CPoint screenPt = m_viewTransform.WorldToScreen(CPoint2d{ worldPt.x, worldPt.y });
				// Draw X mark
				pDrawDC->MoveTo(screenPt.x - markerSize, screenPt.y - markerSize);
				pDrawDC->LineTo(screenPt.x + markerSize, screenPt.y + markerSize);
				pDrawDC->MoveTo(screenPt.x + markerSize, screenPt.y - markerSize);
				pDrawDC->LineTo(screenPt.x - markerSize, screenPt.y + markerSize);
			}

			pDrawDC->SelectObject(oldPen);
		}

		const auto* preview = boundsHandler.GetDraftPreview();
		if (preview) {
			ShapeDrawStyle style;
			style.state = ShapeDrawStyle::State::Draft;
			style.type = boundsHandler.GetShapeType();
			style.showHandles = false;
			m_shapeDrawDispatcher.Draw(*preview, *pDrawDC, style, m_viewTransform);			
		}
	}
	
	// Phase D: Draw preview shape during drag operations (move/resize handles, body drag)
	if (m_boundsInputHandler.GetBoundsHandler().IsDragging()) {
		const auto& boundsHandler = m_boundsInputHandler.GetBoundsHandler();
		const auto* previewShape = boundsHandler.GetPreviewShape();
		if (previewShape) {
			ShapeDrawStyle style;
			style.state = ShapeDrawStyle::State::Selected;
			style.type = boundsHandler.GetHoveredShapeType();
			style.showHandles = true;
			m_shapeDrawDispatcher.Draw(*previewShape, *pDrawDC, style, m_viewTransform);
			m_shapeDrawDispatcher.DrawHandles(*previewShape, *pDrawDC, style, m_viewTransform);
		}
	}
	// Drawing performed here... (no world-transform applied)
	if (pDrawDC != pDC) {
		pDC->SetViewportOrg(0, 0);
		pDC->SetWindowOrg(0, 0);
		pDC->SetMapMode(MM_TEXT);
		dc.SetViewportOrg(0, 0);
		dc.SetWindowOrg(0, 0);
		dc.SetMapMode(MM_TEXT);
		pDC->BitBlt(rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
			&dc, 0, 0, SRCCOPY);
		dc.SelectObject(pOldBitmap);
	}
	
	// ========================================================================
	// Day 3: Render InteractionManager View State (unified tool visualization)
	// ========================================================================
	
	auto viewState = GetInteractionManager().GetViewState();
	
	// Render shapes from all active tools (bounds, fringe, etc.)
	for (const auto& layer : viewState.layers) {
		for (const auto& shape : layer.toolState.shapes) {
			m_shapeDrawDispatcher.Draw(
				*shape.shape,
				*pDrawDC,
				shape.style,
				m_viewTransform
			);
		}
	}
	
	if (pDoc) {
		// Update text controls
		pDoc->SetZoomToTitle();
		GetMainFrame()->SetCurrentNumber(pDoc->Digit.CurrentNumber);
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
	if (!pDoc || !pFR || !pCtrls)
		return;

	if (bActivate) {
		Invalidate(FALSE);
		pDoc->OnActivate();
		pDoc->SetTextWndInfo();
		pFR->SetCurrentNumber(pDoc->Digit.CurrentNumber);
		pFR->SetImageInfo(pDoc->Digit.Comments, pDoc->Digit.ScaleFactor, pDoc->Digit.Rotation);
	}
	else {
		pFR->GetImageInfo(pDoc->Digit.Comments, pDoc->Digit.ScaleFactor, pDoc->Digit.Rotation);
	}
	CBaseImageView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CImageView::OnMeasure()
{
	CMainFrame* pMFr = GetMainFrame();
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CControls* pCtrls = GetControls();
	if (pCtrls->EnableOptions & I_MEASURELINE) {
		DeActivateMode(I_MEASURELINE);
		pMFr->ShowMeasurePane(FALSE);
	}
	else {
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
	if (!pDoc->IsFotoSections()) {
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

void CImageView::BeginLine(CPoint P)
{
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CString s = pDoc->GetPathName();
	CImageCtrls* pImCtrls = GetImageCtrls(this);
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
	CControls* pCtrls = GetControls();

	int step = pCtrls->MarkerSide;
	CPoint cP(P);
	CRect rP(cP.x - step, cP.y - step, cP.x + step, cP.y + step);

	CPoint P1(MeasureLine.left, MeasureLine.top);
	CPoint P2(MeasureLine.right, MeasureLine.bottom);

	PisActive = 0;
	// Check if clicking near existing measure endpoints (P, P1, P2 are all in world coords)
	if (rP.PtInRect(P1)) {
		PisActive = 1;
		// Snap cursor to P1 in screen space
		CPoint screenP1 = m_viewTransform.WorldToScreen(CPoint2d{ (double)P1.x, (double)P1.y });
		ClientToScreen(&screenP1);
		SetCursorPos(screenP1.x, screenP1.y);
	}
	else if (rP.PtInRect(P2)) {
		PisActive = 2;
		CPoint screenP2 = m_viewTransform.WorldToScreen(CPoint2d{ (double)P2.x, (double)P2.y });
		ClientToScreen(&screenP2);
		SetCursorPos(screenP2.x, screenP2.y);
	}
	else {
		pCtrls->EnableOptions &= ~I_MEASURE_ACTIVE;
		Invalidate(FALSE);
		MakeLoopMessage();

		// Start new measure line at clicked point (P is in world coords)
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

	if (PisActive == 2) {
		MeasureLine.right = P2.x;
		MeasureLine.bottom = P2.y;
	}
	else if (PisActive == 1) {
		MeasureLine.left = P2.x;
		MeasureLine.top = P2.y;
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
	if (GetCapture() != this)
		return;

	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CImageCtrls* pImCtrls = GetImageCtrls(this);
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);

	COLORREF Color = InvColor;
	CPen penLine;
	penLine.CreatePen(PS_COSMETIC, 0, Color);

	CClientDC dc(this);
	OnPrepareDC(&dc);
	CPen* pOld = dc.SelectObject(&penLine);
	int orop = dc.SetROP2(R2_XORPEN);

	// Convert world coords to screen for XOR drawing
	CPoint scrP1 = m_viewTransform.WorldToScreen(CPoint2d{ (double)MeasureLine.left, (double)MeasureLine.top });
	CPoint scrP2 = m_viewTransform.WorldToScreen(CPoint2d{ (double)MeasureLine.right, (double)MeasureLine.bottom });
	dc.MoveTo(scrP1);
	dc.LineTo(scrP2);
	DrawMarker(&dc, scrP1);
	DrawMarker(&dc, scrP2);

	// Update world coords with new position
	if (PisActive == 2) {
		MeasureLine.right = P2.x;
		MeasureLine.bottom = P2.y;
	}
	else if (PisActive == 1) {
		MeasureLine.left = P2.x;
		MeasureLine.top = P2.y;
	}

	if (PisActive == 1 || PisActive == 2) {
		scrP1 = m_viewTransform.WorldToScreen(CPoint2d{ (double)MeasureLine.left, (double)MeasureLine.top });
		scrP2 = m_viewTransform.WorldToScreen(CPoint2d{ (double)MeasureLine.right, (double)MeasureLine.bottom });
		dc.MoveTo(scrP1);
		dc.LineTo(scrP2);
		DrawMarker(&dc, scrP1);
		DrawMarker(&dc, scrP2);
	}

	CPoint TL = MeasureLine.TopLeft();
	CPoint BR = MeasureLine.BottomRight();
	CRect rcDIB(pImCtrls->GetDIBRect());
	CRect rcDest(pImCtrls->GetDIBRect());
	pMCtrls->SetFirstPoint(rcDest, rcDIB, TL);
	pMCtrls->SetSecondPoint(rcDIB, rcDest, BR);
	pDoc->WriteMeasureCtrls();

	dc.SetROP2(orop);
	CPen* pRetPen = dc.SelectObject(pOld);
	if (pRetPen) pRetPen->DeleteObject();
}

void CImageView::DrawMeasureLine(CDC* pDC)
{
	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	CImageCtrls* pImCtrls = GetImageCtrls(this);
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(this);
	CControls* pCtrls = GetControls();

	if (pCtrls->EnableOptions & I_MEASURE_ACTIVE) {
		COLORREF Color = InvColor;
		CPen* open = NULL;
		CPen pen0;
		pen0.CreatePen(PS_COSMETIC, 0, RGB(0, 0, 0));
		open = pDC->SelectObject(&pen0);

		CRect rDIB(pImCtrls->GetDIBRect());
		CRect rcDest = (pImCtrls->GetDIBRect());

		int orop = pDC->SetROP2(R2_XORPEN);
		CPen pen1;
		pen1.CreatePen(PS_COSMETIC, 0, Color);
		CPen* pPen = pDC->SelectObject(&pen1);
		pPen->DeleteObject();

		// convert measure endpoints from world to screen so markers align with view
		CDPoint dP1 = pMCtrls->L.P1;
		CDPoint dP2 = pMCtrls->L.P2;
		CPoint cP1 = m_viewTransform.WorldToScreen(CPoint2d{ dP1.x, dP1.y });
		CPoint cP2 = m_viewTransform.WorldToScreen(CPoint2d{ dP2.x, dP2.y });
		DrawMarker(pDC, cP1);
		DrawMarker(pDC, cP2);

		CPen pen2;
		pen2.CreatePen(PS_COSMETIC, 0, Color);
		pPen = pDC->SelectObject(&pen2);
		pPen->DeleteObject();

		pDC->MoveTo(cP1.x, cP1.y);
		pDC->LineTo(cP2.x, cP2.y);

		pDC->SetROP2(orop);
		if (open) {
			CPen* pRetPen = pDC->SelectObject(open);
			if (pRetPen) pRetPen->DeleteObject();
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
	// Convert client rect to screen coordinates via world->screen corners
	CPoint tl = m_viewTransform.WorldToScreen(CPoint2d{ (double)clientR.left, (double)clientR.top });
	CPoint br = m_viewTransform.WorldToScreen(CPoint2d{ (double)clientR.right, (double)clientR.bottom });
	CRect scrRect(tl, br);
	CPoint P1, P2;

	if (m_CursorPos != CPoint(-1, -1)) {
		CPoint s = m_viewTransform.WorldToScreen(CPoint2d{ (double)m_CursorPos.x, (double)m_CursorPos.y });
		P1.x = scrRect.left; P1.y = s.y;
		P2.x = scrRect.right; P2.y = s.y;
		pDC->MoveTo(P1); pDC->LineTo(P2);
		P1.x = s.x; P1.y = scrRect.top;
		P2.x = s.x; P2.y = scrRect.bottom;
		pDC->MoveTo(P1); pDC->LineTo(P2);
	}

	pDC->SetROP2(orop);
	CPen* retPen = pDC->SelectObject(open);
	if (retPen)
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

	if (m_CursorPos != CPoint(-1, -1)) {
		P1.x = clientR.left; P1.y = m_CursorPos.y;
		P2.x = clientR.right; P2.y = m_CursorPos.y;
		dc.MoveTo(P1);
		dc.LineTo(P2);
		P1.x = m_CursorPos.x; P1.y = clientR.top;
		P2.x = m_CursorPos.x; P2.y = clientR.bottom;
		dc.MoveTo(P1);
		dc.LineTo(P2);
	}

	m_CursorPos = P;
	P1.x = clientR.left; P1.y = m_CursorPos.y;
	P2.x = clientR.right; P2.y = m_CursorPos.y;
	dc.MoveTo(P1);
	dc.LineTo(P2);
	P1.x = m_CursorPos.x; P1.y = clientR.top;
	P2.x = m_CursorPos.x; P2.y = clientR.bottom;
	dc.MoveTo(P1);
	dc.LineTo(P2);
	dc.SetROP2(orop);

	CPen* retPen = dc.SelectObject(open);
	if (retPen)
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
	if (m_Captured) {
		CImageDoc* pDoc = (CImageDoc*)GetDocument();
		CControls* pCtrls = GetControls();
		int DotSide;
		pCtrls->GetCorrectDotSize(DotSide, pDoc);
		int DotSide12 = DotSide / 2;
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

		if (pCtrls->ViewState & V_ZAPSECTIONS) {
			P.y = P1.y;
		}

		pDoc->SetLockedDotPos(P);
		if (newPos) {
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

// deprecated/eliminated - no zap sections edits
void CImageView::DragZapSection(CPoint P, BOOL newPos/*TRUE*/)
{
	//if (m_Captured) {
	//	CClientDC dc(this);
	//	OnPrepareDC(&dc);
	//	CPen pen;
	//	pen.CreatePen(PS_SOLID, 0, InvColor);
	//	CPen* open = dc.SelectObject(&pen);
	//	int orop = dc.SetROP2(R2_XORPEN);

	//	CImageDoc* pDoc = (CImageDoc*)GetDocument();
	//	CPoint P1, P2;
	//	pDoc->GetLockedZapSectionXYPos(P1, P2);
	//	dc.MoveTo(P1);
	//	dc.LineTo(P2);
	//	pDoc->SetLockedZapSectionYPos(P.y);
	//	if (newPos) {
	//		pDoc->GetLockedZapSectionXYPos(P1, P2);
	//		dc.MoveTo(P1);
	//		dc.LineTo(P2);
	//	}
	//	dc.SetROP2(orop);
	//	CPen* retPen = dc.SelectObject(open);
	//	if (retPen)
	//		retPen->DeleteObject();
	//}
}

void CImageView::DropZapSection(CPoint P)
{
	m_Captured = FALSE;
	InvalidateRect(NULL, FALSE);
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
	//CBoundCtrls* pB = GetBoundCtrls(this);
	//int xDIB = pI->ImageSize.cx;
	//int yDIB = pI->ImageSize.cy;
	//CRect BoundR(0, 0, 0, 0);
	//pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
	// CImageCtrls* pI = GetImageCtrls();
	auto* pA = GetApertureCtrls();
	if (!GetImageCtrls()->HasImage() || pA->GetShapes().getApertures().empty())
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
	//CBoundCtrls* pB = GetBoundCtrls(this);
	//CImageCtrls* pI = GetImageCtrls(this);
	//int xDIB = pI->ImageSize.cx;
	//int yDIB = pI->ImageSize.cy;
	//CRect BoundR(0, 0, 0, 0);
	//pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
	auto* pA = GetApertureCtrls();
	if (pA->GetShapes().getApertures().empty())
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

	if (pDoc->IsDots())
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
	if (pCtrls->FringeCenterAs == FC_MAX)
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
	if (pCtrls->FringeCenterAs == FC_MIN)
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
	if (pCtrls->FringeCenterAs == FC_MINMAX)
		pCmdUI->SetRadio(TRUE);
	else
		pCmdUI->SetRadio(FALSE);
}


void CImageView::SingleIsoline(int pn, ISO_POINT* plist, double level, int ilevel)
{
	if (pn < 2)
		return;

	CDC* pDC = CDC::FromHandle(hDC);
	CControls* pCtrls = GetControls();

	int n = pn;
	ISO_POINT* list = plist;
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

	while (--n) {
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
	if (retPen) retPen->DeleteObject();
}

void CImageView::OnAddBoundCircle()
{
	if (!GetImageCtrls()->HasImage())
		return;

	ActivateBoundsTool();
	DigitMode::BoundsHandler& handler = m_boundsInputHandler.GetBoundsHandler();
	aperture::TypeLimits currentType = handler.GetShapeType();
	handler.SetShapeType(currentType);
	handler.SetEditMode(DigitMode::ShapeEditMode::AddCircle);
	GetMainFrame()->SetStatusText(_T("Click center and edge to define circular bound"));
	Invalidate(FALSE);
}

void CImageView::OnUpdateAddBound(CCmdUI* pCmdUI)
{
	// Enable only when we have an image
	bool hasImage = GetImageCtrls()->HasImage();
	pCmdUI->Enable(hasImage ? TRUE : FALSE);
	if (!hasImage) {
		pCmdUI->SetRadio(FALSE);
		return;
	}

	// Button should only appear active if the bounds tool is the active tool
	auto* activeTool = GetInputRouter().GetActiveTool();
	bool boundsToolActive = (activeTool == &m_boundsInputHandler);
	if (!boundsToolActive) {
		pCmdUI->SetRadio(FALSE);
		return;
	}

	using DigitMode::ShapeEditMode;
	ShapeEditMode mode = m_boundsInputHandler.GetEditMode();

	bool isThisMode = false;
	switch (pCmdUI->m_nID)
	{
	case ID_ADD_BOUND_CIRCLE:
		isThisMode = (mode == ShapeEditMode::AddCircle);
		break;
	case ID_ADD_BOUND_ELLIPSE:
		isThisMode = (mode == ShapeEditMode::AddEllipse);
		break;
	case ID_ADD_BOUND_RECT:
		isThisMode = (mode == ShapeEditMode::AddRectangle);
		break;
	case ID_ADD_BOUND_POLYGON:
		isThisMode = (mode == ShapeEditMode::AddPolygon);
		break;
	default:
		break;
	}

	// Use radio semantics so only one add-mode button appears pressed
	pCmdUI->SetRadio(isThisMode ? TRUE : FALSE);
}

void CImageView::OnAddBoundEllipse()
{
    if (!GetImageCtrls()->HasImage())
		return;

	ActivateBoundsTool();
	DigitMode::BoundsHandler& handler = m_boundsInputHandler.GetBoundsHandler();
	aperture::TypeLimits currentType = handler.GetShapeType();
	handler.SetShapeType(currentType);
	handler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);
	GetMainFrame()->SetStatusText(_T("Click to define elliptical bound"));
	Invalidate(FALSE);
}
void CImageView::OnAddBoundRect()
{
    if (!GetImageCtrls()->HasImage())
		return;

	ActivateBoundsTool();
	DigitMode::BoundsHandler& handler = m_boundsInputHandler.GetBoundsHandler();
	aperture::TypeLimits currentType = handler.GetShapeType();
	handler.SetShapeType(currentType);
	handler.SetEditMode(DigitMode::ShapeEditMode::AddRectangle);
	GetMainFrame()->SetStatusText(_T("Click corners to create rectangular bound"));
	Invalidate(FALSE);
}
void CImageView::OnAddBoundPolygon()
{
    if (!GetImageCtrls()->HasImage())
		return;

	ActivateBoundsTool();
	DigitMode::BoundsHandler& handler = m_boundsInputHandler.GetBoundsHandler();
	aperture::TypeLimits currentType = handler.GetShapeType();
	handler.SetShapeType(currentType);
	handler.SetEditMode(DigitMode::ShapeEditMode::AddPolygon);
	GetMainFrame()->SetStatusText(_T("Click vertices to create polygonal bound (press Enter when done)"));
	Invalidate(FALSE);
}
void CImageView::OnBoundVisisbility()
{
    // Toggle between APERTURE and INTERNAL bounds type for subsequent Add commands
	ActivateBoundsTool();
	DigitMode::BoundsHandler& handler = m_boundsInputHandler.GetBoundsHandler();
	aperture::TypeLimits currentType = handler.GetShapeType();
	aperture::TypeLimits nextType =
		(currentType == aperture::TypeLimits::INTERNAL)
			? aperture::TypeLimits::APERTURE
			: aperture::TypeLimits::INTERNAL;

	handler.SetShapeType(nextType);
	if (handler.IsDrafting()) {
		handler.CancelDraft();
	}

	LPCTSTR msg = (nextType == aperture::TypeLimits::INTERNAL)
		? _T("Bounds type: INTERNAL (obstructions)")
		: _T("Bounds type: APERTURE (visible pupil)");
	GetMainFrame()->SetStatusText(msg);
	Invalidate(FALSE);
}
void CImageView::OnUpdateBoundVisibility(CCmdUI* pCmdUI)
{
	bool hasImage = GetImageCtrls()->HasImage();
	pCmdUI->Enable(hasImage ? TRUE : FALSE);
	if (!hasImage)
		return;

	aperture::TypeLimits currentType = m_boundsInputHandler.GetBoundsHandler().GetShapeType();
	bool isInternal = (currentType == aperture::TypeLimits::INTERNAL);
	pCmdUI->SetCheck(isInternal ? TRUE : FALSE);
}
void CImageView::OnBoundModeSelect()
{
	ActivateBoundsTool();
	m_boundsInputHandler.SetEditMode(DigitMode::ShapeEditMode::Select);
	GetMainFrame()->SetStatusText(_T("Bounds: Select mode - drag to modify shapes"));
	Invalidate(FALSE);
}
void CImageView::OnUpdateBoundModeSelect(CCmdUI* pCmdUI)
{
	bool hasImage = GetImageCtrls()->HasImage();
	pCmdUI->Enable(hasImage ? TRUE : FALSE);
	if (!hasImage) { pCmdUI->SetRadio(FALSE); return; }

	bool boundsToolActive = (GetInputRouter().GetActiveTool() == &m_boundsInputHandler);
	if (!boundsToolActive) { pCmdUI->SetRadio(FALSE); return; }

	pCmdUI->SetRadio(
		m_boundsInputHandler.GetEditMode() == DigitMode::ShapeEditMode::Select ? TRUE : FALSE
	);
}
void CImageView::OnBoundModeDelete()
{
	ActivateBoundsTool();
	m_boundsInputHandler.SetEditMode(DigitMode::ShapeEditMode::Delete);
	GetMainFrame()->SetStatusText(_T("Bounds: Delete mode - click shapes to remove"));
	Invalidate(FALSE);
}
void CImageView::OnUpdateBoundModeDelete(CCmdUI* pCmdUI)
{
	bool hasImage = GetImageCtrls()->HasImage();
	pCmdUI->Enable(hasImage ? TRUE : FALSE);
	if (!hasImage) { pCmdUI->SetRadio(FALSE); return; }

	bool boundsToolActive = (GetInputRouter().GetActiveTool() == &m_boundsInputHandler);
	if (!boundsToolActive) { pCmdUI->SetRadio(FALSE); return; }

	pCmdUI->SetRadio(
		m_boundsInputHandler.GetEditMode() == DigitMode::ShapeEditMode::Delete ? TRUE : FALSE
	);
}
void CImageView::OnFringesEdit()
{
	ActivateFringeTool();
	m_fringeInputHandler.SetMode(DigitMode::FringeEditMode::Draw);
	GetMainFrame()->SetStatusText(_T("Fringes: Draw mode - click segment to continue"));
	Invalidate(FALSE);
}
// TODO: consider fringes drawing mode swithing scenarios
void CImageView::OnUpdateFringesEdit(CCmdUI *pCmdUI)
{
	bool hasImage = GetImageCtrls()->HasImage();
	pCmdUI->Enable(hasImage ? TRUE : FALSE);
	if (!hasImage) { pCmdUI->SetRadio(FALSE); return; }
	bool boundsToolActive = (GetInputRouter().GetActiveTool() == &m_fringeInputHandler);
	if (!boundsToolActive) { pCmdUI->SetRadio(FALSE); return; }

	pCmdUI->SetRadio(
		m_fringeInputHandler.GetEditMode() == DigitMode::FringeEditMode::Draw ? TRUE : FALSE
	);
}

// ========================================================================
// Undo/Redo Command Handlers
// ========================================================================

void CImageView::OnEditUndo()
{
	DigitMode::CApertureCtrls* pApertureCtrls = GetApertureCtrls(this);
	if (!pApertureCtrls) return;
	
	DigitMode::CommandDispatcher& dispatcher = pApertureCtrls->GetCommandDispatcher();
	if (dispatcher.CanUndo()) {
		dispatcher.Undo();
		Invalidate(FALSE);
		
		// Update status bar
		CString status;
		if (dispatcher.CanUndo()) {
			status.Format(_T("Undone. %s available"), 
				CString(dispatcher.GetUndoLabel().c_str()));
		} else {
			status = _T("Undone. No more undo available");
		}
		GetMainFrame()->SetStatusText(status);
	}
}

void CImageView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	DigitMode::CApertureCtrls* pApertureCtrls = GetApertureCtrls(this);
	if (!pApertureCtrls) {
		pCmdUI->Enable(FALSE);
		return;
	}
	
	DigitMode::CommandDispatcher& dispatcher = pApertureCtrls->GetCommandDispatcher();
	pCmdUI->Enable(dispatcher.CanUndo() ? TRUE : FALSE);
	
	// Update menu text with action name
	CString text = CString(dispatcher.GetUndoLabel().c_str());
	if (!text.IsEmpty()) {
		pCmdUI->SetText(text);
	}
}

void CImageView::OnEditRedo()
{
	DigitMode::CApertureCtrls* pApertureCtrls = GetApertureCtrls(this);
	if (!pApertureCtrls) return;
	
	DigitMode::CommandDispatcher& dispatcher = pApertureCtrls->GetCommandDispatcher();
	if (dispatcher.CanRedo()) {
		dispatcher.Redo();
		Invalidate(FALSE);
		
		// Update status bar
		CString status;
		if (dispatcher.CanRedo()) {
			status.Format(_T("Redone. %s available"), 
				CString(dispatcher.GetRedoLabel().c_str()));
		} else {
			status = _T("Redone. No more redo available");
		}
		GetMainFrame()->SetStatusText(status);
	}
}

void CImageView::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	DigitMode::CApertureCtrls* pApertureCtrls = GetApertureCtrls(this);
	if (!pApertureCtrls) {
		pCmdUI->Enable(FALSE);
		return;
	}
	
	DigitMode::CommandDispatcher& dispatcher = pApertureCtrls->GetCommandDispatcher();
	pCmdUI->Enable(dispatcher.CanRedo() ? TRUE : FALSE);
	
	// Update menu text with action name
	CString text = CString(dispatcher.GetRedoLabel().c_str());
	if (!text.IsEmpty()) {
		pCmdUI->SetText(text);
	}
}

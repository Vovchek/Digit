// ImageDoc.cpp : implementation file
//

#include "stdafx.h"
#include "BaseImageDoc.h"
#include "ImageDoc.h"
#include "ImageView.h"

#include "Utils\FileUtils.h"
#include "Utils\mutils.h"
#include "ImageFeatures\SectionFrame.h"
#include "Utils\Edit\BaseTextDoc.h"
#include "Options\ApproxSetDlg.h"
#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageDoc

IMPLEMENT_DYNCREATE(CImageDoc, CBaseImageDoc)

CImageDoc::CImageDoc()
{
	pHorz = pVert = NULL;
	IsAproxMatrix = FALSE;
	nx = 0;
	ny = 0;
	nLevel = 11;
	pLevel = NULL;
	pX = NULL;
	pY = NULL;
	pMatr = NULL;
	minZ = maxZ = 0.;
}

BOOL CImageDoc::OnNewDocument()
{
	if (!CBaseImageDoc::OnNewDocument())
		return FALSE;

	return TRUE;
}

CImageDoc::~CImageDoc()
{
	if (pHorz) { delete pHorz; pHorz = NULL; }
	if (pVert) { delete pVert; pVert = NULL; }
}

// deprecated/eliminated use Command pattern for undo/redo
void CImageDoc::PrepareOperationUndo(int OperType)
{
	//LastOperationType = OperType;

	//switch (LastOperationType) {
	//case O_ROTATION:
	//case O_INCLINE_CORRECT:
	//case O_GAMMA:
	//case O_GAUSS:
	//case O_ZOOM:
	//case O_CROP:
	//case O_BRITGHTNESS_CONTRAST:
	//case O_EXPAND:
	//	::CopyFile(LPCTSTR(TmpPath), LPCTSTR(UndoTmpPath), FALSE);
	//	break;
	//}
}

void CImageDoc::OnActivate()
{
	CControls* pCtrls = GetControls();
	if (LoadedFileType == T_FRN)
		pCtrls->ActiveEditMode = E_ADD_DOT;
}

void CImageDoc::DeActivateAllMode()
{
	//DeActivateMode(I_BOUNDS_EXT);
	//DeActivateMode(I_BOUNDS_INS);
	DeActivateMode(I_MEASURELINE);
	DeActivateMode(I_FOTO_SECTIONS);
}

// deprecated/eliminated use Command pattern for undo/redo
void CImageDoc::LastOperationUndo()
{
	//CImageCtrls* pImCtrls = GetImageCtrls();
	//switch (LastOperationType) {
	//case O_IMAGE_SCENARIO:
	//	::CopyFile(LPCTSTR(UndoTmpPath), LPCTSTR(TmpPath), FALSE);
	//	ReloadDocument(LPCTSTR(UndoTmpPath));
	//	break;
	//case O_ROTATION:
	//case O_INCLINE_CORRECT:
	//case O_GAMMA:
	//case O_GAUSS:
	//case O_CROP:
	//case O_EXPAND:
	//	::CopyFile(LPCTSTR(UndoTmpPath), LPCTSTR(TmpPath), FALSE);
	//	ReloadDocument(LPCTSTR(UndoTmpPath));
	//	RemoveLastRecordScenario();
	//	break;
	//case O_BRITGHTNESS_CONTRAST:
	//	pImCtrls->kBright = pImCtrls->undokBright;
	//	pImCtrls->kContrast = pImCtrls->undokContrast;
	//	ReloadDocument(LPCTSTR(UndoTmpPath));
	//	RemoveLastRecordScenario();
	//	break;
	//}
	//LastOperationType = O_NO_UNDO;
}

void CImageDoc::ActivateMeasure(BOOL key)
{
	CImageView* pV = GetView();
	pV->Init();
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(pV);
	CControls* pCtrls = GetControls();
	if (key) {
		pCtrls->EnableOptions |= I_MEASURELINE;
	}
	else {
		pCtrls->EnableOptions &= ~I_MEASURELINE;
		pCtrls->EnableOptions &= ~I_MEASURE_ACTIVE;
	}
	pV->Invalidate(FALSE);
}

// deprecated/eliminated switched to CApertureCtrls
void CImageDoc::ActivateExtBounds(BOOL key)
{
//	CImageView* pV = GetView();
//	CControls* pCtrls = GetControls();
//	if (key) {
//		pCtrls->LoadBoundSettings();
//		pCtrls->EnableOptions &= ~I_BOUNDS_INS;
//		pCtrls->EnableOptions |= I_BOUNDS_EXT;
//		if (!pCtrls->EnableCustomDots && !pCtrls->EnableTracker)
//			pCtrls->EnableCustomDots = TRUE;
//	}
//	else {
//		pCtrls->SaveBoundSettings();
//		pCtrls->EnableOptions &= ~I_BOUNDS_EXT;
//		Tracker.SetEnableState(FALSE);
//		//boundCtrls.CustomDots.RemoveAll();
//	}
//	pV->Invalidate(FALSE);
}

// deprecated/eliminated switched to CApertureCtrls
void CImageDoc::ActivateInsBounds(BOOL key)
{
	//CImageView* pV = GetView();
	//CControls* pCtrls = GetControls();
	//if (key) {
	//	pCtrls->EnableOptions &= ~I_BOUNDS_EXT;
	//	pCtrls->EnableOptions |= I_BOUNDS_INS;
	//	if (!pCtrls->EnableCustomDots && !pCtrls->EnableTracker)
	//		pCtrls->EnableCustomDots = TRUE;
	//}
	//else {
	//	pCtrls->EnableOptions &= ~I_BOUNDS_INS;
	//	Tracker.SetEnableState(FALSE);
	//	//boundCtrls.CustomDots.RemoveAll();
	//}
	//pV->Invalidate(FALSE);
}

void CImageDoc::WriteMeasureCtrls()
{
	CImageView* pV = GetView();
	CMainFrame* pMFr = GetMainFrame();
	CDialogBar* pDB = pMFr->GetMeasureDlgBar();
	CMeasureCtrls* pMCtrls = GetMeasureCtrls(pV);
	CImageCtrls* pICtrls = GetImageCtrls(pV);

	CDLine L(pMCtrls->L);
	CWnd* pW;
	CString s;
	if (L.P1.x < 0 || pICtrls->m_pDIB->m_dwHeight - L.P1.y < 0) {
		pW = pDB->GetDlgItem(IDT_X_measure);
		pW->SetWindowText(" ");
		pW = pDB->GetDlgItem(IDT_Y_measure);
		pW->SetWindowText(" ");
	}
	else {
		s.Format("%0.2f", L.P1.x);
		pW = pDB->GetDlgItem(IDT_X_measure);
		pW->SetWindowText(LPCTSTR(s));
		s.Format("%0.2f", pICtrls->m_pDIB->m_dwHeight - L.P1.y);
		pW = pDB->GetDlgItem(IDT_Y_measure);
		pW->SetWindowText(LPCTSTR(s));
	}
	s.Format("%0.2f", L.GetW());
	pW = pDB->GetDlgItem(IDT_W_measure);
	pW->SetWindowText(LPCTSTR(s));
	s.Format("%0.2f", (-1) * L.GetH());
	pW = pDB->GetDlgItem(IDT_H_measure);
	pW->SetWindowText(LPCTSTR(s));
	s.Format("%0.2f", L.GetA());
	pW = pDB->GetDlgItem(IDT_A_measure);
	pW->SetWindowText(LPCTSTR(s));
	s.Format("%0.2f", L.GetD());
	pW = pDB->GetDlgItem(IDT_D_measure);
	pW->SetWindowText(LPCTSTR(s));

}

bool CImageDoc::IsFotoSections()
{
	if ((pHorz && pHorz->GetSafeHwnd()) || (pVert && pVert->GetSafeHwnd()))
		return true;
	else {
		return false;
	}
}

void CImageDoc::ActivateAproximation()
{
	if (!IsAproximation() || IsModified()) {
		CalcAproximation();
	}
}

// temporary check algorithms for restoring topography from isolines
#include <fstream>
#include <memory>
#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"
void CImageDoc::CalcAproximation()
{

	DigitMode::CApertureCtrls* pA = GetApertureCtrls();
	auto &aperture = pA->GetShapes().getVisibleRegion();
	auto mask = pA->GetMaskProvider().getMask();
	
	WavefrontFromContoursInput input(
        aperture, 
		mask, 
		Digit.Fringes
    );
	
	std::string path = GetRealPath();
	
	// Test different solvers
	WavefrontFromContours wf(input);
	
	// 1. Linear interpolation
	{
		WavefrontFromContoursSolver_HorizontalLinear solver;
		auto topogram = wf.run(solver);
		
		auto file = path + "topogram_linear.txt";
		std::ofstream out(file);
		out << topogram;
		file = path + "topogram_linear.mtr";
		std::ofstream outMtr(file);
		topogram.saveMtrMatrix(outMtr);
	}
	
	// 2. Cubic spline interpolation
	{
		WavefrontFromContoursSolver_HorizontalSpline solver;
		auto topogram = wf.run(solver);
		
		auto file = path + "topogram_spline.txt";
		std::ofstream out(file);
		out << topogram;
		file = path + "topogram_spline.mtr";
		std::ofstream outMtr(file);
		topogram.saveMtrMatrix(outMtr);
	}

	//CControls* pCtrls = GetControls();
}

void CImageDoc::CreateAproxImage(CArrayDouble& ArrFNumbers)
{
	int i = 0;
	nx = MApr.GetSizeX();
	ny = MApr.GetSizeY();
	double* _pMatr = MApr.GetMem();
	int n = nx * ny;
	if (pMatr)
		delete[] pMatr;
	pMatr = new double[n];
	if (pX)
		delete[] pX;
	if (pY)
		delete[] pY;
	int max_xy = __max(nx, ny);
	pX = new double[max_xy];
	pY = new double[max_xy];
	double val;
	double min_level = 0.00005;
	for (int i = 0; i < n; i++) {
		val = _pMatr[i];
		if (val == E18)
			val = F_EXCLUDE_VAL;
		pMatr[i] = val;
		if (val == min_level)
			continue;
		minZ = __min(val, minZ);
		maxZ = __max(val, maxZ);
	}
	for (i = 0; i < max_xy; i++) {
		pX[i] = i;
	}
	for (i = 0; i < max_xy; i++) {
		pY[i] = i;
	}

	nLevel = ArrFNumbers.GetSize();
	if (pLevel)
		delete[] pLevel;
	pLevel = new double[nLevel];
	for (i = 0; i < nLevel; i++) {
		pLevel[i] = ArrFNumbers[i];
	}

	//   CMultiDocTemplate* p3D = Get3DTempl();
	//   p3D->OpenDocumentFile(NULL);

}

void CImageDoc::ActivateFotoSections(BOOL key)
{
	CImageView* pV = GetView();
	CControls* pCtrls = GetControls();
	CMDIFrameWnd* pWPar = (CMDIFrameWnd*)GetMainFrame();
	if (!key) {
		if (pHorz) { delete pHorz; pHorz = NULL; }
		if (pVert) { delete pVert; pVert = NULL; }
		pV->Invalidate(FALSE);
		return;
	}
	pV->Init();
	CRect wRect(0, 0, 0, 0);
	CString Title;
	DWORD style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

	if (pHorz && !pHorz->GetSafeHwnd()) {
		delete pHorz; pHorz = NULL;
	}
	if (!pHorz && !pHorz->GetSafeHwnd()) {
		pHorz = new CSectionFrame;
		pHorz->SetRedrawOnClose(pV);
		Title = CRS("Y", "Y");
		pHorz->Create(NULL, LPCTSTR(Title), style, wRect, pWPar);
		pHorz->ShowWindow(SW_SHOWNORMAL);
		pHorz->UpdateWindow();
		pHorz->pGr->SetClientWnd(FRAME_CLIENT);
	}
	if (pVert && !pVert->GetSafeHwnd()) {
		delete pVert; pVert = NULL;
	}
	if (!pVert && !pVert->GetSafeHwnd()) {
		pVert = new CSectionFrame;
		pVert->SetRedrawOnClose(pV);
		Title = CRS("X", "X");
		pVert->Create(NULL, LPCTSTR(Title), style, wRect, pWPar);
		pVert->ShowWindow(SW_SHOWNORMAL);
		pVert->UpdateWindow();
		pVert->pGr->SetClientWnd(FRAME_CLIENT);
	}
	AlignFotoSections();
	ReSetSections();
	pV->Invalidate(FALSE);
}

void CImageDoc::ReSetSections(CPoint l_P)
{
	if (l_P != CPoint(-1, -1)) {
		ReSetHorzSection(l_P);
		ReSetVertSection(l_P);
	}
}

void CImageDoc::ReSetHorzSection(CPoint l_P)
{
	if (pHorz && pHorz->GetSafeHwnd()) {
		pHorz->pGr->SetRegionNumber(1);
		pHorz->pGr->SetRegion(0, 0., 0.05, 1., 0.75);
		pHorz->pGr->SetGridColor(0, RGB(0, 255, 0), RGB(0, 255, 0));
		pHorz->pGr->Enable0OnAxis(FALSE);

		CImageView* pV = GetView();
		CImageCtrls* pImCtrls = GetImageCtrls(pV);

		CRect clientR;	pV->GetClientRect(clientR);
		CRect l_regDIB(pImCtrls->GetDIBRect());
		CRect d_regDIB(l_regDIB);
		pV->DocToClient(d_regDIB);
		CPoint d_P(l_P);
		pV->DocToClient(d_P);

		int iScrollWidth = 0;
		if (pV->GetStyle() & WS_VSCROLL)
			iScrollWidth = 15;

		int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
		int yDIB = pImCtrls->m_pDIB->m_dwHeight;
		double xMin, xMax, minF, maxF;
		xMin = 0.;
		xMax = clientR.Width() + iScrollWidth;
		minF = 0.;
		maxF = 1.;
		if (l_P != CPoint(-1, -1)) {
			int nP = clientR.Width();
			double redP = d_P.x;
			double* pR = new double[nP];
			double* pF = new double[nP];
			pHorz->pGr->RemoveAllLines(0);
			pV->GetXPixelLine(d_P, pR, pF, nP);
			double sx = nP / 5;
			double sy = 0.5;
			pHorz->pGr->SetLimits(0, xMin, xMax, minF, maxF);
			pHorz->pGr->SetGrid(0, 0., nP / 2, sx, sy, 1, 1);
			pHorz->pGr->SetGridDrawStatus(0, TRUE, TRUE, TRUE);
			pHorz->pGr->AddLine(0, RGB(0, 255, 0), 1, nP, pR, pF);

			pR[0] = pR[1] = redP;
			pF[0] = 0.; pF[1] = 1.;
			pHorz->pGr->AddLine(0, RGB(255, 0, 0), 1, 2, pR, pF);

			CString s;
			if (d_regDIB.PtInRect(d_P)) {
				s.Format("Y = %d", yDIB - l_P.y);
			}
			else {
				s = "Y";
			}
			pHorz->SetWindowText(s);
			delete[] pR;
			delete[] pF;
		}
		pHorz->pGr->RefreshPicture();
	}
}

void CImageDoc::ReSetVertSection(CPoint l_P)
{
	if (pVert && pVert->GetSafeHwnd()) {
		pVert->pGr->SetRegionNumber(1);
		pVert->pGr->SetRegion(0, 0.2, 0., 0.7, 1.);
		pVert->pGr->SetGridColor(0, RGB(0, 255, 0), RGB(0, 255, 0));
		pVert->pGr->Enable0OnAxis(FALSE);

		CImageView* pV = GetView();
		CImageCtrls* pImCtrls = GetImageCtrls(pV);

		CRect clientR;	pV->GetClientRect(clientR);
		CRect l_regDIB(pImCtrls->GetDIBRect());
		CRect d_regDIB(l_regDIB);
		pV->DocToClient(d_regDIB);
		CPoint d_P(l_P);
		pV->DocToClient(d_P);

		int iScrollWidth = 0;
		if (pV->GetStyle() & WS_HSCROLL)
			iScrollWidth = 15;

		int xDIB = pImCtrls->m_pDIB->m_dwWidth;
		int yDIB = pImCtrls->m_pDIB->m_dwHeight;

		double xMin, xMax, minF, maxF;
		xMin = 0.;
		xMax = 1.;
		minF = -iScrollWidth;
		maxF = clientR.Height();
		if (l_P != CPoint(-1, -1)) {
			int nP = clientR.Height();
			double* pR = new double[nP];
			double* pF = new double[nP];
			double redP = clientR.Height() - d_P.y;
			pVert->pGr->RemoveAllLines(0);
			pV->GetYPixelLine(d_P, pR, pF, nP);
			double sx = 0.5;
			double sy = nP / 5;
			pVert->pGr->SetLimits(0, xMin, xMax, minF, maxF);
			pVert->pGr->SetGrid(0, minF + (maxF - minF) / 2, 0, sx, sy, 1, 1);
			pVert->pGr->SetGridDrawStatus(0, TRUE, TRUE, TRUE);
			pVert->pGr->AddLine(0, RGB(0, 255, 0), 1, nP, pF, pR);

			CString s;
			if (d_regDIB.PtInRect(d_P)) {
				s.Format("X = %d", l_P.x + 1);
			}
			else {
				s = "X";
			}
			pVert->SetWindowText(s);
			pF[0] = pF[1] = redP;
			pR[0] = 0.; pR[1] = 1.;
			pVert->pGr->AddLine(0, RGB(255, 0, 0), 1, 2, pR, pF);
			delete[] pR;
			delete[] pF;
		}
		pVert->pGr->RefreshPicture();
	}
}

void CImageDoc::AlignFotoSections(BOOL ActivateImage/*=TRUE*/)
{
	CRect wRect, posRect;
	CImageView* pV = GetView();
	WINDOWPLACEMENT wp;
	pV->GetParentFrame()->GetWindowPlacement(&wp);
	wRect = wp.rcNormalPosition;
	int W = wRect.Width();
	int H = wRect.Height();

	posRect.left = wRect.left;
	posRect.top = wRect.bottom;
	posRect.right = wRect.right;
	posRect.bottom = posRect.top + 150;
	W = posRect.Width();
	H = posRect.Height();
	if (pHorz && pHorz->GetSafeHwnd()) {
		pHorz->SetWindowPos(NULL, posRect.left, posRect.top,
			posRect.Width(), posRect.Height(), SWP_SHOWWINDOW);
		pHorz->UpdateWindow();
	}
	posRect.left = wRect.right;
	posRect.top = wRect.top;
	posRect.right = posRect.left + 150;
	posRect.bottom = wRect.bottom;
	W = posRect.Width();
	H = posRect.Height();
	if (pVert && pVert->GetSafeHwnd()) {
		pVert->SetWindowPos(NULL, posRect.left, posRect.top,
			posRect.Width(), posRect.Height(), SWP_SHOWWINDOW);
		pVert->UpdateWindow();
	}

	if (ActivateImage)
		GetMainFrame()->MDIActivate((CWnd*)pV->GetParentFrame());
}

CImageView* CImageDoc::GetView()
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CImageView)))
			return (CImageView*)pView;
	}
	return NULL;
}

BEGIN_MESSAGE_MAP(CImageDoc, CBaseImageDoc)
	//{{AFX_MSG_MAP(CImageDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDoc diagnostics

#ifdef _DEBUG
void CImageDoc::AssertValid() const
{
	CBaseImageDoc::AssertValid();
}

void CImageDoc::Dump(CDumpContext& dc) const
{
	CBaseImageDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImageDoc serialization

void CImageDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CImageDoc commands
void CImageDoc::SetTitle(LPCTSTR lpszTitle)
{
	CString name = GetPathName();
	int iP = name.ReverseFind('\\');
	name = name.Mid(iP + 1);

	CDocument::SetTitle(LPCTSTR(name));
}

void CImageDoc::SetZoomToTitle()
{
	CImageView* pV = GetView();
    float zLevel = 1.0f;
    if (pV) zLevel = (float)pV->GetViewTransform().GetScale();
	CString Title, s;
	Title = GetTitle();
	int iP = Title.Find(" @");
	if (iP != -1) {
		Title = Title.Left(iP);
	}
	s.Format(" @ %d", (int)(zLevel * 100));
	Title += (s + "%");
	CDocument::SetTitle(LPCTSTR(Title));
}
//
BOOL CImageDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	TRACE("CImageDoc::OnOpenDocument(%s)\n", lpszPathName ? lpszPathName : "NULL");

	LoadedFileType = FileType(lpszPathName);
	if (LoadedFileType >= T_BMP && LoadedFileType <= T_TIF) {
		LoadedFileType = T_PIC;
	}
	else if (LoadedFileType != T_ZAP && LoadedFileType != T_FRN) {
		TRACE("CImageDoc::OnOpenDocument - returning FALSE (invalid file type)\n");
		return FALSE;
	}

	CString ImageFileName = lpszPathName;

	CSize cs(100, 100);
	if (LoadedFileType != T_PIC)
	{
		GetImageFileName(ImageFileName, cs);
		imageCtrls.ImageSize = cs;
	}

	if (ImageFileName.IsEmpty())
	{ // no or broken Image ref in zap or frn
		NUMBERING_INTERFEROGRAM_INFO IntInfo;
		CString s = lpszPathName;
		auto res{ FALSE };
		switch (LoadedFileType) {
		case T_ZAP:
			res = ReadZAPData(s, IntInfo);
			break;
		case T_FRN:
			res = ReadFRNData(s, IntInfo);
			break;
		default:
			res = FALSE;
			break;
		}
		if (res)
		{
			cs.cx = IntInfo.ImageSize[0];
			cs.cy = IntInfo.ImageSize[1];
			imageCtrls.ImageSize = cs;
			TRACE("CImageDoc::OnOpenDocument - returning TRUE (metadata only)\n");
			return TRUE;
		}
		else {
			TRACE("CImageDoc::OnOpenDocument - returning FALSE (failed to read metadata)\n");
			return FALSE;
		}
	}
	else
	{
		if (!CBaseImageDoc::OnOpenDocument(LPCTSTR(ImageFileName))) {
			TRACE("CImageDoc::OnOpenDocument - returning FALSE (base OnOpenDocument failed)\n");
			return FALSE;
		}
	}

	CControls* pCtrls = GetControls();
	pCtrls->OnOpenImageDocument();

	if (!ReloadDocument(LPCTSTR(ImageFileName))) {
		TRACE("CImageDoc::OnOpenDocument - returning FALSE (ReloadDocument failed)\n");
		return FALSE;
	}

	if (!(pCtrls->ViewState & V_INTERFEROGRAM))
		pCtrls->ViewState |= V_INTERFEROGRAM;

	TRACE("CImageDoc::OnOpenDocument - returning TRUE\n");
	return TRUE;
}
//
BOOL CImageDoc::ReloadDocument(LPCTSTR lpszImagePathName/*NULL*/)
{
	TRACE("CImageDoc::ReloadDocument(%s)\n", lpszImagePathName ? lpszImagePathName : "NULL");

	if (lpszImagePathName == NULL) {
		lpszImagePathName = LPCTSTR(TmpPath);
	}
	if (!CBaseImageDoc::ReloadDocument(lpszImagePathName)) {
		TRACE("CImageDoc::ReloadDocument - returning FALSE (base ReloadDocument failed)\n");
		return FALSE;
	}

	CControls* pCtrls = GetControls();
	pCtrls->ActiveEditMode = -1;
	CImageView* pV = GetView();
	pV->Invalidate(FALSE);

	TRACE("CImageDoc::ReloadDocument - returning TRUE\n");
	return TRUE;
}

void CImageDoc::SetTextWndInfo()
{
	CBaseTextDoc* pDoc = (CBaseTextDoc*)GetDocTextInfo();
	if (pDoc) {
		pDoc->DeleteContents();
		CString Title, Contents;
		Title = GetTitle();
		GetImageInfo(Contents);
		pDoc->SetContents(LPCTSTR(Title), LPCTSTR(Contents));
	}
}

void CImageDoc::GetImageInfo(CString& Info)
{
	CImageCtrls* pImg = GetImageCtrls();
	CString s;
	LPCTSTR text = CRS("Размер :", "Size :");
	s.Format("%d x %d\r\n", pImg->m_pDIB->m_dwWidth, pImg->m_pDIB->m_dwHeight);
	s = text + s;
	Info += s;
}

void CImageDoc::OnCloseDocument()
{
	if (IsLastActiveDoc()) {
		CControls* pCtrls = GetControls();
		//	  pCtrls->EnableOptions = NULL;
		pCtrls->EnableTracker = FALSE;
		pCtrls->EnableCustomDots = FALSE;
	}
	CBaseImageDoc::OnCloseDocument();
}

void CImageDoc::DeleteContents()
{
	CControls* pCtrls = GetControls();
	if (pCtrls) {
		pCtrls->OnCloseImageDocument();
	}

	CBaseImageDoc::DeleteContents();
}

void CImageDoc::AutoDigit()
{
	Digit.Auto();
	GetMainFrame()->SetCurrentNumber(Digit.CurrentNumber);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Digit.OnKeyDown(nChar, nRepCnt, nFlags);
	GetMainFrame()->SetCurrentNumber(Digit.CurrentNumber);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::OnLButDown(CPoint P)
{
	SetModifiedFlag(TRUE);
	CControls* pCtrls = GetControls();
	switch (pCtrls->ActiveEditMode) {
	case E_ADD_DOT:
		AddDot(P);
		break;
	case E_DELETE_DOT:
		RemoveDot(P);
		break;
	case E_DELETE_FRINGE:
		RemoveFringe(P);
		break;
	case E_ADD_SECTION:
		// deprecated/eliminated - no zap sections anymore
		//AddZapSection(P.y);
		break;
	case E_DELETE_SECTION:
		// deprecated/eliminated - no zap sections anymore
		//DeleteZapSection(P.y);
		break;
	case E_SECTION_LEFT:
		SectionLeft(P);
		break;
	case E_SECTION_RIGHT:
		SectionRight(P);
		break;
	case E_RENUM_FRINGE:
		RenumFringe(P);
		break;
	case E_RENUM_DOT:
		RenumDot(P);
		break;
	}
}

void CImageDoc::OnRButDown(CPoint P)
{
	SelectMainDot(P);
}

void CImageDoc::SelectMainDot(CPoint P)
{
	CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.SelectMainDot(P, DotSide);

	GetMainFrame()->SetCurrentNumber(Digit.CurrentNumber);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::SelectMainDot(int iSec, double Number)
{
	Digit.SelectMainDot(iSec, Number);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::AddDot(CPoint P)
{
	CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.AddDot(P, DotSide);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::RemoveDot(CPoint P)
{
	CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.RemoveDot(P, DotSide);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::RemoveFringe(CPoint P)
{
	CControls* pCtrls = GetControls();
	int DotSide;
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.RemoveFringe(P, DotSide);
	GetView()->Invalidate(FALSE);
}

// deprecated/eliminated - no zap sections anymore
void CImageDoc::AddZapSection(int iy)
{
	//CBoundCtrls* pB = GetBoundCtrls();
	//CImageCtrls* pI = GetImageCtrls();
	//int xDIB = pI->ImageSize.cx;
	//int yDIB = pI->ImageSize.cy;
	//CRect BoundR;
	//if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
	//	return;
	//int begY = BoundR.top;
	//int i = iy - begY;
	//Digit.AddZapSection(iy);
	//GetView()->Invalidate(FALSE);
}

// deprecated/eliminated - no zap sections anymore
void CImageDoc::DeleteZapSection(int iy)
{
	//Digit.DeleteZapSection(iy);
	//GetView()->Invalidate(FALSE);
}

// deprecated/eliminated - no zap sections anymore
void CImageDoc::SectionLeft(CPoint P)
{
	//CControls* pCtrls = GetControls();
	//int DotSide;
	//pCtrls->GetCorrectDotSize(DotSide, this);
	//Digit.SectionLeft(P, DotSide);
	//GetView()->Invalidate(FALSE);
}

// deprecated/eliminated - no zap sections anymore
void CImageDoc::SectionRight(CPoint P)
{
	//CControls* pCtrls = GetControls();
	//int DotSide;
	//pCtrls->GetCorrectDotSize(DotSide, this);
	//Digit.SectionRight(P, DotSide);
	//GetView()->Invalidate(FALSE);
}

void CImageDoc::RenumFringe(CPoint P)
{
	int DotSide;
	CControls* pCtrls = GetControls();
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.RenumFringe(P, DotSide);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::RenumDot(CPoint P)
{
	int DotSide;
	CControls* pCtrls = GetControls();
	pCtrls->GetCorrectDotSize(DotSide, this);
	Digit.RenumDot(P, DotSide);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::OnNumberMinus()
{
	Digit.NumberMinus();
	GetMainFrame()->SetCurrentNumber(Digit.CurrentNumber);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::OnNumberPlus()
{
	Digit.NumberPlus();
	GetMainFrame()->SetCurrentNumber(Digit.CurrentNumber);
	GetView()->Invalidate(FALSE);
}

void CImageDoc::ClearDigit()
{
	Digit.Clear();
	IsAproxMatrix = FALSE;
	GetView()->Invalidate(FALSE);
}

bool CImageDoc::IsDots()
{
	return Digit.IsDots();
}

BOOL CImageDoc::IsAproximation()
{
	return IsAproxMatrix;
}

bool CImageDoc::IsDotUnderCursor(CPoint P)
{
	int DotSide, idx;
	CControls* pCtrls = GetControls();
	pCtrls->GetCorrectDotSize(DotSide, this);
	return Digit.IsDotUnderCursor(P, DotSide, idx);
}

bool CImageDoc::IsLockedDot()
{
	return Digit.IsLockedDot();
}

bool CImageDoc::LockDot(CPoint P, BOOL Enable)
{
	int DotSide;
	CControls* pCtrls = GetControls();
	pCtrls->GetCorrectDotSize(DotSide, this);
	return Digit.LockDot(P, DotSide, Enable);
}

void CImageDoc::SetLockedDotPos(CPoint P)
{
	Digit.SetLockedDotPos(P);
}

void CImageDoc::GetLockedDotPos(CPoint& P1)
{
	Digit.GetLockedDotPos(P1);
}

bool CImageDoc::IsInterferogram()
{
	CImageCtrls* pImCtrls = GetImageCtrls();
	if (pImCtrls->m_pDIB)
		return true;
	else
		return false;
}

bool CImageDoc::IsSections()
{
	return Digit.IsSections();
}

bool CImageDoc::IsZapSections()
{
	return Digit.IsZapSections();
}

bool CImageDoc::IsLockedZapSection()
{
	return Digit.IsLockedZapSection();
}

bool CImageDoc::IsZapSectionUnderCursor(CPoint P)
{
	int idx;
	return Digit.IsZapSectionUnderCursor(P, idx);
}

bool CImageDoc::LockZapSection(CPoint P, BOOL Enable)
{
	//return Digit.LockZapSection(P, Enable);
	return false;
}

void CImageDoc::SetLockedZapSectionYPos(int iy)
{
	//Digit.SetLockedZapSectionYPos(iy);
}

void CImageDoc::GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2)
{
	//Digit.GetLockedZapSectionXYPos(P1, P2);
	//Digit.RemoveDotZAPSection(Digit.idxDragZapLine);
}

BOOL CImageDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return CBaseImageDoc::OnSaveDocument(lpszPathName);
}

BOOL CImageDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	BOOL res = CheckForSave();
	if (!res)
		return FALSE;
	return CBaseImageDoc::CanCloseFrame(pFrame);
}

BOOL CImageDoc::CheckForSave()
{
	if (IsModified()) {
		CString Title = GetTitle();
		LPCTSTR text = CRS("Сохранить оцифровку\r\n", "Save digitization of\r\n");
		CString mes;
		mes += text; mes += Title;
		LPCTSTR war = CRS("Вопрос", "Question");
		int res = GetView()->MessageBox(text, war, MB_ICONQUESTION | MB_YESNOCANCEL);
		if (res == IDYES)
			OnFileSave();
		else if (res == IDCANCEL)
			return FALSE;
	}
	SetModifiedFlag(FALSE);
	return TRUE;
}

void CImageDoc::OnFileSave()
{
	CMainFrame* pMFr = GetMainFrame();
	pMFr->GetImageInfo(Digit.Comments, Digit.ScaleFactor, Digit.Rotation);
	CString path = GetPathName();
	CString ext = path.Right(3);
	ext.MakeLower();
	if (ext == "zap" || ext == "frn") {
		if (Digit.Save(LPCTSTR(path), 2))
			SetModifiedFlag(FALSE);
	}
	else {
		OnFileSaveAs();
	}
}

void CImageDoc::OnFileSaveAs()
{
	CFileDialog fileDlg(FALSE);

	CControls* pCtrls = GetControls();
	CString str;
	int nFilters = 0;

	if (!pCtrls->GetCorrectFilterForSave(str, nFilters)) // modifies str, nFilters
		return;

	CMainFrame* pMFr = GetMainFrame();
	pMFr->GetImageInfo(Digit.Comments, Digit.ScaleFactor, Digit.Rotation);

	CString path = GetPathName();
	int iP = path.ReverseFind('\\');
	CString name = path.Mid(iP + 1);
	name = name.Left(name.GetLength() - 4);

	TCHAR strName[_MAX_PATH];
	strcpy(strName, LPCTSTR(name));
	fileDlg.m_ofn.lpstrFile = strName;
	TCHAR Str[_MAX_PATH];
	fileDlg.m_ofn.lpstrFilter = LPCTSTR(str);
	if (ReadPath("SAVE_INDEX_FILE", Str, GetIniFile())) {
		fileDlg.m_ofn.nFilterIndex = atoi(Str);
	}
	else
		fileDlg.m_ofn.nFilterIndex = FilterIndex::Filter_FRN;

	if (fileDlg.m_ofn.nFilterIndex > static_cast<unsigned>(nFilters))
		fileDlg.m_ofn.nFilterIndex = FilterIndex::Filter_FRN;

	if (fileDlg.m_ofn.nFilterIndex < 3)
		fileDlg.m_ofn.lpstrDefExt = "frn";

	fileDlg.m_ofn.Flags |= (OFN_EXTENSIONDIFFERENT | OFN_OVERWRITEPROMPT);

	CString s = GetPathName();
	if (!s.IsEmpty()) {
		int iP = s.ReverseFind('\\');
		s = s.Left(iP);
		fileDlg.m_ofn.lpstrInitialDir = s.GetBuffer(s.GetLength());
		s.ReleaseBuffer();
	}

	else if (ReadPath("LOAD_FILE", Str, GetIniFile()))
		fileDlg.m_ofn.lpstrInitialDir = Str;
	else
		fileDlg.m_ofn.lpstrInitialDir = NULL;

	if (fileDlg.DoModal() == IDOK) {
		CString FileName = fileDlg.GetPathName();
		int iP = FileName.ReverseFind('\\');
		CString sav = FileName.Left(iP);
		SavePath("LOAD_FILE", sav, GetIniFile());
		sav.Format("%d", fileDlg.m_ofn.nFilterIndex);
		SavePath("SAVE_INDEX_FILE", sav, GetIniFile());
		CString Ext = fileDlg.GetFileExt();
		if (Ext.IsEmpty()) {
			if (fileDlg.m_ofn.nFilterIndex == FilterIndex::Filter_FRN ||
				fileDlg.m_ofn.nFilterIndex == FilterIndex::Filter_WinFringe_FRN)
				FileName += ".frn";
			else if (fileDlg.m_ofn.nFilterIndex == FilterIndex::Filter_MTR)
				FileName += ".mtr";
		}

		pMFr->GetImageInfo(Digit.Comments, Digit.ScaleFactor, Digit.Rotation);
		if (Digit.Save(LPCTSTR(FileName), int(fileDlg.m_ofn.nFilterIndex)))
			SetModifiedFlag(FALSE);

	}
}

void CImageDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	if (IsDots())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CImageDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	if (IsDots())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


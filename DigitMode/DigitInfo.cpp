#include "DigitInfo.h"
#include "Utils\mutils.h"
#include "Utils\middle.h"
#include "MGTools\Include\Utils\Utils.h"
#include <math.h>
#include <filesystem>
#include <string>
#include <windowsx.h>

CDigitInfo::CDigitInfo()
{
	Init();
}

CDigitInfo::~CDigitInfo()
{
	Clear();
}

void CDigitInfo::Init()
{
	ny_buf_line = 0;
	buf_line = NULL;
	SecSegm = -1;
	isInsideScreen = FALSE;
	numStep = 1.;
	MainFringeNumber = -1000.;
	idxMainSection = -1;
	idxDragZapLine = -1;
	idxDragDot = -1;
	idxMainDot = -1;
	HandSetZapLines = FALSE;
	CorrectionSecSegm = 0.3;
	CurrentNumber = 0.;

	Comments = _T("No comments");
	ScaleFactor = 1.;
	Rotation = 0.;
	
	// NEW: Initialize fringe model
	m_bUseFringeModel = FALSE;
	idxDraggedPoint.Clear();
	idxMainPoint.Clear();
}

BOOL CDigitInfo::IsDigiting()
{
	if (Sections.GetSize())
		return TRUE;
	else
		return FALSE;
}

void CDigitInfo::Delete_buf_line()
{
	if (buf_line) {
		for (int i = 0; i < ny_buf_line; i++) {
			free(buf_line[i]);
		}
		free(buf_line);
	}
	buf_line = NULL;
}

void CDigitInfo::Init_buf_line(int ny, int n)
{
	int i = 0;
	Delete_buf_line();
	ny_buf_line = ny;
	buf_line = (int**)malloc(sizeof(int) * (ny_buf_line));
	for (i = 0; i < ny; i++) {
		buf_line[i] = (int*)malloc(sizeof(int) * (n));
	}
	for (i = 0; i < ny_buf_line; i++) {
		buf_line[i][0] = -1;
		buf_line[i][1] = -1;
		buf_line[i][2] = -1;
		buf_line[i][3] = -1;
	}
}

void CDigitInfo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bUseFringeModel) {
		if (!idxMainPoint.IsValid()) return;
		CFringeSegment* pFr = GetFringe(idxMainPoint.iFringe);
		if (!pFr) return;
		int currentIdx = idxMainPoint.iPoint;
		int nextIdx = -1;

		if (nChar == VK_LEFT) {
			nextIdx = currentIdx - 1;
		}
		else if (nChar == VK_RIGHT) {
			nextIdx = currentIdx + 1;
		}
		else if (nChar == VK_UP) {
			nextIdx = currentIdx - 1;
		}
		else if (nChar == VK_DOWN) {
			nextIdx = currentIdx + 1;
		}

		if (nextIdx >= 0 && nextIdx < pFr->GetPointCount()) {
			idxMainPoint.iPoint = nextIdx;
			CurrentNumber = pFr->GetNumber();
		}
		return;
	}

	int idx;
	CDPoint dP;
	if (idxMainDot != -1) {
		dP = Dots[idxMainDot].P;
		if (nChar == VK_LEFT) {
			if (GetNextDotInSection(Dots[idxMainDot].iZapSec, -1, idx, dP)) {
				idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_RIGHT) {
			if (GetNextDotInSection(Dots[idxMainDot].iZapSec, 1, idx, dP)) {
				idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_UP) {
			if (GetNextDotInFringe(Dots[idxMainDot].Number, -1, idx, dP)) {
			idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
		else if (nChar == VK_DOWN) {
			if (GetNextDotInFringe(Dots[idxMainDot].Number, 1, idx, dP)) {
			 idxMainDot = idx;
				CurrentNumber = Dots[idx].Number;
			}
		}
	}
}

void CDigitInfo::Auto()
{
	::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	Clear(FALSE);
	CreateBufLine();
	CreateRedCenters();
	SelectFringeStep();
	SelectMainSection();

	CreateNumLines();
	if (!isInsideScreen) {
		SelectMainFringe();
		CorrectNumbers();
	}

	CreateZAPSections();
	// sort dots by increasing fringe,then by Y downwards
	SortDotsFY();
	SelectMainDot();
	Delete_buf_line();
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	m_bUseFringeModel = false; // TODO: update after transition
	SyncFringesToDots();
	m_bUseFringeModel = true;
}

void CDigitInfo::CreateBufLine()
{
	CreateBufLineAperture();
	CreateBufLineOnstruction();
}

void CDigitInfo::CreateBufLineAperture()
{
	CBoundCtrls* pB = GetBoundCtrls();
	if (pB->ExtBoundType == BOUND_POLYGON)
		CreateBufLineApertureComplex();
	else
		CreateBufLineApertureSimple();
}

void CDigitInfo::CreateBufLineOnstruction()
{
	CBoundCtrls* pB = GetBoundCtrls();
	if (pB->InsBoundType == BOUND_POLYGON)
		CreateBufLineObstructionComplex();
	else
		CreateBufLineObstructionSimple();
}

void CDigitInfo::CreateBufLineApertureSimple()
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();

	CDPoint cent;
	int naP;
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE)) {
		TRACE("CreateBufLineApertureSimple: GetExtCorBound FAILED!\n");
		return;
	}
	TRACE("CreateBufLineApertureSimple: xDIB=%d, yDIB=%d, BoundR: top=%d, bottom=%d, left=%d, right=%d\n",
		xDIB, yDIB, BoundR.top, BoundR.bottom, BoundR.left, BoundR.right);
	TRACE("CreateBufLineApertureSimple: ExtBoundType=%d, ArrEll.size=%d\n", pB->ExtBoundType, pB->ArrEll.GetSize());
	
	int n = 4;
	int x, y;
	int l_x, r_x;
	double a, b;
	int ny = BoundR.Height() + 1;

	Init_buf_line(ny, n);

	// Aperture
	if (pB->ExtBoundType == BOUND_ROUND || pB->ExtBoundType == BOUND_ELLIPSE) {
		naP = ny / 2;
		a = BoundR.Width() / 2.;
		b = ny / 2.;
		cent = BoundR.CenterPoint();
		TRACE("CreateBufLineApertureSimple: ELLIPSE mode, naP=%d, a=%f, b=%f, cent=(%f,%f)\n",
			naP, a, b, cent.x, cent.y);
		for (int i = 0; i < naP; i++) {
			y = (int)(BoundR.top - cent.y + i);
			x = (int)((1. - y * y / b / b) * a * a);
			if (!i) x = 0;
			else {
				float ix = x;
				ix = (fabs(ix));
				x = sqrt(ix);
			}
			l_x = (int)((int)-x + cent.x);
			r_x = (int)((int)x + cent.x);
			if (l_x < 0) l_x = 0;
			if (r_x > xDIB) r_x = xDIB;
			buf_line[i][0] = l_x;
			buf_line[i][1] = r_x;
			buf_line[naP * 2 - i - 1][0] = l_x;
			buf_line[naP * 2 - i - 1][1] = r_x;
			buf_line[i][2] = -1;
			buf_line[i][3] = -1;
			buf_line[naP * 2 - i - 1][2] = -1;
			buf_line[naP * 2 - i - 1][3] = -1;
		}
		TRACE("CreateBufLineApertureSimple: First line buf_line[0]: [%d,%d], Last line buf_line[%d]: [%d,%d]\n",
			buf_line[0][0], buf_line[0][1], ny-1, buf_line[ny-1][0], buf_line[ny-1][1]);
	}
	else if (pB->ExtBoundType == BOUND_RECT) {
		for (int i = 0; i < ny; i++) {
			l_x = (int)BoundR.left;
			r_x = (int)BoundR.right;
			if (l_x < 0) l_x = 0;
			if (r_x > xDIB) r_x = xDIB;
			buf_line[i][0] = l_x;
			buf_line[i][1] = r_x;
			buf_line[i][2] = -1;
			buf_line[i][3] = -1;
		}
		TRACE("CreateBufLineApertureSimple: RECT mode, all lines set to [%d,%d]\n", l_x, r_x);
	}
}

void CDigitInfo::CreateBufLineApertureComplex()
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pImCtrls = GetImageCtrls();
	int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
	int yDIB = pImCtrls->m_pDIB->m_dwHeight;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int n = 4;
	int ny = BoundR.Height() + 1;

	Init_buf_line(ny, n);

	int l_x = BoundR.left;
	int r_x = BoundR.right + 1;
	int t_y = BoundR.top;
	int b_y = BoundR.bottom + 1;

	XYPoint P;
	int l_b, r_b, ix, iy;
	CArrayXYEllipse ArrEll;
	CArrayXYPolygon ArrPlg;
	CArrayXYRect ArrRect;
	BOOL res = pB->GetPartsOfContours(EXTERNAL, ArrEll, ArrRect, ArrPlg);
	for (iy = t_y; iy < b_y; iy++) {
		l_b = r_b = -1;
		for (int ix = l_x; ix < r_x; ix++) {
			P.X = ix; P.Y = iy;
			if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
				l_b = ix;
				break;
			}
		}
		for (ix = r_x - 1; ix > l_x - 1; ix--) {
			P.X = ix; P.Y = iy;
			if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
				r_b = ix;
				break;
			}
		}
		buf_line[iy - t_y][0] = l_b;
		buf_line[iy - t_y][1] = r_b;
	}
}

void CDigitInfo::CreateBufLineObstructionSimple()
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();

	CDPoint cent;
	int naP;
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundRExt;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundRExt, FALSE, TRUE))
		return;
	CRect BoundR;
	if (!pB->GetInsCorBound(pB->InsBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int n = 4;
	int x, y;
	int l_x, r_x, i;
	double a, b;
	int ny = BoundR.Height() + 1;

	// Obstruction
	if (pB->InsBoundType != -1) {
		isInsideScreen = TRUE;
		if (pB->InsBoundType == BOUND_ROUND || pB->InsBoundType == BOUND_ELLIPSE) {
			naP = (int)(BoundR.Height() / 2);
			int Sh = BoundR.top - BoundRExt.top;
			a = BoundR.Width() / 2.;
			b = BoundR.Height() / 2.;
			cent = BoundR.CenterPoint();
			for (i = 0; i < naP; i++) {
				y = (int)(BoundR.top - cent.y + i);
				x = (int)((1. - y * y / b / b) * a * a);
				if (!i) x = 0.;
				else {
					float ix = x;
					ix = (fabs(ix));
					x = sqrt(ix);
				}
				l_x = (int)((int)-x + cent.x);
				r_x = (int)((int)x + cent.x);
				if (l_x < 0) l_x = 0;
				if (r_x > xDIB) r_x = xDIB;
				buf_line[i + Sh][2] = l_x;
				buf_line[i + Sh][3] = r_x;
				buf_line[naP * 2 - i + Sh - 1][2] = l_x;
				buf_line[naP * 2 - i + Sh - 1][3] = r_x;
			}
		}
		else if (pB->InsBoundType == BOUND_RECT) {
			for (int i = 0; i < ny; i++) {
				l_x = (int)BoundR.left;
				r_x = (int)BoundR.right;
				if (l_x < 0) l_x = 0;
				if (r_x > xDIB) r_x = xDIB;
				buf_line[i][2] = l_x;
				buf_line[i][3] = r_x;
			}
		}
	}
}

void CDigitInfo::CreateBufLineObstructionComplex()
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pImCtrls = GetImageCtrls();
	int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
	int yDIB = pImCtrls->m_pDIB->m_dwHeight;

	CRect BoundRExt;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundRExt, FALSE, TRUE))
		return;
	int ext_t_y = BoundRExt.top;

	CRect BoundR;
	if (!pB->GetInsCorBound(pB->InsBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int n = 4;
	int ny = BoundR.Height() + 1;

	int l_x = BoundR.left;
	int r_x = BoundR.right + 1;
	int t_y = BoundR.top;
	int b_y = BoundR.bottom + 1;

	XYPoint P;
	int l_b, r_b, ix, iy;
	CArrayXYEllipse ArrEll;
	CArrayXYPolygon ArrPlg;
	CArrayXYRect ArrRect;
	BOOL res = pB->GetPartsOfContours(INTERNAL, ArrEll, ArrRect, ArrPlg);
	for (iy = t_y; iy < b_y; iy++) {
		l_b = r_b = -1;
		for (int ix = l_x; ix < r_x; ix++) {
			P.X = ix; P.Y = iy;
			if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
				l_b = ix;
				break;
			}
		}
		for (ix = r_x - 1; ix > l_x - 1; ix--) {
			P.X = ix; P.Y = iy;
			if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
				r_b = ix;
				break;
			}
		}
		buf_line[iy - ext_t_y][2] = l_b;
		buf_line[iy - ext_t_y][3] = r_b;
	}
}

void CDigitInfo::CreateRedCenters()
{
	HidenDots.RemoveAll();
	Sections.RemoveAll();
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	CControls* pCtrls = GetControls();
	BOOL bPrePadded = pI->m_pDIB->m_bIsPadded;
	if (bPrePadded)
		pI->m_pDIB->PubUnPadBits();

	unsigned char* line = (unsigned char*)malloc(pI->m_pDIB->m_dwWidth * sizeof(unsigned char));
	unsigned char* inv_line = (unsigned char*)malloc(pI->m_pDIB->m_dwWidth * sizeof(unsigned char));
	RGBQUAD rgbPix;
	BYTE Pixel;
	int xDIB = pI->m_pDIB->m_dwWidth;
	int yDIB = pI->m_pDIB->m_dwHeight;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE)) {
		TRACE("CreateRedCenters: GetExtCorBound FAILED!\n");
		return;
	}
	TRACE("CreateRedCenters: xDIB=%d, yDIB=%d, BoundR: top=%d, bottom=%d, left=%d, right=%d, height=%d\n",
		xDIB, yDIB, BoundR.top, BoundR.bottom, BoundR.left, BoundR.right, BoundR.Height());
	TRACE("CreateRedCenters: buf_line allocated with ny_buf_line=%d\n", ny_buf_line);
	
	int idx;
	int begY = BoundR.top;
	int endY = BoundR.bottom;
	if (endY >= yDIB) endY = yDIB;
	int n = -1;
	int lineFringes = 0;
	int ny = (int)BoundR.Height() + 1;
	Sections.SetSize(ny);
	int nContours = pB->ArrContour.GetSize();
	XYPoint P;
	bool useComplexMasking = (pB->ExtBoundType == BOUND_POLYGON || pB->InsBoundType == BOUND_POLYGON);
	
	TRACE("CreateRedCenters: begY=%d, endY=%d, ny=%d, useComplexMasking=%d, ArrEll.size=%d\n",
		begY, endY, ny, useComplexMasking, pB->ArrEll.GetSize());
	
	int totalRedCenters = 0;
	for (int iy = begY; iy < endY; iy++) {
		int n = -1;
		for (auto iCol = 0; iCol < pI->m_pDIB->m_dwWidth; iCol++) {
			idx = ((yDIB - iy) * pI->m_pDIB->m_dwWidth + iCol);
			Pixel = pI->m_pDIB->m_lpSrcBits[idx];
			rgbPix.rgbRed = Pixel;
			++n;
			if (useComplexMasking) {
				P.X = iCol; P.Y = iy;
				// Use ArrContour (computed polygons) instead of source shapes
				if (isPupil(P, pB->ArrContour))
					line[n] = inv_line[n] = ((unsigned char)rgbPix.rgbRed);
				else
					line[n] = inv_line[n] = 0;
			}
			else {
				line[n] = inv_line[n] = ((unsigned char)rgbPix.rgbRed);
			}
		}
		int i = iy - begY;
		Sections[i].L.P1.y = Sections[i].L.P2.y = iy;
		Sections[i].L.P1.x = buf_line[i][0];
		Sections[i].L.P2.x = buf_line[i][1];
		if (pCtrls->FringeCenterAs == FC_MAX) {
			fon_del(line, 0, n);
			Sections[i].Form(i, line, n, ny, buf_line);
		}
		else if (pCtrls->FringeCenterAs == FC_MIN) {
			invert_line(inv_line, 0, n);
			fon_del(inv_line, 0, n);
			Sections[i].Form(i, inv_line, n, ny, buf_line);
		}
		else if (pCtrls->FringeCenterAs == FC_MINMAX) {
			fon_del(line, 0, n);
			Sections[i].Form(i, line, n, ny, buf_line);
			invert_line(inv_line, 0, n);
			fon_del(inv_line, 0, n);
			Sections[i].Form(i, inv_line, n, ny, buf_line);
			Sections[i].Sort();
		}

		for (int ii = 0; ii < Sections[i].NumLines.GetSize(); ii++) {
			HidenDots.Add(CDPoint(Sections[i].NumLines[ii].redX, Sections[i].L.P1.y));
			totalRedCenters++;
		}

	}
	TRACE("CreateRedCenters: TOTAL red centers found: %d, HidenDots.size=%d\n", totalRedCenters, HidenDots.GetSize());
	
	free(line);
	free(inv_line);

	if (bPrePadded)
		pI->m_pDIB->PubPadBits();
}

void CDigitInfo::Draw(CDC* pDC, int DotSide, CPoint activeDot, CPoint cursorPos, bool rubberBand)
{
	int iS = 0;
	CControls* pCtrls = GetControls();

	COLORREF Color = RGB(255, 0, 0);
	CPen pen;
	pen.CreatePen(PS_SOLID, 0, InvColor);
	CPen* open = pDC->SelectObject(&pen);

	if (pCtrls->ViewState & V_ZAPSECTIONS) {
		for (int i = 0; i < ZapLines.GetSize(); i++) {
			if (idxDragZapLine == i || ZapLines[i].Removed)
				continue;
			ZapLines[i].Draw(pDC);
		}
	}

	CPen* retPen = pDC->SelectObject(open);
	if (retPen)
		retPen->DeleteObject();

	if (pCtrls->ViewState & V_EXTREMUMS) {
		Color = RGB(255, 0, 0);
		for (iS = 0; iS < HidenDots.GetSize(); iS++) {
			CPoint P;
			P.x = HidenDots[iS].x;  P.y = HidenDots[iS].y;
			pDC->SetPixelV(P.x, P.y, Color);
		}
		Color = RGB(255, 0, 0);
		for (iS = 0; iS < Sections.GetSize(); iS++) {
			Sections[iS].Draw(pDC, MainFringeNumber);
		}
	}

	if (pCtrls->ViewState & V_DOTLINES) {
		if (m_bUseFringeModel) {
			for (int iF = 0; iF < Fringes.size(); iF++) {
				double num = Fringes[iF].GetNumber();
				pCtrls->GetIndexColor(num, Color);
				CPen pen1;
				pen1.CreatePen(PS_SOLID, 0, Color);
				CPen* open1 = pDC->SelectObject(&pen1);
				Fringes[iF].DrawPolyline(pDC, Color);
				CPen* retPen1 = pDC->SelectObject(open1);
				if (retPen1)
					retPen1->DeleteObject();
			}
		}
		else {
			CList<double, double> Numbers;
			CArray<CDPoint> adP;
			GetDotNumbers(Numbers);
			POSITION pos = Numbers.GetHeadPosition();
			double Num;
			CDPoint dP;
			CPoint wP;
			for (int i = 0; i < Numbers.GetCount(); i++) {
				Num = Numbers.GetNext(pos);
				pCtrls->GetIndexColor(Num, Color);
				CPen pen1;
				pen1.CreatePen(PS_SOLID, 0, Color);
				CPen* open1 = pDC->SelectObject(&pen1);
				GetFringeDots(Num, adP);
				for (int ii = 0; ii < adP.GetSize(); ii++) {
					wP.x = adP[ii].x; wP.y = adP[ii].y;
					if (ii == 0) pDC->MoveTo(wP);
					else      pDC->LineTo(wP);
				}
				CPen* retPen1 = pDC->SelectObject(open1);
				if (retPen1)
					retPen1->DeleteObject();
			}
		}
	}

	if (pCtrls->ViewState & V_DOTS) {
		if (m_bUseFringeModel) {
			for (int iF = 0; iF < Fringes.size(); iF++) {
				double num = Fringes[iF].GetNumber();
				pCtrls->GetIndexColor(num, Color);
				Fringes[iF].DrawDots(pDC, DotSide, Color);
			}

			// Highlight main selected point if any
			if (activeDot != CPoint(-1,-1)) { // check if activeDot is valid
				int half = (DotSide + 2) / 2;
				CBrush brush(RGB(255, 64, 64));// noticeable color
				CBrush* oldBr = pDC->SelectObject(&brush);
				pDC->Ellipse(activeDot.x - half, activeDot.y - half, activeDot.x + half, activeDot.y + half);
				pDC->SelectObject(oldBr);

				if (cursorPos != CPoint(-1, -1) && rubberBand) {
					CPen rubberPen;
					rubberPen.CreatePen(PS_DOT, 1, RGB(255, 128, 0));
					CPen* oldPen = pDC->SelectObject(&rubberPen);
					pDC->MoveTo(activeDot);
					pDC->LineTo(cursorPos);
					pDC->SelectObject(oldPen);
				}
			}
		}
		else {
			for (int iD = 0; iD < Dots.size(); iD++) {
				if (idxDragDot == iD)
					continue;
				if (idxMainDot == iD)
					Dots[iD].Draw(pDC, DotSide, Color);
				else
					Dots[iD].Draw(pDC, DotSide, Color);
			}
		}
	}
}

void CDigitInfo::Clear(BOOL AllZAPSections/*TRUE*/)
{
	HidenDots.RemoveAll();
	Sections.RemoveAll();
	Dots.clear();
	Fringes.clear();  // NEW: Clear fringes
	
	if (AllZAPSections)
		ZapLines.RemoveAll();
	else if (!HandSetZapLines)
		ZapLines.RemoveAll();

	idxMainSection = -1;
	idxDragZapLine = -1;
	idxDragDot = -1;
	idxMainDot = -1;
	HandSetZapLines = FALSE;

	// NEW: Clear fringe selection
	MainFringeNumber = -1000.;
	idxDraggedPoint.Clear();
	idxMainPoint.Clear();
}


BOOL CDigitInfo::SaveZAP(LPCTSTR fname, int extIdx)
{
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return FALSE;
	
	if (extIdx == 2)
	{
		WriteWinZAPData(fname, IntInfo);
		return true;
	}
	else if (extIdx == 3)
	{
		WriteDosZAPData(fname, IntInfo);
		return true;
	}

	return false;
}

BOOL CDigitInfo::SaveFRN(LPCTSTR fname)
{
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return false;
	WriteFRNData(fname, IntInfo);
	return true;
}

#include "digitInfoFringe.hxx" // extracted for testing purposes

#include "digitInfoExt.hxx" // returned back the stuff copilot recklessly removed

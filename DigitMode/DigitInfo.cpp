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
	SelectMainDot();
	Delete_buf_line();
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
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

void CDigitInfo::Draw(CDC* pDC, int DotSide)
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

	if (pCtrls->ViewState & V_DOTS) {
		for (int iD = 0; iD < Dots.GetSize(); iD++) {
			if (idxDragDot == iD)
				continue;
			if (idxMainDot == iD)
				Dots[iD].Draw(pDC, DotSide, Color);
			else
				Dots[iD].Draw(pDC, DotSide);
		}
	}
}

void CDigitInfo::Clear(BOOL AllZAPSections/*TRUE*/)
{
	HidenDots.RemoveAll();
	Sections.RemoveAll();
	Dots.RemoveAll();
	if (AllZAPSections)
		ZapLines.RemoveAll();
	else if (!HandSetZapLines)
		ZapLines.RemoveAll();
	idxMainSection = -1;
	idxDragZapLine = -1;
	idxDragDot = -1;
	idxMainDot = -1;
	HandSetZapLines = FALSE;
}

void CDigitInfo::SelectFringeStep()
{
	CArray<double, double> SortedSteps;

	for (int i = 0; i < Sections.GetSize(); i++) {
		if (buf_line[i][2] != -1 && buf_line[i][3] != -1)
			continue;
		Sections[i].CalcAveStep();
		if (Sections[i].aveStep != -1)
			SortedSteps.Add(Sections[i].aveStep);
	}
	SortDouble(SortedSteps);
	int nS = SortedSteps.GetSize();
	if (nS) {
		SecSegm = SortedSteps[nS / 2];
	}
}

void CDigitInfo::SelectMainSection()
{
	double minSecSegm = SecSegm - SecSegm * CorrectionSecSegm;
	double maxSecSegm = SecSegm + SecSegm * CorrectionSecSegm;
	int maxFringeNumber1 = -1;
	int maxFringeNumber2 = -1;
	int iS1 = -1;
	int iS2 = -1;
	double step;
	int nDot;
	int nTimes = 5;
	int iTime = 0;
	int i = 0;
	for (i = 0; i < Sections.GetSize(); i++) {
		if (buf_line[i][2] != -1 && buf_line[i][3] != -1)
			continue;
		step = Sections[i].aveStep;
		nDot = Sections[i].NumLines.GetSize();
		if (nDot > maxFringeNumber1) iTime = 0;
		if (minSecSegm < step && step < maxSecSegm && nDot >= maxFringeNumber1 && iTime < nTimes) {
			if (nDot == maxFringeNumber1) iTime++;
			maxFringeNumber1 = nDot;
			iS1 = i;
		}
	}
	iTime = 0;
	for (i = Sections.GetSize() - 1; i > -1; i--) {
		if (buf_line[i][2] != -1 && buf_line[i][3] != -1)
			continue;
		step = Sections[i].aveStep;
		nDot = Sections[i].NumLines.GetSize();
		if (nDot > maxFringeNumber2) iTime = 0;
		if (minSecSegm < step && step < maxSecSegm && nDot >= maxFringeNumber2 && iTime < nTimes && maxFringeNumber1 == maxFringeNumber2) {
			if (nDot == maxFringeNumber2) iTime++;
			maxFringeNumber2 = nDot;
			iS2 = i;
		}
	}
	if (iS1 != -1 && iS2 != -1 && iS1 != iS2 && !isInsideScreen) {
		int idx = iS1 + (iS2 - iS1) / 2;
		Sections[idx].MainLine = TRUE;
		idxMainSection = idx;
	}
	else {
		if (iS1 != -1) {
			Sections[iS1].MainLine = TRUE;
			idxMainSection = iS1;
		}
		//	 if(iS2 != -1)
		//		Sections[iS2].VisibleLine = TRUE;
	}
}

#include "DigitMode\CreateNumLines.cxx"

void CDigitInfo::SelectMainFringe()
{
	if (idxMainSection == -1)
		return;

	int nFringes = Sections[idxMainSection].NumLines.GetSize();
	CMapStringToPtr mapNum(nFringes);
	CString key;

	for (int i = 0; i < nFringes; i++) {
		key.Format("%g", Sections[idxMainSection].NumLines[i].Number);
		int* count = new int;
		*count = 0;
		mapNum.SetAt(LPCTSTR(key), (void*)count);
	}

	POSITION pos;
	double curNum;
	for (pos = mapNum.GetStartPosition(); pos != NULL;) {
		int* count;
		mapNum.GetNextAssoc(pos, key, (void*&)count);
		curNum = atof(LPCTSTR(key));
		int max_n = 0;
		for (int iS = 0; iS < Sections.GetSize(); iS++) {
			int n = 0;
			for (int iL = 0; iL < Sections[iS].NumLines.GetSize(); iL++) {
				if (Sections[iS].NumLines[iL].Number == curNum)
					n++;
			}
			if (n == 1) max_n++;
		}
		*count = max_n;
		mapNum.SetAt(LPCTSTR(key), (void*&)count);
	}

	int max_count = INT_MIN;
	double mainNum = -1000.;
	for (pos = mapNum.GetStartPosition(); pos != NULL;) {
		int* count;
		mapNum.GetNextAssoc(pos, key, (void*&)count);
		if (*count > max_count) {
			max_count = *count;
			mainNum = atof(LPCTSTR(key));
		}
		delete count;
	}
	mapNum.RemoveAll();

	MainFringeNumber = mainNum;
}

#include "DigitMode\SelectNumber.cxx"

void CDigitInfo::CorrectNumbers()
{
	int iL, i;
	double mainNum = MainFringeNumber;
	for (int iS = 0; iS < Sections.GetSize(); iS++) {
		int idxMainFringe = -1;
		for (i = 0; i < Sections[iS].NumLines.GetSize(); i++) {
			if (Sections[iS].NumLines[i].Number == mainNum) {
				idxMainFringe = i;
				break;
			}
		}
		if (idxMainFringe == -1)
			continue;

		for (iL = idxMainFringe - 1; iL > -1; iL--) {
			Sections[iS].NumLines[iL].Number = mainNum - numStep * (idxMainFringe - iL);
		}
		for (iL = idxMainFringe + 1; iL < Sections[iS].NumLines.GetSize(); iL++) {
			Sections[iS].NumLines[iL].Number = mainNum + numStep * (iL - idxMainFringe);
		}
	}
}

void CDigitInfo::CreateZAPSections()
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int maxN = INT_MIN;
	int minN = INT_MAX;
	for (int iS = 0; iS < Sections.GetSize(); iS++) {
		for (int iL = 0; iL < Sections[iS].NumLines.GetSize(); iL++) {
			if (Sections[iS].NumLines[iL].Number != -1000.)
				minN = __min(minN, Sections[iS].NumLines[iL].Number);
			maxN = __max(maxN, Sections[iS].NumLines[iL].Number);
		}
	}
	int nFringes = (maxN - minN) / numStep + 1;
	int begY = 0;
	int bH = BoundR.Height();
	double SecGap;
	int i;
	if (ZapLines.GetSize() < 2) {
		SecGap = (double)(bH) / (nFringes - 1);
	}
	else {
		SortZapLines();
		int maxSize = ZapLines.GetSize();
		SecGap = (fabs(ZapLines[maxSize - 1].L.P1.y - ZapLines[0].L.P1.y)) / (maxSize - 1);
		begY = ZapLines[maxSize - 1].L.P1.y - BoundR.top;
		nFringes = ((double)(bH - begY)) / SecGap + 1;
	}

	for (i = 1; i < nFringes - 1; i++) {
		int iy = (int)(begY + SecGap * i);
		CZapLineInfo zL;
		zL.L = Sections[iy].L;
		zL.iSec = iy;
		ZapLines.Add(zL);
	}

	for (i = 0; i < ZapLines.GetSize(); i++) {
		PutDotsOnZAPSections(i);
	}
	HandSetZapLines = FALSE;
}

void CDigitInfo::CreateZAPSectionsOnLoadZAPFile()
{
	int i;
	CImageCtrls* pI = GetImageCtrls();
	CBoundCtrls* pB = GetBoundCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;

	CList<double, double> YLines;
	double y;
	for (i = 0; i < Dots.GetSize(); i++) {
		y = Dots[i].P.y;
		if (!YLines.Find(y))
			YLines.AddTail(y);
	}

	int ext_t_y = BoundR.top;
	int iy;
	POSITION pos = YLines.GetHeadPosition();
	for (i = 0; i < YLines.GetCount(); i++) {
		y = YLines.GetNext(pos);
		iy = int(y);
		CZapLineInfo zL;
		if (pI->m_pDIB) {
			zL.L = Sections[iy - ext_t_y].L;
			zL.iSec = iy - ext_t_y;
		}
		else {
			zL.L.P1.x = buf_line[iy - ext_t_y][0];
			zL.L.P2.x = buf_line[iy - ext_t_y][1];
			zL.L.P1.y = zL.L.P2.y = iy - ext_t_y;
			zL.iSec = iy - ext_t_y;
		}
		ZapLines.Add(zL);
	}

	CPoint P;
	int idx;
	for (i = 0; i < Dots.GetSize(); i++) {
		P.x = Dots[i].P.x; P.y = Dots[i].P.y;
		if (GetNearestZapSection(P, idx)) {
			Dots[i].iZapSec = idx;
		}
	}
}

void CDigitInfo::SortZapLines()
{
	int i;
	for (i = 0; i < ZapLines.GetSize(); i++) {
		if (ZapLines[i].Removed) {
			ZapLines.RemoveAt(i);
			i--;
		}
	}
	int swapAt;
	int begIdx = 0;
	int endIdx = ZapLines.GetSize();
	double d_swapAt;
	double d_k;
	for (int j = begIdx; j < endIdx - 1; j++) {
		swapAt = j;
		for (int k = j + 1; k < endIdx; k++) {
			d_swapAt = ZapLines[swapAt].L.P1.y;
			d_k = ZapLines[k].L.P1.y;
			if (d_swapAt > d_k)
				swapAt = k;
		}
		Swap(ZapLines[j], ZapLines[swapAt]);
	}
}

void CDigitInfo::SortDots(CArray<CDPoint>& adP, int XY)
{
	int swapAt;
	int begIdx = 0;
	int endIdx = adP.GetSize();
	double d_swapAt;
	double d_k;
	for (int j = begIdx; j < endIdx - 1; j++) {
		swapAt = j;
		for (int k = j + 1; k < endIdx; k++) {
			if (XY == 1) {
				d_swapAt = adP[swapAt].x;
				d_k = adP[k].x;
			}
			else if (XY == 2) {
				d_swapAt = adP[swapAt].y;
				d_k = adP[k].y;
			}
			if (d_swapAt > d_k)
				swapAt = k;
		}
		Swap(adP[j], adP[swapAt]);
	}
}

void CDigitInfo::PutDotsOnZAPSections(int iZAPSec)
{
	if (Sections.GetSize() == 0)
		return;
	int iy = ZapLines[iZAPSec].iSec;
	for (int i = 0; i < Sections[iy].NumLines.GetSize(); i++) {
		if (Sections[iy].NumLines[i].Included) {
			CDotInfo Dot;
			Dot.P.x = Sections[iy].NumLines[i].redX;
			Dot.P.y = ZapLines[iZAPSec].L.P1.y;
			Dot.Number = Sections[iy].NumLines[i].Number;
			Dot.iZapSec = iZAPSec;
			Dots.Add(Dot);
		}
	}
}

bool CDigitInfo::IsSections()
{
	if (Sections.GetSize())
		return true;
	else
		return false;
}

bool CDigitInfo::IsZapSections()
{
	if (ZapLines.GetSize())
		return true;
	else
		return false;
}

bool CDigitInfo::IsLockedZapSection()
{
	if (idxDragZapLine == -1)
		return false;
	else
		return true;
}

bool CDigitInfo::IsZapSectionUnderCursor(CPoint P, int& idx)
{
	double y1 = P.y - 1;
	double y2 = P.y + 1;
	double y;
	for (int i = 0; i < ZapLines.GetSize(); i++) {
		if (ZapLines[i].Removed)
			continue;
		y = ZapLines[i].L.P1.y;
		if (y1 <= y && y <= y2) {
			idx = i;
			return true;
		}
	}
	return false;
}

bool CDigitInfo::LockZapSection(CPoint P, BOOL Enable)
{
	if (ZapLines.GetSize() == 0)
		return false;

	if (!Enable) {
		PutDotsOnZAPSections(idxDragZapLine);
		idxDragZapLine = -1;
		return true;
	}

	int idx;
	if (IsZapSectionUnderCursor(P, idx)) {
		idxDragZapLine = idx;
		return true;
	}
	else
		return false;
}

void CDigitInfo::SetLockedZapSectionYPos(int iy)
{
	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int begY = BoundR.top;
	int i = iy - begY;
	if (Sections.GetSize()) {
		ZapLines[idxDragZapLine].L = Sections[i].L;
	}
	else {
		CDLine L;
		L.P1.y = L.P2.y = iy;
		L.P1.x = buf_line[i][0];
		L.P2.x = buf_line[i][1];
		ZapLines[idxDragZapLine].L = L;
	}
}

void CDigitInfo::GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2)
{
	P1.x = (int)ZapLines[idxDragZapLine].L.P1.x;
	P1.y = (int)ZapLines[idxDragZapLine].L.P1.y;

	P2.x = (int)ZapLines[idxDragZapLine].L.P2.x;
	P2.y = (int)ZapLines[idxDragZapLine].L.P2.y;
}

void CDigitInfo::AddZapSection(int iy)
{
	if (!buf_line)
		CreateBufLine();

	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return;
	int begY = BoundR.top;
	int i = iy - begY;
	CZapLineInfo zL;
	if (Sections.GetSize()) {
		zL.L = Sections[i].L;
		zL.iSec = i;
		ZapLines.Add(zL);
		PutDotsOnZAPSections(ZapLines.GetSize() - 1);
	}
	else {
		CDLine L;
		L.P1.y = L.P2.y = iy;
		L.P1.x = buf_line[i][0];
		L.P2.x = buf_line[i][1];
		zL.iSec = i;
		zL.L = L;
		ZapLines.Add(zL);
	}
	HandSetZapLines = TRUE;
}

void CDigitInfo::SectionLeft(CPoint P, int dotSide)
{
	int idx;
	if (idxMainDot < Dots.GetSize()) {
		if (IsDotUnderCursor(P, dotSide, idx) && idxMainDot != -1) {
			int iZapSec = Dots[idx].iZapSec;
			double Number = Dots[idxMainDot].Number + numStep;
			CDPoint dP = Dots[idx].P;
			bool res = true;
			while (res) {
				Dots[idx].Number = Number = Number - numStep;
				res = GetNextDotInSection(iZapSec, -1, idx, dP);
			}
		}
	}
}

void CDigitInfo::SectionRight(CPoint P, int dotSide)
{
	int idx;
	if (idxMainDot < Dots.GetSize()) {
		if (IsDotUnderCursor(P, dotSide, idx) && idxMainDot != -1) {
			int iZapSec = Dots[idx].iZapSec;
			double Number = Dots[idxMainDot].Number - numStep;
			CDPoint dP = Dots[idx].P;
			bool res = true;
			while (res) {
				Dots[idx].Number = Number = Number + numStep;
				res = GetNextDotInSection(iZapSec, 1, idx, dP);
			}
		}
	}
}

void CDigitInfo::DeleteZapSection(int iy)
{
	CPoint P(0, iy);
	int idx;
	if (IsZapSectionUnderCursor(P, idx)) {
		RemoveDotZAPSection(idx);
		ZapLines[idx].Removed = TRUE;;
	}
}

void CDigitInfo::RemoveDotZAPSection(int iSec)
{
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].iZapSec == iSec) {
			Dots.RemoveAt(iD);
			iD--;
		}
	}
}

void CDigitInfo::AddDot(CPoint P, int dotSide)
{
	int idx;
	CControls* pCtrls = GetControls();
	CDotInfo dot;
	dot.Number = CurrentNumber;
	dot.P.x = P.x;
	dot.P.y = P.y;
	if (pCtrls->ViewState & V_ZAPSECTIONS) {
		if (GetNearestZapSection(P, idx)) {
			dot.iZapSec = idx;
			dot.P.y = P.y = ZapLines[idx].L.P1.y;
		}
	}
	/*if (pCtrls->ViewState & V_EXTREMUMS) {
	   double x;
	   if(GetNearestXInSection(P, x)){
		   dot.P.x = x;
	   }
	}*/
	Dots.Add(dot);
}

void CDigitInfo::RemoveDot(CPoint P, int dotSide)
{
	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		Dots.RemoveAt(idx);
	}
}

void CDigitInfo::RemoveFringe(CPoint P, int dotSide)
{
	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		double Number = Dots[idx].Number;
		for (int i = 0; i < Dots.GetSize(); i++) {
			if (Dots[i].Number == Number) {
				Dots.RemoveAt(i);
				i--;
			}
		}
	}
}

bool CDigitInfo::IsDots()
{
	if (Dots.GetSize())
		return true;
	else
		return false;
}

bool CDigitInfo::IsDotUnderCursor(CPoint P, int dotSide, int& idx)
{
	int DotSide12 = dotSide / 2;
	CPoint lP;
	CRect dotR;
for (int i = 0; i < Dots.GetSize(); i++) {
		lP.x = int(Dots[i].P.x);
		lP.y = int(Dots[i].P.y);
		dotR.left = lP.x - DotSide12;
		dotR.right = lP.x + DotSide12;
		dotR.top = lP.y - DotSide12;
		dotR.bottom = lP.y + DotSide12;
		if (dotR.PtInRect(P)) {
			idx = i;
			return true;
		}
	}
	return false;
}

bool CDigitInfo::IsLockedDot()
{
	if (idxDragDot == -1)
		return false;
	else
		return true;
}

bool CDigitInfo::LockDot(CPoint P, int dotSide, BOOL Enable)
{
	if (Dots.GetSize() == 0)
		return false;

	if (!Enable) {
		idxDragDot = -1;
		return true;
	}

	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		idxDragDot = idx;
		return true;
	}
	else
		return false;
}

void CDigitInfo::SetLockedDotPos(CPoint P)
{
	Dots[idxDragDot].P.x = P.x;
	Dots[idxDragDot].P.y = P.y;
}

void CDigitInfo::GetLockedDotPos(CPoint& P1)
{
	P1.x = Dots[idxDragDot].P.x;
	P1.y = Dots[idxDragDot].P.y;
}

void CDigitInfo::SelectMainDot(CPoint P, int dotSide)
{
	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		idxMainDot = idx;
		CurrentNumber = Dots[idx].Number;
	}
}

void CDigitInfo::SelectMainDot(int iZapSec/*=-1*/, double Number/*=INT_MIN*/)
{
	if (iZapSec == -1 && Number == INT_MIN && Dots.GetSize()) {
		idxMainDot = 0;
		CurrentNumber = Dots[0].Number;
		return;
	}
	else {
		int idx;
		CDPoint dP;
		if (GetDot(iZapSec, Number, idx, dP)) {
		idxMainDot = idx;
			CurrentNumber = Number;
			return;
		}
	}
	idxMainDot = -1;
}

bool CDigitInfo::GetDot(int iZapSec, double Number, int& idx, CDPoint& dP)
{
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].iZapSec == iZapSec && Dots[iD].Number == Number) {
			idx = iD;
			dP = Dots[iD].P;
			return true;
		}
	}
	return false;
}

bool CDigitInfo::GetDotNumbers(CList<double, double>& Numbers)
{
	double Num;
	POSITION Pos;
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		Num = Dots[iD].Number;
		Pos = Numbers.Find(Num);
		if (!Pos)
			Numbers.AddTail(Num);
	}
	return true;
}

bool CDigitInfo::GetFirstDotInSection(int iZapSec, int& idx, CDPoint& dP)
{
	int minidx = -1;
	double minx = INT_MAX;
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].iZapSec == iZapSec) {
			if (Dots[iD].P.x < minx) {
				minx = Dots[iD].P.x;
				minidx = iD;
			}
		}
	}
	if (minidx != -1) {
		idx = minidx;
		dP = Dots[idx].P;
		return true;
	}
	else
		return false;
}

bool CDigitInfo::GetNextDotInSection(int iZapSec, int direct, int& idx, CDPoint& dP)
{
	int minidx = -1;
	double minx = INT_MAX;
	double dif;
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].iZapSec == iZapSec) {
			dif = 0.;
			if (direct > 0 && Dots[iD].P.x > dP.x)
				dif = Dots[iD].P.x - dP.x;
			else if (direct < 0 && dP.x > Dots[iD].P.x)
				dif = dP.x - Dots[iD].P.x;
			if (dif == 0.)
				continue;
			if (dif < minx) {
				minx = dif;
				minidx = iD;
			}
		}
	}
	if (minidx != -1) {
		idx = minidx;
		dP = Dots[idx].P;
		return true;
	}
	else
		return false;
}

bool CDigitInfo::GetFringeDots(double Number, CUIntArray& idxDots)
{
	idxDots.RemoveAll();
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Number == Dots[iD].Number) {
			idxDots.Add(iD);
		}
	}
	return true;
}

bool CDigitInfo::GetFringeDots(double Number, CArray<CDPoint>& adP)
{
	adP.RemoveAll();
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Number == Dots[iD].Number) {
			adP.Add(Dots[iD].P);
		}
	}
	SortDots(adP, 2);
	return true;
}

bool CDigitInfo::GetFirstDotInFringe(double Number, int& idx, CDPoint& dP)
{
	int minidx = -1;
	double minx = INT_MAX;
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].Number == Number) {
			if (Dots[iD].P.x < minx) {
				minx = Dots[iD].P.x;
				minidx = iD;
			}
		}
	}
	if (minidx != -1) {
	idx = minidx;
	dP = Dots[idx].P;
	return true;
	}
	else
		return false;
}

bool CDigitInfo::GetNextDotInFringe(double Number, int direct, int& idx, CDPoint& dP)
{
	int minidx = -1;
	double minx = INT_MAX;
	double dif;
	for (int iD = 0; iD < Dots.GetSize(); iD++) {
		if (Dots[iD].Number == Number) {
			dif = 0.;
			if (direct > 0 && Dots[iD].P.y > dP.y)
				dif = Dots[iD].P.y - dP.x;
			else if (direct < 0 && dP.y > Dots[iD].P.y)
				dif = dP.y - Dots[iD].P.y;
			if (dif == 0.)
				continue;
			if (dif < minx) {
				minx = dif;
				minidx = iD;
			}
		}
	}
	if (minidx != -1) {
		idx = minidx;
		dP = Dots[idx].P;
		return true;
	}
	else
		return false;
}

bool CDigitInfo::GetNearestZapSection(CPoint P, int& idx)
{
	double mindif = INT_MAX;
	int iy;
	int _idx = -1;
	for (int i = 0; i < ZapLines.GetSize(); i++) {
		iy = (int)ZapLines[i].L.P1.y;
		if (abs(iy - P.y) < mindif) {
			mindif = abs(iy - P.y);
			_idx = i;
		}
	}
	if (_idx != -1) {
		idx = _idx;
		return true;
	}
	return false;
}

bool CDigitInfo::GetNearestXInSection(CPoint P, double& x)
{
	if (Sections.GetSize() == 0)
		return false;

	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	CRect BoundR;
	if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
		return false;
	int begY = BoundR.top;
	int i = P.y - begY;
	double mindif = INT_MAX;
	int _idx = -1;
	int ix;
	for (int ii = 0; ii < Sections[i].NumLines.GetSize(); ii++) {
		ix = (int)Sections[i].NumLines[ii].redX;
		if (abs(ix - P.x) < mindif) {
			mindif = abs(ix - P.x);
			_idx = ii;
		}
	}
	if (_idx != -1) {
		x = Sections[i].NumLines[_idx].redX;
		return true;
	}
	return false;
}

void CDigitInfo::RenumFringe(CPoint P, int dotSide)
{
	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		double Number = Dots[idx].Number;
		CUIntArray idxDots;
		GetFringeDots(Number, idxDots);
		for (int i = 0; i < idxDots.GetSize(); i++) {
			Dots[idxDots[i]].Number = CurrentNumber;
		}
	}
}

void CDigitInfo::RenumDot(CPoint P, int dotSide)
{
	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		Dots[idx].Number = CurrentNumber;
	}
}

void CDigitInfo::NumberMinus()
{
	CList<double, double> Numbers;
	GetDotNumbers(Numbers);
	double Num1 = CurrentNumber;
	double Num2 = CurrentNumber - numStep;
	POSITION posNum1 = Numbers.Find(Num1);
	POSITION posNum2 = Numbers.Find(Num2);
	if (!posNum1 && !posNum2)
		return;

	CurrentNumber = Num2;
	if (idxMainDot != -1) {
		int iZapSec = Dots[idxMainDot].iZapSec;
		SelectMainDot(iZapSec, CurrentNumber);
	}
}

void CDigitInfo::NumberPlus()
{
	CList<double, double> Numbers;
	GetDotNumbers(Numbers);
	double Num1 = CurrentNumber;
	double Num2 = CurrentNumber + numStep;
	POSITION posNum1 = Numbers.Find(Num1);
	POSITION posNum2 = Numbers.Find(Num2);
	if (!posNum1 && !posNum2)
		return;

	CurrentNumber = Num2;
	if (idxMainDot != -1) {
		int iZapSec = Dots[idxMainDot].iZapSec;
		SelectMainDot(iZapSec, CurrentNumber);
	}
}

BOOL CDigitInfo::CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	IntInfo.Title = Comments;
	IntInfo.ScaleFactor = ScaleFactor;
	IntInfo.FiScan = Rotation;

	CImageCtrls* pIm = GetImageCtrls();
	CBoundCtrls* pB = GetBoundCtrls();
	CDocument* pDoc = GetWIActiveDocument();

	IntInfo.ArrEll.RemoveAll();
	IntInfo.ArrEll.Append(pB->ArrEll);
	IntInfo.ArrRect.RemoveAll();
	IntInfo.ArrRect.Append(pB->ArrRect);
	IntInfo.ArrPlg.RemoveAll();
	IntInfo.ArrPlg.Append(pB->ArrPlg);
	SAMPLE_DATA DigitDat;
	IntInfo.ImageSize[0] = pIm->ImageSize.cx;
	IntInfo.ImageSize[1] = pIm->ImageSize.cy;
	IntInfo.ImageFileName = pIm->OriginalPath;

	int nD = Dots.GetSize();
	if (nD == 0)
		return FALSE;
	IntInfo.DigitDat.Properties.SetSize(nD);
	IntInfo.DigitDat.XPnt.SetSize(nD);
	IntInfo.DigitDat.YPnt.SetSize(nD);
	IntInfo.DigitDat.FPnt.SetSize(nD);
	for (int i = 0; i < nD; i++) {
		IntInfo.DigitDat.XPnt[i] = Dots[i].P.x;
		IntInfo.DigitDat.YPnt[i] = Dots[i].P.y;
		IntInfo.DigitDat.FPnt[i] = Dots[i].Number;
		IntInfo.DigitDat.Properties[i] = 0.;
	}

	return TRUE;
}

BOOL CDigitInfo::ExamineNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	Comments = IntInfo.Title;
	ScaleFactor = IntInfo.ScaleFactor;
	Rotation = IntInfo.FiScan;

	CDocument* pDoc = GetWIActiveDocument();
	CImageCtrls* pIm = GetImageCtrls();
	CBoundCtrls* pB = GetBoundCtrls();

	pB->ArrEll.RemoveAll();
	pB->ArrEll.Append(IntInfo.ArrEll);
	pB->ArrRect.RemoveAll();
	pB->ArrRect.Append(IntInfo.ArrRect);
	pB->ArrPlg.RemoveAll();
	pB->ArrPlg.Append(IntInfo.ArrPlg);
	pB->FormBoundsOnLoadFile();

	SAMPLE_DATA DigitDat;
	pIm->ImageSize.cx = IntInfo.ImageSize[0];
	pIm->ImageSize.cy = IntInfo.ImageSize[1];

	int nD = IntInfo.DigitDat.XPnt.GetSize();
	if (nD == 0)
		return FALSE;
	Dots.SetSize(nD);
	for (int i = 0; i < nD; i++) {
		Dots[i].P.x = IntInfo.DigitDat.XPnt[i];
		Dots[i].P.y = IntInfo.DigitDat.YPnt[i];
		Dots[i].Number = IntInfo.DigitDat.FPnt[i];
	}
	return TRUE;
}

BOOL CDigitInfo::Load(LPCTSTR fname)
{
	if (!IsFileExist(fname, FALSE))
		return FALSE;

	CString path = fname;
	CString ext = path.Right(3);
	ext.MakeLower();
	if (ext == "zap")
		return LoadZAP(fname);
	else if (ext == "frn")
		return LoadFRN(fname);
	else
		return FALSE;
}

BOOL CDigitInfo::Save(LPCTSTR fname, int extIdx)
{
	CString path = fname;
	CString ext = path.Right(3);
	ext.MakeLower();
	if (ext == "zap")
		return SaveZAP(fname, extIdx);
	else if (ext == "frn")
		return SaveFRN(fname);
	else
		return FALSE;
}

/// <summary>
/// Resolves a potentially relative image filename to an absolute path.
/// Checks if the path exists relative to the data file's directory.
/// </summary>
/// <param name="dataFilePath">Full path to the .zap or .frn file</param>
/// <param name="imageFileName">Image filename from the data file (may be relative or absolute)</param>
/// <returns>Resolved absolute path if file exists, original path otherwise</returns>
std::string CDigitInfo::ResolveImagePath(const std::string& dataFilePath, const std::string& imageFileName)
{
	TRACE("ResolveImagePath: dataFile=%s, imageFile=%s\n", dataFilePath.c_str(), imageFileName.c_str());

	if (imageFileName.empty()) {
		return imageFileName;
	}

	namespace fs = std::filesystem;

	try {
		fs::path imgPath(imageFileName);

		// Check if already absolute
		if (imgPath.is_absolute()) {
			TRACE("ResolveImagePath: Already absolute, returning as-is\n");
			return imageFileName;
		}

		// Resolve relative to data file directory
		fs::path dataPath(dataFilePath);
		fs::path parentDir = dataPath.parent_path();
		fs::path combinedPath = parentDir / imgPath;

		// Normalize path (resolve .., ., etc.)
		combinedPath = fs::absolute(combinedPath).lexically_normal();

		// Check if resolved path exists
		if (fs::exists(combinedPath)) {
			std::string resolved = combinedPath.string();
			TRACE("ResolveImagePath: Resolved to %s\n", resolved.c_str());
			return resolved;
		}
		else {
			TRACE("ResolveImagePath: Resolved path doesn't exist, returning original\n");
			return imageFileName;
		}
	}
	catch (const fs::filesystem_error& e) {
		TRACE("ResolveImagePath: Filesystem error: %s\n", e.what());
		return imageFileName;
	}
}

/// <summary>
/// Creates a fake gray DIB image of specified dimensions.
/// Used when actual image file is missing but vector data needs to be displayed.
/// </summary>
/// <param name="pImageCtrls">Pointer to image controls</param>
/// <param name="width">Image width in pixels</param>
/// <param name="height">Image height in pixels</param>
/// <returns>TRUE if successful, FALSE otherwise</returns>
BOOL CDigitInfo::CreateFakeGrayImage(CImageCtrls* pImageCtrls, int width, int height)
{
	TRACE("CreateFakeGrayImage: Creating %dx%d gray image\n", width, height);

	if (!pImageCtrls) {
		TRACE("CreateFakeGrayImage: pImageCtrls is NULL\n");
		return FALSE;
	}

	if (width <= 0 || height <= 0) {
		TRACE("CreateFakeGrayImage: Invalid dimensions %dx%d\n", width, height);
		return FALSE;
	}

	// Clean up existing DIB if any
	if (pImageCtrls->m_pDIB) {
		delete pImageCtrls->m_pDIB;
		pImageCtrls->m_pDIB = NULL;
	}

	// Create new CDIB instance
	pImageCtrls->m_pDIB = new SECDib();
	if (!pImageCtrls->m_pDIB) {
		TRACE("CreateFakeGrayImage: Failed to allocate CDIB\n");
		return FALSE;
	}

	// Set dimensions
	pImageCtrls->m_pDIB->m_dwWidth = width;
	pImageCtrls->m_pDIB->m_dwHeight = height;
	pImageCtrls->m_pDIB->m_nSrcBitsPerPixel = 8; // 8-bit grayscale
	pImageCtrls->m_pDIB->m_wColors = 256;
	pImageCtrls->m_pDIB->m_nBitPlanes = 1;

	// Calculate padded width (DWORD-aligned)
	pImageCtrls->m_pDIB->m_dwPadWidth = ((width + 3) / 4) * 4;

	// Allocate BITMAPINFO structure using GlobalAllocPtr
	DWORD bmiSize = sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
	pImageCtrls->m_pDIB->m_lpBMI = (LPBITMAPINFO)GlobalAllocPtr(GHND, bmiSize);
	if (!pImageCtrls->m_pDIB->m_lpBMI) {
		TRACE("CreateFakeGrayImage: Failed to allocate BITMAPINFO\n");
		delete pImageCtrls->m_pDIB;
		pImageCtrls->m_pDIB = NULL;
		return FALSE;
	}

	// Fill BITMAPINFOHEADER
	LPBITMAPINFOHEADER lpbi = &pImageCtrls->m_pDIB->m_lpBMI->bmiHeader;
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biWidth = width;
	lpbi->biHeight = height;
	lpbi->biPlanes = 1;
	lpbi->biBitCount = 8;
	lpbi->biCompression = BI_RGB;
	lpbi->biSizeImage = pImageCtrls->m_pDIB->m_dwPadWidth * height;
	lpbi->biXPelsPerMeter = 0;
	lpbi->biYPelsPerMeter = 0;
	lpbi->biClrUsed = 256;
	lpbi->biClrImportant = 0;

	// Set m_lpRGB pointer to bmiColors array
	pImageCtrls->m_pDIB->m_lpRGB = pImageCtrls->m_pDIB->m_lpBMI->bmiColors;

	// Fill grayscale palette in BITMAPINFO.bmiColors
	for (int i = 0; i < 256; i++) {
		pImageCtrls->m_pDIB->m_lpBMI->bmiColors[i].rgbRed = (BYTE)i;
		pImageCtrls->m_pDIB->m_lpBMI->bmiColors[i].rgbGreen = (BYTE)i;
		pImageCtrls->m_pDIB->m_lpBMI->bmiColors[i].rgbBlue = (BYTE)i;
		pImageCtrls->m_pDIB->m_lpBMI->bmiColors[i].rgbReserved = 0;
	}

	// Allocate pixel buffer using GlobalAllocPtr
	DWORD imageSize = lpbi->biSizeImage;
	pImageCtrls->m_pDIB->m_lpSrcBits = (LPBYTE)GlobalAllocPtr(GHND, imageSize);
	if (!pImageCtrls->m_pDIB->m_lpSrcBits) {
		TRACE("CreateFakeGrayImage: Failed to allocate pixel buffer\n");
		GlobalFreePtr(pImageCtrls->m_pDIB->m_lpBMI);
		pImageCtrls->m_pDIB->m_lpBMI = NULL;
		pImageCtrls->m_pDIB->m_lpRGB = NULL;
		delete pImageCtrls->m_pDIB;
		pImageCtrls->m_pDIB = NULL;
		return FALSE;
	}

	// Fill with medium gray (128)
	memset(pImageCtrls->m_pDIB->m_lpSrcBits, 128, imageSize);

	// The palette will be created by SECImage::CreatePalette() when needed
	// We don't need to create it here - let the base class handle it
	//pImageCtrls->m_pDIB->CreatePalette();

	// Set other required fields
	pImageCtrls->m_pDIB->m_bIsPadded = FALSE;
	pImageCtrls->ImageSize.cx = width;
	pImageCtrls->ImageSize.cy = height;

	TRACE("CreateFakeGrayImage: Successfully created %dx%d image\n", width, height);
	return TRUE;
}

BOOL CDigitInfo::LoadZAP(LPCTSTR fname)
{
	CString FileName = fname;
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!ReadZAPData(FileName, IntInfo))
		return FALSE;

	// --- Resolve image filename relative to ZAP file directory ---
	if (!IntInfo.ImageFileName.IsEmpty()) {
		std::string zapFile = CT2A(FileName);
		std::string imgFile = CT2A(IntInfo.ImageFileName);
		std::string resolved = ResolveImagePath(zapFile, imgFile);
		IntInfo.ImageFileName = CString(resolved.c_str());
		TRACE("LoadZAP: Image path resolved to: %s\n", resolved.c_str());
	}

	//Вызов LoadImage для инициализации m_pDIB
	CImageCtrls* pI = GetImageCtrls();
	BOOL imageLoaded = FALSE;

	if (!IntInfo.ImageFileName.IsEmpty()) {
		imageLoaded = pI->LoadImage(IntInfo.ImageFileName);
		if (!imageLoaded) {
			TRACE("LoadZAP: Failed to load image %s\n", CT2A(IntInfo.ImageFileName));
		}
	}

	// Create fake gray image if actual image failed to load
	// TODO: investigate throw and memory leaks when image is missing
	if (!imageLoaded && IntInfo.ImageSize[0] > 0 && IntInfo.ImageSize[1] > 0) {
		TRACE("LoadZAP: Creating fake gray image as fallback\n");
		if (CreateFakeGrayImage(pI, IntInfo.ImageSize[0], IntInfo.ImageSize[1])) {
			imageLoaded = TRUE; // Treat as successful load for processing
			AfxMessageBox(_T("Изображение отсутствует. Создан серый фон для отображения векторных данных."));
			TRACE("LoadZAP: Fake image created successfully\n");
		}
		else {
			AfxMessageBox(_T("Не удалось создать изображение для отображения векторных данных."));
			TRACE("LoadZAP: Failed to create fake image\n");
		}
	}

	TRACE("LoadZAP: Image loaded=%d, m_pDIB=%p\n", imageLoaded, pI->m_pDIB);

	if (IntInfo.LoadedFileType == NUMBERING_INTERFEROGRAM_INFO::TYP_ZAP_DOS)
	{
		// Use ImageSize from IntInfo if image failed to load
		int actualHeight = imageLoaded ? pI->ImageSize.cy : IntInfo.ImageSize[1];
		double dY = (actualHeight - IntInfo.ImageSize[1]);
		TRACE("LoadZAP: DOS ZAP detected, actualHeight=%d, fakeHeight=%d, dY=%f\n", 
			actualHeight, IntInfo.ImageSize[1], dY);
		
		IntInfo.DigitDat.ShiftY(dY);
		IntInfo.EBnd.ShiftY(dY);
		TRACE("LoadZAP: After EBnd shift: XLeft=%f, YTop=%f, XRight=%f, YBottom=%f\n",
			IntInfo.EBnd.XLeft, IntInfo.EBnd.YTop, IntInfo.EBnd.XRight, IntInfo.EBnd.YBottom);
			
		for (auto i = 0; i < IntInfo.ArrEll.GetSize(); i++) {
			TRACE("LoadZAP: Before shift Ell[%d]: Xc=%f, Yc=%f, Ax=%f, By=%f\n", 
				i, IntInfo.ArrEll[i].Xc, IntInfo.ArrEll[i].Yc, IntInfo.ArrEll[i].Ax, IntInfo.ArrEll[i].By);
			IntInfo.ArrEll[i].ShiftY(dY);
			TRACE("LoadZAP: After shift Ell[%d]: Xc=%f, Yc=%f, Ax=%f, By=%f\n", 
				i, IntInfo.ArrEll[i].Xc, IntInfo.ArrEll[i].Yc, IntInfo.ArrEll[i].Ax, IntInfo.ArrEll[i].By);
		}
		IntInfo.ImageSize[0] = imageLoaded ? pI->ImageSize.cx : IntInfo.ImageSize[0];
		IntInfo.ImageSize[1] = actualHeight;
	}

	if (!ExamineNumberingInterferogramInfo(IntInfo))
		return FALSE;

	// Ensure ImageSize is set even without loaded image
	if (!imageLoaded) {
		pI->ImageSize.cx = IntInfo.ImageSize[0];
		pI->ImageSize.cy = IntInfo.ImageSize[1];
	}

	CreateBufLine();
	if (pI->m_pDIB) {
		CreateRedCenters();
		SelectFringeStep();
		SelectMainSection();
		CreateNumLines();
	}
	else {
		TRACE("LoadZAP: No image loaded - skipping fringe processing (CreateRedCenters/CreateNumLines)\n");
	}
	CreateZAPSectionsOnLoadZAPFile();
	Delete_buf_line();

	return TRUE;
}

BOOL CDigitInfo::LoadFRN(LPCTSTR fname)
{
	CString FileName = fname;
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!ReadFRNData(FileName, IntInfo))
		return FALSE;

	// --- Resolve image filename relative to FRN file directory ---
	if (!IntInfo.ImageFileName.IsEmpty()) {
		std::string frnFile = CT2A(FileName);
		std::string imgFile = CT2A(IntInfo.ImageFileName);
		std::string resolved = ResolveImagePath(frnFile, imgFile);
		IntInfo.ImageFileName = CString(resolved.c_str());
		TRACE("LoadFRN: Image path resolved to: %s\n", resolved.c_str());
	}

	//Вызов LoadImage для инициализации m_pDIB
	CImageCtrls* pI = GetImageCtrls();
	BOOL imageLoaded = FALSE;

	if (!IntInfo.ImageFileName.IsEmpty()) {
		imageLoaded = pI->LoadImage(IntInfo.ImageFileName);
		if (!imageLoaded) {
			TRACE("LoadFRN: Failed to load image %s\n", CT2A(IntInfo.ImageFileName));
		}
	}

	// Create fake gray image if actual image failed to load
	if (!imageLoaded && IntInfo.ImageSize[0] > 0 && IntInfo.ImageSize[1] > 0) {
		TRACE("LoadFRN: Creating fake gray image as fallback\n");
		if (CreateFakeGrayImage(pI, IntInfo.ImageSize[0], IntInfo.ImageSize[1])) {
			imageLoaded = TRUE; // Treat as successful load
			AfxMessageBox(_T("Изображение отсутствует. Создан серый фон для отображения векторных данных."));
			TRACE("LoadFRN: Fake image created successfully\n");
		}
		else {
			AfxMessageBox(_T("Не удалось создать изображение для отображения векторных данных."));
			TRACE("LoadFRN: Failed to create fake image\n");
		}
	}

	if (!ExamineNumberingInterferogramInfo(IntInfo))
		return FALSE;

	CreateBufLine();
	if (pI->m_pDIB) {
		CreateRedCenters();
	}
	Delete_buf_line();
	return TRUE;
}

BOOL CDigitInfo::SaveZAP(LPCTSTR fname, int extIdx)
{
	CString FileName = fname;
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return FALSE;
	if (extIdx == 2)
		WriteWinZAPData(FileName, IntInfo);
	else if (extIdx == 3)
		WriteDosZAPData(FileName, IntInfo);
	return TRUE;
}

BOOL CDigitInfo::SaveFRN(LPCTSTR fname)
{
	CString FileName = fname;
	NUMBERING_INTERFEROGRAM_INFO IntInfo;
	if (!CollectNumberingInterferogramInfo(IntInfo))
		return FALSE;
	WriteFRNData(FileName, IntInfo);
	return TRUE;
}


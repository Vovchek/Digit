void CDigitInfo::SelectFringeStep()
{
	CArray<double, double> SortedSteps;

	for (int i = 0; i < Sections.GetSize(); i++) {
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

/// <summary>
/// Selects the main section for interferogram analysis by finding the section with the maximum
/// number of fringe lines within an acceptable step range. Performs bidirectional scanning
/// (forward and backward) to identify candidate sections and determines the optimal main section.
/// </summary>
/// <remarks>
/// Algorithm steps:
/// 1. Calculates acceptable step range using SecSegm ± (SecSegm * CorrectionSecSegm)
/// 2. Forward scan (first to last): Finds section with maximum fringe count meeting criteria
/// 3. Backward scan (last to first): Finds section with maximum fringe count meeting criteria
/// 4. Selection logic:
///    - If two different valid sections found and not inside screen: selects middle section
///    - Otherwise: selects the first valid section (iS1)
/// 
/// The nTimes counter (max 5) prevents selecting sections too far from the first candidate
/// with the same fringe count, ensuring stability in selection.
/// 
/// Sets:
/// - Sections[idx].MainLine = TRUE for the selected section
/// - idxMainSection to the index of the selected section
/// </remarks>
void CDigitInfo::SelectMainSection()
{
	double minSecSegm = SecSegm - SecSegm * CorrectionSecSegm;
	double maxSecSegm = SecSegm + SecSegm * CorrectionSecSegm;
	int maxFringeNumber1 = -1;
	int maxFringeNumber2 = -1;
	int iS1 = -1;
	int iS2 = -1;
	const int MAX_HITS = 5;
	
	// Forward scan: Find section with maximum fringe lines within acceptable step range
	int hitCount = 0;
	for (int i = 0; i < Sections.GetSize(); i++) {
		double step = Sections[i].aveStep;
		if (minSecSegm < step && step < maxSecSegm) {
			int nDots = Sections[i].NumLines.GetSize();
			if (nDots > maxFringeNumber1) hitCount = 0;
			if (nDots >= maxFringeNumber1 && hitCount < MAX_HITS) {
				if (nDots == maxFringeNumber1) ++hitCount;
				maxFringeNumber1 = nDots;
				iS1 = i;
			}
		}
	}
	
	// Backward scan: Find section with maximum fringe lines within acceptable step range
	hitCount = 0;
	for (int i = Sections.GetSize() - 1; i > -1; i--) {
		double step = Sections[i].aveStep;
		if (minSecSegm < step && step < maxSecSegm) {
			int nDots = Sections[i].NumLines.GetSize();
			if (nDots > maxFringeNumber2) hitCount = 0;
			if (nDots >= maxFringeNumber2 && hitCount < MAX_HITS && maxFringeNumber1 == maxFringeNumber2) {
				if (nDots == maxFringeNumber2) ++hitCount;
				maxFringeNumber2 = nDots;
				iS2 = i;
			}
		}
	}
	
	// Select main section based on forward/backward scan results
	if (iS1 != -1 && iS2 != -1 && iS1 != iS2 && !isInsideScreen) {
		// Two different valid sections found: select middle section
		int idx = (iS1 + iS2) / 2;
		Sections[idx].MainLine = TRUE;
		idxMainSection = idx;
	}
	else {
		// Use first valid section from forward scan
		if (iS1 != -1) {
			Sections[iS1].MainLine = TRUE;
			idxMainSection = iS1;
		}
		//	 if(iS2 != -1)
		//		Sections[iS2].VisibleLine = TRUE;
	}
}

#include "DigitMode\CreateNumLines.hxx"

/// <summary>
/// Selects the main fringe number by finding the fringe that appears most consistently
/// across different sections. A fringe is considered to appear in a section if it exists
/// exactly once within that section.
/// </summary>
/// <remarks>
/// Algorithm steps:
/// 1. Validates that a main section has been selected (idxMainSection != -1)
/// 2. Collects all unique fringe numbers from the main section's NumLines
/// 3. For each fringe number:
///    - Iterates through all sections in the image
///    - Counts how many sections contain that fringe number exactly once
///    - Stores this count in a map (key: fringe number, value: occurrence count)
/// 4. Selects the fringe number with the highest occurrence count across all sections
/// 5. If multiple fringes have the same count, the first one encountered is selected
/// 
/// The selected main fringe number is stored in MainFringeNumber and is used for
/// correcting and renumbering fringe lines throughout the interferogram.
/// 
/// Sets:
/// - MainFringeNumber to the selected fringe number (or -1000.0 if no valid fringe found)
/// </remarks>
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

#include "DigitMode\SelectNumber.hxx"

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
	//CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	//CRect BoundR;
	//if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
	//	return;
	int maxN = INT_MIN;
	int minN = INT_MAX;
	for (int iS = 0; iS < Sections.GetSize(); iS++) {
		for (int iL = 0; iL < Sections[iS].NumLines.GetSize(); iL++) {
			if (Sections[iS].NumLines[iL].Number != -1000.)
				minN = __min(minN, static_cast<int>(Sections[iS].NumLines[iL].Number));
			maxN = __max(maxN, static_cast<int>(Sections[iS].NumLines[iL].Number));
		}
	}
	int nFringes = static_cast<int>((maxN - minN) / numStep + 1);
	int begY = 0;
	// switched to apertures
	auto pA = GetApertureCtrls();
	auto BoundR = pA->GetShapes().getVisibleRegion();
	int bH = static_cast<int>(BoundR.height());
	// ----------------------
	double SecGap;
	if (ZapLines.GetSize() < 2) {
		int nSections = static_cast<int>(2. * nFringes * BoundR.height() / BoundR.width());
		SecGap = std::ceil(BoundR.height() / (nSections - 1));
		begY = static_cast<int>(BoundR.top);
	}
	else {
		SortZapLines();
		int maxSize = ZapLines.GetSize();
		SecGap = (fabs(ZapLines[maxSize - 1].L.P1.y - ZapLines[0].L.P1.y)) / (maxSize - 1);
		begY = static_cast<int>(ZapLines[maxSize - 1].L.P1.y - BoundR.top);
		nFringes = static_cast<int>(((double)(bH - begY)) / SecGap + 1);
	}

	int nLines = static_cast<int>(bH / SecGap) + 2;
	for (int i = 0; i < nLines; i++) {
		int iy = static_cast<int>(begY + SecGap * i);
		if (iy < 0 || iy >= Sections.GetSize())
			continue;
		if (iy >= begY + static_cast<int>(BoundR.height())) {
			iy = begY + static_cast<int>(BoundR.height());
		}
		CZapLineInfo zL;
		zL.L = Sections[iy].L;
		zL.iSec = iy;
		ZapLines.Add(zL);
	}

	for (int i = 0; i < ZapLines.GetSize(); i++) {
		PutDotsOnZAPSections(i);
	}
	HandSetZapLines = FALSE;
}

void CDigitInfo::CreateZAPSectionsOnLoadZAPFile()
{
	CImageCtrls* pI = GetImageCtrls();
	//CBoundCtrls* pB = GetBoundCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	//CRect BoundR;
	//if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
	//	return;

	CList<double, double> YLines;
	double y;
	for (size_t i = 0; i < Dots.size(); i++) {
		y = Dots[i].P.y;
		if (!YLines.Find(y))
			YLines.AddTail(y);
	}
	// Switched to aperture
	auto BoundR = GetApertureCtrls()->GetShapes().getVisibleRegion();
	// --------------------
	int ext_t_y = static_cast<int>(BoundR.top);
	int iy;
	POSITION pos = YLines.GetHeadPosition();
	for (int i = 0; i < YLines.GetCount(); i++) {
		y = YLines.GetNext(pos);
		iy = int(y);
		CZapLineInfo zL;
		if (iy >= 0 && iy < Sections.GetSize()) {
			zL.L = Sections[iy].L;
			zL.iSec = iy;
		}
		else {
			zL.L.P1.x = BoundR.left;
			zL.L.P2.x = BoundR.right;
			zL.L.P1.y = zL.L.P2.y = iy;
			zL.iSec = iy;
		}
		ZapLines.Add(zL);
	}

	CPoint P;
	int idx;
	for (size_t i = 0; i < Dots.size(); i++) {
		P.x = static_cast<LONG>(Dots[i].P.x); 
		P.y = static_cast<LONG>(Dots[i].P.y);
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

void CDigitInfo::SortDotsFY()
{
	if (Dots.empty())
		return;

	std::sort(Dots.begin(), Dots.end(), [](const CDotInfo& a, const CDotInfo& b) {
		if (a.Number != b.Number)
			return a.Number < b.Number;
		if (a.segIdx != b.segIdx)
			return a.segIdx < b.segIdx;
		return a.P.y < b.P.y;
		});
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
			Dots.emplace_back(Dot);
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
	return false;
	// deprecated/eliminated - no zap sections anymore
	//if (ZapLines.GetSize())
	//	return true;
	//else
	//	return false;
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

// deprecated/eliminated - no zap sections anymore
void CDigitInfo::SetLockedZapSectionYPos(int iy)
{
	////CBoundCtrls* pB = GetBoundCtrls();
	//CImageCtrls* pI = GetImageCtrls();
	//int xDIB = pI->ImageSize.cx;
	//int yDIB = pI->ImageSize.cy;
	//CRect BoundR;
	////if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
	////	return;
	//int begY = BoundR.top;
	//int i = iy - begY;
	//if (Sections.GetSize()) {
	//	ZapLines[idxDragZapLine].L = Sections[i].L;
	//}
	//else {
	//	CDLine L;
	//	L.P1.y = L.P2.y = iy;
	//	L.P1.x = BoundR.left;
	//	L.P2.x = BoundR.right;
	//	ZapLines[idxDragZapLine].L = L;
	//}
}

// deprecated/eliminated - no zap sections anymore
void CDigitInfo::GetLockedZapSectionXYPos(CPoint& P1, CPoint& P2)
{
	//P1.x = (int)ZapLines[idxDragZapLine].L.P1.x;
	//P1.y = (int)ZapLines[idxDragZapLine].L.P1.y;

	//P2.x = (int)ZapLines[idxDragZapLine].L.P2.x;
	//P2.y = (int)ZapLines[idxDragZapLine].L.P2.y;
}

// deprecated/eliminated - no zap sections anymore
void CDigitInfo::AddZapSection(int iy)
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
	//CZapLineInfo zL;
	//if (Sections.GetSize() && i >= 0 && i < Sections.GetSize()) {
	//	zL.L = Sections[i].L;
	//	zL.iSec = i;
	//	ZapLines.Add(zL);
	//	PutDotsOnZAPSections(ZapLines.GetSize() - 1);
	//}
	//else {
	//	CDLine L;
	//	L.P1.y = L.P2.y = iy;
	//	L.P1.x = BoundR.left;
	//	L.P2.x = BoundR.right;
	//	zL.iSec = i;
	//	zL.L = L;
	//	ZapLines.Add(zL);
	//}
	//HandSetZapLines = TRUE;
}

void CDigitInfo::SectionLeft(CPoint P, int dotSide)
{
	int idx;
	if (static_cast<size_t>(idxMainDot) < Dots.size()) {
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
	if (static_cast<size_t>(idxMainDot) < Dots.size()) {
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Dots[iD].iZapSec == iSec) {
			Dots.erase(Dots.begin()+iD);
			iD--;
		}
	}
}

void CDigitInfo::AddDot(CPoint P, int dotSide)
{
	if (m_bUseFringeModel) {
		int targetFringe = -1;
		for (size_t iF = 0; iF < Fringes.size(); iF++) {
			if (Fringes[iF].GetNumber() == CurrentNumber) {
				targetFringe = static_cast<int>(iF);
				break;
			}
		}
		if (targetFringe < 0) {
			targetFringe = CreateFringe(CurrentNumber);
		}
		AddPointToFringe(targetFringe, CDPoint(P.x, P.y));
		SyncFringesToDots();
		idxMainPoint = SelectedPoint(targetFringe, Fringes[targetFringe].GetPointCount() - 1);
		CurrentNumber = Fringes[targetFringe].GetNumber();
		return;
	}

	int idx;
	CControls* pCtrls = GetControls();
	CDotInfo dot;
	dot.Number = CurrentNumber;
	dot.P.x = P.x;
	dot.P.y = P.y;
	if (pCtrls->ViewState & V_ZAPSECTIONS) {
		if (GetNearestZapSection(P, idx)) {
			dot.iZapSec = idx;
			dot.P.y = ZapLines[idx].L.P1.y;
			P.y = static_cast<LONG>(dot.P.y);
		}
	}
	/*if (pCtrls->ViewState & V_EXTREMUMS) {
	   double x;
	   if(GetNearestXInSection(P, x)){
		   dot.P.x = x;
	   }
	}*/
	Dots.emplace_back(dot);
}

void CDigitInfo::RemoveDot(CPoint P, int dotSide)
{
	if (m_bUseFringeModel) {
		int tol = dotSide / 2;
		int fIdx, pIdx;
		if (FindPointUnderCursor(P, tol, fIdx, pIdx)) {
			RemovePointFromFringe(fIdx, pIdx);
			SyncFringesToDots();
		}
		return;
	}

	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		Dots.erase(Dots.begin()+idx);
	}
}

void CDigitInfo::RemoveFringe(CPoint P, int dotSide)
{
	if (m_bUseFringeModel) {
		int tol = dotSide / 2;
		int fIdx, pIdx;
		if (FindPointUnderCursor(P, tol, fIdx, pIdx)) {
			double Number = Fringes[fIdx].GetNumber();
			for (int i = Fringes.size() - 1; i >= 0; i--) { // TODO: modernize with erase-remove idiom
				if (Fringes[i].GetNumber() == Number) {
					Fringes.erase(Fringes.begin()+i);
				}
			}
			SyncFringesToDots();
		}
		return;
	}

	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		double Number = Dots[idx].Number;
		for (size_t i = 0; i < Dots.size(); i++) {
			if (Dots[i].Number == Number) {
				Dots.erase(Dots.begin()+i);
				i--;
			}
		}
	}
}

bool CDigitInfo::IsDots()
{
	if (Dots.size())
		return true;
	else
		return false;
}

bool CDigitInfo::IsDotUnderCursor(CPoint P, int dotSide, int& idx)
{
	int DotSide12 = dotSide / 2;
	CPoint lP;
	CRect dotR;
	for (size_t i = 0; i < Dots.size(); i++) {
		lP.x = static_cast<int>(Dots[i].P.x);
		lP.y = static_cast<int>(Dots[i].P.y);
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
	if (m_bUseFringeModel) {
		return idxDraggedPoint.IsValid();
	}

	return idxDragDot != -1;
}

bool CDigitInfo::LockDot(CPoint P, int dotSide, BOOL Enable)
{
	if (m_bUseFringeModel) {
		if (!Enable) {
			idxDraggedPoint.Clear();
			return true;
		}
		int tol = dotSide / 2;
		int fIdx, pIdx;
		if (FindPointUnderCursor(P, tol, fIdx, pIdx)) {
			idxDraggedPoint = SelectedPoint(fIdx, pIdx);
			idxMainPoint = idxDraggedPoint;
			CurrentNumber = Fringes[fIdx].GetNumber();
			return true;
		}
		return false;
	}

	if (Dots.size() == 0)
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
	if (m_bUseFringeModel) {
		if (idxDraggedPoint.IsValid()) {
			MovePointInFringe(idxDraggedPoint.iFringe, idxDraggedPoint.iPoint, CDPoint(P.x, P.y));
			SyncFringesToDots();
		}
		return;
	}

	Dots[idxDragDot].P.x = P.x;
	Dots[idxDragDot].P.y = P.y;
}

void CDigitInfo::GetLockedDotPos(CPoint& P1)
{
	if (m_bUseFringeModel) {
		if (idxDraggedPoint.IsValid()) {
			const CFringeSegment* pFr = GetFringe(idxDraggedPoint.iFringe);
			if (pFr && idxDraggedPoint.iPoint >= 0 && idxDraggedPoint.iPoint < pFr->GetPointCount()) {
				CDPoint d = pFr->GetPoint(idxDraggedPoint.iPoint);
				P1.x = (int)d.x;
				P1.y = (int)d.y;
				return;
			}
		}
		P1.x = P1.y = 0;
		return;
	}

	P1.x = static_cast<LONG>(Dots[idxDragDot].P.x);
	P1.y = static_cast<LONG>(Dots[idxDragDot].P.y);
}

void CDigitInfo::SelectMainDot(CPoint P, int dotSide)
{
	if (m_bUseFringeModel) {
		int tol = dotSide / 2;
		int fIdx, pIdx;
		if (FindPointUnderCursor(P, tol, fIdx, pIdx)) {
			idxMainPoint = SelectedPoint(fIdx, pIdx);
			CurrentNumber = Fringes[fIdx].GetNumber();
		}
		return;
	}

	int idx;
	if (IsDotUnderCursor(P, dotSide, idx)) {
		idxMainDot = idx;
		CurrentNumber = Dots[idx].Number;
	}
}

void CDigitInfo::SelectMainDot(int iZapSec/*=-1*/, double Number/*=INT_MIN*/)
{
	if (m_bUseFringeModel) {
		for (size_t iF = 0; iF < Fringes.size(); iF++) {
			if (Number == INT_MIN || fabs(Fringes[iF].GetNumber() - Number) < 1e-6) {
				if (Fringes[iF].GetPointCount() > 0) {
					idxMainPoint = SelectedPoint(static_cast<int>(iF), 0);
					CurrentNumber = Fringes[iF].GetNumber();
					return;
				}
			}
		}
		idxMainPoint.Clear();
		return;
	}

	if (iZapSec == -1 && Number == INT_MIN && Dots.size()) {
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Dots[iD].iZapSec == iZapSec && Dots[iD].Number == Number) {
			idx = static_cast<int>(iD);
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Dots[iD].iZapSec == iZapSec) {
			if (Dots[iD].P.x < minx) {
				minx = Dots[iD].P.x;
				minidx = static_cast<int>(iD);
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Number == Dots[iD].Number) {
			idxDots.Add(static_cast<int>(iD));
		}
	}
	return true;
}

bool CDigitInfo::GetFringeDots(double Number, CArray<CDPoint>& adP)
{
	adP.RemoveAll();
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Number == Dots[iD].Number) {
			adP.Add(Dots[iD].P);
		}
	}
	//SortDots(adP, 2);
	return true;
}

bool CDigitInfo::GetFirstDotInFringe(double Number, int& idx, CDPoint& dP)
{
	int minidx = -1;
	double minx = INT_MAX;
	for (size_t iD = 0; iD < Dots.size(); iD++) {
		if (Dots[iD].Number == Number) {
			if (Dots[iD].P.x < minx) {
				minx = Dots[iD].P.x;
				minidx = static_cast<int>(iD);
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
	for (size_t iD = 0; iD < Dots.size(); iD++) {
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

	//CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pI = GetImageCtrls();
	int xDIB = pI->ImageSize.cx;
	int yDIB = pI->ImageSize.cy;
	auto* pA = GetApertureCtrls();
	//CRect BoundR;
	//if (!pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE))
	//	return false;
	// switched to apertures
	auto BoundR = pA->GetShapes().getVisibleRegion();
	// ---------------------
	int begY = static_cast<int>(BoundR.top);
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

#include "DigitMode/WavefrontSolver/WavefrontFromContours.h"

BOOL CDigitInfo::Save(LPCTSTR fname, int extIdx)
{
	CString path = fname;
	CString ext = path.Right(3);
	ext.MakeLower();
	if (extIdx == FilterIndex::Filter_FRN ||
		extIdx == FilterIndex::Filter_WinFringe_FRN) {
		bool saveAsWinFringe = (extIdx == FilterIndex::Filter_WinFringe_FRN);
		return SaveFRN(fname, saveAsWinFringe);
	}
	else if (extIdx == FilterIndex::Filter_MTR) {
		DigitMode::CApertureCtrls* pA = GetApertureCtrls();
		auto& aperture = pA->GetShapes().getVisibleRegion();
		auto mask = pA->GetMaskProvider().getMask();

		WavefrontFromContoursInput input(
			aperture,
			mask,
			Fringes
		);
		WavefrontFromContours wf(input);
		WavefrontFromContoursSolver_HorizontalSpline solver;

		auto topogram = wf.run(solver);
		topogram.setScaleFactor(GetScaleFactor());

		std::ofstream outMtr(fname);
		return topogram.saveMtrMatrix(outMtr);
	}
	
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
	// TODO: switch to CFringeSegment array latter
	m_bUseFringeModel = true;

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

	return TRUE;
}

BOOL CDigitInfo::LoadFRN(LPCTSTR fname)
{
	// Load FRN into CFringeSegment array, create legacy Dots from it
	m_bUseFringeModel = true;

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

	
	// Call LoadImage to initialize m_pDIB
	CImageCtrls* pI = GetImageCtrls();

	if (!ExamineNumberingInterferogramInfo(IntInfo))
		return FALSE;

	if (pI->m_pDIB) {
		CreateRedCenters();
	}
	return TRUE;
}
/*
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
*/
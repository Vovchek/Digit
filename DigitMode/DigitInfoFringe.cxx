// ===== NEW: Fringe-based operations =====

int CDigitInfo::CreateFringe(double number, int segment)
{
	CFringe fringe(number, segment);
	Fringes.Add(fringe);
	return Fringes.GetSize() - 1;
}

void CDigitInfo::DeleteFringe(int iFringe)
{
	if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
		Fringes.RemoveAt(iFringe);
	}
}

CFringe* CDigitInfo::GetFringe(int i)
{
	if (i >= 0 && i < Fringes.GetSize()) {
		return &Fringes[i];
	}
	return NULL;
}

const CFringe* CDigitInfo::GetFringe(int i) const
{
	if (i >= 0 && i < Fringes.GetSize()) {
		return &Fringes[i];
	}
	return NULL;
}

void CDigitInfo::FindFringesByNumber(double number, CArray<int>& indices)
{
	indices.RemoveAll();
	for (int iF = 0; iF < Fringes.GetSize(); iF++) {
		if (Fringes[iF].GetNumber() == number) {
			indices.Add(iF);
		}
	}
}

void CDigitInfo::AddPointToFringe(int iFringe, CDPoint p)
{
	if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
		Fringes[iFringe].AddPoint(p);
	}
}

void CDigitInfo::InsertPointInFringe(int iFringe, int iPoint, CDPoint p)
{
	if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
		Fringes[iFringe].InsertPoint(iPoint, p);
	}
}

void CDigitInfo::RemovePointFromFringe(int iFringe, int iPoint)
{
	if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
		Fringes[iFringe].RemovePoint(iPoint);
	}
}

void CDigitInfo::MovePointInFringe(int iFringe, int iPoint, CDPoint newP)
{
	if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
		Fringes[iFringe].MovePoint(iPoint, newP);
	}
}

BOOL CDigitInfo::FindPointUnderCursor(CPoint P, int tolerance, int& outFringe, int& outPoint)
{
	for (int iF = 0; iF < Fringes.GetSize(); iF++) {
		int iPt = Fringes[iF].FindNearestPoint(P, tolerance);
		if (iPt >= 0) {
			outFringe = iF;
			outPoint = iPt;
			return TRUE;
		}
	}
	outFringe = outPoint = -1;
	return FALSE;
}

BOOL CDigitInfo::FindFringeUnderCursor(CPoint P, int tolerance, int& outFringe)
{
	int outPoint;
	return FindPointUnderCursor(P, tolerance, outFringe, outPoint);
}

void CDigitInfo::RenumberFringes(double oldNumber, double newNumber)
{
	for (int iF = 0; iF < Fringes.GetSize(); iF++) {
		if (Fringes[iF].GetNumber() == oldNumber) {
			Fringes[iF].SetNumber(newNumber);
		}
	}
}

void CDigitInfo::DeleteFringesByNumber(double number)
{
	for (int iF = Fringes.GetSize() - 1; iF >= 0; iF--) {
		if (Fringes[iF].GetNumber() == number) {
			Fringes.RemoveAt(iF);
		}
	}
}

BOOL CDigitInfo::MergeFringes(int iFringe1, int iFringe2)
{
	if (iFringe1 < 0 || iFringe1 >= Fringes.GetSize()) return FALSE;
	if (iFringe2 < 0 || iFringe2 >= Fringes.GetSize()) return FALSE;
	if (iFringe1 == iFringe2) return FALSE;

	// Only merge fringes with same number
	if (Fringes[iFringe1].GetNumber() != Fringes[iFringe2].GetNumber()) {
		return FALSE;
	}

	// Append all points from iFringe2 to iFringe1
	Fringes[iFringe1].AppendPoints(Fringes[iFringe2]);

	// Delete iFringe2
	Fringes.RemoveAt(iFringe2);

	return TRUE;
}

// ===== Conversion utilities (Transition only) =====

void CDigitInfo::ConvertDotsToFringes()
{
	Fringes.RemoveAll();

	if (Dots.GetSize() == 0) return;

	// Group by (Number, segment) using a map-like structure
	// Build a simple grouping structure
	for (int i = 0; i < Dots.GetSize(); i++) {
		double num = Dots[i].Number;
		int segment = Dots[i].segIdx;

		// Find or create fringe for this (Number, segment index)
		int targetFringe = -1;
		for (int iF = 0; iF < Fringes.GetSize(); iF++) {
			// Check if fringe has same number and segment
			if (Fringes[iF].GetNumber() == num && Fringes[iF].GetIndex() == segment) {
				targetFringe = iF;
				break;
			}
		}

		if (targetFringe < 0) {
			// Create new fringe
			targetFringe = CreateFringe(num);
		}

		// Add point to fringe
		Fringes[targetFringe].AddPoint(Dots[i].P);
	}
}

void CDigitInfo::ConvertFringesToDots()
{
	Dots.RemoveAll();

	for (int iF = 0; iF < Fringes.GetSize(); iF++) {
		double number = Fringes[iF].GetNumber();
		int segment = Fringes[iF].GetIndex();

		for (int iP = 0; iP < Fringes[iF].GetPointCount(); iP++) {
			CDotInfo dot;
			dot.P = Fringes[iF].GetPoint(iP);
			dot.Number = number;
			dot.segIdx = segment;
			dot.iZapSec = -1;  // Will be set later if needed
			Dots.Add(dot);
		}
	}

	// Assign iZapSec based on ZapLines if they exist
	if (ZapLines.GetSize() > 0) {
		CPoint P;
		int idx;
		for (int i = 0; i < Dots.GetSize(); i++) {
			P.x = (int)Dots[i].P.x;
			P.y = (int)Dots[i].P.y;
			if (GetNearestZapSection(P, idx)) {
				Dots[i].iZapSec = idx;
			}
		}
	}
}

void CDigitInfo::SyncFringesToDots()
{
	// Update Dots array from Fringes (maintains both models)
	if (m_bUseFringeModel)
		ConvertFringesToDots();
	else
		ConvertDotsToFringes();
}

// ===== File I/O methods =====

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

	pIm->ImageSize.cx = IntInfo.ImageSize[0];
	pIm->ImageSize.cy = IntInfo.ImageSize[1];

	int nD = IntInfo.DigitDat.GetSize();
	if (nD == 0)
		return FALSE;

	if (m_bUseFringeModel) {
		// NEW: Group into fringes by (Number, segment index)
		Fringes.RemoveAll();

		// Build grouping: (Number, Index) -> points
		// Using simple linear search approach (MFC-compatible)
		for (int i = 0; i < nD; i++) {
			double num = IntInfo.DigitDat[i].F.Number;
			int id = IntInfo.DigitDat[i].F.Index;
			double y = IntInfo.DigitDat[i].Y;
			double x = IntInfo.DigitDat[i].X;

			// Find or create fringe for this (Number, id) combination
			int targetFringe = -1;
			for (int iF = 0; iF < Fringes.GetSize(); iF++) {
				if (Fringes[iF].GetNumber() == num && Fringes[iF].GetIndex() == id) {
					targetFringe = iF;
					break;
				}
			}

			if (targetFringe < 0) {
				// Create new fringe segment
				targetFringe = CreateFringe(num, id);
			}

			// Add point to fringe
			Fringes[targetFringe].AddPoint(CDPoint(x, y));
		}

		// Maintain Dots for compatibility during transition
		ConvertFringesToDots();
	}
	else {
		// OLD: Flat loading into Dots array
		Dots.SetSize(nD);
		for (int i = 0; i < nD; i++) {
			Dots[i].P.x = IntInfo.DigitDat[i].X;
			Dots[i].P.y = IntInfo.DigitDat[i].Y;
			Dots[i].Number = IntInfo.DigitDat[i].F.Number;
			Dots[i].segIdx = IntInfo.DigitDat[i].F.Index;
			Dots[i].iZapSec = -1;
		}
	}

	return TRUE;
}

BOOL CDigitInfo::CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO& IntInfo)
{
	IntInfo.Clear();
	IntInfo.Title = Comments;
	IntInfo.ScaleFactor = ScaleFactor;
	IntInfo.FiScan = Rotation;

	CBoundCtrls* pB = GetBoundCtrls();
	CImageCtrls* pIm = GetImageCtrls();

	IntInfo.ArrEll.RemoveAll();
	IntInfo.ArrEll.Append(pB->ArrEll);
	IntInfo.ArrRect.RemoveAll();
	IntInfo.ArrRect.Append(pB->ArrRect);
	IntInfo.ArrPlg.RemoveAll();
	IntInfo.ArrPlg.Append(pB->ArrPlg);

	IntInfo.ImageSize[0] = pIm->ImageSize.cx;
	IntInfo.ImageSize[1] = pIm->ImageSize.cy;
	IntInfo.ImageFileName = pIm->OriginalPath;

	// m_bUseFringeModel = (Fringes.GetSize() > 0); // TODO: consider where to choose model

	if (m_bUseFringeModel) {
		// NEW: Direct fringe iteration
		int totalPoints = 0;
		for (int iF = 0; iF < Fringes.GetSize(); iF++) {
			totalPoints += Fringes[iF].GetPointCount();
		}

		if (totalPoints == 0)
			return FALSE;

		IntInfo.DigitDat.SetSize(totalPoints);

		int idx = 0;
		for (int iF = 0; iF < Fringes.GetSize(); iF++) {
			double number = Fringes[iF].GetNumber();
			int index = Fringes[iF].GetIndex();
			for (int iP = 0; iP < Fringes[iF].GetPointCount(); iP++) {
				CDPoint p = Fringes[iF].GetPoint(iP);
				IntInfo.DigitDat[idx].X = p.x;
				IntInfo.DigitDat[idx].Y = p.y;
				IntInfo.DigitDat[idx].F.Number = number;
				IntInfo.DigitDat[idx].F.Index = index;
				IntInfo.DigitDat[idx].P = 0.;
				idx++;
			}
		}
	}
	else {
		// OLD: Existing Dots iteration
		int nD = Dots.GetSize();
		if (nD == 0)
			return FALSE;

		for (int i = 0; i < nD; i++) {
			IntInfo.DigitDat.Add(
				Dots[i].P.x, Dots[i].P.y, Dots[i].Number, 0.0, Dots[i].segIdx);
		}
		// IntInfo.DigitDat.SortIncreaseFY(); // FIXED: do sorting only after auto-detection, don't spoil a craftwork
	}

	return TRUE;
}

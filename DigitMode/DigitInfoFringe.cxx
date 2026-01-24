// ===== NEW: Fringe-based operations =====

int CDigitInfo::CreateFringe(double number)
{
	CFringe fringe(number);
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

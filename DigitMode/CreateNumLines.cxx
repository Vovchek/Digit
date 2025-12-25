/// This file is intended to be icluded into DigitInfo.cpp
/// It was extracted to facilitate testing and debugging of CreateNumLines()

// TODO: refNumLines array index goes out of bound when sections do not fit the window
/// <summary>
/// Creates and assigns fringe numbers to all sections based on the main section.
/// This function propagates numbering from the main section to adjacent sections
/// by matching fringe positions across neighboring scan lines.
/// </summary>
/// <remarks>
/// Algorithm:
/// 1. Initialize numbers for the main section (idxMainSection) sequentially
/// 2. Propagate upward (i = idxMain-1 to 0): Match fringes to previous section
/// 3. Propagate downward (i = idxMain+1 to end): Match fringes to previous section
/// 
/// For each section:
/// - Creates a reference array (refNumLines) sized to accommodate existing + new fringes
/// - Attempts to match each fringe to a numbered fringe in the adjacent processed section
/// - If matched: assigns the same number
/// - If unmatched: creates new number at left edge (minN - numStep) or right edge (maxN + numStep)
/// 
/// TODO: CRITICAL BUG - refNumLines array index can go out of bounds when:
/// - Sections don't fit within the window boundaries
/// - idxN calculation exceeds array size after collision resolution (idxN++)
/// - Dynamic fringe insertions exceed pre-calculated maxSize
/// 
/// Root cause: maxSize calculation doesn't account for all edge cases:
///   maxSize = max((maxN - minN)/numStep + 2, Sections[i].NumLines.GetSize())
/// This may be insufficient when many new fringes appear at edges or collisions occur.
/// 
/// Potential fixes:
/// - Add bounds checking before accessing refNumLines[idxN]
/// - Dynamically resize refNumLines when needed
/// - Recalculate maxSize during iteration when new fringes are added
/// </remarks>
void CDigitInfo::CreateNumLines()
{
	int i = 0;
	// Main section index determined by SelectMainSection()
	int idxMain = idxMainSection;
	// Fringe segment width tolerance based on detected step size
	double SecSegm12 = SecSegm * CorrectionSecSegm;
	double N = -numStep;
	int maxNFringe = Sections[idxMain].NumLines.GetSize();

	// Initialize main section with sequential numbers starting from 0
	for (i = 0; i < Sections[idxMain].NumLines.GetSize(); i++) {
		Sections[idxMain].NumLines[i].segmL.P1.x = Sections[idxMain].NumLines[i].redX - SecSegm12;
		Sections[idxMain].NumLines[i].segmL.P2.x = Sections[idxMain].NumLines[i].redX + SecSegm12;
		N += numStep;
		Sections[idxMain].NumLines[i].Number = N;
		Sections[idxMain].NumLines[i].Included = TRUE;
	}

	// Temporary array to build numbered fringes for current section
	CArray<CNumLine> refNumLines;
	int idxS, idxL; // Section and line indices from SelectNumber match
	double minN, maxN; // Current numbering range
	double leftX, rightX; // Spatial extent of numbered fringes
	leftX = INT_MAX;
	rightX = INT_MIN;
	minN = Sections[idxMain].NumLines[0].Number;
	maxN = Sections[idxMain].NumLines[Sections[idxMain].NumLines.GetSize() - 1].Number;
	int maxSize = INT_MIN;

	// Propagate numbering upward from main section
	for (i = idxMain - 1; i > -1; i--) {
		refNumLines.RemoveAll();
		// Calculate required array size (existing numbers + current section fringes)
		maxSize = __max(maxSize, (maxN - minN) / numStep + 2);
		maxSize = __max(maxSize, Sections[i].NumLines.GetSize());
		refNumLines.SetSize(maxSize); // WARNING: May be insufficient!

		// Use section-specific step if available
		if (Sections[i].aveStep != -1)
			SecSegm12 = CorrectionSecSegm * Sections[i].aveStep;

		for (int iN = 0; iN < Sections[i].NumLines.GetSize(); iN++) {
			// Try to match current fringe to a numbered fringe in adjacent section
			if (SelectNumber(i, 1, Sections[i].NumLines[iN].redX, idxS, idxL)) {
				ASSERT(idxL < refNumLines.GetSize());
				int idxN = idxL;
				// Collision resolution: if slot occupied, try next slot
				if (idxN + 1 < refNumLines.GetSize() && refNumLines[idxN].Included)
					idxN++; // FIXED: BUG: idxN may still exceed bounds!
				refNumLines[idxN].redX = Sections[i].NumLines[iN].redX;
				leftX = __min(leftX, Sections[i].NumLines[iN].redX);
				rightX = __max(rightX, Sections[i].NumLines[iN].redX);
				refNumLines[idxN].segmL.P1.x = Sections[i].NumLines[iN].redX - SecSegm12;
				refNumLines[idxN].segmL.P2.x = Sections[i].NumLines[iN].redX + SecSegm12;
				refNumLines[idxN].Number = Sections[idxS].NumLines[idxL].Number;
				minN = __min(minN, refNumLines[idxN].Number);
				maxN = __max(maxN, refNumLines[idxN].Number);
				refNumLines[idxN].Included = TRUE;
			}
			else {
				// No match found - create new number at edge
				CNumLine nL;
				nL.redX = Sections[i].NumLines[iN].redX;
				nL.Included = TRUE;
				nL.segmL.P1.x = nL.redX - SecSegm12;
				nL.segmL.P2.x = nL.redX + SecSegm12;
				if (Sections[i].NumLines[iN].redX < leftX) {
					// New fringe at left edge
					nL.Number = minN - numStep;
					refNumLines.InsertAt(0, nL); // Shifts array, may cause bounds issues
					minN = __min(minN, nL.Number);
					maxN = __max(maxN, nL.Number);
					leftX = __min(leftX, nL.redX);
					rightX = __max(rightX, nL.redX);
				}
				else if (Sections[i].NumLines[iN].redX > rightX) {
					// New fringe at right edge
					nL.Number = maxN + numStep;
					refNumLines.Add(nL); // May exceed preallocated size
					minN = __min(minN, nL.Number);
					maxN = __max(maxN, nL.Number);
					leftX = __min(leftX, nL.redX);
					rightX = __max(rightX, nL.redX);
				}
				else {
					// Fringe between existing range but no match - orphan case
					int rr = 0; // TODO: Handle orphan fringes properly
				}
			}
		}
		// Replace section's NumLines with the newly numbered array
		Sections[i].NumLines.RemoveAll();
		Sections[i].NumLines.Append(refNumLines);
	}

	// Propagate numbering downward from main section (same logic as upward)
	for (i = idxMain + 1; i < Sections.GetSize(); i++) {
		refNumLines.RemoveAll();
		maxSize = __max(maxSize, (maxN - minN) / numStep + 2);
		maxSize = __max(maxSize, Sections[i].NumLines.GetSize());
		refNumLines.SetSize(maxSize); // WARNING: May be insufficient!

		if (Sections[i].aveStep != -1)
			SecSegm12 = CorrectionSecSegm * Sections[i].aveStep;

		for (int iN = 0; iN < Sections[i].NumLines.GetSize(); iN++) {
			if (SelectNumber(i, -1, Sections[i].NumLines[iN].redX, idxS, idxL)) {
				ASSERT(idxL < refNumLines.GetSize());
				int idxN = idxL;
				if ((idxN + 1) < refNumLines.GetSize() && refNumLines[idxN].Included)
					idxN++; // FIXED: BUG: idxN may exceed bounds!
				refNumLines[idxN].redX = Sections[i].NumLines[iN].redX;
				leftX = __min(leftX, Sections[i].NumLines[iN].redX);
				rightX = __max(rightX, Sections[i].NumLines[iN].redX);
				refNumLines[idxN].segmL.P1.x = Sections[i].NumLines[iN].redX - SecSegm12;
				refNumLines[idxN].segmL.P2.x = Sections[i].NumLines[iN].redX + SecSegm12;
				refNumLines[idxN].Number = Sections[idxS].NumLines[idxL].Number;
				minN = __min(minN, refNumLines[idxN].Number);
				maxN = __max(maxN, refNumLines[idxN].Number);
				refNumLines[idxN].Included = TRUE;
			}
			else {
				CNumLine nL;
				nL.redX = Sections[i].NumLines[iN].redX;
				nL.Included = TRUE;
				nL.segmL.P1.x = nL.redX - SecSegm12;
				nL.segmL.P2.x = nL.redX + SecSegm12;
				if (Sections[i].NumLines[iN].redX < leftX) {
					nL.Number = minN - numStep;
					refNumLines.InsertAt(0, nL);
					minN = __min(minN, nL.Number);
					maxN = __max(maxN, nL.Number);
					leftX = __min(leftX, nL.redX);
					rightX = __max(rightX, nL.redX);
				}
				else if (Sections[i].NumLines[iN].redX > rightX) {
					nL.Number = maxN + numStep;
					refNumLines.Add(nL);
					minN = __min(minN, nL.Number);
					maxN = __max(maxN, nL.Number);
					leftX = __min(leftX, nL.redX);
					rightX = __max(rightX, nL.redX);
				}
				else {
					int rr = 0; // TODO: Handle orphan fringes properly
				}
			}
		}
		Sections[i].NumLines.RemoveAll();
		Sections[i].NumLines.Append(refNumLines);
	}
}
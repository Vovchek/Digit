/// This file is intended to be icluded into DigitInfo.cpp
/// It was extracted to facilitate testing and debugging of CreateNumLines()

// TODO: refNumLines array index goes out of bound when sections do not fit the window

/// <summary>
/// Processes fringe numbering propagation for a single section.
/// This is a helper method extracted from CreateNumLines to eliminate code duplication.
/// </summary>
/// <param name="sectionIndex">Index of the section to process</param>
/// <param name="direction">Direction of propagation: -1 for upward, +1 for downward</param>
/// <param name="refNumLines">Reference array to build numbered fringes for current section</param>
/// <param name="maxSize">Current maximum size tracking variable</param>
/// <param name="minN">Current minimum number in range</param>
/// <param name="maxN">Current maximum number in range</param>
/// <param name="leftX">Current left spatial extent</param>
/// <param name="rightX">Current right spatial extent</param>
/// <remarks>
/// This method processes a single section during the propagation phase.
/// It matches fringes to numbered fringes in adjacent sections and assigns numbers.
/// For unmatched fringes, it creates new numbers at the left or right edge.
/// 
/// Refactored to use lambdas for eliminating repetitive bound updates and fringe initialization.
/// </remarks>
void CDigitInfo::ProcessSectionPropagation(int sectionIndex, int direction, 
	CArray<CNumLine>& refNumLines, int& maxSize, 
	double& minN, double& maxN, double& leftX, double& rightX)
{
	refNumLines.RemoveAll();
	// Calculate required array size (existing numbers + current section fringes)
	maxSize = __max(maxSize, (maxN - minN) / numStep + 2);
	maxSize = __max(maxSize, Sections[sectionIndex].NumLines.GetSize());
	refNumLines.SetSize(maxSize);

	// Use section-specific step if available
	double SecSegm12 = SecSegm * CorrectionSecSegm;
	if (Sections[sectionIndex].aveStep != -1)
		SecSegm12 = CorrectionSecSegm * Sections[sectionIndex].aveStep;

	// Lambda: Update spatial and number range bounds
	auto updateBounds = [&](double x, double number) {
		leftX = __min(leftX, x);
		rightX = __max(rightX, x);
		minN = __min(minN, number);
		maxN = __max(maxN, number);
	};

	// Lambda: Initialize a fringe line with common properties
	auto createFringeLine = [&](double x, double number) {
		CNumLine nL;
		nL.redX = x;
		nL.Included = TRUE;
		nL.segmL.P1.x = x - SecSegm12;
		nL.segmL.P2.x = x + SecSegm12;
		nL.Number = number;
		return nL;
	};

	// Lambda: Populate a fringe entry in refNumLines at specified index
	auto assignFringe = [&](int idx, double x, double number) {
		refNumLines[idx].redX = x;
		refNumLines[idx].segmL.P1.x = x - SecSegm12;
		refNumLines[idx].segmL.P2.x = x + SecSegm12;
		refNumLines[idx].Number = number;
		refNumLines[idx].Included = TRUE;
	};

	int idxS, idxL; // Section and line indices from SelectNumber match

	for (int iN = 0; iN < Sections[sectionIndex].NumLines.GetSize(); iN++) {
		double currentX = Sections[sectionIndex].NumLines[iN].redX;
		
		// Try to match current fringe to a numbered fringe in adjacent section
		if (SelectNumber(sectionIndex, direction, currentX, idxS, idxL)) {
			ASSERT(idxL >= 0);
			// Dynamic resize if needed
			if (idxL >= refNumLines.GetSize())
				refNumLines.SetSize(idxL + 1);
			
			int idxN = idxL;
			// Collision resolution: if slot occupied, try next slot
			if (idxN + 1 < refNumLines.GetSize() && refNumLines[idxN].Included)
				idxN++;
			
			double matchedNumber = Sections[idxS].NumLines[idxL].Number;
			assignFringe(idxN, currentX, matchedNumber);
			updateBounds(currentX, matchedNumber);
		}
		else {
			// No match found - create new number at edge
			if (currentX < leftX) {
				// New fringe at left edge
				double newNumber = minN - numStep;
				CNumLine nL = createFringeLine(currentX, newNumber);
				refNumLines.InsertAt(0, nL);
				updateBounds(currentX, newNumber);
			}
			else if (currentX > rightX) {
				// New fringe at right edge
				double newNumber = maxN + numStep;
				CNumLine nL = createFringeLine(currentX, newNumber);
				refNumLines.Add(nL);
				updateBounds(currentX, newNumber);
			}
			else {
				// Fringe between existing range but no match - orphan case
				// TODO: Handle orphan fringes properly
			}
		}
	}
	
	// Replace section's NumLines with the newly numbered array
	// Only copy numbered fringes (Included = TRUE), not empty slots
	Sections[sectionIndex].NumLines.RemoveAll();
	for (int idx = 0; idx < refNumLines.GetSize(); idx++) {
		if (refNumLines[idx].Included) {
			Sections[sectionIndex].NumLines.Add(refNumLines[idx]);
		}
	}
}

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
	// Main section index determined by SelectMainSection()
	int idxMain = max(0, min(idxMainSection, Sections.GetSize()));
	if (Sections.GetSize() < 1 || Sections[idxMain].NumLines.GetSize() < 1)
		return; // No sections or no fringes in main section

	// Fringe segment width tolerance based on detected step size
	double SecSegm12 = SecSegm * CorrectionSecSegm;
	double N = -numStep;

	// Initialize main section with sequential numbers starting from 0
	for (int i = 0; i < Sections[idxMain].NumLines.GetSize(); i++) {
		Sections[idxMain].NumLines[i].segmL.P1.x = Sections[idxMain].NumLines[i].redX - SecSegm12;
		Sections[idxMain].NumLines[i].segmL.P2.x = Sections[idxMain].NumLines[i].redX + SecSegm12;
		N += numStep;
		Sections[idxMain].NumLines[i].Number = N;
		Sections[idxMain].NumLines[i].Included = TRUE;
	}

	// Temporary array to build numbered fringes for current section
	CArray<CNumLine> refNumLines;
	double leftX{ INT_MAX };  //  Spatial extent
	double rightX{ INT_MIN }; //  of numbered fringes
	auto minN = Sections[idxMain].NumLines[0].Number;
	auto maxN = Sections[idxMain].NumLines[Sections[idxMain].NumLines.GetSize() - 1].Number;
	int maxSize{ INT_MIN };

	// Propagate numbering upward from main section
	for (int i = idxMain - 1; i > -1; i--) {
		ProcessSectionPropagation(i, 1, refNumLines, maxSize, minN, maxN, leftX, rightX);
	}

	// Propagate numbering downward from main section
	for (int i = idxMain + 1; i < Sections.GetSize(); i++) {
		ProcessSectionPropagation(i, -1, refNumLines, maxSize, minN, maxN, leftX, rightX);
	}
}
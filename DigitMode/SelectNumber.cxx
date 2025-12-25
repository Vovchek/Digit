/// <summary>
/// Attempts to match a fringe position to a numbered fringe in an adjacent section.
/// Used during fringe propagation to maintain consistent numbering across scan lines.
/// </summary>
/// <param name="iSec">Current section index being numbered</param>
/// <param name="Sign">Search direction: -1 for upward (decreasing section index), +1 for downward (increasing section index)</param>
/// <param name="redX">X-coordinate of the fringe center to match</param>
/// <param name="idxS">Output: Index of the section containing the matched fringe</param>
/// <param name="idxL">Output: Index of the matched fringe line within the section</param>
/// <returns>true if a matching fringe was found within tolerance; false otherwise</returns>
/// <remarks>
/// Search algorithm:
/// 1. Iterates through adjacent sections in the specified direction (Sign)
/// 2. For each section, finds the fringe with minimum distance to redX
/// 3. Returns the first match where distance &lt; _SecSegm (tolerance threshold)
/// 4. Early exit on first match (does not search further sections)
/// 
/// Distance calculation:
/// - Computes distance from redX to the center of each fringe segment
/// - Center = segmL.P1.x + wL, where wL = segment width / 2
/// - Skips fringes with zero width (wL == 0)
/// 
/// Tolerance (_SecSegm):
/// - Uses section-specific step if available (Sections[iSec].aveStep)
/// - Falls back to global SecSegm with CorrectionSecSegm factor
/// - Ensures robustness to slight fringe position variations
/// 
/// Performance note:
/// - Nested loop structure: O(n * m) where n = sections, m = fringes per section
/// - Early termination on first match reduces average search time
/// - Called extensively during CreateNumLines() propagation
/// </remarks>
bool CDigitInfo::SelectNumber(int iSec, int Sign, double redX, int& idxS, int& idxL)
{
	double Dist;
	double wL;
	double _SecSegm;

	// Determine tolerance threshold based on section-specific or global step size
	if (Sections[iSec].aveStep == -1)
		_SecSegm = SecSegm * CorrectionSecSegm;
	else
		_SecSegm = CorrectionSecSegm * Sections[iSec].aveStep;

	// Search upward (toward section 0)
	if (Sign < 0) {
		for (int iS = iSec - 1; iS > -1; iS--) {
			double minDist = INT_MAX;
			int miniL = -1;

			// Find closest fringe in current section
			for (int iL = 0; iL < Sections[iS].NumLines.GetSize(); iL++) {
				wL = Sections[iS].NumLines[iL].segmL.GetW() / 2;
				if (wL == 0.)  // Skip degenerate segments
					continue;

				// Distance to fringe center
				Dist = fabs(redX - (Sections[iS].NumLines[iL].segmL.P1.x + wL));
				if (Dist < minDist) {
					minDist = Dist;
					miniL = iL;
				}
			}

			// Return first match within tolerance
			if (miniL != -1 && minDist < _SecSegm) {
				idxS = iS;
				idxL = miniL;
				return true;
			}
		}
	}
	// Search downward (toward last section)
	else {
		for (int iS = iSec + 1; iS < Sections.GetSize(); iS++) {
			double minDist = INT_MAX;
			int miniL = -1;

			// Find closest fringe in current section
			for (int iL = 0; iL < Sections[iS].NumLines.GetSize(); iL++) {
				wL = Sections[iS].NumLines[iL].segmL.GetW() / 2;
				if (wL == 0.)  // Skip degenerate segments
					continue;

				// Distance to fringe center
				Dist = fabs(redX - (Sections[iS].NumLines[iL].segmL.P1.x + wL));
				if (Dist < minDist) {
					minDist = Dist;
					miniL = iL;
				}
			}

			// Return first match within tolerance
			if (miniL != -1 && minDist < _SecSegm) {
				idxS = iS;
				idxL = miniL;
				return true;
			}
		}
	}

	// No match found in any adjacent section
	return false;
}

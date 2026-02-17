#pragma once

#include "DigitizationParams.h"
#include <vector>
#include <functional>

namespace DigitMode::digitization {

// Local constants (avoid AppDef.h dependency)
#define FC_MAX    0
#define FC_MIN    1
#define FC_MINMAX 2

/**
 * @brief Constructs continuous fringes from detected extrema with quality constraints
 * 
 * Phase 3: Fringe Construction Algorithm
 * 
 * Responsibilities:
 * - Connect extrema across scanlines into continuous fringe polylines
 * - Enforce quality constraints:
 *   1. Each fringe contains only one extremum type (Red OR Black)
 *   2. Fringes do not cross each other
 *   3. In FC_MINMAX mode: adjacent fringes must alternate Red/Black
 *      (exception: obstruction between fringes hides intermediate fringes)
 * 
 * Algorithm (based on CreateNumLines):
 * 1. Select main scanline (highest fringe density)
 * 2. Initialize fringes with sequential numbers
 * 3. Propagate upward/downward matching extrema to fringes
 * 4. Assign new numbers for unmatched extrema at edges
 * 
 * Quality improvements over legacy:
 * - Type consistency: validates extremum type within fringe
 * - Non-crossing: rejects matches that would cause crossing
 * - Alternation: enforces Red/Black pattern in MINMAX mode
 */
class FringeConstructor {
public:
    /**
     * @brief Construct fringes from detected extrema
     * @param extrema Detected extremum points (from RedCenterDetector)
     * @param imageWidth Image width
     * @param imageHeight Image height
     * @param isVisible Visibility predicate (x, y) -> bool
     * @param fringeCenterAs FC_MAX, FC_MIN, or FC_MINMAX
     * @param fringeStep Expected spacing between fringes (e.g., 1.0, 0.5)
     * @param toleranceFactor Matching tolerance as fraction of fringeStep (e.g., 0.3)
     * @return Numbered fringes as polylines
     */
    static std::vector<NumberedFringe> ConstructFringes(
        const std::vector<ExtremumPoint>& extrema,
        int imageWidth,
        int imageHeight,
        const std::function<bool(int, int)>& isVisible,
        int fringeCenterAs,
        double fringeStep,
        double toleranceFactor);

private:
    struct NumberedExtremum {
        ExtremumPoint extremum;
        double number{-1000.0};
        bool assigned{false};
    };

    struct ScanlineData {
        int y{0};
        std::vector<NumberedExtremum> extrema;
        double averageStep{-1.0};
    };

    /**
     * @brief Select main scanline with highest fringe density
     */
    static int SelectMainScanline(
        const std::vector<ScanlineData>& scanlines,
        double fringeStep,
        double toleranceFactor);

    /**
     * @brief Match extremum to existing numbered fringe
     * @param extremum Current extremum to match
     * @param currentX X position of current extremum
     * @param adjacentScanline Previously processed scanline
     * @param tolerance Matching distance tolerance
     * @param fringeCenterAs FC_MAX, FC_MIN, or FC_MINMAX
     * @return Index of matched extremum in adjacentScanline, or -1 if no match
     * 
     * Enforces:
     * - Extremum type consistency
     * - Non-crossing constraint
     * - Alternation constraint (if FC_MINMAX)
     */
    static int FindMatchingExtremum(
        const ExtremumPoint& extremum,
        double currentX,
        const ScanlineData& adjacentScanline,
        double tolerance,
        int fringeCenterAs);

    /**
     * @brief Resolve crossing by swapping fringe numbers at crossing section
     * 
     * When two fringes cross, swapping their numbers at the crossing point uncrosses them.
     * This is a simple geometric fix that maintains fringe continuity.
     * 
     * @param currentIdx Index of current extremum causing crossing
     * @param proposedIdx Index of proposed match in adjacent scanline
     * @param scanlines All scanline data (to modify fringe assignments)
     * @param currentY Y coordinate of current scanline
     * @param adjacentY Y coordinate of adjacent scanline
     * @return true if crossing was successfully resolved by swapping
     * 
     * Algorithm:
     * - Find which existing fringe segment would be crossed
     * - Swap the fringe number of current extremum with the crossing fringe
     * - This uncrosses the fringes at this section
     */
    static bool ResolveCrossingBySwap(
        int currentIdx,
        int proposedIdx,
        std::vector<ScanlineData>& scanlines,
        int currentY,
        int adjacentY);


    /**
     * @brief Check if matching would violate non-crossing constraint (FULL CHECK)
     * 
     * Full crossing criterion: New segment must not cross ANY existing fringe segment.
     * This checks against all already-built fringe segments across all scanlines,
     * not just the immediate adjacent scanline.
     * 
     * @param currentIdx Index of current extremum in current scanline
     * @param proposedIdx Index of proposed match in adjacent scanline
     * @param currentExtrema extremums of current scanline
     * @param adjacentExtrema extremuns of adjacent (down) scanline
     * @return true if connecting current→proposed would cross any existing fringe segment
     */
    static bool WouldCross(
        int currentIdx,
        int proposedIdx,
        const std::vector<NumberedExtremum>& currentExtrema,
        const std::vector<NumberedExtremum>& adjacentExtrema);

    /**
     * @brief Check if two line segments intersect
     * @return true if segments (p1→p2) and (p3→p4) intersect (excluding endpoints)
     */
    static bool SegmentsIntersect(
        double x1, double y1, double x2, double y2,
        double x3, double y3, double x4, double y4);

    /**
     * @brief Check if matching would violate alternation constraint
     * 
     * In FC_MINMAX mode: Adjacent fringes must alternate Red/Black.
     * Exception: Obstruction gaps may hide intermediate fringes.
     * 
     * @param currentType Type of current extremum
     * @param proposedNumber Number of proposed match
     * @param adjacentScanline Adjacent scanline data
     * @param fringeCenterAs FC_MAX, FC_MIN, or FC_MINMAX
     * @param fringeStep Expected step between fringes
     * @return true if matching would violate alternation pattern
     */
    static bool WouldViolateAlternation(
        ExtremumType currentType,
        double proposedNumber,
        const ScanlineData& adjacentScanline,
        int fringeCenterAs,
        double fringeStep);

    /**
     * @brief Convert numbered extrema to fringe polylines
     */
    static std::vector<NumberedFringe> ConvertToFringes(
        const std::vector<ScanlineData>& scanlines);
};

#undef FC_MAX
#undef FC_MIN
#undef FC_MINMAX

} // namespace DigitMode::digitization

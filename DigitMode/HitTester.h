#pragma once

#include "SelectionManager.h"
#include "MGTools/Include/Utils/BaseDataType.h"  // For CDPoint
#include <vector>

// Forward declaration (global namespace)
class CFringeSegment;

namespace DigitMode {

/**
 * @brief Performs hit testing to determine what object is under the cursor
 * 
 * Hit Priority: Dot > Edge > Segment > Fringe > Nothing
 * 
 * Uses segment-primary model:
 * - Directly queries segment geometry
 * - No topology reconstruction needed
 * - Returns segment index, not fringe container index
 */
class HitTester {
private:
    static constexpr int HIT_TOLERANCE = 5;  ///< Hit tolerance in pixels (INCLUSIVE: distance <= 5)

public:
    /**
     * @brief Perform hit test at given point
     * @param P Point in image coordinates
     * @param outSegment [out] Segment index if hit
     * @param outDot [out] Dot or edge index if hit
     * @param segments Reference to segment array
     * @return Selection level of hit object (None if no hit)
     * 
     * Priority order:
     * 1. Dots (highest priority)
     * 2. Edges
     * 3. Nothing (SelectionLevel::None)
     * 
     * Tolerance Policy:
     * - Uses INCLUSIVE tolerance (distance <= HIT_TOLERANCE)
     * - Standard CAD convention: "within 5 pixels" includes exactly 5 pixels
     * - Objects at exactly tolerance boundary are selectable
     * 
     * Note: Segments in reverse order (top-to-bottom z-order)
     */
    SelectionLevel HitTest(
        CPoint P,
        int& outSegment,
        int& outDot,
        const std::vector<::CFringeSegment>& segments
    ) const;

private:
    /**
     * @brief Calculate distance from point to dot
     * @param P Point in image coordinates
     * @param dot Dot position
     * @return Euclidean distance in pixels
     */
    double DotDistance(CPoint P, CDPoint dot) const;

    /**
     * @brief Calculate distance from point to line segment
     * @param P Point in image coordinates
     * @param A Start point of segment
     * @param B End point of segment
     * @return Perpendicular distance to segment (or endpoint if outside)
     */
    double DistanceToSegment(CPoint P, CDPoint A, CDPoint B) const;
};

} // namespace DigitMode

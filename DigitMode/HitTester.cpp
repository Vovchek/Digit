#include "stdafx.h"
#include "HitTester.h"
#include "CFringeSegment.h"  // Global namespace, not DigitMode::
#include <cmath>

#undef max  // Windows.h defines max as a macro
#undef min

#include <algorithm>  // For std::max, std::min

namespace DigitMode {

SelectionLevel HitTester::HitTest(
    CPoint P,
    int& outSegment,
    int& outDot,
    const std::vector<CFringeSegment>& segments
) const {
    // Priority: Dot > Edge > Segment > Nothing
    // Iterate segments in reverse (top to bottom z-order)

    for (int iSeg = static_cast<int>(segments.size()) - 1; iSeg >= 0; iSeg--) {
        const CFringeSegment& segment = segments[iSeg];
        int dotCount = segment.GetPointCount();  // Correct method name

        // 1. Check dots (highest priority)
        for (int iD = 0; iD < dotCount; iD++) {
            CDPoint dot = segment.GetPoint(iD);
            if (DotDistance(P, dot) < HIT_TOLERANCE) {
                outSegment = iSeg;
                outDot = iD;
                return SelectionLevel::Dot;
            }
        }

        // 2. Check edges
        for (int iE = 0; iE < dotCount - 1; iE++) {
            CDPoint A = segment.GetPoint(iE);
            CDPoint B = segment.GetPoint(iE + 1);
            double dist = DistanceToSegment(P, A, B);

            if (dist < HIT_TOLERANCE) {
                outSegment = iSeg;
                outDot = iE;  // Edge start index
                return SelectionLevel::Edge;
            }
        }
    }

    return SelectionLevel::None;
}

double HitTester::DotDistance(CPoint P, CDPoint dot) const {
    double dx = P.x - dot.x;
    double dy = P.y - dot.y;
    return std::sqrt(dx * dx + dy * dy);
}

double HitTester::DistanceToSegment(CPoint P, CDPoint A, CDPoint B) const {
    // Vector AB
    double ABx = B.x - A.x;
    double ABy = B.y - A.y;

    // Vector AP
    double APx = P.x - A.x;
    double APy = P.y - A.y;

    // Project AP onto AB
    double dotProduct = APx * ABx + APy * ABy;
    double lenSq = ABx * ABx + ABy * ABy;

    // Parameter t = projection ratio (0 = at A, 1 = at B)
    double t = (lenSq > 0) ? dotProduct / lenSq : 0;
    t = std::max(0.0, std::min(1.0, t));  // Clamp to [0, 1]

    // Closest point on segment
    double closestX = A.x + t * ABx;
    double closestY = A.y + t * ABy;

    // Distance from P to closest point
    double dx = P.x - closestX;
    double dy = P.y - closestY;
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace DigitMode

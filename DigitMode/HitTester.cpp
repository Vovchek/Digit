#include "stdafx.h"
#include "HitTester.h"
#include "CFringeSegment.h"  // Global namespace, not DigitMode::
#include <cmath>

#undef max  // Windows.h defines max as a macro
#undef min

#include <algorithm>  // For std::max, std::min

namespace DigitMode {

SelectionLevel HitTester::HitTest(
    CDPoint P,
    int& outSegment,
    int& outDot,
    const std::vector<CFringeSegment>& segments,
	double tolerance
) const {
    // Priority: Dot > Edge > Segment > Nothing
    // Iterate segments in reverse (top to bottom z-order)
    struct HitResult {
        int segmentIndex;
        int dotOrEdgeIndex;
		double distance;  // Distance for tie-breaking (not used in current priority scheme)
        SelectionLevel level;
    };
	std::vector<HitResult> dotHits, edgeHits;

    for (int iSeg = static_cast<int>(segments.size()) - 1; iSeg >= 0; iSeg--) {
        const CFringeSegment& segment = segments[iSeg];
        int dotCount = segment.GetPointCount();  // Correct method name

        // 1. Check dots (highest priority)
        for (int iD = 0; iD < dotCount; iD++) {
            CDPoint dot = segment.GetPoint(iD);
			auto dist = DotDistance(P, dot);
            if (dist <= tolerance) {  // Inclusive tolerance
				dotHits.push_back({ iSeg, iD, dist, SelectionLevel::Dot });
            }
        }

        // 2. Check edges
        for (int iE = 0; iE < dotCount - 1; iE++) {
            CDPoint A = segment.GetPoint(iE);
            CDPoint B = segment.GetPoint(iE + 1);
            double dist = DistanceToSegment(P, A, B);

            if (dist <= tolerance) {  // Inclusive tolerance
				edgeHits.push_back({ iSeg, iE, dist, SelectionLevel::Edge });
            }
        }
    }
	if (dotHits.empty() && edgeHits.empty()) {
        outSegment = -1;
        outDot = -1;
        return SelectionLevel::None;
    }
    
    // Find the best hit based on priority: Dot > Edge
    auto bestHitIt = std::min_element(dotHits.begin(), dotHits.end(), [](const HitResult& a, const HitResult& b) {
        return a.distance < b.distance;  // Tie-breaker: closer distance
    });

    if (bestHitIt != dotHits.end()) {
        outSegment = bestHitIt->segmentIndex;
        outDot = bestHitIt->dotOrEdgeIndex;
        return bestHitIt->level;
    }

    // If no dot hit, check edge hits
    bestHitIt = std::min_element(edgeHits.begin(), edgeHits.end(), [](const HitResult& a, const HitResult& b) {
        return a.distance < b.distance;  // Tie-breaker: closer distance
        });

    if (bestHitIt != edgeHits.end()) {
        outSegment = bestHitIt->segmentIndex;
        outDot = bestHitIt->dotOrEdgeIndex;
        return bestHitIt->level;
    }

    return SelectionLevel::None;
}

double HitTester::DotDistance(CDPoint P, CDPoint dot) const {
    double dx = P.x - dot.x;
    double dy = P.y - dot.y;
    return std::sqrt(dx * dx + dy * dy);
}

double HitTester::DistanceToSegment(CDPoint P, CDPoint A, CDPoint B) const {
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

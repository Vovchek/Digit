#include "DigitMode/FringeConstructor.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace DigitMode::digitization {

// Local constants (avoid AppDef.h dependency)
#define FC_MAX    0
#define FC_MIN    1
#define FC_MINMAX 2

std::vector<NumberedFringe> FringeConstructor::ConstructFringes(
    std::vector<Section>& scanlines,
    int imageWidth,
    int imageHeight,
    const std::function<bool(int, int)>& isVisible,
    int fringeCenterAs,
    double fringeStep,
    double toleranceFactor)
{
    (void)imageWidth;
    (void)isVisible;

    if (scanlines.empty() || imageHeight <= 0 || fringeStep <= 0.0) {
        return {};
    }

    // Calculate average step for each scanline and pick median value
    std::vector<double> steps;
    for (auto& scanline : scanlines) {
        if (scanline.points.size() < 2) continue;

        double sumStep = 0.0;
            for (size_t i = 1; i < scanline.points.size(); ++i) {
                sumStep += scanline.points[i].position.x -
                          scanline.points[i - 1].position.x;
            }
        scanline.averageStep = sumStep / (scanline.points.size() - 1);
        steps.push_back(scanline.averageStep);
    }
    auto medianStep = steps[steps.size() / 2];

    // Select main scanline
    int mainIdx = SelectMainScanline(scanlines, medianStep, toleranceFactor);
    if (mainIdx < 0 || scanlines[static_cast<size_t>(mainIdx)].points.empty()) {
        return {};
    }

    // Initialize main scanline with sequential numbers
    double currentNumber = 0.0;
    for (auto& ne : scanlines[static_cast<size_t>(mainIdx)].points) {
        ne.number = currentNumber;
        ne.assigned = true;
        currentNumber += fringeStep;
    }

    double tolerance = medianStep * toleranceFactor;

    // Propagate upward from main scanline
    for (int y = mainIdx - 1; y >= 0; --y) {
        auto& currentScanline = scanlines[static_cast<size_t>(y)];
        const auto& adjacentScanline = scanlines[static_cast<size_t>(y + 1)];

        for (size_t curIdx = 0; curIdx < currentScanline.points.size(); ++curIdx) {
            auto& ne = currentScanline.points[curIdx];
            
            int matchIdx = FindMatchingExtremum(
                ne,
                ne.position.x,
                adjacentScanline,
                tolerance,
                fringeCenterAs);

            if (matchIdx >= 0) {
                double proposedNumber = adjacentScanline.points[static_cast<size_t>(matchIdx)].number;
                
                // CONSTRAINT 2: Check non-crossing
                if (WouldCross(
                    static_cast<int>(curIdx),
                    matchIdx,
                    currentScanline.points,
                    adjacentScanline.points)) {
                    // TODO: if(!ResolveCrossingBySwap(...)
                    continue; // Skip this match - would cause crossing
                }

                // CONSTRAINT 3: Check alternation (FC_MINMAX only)
                if (WouldViolateAlternation(
                    ne.extremumType,
                    proposedNumber,
                    adjacentScanline,
                    fringeCenterAs,
                    fringeStep)) {
                    continue; // Skip this match - would violate alternation
                }

                // All constraints passed - assign number
                ne.number = proposedNumber;
                ne.assigned = true;
            }
        }

        // Assign new numbers to unmatched extrema at edges
        // Find min/max assigned numbers
        double minNum = std::numeric_limits<double>::max();
        double maxNum = std::numeric_limits<double>::lowest();
        for (const auto& ne : currentScanline.points) {
            if (ne.assigned) {
                minNum = std::min(minNum, ne.number);
                maxNum = std::max(maxNum, ne.number);
            }
        }

        // Assign numbers to unmatched extrema
        for (auto& ne : currentScanline.points) {
            if (!ne.assigned) {
                // Check if it's at left or right edge
                bool isLeftmost = true;
                for (const auto& other : currentScanline.points) {
                    if (other.assigned && other.position.x < ne.position.x) {
                        isLeftmost = false;
                        break;
                    }
                }

                if (isLeftmost) {
                    ne.number = minNum - fringeStep;
                    minNum = ne.number;
                } else {
                    ne.number = maxNum + fringeStep;
                    maxNum = ne.number;
                }
                ne.assigned = true;
            }
        }
    }

    // Propagate downward from main scanline
    for (int y = mainIdx + 1; y < imageHeight; ++y) {
        auto& currentScanline = scanlines[static_cast<size_t>(y)];
        const auto& adjacentScanline = scanlines[static_cast<size_t>(y - 1)];

        for (size_t curIdx = 0; curIdx < currentScanline.points.size(); ++curIdx) {
            auto& ne = currentScanline.points[curIdx];
            
            int matchIdx = FindMatchingExtremum(
                ne,
                ne.position.x,
                adjacentScanline,
                tolerance,
                fringeCenterAs);

            if (matchIdx >= 0) {
                double proposedNumber = adjacentScanline.points[static_cast<size_t>(matchIdx)].number;
                
                // CONSTRAINT 2: Check non-crossing
                if (WouldCross(
                    static_cast<int>(curIdx),
                    matchIdx,
                    currentScanline.points,
                    adjacentScanline.points)) {
                    // TODO: if(!ResolveCrossingBySwap(...)
                    continue; // Skip this match - would cause crossing
                }

                // CONSTRAINT 3: Check alternation (FC_MINMAX only)
                if (WouldViolateAlternation(
                    ne.extremumType,
                    proposedNumber,
                    adjacentScanline,
                    fringeCenterAs,
                    fringeStep)) {
                    continue; // Skip this match - would violate alternation
                }

                // All constraints passed - assign number
                ne.number = proposedNumber;
                ne.assigned = true;
            }
        }

        // Assign new numbers to unmatched extrema at edges
        double minNum = std::numeric_limits<double>::max();
        double maxNum = std::numeric_limits<double>::lowest();
        for (const auto& ne : currentScanline.points) {
            if (ne.assigned) {
                minNum = (std::min)(minNum, ne.number);
                maxNum = (std::max)(maxNum, ne.number);
            }
        }

        for (auto& ne : currentScanline.points) {
            if (!ne.assigned) {
                bool isLeftmost = true;
                for (const auto& other : currentScanline.points) {
                    if (other.assigned && other.position.x < ne.position.x) {
                        isLeftmost = false;
                        break;
                    }
                }

                if (isLeftmost) {
                    ne.number = minNum - fringeStep;
                    minNum = ne.number;
                } else {
                    ne.number = maxNum + fringeStep;
                    maxNum = ne.number;
                }
                ne.assigned = true;
            }
        }
    }

    // Convert to fringes
    return ConvertToFringes(scanlines);
}

int FringeConstructor::SelectMainScanline(
    const std::vector<Section>& scanlines,
    double fringeStep,
    double toleranceFactor)
{
    double minStep = fringeStep * (1.0 - toleranceFactor);
    double maxStep = fringeStep * (1.0 + toleranceFactor);

    int maxFringeCount1 = -1;
    int maxFringeCount2 = -1;
    int idx1 = -1;
    int idx2 = -1;

    // Forward pass - find first scanline with max fringe count
    for (size_t i = 0; i < scanlines.size(); ++i) {
        const auto& sl = scanlines[i];
        int count = static_cast<int>(sl.points.size());
        double step = sl.averageStep;

        if (count > maxFringeCount1 && 
            (step < 0 || (step >= minStep && step <= maxStep))) {
            maxFringeCount1 = count;
            idx1 = static_cast<int>(i);
        }
    }

    // Backward pass - find last scanline with max fringe count
    for (int i = static_cast<int>(scanlines.size()) - 1; i >= 0; --i) {
        const auto& sl = scanlines[static_cast<size_t>(i)];
        int count = static_cast<int>(sl.points.size());
        double step = sl.averageStep;

        if (count >= maxFringeCount2 && 
            (step < 0 || (step >= minStep && step <= maxStep)) &&
            count == maxFringeCount1) {
            maxFringeCount2 = count;
            idx2 = i;
        }
    }

    // Return middle if both found and equal
    if (idx1 >= 0 && idx2 >= 0 && idx1 != idx2 && maxFringeCount1 == maxFringeCount2) {
        return idx1 + (idx2 - idx1) / 2;
    }

    return idx1 >= 0 ? idx1 : idx2;
}

int FringeConstructor::FindMatchingExtremum(
    const ExtremumPoint& extremum,
    double currentX,
    const Section& adjacentScanline,
    double tolerance,
    int fringeCenterAs)
{
    // NOTE: This function needs fringeStep for alternation check
    // For now, we'll defer alternation check to the caller
    // TODO: Refactor to pass fringeStep or check in propagation loop
    
    int bestMatch = -1;
    double bestDistance = std::numeric_limits<double>::max();

    for (size_t i = 0; i < adjacentScanline.points.size(); ++i) {
        const auto& adjExt = adjacentScanline.points[i];
        if (!adjExt.assigned) continue;

        double adjX = adjExt.position.x;
        double distance = std::abs(currentX - adjX);

        // Check if within tolerance
        if (distance > tolerance) continue;

        // CONSTRAINT 1: Type consistency
        if (extremum.extremumType != adjExt.extremumType) {
            continue;
        }

        // CONSTRAINT 3: Alternation - deferred to caller
        // (needs fringeStep which isn't passed here)

        // CONSTRAINT 2: Non-crossing - deferred to caller
        // (needs current scanline context)

        // Take closest match
        if (distance < bestDistance) {
            bestDistance = distance;
            bestMatch = static_cast<int>(i);
        }
    }

    return bestMatch;
}

bool FringeConstructor::WouldCross(
    int currentIdx,
    int proposedIdx,
    const std::vector<ExtremumPoint>& currentExtrema,
    const std::vector<ExtremumPoint>& adjacentExtrema)
{
    if (currentIdx < 0 || currentIdx >= static_cast<int>(currentExtrema.size())) return false;
    if (proposedIdx < 0 || proposedIdx >= static_cast<int>(adjacentExtrema.size())) return false;

    const auto& current = currentExtrema[static_cast<size_t>(currentIdx)];
    const auto& proposed = adjacentExtrema[static_cast<size_t>(proposedIdx)];

    double x1 = current.position.x;
    double y1 = current.position.y;
    double x2 = proposed.position.x;
    double y2 = proposed.position.y;

    // Check against all other matched pairs in these scanlines
    for (size_t i = 0; i < currentExtrema.size(); ++i) {
        if (static_cast<int>(i) == currentIdx) continue;
        if (!currentExtrema[i].assigned) continue;

        // Find matching extremum in adjacent scanline with same number
        double targetNumber = currentExtrema[i].number;
        for (size_t j = 0; j < adjacentExtrema.size(); ++j) {
            if (static_cast<int>(j) == proposedIdx) continue;
            if (!adjacentExtrema[j].assigned) continue;
            if (adjacentExtrema[j].number != targetNumber) continue;

            // Found a matched pair - check if segments would cross
            double x3 = currentExtrema[i].position.x;
            double y3 = currentExtrema[i].position.y;
            double x4 = adjacentExtrema[j].position.x;
            double y4 = adjacentExtrema[j].position.y;

            if (SegmentsIntersect(x1, y1, x2, y2, x3, y3, x4, y4)) {
                return true; // Would cross existing fringe segment
            }
        }
    }

    return false; // No crossing detected
}

bool FringeConstructor::SegmentsIntersect(
    double x1, double y1, double x2, double y2,
    double x3, double y3, double x4, double y4)
{
    // Check if line segment (p1→p2) intersects line segment (p3→p4)
    // Using parametric form and cross products
    
    auto crossProduct = [](double ax, double ay, double bx, double by) -> double {
        return ax * by - ay * bx;
    };

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;
    double dx3 = x3 - x1;
    double dy3 = y3 - y1;

    double cross1 = crossProduct(dx1, dy1, dx2, dy2);
    
    // Parallel or collinear segments (cross product ≈ 0)
    if (std::abs(cross1) < 1e-10) {
        return false; // Treat parallel/collinear as non-crossing
    }

    double t1 = crossProduct(dx3, dy3, dx2, dy2) / cross1;
    double t2 = crossProduct(dx3, dy3, dx1, dy1) / cross1;

    // Segments intersect if both parameters are in [0, 1]
    // Use strict inequality to allow endpoint touching
    return (t1 > 0.0 && t1 < 1.0) && (t2 > 0.0 && t2 < 1.0);
}

bool FringeConstructor::WouldViolateAlternation(
    ExtremumType currentType,
    double proposedNumber,
    const Section& adjacentScanline,
    int fringeCenterAs,
    double fringeStep)
{
    if (fringeCenterAs != FC_MINMAX) {
        return false; // Alternation only applies in MINMAX mode
    }

    // In MINMAX mode, adjacent fringes must alternate Red/Black
    // For proposed number N, check neighbors N-step and N+step
    // They should have opposite type to currentType
    
    // Check both neighboring fringe numbers
    double prevNumber = proposedNumber - fringeStep;
    double nextNumber = proposedNumber + fringeStep;
    
    bool foundPrevSameType = false;
    bool foundNextSameType = false;
    
    for (const auto& ne : adjacentScanline.points) {
        if (!ne.assigned) continue;
        
        // Check if this is the previous fringe (N - step)
        if (std::abs(ne.number - prevNumber) < fringeStep * 0.1) {
            if (ne.extremumType == currentType) {
                foundPrevSameType = true;
            }
        }
        
        // Check if this is the next fringe (N + step)
        if (std::abs(ne.number - nextNumber) < fringeStep * 0.1) {
            if (ne.extremumType == currentType) {
                foundNextSameType = true;
            }
        }
    }

    // Violation if any adjacent fringe has same type
    // TODO: Add obstruction gap exception
    return foundPrevSameType || foundNextSameType;
}

std::vector<NumberedFringe> FringeConstructor::ConvertToFringes(
    const std::vector<Section>& scanlines)
{
    // Group extrema by number
    std::map<double, std::vector<Point2d>> fringeMap;

    for (const auto& scanline : scanlines) {
        for (const auto& ne : scanline.points) {
            if (ne.assigned) {
                fringeMap[ne.number].push_back(ne.position);
            }
        }
    }

    // Convert map to vector of NumberedFringe
    std::vector<NumberedFringe> result;
    for (const auto& pair : fringeMap) {
        NumberedFringe fringe;
        fringe.number = pair.first;
        fringe.points = pair.second;
        fringe.segmentIndex = static_cast<int>(result.size());
        result.push_back(fringe);
    }

    return result;
}

#undef FC_MAX
#undef FC_MIN
#undef FC_MINMAX

} // namespace DigitMode::digitization

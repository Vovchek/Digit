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
    const std::vector<ExtremumPoint>& extrema,
    int imageWidth,
    int imageHeight,
    const std::function<bool(int, int)>& isVisible,
    int fringeCenterAs,
    double fringeStep,
    double toleranceFactor)
{
    (void)imageWidth;
    (void)isVisible;

    if (extrema.empty() || imageHeight <= 0 || fringeStep <= 0.0) {
        return {};
    }

    // Group extrema by scanline
    std::vector<ScanlineData> scanlines(static_cast<size_t>(imageHeight));
    for (int y = 0; y < imageHeight; ++y) {
        scanlines[static_cast<size_t>(y)].y = y;
    }

    for (const auto& ext : extrema) {
        int y = static_cast<int>(ext.position.y + 0.5);
        if (y >= 0 && y < imageHeight) {
            NumberedExtremum ne;
            ne.extremum = ext;
            ne.assigned = false;
            ne.number = -1000.0;
            scanlines[static_cast<size_t>(y)].extrema.push_back(ne);
        }
    }

    // Calculate average step for each scanline and sort by x
    for (auto& scanline : scanlines) {
        if (scanline.extrema.empty()) continue;

        // Sort by x
        std::sort(scanline.extrema.begin(), scanline.extrema.end(),
            [](const auto& a, const auto& b) {
                return a.extremum.position.x < b.extremum.position.x;
            });

        if (scanline.extrema.size() > 1) {
            double sumStep = 0.0;
            for (size_t i = 1; i < scanline.extrema.size(); ++i) {
                sumStep += scanline.extrema[i].extremum.position.x -
                          scanline.extrema[i - 1].extremum.position.x;
            }
            scanline.averageStep = sumStep / (scanline.extrema.size() - 1);
        }
    }

    // Select main scanline
    int mainIdx = SelectMainScanline(scanlines, fringeStep, toleranceFactor);
    if (mainIdx < 0 || scanlines[static_cast<size_t>(mainIdx)].extrema.empty()) {
        return {};
    }

    // Initialize main scanline with sequential numbers
    double currentNumber = 0.0;
    for (auto& ne : scanlines[static_cast<size_t>(mainIdx)].extrema) {
        ne.number = currentNumber;
        ne.assigned = true;
        currentNumber += fringeStep;
    }

    double tolerance = fringeStep * toleranceFactor;

    // Propagate upward from main scanline
    for (int y = mainIdx - 1; y >= 0; --y) {
        auto& currentScanline = scanlines[static_cast<size_t>(y)];
        const auto& adjacentScanline = scanlines[static_cast<size_t>(y + 1)];

        for (auto& ne : currentScanline.extrema) {
            int matchIdx = FindMatchingExtremum(
                ne.extremum,
                ne.extremum.position.x,
                adjacentScanline,
                tolerance,
                fringeCenterAs);

            if (matchIdx >= 0) {
                // Matched to existing numbered extremum
                ne.number = adjacentScanline.extrema[static_cast<size_t>(matchIdx)].number;
                ne.assigned = true;
            }
        }

        // Assign new numbers to unmatched extrema at edges
        // Find min/max assigned numbers
        double minNum = std::numeric_limits<double>::max();
        double maxNum = std::numeric_limits<double>::lowest();
        for (const auto& ne : currentScanline.extrema) {
            if (ne.assigned) {
                minNum = std::min(minNum, ne.number);
                maxNum = std::max(maxNum, ne.number);
            }
        }

        // Assign numbers to unmatched extrema
        for (auto& ne : currentScanline.extrema) {
            if (!ne.assigned) {
                // Check if it's at left or right edge
                bool isLeftmost = true;
                for (const auto& other : currentScanline.extrema) {
                    if (other.assigned && other.extremum.position.x < ne.extremum.position.x) {
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

        for (auto& ne : currentScanline.extrema) {
            int matchIdx = FindMatchingExtremum(
                ne.extremum,
                ne.extremum.position.x,
                adjacentScanline,
                tolerance,
                fringeCenterAs);

            if (matchIdx >= 0) {
                // Matched to existing numbered extremum
                ne.number = adjacentScanline.extrema[static_cast<size_t>(matchIdx)].number;
                ne.assigned = true;
            }
        }

        // Assign new numbers to unmatched extrema at edges
        double minNum = std::numeric_limits<double>::max();
        double maxNum = std::numeric_limits<double>::lowest();
        for (const auto& ne : currentScanline.extrema) {
            if (ne.assigned) {
                minNum = std::min(minNum, ne.number);
                maxNum = std::max(maxNum, ne.number);
            }
        }

        for (auto& ne : currentScanline.extrema) {
            if (!ne.assigned) {
                bool isLeftmost = true;
                for (const auto& other : currentScanline.extrema) {
                    if (other.assigned && other.extremum.position.x < ne.extremum.position.x) {
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
    const std::vector<ScanlineData>& scanlines,
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
        int count = static_cast<int>(sl.extrema.size());
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
        int count = static_cast<int>(sl.extrema.size());
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
    const ScanlineData& adjacentScanline,
    double tolerance,
    int fringeCenterAs)
{
    int bestMatch = -1;
    double bestDistance = std::numeric_limits<double>::max();

    for (size_t i = 0; i < adjacentScanline.extrema.size(); ++i) {
        const auto& adjExt = adjacentScanline.extrema[i];
        if (!adjExt.assigned) continue;

        double adjX = adjExt.extremum.position.x;
        double distance = std::abs(currentX - adjX);

        // Check if within tolerance
        if (distance > tolerance) continue;

        // Check type consistency
        if (extremum.extremumType != adjExt.extremum.extremumType) {
            continue;
        }

        // Check alternation constraint (TODO: implement)
        if (fringeCenterAs == FC_MINMAX) {
            if (WouldViolateAlternation(
                extremum.extremumType,
                adjExt.number,
                adjacentScanline,
                fringeCenterAs)) {
                continue;
            }
        }

        // Check crossing constraint (TODO: implement)
        // For now, skip crossing check

        // Take closest match
        if (distance < bestDistance) {
            bestDistance = distance;
            bestMatch = static_cast<int>(i);
        }
    }

    return bestMatch;
}

bool FringeConstructor::WouldCross(
    double currentX,
    int proposedExtremumIndex,
    const std::vector<NumberedExtremum>& currentExtrema,
    const std::vector<NumberedExtremum>& adjacentExtrema)
{
    (void)currentX;
    (void)proposedExtremumIndex;
    (void)currentExtrema;
    (void)adjacentExtrema;
    // TODO: Implement crossing detection
    // Check if connecting current extremum to proposed extremum would cross any existing fringe
    return false;
}

bool FringeConstructor::WouldViolateAlternation(
    ExtremumType currentType,
    double proposedNumber,
    const ScanlineData& adjacentScanline,
    int fringeCenterAs)
{
    if (fringeCenterAs != FC_MINMAX) {
        return false; // Alternation only applies in MINMAX mode
    }

    // Find adjacent numbered extrema (numbers ± step from proposedNumber)
    // They should have opposite types
    (void)currentType;
    (void)proposedNumber;
    (void)adjacentScanline;

    // TODO: Implement full alternation check
    return false;
}

std::vector<NumberedFringe> FringeConstructor::ConvertToFringes(
    const std::vector<ScanlineData>& scanlines)
{
    // Group extrema by number
    std::map<double, std::vector<Point2d>> fringeMap;

    for (const auto& scanline : scanlines) {
        for (const auto& ne : scanline.extrema) {
            if (ne.assigned) {
                fringeMap[ne.number].push_back(ne.extremum.position);
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

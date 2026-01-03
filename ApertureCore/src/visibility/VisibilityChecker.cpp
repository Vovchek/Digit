/**
 * @file VisibilityChecker.cpp
 * @brief Implementation of VisibilityChecker class
 */

#include "aperturecore/visibility/VisibilityChecker.h"

namespace aperture {

VisibilityChecker::VisibilityChecker(const ShapeCollection& shapes)
    : shapes_(shapes)
    , stats_{}
{
}

bool VisibilityChecker::isVisible(const Point& point) const {
    stats_.totalChecks++;
    
    // Step 1: Initial state
    // visible = true if any EXTERNAL exists, false otherwise
    bool visible = shapes_.hasAnyExternal();
    
    // Step 2: Check EXTERNAL shapes (apertures)
    // Point must be INSIDE ALL EXTERNAL shapes (intersection required)
    const auto& externals = shapes_.getExternal();
    for (const auto& shape : externals) {
        stats_.externalChecks++;
        if (!shape->isInside(point)) {
            // Outside any EXTERNAL ? blocked
            stats_.earlyExits++;
            return false;
        }
    }
    
    // Step 3: Check APERTURE shapes (openings)
    // Point inside ANY APERTURE ? force visible (union)
    const auto& apertures = shapes_.getApertures();
    for (const auto& shape : apertures) {
        stats_.apertureChecks++;
        if (shape->isInside(point)) {
            // Inside APERTURE ? force visible
            visible = true;
            break;  // Early exit optimization
        }
    }
    
    // Step 4: Check INTERNAL shapes (obstructions)
    // Point inside ANY INTERNAL ? blocked (final veto)
    const auto& internals = shapes_.getInternal();
    for (const auto& shape : internals) {
        stats_.internalChecks++;
        if (shape->isInside(point)) {
            // Inside INTERNAL ? blocked
            stats_.earlyExits++;
            return false;
        }
    }
    
    return visible;
}

std::vector<bool> VisibilityChecker::checkPoints(const std::vector<Point>& points) const {
    std::vector<bool> results;
    results.reserve(points.size());
    
    for (const auto& point : points) {
        results.push_back(isVisible(point));
    }
    
    return results;
}

void VisibilityChecker::resetStats() {
    stats_ = Stats{};
}

Bounds VisibilityChecker::getVisibleRegion() const {
    return shapes_.getVisibleRegion();
}

} // namespace aperture

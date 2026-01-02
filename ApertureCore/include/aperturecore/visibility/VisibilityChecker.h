/**
 * @file VisibilityChecker.h
 * @brief Point visibility testing with APERTURE support
 */
#pragma once

#include "ShapeCollection.h"
#include "../geometry/Point.h"
#include <vector>

namespace aperture {

/**
 * @brief Checks point visibility based on shape collection
 * 
 * Implements the new 3-type visibility algorithm:
 * 1. Initial state: visible if any EXTERNAL exists
 * 2. EXTERNAL check: must be inside ALL (intersection)
 * 3. APERTURE check: can be inside ANY (union, forces visible)
 * 4. INTERNAL check: must be outside ALL (final veto)
 */
class VisibilityChecker {
public:
    /**
     * @brief Construct checker with shape collection
     * @param shapes Collection of shapes to check against
     */
    explicit VisibilityChecker(const ShapeCollection& shapes);
    
    /**
     * @brief Check if a point is visible
     * @param point Point to test
     * @return true if point is visible according to visibility rules
     */
    bool isVisible(const Point& point) const;
    
    /**
     * @brief Batch check multiple points
     * @param points Vector of points to test
     * @return Vector of boolean results (same order as input)
     */
    std::vector<bool> checkPoints(const std::vector<Point>& points) const;
    
    /**
     * @brief Performance statistics
     */
    struct Stats {
        size_t totalChecks{0};      ///< Total isVisible() calls
        size_t externalChecks{0};   ///< EXTERNAL shape tests
        size_t apertureChecks{0};   ///< APERTURE shape tests
        size_t internalChecks{0};   ///< INTERNAL shape tests
        size_t earlyExits{0};       ///< Early exits (optimization)
    };
    
    /**
     * @brief Get performance statistics
     * @return Current statistics
     */
    Stats getStats() const { return stats_; }
    
    /**
     * @brief Reset statistics counters
     */
    void resetStats();

private:
    const ShapeCollection& shapes_;  ///< Reference to shape collection
    mutable Stats stats_;            ///< Performance counters (mutable for const methods)
};

} // namespace aperture

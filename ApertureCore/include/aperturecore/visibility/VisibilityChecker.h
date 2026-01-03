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
    
    /**
     * @brief Get visible region bounds (conservative ROI)
     * @return Conservative bounds where visible points MAY exist
     * 
     * Returns a conservative Region of Interest for visibility checking.
     * Points outside this region are **guaranteed invisible** (safe to skip).
     * Points inside this region **might be visible** (must check with isVisible()).
     * 
     * The ROI is conservative due to INTERNAL obstructions - it returns the
     * EXTERNAL intersection bounds, but INTERNAL shapes within that region
     * are not accounted for in the bounds calculation.
     * 
     * **Optimization strategy:**
     * 1. Get ROI to find maximum possible visible area
     * 2. Skip all pixels outside ROI (major speedup - often 80-90% of pixels)
     * 3. Check remaining pixels with isVisible() (handles INTERNAL obstructions)
     * 
     * Use cases:
     * - Image processing optimization (skip pixels outside ROI)
     * - Coordinate normalization (scale relative to EXTERNAL bounds)
     * - Progress estimation (worst-case pixel count = ROI area)
     * - Memory allocation (allocate for maximum visible area)
     * 
     * @code{.cpp}
     * VisibilityChecker checker(shapes);
     * Bounds roi = checker.getVisibleRegion();
     * 
     * // Two-stage checking (optimal performance)
     * for (int y = roi.top; y <= roi.bottom; ++y) {
     *     for (int x = roi.left; x <= roi.right; ++x) {
     *         // Stage 1: ROI culls ~90% of pixels (O(1) check)
     *         // Stage 2: isVisible() checks remaining pixels
     *         if (checker.isVisible({x, y})) {
     *             // Process visible pixel
     *         }
     *     }
     * }
     * 
     * // Normalization uses EXTERNAL bounds
     * Point p{100, 50};
     * Point normalized = {
     *     (p.x - roi.left) / roi.width(),
     *     (p.y - roi.top) / roi.height()
     * };
     * @endcode
     * 
     * @note Points outside ROI are guaranteed invisible
     * @note Points inside ROI require isVisible() check (INTERNAL obstructions)
     * @see ShapeCollection::getVisibleRegion()
     */
    Bounds getVisibleRegion() const;

private:
    const ShapeCollection& shapes_;  ///< Reference to shape collection
    mutable Stats stats_;            ///< Performance counters (mutable for const methods)
};

} // namespace aperture

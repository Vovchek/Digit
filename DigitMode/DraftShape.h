/**
 * @file DraftShape.h
 * @brief Draft shape accumulator for interactive shape creation
 * 
 * ## Overview
 * 
 * DraftShape accumulates user input points during shape creation workflow.
 * It converts perimeter samples into committed shapes on finalization.
 * 
 * ## Key Principles
 * 
 * 1. **Perimeter-Based Creation** (UX Spec §2)
 *    - No "center-click" workflows
 *    - All shapes defined by perimeter points
 *    - Center/axes derived, not directly input
 * 
 * 2. **Modal Tool Selection** (UX Spec §1.1)
 *    - Tool determines shape type (Circle vs Ellipse)
 *    - Never auto-infer type from fit residuals
 * 
 * 3. **Preview Feedback**
 *    - GetPreview() returns intermediate shape
 *    - Updates on each AddPoint() call
 *    - Enables live visual feedback during creation
 * 
 * ## Workflow
 * 
 * ```
 * DraftShape draft{DraftShape::Kind::Rectangle, TypeLimits::EXTERNAL};
 * 
 * // User clicks points
 * draft.AddPoint({10, 20});  // Point 1
 * draft.AddPoint({110, 20}); // Point 2
 * if (!draft.CanCommit()) {
 *     // Show preview for feedback
 *     auto preview = draft.GetPreview();
 *     // ...render preview...
 * }
 * 
 * draft.AddPoint({110, 70}); // Point 3
 * if (draft.CanCommit()) {
 *     // Finalize and create committed shape
 *     auto shape = draft.ToShape();
 *     // ...create AddShapeCommand(shape)...
 *     draft.Clear();
 * }
 * ```
 * 
 * @see BoundsHandler - Uses DraftShape for shape creation modes
 * @see UX Specification §2 - Shape creation workflows
 */
#pragma once

#include "ApertureCore/include/aperturecore/geometry/Point.h"
#include "ApertureCore/include/aperturecore/geometry/Shape.h"
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"
#include <vector>
#include <memory>

namespace DigitMode {

/**
 * @brief Draft shape accumulator for interactive creation
 * 
 * Collects user clicks as perimeter points and converts to committed Shape.
 * 
 * ## Shape Creation Rules (UX Spec §2)
 * 
 * - **Rectangle**: 3 points required (p0, p1 = width, p2 = height)
 * - **Ellipse**: ≥3 points (LSM fit, allows unequal radii)
 * - **Circle**: ≥3 points (LSM fit with equal-radii constraint)
 * - **Polygon**: ≥3 points (vertices, no self-intersection)
 * 
 * ## Preview Behavior
 * 
 * - Rectangle: Shows partial rectangle after 2 points (axis-aligned guess)
 * - Ellipse/Circle: Shows fitted shape after ≥3 points
 * - Polygon: Shows open polyline until commit
 * 
 * @note Draft is transient - never stored in document
 * @note Only committed shapes (via ToShape) enter ShapeCollection
 */
struct DraftShape {
    /**
     * @brief Shape type being created
     * 
     * Determined by active tool mode, never inferred from data.
     */
    enum class Kind {
        Rectangle,  ///< 3-point rectangle (UX Spec §2.1)
        Ellipse,    ///< Perimeter-fitted ellipse (UX Spec §2.2)
        Circle,     ///< Perimeter-fitted circle (equal radii) (UX Spec §2.3)
        Polygon     ///< Vertex-based polygon (UX Spec §2.4)
    };
    
    Kind kind = Kind::Rectangle;                   ///< Shape type from tool mode
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL; ///< Visibility type
    std::vector<aperture::Point> perimeterPoints;  ///< Accumulated clicks
    
    /**
     * @brief Check if draft has minimum points for commit
     * @return true if ToShape() will succeed
     * 
     * Minimum points required:
     * - Rectangle: 3 points
     * - Ellipse: 3 points (minimum for LSM)
     * - Circle: 3 points (minimum for LSM)
     * - Polygon: 3 points (minimum for closed polygon)
     */
    bool CanCommit() const;
    
    /**
     * @brief Convert draft to committed shape
     * @return Unique pointer to created shape, or nullptr if invalid
     * 
     * Creates shape using appropriate constructor/factory:
     * - Rectangle: Uses 3-point constructor
     * - Ellipse: Uses Ellipse::FitEllipse()
     * - Circle: Uses Ellipse::FitCircle()
     * - Polygon: Uses Polygon(vertices) constructor
     * 
     * ## Validation
     * 
     * Returns nullptr if:
     * - Insufficient points (< required minimum)
     * - Polygon has self-intersection
     * - Points are collinear (degenerate)
     * 
     * @note Caller should check CanCommit() before calling
     * @note Type and coordinate system preserved from draft
     */
    std::unique_ptr<aperture::Shape> ToShape() const;
    
    /**
     * @brief Get preview shape for rendering
     * @return Preview shape, or nullptr if insufficient points
     * 
     * Returns intermediate shape for visual feedback:
     * - Rectangle (2 pts): Axis-aligned rectangle from p0 to p1
     * - Rectangle (3 pts): Final oriented rectangle
     * - Ellipse/Circle (1-2 pts): Null (need ≥3 for fit)
     * - Ellipse/Circle (≥3 pts): Current LSM fit
     * - Polygon (≥2 pts): Open polyline (not closed)
     * 
     * @note Preview updates on each AddPoint() call
     * @note Not the same as final committed shape
     */
    std::unique_ptr<aperture::Shape> GetPreview();
    
    /**
     * @brief Add a point to the draft
     * @param pt Point in world coordinates
     * 
     * Appends point to perimeterPoints vector.
     * Caller should call GetPreview() after to update display.
     */
    void AddPoint(const aperture::Point& pt);
    
    /**
     * @brief Clear all points
     * 
     * Resets draft to empty state (preserves kind and type).
     */
    void Clear();
    
    /**
     * @brief Create draft from bounding box (Phase B - drag creation)
     * @param topLeft Top-left corner of box (world coordinates)
     * @param bottomRight Bottom-right corner of box (world coordinates)
     * 
     * Generates appropriate perimeter points from bounding box:
     * - Rectangle: 3 corners (TL, TR, BR) for 3-point constructor
     * - Ellipse: 8 points around ellipse perimeter
     * - Circle: 8 points around circle perimeter (min radius)
     * - Polygon: Not supported (use AddPoint instead)
     * 
     * Used for click-drag-release shape creation workflow.
     * Clears any existing perimeter points.
     */
    void CreateFromBoundingBox(
        const aperture::Point& topLeft,
        const aperture::Point& bottomRight
    );
    
    /**
     * @brief Get number of points in draft
     */
    size_t PointCount() const { return perimeterPoints.size(); }
};

} // namespace DigitMode

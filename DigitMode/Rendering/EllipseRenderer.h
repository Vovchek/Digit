/**
 * @file EllipseRenderer.h
 * @brief Concrete renderer for Ellipse and Circle shapes (Phase 3)
 */
#pragma once

#include "IShapeRenderer.h"

// Forward declarations
namespace aperture {
    class Ellipse;
    struct Point;
}

// Forward declaration for helper
struct CPoint2d;

namespace DigitMode {

/**
 * @brief Renderer for aperture::Ellipse shapes (including circles)
 * 
 * Draws ellipses and circles with optional rotation.
 * Handles include: move, rotate, major axis endpoints, minor axis endpoints.
 * 
 * ## Circle vs Ellipse
 * - Checks `ellipse.isCircle()` to determine rendering approach
 * - Circles: Simple arc drawing (no rotation needed)
 * - Ellipses: May require rotation transformation or polygon approximation
 */
class EllipseRenderer : public IShapeRenderer {
public:
    /**
     * @brief Draw ellipse/circle outline and fill
     * 
     * For circles: Uses CDC::Ellipse() with bounding box
     * For rotated ellipses: Uses polygon approximation or rotation transformation
     */
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;
    
    /**
     * @brief Draw interactive handles for ellipse manipulation
     * 
     * Handles typically include:
     * - Move handle (center)
     * - Rotate handle (rotation anchor)
     * - Major axis endpoints (2 handles)
     * - Minor axis endpoints (2 handles)
     */
    void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;

private:
    /**
     * @brief Draw axis-aligned ellipse (no rotation)
     */
    void DrawAxisAligned(
        const aperture::Ellipse& ellipse,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const;
    
    /**
     * @brief Draw rotated ellipse (polygon approximation)
     */
    void DrawRotated(
        const aperture::Ellipse& ellipse,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const;
};

} // namespace DigitMode

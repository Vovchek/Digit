/**
 * @file RectangleRenderer.h
 * @brief Concrete renderer for Rectangle shapes (Phase 3)
 */
#pragma once

#include "IShapeRenderer.h"

namespace DigitMode {

/**
 * @brief Renderer for aperture::Rectangle shapes
 * 
 * Draws rectangles as 4-point polygons with optional fill and rotation.
 * Handles include: move, rotate, 4 corners, 4 edge midpoints.
 */
class RectangleRenderer : public IShapeRenderer {
public:
    /**
     * @brief Draw rectangle outline and fill
     * 
     * Extracts 4 corners from Rectangle, transforms to screen space,
     * draws as polygon with style-specified pen and brush.
     */
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;
    
    /**
     * @brief Draw interactive handles for rectangle manipulation
     * 
     * Enumerates all handles (move, rotate, corners, edges),
     * renders as small squares with active handle highlighted.
     */
    void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;
};

} // namespace DigitMode

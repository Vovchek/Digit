/**
 * @file PolygonRenderer.h
 * @brief Concrete renderer for Polygon shapes (Phase 3)
 */
#pragma once

#include "IShapeRenderer.h"

namespace DigitMode {

/**
 * @brief Renderer for aperture::Polygon shapes
 * 
 * Draws polygons with arbitrary vertex counts (convex or concave).
 * Handles are placed at each vertex for editing.
 */
class PolygonRenderer : public IShapeRenderer {
public:
    /**
     * @brief Draw polygon outline and fill
     * 
     * Extracts vertices, transforms to screen space,
     * draws as closed polyline/polygon.
     */
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;
    
    /**
     * @brief Draw interactive handles for polygon manipulation
     * 
     * Draws handle at each vertex position.
     * Vertices can be dragged to reshape polygon.
     */
    void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const override;
};

} // namespace DigitMode

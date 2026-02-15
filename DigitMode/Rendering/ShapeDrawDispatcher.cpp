/**
 * @file ShapeDrawDispatcher.cpp
 * @brief Implementation of ShapeDrawDispatcher
 */

#include "stdafx.h"
#include "ShapeDrawDispatcher.h"
#include "ShapeDrawStyle.h"
#include "RectangleRenderer.h"
#include "EllipseRenderer.h"
#include "PolygonRenderer.h"
#include "ApertureCore/include/aperturecore/geometry/Shape.h"
#include <string>

namespace DigitMode {

ShapeDrawDispatcher::ShapeDrawDispatcher()
    : m_rectangleRenderer(std::make_unique<RectangleRenderer>())
    , m_ellipseRenderer(std::make_unique<EllipseRenderer>())
    , m_polygonRenderer(std::make_unique<PolygonRenderer>())
{
}

ShapeDrawDispatcher::~ShapeDrawDispatcher() = default;

void ShapeDrawDispatcher::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Get appropriate renderer based on shape type
    const IShapeRenderer* renderer = GetRenderer(shape.typeName());
    
    if (renderer) {
        // Delegate to concrete renderer
        renderer->Draw(shape, dc, style, worldToScreen);
    }
    // Unknown shape types are silently ignored
}

void ShapeDrawDispatcher::DrawHandles(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Early exit if handles shouldn't be shown
    if (!style.showHandles) {
        return;
    }
    
    // Get appropriate renderer based on shape type
    const IShapeRenderer* renderer = GetRenderer(shape.typeName());
    
    if (renderer) {
        // Delegate to concrete renderer
        renderer->DrawHandles(shape, dc, style, worldToScreen);
    }
    // Unknown shape types are silently ignored
}

const IShapeRenderer* ShapeDrawDispatcher::GetRenderer(const std::string& typeName) const {
    // Dispatch based on shape type name (case-sensitive)
    if (typeName == "Rectangle") {
        return m_rectangleRenderer.get();
    }
    else if (typeName == "Ellipse") {
        return m_ellipseRenderer.get();
    }
    else if (typeName == "Polygon") {
        return m_polygonRenderer.get();
    }
    else {
        // Unknown shape type - no renderer available
        return nullptr;
    }
}

} // namespace DigitMode

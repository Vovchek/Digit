/**
 * @file PolygonRenderer.cpp
 * @brief Implementation of PolygonRenderer
 */

#include "stdafx.h"
#include "PolygonRenderer.h"
#include "ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

void PolygonRenderer::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Downcast to Polygon
    const auto& polygon = static_cast<const aperture::Polygon&>(shape);
    
    // Get vertices in world coordinates
    const auto& vertices = polygon.vertices();
    
    // Early exit if polygon has no vertices (degenerate case)
    if (vertices.empty()) {
        return;
    }
    
    // Transform to screen coordinates
    std::vector<CPoint> screenPoints;
    screenPoints.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        CPoint2d worldPt{vertex.x, vertex.y};
        screenPoints.push_back(worldToScreen.WorldToScreen(worldPt));
    }
    
    // Create pen based on style
    CPen pen(style.GetOutlineStyle(), style.GetOutlineWidth(), style.GetOutlineColor());
    CPen* oldPen = dc.SelectObject(&pen);
    
    // Create brush (if needed for INTERNAL shapes)
    CBrush* oldBrush = nullptr;
    CBrush brush;
    if (style.HasFill()) {
        brush.CreateSolidBrush(style.GetFillColor());
        oldBrush = dc.SelectObject(&brush);
    } else {
        oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    }
    
    // Draw closed polygon
    // Note: Polygon() automatically closes the shape by connecting last vertex to first
    dc.Polygon(screenPoints.data(), static_cast<int>(screenPoints.size()));
    
    // Restore device context
    dc.SelectObject(oldPen);
    dc.SelectObject(oldBrush);
}

void PolygonRenderer::DrawHandles(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Early exit if handles shouldn't be shown
    if (!style.showHandles) {
        return;
    }
    
    // Downcast to Polygon
    const auto& polygon = static_cast<const aperture::Polygon&>(shape);
    
    // Enumerate all handles (one per vertex)
    std::vector<aperture::HandleDesc> handles;
    polygon.EnumerateHandles(handles);
    
    // Draw each handle
    for (size_t i = 0; i < handles.size(); ++i) {
        // Transform handle position to screen coordinates
        CPoint2d worldPt{handles[i].localPos.x, handles[i].localPos.y};
        CPoint screenPos = worldToScreen.WorldToScreen(worldPt);
        
        // Get handle appearance
        COLORREF handleColor = style.GetHandleColor(static_cast<int>(i));
        int handleSize = style.GetHandleSize();
        
        // Draw handle as filled square with black outline
        CRect handleRect(
            screenPos.x - handleSize,
            screenPos.y - handleSize,
            screenPos.x + handleSize,
            screenPos.y + handleSize
        );
        
        // Fill handle
        CBrush handleBrush(handleColor);
        CBrush* oldBrush = dc.SelectObject(&handleBrush);
        CPen handlePen(PS_SOLID, 1, RGB(0, 0, 0));  // Black outline
        CPen* oldPen = dc.SelectObject(&handlePen);
        
        dc.Rectangle(handleRect);
        
        // Restore DC
        dc.SelectObject(oldPen);
        dc.SelectObject(oldBrush);
    }
}

} // namespace DigitMode

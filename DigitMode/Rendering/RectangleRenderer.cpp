/**
 * @file RectangleRenderer.cpp
 * @brief Implementation of RectangleRenderer
 */

#include "stdafx.h"
#include "RectangleRenderer.h"
#include "ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

void RectangleRenderer::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // 1. Downcast to Rectangle (safe - enforced by dispatcher)
    const auto& rect = static_cast<const aperture::Rectangle&>(shape);
    
    // 2. Get corners in world coordinates (4 points)
    auto corners = rect.corners();
    
    // 3. Transform to screen coordinates
    CPoint screenCorners[4];
    for (int i = 0; i < 4; ++i) {
        CPoint2d worldPt{corners[i].x, corners[i].y};
        screenCorners[i] = worldToScreen.WorldToScreen(worldPt);
    }
    
    // 4. Create pen based on style
    CPen pen(style.GetOutlineStyle(), style.GetOutlineWidth(), style.GetOutlineColor());
    CPen* oldPen = dc.SelectObject(&pen);
    
    // 5. Create brush (if needed for INTERNAL shapes)
    CBrush* oldBrush = nullptr;
    CBrush brush;
    if (style.HasFill()) {
        // Semi-transparent fill for INTERNAL shapes
        // Note: GDI doesn't support alpha blending directly, so we use solid color
        // For true transparency, would need AlphaBlend() or GDI+
        brush.CreateSolidBrush(style.GetFillColor());
        oldBrush = dc.SelectObject(&brush);
    } else {
        // No fill - transparent brush
        oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    }
    
    // 6. Draw polygon (closed 4-sided shape)
    dc.Polygon(screenCorners, 4);
    
    // 7. Restore device context
    dc.SelectObject(oldPen);
    dc.SelectObject(oldBrush);
}

void RectangleRenderer::DrawHandles(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Early exit if handles shouldn't be shown
    if (!style.showHandles) {
        return;
    }
    
    // Downcast to Rectangle
    const auto& rect = static_cast<const aperture::Rectangle&>(shape);
    
    // Enumerate all handles
    std::vector<aperture::HandleDesc> handles;
    rect.EnumerateHandles(handles);
    
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

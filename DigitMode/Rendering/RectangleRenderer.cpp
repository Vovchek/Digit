/**
 * @file RectangleRenderer.cpp
 * @brief Implementation of RectangleRenderer
 */

#include "stdafx.h"
#include "RectangleRenderer.h"
#include "ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ImageTempl/ViewTransform.h"

// GDI+ for alpha-blended fills
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

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
    
    // 4. Draw outline (GDI pen)
    CPen pen(style.GetOutlineStyle(), style.GetOutlineWidth(), style.GetOutlineColor());
    CPen* oldPen = dc.SelectObject(&pen);
    
    // 5. Draw fill with alpha blending (GDI+ for transparency)
    if (style.HasFill()) {
        COLORREF fillColorRef = style.GetFillColor();
        BYTE alpha = static_cast<BYTE>(style.GetFillAlpha());
        BYTE red = GetRValue(fillColorRef);
        BYTE green = GetGValue(fillColorRef);
        BYTE blue = GetBValue(fillColorRef);
        
        try {
            // Create GDI+ Graphics object from device context
            Gdiplus::Graphics graphics(dc.GetSafeHdc());
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
            
            // Create alpha-blended color (order: alpha, red, green, blue)
            Gdiplus::Color fillColor(alpha, red, green, blue);
            
            // Create brush with alpha
            Gdiplus::SolidBrush gdiBrush(fillColor);
            
            // Convert screen corners to GDI+ PointF
            Gdiplus::PointF gdiPoints[4];
            for (int i = 0; i < 4; ++i) {
                gdiPoints[i] = Gdiplus::PointF(
                    static_cast<Gdiplus::REAL>(screenCorners[i].x),
                    static_cast<Gdiplus::REAL>(screenCorners[i].y)
                );
            }
            
            // Fill polygon with alpha-blended brush
            graphics.FillPolygon(&gdiBrush, gdiPoints, 4);
        }
        catch (const std::exception& e) {
            // Fallback to GDI solid fill if GDI+ fails
            CBrush brush;
            brush.CreateSolidBrush(style.GetFillColor());
            CBrush* oldBrush = dc.SelectObject(&brush);
            dc.Polygon(screenCorners, 4);
            dc.SelectObject(oldBrush);
        }
        catch (...) {
            // Fallback to GDI solid fill if GDI+ fails
            CBrush brush;
            brush.CreateSolidBrush(style.GetFillColor());
            CBrush* oldBrush = dc.SelectObject(&brush);
            dc.Polygon(screenCorners, 4);
            dc.SelectObject(oldBrush);
        }
    }
    
    // 6. Redraw outline ONLY (no fill) - select NULL_BRUSH to prevent fill
    CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    
    // Draw just the outline with the pen
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

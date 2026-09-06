/**
 * @file PolygonRenderer.cpp
 * @brief Implementation of PolygonRenderer
 */

#include "stdafx.h"
#include "PolygonRenderer.h"
#include "ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"
#include "ImageTempl/ViewTransform.h"

// GDI+ for alpha-blended fills
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

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
    
    // Draw fill with alpha blending (GDI+ for transparency)
    if (style.HasFill()) {
        try {
            // Create GDI+ Graphics object from device context
            Gdiplus::Graphics graphics(dc.GetSafeHdc());
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
            
            // Get fill color and alpha
            COLORREF fillColorRef = style.GetFillColor();
            BYTE alpha = static_cast<BYTE>(style.GetFillAlpha());
            BYTE red = GetRValue(fillColorRef);
            BYTE green = GetGValue(fillColorRef);
            BYTE blue = GetBValue(fillColorRef);
            
            // Create alpha-blended color
            Gdiplus::Color fillColor(alpha, red, green, blue);
            Gdiplus::SolidBrush gdiBrush(fillColor);
            
            // Convert to GDI+ PointF array
            std::vector<Gdiplus::PointF> gdiPoints;
            gdiPoints.reserve(screenPoints.size());
            for (const auto& pt : screenPoints) {
                gdiPoints.push_back(Gdiplus::PointF(
                    static_cast<Gdiplus::REAL>(pt.x),
                    static_cast<Gdiplus::REAL>(pt.y)
                ));
            }
            
            // Draw filled polygon with GDI+
            graphics.FillPolygon(&gdiBrush, gdiPoints.data(), static_cast<INT>(gdiPoints.size()));
        }
        catch (...) {
            // Fallback to GDI solid fill
            CBrush brush;
            brush.CreateSolidBrush(style.GetFillColor());
            CBrush* oldBrush = dc.SelectObject(&brush);
            dc.Polygon(screenPoints.data(), static_cast<int>(screenPoints.size()));
            dc.SelectObject(oldBrush);
        }
    }
    
    // Draw outline ONLY (no fill) - select NULL_BRUSH
    // Note: Polygon() automatically closes the shape by connecting last vertex to first
    CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
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
    polygon.enumerateHandles(handles);
    
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

/**
 * @file EllipseRenderer.cpp
 * @brief Implementation of EllipseRenderer
 */

#include "stdafx.h"
#include "EllipseRenderer.h"
#include "ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/geometry/Ellipse.h"
#include "ImageTempl/ViewTransform.h"
#include <cmath>

// GDI+ for alpha-blended fills
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace DigitMode {

// Helper to convert aperture::Point to CPoint2d
static inline CPoint2d ToCPoint2d(const aperture::Point& pt) {
    return CPoint2d{pt.x, pt.y};
}

void EllipseRenderer::Draw(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Downcast to Ellipse
    const auto& ellipse = static_cast<const aperture::Ellipse&>(shape);
    
    // Check if rotation is negligible (within 0.1 degrees)
    const double rotationThreshold = 0.001;  // ~0.06 degrees in radians
    
    if (std::abs(ellipse.rotationRadians()) < rotationThreshold) {
        // Axis-aligned ellipse - use optimized GDI rendering
        DrawAxisAligned(ellipse, dc, style, worldToScreen);
    } else {
        // Rotated ellipse - use polygon approximation
        DrawRotated(ellipse, dc, style, worldToScreen);
    }
}

void EllipseRenderer::DrawAxisAligned(
    const aperture::Ellipse& ellipse,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Get ellipse parameters
    auto center = ellipse.center();
    double radiusX = ellipse.semiMajor();
    double radiusY = ellipse.semiMinor();
    
    // Transform center to screen coordinates
    CPoint screenCenter = worldToScreen.WorldToScreen(ToCPoint2d(center));
    
    // Transform radii (approximate scaling)
    // Note: This assumes uniform scaling; for non-uniform scaling, would need to transform axis endpoints
    double scale = worldToScreen.GetScale();
    
    int screenRadiusX = static_cast<int>(radiusX * scale);
    int screenRadiusY = static_cast<int>(radiusY * scale);
    
    // Create bounding rectangle
    CRect boundingRect(
        screenCenter.x - screenRadiusX,
        screenCenter.y - screenRadiusY,
        screenCenter.x + screenRadiusX,
        screenCenter.y + screenRadiusY
    );
    
    // Create pen and brush
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
            
            // Draw filled ellipse with GDI+
            graphics.FillEllipse(&gdiBrush, 
                screenCenter.x - screenRadiusX,
                screenCenter.y - screenRadiusY,
                screenRadiusX * 2,
                screenRadiusY * 2);
        }
        catch (...) {
            // Fallback to GDI solid fill
            CBrush brush;
            brush.CreateSolidBrush(style.GetFillColor());
            CBrush* oldBrush = dc.SelectObject(&brush);
            dc.Ellipse(boundingRect);
            dc.SelectObject(oldBrush);
        }
    }
    
    // Draw outline ONLY (no fill) - select NULL_BRUSH
    CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    dc.Ellipse(boundingRect);
    
    // Restore DC
    dc.SelectObject(oldPen);
    dc.SelectObject(oldBrush);
}

void EllipseRenderer::DrawRotated(
    const aperture::Ellipse& ellipse,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // For rotated ellipses, approximate with polygon
    const int segments = 64;  // Number of line segments
    
    auto center = ellipse.center();
    double radiusX = ellipse.semiMajor();
    double radiusY = ellipse.semiMinor();
    double rotation = ellipse.rotationRadians();
    
    // Generate ellipse points in world coordinates
    std::vector<CPoint> screenPoints;
    screenPoints.reserve(segments);
    
    for (int i = 0; i < segments; ++i) {
        double t = (2.0 * M_PI * i) / segments;
        
        // Parametric ellipse equation (before rotation)
        double x = radiusX * std::cos(t);
        double y = radiusY * std::sin(t);
        
        // Apply rotation
        double cosRot = std::cos(rotation);
        double sinRot = std::sin(rotation);
        double xRot = x * cosRot - y * sinRot;
        double yRot = x * sinRot + y * cosRot;
        
        // Translate to center
        aperture::Point worldPoint(center.x + xRot, center.y + yRot);
        
        // Transform to screen
        screenPoints.push_back(worldToScreen.WorldToScreen(ToCPoint2d(worldPoint)));
    }
    
    // Create pen and brush
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
    CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
    dc.Polygon(screenPoints.data(), static_cast<int>(screenPoints.size()));
    
    // Restore DC
    dc.SelectObject(oldPen);
    dc.SelectObject(oldBrush);
}

void EllipseRenderer::DrawHandles(
    const aperture::Shape& shape,
    CDC& dc,
    const ShapeDrawStyle& style,
    const ViewTransform& worldToScreen
) const {
    // Early exit if handles shouldn't be shown
    if (!style.showHandles) {
        return;
    }
    
    // Downcast to Ellipse
    const auto& ellipse = static_cast<const aperture::Ellipse&>(shape);
    
    // Enumerate all handles
    std::vector<aperture::HandleDesc> handles;
    ellipse.EnumerateHandles(handles);
    
    // Draw each handle
    for (size_t i = 0; i < handles.size(); ++i) {
        // Transform handle position to screen coordinates
        CPoint screenPos = worldToScreen.WorldToScreen(ToCPoint2d(handles[i].localPos));
        
        // Get handle appearance
        COLORREF handleColor = style.GetHandleColor(static_cast<int>(i));
        int handleSize = style.GetHandleSize();
        
        // Draw handle as filled circle (more appropriate for ellipse)
        CBrush handleBrush(handleColor);
        CBrush* oldBrush = dc.SelectObject(&handleBrush);
        CPen handlePen(PS_SOLID, 1, RGB(0, 0, 0));  // Black outline
        CPen* oldPen = dc.SelectObject(&handlePen);
        
        // Draw circle centered at handle position
        CRect handleRect(
            screenPos.x - handleSize,
            screenPos.y - handleSize,
            screenPos.x + handleSize,
            screenPos.y + handleSize
        );
        dc.Ellipse(handleRect);
        
        // Restore DC
        dc.SelectObject(oldPen);
        dc.SelectObject(oldBrush);
    }
}

} // namespace DigitMode

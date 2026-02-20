/**
 * @file ShapeDrawStyle.cpp
 * @brief Implementation of ShapeDrawStyle methods
 */

#include "stdafx.h"
#include "ShapeDrawStyle.h"

namespace DigitMode {

COLORREF ShapeDrawStyle::GetOutlineColor() const {
    switch (type) {
        case aperture::TypeLimits::EXTERNAL:
            return RGB(0, 255, 0);  // Green
        
        case aperture::TypeLimits::APERTURE:
            return RGB(0, 255, 255);  // Cyan
        
        case aperture::TypeLimits::INTERNAL:
            return RGB(255, 0, 0);  // Red
        
        default:
            return RGB(128, 128, 128);  // Gray fallback
    }
}

int ShapeDrawStyle::GetOutlineWidth() const {
    // COSMETIC pens must use width 0 or 1 (GDI limitation)
    // For dashed lines (Draft or INTERNAL), always use 0 (thinnest)
    if (state == State::Draft || type == aperture::TypeLimits::INTERNAL) {
        return 0;  // Thinnest cosmetic pen
    }
    
    switch (state) {
        case State::Idle:
            return 0;  // Thin (cosmetic)
        
        case State::Hovered:
            return 1;  // Slightly thicker (cosmetic)
        
        case State::Selected:
        case State::Dragging:
            return 1;  // Thick as possible with cosmetic
        
        default:
            return 0;
    }
}

int ShapeDrawStyle::GetOutlineStyle() const {
    // INTERNAL shapes always have dashed outline
    if (type == aperture::TypeLimits::INTERNAL) {
        return PS_DASH | PS_COSMETIC;  // COSMETIC required for dashed lines
    }
    
    // Draft shapes have dashed outline
    if (state == State::Draft) {
        return PS_DASH | PS_COSMETIC;  // COSMETIC required for dashed lines
    }
    
    // All others are solid
    return PS_SOLID | PS_COSMETIC;  // Use COSMETIC for consistency
}

COLORREF ShapeDrawStyle::GetFillColor() const {
    if (type == aperture::TypeLimits::INTERNAL) {
        // Semi-transparent orange/red for INTERNAL obstructions
        // Note: GDI doesn't support true alpha, so we'll use lighter color
        // to simulate transparency (or use AlphaBlend in renderers)
        return RGB(255, 128, 64);  // Light orange (simulated transparency)
    }
    
    // No fill for other types
    return RGB(0, 0, 0);  // Won't be used
}

int ShapeDrawStyle::GetFillAlpha() const {
    if (type == aperture::TypeLimits::INTERNAL) {
        return 128;  // 50% transparency (0-255 scale)
    }
    return 255;  // Fully opaque
}

bool ShapeDrawStyle::HasFill() const {
    // Only INTERNAL shapes have fill
    return (type == aperture::TypeLimits::INTERNAL);
}

COLORREF ShapeDrawStyle::GetHandleColor(int handleIndex) const {
    if (handleIndex == activeHandleIndex) {
        return RGB(255, 255, 0);  // Yellow for active handle
    }
    
    return RGB(255, 255, 255);  // White for normal handles
}

int ShapeDrawStyle::GetHandleSize() const {
    // Larger handles when selected/dragging for easier interaction
    if (state == State::Selected || state == State::Dragging) {
        return 6;  // pixels radius
    }
    
    return 5;  // pixels radius
}

} // namespace DigitMode

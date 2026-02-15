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
    switch (state) {
        case State::Idle:
            return 1;  // Thin
        
        case State::Hovered:
        case State::Draft:
            return 2;  // Thicker
        
        case State::Selected:
        case State::Dragging:
            return 3;  // Thick
        
        default:
            return 1;
    }
}

int ShapeDrawStyle::GetOutlineStyle() const {
    // INTERNAL shapes always have dashed outline
    if (type == aperture::TypeLimits::INTERNAL) {
        return PS_DASH;
    }
    
    // Draft shapes have dashed outline
    if (state == State::Draft) {
        return PS_DASH;
    }
    
    // All others are solid
    return PS_SOLID;
}

COLORREF ShapeDrawStyle::GetFillColor() const {
    if (type == aperture::TypeLimits::INTERNAL) {
        // Semi-transparent red (alpha blending done manually with brush)
        return RGB(255, 0, 0);
    }
    
    // No fill for other types
    return RGB(0, 0, 0);  // Won't be used
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

/**
 * @file EditMode.cpp
 * @brief Implementation of edit mode utility functions
 */

#include "stdafx.h"  // Includes Windows headers
#include "EditMode.h"

namespace DigitMode {

const char* GetEditModeName(EditMode mode) {
    switch (mode) {
        case EditMode::Select:
            return "Select/Edit";
        
        case EditMode::AddRectangle:
            return "Add Rectangle";
        
        case EditMode::AddEllipse:
            return "Add Ellipse";
        
        case EditMode::AddCircle:
            return "Add Circle";
        
        case EditMode::AddPolygon:
            return "Add Polygon";
        
        case EditMode::Delete:
            return "Delete";
        
        default:
            return "Unknown";
    }
}

LPCTSTR GetEditModeCursor(EditMode mode) {
    switch (mode) {
        case EditMode::Select:
            return IDC_ARROW;  // Standard pointer
        
        case EditMode::AddRectangle:
        case EditMode::AddEllipse:
        case EditMode::AddCircle:
        case EditMode::AddPolygon:
            return IDC_CROSS;  // Crosshair for precision placement
        
        case EditMode::Delete:
            return IDC_NO;     // Slash cursor (deletion indicator)
        
        default:
            return IDC_ARROW;
    }
}

} // namespace DigitMode

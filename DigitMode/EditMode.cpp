/**
 * @file EditMode.cpp
 * @brief Implementation of edit mode utility functions
 */

#include "stdafx.h"  // Includes Windows headers
#include "EditMode.h"

namespace DigitMode {

const char* GetEditModeName(ShapeEditMode mode) {
    switch (mode) {
        case ShapeEditMode::Select:
            return "Select/Edit";
        
        case ShapeEditMode::AddRectangle:
            return "Add Rectangle";
        
        case ShapeEditMode::AddEllipse:
            return "Add Ellipse";
        
        case ShapeEditMode::AddCircle:
            return "Add Circle";
        
        case ShapeEditMode::AddPolygon:
            return "Add Polygon";
        
        case ShapeEditMode::Delete:
            return "Delete";
        
        default:
            return "Unknown";
    }
}

LPCTSTR GetEditModeCursor(ShapeEditMode mode) {
    switch (mode) {
        case ShapeEditMode::Select:
            return IDC_ARROW;  // Standard pointer
        
        case ShapeEditMode::AddRectangle:
        case ShapeEditMode::AddEllipse:
        case ShapeEditMode::AddCircle:
        case ShapeEditMode::AddPolygon:
            return IDC_CROSS;  // Crosshair for precision placement
        
        case ShapeEditMode::Delete:
            return IDC_NO;     // Slash cursor (deletion indicator)
        
        default:
            return IDC_ARROW;
    }
}

} // namespace DigitMode

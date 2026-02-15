/**
 * @file DraftShape.cpp
 * @brief Implementation of DraftShape methods
 */

#include "DraftShape.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ApertureCore/include/aperturecore/geometry/Ellipse.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"

namespace DigitMode {

bool DraftShape::CanCommit() const {
    switch (kind) {
        case Kind::Rectangle:
            return perimeterPoints.size() >= 3;
        
        case Kind::Ellipse:
        case Kind::Circle:
            return perimeterPoints.size() >= 3;  // Minimum for LSM fit
        
        case Kind::Polygon:
            return perimeterPoints.size() >= 3;  // Minimum for closed polygon
        
        default:
            return false;
    }
}

std::unique_ptr<aperture::Shape> DraftShape::ToShape() const {
    if (!CanCommit()) {
        return nullptr;
    }
    
    switch (kind) {
        case Kind::Rectangle: {
            // Use 3-point constructor (UX Spec §2.1)
            if (perimeterPoints.size() < 3) {
                return nullptr;
            }
            
            return std::make_unique<aperture::Rectangle>(
                perimeterPoints[0],
                perimeterPoints[1],
                perimeterPoints[2],
                type
            );
        }
        
        case Kind::Ellipse: {
            // General ellipse fit (no circular constraint) (UX Spec §2.2)
            return aperture::Ellipse::FitEllipse(perimeterPoints, type);
        }
        
        case Kind::Circle: {
            // Constrained circle fit (equal radii enforced) (UX Spec §2.3)
            return aperture::Ellipse::FitCircle(perimeterPoints, type);
        }
        
        case Kind::Polygon: {
            // Direct vertex construction (UX Spec §2.4)
            if (perimeterPoints.size() < 3) {
                return nullptr;
            }
            
            // Check for self-intersection (basic validation)
            // TODO: Implement proper self-intersection check
            // For now, just create the polygon and let renderer handle it
            
            return std::make_unique<aperture::Polygon>(perimeterPoints, type);
        }
        
        default:
            return nullptr;
    }
}

std::unique_ptr<aperture::Shape> DraftShape::GetPreview() const {
    const size_t n = perimeterPoints.size();
    
    if (n == 0) {
        return nullptr;
    }
    
    switch (kind) {
        case Kind::Rectangle: {
            if (n == 1) {
                // Single point - no preview yet
                return nullptr;
            }
            else if (n == 2) {
                // Two points - show axis-aligned rectangle preview
                const auto& p0 = perimeterPoints[0];
                const auto& p1 = perimeterPoints[1];
                
                double width = std::abs(p1.x - p0.x);
                double height = std::abs(p1.y - p0.y);
                double centerX = (p0.x + p1.x) / 2.0;
                double centerY = (p0.y + p1.y) / 2.0;
                
                return std::make_unique<aperture::Rectangle>(
                    width, height, centerX, centerY, 0.0, type
                );
            }
            else {
                // Three or more points - use final 3-point constructor
                return std::make_unique<aperture::Rectangle>(
                    perimeterPoints[0],
                    perimeterPoints[1],
                    perimeterPoints[2],
                    type
                );
            }
        }
        
        case Kind::Ellipse: {
            if (n < 3) {
                // Need at least 3 points for ellipse fit
                return nullptr;
            }
            // Show current LSM fit
            return aperture::Ellipse::FitEllipse(perimeterPoints, type);
        }
        
        case Kind::Circle: {
            if (n < 3) {
                // Need at least 3 points for circle fit
                return nullptr;
            }
            // Show current LSM circle fit
            return aperture::Ellipse::FitCircle(perimeterPoints, type);
        }
        
        case Kind::Polygon: {
            if (n < 2) {
                // Need at least 2 points for polyline
                return nullptr;
            }
            // Show open polyline (not closed yet)
            // Use same constructor but mark as preview
            // Note: Polygon constructor closes automatically, so preview = final
            return std::make_unique<aperture::Polygon>(perimeterPoints, type);
        }
        
        default:
            return nullptr;
    }
}

void DraftShape::AddPoint(const aperture::Point& pt) {
    perimeterPoints.push_back(pt);
}

void DraftShape::Clear() {
    perimeterPoints.clear();
}

} // namespace DigitMode

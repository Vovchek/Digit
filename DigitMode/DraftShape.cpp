/**
 * @file DraftShape.cpp
 * @brief Implementation of DraftShape methods
 */
#include "DraftShape.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ApertureCore/include/aperturecore/geometry/Ellipse.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

std::unique_ptr<aperture::Shape> DraftShape::GetPreview() {
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
                // Remove unused points exceeding 3, if any
                if (n > 3) {
                    perimeterPoints.erase(perimeterPoints.begin()+2, perimeterPoints.begin() + n - 1);
                }
                return std::make_unique<aperture::Rectangle>(
                    perimeterPoints[0],
                    perimeterPoints[1],
                    perimeterPoints[2],
                    type
                );
            }
        }
        
        case Kind::Ellipse: {
            if (n < 4) {
                // Need at least 4 points for ellipse fit
                return nullptr;
            }
            // band-aid: limit to 5 points while LST is buggy
            // TODO: fix >5 points LSM fit in ellips constructor
            if (n > 5) {
                perimeterPoints.erase(perimeterPoints.begin() + 4, perimeterPoints.begin() + n - 1);
            }
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
    static aperture::Point last_pt{ 0.,0. };
    last_pt = pt;
    perimeterPoints.push_back(pt);
}

void DraftShape::Clear() {
    perimeterPoints.clear();
}

void DraftShape::CreateFromBoundingBox(
    const aperture::Point& topLeft,
    const aperture::Point& bottomRight
) {
    // Clear any existing points
    perimeterPoints.clear();
    
    // Calculate box parameters
    double centerX = (topLeft.x + bottomRight.x) / 2.0;
    double centerY = (topLeft.y + bottomRight.y) / 2.0;
    double width = std::abs(bottomRight.x - topLeft.x);
    double height = std::abs(bottomRight.y - topLeft.y);
    
    switch (kind) {
        case Kind::Rectangle: {
            // Add 3 corners for 3-point rectangle constructor
            // TL, TR, BR (avoids needing BL - 3 points define orientation)
            aperture::Point topRight(bottomRight.x, topLeft.y);
            perimeterPoints.push_back(topLeft);
            perimeterPoints.push_back(topRight);
            perimeterPoints.push_back(bottomRight);
            break;
        }
        
        case Kind::Ellipse: {
            // Generate 4 points around ellipse
            double radiusX = width / std::sqrt(2.0);
            double radiusY = height / std::sqrt(2.0);
            perimeterPoints.push_back(aperture::Point(centerX - radiusX, centerY));
            perimeterPoints.push_back(aperture::Point(centerX + radiusX, centerY));
            perimeterPoints.push_back(aperture::Point(centerX, centerY - radiusY));
            perimeterPoints.push_back(aperture::Point(centerX, centerY + radiusY));
            break;
        }
        
        case Kind::Circle: {
            // Generate 8 points around circle perimeter (use min radius for circle)
            double radius = std::min(width, height) / 2.0;
            
            for (int i = 0; i < 8; ++i) {
                double angle = (2.0 * M_PI * i) / 8.0;
                double x = centerX + radius * std::cos(angle);
                double y = centerY + radius * std::sin(angle);
                perimeterPoints.push_back(aperture::Point(x, y));
            }
            break;
        }
        
        case Kind::Polygon:
            // Polygon doesn't support bounding box creation
            // (use AddPoint for vertices instead)
            break;
    }
}

} // namespace DigitMode

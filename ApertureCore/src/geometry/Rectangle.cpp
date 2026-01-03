/**
 * @file Rectangle.cpp
 * @brief Implementation of Rectangle class
 */

#include "aperturecore/geometry/Rectangle.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace aperture {

Rectangle::Rectangle(double width, double height,
                     double centerX, double centerY,
                     double rotationDegrees)
    : width_(width)
    , height_(height)
    , center_(centerX, centerY)
    , rotationDeg_(rotationDegrees)
    , rotationRad_(rotationDegrees * M_PI / 180.0)
    , cosRot_(0.0)
    , sinRot_(0.0)
{
    updateRotationCache();
}

void Rectangle::updateRotationCache() {
    cosRot_ = std::cos(rotationRad_);
    sinRot_ = std::sin(rotationRad_);
}

Point Rectangle::toLocalCoordinates(const Point& point) const {
    // Translate to origin
    double dx = point.x - center_.x;
    double dy = point.y - center_.y;
    
    // Rotate by -rotation to align with axes
    return Point{
        dx * cosRot_ + dy * sinRot_,
        -dx * sinRot_ + dy * cosRot_
    };
}

bool Rectangle::isInside(const Point& point) const {
    // Transform to rectangle-local coordinates
    Point local = toLocalCoordinates(point);
    
    // Check if within rectangle bounds
    double halfWidth = width_ / 2.0;
    double halfHeight = height_ / 2.0;
    
    return (std::abs(local.x) <= halfWidth) && 
           (std::abs(local.y) <= halfHeight);
}

Bounds Rectangle::getBounds() const {
    auto cornerPts = corners();
    
    Bounds bounds = Bounds::infinite();
    bounds.clear();
    
    for (const auto& corner : cornerPts) {
        bounds.expand(corner);
    }
    
    return bounds;
}

std::array<Point, 4> Rectangle::corners() const {
    double halfW = width_ / 2.0;
    double halfH = height_ / 2.0;
    
    // Corners in local coordinates (before rotation)
    std::array<Point, 4> localCorners = {{
        {-halfW, -halfH},  // Top-left
        { halfW, -halfH},  // Top-right
        { halfW,  halfH},  // Bottom-right
        {-halfW,  halfH}   // Bottom-left
    }};
    
    // Transform to world coordinates
    std::array<Point, 4> worldCorners;
    for (size_t i = 0; i < 4; ++i) {
        double x = localCorners[i].x * cosRot_ - localCorners[i].y * sinRot_;
        double y = localCorners[i].x * sinRot_ + localCorners[i].y * cosRot_;
        worldCorners[i] = Point{x + center_.x, y + center_.y};
    }
    
    return worldCorners;
}

std::vector<Point> Rectangle::getContour(double stepSize) const {
    // For rectangles, stepSize determines points along edges
    auto cornerPts = corners();
    
    std::vector<Point> contour;
    
    // Calculate points per edge based on edge length and stepSize
    for (size_t i = 0; i < 4; ++i) {
        const Point& start = cornerPts[i];
        const Point& end = cornerPts[(i + 1) % 4];
        
        double edgeLength = start.distanceTo(end);
        int numSegments = std::max(1, static_cast<int>(edgeLength / stepSize));
        
        // Add points along this edge
        for (int j = 0; j < numSegments; ++j) {
            double t = static_cast<double>(j) / numSegments;
            contour.push_back(lerp(start, end, t));
        }
    }
    
    // Close the contour
    contour.push_back(cornerPts[0]);
    
    return contour;
}

double Rectangle::perimeter() const {
    return 2.0 * (width_ + height_);
}

double Rectangle::area() const {
    return width_ * height_;
}

std::unique_ptr<Shape> Rectangle::clone() const {
    return std::make_unique<Rectangle>(*this);
}

void Rectangle::normalize(double originX, double originY, double radius) {
    width_ /= radius;
    height_ /= radius;
    center_.x = (center_.x - originX) / radius;
    center_.y = (center_.y - originY) / radius;
    normState_ = NormalizationState::NORMALIZED;
}

void Rectangle::denormalize(double originX, double originY, double radius) {
    width_ *= radius;
    height_ *= radius;
    center_.x = center_.x * radius + originX;
    center_.y = center_.y * radius + originY;
    normState_ = NormalizationState::MEASURING;
}

void Rectangle::inverseY(double centerY) {
    center_.y = centerY - center_.y;
    rotationDeg_ = -rotationDeg_;
    rotationRad_ = -rotationRad_;
    updateRotationCache();
}

void Rectangle::shiftX(double deltaX) {
    center_.x += deltaX;
}

void Rectangle::shiftY(double deltaY) {
    center_.y += deltaY;
}

} // namespace aperture

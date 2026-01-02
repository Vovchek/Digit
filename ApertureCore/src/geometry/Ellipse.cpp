/**
 * @file Ellipse.cpp
 * @brief Implementation of Ellipse class
 */

#include "aperturecore/geometry/Ellipse.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace aperture {

Ellipse::Ellipse(double semiMajorAxis, double semiMinorAxis,
                 double centerX, double centerY,
                 double rotationDegrees)
    : semiMajor_(semiMajorAxis)
    , semiMinor_(semiMinorAxis)
    , center_(centerX, centerY)
    , rotationDeg_(rotationDegrees)
    , rotationRad_(rotationDegrees * M_PI / 180.0)
    , cosRot_(0.0)
    , sinRot_(0.0)
{
    updateRotationCache();
}

void Ellipse::updateRotationCache() {
    cosRot_ = std::cos(rotationRad_);
    sinRot_ = std::sin(rotationRad_);
}

Point Ellipse::toLocalCoordinates(const Point& point) const {
    // Translate to origin
    double dx = point.x - center_.x;
    double dy = point.y - center_.y;
    
    // Rotate by -rotation to align with axes
    return Point{
        dx * cosRot_ + dy * sinRot_,
        -dx * sinRot_ + dy * cosRot_
    };
}

Point Ellipse::toWorldCoordinates(const Point& point) const {
    // Rotate by +rotation
    double x = point.x * cosRot_ - point.y * sinRot_;
    double y = point.x * sinRot_ + point.y * cosRot_;
    
    // Translate to center
    return Point{x + center_.x, y + center_.y};
}

bool Ellipse::isInside(const Point& point) const {
    // Transform to ellipse-local coordinates
    Point local = toLocalCoordinates(point);
    
    // Check ellipse equation: (x/a)^2 + (y/b)^2 <= 1
    double term1 = (local.x * local.x) / (semiMajor_ * semiMajor_);
    double term2 = (local.y * local.y) / (semiMinor_ * semiMinor_);
    
    return (term1 + term2) <= 1.0;
}

Bounds Ellipse::getBounds() const {
    if (std::abs(rotationDeg_) < 1e-6) {
        // No rotation - simple case
        return Bounds{
            center_.x - semiMajor_,
            center_.y - semiMinor_,
            center_.x + semiMajor_,
            center_.y + semiMinor_
        };
    }
    
    // For rotated ellipse, find the extreme points
    // The bounding box vertices are where dx/dt = 0 and dy/dt = 0
    // in the parametric equations:
    // x(t) = cx + a*cos(t)*cos(r) - b*sin(t)*sin(r)
    // y(t) = cy + a*cos(t)*sin(r) + b*sin(t)*cos(r)
    
    double a2_cos2 = semiMajor_ * semiMajor_ * cosRot_ * cosRot_;
    double b2_sin2 = semiMinor_ * semiMinor_ * sinRot_ * sinRot_;
    double a2_sin2 = semiMajor_ * semiMajor_ * sinRot_ * sinRot_;
    double b2_cos2 = semiMinor_ * semiMinor_ * cosRot_ * cosRot_;
    
    double halfWidth = std::sqrt(a2_cos2 + b2_sin2);
    double halfHeight = std::sqrt(a2_sin2 + b2_cos2);
    
    return Bounds{
        center_.x - halfWidth,
        center_.y - halfHeight,
        center_.x + halfWidth,
        center_.y + halfHeight
    };
}

std::vector<Point> Ellipse::getContour(double stepSize) const {
    // Calculate number of points based on perimeter and step size
    double perim = perimeter();
    int numPoints = std::max(8, static_cast<int>(perim / stepSize));
    
    std::vector<Point> contour;
    contour.reserve(numPoints + 1);  // +1 for closing point
    
    // Generate points parametrically
    double angleStep = 2.0 * M_PI / numPoints;
    
    for (int i = 0; i <= numPoints; ++i) {
        double t = i * angleStep;
        
        // Parametric ellipse in local coordinates
        Point local{
            semiMajor_ * std::cos(t),
            semiMinor_ * std::sin(t)
        };
        
        // Transform to world coordinates
        contour.push_back(toWorldCoordinates(local));
    }
    
    return contour;
}

double Ellipse::perimeter() const {
    // Use Ramanujan's approximation for ellipse perimeter
    // P ? ? * (3(a + b) - sqrt((3a + b)(a + 3b)))
    // This is accurate to within 0.01% for most ellipses
    
    double a = semiMajor_;
    double b = semiMinor_;
    
    if (isCircle()) {
        // Exact for circles
        return 2.0 * M_PI * a;
    }
    
    // Ramanujan's second approximation
    double h = ((a - b) * (a - b)) / ((a + b) * (a + b));
    return M_PI * (a + b) * (1.0 + (3.0 * h) / (10.0 + std::sqrt(4.0 - 3.0 * h)));
}

double Ellipse::area() const {
    return M_PI * semiMajor_ * semiMinor_;
}

double Ellipse::eccentricity() const {
    if (semiMajor_ < semiMinor_) {
        // b > a, swap for calculation
        double e2 = 1.0 - (semiMajor_ * semiMajor_) / (semiMinor_ * semiMinor_);
        return std::sqrt(std::max(0.0, e2));
    }
    
    double e2 = 1.0 - (semiMinor_ * semiMinor_) / (semiMajor_ * semiMajor_);
    return std::sqrt(std::max(0.0, e2));
}

double Ellipse::focalDistance() const {
    if (isCircle()) {
        return 0.0;
    }
    
    double a = std::max(semiMajor_, semiMinor_);
    double b = std::min(semiMajor_, semiMinor_);
    
    return std::sqrt(a * a - b * b);
}

std::unique_ptr<Shape> Ellipse::clone() const {
    return std::make_unique<Ellipse>(*this);
}

void Ellipse::normalize(double originX, double originY, double radius) {
    semiMajor_ /= radius;
    semiMinor_ /= radius;
    center_.x = (center_.x - originX) / radius;
    center_.y = (center_.y - originY) / radius;
    coordSystem_ = CoordinateSystem::NORMALIZED;
}

void Ellipse::denormalize(double originX, double originY, double radius) {
    semiMajor_ *= radius;
    semiMinor_ *= radius;
    center_.x = center_.x * radius + originX;
    center_.y = center_.y * radius + originY;
    coordSystem_ = CoordinateSystem::MEASURING;
}

void Ellipse::inverseY(double centerY) {
    center_.y = centerY - center_.y;
    rotationDeg_ = -rotationDeg_;
    rotationRad_ = -rotationRad_;
    updateRotationCache();
}

void Ellipse::shiftX(double deltaX) {
    center_.x += deltaX;
}

void Ellipse::shiftY(double deltaY) {
    center_.y += deltaY;
}

} // namespace aperture

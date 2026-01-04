/**
 * @file Polygon.cpp
 * @brief Implementation of Polygon class
 */

#include "aperturecore/geometry/Polygon.h"
#include <cmath>
#include <algorithm>

namespace aperture {

Polygon::Polygon(const std::vector<Point>& vertices,
                 TypeLimits typeLimits,
                 CoordinateSystem spatialSystem,
                 NormalizationState normState)
    : vertices_(vertices)
{
    typeLimits_ = typeLimits;
    spatialSystem_ = spatialSystem;
    normState_ = normState;
}

Polygon::Polygon(std::initializer_list<Point> vertices,
                 TypeLimits typeLimits,
                 CoordinateSystem spatialSystem,
                 NormalizationState normState)
    : vertices_(vertices)
{
    typeLimits_ = typeLimits;
    spatialSystem_ = spatialSystem;
    normState_ = normState;
}

void Polygon::addVertex(const Point& point) {
    vertices_.push_back(point);
}

bool Polygon::isClosed(double tolerance) const {
    if (vertices_.size() < 2) {
        return false;
    }
    
    return vertices_.front().isNear(vertices_.back(), tolerance);
}

bool Polygon::isDegenerate(double tolerance) const {
    if (vertices_.size() < 3) {
        return true;
    }
    
    return std::abs(area()) < tolerance;
}

void Polygon::ensureClosed(double tolerance) {
    if (vertices_.size() < 2) {
        return;
    }
    
    if (!isClosed(tolerance)) {
        vertices_.push_back(vertices_.front());
    }
}

bool Polygon::isInside(const Point& point) const {
    if (vertices_.size() < 3) {
        return false;  // Degenerate polygon
    }
    
    // Ray-casting algorithm
    // Cast a ray from the point to the right and count intersections
    // If odd number of intersections, point is inside
    
    bool inside = false;
    size_t n = vertices_.size();
    
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const Point& vi = vertices_[i];
        const Point& vj = vertices_[j];
        
        // Check if ray crosses edge
        if (((vi.y > point.y) != (vj.y > point.y)) &&
            (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x)) {
            inside = !inside;
        }
    }
    
    return inside;
}

Bounds Polygon::getBounds() const {
    if (vertices_.empty()) {
        return Bounds{};
    }
    
    Bounds bounds;
    for (const auto& vertex : vertices_) {
        bounds.expand(vertex);
    }
    
    return bounds;
}

std::vector<Point> Polygon::getContour(double stepSize) const {
    if (vertices_.empty()) {
        return {};
    }
    
    // For polygons, we can interpolate along edges if stepSize requires it
    std::vector<Point> contour;
    size_t n = vertices_.size();
    
    for (size_t i = 0; i < n - 1; ++i) {
        const Point& start = vertices_[i];
        const Point& end = vertices_[i + 1];
        
        double edgeLength = start.distanceTo(end);
        int numSegments = std::max(1, static_cast<int>(edgeLength / stepSize));
        
        // Add interpolated points along edge
        for (int j = 0; j < numSegments; ++j) {
            double t = static_cast<double>(j) / numSegments;
            contour.push_back(lerp(start, end, t));
        }
    }
    
    // Ensure closed
    if (!vertices_.empty() && contour.back() != vertices_.front()) {
        contour.push_back(vertices_.front());
    }
    
    return contour;
}

double Polygon::perimeter() const {
    if (vertices_.size() < 2) {
        return 0.0;
    }
    
    double perim = 0.0;
    size_t n = vertices_.size();
    
    for (size_t i = 0; i < n - 1; ++i) {
        perim += vertices_[i].distanceTo(vertices_[i + 1]);
    }
    
    // Add closing edge if not already closed
    if (!isClosed()) {
        perim += vertices_.back().distanceTo(vertices_.front());
    }
    
    return perim;
}

double Polygon::signedArea() const {
    if (vertices_.size() < 3) {
        return 0.0;
    }
    
    // Shoelace formula (also known as surveyor's formula)
    double sum = 0.0;
    size_t n = vertices_.size();
    
    for (size_t i = 0; i < n - 1; ++i) {
        sum += vertices_[i].x * vertices_[i + 1].y;
        sum -= vertices_[i + 1].x * vertices_[i].y;
    }
    
    // Close the polygon if needed
    if (!isClosed()) {
        sum += vertices_[n - 1].x * vertices_[0].y;
        sum -= vertices_[0].x * vertices_[n - 1].y;
    }
    
    return sum / 2.0;
}

double Polygon::area() const {
    return std::abs(signedArea());
}

Point Polygon::centroid() const {
    if (vertices_.empty()) {
        return Point{};
    }
    
    if (vertices_.size() == 1) {
        return vertices_[0];
    }
    
    // Calculate centroid using weighted average by area
    double cx = 0.0, cy = 0.0;
    double signedAreaTotal = 0.0;
    size_t n = vertices_.size();
    
    for (size_t i = 0; i < n - 1; ++i) {
        double x0 = vertices_[i].x;
        double y0 = vertices_[i].y;
        double x1 = vertices_[i + 1].x;
        double y1 = vertices_[i + 1].y;
        
        double a = x0 * y1 - x1 * y0;
        signedAreaTotal += a;
        cx += (x0 + x1) * a;
        cy += (y0 + y1) * a;
    }
    
    // Close if needed
    if (!isClosed()) {
        double x0 = vertices_[n - 1].x;
        double y0 = vertices_[n - 1].y;
        double x1 = vertices_[0].x;
        double y1 = vertices_[0].y;
        
        double a = x0 * y1 - x1 * y0;
        signedAreaTotal += a;
        cx += (x0 + x1) * a;
        cy += (y0 + y1) * a;
    }
    
    if (std::abs(signedAreaTotal) < 1e-10) {
        // Degenerate polygon, use simple average
        Point sum{};
        for (const auto& v : vertices_) {
            sum += v;
        }
        return sum / static_cast<double>(vertices_.size());
    }
    
    signedAreaTotal *= 3.0;
    return Point{cx / signedAreaTotal, cy / signedAreaTotal};
}

bool Polygon::isConvex() const {
    if (vertices_.size() < 3) {
        return false;
    }
    
    bool hasPositive = false;
    bool hasNegative = false;
    size_t n = vertices_.size();
    
    for (size_t i = 0; i < n; ++i) {
        const Point& p0 = vertices_[i];
        const Point& p1 = vertices_[(i + 1) % n];
        const Point& p2 = vertices_[(i + 2) % n];
        
        // Calculate cross product of vectors (p1-p0) and (p2-p1)
        Point v1 = p1 - p0;
        Point v2 = p2 - p1;
        double cross = v1.cross(v2);
        
        if (cross > 1e-10) hasPositive = true;
        if (cross < -1e-10) hasNegative = true;
        
        if (hasPositive && hasNegative) {
            return false;  // Mixed signs = not convex
        }
    }
    
    return true;
}

std::unique_ptr<Shape> Polygon::clone() const {
    return std::make_unique<Polygon>(*this);
}

void Polygon::normalize(double originX, double originY, double radius) {
    for (auto& vertex : vertices_) {
        vertex.x = (vertex.x - originX) / radius;
        vertex.y = (vertex.y - originY) / radius;
    }
    normState_ = NormalizationState::NORMALIZED;
}

void Polygon::denormalize(double originX, double originY, double radius) {
    for (auto& vertex : vertices_) {
        vertex.x = vertex.x * radius + originX;
        vertex.y = vertex.y * radius + originY;
    }
    normState_ = NormalizationState::MEASURING;
}

void Polygon::inverseY(double centerY) {
    for (auto& vertex : vertices_) {
        vertex.y = centerY - vertex.y;
    }
}

void Polygon::shiftX(double deltaX) {
    for (auto& vertex : vertices_) {
        vertex.x += deltaX;
    }
}

void Polygon::shiftY(double deltaY) {
    for (auto& vertex : vertices_) {
        vertex.y += deltaY;
    }
}

} // namespace aperture

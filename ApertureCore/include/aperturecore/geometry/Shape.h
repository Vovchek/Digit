/**
 * @file Shape.h
 * @brief Abstract base class for all geometric shapes
 */
#pragma once

#include "Point.h"
#include "Bounds.h"
#include "../visibility/TypeLimits.h"
#include <vector>
#include <memory>

namespace aperture {

/**
 * @brief Abstract base class for geometric shapes
 * 
 * Provides common interface for all shapes (Ellipse, Rectangle, Polygon).
 * Handles TypeLimits and defines pure virtual methods for geometry operations.
 */
class Shape {
public:
    virtual ~Shape() = default;
    
    // Core geometric interface (pure virtual)
    
    /**
     * @brief Test if a point is inside the shape
     * @param point Point to test
     * @return true if point is inside the shape boundary
     */
    virtual bool isInside(const Point& point) const = 0;
    
    /**
     * @brief Get axis-aligned bounding box
     * @return Bounds containing the entire shape
     */
    virtual Bounds getBounds() const = 0;
    
    /**
     * @brief Get contour points of the shape
     * @param stepSize Maximum distance between consecutive points
     * @return Vector of points forming the shape boundary
     */
    virtual std::vector<Point> getContour(double stepSize) const = 0;
    
    /**
     * @brief Calculate perimeter/circumference
     * @return Length of the shape boundary
     */
    virtual double perimeter() const = 0;
    
    /**
     * @brief Create a deep copy of the shape
     * @return Unique pointer to cloned shape
     */
    virtual std::unique_ptr<Shape> clone() const = 0;
    
    // TypeLimits management
    
    /**
     * @brief Get visibility type of this shape
     * @return Current TypeLimits setting
     */
    TypeLimits getTypeLimits() const { 
        return typeLimits_; 
    }
    
    /**
     * @brief Set visibility type of this shape
     * @param type New TypeLimits value
     */
    void setTypeLimits(TypeLimits type) { 
        typeLimits_ = type; 
    }
    
    // Optional: Area calculation (not all shapes implement this)
    
    /**
     * @brief Calculate area enclosed by the shape
     * @return Area in square units (returns 0.0 if not implemented)
     */
    virtual double area() const {
        return 0.0;  // Default: not implemented
    }
    
    // Shape type identification (for debugging/serialization)
    
    /**
     * @brief Get human-readable type name
     * @return String identifier (e.g., "Ellipse", "Rectangle")
     */
    virtual const char* typeName() const = 0;

protected:
    TypeLimits typeLimits_{TypeLimits::EXTERNAL};  ///< Visibility behavior
};

} // namespace aperture

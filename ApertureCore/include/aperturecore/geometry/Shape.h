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
 * @brief Coordinate system type for shapes
 */
enum class CoordinateSystem {
    MEASURING = 0,   ///< Original measuring coordinates
    NORMALIZED = 1   ///< Normalized coordinates (centered, scaled)
};

/**
 * @brief Abstract base class for geometric shapes
 * 
 * Provides common interface for all shapes (Ellipse, Rectangle, Polygon).
 * Handles TypeLimits, coordinate system tracking, and defines pure virtual 
 * methods for geometry operations.
 * 
 * Replaces XYShape with modern C++ design while maintaining compatibility
 * with legacy coordinate normalization functionality.
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
    
    // Coordinate transformation interface (pure virtual)
    
    /**
     * @brief Normalize coordinates to unit system
     * @param originX Origin X coordinate in measuring system
     * @param originY Origin Y coordinate in measuring system
     * @param radius Normalization radius/scale factor
     * 
     * Transforms from measuring coordinates to normalized coordinates:
     * x_norm = (x_meas - originX) / radius
     * y_norm = (y_meas - originY) / radius
     */
    virtual void normalize(double originX, double originY, double radius) = 0;
    
    /**
     * @brief Denormalize coordinates back to measuring system
     * @param originX Origin X coordinate in measuring system
     * @param originY Origin Y coordinate in measuring system
     * @param radius Normalization radius/scale factor
     * 
     * Transforms from normalized coordinates to measuring coordinates:
     * x_meas = x_norm * radius + originX
     * y_meas = y_norm * radius + originY
     */
    virtual void denormalize(double originX, double originY, double radius) = 0;
    
    /**
     * @brief Invert Y coordinate (mirror across horizontal line)
     * @param centerY Y coordinate of inversion axis
     */
    virtual void inverseY(double centerY) = 0;
    
    /**
     * @brief Shift shape in X direction
     * @param deltaX Amount to shift
     */
    virtual void shiftX(double deltaX) = 0;
    
    /**
     * @brief Shift shape in Y direction
     * @param deltaY Amount to shift
     */
    virtual void shiftY(double deltaY) = 0;
    
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
    
    // Coordinate system management
    
    /**
     * @brief Get current coordinate system type
     * @return Current coordinate system
     */
    CoordinateSystem getCoordinateSystem() const {
        return coordSystem_;
    }
    
    /**
     * @brief Set coordinate system type
     * @param system New coordinate system
     */
    void setCoordinateSystem(CoordinateSystem system) {
        coordSystem_ = system;
    }
    
    /**
     * @brief Check if coordinates are normalized
     * @return true if in normalized coordinate system
     */
    bool isNormalized() const {
        return coordSystem_ == CoordinateSystem::NORMALIZED;
    }
    
    /**
     * @brief Check if coordinates are in measuring system
     * @return true if in measuring coordinate system
     */
    bool isMeasuring() const {
        return coordSystem_ == CoordinateSystem::MEASURING;
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
    TypeLimits typeLimits_{TypeLimits::EXTERNAL};          ///< Visibility behavior
    CoordinateSystem coordSystem_{CoordinateSystem::MEASURING};  ///< Coordinate system type
};

} // namespace aperture

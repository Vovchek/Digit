/**
 * @file Shape.h
 * @brief Abstract base class for all geometric shapes
 */
#pragma once

#include "Point.h"
#include "Bounds.h"
#include "CoordinateSystem.h"
#include "../visibility/TypeLimits.h"
#include <vector>
#include <memory>

namespace aperture {

/**
 * @brief Normalization state for shapes
 */
enum class NormalizationState {
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
 * ## Coordinate Systems
 * 
 * Shapes track TWO coordinate properties:
 * 
 * 1. **Spatial System** (SCREEN vs MATH):
 *    - SCREEN: Y+ downward (bitmap/image coordinates)
 *    - MATH: Y+ upward (mathematical coordinates)
 *    - Affects: rotation direction, bounds validation, Y comparisons
 * 
 * 2. **Normalization State** (MEASURING vs NORMALIZED):
 *    - MEASURING: Original physical coordinates
 *    - NORMALIZED: Scaled/centered coordinates
 *    - Affects: coordinate interpretation, denormalization
 * 
 * These are independent: a shape can be in any combination
 * (SCREEN+MEASURING, SCREEN+NORMALIZED, MATH+MEASURING, MATH+NORMALIZED).
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
     * 
     * Used for converting between SCREEN and MATH coordinate systems.
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
    
    // Spatial coordinate system management (SCREEN vs MATH)
    
    /**
     * @brief Get spatial coordinate system
     * @return Current coordinate system (SCREEN or MATH)
     */
    CoordinateSystem getSpatialSystem() const {
        return spatialSystem_;
    }
    
    /**
     * @brief Set spatial coordinate system
     * @param system New coordinate system
     * 
     * @note This does NOT transform coordinates, only updates the tag.
     *       Use transformToSystem() to convert coordinates.
     */
    void setSpatialSystem(const CoordinateSystem& system) {
        spatialSystem_ = system;
    }
    
    /**
     * @brief Transform shape to different spatial coordinate system
     * @param targetSystem Target coordinate system
     * 
     * Converts shape coordinates from current system to target system.
     * For SCREEN ? MATH conversion, this inverts the Y-axis around
     * the reference height stored in the coordinate system.
     * 
     * @code{.cpp}
     * // Shape in screen coordinates
     * Ellipse ellipse(50, 50, 100, 50);  // Center at screen (100, 50)
     * ellipse.setSpatialSystem(CoordinateSystem::screen(768.0));
     * 
     * // Convert to math coordinates
     * ellipse.transformToSystem(CoordinateSystem::math(768.0));
     * // Now center is at math (100, 718)  ? 768 - 50 = 718
     * @endcode
     * 
     * @throws std::invalid_argument if reference height not set for conversion
     */
    virtual void transformToSystem(const CoordinateSystem& targetSystem) {
        if (spatialSystem_.type() == targetSystem.type()) {
            return;  // Already in target system
        }
        
        // Get reference height for Y-axis inversion
        double height = targetSystem.referenceHeight();
        if (height <= 0.0) {
            height = spatialSystem_.referenceHeight();
        }
        
        if (height <= 0.0) {
            throw std::invalid_argument(
                "Shape::transformToSystem: reference height must be set for coordinate conversion"
            );
        }
        
        // Invert Y-axis around midpoint
        inverseY(height / 2.0);
        
        // Update system tag
        spatialSystem_ = targetSystem;
    }
    
    // Normalization state management (MEASURING vs NORMALIZED)
    
    /**
     * @brief Get normalization state
     * @return Current normalization state
     */
    NormalizationState getNormalizationState() const {
        return normState_;
    }
    
    /**
     * @brief Set normalization state
     * @param state New normalization state
     * 
     * @note This does NOT transform coordinates, only updates the tag.
     *       Use normalize()/denormalize() to transform coordinates.
     */
    void setNormalizationState(NormalizationState state) {
        normState_ = state;
    }
    
    /**
     * @brief Check if coordinates are normalized
     * @return true if in normalized coordinate system
     */
    bool isNormalized() const {
        return normState_ == NormalizationState::NORMALIZED;
    }
    
    /**
     * @brief Check if coordinates are in measuring system
     * @return true if in measuring coordinate system
     */
    bool isMeasuring() const {
        return normState_ == NormalizationState::MEASURING;
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
    TypeLimits typeLimits_{TypeLimits::EXTERNAL};           ///< Visibility behavior
    CoordinateSystem spatialSystem_{CoordinateSystem::screen()};  ///< Spatial coordinate system (SCREEN/MATH)
    NormalizationState normState_{NormalizationState::MEASURING};  ///< Normalization state
};

} // namespace aperture

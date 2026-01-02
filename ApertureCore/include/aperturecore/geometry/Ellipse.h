/**
 * @file Ellipse.h
 * @brief Elliptical shape with rotation support
 */
#pragma once

#include "Shape.h"
#include <cmath>

namespace aperture {

/**
 * @brief Ellipse shape with optional rotation
 * 
 * Represents an ellipse defined by semi-major and semi-minor axes,
 * center position, and rotation angle. Supports all Shape operations
 * including rotated point-in-ellipse testing and contour generation.
 * 
 * Replaces XYEllipse with modern C++ design.
 */
class Ellipse : public Shape {
public:
    /**
     * @brief Construct ellipse
     * @param semiMajorAxis Semi-major axis length (A)
     * @param semiMinorAxis Semi-minor axis length (B)
     * @param centerX Center X coordinate
     * @param centerY Center Y coordinate
     * @param rotationDegrees Rotation angle in degrees (counter-clockwise)
     */
    Ellipse(double semiMajorAxis, double semiMinorAxis,
            double centerX, double centerY,
            double rotationDegrees = 0.0);
    
    // Shape interface implementation
    
    bool isInside(const Point& point) const override;
    Bounds getBounds() const override;
    std::vector<Point> getContour(double stepSize) const override;
    double perimeter() const override;
    double area() const override;
    std::unique_ptr<Shape> clone() const override;
    const char* typeName() const override { return "Ellipse"; }
    
    // Ellipse-specific properties
    
    /**
     * @brief Get center point
     */
    Point center() const { return center_; }
    
    /**
     * @brief Get semi-major axis length
     */
    double semiMajor() const { return semiMajor_; }
    
    /**
     * @brief Get semi-minor axis length
     */
    double semiMinor() const { return semiMinor_; }
    
    /**
     * @brief Get rotation angle in degrees
     */
    double rotationDegrees() const { return rotationDeg_; }
    
    /**
     * @brief Get rotation angle in radians
     */
    double rotationRadians() const { return rotationRad_; }
    
    /**
     * @brief Check if ellipse is actually a circle
     */
    bool isCircle(double tolerance = 1e-6) const {
        return std::abs(semiMajor_ - semiMinor_) < tolerance;
    }
    
    /**
     * @brief Get eccentricity (0 for circle, approaching 1 for elongated)
     */
    double eccentricity() const;
    
    /**
     * @brief Get focal distance (distance from center to focus)
     */
    double focalDistance() const;

private:
    double semiMajor_;      ///< Semi-major axis (A)
    double semiMinor_;      ///< Semi-minor axis (B)
    Point center_;          ///< Center point
    double rotationDeg_;    ///< Rotation in degrees
    double rotationRad_;    ///< Rotation in radians (cached)
    
    // Cached trigonometric values for rotation
    double cosRot_;         ///< cos(rotation)
    double sinRot_;         ///< sin(rotation)
    
    /**
     * @brief Update cached rotation values
     */
    void updateRotationCache();
    
    /**
     * @brief Transform point from world to ellipse local coordinates
     * @param point Point in world coordinates
     * @return Point in ellipse-local coordinates (centered, aligned)
     */
    Point toLocalCoordinates(const Point& point) const;
    
    /**
     * @brief Transform point from ellipse local to world coordinates
     * @param point Point in ellipse-local coordinates
     * @return Point in world coordinates
     */
    Point toWorldCoordinates(const Point& point) const;
};

} // namespace aperture

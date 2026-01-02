/**
 * @file Rectangle.h
 * @brief Rectangular shape with rotation support
 */
#pragma once

#include "Shape.h"
#include <array>

namespace aperture {

/**
 * @brief Rectangle shape with optional rotation
 * 
 * Represents an axis-aligned or rotated rectangle defined by width, height,
 * center position, and rotation angle. Supports all Shape operations.
 * 
 * Replaces XYRect with modern C++ design.
 */
class Rectangle : public Shape {
public:
    /**
     * @brief Construct rectangle
     * @param width Width (along local X axis before rotation)
     * @param height Height (along local Y axis before rotation)
     * @param centerX Center X coordinate
     * @param centerY Center Y coordinate
     * @param rotationDegrees Rotation angle in degrees (counter-clockwise)
     */
    Rectangle(double width, double height,
              double centerX, double centerY,
              double rotationDegrees = 0.0);
    
    // Shape interface implementation
    
    bool isInside(const Point& point) const override;
    Bounds getBounds() const override;
    std::vector<Point> getContour(double stepSize) const override;
    double perimeter() const override;
    double area() const override;
    std::unique_ptr<Shape> clone() const override;
    const char* typeName() const override { return "Rectangle"; }
    
    // Rectangle-specific properties
    
    /**
     * @brief Get center point
     */
    Point center() const { return center_; }
    
    /**
     * @brief Get width
     */
    double width() const { return width_; }
    
    /**
     * @brief Get height
     */
    double height() const { return height_; }
    
    /**
     * @brief Get rotation angle in degrees
     */
    double rotationDegrees() const { return rotationDeg_; }
    
    /**
     * @brief Get rotation angle in radians
     */
    double rotationRadians() const { return rotationRad_; }
    
    /**
     * @brief Check if rectangle is actually a square
     */
    bool isSquare(double tolerance = 1e-6) const {
        return std::abs(width_ - height_) < tolerance;
    }
    
    /**
     * @brief Get corner points in order: TL, TR, BR, BL
     */
    std::array<Point, 4> corners() const;

private:
    double width_;          ///< Width (local X)
    double height_;         ///< Height (local Y)
    Point center_;          ///< Center point
    double rotationDeg_;    ///< Rotation in degrees
    double rotationRad_;    ///< Rotation in radians (cached)
    
    // Cached trigonometric values
    double cosRot_;         ///< cos(rotation)
    double sinRot_;         ///< sin(rotation)
    
    /**
     * @brief Update cached rotation values
     */
    void updateRotationCache();
    
    /**
     * @brief Transform point from world to rectangle local coordinates
     */
    Point toLocalCoordinates(const Point& point) const;
};

} // namespace aperture

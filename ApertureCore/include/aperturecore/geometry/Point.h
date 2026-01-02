/**
 * @file Point.h
 * @brief 2D point class with geometric operations
 */
#pragma once

#include <cmath>
#include <iosfwd>

namespace aperture {

/**
 * @brief 2D point in Cartesian coordinates
 * 
 * Replaces XYPoint with modern C++ design.
 * Uses double precision for compatibility with legacy code.
 */
struct Point {
    double x{0.0};  ///< X coordinate
    double y{0.0};  ///< Y coordinate
    
    // Constructors
    constexpr Point() = default;
    constexpr Point(double x_, double y_) : x(x_), y(y_) {}
    
    // Distance calculations
    
    /**
     * @brief Calculate Euclidean distance to another point
     */
    double distanceTo(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx*dx + dy*dy);
    }
    
    /**
     * @brief Calculate squared distance (faster, no sqrt)
     */
    double distanceSquaredTo(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx*dx + dy*dy;
    }
    
    /**
     * @brief Get distance from origin
     */
    double magnitude() const {
        return std::sqrt(x*x + y*y);
    }
    
    /**
     * @brief Get squared magnitude (faster)
     */
    double magnitudeSquared() const {
        return x*x + y*y;
    }
    
    // Arithmetic operators
    
    Point& operator+=(const Point& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Point& operator-=(const Point& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    Point& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    
    Point& operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }
    
    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }
    
    Point operator-(const Point& other) const {
        return {x - other.x, y - other.y};
    }
    
    Point operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }
    
    Point operator/(double scalar) const {
        return {x / scalar, y / scalar};
    }
    
    Point operator-() const {
        return {-x, -y};
    }
    
    // Comparison operators
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Check if points are within tolerance distance
     * @param other Point to compare with
     * @param tolerance Maximum distance for equality
     */
    bool isNear(const Point& other, double tolerance = 1e-6) const {
        return distanceSquaredTo(other) <= tolerance * tolerance;
    }
    
    // Geometric operations
    
    /**
     * @brief Dot product with another point
     */
    double dot(const Point& other) const {
        return x * other.x + y * other.y;
    }
    
    /**
     * @brief Cross product (z-component of 3D cross product)
     */
    double cross(const Point& other) const {
        return x * other.y - y * other.x;
    }
    
    /**
     * @brief Normalize to unit length
     * @return Normalized point (undefined if magnitude is 0)
     */
    Point normalized() const {
        double mag = magnitude();
        return mag > 0.0 ? (*this / mag) : Point{};
    }
    
    /**
     * @brief Rotate point around origin
     * @param angleRad Rotation angle in radians (counter-clockwise)
     */
    Point rotated(double angleRad) const {
        double cosA = std::cos(angleRad);
        double sinA = std::sin(angleRad);
        return {
            x * cosA - y * sinA,
            x * sinA + y * cosA
        };
    }
    
    /**
     * @brief Rotate point around a center
     */
    Point rotatedAround(const Point& center, double angleRad) const {
        return (*this - center).rotated(angleRad) + center;
    }
};

// Free functions

inline Point operator*(double scalar, const Point& p) {
    return p * scalar;
}

/**
 * @brief Calculate distance between two points
 */
inline double distance(const Point& a, const Point& b) {
    return a.distanceTo(b);
}

/**
 * @brief Linear interpolation between two points
 * @param a First point
 * @param b Second point
 * @param t Interpolation parameter [0, 1]
 */
inline Point lerp(const Point& a, const Point& b, double t) {
    return a + (b - a) * t;
}

/**
 * @brief Stream output operator
 */
std::ostream& operator<<(std::ostream& os, const Point& p);

} // namespace aperture

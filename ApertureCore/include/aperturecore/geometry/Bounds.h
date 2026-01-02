/**
 * @file Bounds.h
 * @brief Axis-aligned bounding box
 */
#pragma once

#include "Point.h"
#include <algorithm>
#include <limits>
#include <array>

namespace aperture {

/**
 * @brief Axis-aligned 2D bounding rectangle
 * 
 * Replaces XYBounds with improved interface and semantics.
 * Convention: top < bottom (Y increases downward, screen coordinates)
 */
class Bounds {
public:
    double left{0.0};    ///< Minimum X coordinate
    double top{0.0};     ///< Minimum Y coordinate  
    double right{0.0};   ///< Maximum X coordinate
    double bottom{0.0};  ///< Maximum Y coordinate
    
    // Constructors
    
    /**
     * @brief Default constructor - creates empty bounds at origin
     */
    constexpr Bounds() = default;
    
    /**
     * @brief Construct from coordinates
     */
    constexpr Bounds(double l, double t, double r, double b)
        : left(l), top(t), right(r), bottom(b) {}
    
    /**
     * @brief Construct from two corner points
     */
    static Bounds fromCorners(const Point& p1, const Point& p2) {
        return {
            std::min(p1.x, p2.x),
            std::min(p1.y, p2.y),
            std::max(p1.x, p2.x),
            std::max(p1.y, p2.y)
        };
    }
    
    /**
     * @brief Construct from center and size
     */
    static Bounds fromCenterAndSize(const Point& center, double width, double height) {
        double halfW = width / 2.0;
        double halfH = height / 2.0;
        return {
            center.x - halfW,
            center.y - halfH,
            center.x + halfW,
            center.y + halfH
        };
    }
    
    /**
     * @brief Create infinite bounds (for initial expansion)
     */
    static Bounds infinite() {
        constexpr double inf = std::numeric_limits<double>::infinity();
        return {inf, inf, -inf, -inf};
    }
    
    // Properties
    
    /**
     * @brief Check if bounds are empty (zero or negative area)
     */
    bool isEmpty() const {
        return left == 0.0 && top == 0.0 && right == 0.0 && bottom == 0.0;
    }
    
    /**
     * @brief Check if bounds are valid (non-negative dimensions)
     */
    bool isValid() const {
        return left <= right && top <= bottom;
    }
    
    /**
     * @brief Get width
     */
    double width() const {
        return right - left;
    }
    
    /**
     * @brief Get height
     */
    double height() const {
        return bottom - top;
    }
    
    /**
     * @brief Get area
     */
    double area() const {
        return width() * height();
    }
    
    /**
     * @brief Get center point
     */
    Point center() const {
        return {(left + right) / 2.0, (top + bottom) / 2.0};
    }
    
    /**
     * @brief Get perimeter
     */
    double perimeter() const {
        return 2.0 * (width() + height());
    }
    
    /**
     * @brief Get corner points (TL, TR, BR, BL)
     */
    std::array<Point, 4> corners() const {
        return {{
            {left, top},      // Top-left
            {right, top},     // Top-right
            {right, bottom},  // Bottom-right
            {left, bottom}    // Bottom-left
        }};
    }
    
    // Modifications
    
    /**
     * @brief Clear bounds to empty state
     */
    void clear() {
        left = top = right = bottom = 0.0;
    }
    
    /**
     * @brief Shift bounds by offset
     */
    void shift(double dx, double dy) {
        left += dx;
        right += dx;
        top += dy;
        bottom += dy;
    }
    
    /**
     * @brief Shift bounds by point
     */
    void shift(const Point& offset) {
        shift(offset.x, offset.y);
    }
    
    /**
     * @brief Expand to include a point
     */
    void expand(const Point& point) {
        if (isEmpty()) {
            left = right = point.x;
            top = bottom = point.y;
        } else {
            left = std::min(left, point.x);
            right = std::max(right, point.x);
            top = std::min(top, point.y);
            bottom = std::max(bottom, point.y);
        }
    }
    
    /**
     * @brief Merge with another bounds (union)
     */
    void merge(const Bounds& other) {
        if (other.isEmpty()) return;
        if (isEmpty()) {
            *this = other;
        } else {
            left = std::min(left, other.left);
            top = std::min(top, other.top);
            right = std::max(right, other.right);
            bottom = std::max(bottom, other.bottom);
        }
    }
    
    /**
     * @brief Expand bounds by margin on all sides
     */
    void inflate(double margin) {
        left -= margin;
        top -= margin;
        right += margin;
        bottom += margin;
    }
    
    /**
     * @brief Expand bounds by different margins
     */
    void inflate(double dx, double dy) {
        left -= dx;
        right += dx;
        top -= dy;
        bottom += dy;
    }
    
    // Queries
    
    /**
     * @brief Check if point is inside bounds (inclusive)
     */
    bool contains(const Point& point) const {
        return point.x >= left && point.x <= right &&
               point.y >= top && point.y <= bottom;
    }
    
    /**
     * @brief Check if other bounds is completely inside
     */
    bool contains(const Bounds& other) const {
        return other.left >= left && other.right <= right &&
               other.top >= top && other.bottom <= bottom;
    }
    
    /**
     * @brief Check if bounds intersects with another
     */
    bool intersects(const Bounds& other) const {
        return !(right < other.left || left > other.right ||
                 bottom < other.top || top > other.bottom);
    }
    
    /**
     * @brief Get intersection with another bounds
     */
    Bounds intersection(const Bounds& other) const {
        if (!intersects(other)) {
            return Bounds{};
        }
        return {
            std::max(left, other.left),
            std::max(top, other.top),
            std::min(right, other.right),
            std::min(bottom, other.bottom)
        };
    }
    
    /**
     * @brief Get union with another bounds
     */
    Bounds unionWith(const Bounds& other) const {
        Bounds result = *this;
        result.merge(other);
        return result;
    }
    
    /**
     * @brief Clamp point to bounds
     */
    Point clamp(const Point& point) const {
        return {
            std::clamp(point.x, left, right),
            std::clamp(point.y, top, bottom)
        };
    }
    
    // Comparison
    
    bool operator==(const Bounds& other) const {
        return left == other.left && top == other.top &&
               right == other.right && bottom == other.bottom;
    }
    
    bool operator!=(const Bounds& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Stream output operator
 */
std::ostream& operator<<(std::ostream& os, const Bounds& b);

} // namespace aperture

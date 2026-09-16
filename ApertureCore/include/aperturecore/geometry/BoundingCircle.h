/**
 * @file BoundingCircle.h
 * @brief Bounding circle structure for 2D shapes
 * @author Vladimir N. Chekal
 * @see https://github.com/Vovchek
 */
#pragma once

#include "Point.h"

namespace aperture {

/**
 * @struct BoundingCircle
 * @brief Represents a circle that bounds one or more shapes
 * 
 * Contains the center and radius of a circle that completely contains
 * the geometry it represents. The circle may be computed using various
 * algorithms (e.g., minimum bounding circle, circumcircle, etc.).
 * 
 * @note When `valid` is false, center and radius values are undefined
 * and should not be used for geometric operations.
 * 
 * ## Usage Example
 * @code{.cpp}
 * #include <aperturecore/geometry/BoundingCircle.h>
 * 
 * using namespace aperture;
 * 
 * // Get bounding circle from shapes
 * ShapeCollection shapes;
 * shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0));
 * 
 * BoundingCircle circle = shapes.getBoundingCircle();
 * if (circle.valid) {
 *     std::cout << "Center: (" << circle.center.x << ", " 
 *               << circle.center.y << "), Radius: " << circle.radius << "\n";
 * }
 * @endcode
 * 
 * @see ShapeCollection::getBoundingCircle()
 */
struct BoundingCircle {
    Point center{0.0, 0.0};  ///< Center point of the circle
    double radius{0.0};       ///< Radius of the circle
    bool valid{false};         ///< Whether the circle represents valid geometry
};

} // namespace aperture#pragma once

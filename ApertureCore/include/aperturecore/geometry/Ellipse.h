/**
 * @file Ellipse.h
 * @brief Elliptical shape with rotation support
 */
#pragma once

#include "Shape.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
     * @param typeLimits Visibility type (default: EXTERNAL)
     * @param spatialSystem Spatial coordinate system (default: SCREEN)
     * @param normState Normalization state (default: MEASURING)
     */
    Ellipse(double semiMajorAxis, double semiMinorAxis,
            double centerX, double centerY,
            double rotationDegrees = 0.0,
            TypeLimits typeLimits = TypeLimits::EXTERNAL,
            CoordinateSystem spatialSystem = CoordinateSystem::screen(),
            NormalizationState normState = NormalizationState::MEASURING);
    
    /**
     * @brief Construct ellipse by fitting to point set
     * @param points Points to fit ellipse through
     * @param typeLimits Visibility type (default: EXTERNAL)
     * @param spatialSystem Spatial coordinate system (default: SCREEN)
     * @param normState Normalization state (default: MEASURING)
     * 
     * Fits an ellipse to the given points using different algorithms based on
     * the number of points:
     * 
     * - **0 points**: Degenerate ellipse at origin with zero radii
     * - **1 point**: Degenerate ellipse (point) at that location
     * - **2 points**: Circle with diameter defined by the two points
     * - **3 points**: Circle through three points (geometric fit)
     * - **4 points**: Axis-aligned ellipse (center = average, axes = max extents)
     * - **5 points**: Exact ellipse (general conic through 5 points)
     * - **6+ points**: Least squares ellipse fit (algebraic distance minimization)
     * 
     * ## Algorithm Details
     * 
     * ### 3 Points - Circle Through Three Points
     * 
     * Solves for circle center (xc, yc) and radius R such that all three
     * points lie on the circle. Uses geometric circle fitting formula.
     * 
     * Falls back to centroid if points are collinear.
     * 
     * ### 5 Points - Exact Ellipse Fit
     * 
     * Fits general conic equation: Ax² + Bxy + Cy² + Dx + Ey + F = 0
     * 
     * Constraints: F = 1 (normalization), B²-4AC < 0 (ellipse condition)
     * 
     * Converts conic coefficients to ellipse parameters:
     * - Center (xc, yc)
     * - Semi-major and semi-minor axes (a, b)
     * - Rotation angle φ
     * 
     * Falls back to 4-point method if conic is not an ellipse.
     * 
     * ### 6+ Points - Least Squares Ellipse Fit
     * 
     * Minimizes algebraic distance: Σ(Ax²ᵢ + Bxᵢyᵢ + Cy²ᵢ + Dxᵢ + Eyᵢ + F)²
     * 
     * Subject to ellipse constraint: B²-4AC < 0
     * 
     * Uses linear system: (D'D)α = D'b where α = [A,B,C,D,E]'
     * 
     * Converts solution to geometric ellipse parameters.
     * 
     * Falls back to 4-point method if fit fails or is degenerate.
     * 
     * ## Usage Examples
     * 
     * @code{.cpp}
     * #include <aperturecore/geometry/Ellipse.h>
     * #include <vector>
     * 
     * using namespace aperture;
     * 
     * // Circle through 3 points
     * std::vector<Point> threePoints = {
     *     {0.0, 0.0},
     *     {10.0, 0.0},
     *     {5.0, 8.66}  // Forms equilateral triangle
     * };
     * Ellipse circle(threePoints);
     * // Result: circle centered at ~(5, 2.89) with radius ~5.77
     * 
     * // Exact ellipse through 5 points
     * std::vector<Point> fivePoints = {
     *     {10.0, 0.0},   // Right
     *     {0.0, 5.0},    // Top
     *     {-10.0, 0.0},  // Left
     *     {0.0, -5.0},   // Bottom
     *     {7.07, 3.54}   // Diagonal
     * };
     * Ellipse exact(fivePoints);
     * // Result: ellipse centered at ~(0,0), a~10, b~5, rotation~0°
     * 
     * // Least squares fit (noisy data)
     * std::vector<Point> noisyPoints;
     * for (int i = 0; i < 100; i++) {
     *     double angle = 2.0 * M_PI * i / 100.0;
     *     double x = 10.0 * cos(angle) + (rand() % 100 - 50) / 100.0;
     *     double y = 5.0 * sin(angle) + (rand() % 100 - 50) / 100.0;
     *     noisyPoints.push_back({x, y});
     * }
     * Ellipse fitted(noisyPoints);
     * // Result: best-fit ellipse to noisy data, a~10, b~5
     * @endcode
     * 
     * ## Performance
     * 
     * - **2-4 points**: O(1) - Simple geometric calculations
     * - **5 points**: O(1) - Solves 5x5 linear system
     * - **N > 5 points**: O(N) for matrix assembly, O(1) for 5x5 solve
     * 
     * ## Fallback Behavior
     * 
     * If ellipse fitting fails (e.g., points are collinear, conic is hyperbola):
     * - Falls back to simpler method (5→4, 6+→4)
     * - 4-point method creates axis-aligned ellipse from bounding extents
     * - Always produces a valid ellipse (possibly degenerate)
     * 
     * ## Mathematical References
     * 
     * - Fitzgibbon, Pilu, Fisher: "Direct Least Square Fitting of Ellipses" (1999)
     * - Halir, Flusser: "Numerically Stable Direct Least Squares Fitting" (1998)
     * 
     * @param points Vector of 2D points to fit
     * @param typeLimits Visibility behavior (EXTERNAL/INTERNAL/APERTURE)
     * @param spatialSystem Coordinate system (SCREEN/MATH)
     * @param normState Normalization state (MEASURING/NORMALIZED)
     * 
     * @note For best results with noisy data, use 10+ points
     * @note Collinear points produce degenerate (zero-area) ellipse
     * @warning Least squares fit can be sensitive to outliers
     * 
     * @see Ellipse(double, double, double, double, double) - Direct construction
     * @see fitEllipseDirect() - Alternative direct fit method (if needed)
     */
    explicit Ellipse(const std::vector<Point>& points,
                    TypeLimits typeLimits = TypeLimits::EXTERNAL,
                    CoordinateSystem spatialSystem = CoordinateSystem::screen(),
                    NormalizationState normState = NormalizationState::MEASURING);
    
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
    
    // Coordinate transformation interface implementation
    
    /**
     * @brief Normalize coordinates to unit system
     */
    void normalize(double originX, double originY, double radius) override;
    
    /**
     * @brief Denormalize coordinates back to measuring system
     */
    void denormalize(double originX, double originY, double radius) override;
    
    /**
     * @brief Invert Y coordinate
     */
    void inverseY(double centerY) override;
    
    /**
     * @brief Shift shape in X direction
     */
    void shiftX(double deltaX) override;
    
    /**
     * @brief Shift shape in Y direction
     */
    void shiftY(double deltaY) override;

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

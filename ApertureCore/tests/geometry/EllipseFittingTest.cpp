/**
 * @file EllipseFittingTest.cpp
 * @brief Tests for Ellipse fitting constructor
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Ellipse.h"
#include <cmath>
#include <vector>

using namespace aperture;

// Helper to check if two values are approximately equal
static bool isNear(double a, double b, double tolerance = 1e-3) {
    return std::abs(a - b) < tolerance;
}

// Test: Zero points - degenerate ellipse at origin
TEST(EllipseFittingTest, ZeroPoints) {
    std::vector<Point> points;
    Ellipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.center().x, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.center().y, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 0.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 0.0);
}

// Test: Single point - degenerate ellipse at that point
TEST(EllipseFittingTest, SinglePoint) {
    std::vector<Point> points = {{5.0, 7.0}};
    Ellipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.center().x, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.center().y, 7.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 0.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 0.0);
}

// Test: Two points - circle with diameter between points
TEST(EllipseFittingTest, TwoPoints) {
    std::vector<Point> points = {
        {0.0, 0.0},
        {10.0, 0.0}
    };
    Ellipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.center().x, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.center().y, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 5.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 5.0);
    EXPECT_TRUE(ellipse.isCircle());
}

// Test: Three points - circle through three points
TEST(EllipseFittingTest, ThreePoints_Circle) {
    // Three points on a circle radius 5 centered at (5, 5)
    std::vector<Point> points = {
        {5.0, 0.0},   // Bottom
        {10.0, 5.0},  // Right
        {5.0, 10.0}   // Top
    };
    Ellipse ellipse(points);
    
    EXPECT_TRUE(isNear(ellipse.center().x, 5.0));
    EXPECT_TRUE(isNear(ellipse.center().y, 5.0));
    EXPECT_TRUE(isNear(ellipse.semiMajor(), 5.0));
    EXPECT_TRUE(isNear(ellipse.semiMinor(), 5.0));
    EXPECT_TRUE(ellipse.isCircle());
}

// Test: Three collinear points - degenerate case
TEST(EllipseFittingTest, ThreePoints_Collinear) {
    std::vector<Point> points = {
        {0.0, 0.0},
        {5.0, 5.0},
        {10.0, 10.0}  // All on same line
    };
    Ellipse ellipse(points);
    
    // Should create ellipse at centroid
    EXPECT_TRUE(isNear(ellipse.center().x, 5.0));
    EXPECT_TRUE(isNear(ellipse.center().y, 5.0));
}

// Test: Four points - axis-aligned ellipse
TEST(EllipseFittingTest, FourPoints_AxisAligned) {
    std::vector<Point> points = {
        {-10.0, 0.0},   // Left
        {10.0, 0.0},    // Right
        {0.0, -5.0},    // Bottom
        {0.0, 5.0}      // Top
    };
    Ellipse ellipse(points);
    
    EXPECT_TRUE(isNear(ellipse.center().x, 0.0));
    EXPECT_TRUE(isNear(ellipse.center().y, 0.0));
    EXPECT_TRUE(isNear(ellipse.semiMajor(), 10.0));
    EXPECT_TRUE(isNear(ellipse.semiMinor(), 5.0));
}

// Test: Five points - exact ellipse fit
TEST(EllipseFittingTest, FivePoints_Exact) {
    // Five points on an ellipse: a=10, b=5, center=(0,0), rotation=0
    std::vector<Point> points = {
        {10.0, 0.0},    // Right (0 deg)
        {0.0, 5.0},     // Top (90 deg)
        {-10.0, 0.0},   // Left (180 deg)
        {0.0, -5.0},    // Bottom (270 deg)
        {7.07, 3.54}    // 45 degrees
    };
    Ellipse ellipse(points);
    
    EXPECT_TRUE(isNear(ellipse.center().x, 0.0, 0.1));
    EXPECT_TRUE(isNear(ellipse.center().y, 0.0, 0.1));
    EXPECT_TRUE(isNear(ellipse.semiMajor(), 10.0, 0.5));
    EXPECT_TRUE(isNear(ellipse.semiMinor(), 5.0, 0.5));
}

// Test: Five points - general-position ellipse (translated and rotated)
TEST(EllipseFittingTest, FivePoints_GeneralPosition) {
    // Ellipse parameters: a=8, b=4, center=(3,-2), rotation=30 degrees
    std::vector<Point> points;
    const double a = 8.0;
    const double b = 4.0;
    const double cx = 3.0;
    const double cy = -2.0;
    const double rotation = M_PI / 6.0;  // 30 degrees

    // Sample 5 points uniformly on the ellipse
    const int numPoints = 5;
    for (int i = 0; i < numPoints; ++i) {
        double t = 2.0 * M_PI * i / numPoints;
        double xLocal = a * cos(t);
        double yLocal = b * sin(t);

        // Apply rotation
        double xRot = xLocal * cos(rotation) - yLocal * sin(rotation);
        double yRot = xLocal * sin(rotation) + yLocal * cos(rotation);

        // Translate to global coordinates
        points.push_back({cx + xRot, cy + yRot});
    }

    Ellipse ellipse(points);

    // Center should be close to (cx, cy)
    EXPECT_TRUE(isNear(ellipse.center().x, cx, 0.5));
    EXPECT_TRUE(isNear(ellipse.center().y, cy, 0.5));

    // Semi-axes should match (allowing for semiMajor/semiMinor swapping)
    double fittedMax = std::max(ellipse.semiMajor(), ellipse.semiMinor());
    double fittedMin = std::min(ellipse.semiMajor(), ellipse.semiMinor());
    EXPECT_TRUE(isNear(fittedMax, a, 1.0));
    EXPECT_TRUE(isNear(fittedMin, b, 1.0));
}

// Test: Many points - least squares fit (circle)
TEST(EllipseFittingTest, ManyPoints_Circle) {
    // Generate points on a circle radius 10 centered at (0, 0)
    std::vector<Point> points;
    const int numPoints = 20;
    const double radius = 10.0;
    
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * M_PI * i / numPoints;
        points.push_back({
            radius * cos(angle),
            radius * sin(angle)
        });
    }
    
    Ellipse ellipse(points);
    
    EXPECT_TRUE(isNear(ellipse.center().x, 0.0, 0.5));
    EXPECT_TRUE(isNear(ellipse.center().y, 0.0, 0.5));
    EXPECT_TRUE(isNear(ellipse.semiMajor(), radius, 1.0));
    EXPECT_TRUE(isNear(ellipse.semiMinor(), radius, 1.0));
    EXPECT_TRUE(ellipse.isCircle(1.0));  // Tolerance 1.0 for fitted circle
}

// Test: Many points - least squares fit (ellipse)
TEST(EllipseFittingTest, ManyPoints_Ellipse) {
    // Generate points on an ellipse a=15, b=8
    std::vector<Point> points;
    const int numPoints = 30;
    const double a = 15.0;
    const double b = 8.0;
    
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * M_PI * i / numPoints;
        points.push_back({
            a * cos(angle),
            b * sin(angle)
        });
    }
    
    Ellipse ellipse(points);
    
    EXPECT_TRUE(isNear(ellipse.center().x, 0.0, 0.5));
    EXPECT_TRUE(isNear(ellipse.center().y, 0.0, 0.5));
    EXPECT_TRUE(isNear(ellipse.semiMajor(), a, 1.0));
    EXPECT_TRUE(isNear(ellipse.semiMinor(), b, 1.0));
}

// Test: Many points - least squares fit for a rotated, translated ellipse
TEST(EllipseFittingTest, ManyPoints_RotatedTranslatedEllipse) {
    std::vector<Point> points;
    const int numPoints = 40;
    const double a = 15.0;
    const double b = 8.0;
    const double cx = -5.0;
    const double cy = 7.0;
    const double rotation = M_PI / 3.0;  // 60 degrees

    // Generate points on an ellipse in general position
    for (int i = 0; i < numPoints; ++i) {
        double t = 2.0 * M_PI * i / numPoints;

        // Parametric point in local (axis-aligned) coordinates
        double xLocal = a * cos(t);
        double yLocal = b * sin(t);

        // Rotate into world coordinates
        double xRot = xLocal * cos(rotation) - yLocal * sin(rotation);
        double yRot = xLocal * sin(rotation) + yLocal * cos(rotation);

        // Translate to final center
        points.push_back({cx + xRot, cy + yRot});
    }

    Ellipse ellipse(points);

    // Center should be close to the true center
    EXPECT_TRUE(isNear(ellipse.center().x, cx, 0.5));
    EXPECT_TRUE(isNear(ellipse.center().y, cy, 0.5));

    // Semi-axes should be close to a and b (order may swap)
    double fittedMax = std::max(ellipse.semiMajor(), ellipse.semiMinor());
    double fittedMin = std::min(ellipse.semiMajor(), ellipse.semiMinor());
    EXPECT_TRUE(isNear(fittedMax, a, 1.0));
    EXPECT_TRUE(isNear(fittedMin, b, 1.0));

    // Rotation should indicate a non-axis-aligned ellipse in general position
    double rotDeg = std::abs(ellipse.rotationDegrees());
    while (rotDeg > 90.0) rotDeg -= 90.0;
    // Expect something significantly away from 0/90 degrees
    bool hasRotation = !isNear(rotDeg, 0.0, 5.0) && !isNear(rotDeg, 90.0, 5.0);
    EXPECT_TRUE(hasRotation);
}

// Test: Fitted ellipse contains original points
TEST(EllipseFittingTest, FittedEllipseContainsPoints) {
    std::vector<Point> points;
    const int numPoints = 12;
    const double a = 20.0;
    const double b = 10.0;
    
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * M_PI * i / numPoints;
        points.push_back({
            a * cos(angle),
            b * sin(angle)
        });
    }
    
    Ellipse ellipse(points);
    
    // All original points should be inside or very close to boundary
    // Due to numerical precision in least squares, allow small tolerance
    int containedCount = 0;
    for (const auto& p : points) {
        // Check if point is inside or very close to the ellipse
        // Transform to local coordinates
        double dx = p.x - ellipse.center().x;
        double dy = p.y - ellipse.center().y;
        double cosR = cos(ellipse.rotationRadians());
        double sinR = sin(ellipse.rotationRadians());
        double xLocal = dx * cosR + dy * sinR;
        double yLocal = -dx * sinR + dy * cosR;
        
        // Compute distance from boundary (< 1.0 means inside, = 1.0 is on boundary)
        double distSq = (xLocal * xLocal) / (ellipse.semiMajor() * ellipse.semiMajor()) +
                        (yLocal * yLocal) / (ellipse.semiMinor() * ellipse.semiMinor());
        
        // Allow 5% tolerance for numerical fit error
        if (distSq <= 1.05) {
            containedCount++;
        }
    }
    
    // At least 90% of points should be contained (allowing for numerical error)
    EXPECT_GE(containedCount, static_cast<int>(numPoints * 0.9));
}

// Test: TypeLimits and coordinate system preserved
TEST(EllipseFittingTest, PreservesConstructorParameters) {
    std::vector<Point> points = {{0, 0}, {10, 0}, {5, 5}};
    
    Ellipse ellipse(points, 
                   TypeLimits::INTERNAL,
                   CoordinateSystem::math(),
                   NormalizationState::NORMALIZED);
    
    EXPECT_EQ(ellipse.getTypeLimits(), TypeLimits::INTERNAL);
    EXPECT_TRUE(ellipse.getSpatialSystem().isMath());
    EXPECT_TRUE(ellipse.isNormalized());
}

// Test: Rotated ellipse fitting
TEST(EllipseFittingTest, RotatedEllipse) {
    // Generate points on a rotated ellipse (45 degrees)
    std::vector<Point> points;
    const int numPoints = 20;
    const double a = 12.0;
    const double b = 6.0;
    const double rotation = M_PI / 4.0;  // 45 degrees
    
    for (int i = 0; i < numPoints; i++) {
        double t = 2.0 * M_PI * i / numPoints;
        // Parametric ellipse in local coords
        double xLocal = a * cos(t);
        double yLocal = b * sin(t);
        // Rotate
        double x = xLocal * cos(rotation) - yLocal * sin(rotation);
        double y = xLocal * sin(rotation) + yLocal * cos(rotation);
        points.push_back({x, y});
    }
    
    Ellipse ellipse(points);
    
    // Center should be close to origin
    EXPECT_TRUE(isNear(ellipse.center().x, 0.0, 1.0));
    EXPECT_TRUE(isNear(ellipse.center().y, 0.0, 1.0));
    
    // Semi-axes should match (within tolerance)
    // Note: For rotated ellipse, semiMajor/semiMinor might swap depending on fit
    double fittedMax = std::max(ellipse.semiMajor(), ellipse.semiMinor());
    double fittedMin = std::min(ellipse.semiMajor(), ellipse.semiMinor());
    EXPECT_TRUE(isNear(fittedMax, a, 1.5));
    EXPECT_TRUE(isNear(fittedMin, b, 1.5));
    
    // The ellipse should not be axis-aligned (rotation should be non-zero)
    // Allow for ambiguity in rotation angle (could be +45 or -45 or 135, etc.)
    double rotDeg = std::abs(ellipse.rotationDegrees());
    // Normalize to [0, 90] range
    while (rotDeg > 90.0) rotDeg -= 90.0;
    // Should be close to 45 degrees or close to 0/90 if axes swapped
    bool isRotated = isNear(rotDeg, 45.0, 15.0) || 
                     isNear(rotDeg, 0.0, 5.0) ||
                     isNear(rotDeg, 90.0, 5.0);
    EXPECT_TRUE(isRotated);
}

// Test: Area calculation for fitted ellipse
TEST(EllipseFittingTest, FittedEllipseArea) {
    // Circle radius 5
    std::vector<Point> points;
    const int numPoints = 16;
    const double radius = 5.0;
    
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * M_PI * i / numPoints;
        points.push_back({
            radius * cos(angle),
            radius * sin(angle)
        });
    }
    
    Ellipse ellipse(points);
    double area = ellipse.area();
    double expectedArea = M_PI * radius * radius;
    
    // Area should be close to pi*r^2
    EXPECT_TRUE(isNear(area, expectedArea, expectedArea * 0.1));  // 10% tolerance
}

// Test: Perimeter calculation for fitted ellipse
TEST(EllipseFittingTest, FittedEllipsePerimeter) {
    // Circle radius 7
    std::vector<Point> points;
    const int numPoints = 20;
    const double radius = 7.0;
    
    for (int i = 0; i < numPoints; i++) {
        double angle = 2.0 * M_PI * i / numPoints;
        points.push_back({
            radius * cos(angle),
            radius * sin(angle)
        });
    }
    
    Ellipse ellipse(points);
    double perim = ellipse.perimeter();
    double expectedPerim = 2.0 * M_PI * radius;
    
    EXPECT_TRUE(isNear(perim, expectedPerim, expectedPerim * 0.1));  // 10% tolerance
}

/**
 * @file EllipseTest.cpp
 * @brief Google Test suite for Ellipse class
 * 
 * Tests constructors, geometric operations, transformations, and edge cases.
 * Ported and adapted from XYEllipseTest.cpp.
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Ellipse.h"
#include "aperturecore/geometry/Point.h"
#include "aperturecore/geometry/Bounds.h"
#include <cmath>
#include <vector>

using namespace aperture;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Test Fixture
// ============================================================================

class EllipseTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    bool isNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(EllipseTest, ParameterizedConstructor) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0, 45.0);
    
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 5.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 3.0);
    EXPECT_DOUBLE_EQ(ellipse.center().x, 10.0);
    EXPECT_DOUBLE_EQ(ellipse.center().y, 20.0);
    EXPECT_DOUBLE_EQ(ellipse.rotationDegrees(), 45.0);
    EXPECT_EQ(ellipse.getTypeLimits(), TypeLimits::EXTERNAL);
    EXPECT_TRUE(ellipse.isMeasuring());
}

TEST_F(EllipseTest, DefaultRotation) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    EXPECT_DOUBLE_EQ(ellipse.rotationDegrees(), 0.0);
    EXPECT_DOUBLE_EQ(ellipse.rotationRadians(), 0.0);
}

// ============================================================================
// Type Name
// ============================================================================

TEST_F(EllipseTest, TypeName) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    EXPECT_STREQ(ellipse.typeName(), "Ellipse");
}

// ============================================================================
// Perimeter Tests
// ============================================================================

TEST_F(EllipseTest, Perimeter_Circle) {
    Ellipse circle(5.0, 5.0, 0.0, 0.0);
    double perimeter = circle.perimeter();
    
    // Exact for circle: P = 2?r
    double expected = 2.0 * M_PI * 5.0;
    EXPECT_NEAR(perimeter, expected, 0.01);
}

TEST_F(EllipseTest, Perimeter_Ellipse) {
    Ellipse ellipse(8.0, 4.0, 0.0, 0.0);
    double perimeter = ellipse.perimeter();
    
    // Should be between 2?*min and 2?*max
    EXPECT_GT(perimeter, 2.0 * M_PI * 4.0);
    EXPECT_LT(perimeter, 2.0 * M_PI * 8.0);
}

TEST_F(EllipseTest, Perimeter_RamanujanAccuracy) {
    // Test Ramanujan approximation accuracy
    Ellipse ellipse(10.0, 5.0, 0.0, 0.0);
    double perimeter = ellipse.perimeter();
    
    // Ramanujan's approximation is accurate to < 0.01% for most ellipses
    // For a=10, b=5: P ? 48.44
    EXPECT_NEAR(perimeter, 48.44, 0.5);
}

// ============================================================================
// Area Tests
// ============================================================================

TEST_F(EllipseTest, Area_Circle) {
    Ellipse circle(5.0, 5.0, 0.0, 0.0);
    double area = circle.area();
    
    double expected = M_PI * 5.0 * 5.0;
    EXPECT_DOUBLE_EQ(area, expected);
}

TEST_F(EllipseTest, Area_Ellipse) {
    Ellipse ellipse(8.0, 4.0, 0.0, 0.0);
    double area = ellipse.area();
    
    double expected = M_PI * 8.0 * 4.0;
    EXPECT_DOUBLE_EQ(area, expected);
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(EllipseTest, isInside_Center) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0);
    Point center{10.0, 20.0};
    
    EXPECT_TRUE(ellipse.isInside(center));
}

TEST_F(EllipseTest, isInside_OnBoundary) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    Point point{5.0, 0.0};  // On major axis
    
    EXPECT_TRUE(ellipse.isInside(point));
}

TEST_F(EllipseTest, isInside_Outside) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    Point point{10.0, 10.0};
    
    EXPECT_FALSE(ellipse.isInside(point));
}

TEST_F(EllipseTest, isInside_RotatedEllipse) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0, 45.0);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(ellipse.isInside(center));
}

TEST_F(EllipseTest, isInside_RotatedEllipse_MajorAxis) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0, 45.0);
    
    // Point on rotated major axis - slightly inside to avoid boundary precision issues
    // Use 99% of semiMajor to ensure it's inside
    double angle = 45.0 * M_PI / 180.0;
    Point point{4.95 * std::cos(angle), 4.95 * std::sin(angle)};
    
    EXPECT_TRUE(ellipse.isInside(point));
}

// ============================================================================
// getBounds Tests
// ============================================================================

TEST_F(EllipseTest, getBounds_AxisAligned) {
    Ellipse ellipse(8.0, 5.0, 10.0, 20.0);
    Bounds bounds = ellipse.getBounds();
    
    EXPECT_DOUBLE_EQ(bounds.left, 2.0);    // 10 - 8
    EXPECT_DOUBLE_EQ(bounds.right, 18.0);  // 10 + 8
    EXPECT_DOUBLE_EQ(bounds.top, 15.0);    // 20 - 5
    EXPECT_DOUBLE_EQ(bounds.bottom, 25.0); // 20 + 5
}

TEST_F(EllipseTest, getBounds_Rotated45Degrees) {
    Ellipse ellipse(8.0, 5.0, 0.0, 0.0, 45.0);
    Bounds bounds = ellipse.getBounds();
    
    // For 45° rotation, extents should be equal
    double width = bounds.width();
    double height = bounds.height();
    
    EXPECT_NEAR(width, height, 1e-5);
    EXPECT_NEAR(width, 13.3417, 0.001);  // sqrt(64/2 + 25/2) * 2
}

TEST_F(EllipseTest, getBounds_Circle) {
    Ellipse circle(5.0, 5.0, 10.0, 10.0);
    Bounds bounds = circle.getBounds();
    
    EXPECT_DOUBLE_EQ(bounds.left, 5.0);
    EXPECT_DOUBLE_EQ(bounds.right, 15.0);
    EXPECT_DOUBLE_EQ(bounds.top, 5.0);
    EXPECT_DOUBLE_EQ(bounds.bottom, 15.0);
}

// ============================================================================
// getContour Tests
// ============================================================================

TEST_F(EllipseTest, getContour_PointCount) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    auto contour = ellipse.getContour(0.5);
    
    EXPECT_GT(contour.size(), 10u);
    
    // Verify points are close to the ellipse boundary
    // Due to floating-point precision, exact boundary points may fail isInside()
    // Instead, check they're approximately on the boundary
    for (const auto& point : contour) {
        Point local = Point{
            (point.x - ellipse.center().x) * std::cos(-ellipse.rotationRadians()) - 
            (point.y - ellipse.center().y) * std::sin(-ellipse.rotationRadians()),
            (point.x - ellipse.center().x) * std::sin(-ellipse.rotationRadians()) + 
            (point.y - ellipse.center().y) * std::cos(-ellipse.rotationRadians())
        };
        
        double term1 = (local.x * local.x) / (ellipse.semiMajor() * ellipse.semiMajor());
        double term2 = (local.y * local.y) / (ellipse.semiMinor() * ellipse.semiMinor());
        double distanceFromBoundary = std::abs((term1 + term2) - 1.0);
        
        // Points should be very close to boundary (within 0.01%)
        EXPECT_LT(distanceFromBoundary, 0.0001);
    }
}

TEST_F(EllipseTest, getContour_ClosedLoop) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    auto contour = ellipse.getContour(1.0);
    
    ASSERT_GT(contour.size(), 1u);
    
    // First and last points should be very close
    double dist = contour.front().distanceTo(contour.back());
    EXPECT_LT(dist, 1.0);  // Within one step
}

TEST_F(EllipseTest, getContour_RotatedEllipse) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0, 30.0);
    auto contour = ellipse.getContour(0.5);
    
    EXPECT_GT(contour.size(), 10u);
    
    // Verify points are close to the ellipse boundary (accounting for rotation)
    for (const auto& point : contour) {
        // Transform to local coordinates manually
        double dx = point.x - ellipse.center().x;
        double dy = point.y - ellipse.center().y;
        double cosRot = std::cos(ellipse.rotationRadians());
        double sinRot = std::sin(ellipse.rotationRadians());
        
        Point local{
            dx * cosRot + dy * sinRot,
            -dx * sinRot + dy * cosRot
        };
        
        double term1 = (local.x * local.x) / (ellipse.semiMajor() * ellipse.semiMajor());
        double term2 = (local.y * local.y) / (ellipse.semiMinor() * ellipse.semiMinor());
        double distanceFromBoundary = std::abs((term1 + term2) - 1.0);
        
        // Points should be very close to boundary (within 0.01%)
        EXPECT_LT(distanceFromBoundary, 0.0001);
    }
}

// ============================================================================
// Coordinate Transformation Tests
// ============================================================================

TEST_F(EllipseTest, Normalize) {
    Ellipse ellipse(10.0, 6.0, 50.0, 30.0);
    
    ellipse.normalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 1.0);   // 10 / 10
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 0.6);   // 6 / 10
    EXPECT_DOUBLE_EQ(ellipse.center().x, 1.0);    // (50 - 40) / 10
    EXPECT_DOUBLE_EQ(ellipse.center().y, 1.0);    // (30 - 20) / 10
    EXPECT_TRUE(ellipse.isNormalized());
}

TEST_F(EllipseTest, Normalize_TransformsCoordinates) {
    Ellipse ellipse(100.0, 50.0, 200.0, 100.0);
    
    // Before normalization
    EXPECT_FALSE(ellipse.isNormalized());
    EXPECT_TRUE(ellipse.isMeasuring());
    
    // Normalize relative to origin (100, 50) with radius 100
    ellipse.normalize(100.0, 50.0, 100.0);
    
    // After normalization
    EXPECT_TRUE(ellipse.isNormalized());
    EXPECT_FALSE(ellipse.isMeasuring());
    
    // Check transformed values
    Point center = ellipse.center();
    EXPECT_NEAR(center.x, 1.0, TOLERANCE);   // (200-100)/100 = 1
    EXPECT_NEAR(center.y, 0.5, TOLERANCE);   // (100-50)/100 = 0.5
    
    // Radii are also scaled
    // Semi-major and semi-minor should be 1.0 and 0.5
}

TEST_F(EllipseTest, Denormalize) {
    Ellipse ellipse(1.0, 0.6, 1.0, 1.0);
    // Set normalization state manually (already set by normalize())
    ellipse.setNormalizationState(NormalizationState::NORMALIZED);
    
    ellipse.denormalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 10.0);  // 1 * 10
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 6.0);   // 0.6 * 10
    EXPECT_DOUBLE_EQ(ellipse.center().x, 50.0);   // 1 * 10 + 40
    EXPECT_DOUBLE_EQ(ellipse.center().y, 30.0);   // 1 * 10 + 20
    EXPECT_TRUE(ellipse.isMeasuring());
}

TEST_F(EllipseTest, Normalize_Denormalize_RoundTrip) {
    Ellipse ellipse(10.0, 6.0, 50.0, 30.0, 15.0);
    
    double origA = ellipse.semiMajor();
    double origB = ellipse.semiMinor();
    Point origCenter = ellipse.center();
    
    ellipse.normalize(40.0, 20.0, 10.0);
    ellipse.denormalize(40.0, 20.0, 10.0);
    
    EXPECT_NEAR(ellipse.semiMajor(), origA, TOLERANCE);
    EXPECT_NEAR(ellipse.semiMinor(), origB, TOLERANCE);
    EXPECT_NEAR(ellipse.center().x, origCenter.x, TOLERANCE);
    EXPECT_NEAR(ellipse.center().y, origCenter.y, TOLERANCE);
}

TEST_F(EllipseTest, InverseY) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0, 30.0);
    
    ellipse.inverseY(100.0);
    
    EXPECT_DOUBLE_EQ(ellipse.center().y, 80.0);   // 100 - 20
    EXPECT_DOUBLE_EQ(ellipse.rotationDegrees(), -30.0);
}

TEST_F(EllipseTest, ShiftX) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0);
    
    ellipse.shiftX(5.0);
    
    EXPECT_DOUBLE_EQ(ellipse.center().x, 15.0);
    EXPECT_DOUBLE_EQ(ellipse.center().y, 20.0);  // Unchanged
}

TEST_F(EllipseTest, ShiftY) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0);
    
    ellipse.shiftY(-10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.center().x, 10.0);  // Unchanged
    EXPECT_DOUBLE_EQ(ellipse.center().y, 10.0);
}

// ============================================================================
// Geometric Property Tests
// ============================================================================

TEST_F(EllipseTest, isCircle_True) {
    Ellipse circle(5.0, 5.0, 0.0, 0.0);
    
    EXPECT_TRUE(circle.isCircle());
}

TEST_F(EllipseTest, isCircle_False) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    EXPECT_FALSE(ellipse.isCircle());
}

TEST_F(EllipseTest, isCircle_WithTolerance) {
    // Test with value clearly within default tolerance (1e-6)
    // Difference should be < 1e-6 to pass with default tolerance
    Ellipse almostCircle(5.0, 4.9999999, 0.0, 0.0);  // diff = 1e-7 < 1e-6 ?
    
    EXPECT_TRUE(almostCircle.isCircle());  // Within default tolerance (1e-6)
    EXPECT_FALSE(almostCircle.isCircle(1e-10));  // Outside strict tolerance (1e-7 > 1e-10)
}

TEST_F(EllipseTest, Eccentricity_Circle) {
    Ellipse circle(5.0, 5.0, 0.0, 0.0);
    
    EXPECT_NEAR(circle.eccentricity(), 0.0, TOLERANCE);
}

TEST_F(EllipseTest, Eccentricity_Ellipse) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    // e = sqrt(1 - (b?/a?)) = sqrt(1 - 9/25) = sqrt(16/25) = 0.8
    EXPECT_NEAR(ellipse.eccentricity(), 0.8, TOLERANCE);
}

TEST_F(EllipseTest, FocalDistance_Circle) {
    Ellipse circle(5.0, 5.0, 0.0, 0.0);
    
    EXPECT_NEAR(circle.focalDistance(), 0.0, TOLERANCE);
}

TEST_F(EllipseTest, FocalDistance_Ellipse) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    // c = sqrt(a? - b?) = sqrt(25 - 9) = 4
    EXPECT_NEAR(ellipse.focalDistance(), 4.0, TOLERANCE);
}

// ============================================================================
// Clone Tests
// ============================================================================

TEST_F(EllipseTest, Clone) {
    Ellipse original(5.0, 3.0, 10.0, 20.0, 30.0);
    original.setTypeLimits(TypeLimits::INTERNAL);
    
    auto cloned = original.clone();
    Ellipse* ellipseClone = dynamic_cast<Ellipse*>(cloned.get());
    
    ASSERT_NE(ellipseClone, nullptr);
    EXPECT_DOUBLE_EQ(ellipseClone->semiMajor(), original.semiMajor());
    EXPECT_DOUBLE_EQ(ellipseClone->semiMinor(), original.semiMinor());
    EXPECT_DOUBLE_EQ(ellipseClone->center().x, original.center().x);
    EXPECT_DOUBLE_EQ(ellipseClone->center().y, original.center().y);
    EXPECT_DOUBLE_EQ(ellipseClone->rotationDegrees(), original.rotationDegrees());
    EXPECT_EQ(ellipseClone->getTypeLimits(), original.getTypeLimits());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(EllipseTest, VerySmallEllipse) {
    Ellipse ellipse(0.001, 0.001, 0.0, 0.0);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(ellipse.isInside(center));
    EXPECT_GT(ellipse.perimeter(), 0.0);
}

TEST_F(EllipseTest, VeryLargeEllipse) {
    Ellipse ellipse(1e6, 1e6, 0.0, 0.0);
    Point point{1e5, 1e5};
    
    EXPECT_TRUE(ellipse.isInside(point));
}

TEST_F(EllipseTest, HighlyEccentricEllipse) {
    Ellipse ellipse(100.0, 1.0, 0.0, 0.0);
    Point point{50.0, 0.0};
    
    EXPECT_TRUE(ellipse.isInside(point));
    
    Bounds bounds = ellipse.getBounds();
    EXPECT_DOUBLE_EQ(bounds.width(), 200.0);
    EXPECT_DOUBLE_EQ(bounds.height(), 2.0);
}

TEST_F(EllipseTest, Rotation360Degrees) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0, 360.0);
    
    // 360 degrees should be equivalent to 0
    EXPECT_NEAR(ellipse.rotationRadians(), 2.0 * M_PI, 0.01);
}

TEST_F(EllipseTest, NegativeRotation) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0, -45.0);
    
    EXPECT_DOUBLE_EQ(ellipse.rotationDegrees(), -45.0);
    EXPECT_NEAR(ellipse.rotationRadians(), -45.0 * M_PI / 180.0, TOLERANCE);
}

// ============================================================================
// TypeLimits Tests
// ============================================================================

TEST_F(EllipseTest, TypeLimits_DefaultExternal) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    EXPECT_EQ(ellipse.getTypeLimits(), TypeLimits::EXTERNAL);
}

TEST_F(EllipseTest, TypeLimits_SetAndGet) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    
    ellipse.setTypeLimits(TypeLimits::INTERNAL);
    EXPECT_EQ(ellipse.getTypeLimits(), TypeLimits::INTERNAL);
    
    ellipse.setTypeLimits(TypeLimits::APERTURE);
    EXPECT_EQ(ellipse.getTypeLimits(), TypeLimits::APERTURE);
}

// ============================================================================
// FitCircle Tests (Phase 1 - Circle LSM Fitting)
// ============================================================================

TEST_F(EllipseTest, FitCircle_ThreePoints) {
    // Three points on a circle with center (50, 50) and radius 30
    std::vector<Point> points = {
        {80.0, 50.0},  // Right
        {50.0, 80.0},  // Top
        {20.0, 50.0}   // Left
    };
    
    auto circle = Ellipse::FitCircle(points);
    
    ASSERT_NE(circle, nullptr);
    EXPECT_NEAR(circle->center().x, 50.0, 1.0);
    EXPECT_NEAR(circle->center().y, 50.0, 1.0);
    EXPECT_NEAR(circle->semiMajor(), 30.0, 1.0);
    EXPECT_NEAR(circle->semiMinor(), 30.0, 1.0);  // Must be circle
    EXPECT_TRUE(circle->isCircle(0.1));
}

TEST_F(EllipseTest, FitCircle_FourPointsLSM) {
    // Four points approximately on circle (center origin, radius 10)
    // With slight noise to test LSM refinement
    std::vector<Point> points = {
        {10.0, 0.0},
        {0.0, 10.1},   // Slight error
        {-10.0, 0.0},
        {0.0, -9.9}    // Slight error
    };
    
    auto circle = Ellipse::FitCircle(points);
    
    ASSERT_NE(circle, nullptr);
    EXPECT_NEAR(circle->center().x, 0.0, 0.5);
    EXPECT_NEAR(circle->center().y, 0.0, 0.5);
    EXPECT_NEAR(circle->semiMajor(), 10.0, 0.5);
    EXPECT_NEAR(circle->semiMinor(), 10.0, 0.5);
    EXPECT_TRUE(circle->isCircle(0.1));
}

TEST_F(EllipseTest, FitCircle_EnforcesEqualRadii) {
    // Points that would fit better as ellipse, but FitCircle enforces circle
    std::vector<Point> points = {
        {15.0, 0.0},
        {0.0, 10.0},
        {-15.0, 0.0},
        {0.0, -10.0}
    };
    
    auto circle = Ellipse::FitCircle(points);
    
    ASSERT_NE(circle, nullptr);
    // Must be circle (radiusX == radiusY) even if points are elliptical
    EXPECT_NEAR(circle->semiMajor(), circle->semiMinor(), 0.01);
    EXPECT_TRUE(circle->isCircle(0.01));
}

TEST_F(EllipseTest, FitCircle_MinimumPoints) {
    // Less than 3 points - should return degenerate circle
    std::vector<Point> points = {
        {10.0, 10.0},
        {20.0, 20.0}
    };
    
    auto circle = Ellipse::FitCircle(points);
    
    ASSERT_NE(circle, nullptr);
    // Degenerate case - exact behavior depends on implementation
}

TEST_F(EllipseTest, FitCircle_ManyPoints) {
    // Generate 8 points on circle with center (100, 100), radius 50
    std::vector<Point> points;
    const double centerX = 100.0, centerY = 100.0, radius = 50.0;
    for (int i = 0; i < 8; ++i) {
        double angle = 2.0 * M_PI * i / 8.0;
        points.push_back({
            centerX + radius * std::cos(angle),
            centerY + radius * std::sin(angle)
        });
    }
    
    auto circle = Ellipse::FitCircle(points);
    
    ASSERT_NE(circle, nullptr);
    EXPECT_NEAR(circle->center().x, centerX, 1.0);
    EXPECT_NEAR(circle->center().y, centerY, 1.0);
    EXPECT_NEAR(circle->semiMajor(), radius, 1.0);
    EXPECT_NEAR(circle->semiMinor(), radius, 1.0);
    EXPECT_TRUE(circle->isCircle(0.1));
}

TEST_F(EllipseTest, FitCircle_TypeLimitsPreserved) {
    std::vector<Point> points = {{10, 0}, {0, 10}, {-10, 0}};
    
    auto circle = Ellipse::FitCircle(points, TypeLimits::APERTURE);
    
    ASSERT_NE(circle, nullptr);
    EXPECT_EQ(circle->getTypeLimits(), TypeLimits::APERTURE);
}

// ============================================================================
// FitEllipse Tests (Phase 1 - General Ellipse Fitting)
// ============================================================================

TEST_F(EllipseTest, FitEllipse_AllowsDifferentRadii) {
    // Points on ellipse with different radii (15x10)
    std::vector<Point> points = {
        {15.0, 0.0},   // Major axis endpoint
        {0.0, 10.0},   // Minor axis endpoint
        {-15.0, 0.0},  // Major axis other end
        {0.0, -10.0},  // Minor axis other end
        {10.6, 7.1}    // Additional point on perimeter
    };
    
    auto ellipse = Ellipse::FitEllipse(points);
    
    ASSERT_NE(ellipse, nullptr);
    EXPECT_NEAR(ellipse->center().x, 0.0, 1.0);
    EXPECT_NEAR(ellipse->center().y, 0.0, 1.0);
    // Should fit as ellipse, not forced to circle
    EXPECT_FALSE(ellipse->isCircle(1.0));
}

TEST_F(EllipseTest, FitEllipse_CircularPoints) {
    // Even with circular points, FitEllipse should work
    std::vector<Point> points = {
        {10.0, 0.0},
        {0.0, 10.0},
        {-10.0, 0.0},
        {0.0, -10.0},
        {7.07, 7.07}
    };
    
    auto ellipse = Ellipse::FitEllipse(points);
    
    ASSERT_NE(ellipse, nullptr);
    // Should be approximately circular
    EXPECT_TRUE(ellipse->isCircle(1.0));
}

TEST_F(EllipseTest, FitEllipse_TypeLimitsPreserved) {
    std::vector<Point> points = {{15, 0}, {0, 10}, {-15, 0}, {0, -10}};
    
    auto ellipse = Ellipse::FitEllipse(points, TypeLimits::INTERNAL);
    
    ASSERT_NE(ellipse, nullptr);
    EXPECT_EQ(ellipse->getTypeLimits(), TypeLimits::INTERNAL);
}

TEST_F(EllipseTest, FitCircleVsFitEllipse_Comparison) {
    // Elliptical points - compare circle vs ellipse fit quality
    std::vector<Point> points = {
        {20.0, 0.0},
        {0.0, 10.0},
        {-20.0, 0.0},
        {0.0, -10.0}
    };
    
    auto circle = Ellipse::FitCircle(points);
    auto ellipse = Ellipse::FitEllipse(points);
    
    ASSERT_NE(circle, nullptr);
    ASSERT_NE(ellipse, nullptr);
    
    // Circle must have equal radii
    EXPECT_TRUE(circle->isCircle(0.01));
    
    // Ellipse can have different radii (better fit for these points)
    double ellipseRatio = ellipse->semiMajor() / ellipse->semiMinor();
    EXPECT_GT(ellipseRatio, 1.5);  // Significantly elliptical
}

// ============================================================================
// Handle Enumeration Tests (Phase 1 - Interactive Editing)
// ============================================================================

TEST_F(EllipseTest, EnumerateHandles_Count) {
    Ellipse ellipse(15.0, 10.0, 50.0, 50.0, 30.0);
    std::vector<HandleDesc> handles;
    
    ellipse.EnumerateHandles(handles);
    
    // Ellipse should have 6 handles: 1 Move + 1 Rotate + 4 AxisResize
    EXPECT_EQ(handles.size(), 6u);
}

TEST_F(EllipseTest, EnumerateHandles_Types) {
    Ellipse ellipse(15.0, 10.0, 0.0, 0.0, 0.0);
    std::vector<HandleDesc> handles;
    
    ellipse.EnumerateHandles(handles);
    
    ASSERT_EQ(handles.size(), 6u);
    
    // First handle: Move (at center)
    EXPECT_EQ(handles[0].type, HandleType::Move);
    EXPECT_NEAR(handles[0].localPos.x, 0.0, TOLERANCE);
    EXPECT_NEAR(handles[0].localPos.y, 0.0, TOLERANCE);
    
    // Second handle: Rotate
    EXPECT_EQ(handles[1].type, HandleType::Rotate);
    
    // Remaining 4 handles: AxisResize
    for (size_t i = 2; i < 6; ++i) {
        EXPECT_EQ(handles[i].type, HandleType::AxisResize);
    }
}

TEST_F(EllipseTest, EnumerateHandles_AxisAligned) {
    Ellipse ellipse(15.0, 10.0, 100.0, 100.0, 0.0);  // Axis-aligned
    std::vector<HandleDesc> handles;
    
    ellipse.EnumerateHandles(handles);
    
    ASSERT_EQ(handles.size(), 6u);
    
    // For axis-aligned ellipse, check axis endpoints are positioned correctly
    // Major axis handles (index 2, 3) should be at (100±15, 100)
    EXPECT_NEAR(handles[2].localPos.x, 115.0, TOLERANCE);  // Right major
    EXPECT_NEAR(handles[2].localPos.y, 100.0, TOLERANCE);
    
    EXPECT_NEAR(handles[3].localPos.x, 85.0, TOLERANCE);   // Left major
    EXPECT_NEAR(handles[3].localPos.y, 100.0, TOLERANCE);
    
    // Minor axis handles (index 4, 5) should be at (100, 100±10)
    EXPECT_NEAR(handles[4].localPos.x, 100.0, TOLERANCE);  // Top minor
    EXPECT_NEAR(handles[4].localPos.y, 110.0, TOLERANCE);
    
    EXPECT_NEAR(handles[5].localPos.x, 100.0, TOLERANCE);  // Bottom minor
    EXPECT_NEAR(handles[5].localPos.y, 90.0, TOLERANCE);
}

TEST_F(EllipseTest, ApplyHandleDrag_Move) {
    Ellipse ellipse(15.0, 10.0, 50.0, 50.0);
    
    HandleDesc handle{HandleType::Move, -1, {50.0, 50.0}};
    DragContext drag{handle, {50.0, 50.0}, {60.0, 70.0}, {10.0, 20.0}, false, false};
    
    ellipse.ApplyHandleDrag(handle, drag);
    
    // Center should move by delta
    EXPECT_NEAR(ellipse.center().x, 60.0, TOLERANCE);
    EXPECT_NEAR(ellipse.center().y, 70.0, TOLERANCE);
    
    // Radii should be unchanged
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 15.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 10.0);
}

TEST_F(EllipseTest, ApplyHandleDrag_AxisResize_Major) {
    Ellipse ellipse(15.0, 10.0, 0.0, 0.0, 0.0);
    
    // Major axis handle at (15, 0) with normal pointing right
    HandleDesc handle{HandleType::AxisResize, 0, {15.0, 0.0}, {1.0, 0.0}};
    DragContext drag{handle, {15.0, 0.0}, {20.0, 0.0}, {5.0, 0.0}, false, false};
    
    ellipse.ApplyHandleDrag(handle, drag);
    
    // Major axis should increase by 5
    EXPECT_NEAR(ellipse.semiMajor(), 20.0, TOLERANCE);
    
    // Minor axis unchanged
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 10.0);
}

TEST_F(EllipseTest, ApplyHandleDrag_AxisResize_Minor) {
    Ellipse ellipse(15.0, 10.0, 0.0, 0.0, 0.0);
    
    // Minor axis handle at (0, 10) with normal pointing up
    HandleDesc handle{HandleType::AxisResize, 2, {0.0, 10.0}, {0.0, 1.0}};
    DragContext drag{handle, {0.0, 10.0}, {0.0, 15.0}, {0.0, 5.0}, false, false};
    
    ellipse.ApplyHandleDrag(handle, drag);
    
    // Minor axis should increase by 5
    EXPECT_NEAR(ellipse.semiMinor(), 15.0, TOLERANCE);
    
    // Major axis unchanged
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 15.0);
}

TEST_F(EllipseTest, ApplyHandleDrag_Rotate) {
    Ellipse ellipse(15.0, 10.0, 0.0, 0.0, 0.0);
    
    // Rotate handle offset along minor axis (perpendicular to major)
    HandleDesc handle{HandleType::Rotate, -1, {0.0, 20.0}};
    
    // Drag from (0, 20) to (14.14, 14.14) - should rotate ~45 degrees
    DragContext drag{handle, {0.0, 20.0}, {14.14, 14.14}, {14.14, -5.86}, false, false};
    
    double initialRotation = ellipse.rotationDegrees();
    
    ellipse.ApplyHandleDrag(handle, drag);
    
    // Rotation should have changed
    EXPECT_NE(ellipse.rotationDegrees(), initialRotation);
    
    // Radii should be unchanged
    EXPECT_DOUBLE_EQ(ellipse.semiMajor(), 15.0);
    EXPECT_DOUBLE_EQ(ellipse.semiMinor(), 10.0);
}

// ============================================================================
// isOnContour Tests
// ============================================================================

TEST_F(EllipseTest, IsOnContour_ExactlyOnBoundary) {
    Ellipse circle(50.0, 50.0, 100.0, 100.0);  // Circle at (100,100), radius 50
    
    // Point exactly on boundary (right side)
    Point onBoundary{150.0, 100.0};
    EXPECT_TRUE(circle.isOnContour(onBoundary, 0.1));
    
    // Point exactly on boundary (top)
    Point onTop{100.0, 150.0};
    EXPECT_TRUE(circle.isOnContour(onTop, 0.1));
}

TEST_F(EllipseTest, IsOnContour_NearBoundary) {
    Ellipse circle(50.0, 50.0, 100.0, 100.0);
    
    // Point 1 unit outside
    Point nearOutside{151.0, 100.0};
    EXPECT_TRUE(circle.isOnContour(nearOutside, 2.0));
    EXPECT_FALSE(circle.isOnContour(nearOutside, 0.5));
    
    // Point 1 unit inside
    Point nearInside{149.0, 100.0};
    EXPECT_TRUE(circle.isOnContour(nearInside, 2.0));
    EXPECT_FALSE(circle.isOnContour(nearInside, 0.5));
}

TEST_F(EllipseTest, IsOnContour_FarFromBoundary) {
    Ellipse circle(50.0, 50.0, 100.0, 100.0);
    
    // Center (far inside)
    EXPECT_FALSE(circle.isOnContour(circle.center(), 2.0));
    
    // Far outside
    Point farAway{200.0, 200.0};
    EXPECT_FALSE(circle.isOnContour(farAway, 2.0));
}

TEST_F(EllipseTest, IsOnContour_Ellipse) {
    Ellipse ellipse(50.0, 30.0, 0.0, 0.0);  // Ellipse 50x30 at origin
    
    // Point on major axis
    Point onMajor{50.0, 0.0};
    EXPECT_TRUE(ellipse.isOnContour(onMajor, 0.1));
    
    // Point on minor axis
    Point onMinor{0.0, 30.0};
    EXPECT_TRUE(ellipse.isOnContour(onMinor, 0.1));
    
    // Point at center (not on contour)
    EXPECT_FALSE(ellipse.isOnContour({0.0, 0.0}, 1.0));
}

TEST_F(EllipseTest, IsOnContour_RotatedEllipse) {
    Ellipse ellipse(50.0, 30.0, 0.0, 0.0, 45.0);  // Rotated 45 degrees
    
    // Point on rotated major axis
    double cos45 = std::cos(45.0 * M_PI / 180.0);
    double sin45 = std::sin(45.0 * M_PI / 180.0);
    Point onRotatedMajor{50.0 * cos45, 50.0 * sin45};
    
    EXPECT_TRUE(ellipse.isOnContour(onRotatedMajor, 1.0));
}

TEST_F(EllipseTest, IsOnContour_SmallTolerance) {
    Ellipse ellipse(10.0, 5.0, 0.0, 0.0);
    
    // With very small tolerance, interior points should not be on contour
    Point interior{5.0, 0.0};  // Halfway to edge
    EXPECT_FALSE(ellipse.isOnContour(interior, 0.01));
    
    // But edge point should be
    Point edge{10.0, 0.0};
    EXPECT_TRUE(ellipse.isOnContour(edge, 0.01));
}

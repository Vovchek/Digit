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
    
    // Point on rotated major axis
    double angle = 45.0 * M_PI / 180.0;
    Point point{5.0 * std::cos(angle), 5.0 * std::sin(angle)};
    
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
    
    EXPECT_GT(contour.size(), 10);
    
    // Verify all points are on or inside the ellipse
    for (const auto& point : contour) {
        EXPECT_TRUE(ellipse.isInside(point));
    }
}

TEST_F(EllipseTest, getContour_ClosedLoop) {
    Ellipse ellipse(5.0, 3.0, 0.0, 0.0);
    auto contour = ellipse.getContour(1.0);
    
    ASSERT_GT(contour.size(), 1);
    
    // First and last points should be very close
    double dist = contour.front().distanceTo(contour.back());
    EXPECT_LT(dist, 1.0);  // Within one step
}

TEST_F(EllipseTest, getContour_RotatedEllipse) {
    Ellipse ellipse(5.0, 3.0, 10.0, 20.0, 30.0);
    auto contour = ellipse.getContour(0.5);
    
    EXPECT_GT(contour.size(), 10);
    
    // All points should be inside
    for (const auto& point : contour) {
        EXPECT_TRUE(ellipse.isInside(point));
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

TEST_F(EllipseTest, Denormalize) {
    Ellipse ellipse(1.0, 0.6, 1.0, 1.0);
    ellipse.setCoordinateSystem(CoordinateSystem::NORMALIZED);
    
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
    Ellipse almostCircle(5.0, 4.999999, 0.0, 0.0);
    
    EXPECT_TRUE(almostCircle.isCircle());
    EXPECT_FALSE(almostCircle.isCircle(1e-10));
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

/**
 * @file RectangleTest.cpp
 * @brief Google Test suite for Rectangle class
 * 
 * Tests constructors, geometric operations, transformations, and edge cases.
 * Ported and adapted from XYRectTest.cpp.
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Rectangle.h"
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

class RectangleTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    bool isNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(RectangleTest, ParameterizedConstructor) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0, 45.0);
    
    EXPECT_DOUBLE_EQ(rect.width(), 10.0);
    EXPECT_DOUBLE_EQ(rect.height(), 5.0);
    EXPECT_DOUBLE_EQ(rect.center().x, 20.0);
    EXPECT_DOUBLE_EQ(rect.center().y, 30.0);
    EXPECT_DOUBLE_EQ(rect.rotationDegrees(), 45.0);
    EXPECT_EQ(rect.getTypeLimits(), TypeLimits::EXTERNAL);
    EXPECT_TRUE(rect.isMeasuring());
}

TEST_F(RectangleTest, DefaultRotation) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    
    EXPECT_DOUBLE_EQ(rect.rotationDegrees(), 0.0);
    EXPECT_DOUBLE_EQ(rect.rotationRadians(), 0.0);
}

TEST_F(RectangleTest, RotationCacheUpdated) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 45.0);
    
    double expectedRad = 45.0 * M_PI / 180.0;
    EXPECT_NEAR(rect.rotationRadians(), expectedRad, TOLERANCE);
}

// ============================================================================
// Type Name
// ============================================================================

TEST_F(RectangleTest, TypeName) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    
    EXPECT_STREQ(rect.typeName(), "Rectangle");
}

// ============================================================================
// Perimeter Tests
// ============================================================================

TEST_F(RectangleTest, Perimeter_Square) {
    Rectangle square(5.0, 5.0, 0.0, 0.0);
    double perimeter = square.perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 20.0);  // 4 * 5
}

TEST_F(RectangleTest, Perimeter_Rectangle) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    double perimeter = rect.perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 30.0);  // 2 * (10 + 5)
}

TEST_F(RectangleTest, Perimeter_RotatedRectangle) {
    // Rotation shouldn't affect perimeter
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 45.0);
    double perimeter = rect.perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 30.0);
}

// ============================================================================
// Area Tests
// ============================================================================

TEST_F(RectangleTest, Area_Square) {
    Rectangle square(5.0, 5.0, 0.0, 0.0);
    double area = square.area();
    
    EXPECT_DOUBLE_EQ(area, 25.0);
}

TEST_F(RectangleTest, Area_Rectangle) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    double area = rect.area();
    
    EXPECT_DOUBLE_EQ(area, 50.0);
}

TEST_F(RectangleTest, Area_RotatedRectangle) {
    // Rotation shouldn't affect area
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 45.0);
    double area = rect.area();
    
    EXPECT_DOUBLE_EQ(area, 50.0);
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(RectangleTest, isInside_Center) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0);
    Point center{20.0, 30.0};
    
    EXPECT_TRUE(rect.isInside(center));
}

TEST_F(RectangleTest, isInside_Corner) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    
    // Corners at (±5, ±2.5) for axis-aligned rectangle
    // Test slightly inside to avoid boundary precision issues
    Point corner{4.99, 2.49};
    
    EXPECT_TRUE(rect.isInside(corner));
}

TEST_F(RectangleTest, isInside_OnEdge) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    
    // Test slightly inside edge to avoid boundary precision
    Point edge{4.99, 0.0};  // Near right edge
    
    EXPECT_TRUE(rect.isInside(edge));
}

TEST_F(RectangleTest, isInside_Outside) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    Point outside{20.0, 20.0};
    
    EXPECT_FALSE(rect.isInside(outside));
}

TEST_F(RectangleTest, isInside_RotatedRectangle) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 45.0);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(rect.isInside(center));
}

TEST_F(RectangleTest, isInside_RotatedRectangle_LocalAxisPoint) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 45.0);
    
    // Point along rotated local X axis (slightly inside)
    double angle = 45.0 * M_PI / 180.0;
    Point point{4.9 * std::cos(angle), 4.9 * std::sin(angle)};
    
    EXPECT_TRUE(rect.isInside(point));
}

// ============================================================================
// getBounds Tests
// ============================================================================

TEST_F(RectangleTest, getBounds_AxisAligned) {
    Rectangle rect(10.0, 6.0, 20.0, 30.0);
    Bounds bounds = rect.getBounds();
    
    EXPECT_DOUBLE_EQ(bounds.left, 15.0);   // 20 - 5
    EXPECT_DOUBLE_EQ(bounds.right, 25.0);  // 20 + 5
    EXPECT_DOUBLE_EQ(bounds.top, 27.0);    // 30 - 3
    EXPECT_DOUBLE_EQ(bounds.bottom, 33.0); // 30 + 3
}

TEST_F(RectangleTest, getBounds_Rotated45Degrees) {
    Rectangle rect(10.0, 6.0, 0.0, 0.0, 45.0);
    Bounds bounds = rect.getBounds();
    
    // For 45° rotation, bounding box should be larger than original
    double width = bounds.width();
    double height = bounds.height();
    
    // At 45°, both dimensions contribute equally
    EXPECT_GT(width, 10.0);
    EXPECT_GT(height, 6.0);
    
    // Check symmetry around center
    EXPECT_NEAR(bounds.left, -width/2, TOLERANCE);
    EXPECT_NEAR(bounds.right, width/2, TOLERANCE);
}

TEST_F(RectangleTest, getBounds_Square) {
    Rectangle square(8.0, 8.0, 10.0, 10.0);
    Bounds bounds = square.getBounds();
    
    EXPECT_DOUBLE_EQ(bounds.left, 6.0);
    EXPECT_DOUBLE_EQ(bounds.right, 14.0);
    EXPECT_DOUBLE_EQ(bounds.top, 6.0);
    EXPECT_DOUBLE_EQ(bounds.bottom, 14.0);
}

// ============================================================================
// getContour Tests
// ============================================================================

TEST_F(RectangleTest, getContour_PointCount) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    auto contour = rect.getContour(0.5);
    
    EXPECT_GT(contour.size(), 4u);  // At least more than just corners
    
    // All contour points should be close to boundary
    for (const auto& point : contour) {
        // For axis-aligned rectangle, check if point is on any edge
        double dx = std::abs(point.x - rect.center().x);
        double dy = std::abs(point.y - rect.center().y);
        
        bool onVerticalEdge = isNear(dx, rect.width() / 2, 0.01);
        bool onHorizontalEdge = isNear(dy, rect.height() / 2, 0.01);
        
        EXPECT_TRUE(onVerticalEdge || onHorizontalEdge)
            << "Point (" << point.x << ", " << point.y << ") not on boundary";
    }
}

TEST_F(RectangleTest, getContour_ClosedLoop) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0);
    auto contour = rect.getContour(1.0);
    
    ASSERT_GT(contour.size(), 1u);
    
    // First and last points should be very close
    double dist = contour.front().distanceTo(contour.back());
    EXPECT_LT(dist, 2.0);  // Within a couple of steps
}

TEST_F(RectangleTest, getContour_RotatedRectangle) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0, 30.0);
    auto contour = rect.getContour(0.5);
    
    EXPECT_GT(contour.size(), 4u);
    
    // All points should be approximately at the same distance from center
    // as the rotated corners (within the rectangle bounds)
    for (const auto& point : contour) {
        double dist = point.distanceTo(rect.center());
        // Distance should be less than or equal to diagonal half-length
        double maxDist = std::sqrt(rect.width() * rect.width() + 
                                   rect.height() * rect.height()) / 2.0;
        EXPECT_LE(dist, maxDist + TOLERANCE);
    }
}

// ============================================================================
// Corners Tests
// ============================================================================

TEST_F(RectangleTest, Corners_AxisAligned) {
    Rectangle rect(10.0, 6.0, 0.0, 0.0);
    auto corners = rect.corners();
    
    EXPECT_EQ(corners.size(), 4u);
    
    // For axis-aligned rectangle centered at origin
    // Corners should be at (±5, ±3)
    EXPECT_NEAR(corners[0].x, -5.0, TOLERANCE);  // TL
    EXPECT_NEAR(corners[0].y, -3.0, TOLERANCE);
    
    EXPECT_NEAR(corners[1].x, 5.0, TOLERANCE);   // TR
    EXPECT_NEAR(corners[1].y, -3.0, TOLERANCE);
    
    EXPECT_NEAR(corners[2].x, 5.0, TOLERANCE);   // BR
    EXPECT_NEAR(corners[2].y, 3.0, TOLERANCE);
    
    EXPECT_NEAR(corners[3].x, -5.0, TOLERANCE);  // BL
    EXPECT_NEAR(corners[3].y, 3.0, TOLERANCE);
}

TEST_F(RectangleTest, Corners_AllInside) {
    Rectangle rect(10.0, 6.0, 0.0, 0.0);
    auto corners = rect.corners();
    
    // All corners should be inside (or on boundary of) the rectangle
    for (const auto& corner : corners) {
        EXPECT_TRUE(rect.isInside(corner));
    }
}

// ============================================================================
// Coordinate Transformation Tests
// ============================================================================

TEST_F(RectangleTest, Normalize) {
    Rectangle rect(20.0, 10.0, 50.0, 30.0);
    
    rect.normalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(rect.width(), 2.0);      // 20 / 10
    EXPECT_DOUBLE_EQ(rect.height(), 1.0);     // 10 / 10
    EXPECT_DOUBLE_EQ(rect.center().x, 1.0);   // (50 - 40) / 10
    EXPECT_DOUBLE_EQ(rect.center().y, 1.0);   // (30 - 20) / 10
    EXPECT_TRUE(rect.isNormalized());
}

TEST_F(RectangleTest, Denormalize) {
    Rectangle rect(2.0, 1.0, 1.0, 1.0);
    rect.setNormalizationState(NormalizationState::NORMALIZED);
    
    rect.denormalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(rect.width(), 20.0);     // 2 * 10
    EXPECT_DOUBLE_EQ(rect.height(), 10.0);    // 1 * 10
    EXPECT_DOUBLE_EQ(rect.center().x, 50.0);  // 1 * 10 + 40
    EXPECT_DOUBLE_EQ(rect.center().y, 30.0);  // 1 * 10 + 20
    EXPECT_TRUE(rect.isMeasuring());
}

TEST_F(RectangleTest, Normalize_Denormalize_RoundTrip) {
    Rectangle rect(20.0, 10.0, 50.0, 30.0, 15.0);
    
    double origW = rect.width();
    double origH = rect.height();
    Point origCenter = rect.center();
    
    rect.normalize(40.0, 20.0, 10.0);
    rect.denormalize(40.0, 20.0, 10.0);
    
    EXPECT_NEAR(rect.width(), origW, TOLERANCE);
    EXPECT_NEAR(rect.height(), origH, TOLERANCE);
    EXPECT_NEAR(rect.center().x, origCenter.x, TOLERANCE);
    EXPECT_NEAR(rect.center().y, origCenter.y, TOLERANCE);
}

TEST_F(RectangleTest, InverseY) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0, 30.0);
    
    rect.inverseY(100.0);
    
    EXPECT_DOUBLE_EQ(rect.center().y, 70.0);   // 100 - 30
    EXPECT_DOUBLE_EQ(rect.rotationDegrees(), -30.0);
}

TEST_F(RectangleTest, ShiftX) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0);
    
    rect.shiftX(5.0);
    
    EXPECT_DOUBLE_EQ(rect.center().x, 25.0);
    EXPECT_DOUBLE_EQ(rect.center().y, 30.0);  // Unchanged
}

TEST_F(RectangleTest, ShiftY) {
    Rectangle rect(10.0, 5.0, 20.0, 30.0);
    
    rect.shiftY(-10.0);
    
    EXPECT_DOUBLE_EQ(rect.center().x, 20.0);  // Unchanged
    EXPECT_DOUBLE_EQ(rect.center().y, 20.0);
}

// ============================================================================
// Geometric Property Tests
// ============================================================================

TEST_F(RectangleTest, isSquare_True) {
    Rectangle square(5.0, 5.0, 0.0, 0.0);
    
    EXPECT_TRUE(square.isSquare());
}

TEST_F(RectangleTest, isSquare_False) {
    Rectangle rect(5.0, 3.0, 0.0, 0.0);
    
    EXPECT_FALSE(rect.isSquare());
}

TEST_F(RectangleTest, isSquare_WithTolerance) {
    // Test with value clearly within default tolerance (1e-6)
    Rectangle almostSquare(5.0, 4.9999999, 0.0, 0.0);  // diff = 1e-7 < 1e-6 ?
    
    EXPECT_TRUE(almostSquare.isSquare());  // Within default tolerance
    EXPECT_FALSE(almostSquare.isSquare(1e-10));  // Outside strict tolerance
}

// ============================================================================
// Clone Tests
// ============================================================================

TEST_F(RectangleTest, Clone) {
    Rectangle original(10.0, 5.0, 20.0, 30.0, 45.0);
    original.setTypeLimits(TypeLimits::INTERNAL);
    
    auto cloned = original.clone();
    Rectangle* rectClone = dynamic_cast<Rectangle*>(cloned.get());
    
    ASSERT_NE(rectClone, nullptr);
    EXPECT_DOUBLE_EQ(rectClone->width(), original.width());
    EXPECT_DOUBLE_EQ(rectClone->height(), original.height());
    EXPECT_DOUBLE_EQ(rectClone->center().x, original.center().x);
    EXPECT_DOUBLE_EQ(rectClone->center().y, original.center().y);
    EXPECT_DOUBLE_EQ(rectClone->rotationDegrees(), original.rotationDegrees());
    EXPECT_EQ(rectClone->getTypeLimits(), original.getTypeLimits());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(RectangleTest, VerySmallRectangle) {
    Rectangle rect(0.001, 0.001, 0.0, 0.0);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(rect.isInside(center));
    EXPECT_GT(rect.perimeter(), 0.0);
}

TEST_F(RectangleTest, VeryLargeRectangle) {
    Rectangle rect(1e6, 1e6, 0.0, 0.0);
    Point point{1e5, 1e5};
    
    EXPECT_TRUE(rect.isInside(point));
}

TEST_F(RectangleTest, HighlyAsymmetricRectangle) {
    Rectangle rect(100.0, 1.0, 0.0, 0.0);
    Point point{50.0, 0.0};
    
    EXPECT_TRUE(rect.isInside(point));
    
    Bounds bounds = rect.getBounds();
    EXPECT_DOUBLE_EQ(bounds.width(), 100.0);
    EXPECT_DOUBLE_EQ(bounds.height(), 1.0);
}

TEST_F(RectangleTest, Rotation90Degrees) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 90.0);
    
    // At 90 degrees, dimensions should effectively swap in bounding box
    Bounds bounds = rect.getBounds();
    
    // Width and height of bounding box
    EXPECT_NEAR(bounds.width(), 5.0, TOLERANCE);   // Original height
    EXPECT_NEAR(bounds.height(), 10.0, TOLERANCE); // Original width
}

TEST_F(RectangleTest, Rotation180Degrees) {
    Rectangle rect(10.0, 5.0, 0.0, 0.0, 180.0);
    
    // 180 degree rotation shouldn't change bounding box for axis-aligned
    Bounds bounds = rect.getBounds();
    
    EXPECT_NEAR(bounds.width(), 10.0, TOLERANCE);
    EXPECT_NEAR(bounds.height(), 5.0, TOLERANCE);
}

// ============================================================================
// Three-Point Constructor Tests
// ============================================================================

TEST_F(RectangleTest, ThreePointConstructor_AxisAligned) {
    // Create axis-aligned rectangle: 100 wide, 50 tall
    Point p0{0.0, 0.0};     // Bottom-left
    Point p1{100.0, 0.0};   // Bottom-right (width direction)
    Point p2{100.0, 50.0};  // Top-right (height)
    
    Rectangle rect(p0, p1, p2);
    
    // Expected: width=100, height=50, center={50, 25}, rotation=0°
    EXPECT_NEAR(rect.width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect.height(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect.center().x, 50.0, TOLERANCE);
    EXPECT_NEAR(rect.center().y, 25.0, TOLERANCE);
    EXPECT_NEAR(rect.rotationDegrees(), 0.0, TOLERANCE);
}

TEST_F(RectangleTest, ThreePointConstructor_Rotated45) {
    // Create rectangle rotated 45 degrees
    double sqrt2 = std::sqrt(2.0);
    Point p0{0.0, 0.0};
    Point p1{100.0 / sqrt2, 100.0 / sqrt2};  // Width 100 at 45°
    Point p2{0.0, 50.0 / sqrt2 * 2.0};       // Height ~50
    
    Rectangle rect(p0, p1, p2);
    
    EXPECT_NEAR(rect.width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect.height(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect.rotationDegrees(), 45.0, 1.0);  // Allow 1° tolerance
}

TEST_F(RectangleTest, ThreePointConstructor_Rotated30) {
    // Create rectangle rotated 30 degrees
    double cos30 = std::cos(30.0 * M_PI / 180.0);
    double sin30 = std::sin(30.0 * M_PI / 180.0);
    
    Point p0{0.0, 0.0};
    Point p1{100.0 * cos30, 100.0 * sin30};  // Width 100 at 30°
    Point p2{-50.0 * sin30, 50.0 * cos30};   // Height 50 perpendicular
    
    Rectangle rect(p0, p1, p2);
    
    EXPECT_NEAR(rect.width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect.height(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect.rotationDegrees(), 30.0, 0.1);
}

TEST_F(RectangleTest, ThreePointConstructor_CornersMatchInput) {
    // Verify that the resulting rectangle's corners are close to input points
    Point p0{10.0, 20.0};
    Point p1{110.0, 20.0};   // Width 100 along X
    Point p2{110.0, 70.0};   // Height 50 along Y
    
    Rectangle rect(p0, p1, p2);
    
    auto corners = rect.corners();
    
    // Corners should be approximately at the input points
    // Order: TL, TR, BR, BL (in local frame, which depends on rotation)
    bool foundP0 = false;
    bool foundP1 = false;
    bool foundP2 = false;
    
    for (const auto& corner : corners) {
        if (isNear(corner.x, p0.x) && isNear(corner.y, p0.y)) foundP0 = true;
        if (isNear(corner.x, p1.x) && isNear(corner.y, p1.y)) foundP1 = true;
        if (isNear(corner.x, p2.x) && isNear(corner.y, p2.y)) foundP2 = true;
    }
    
    EXPECT_TRUE(foundP0) << "Corner p0 not found in rectangle corners";
    EXPECT_TRUE(foundP1) << "Corner p1 not found in rectangle corners";
    EXPECT_TRUE(foundP2) << "Corner p2 not found in rectangle corners";
}

TEST_F(RectangleTest, ThreePointConstructor_GeometricProperties) {
    // Create a rectangle and verify geometric properties
    Point p0{0.0, 0.0};
    Point p1{60.0, 0.0};
    Point p2{60.0, 40.0};
    
    Rectangle rect(p0, p1, p2);
    
    // Area should be 60 * 40 = 2400
    EXPECT_NEAR(rect.area(), 2400.0, TOLERANCE);
    
    // Perimeter should be 2 * (60 + 40) = 200
    EXPECT_NEAR(rect.perimeter(), 200.0, TOLERANCE);
}

TEST_F(RectangleTest, ThreePointConstructor_SquareDetection) {
    // Create a square via 3-point constructor
    Point p0{0.0, 0.0};
    Point p1{50.0, 0.0};
    Point p2{50.0, 50.0};
    
    Rectangle rect(p0, p1, p2);
    
    EXPECT_TRUE(rect.isSquare());
}

TEST_F(RectangleTest, ThreePointConstructor_NegativeHeight) {
    // P2 projects in negative direction along perpendicular
    Point p0{0.0, 0.0};
    Point p1{100.0, 0.0};
    Point p2{100.0, -50.0};  // Height projects downward
    
    Rectangle rect(p0, p1, p2);
    
    // Height should be absolute value
    EXPECT_NEAR(rect.width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect.height(), 50.0, TOLERANCE);
    
    // Center should still be computed correctly
    EXPECT_NEAR(rect.center().x, 50.0, TOLERANCE);
    EXPECT_NEAR(rect.center().y, -25.0, TOLERANCE);
}

TEST_F(RectangleTest, ThreePointConstructor_OffsetOrigin) {
    // Rectangle not at origin
    Point p0{100.0, 200.0};
    Point p1{150.0, 200.0};
    Point p2{150.0, 250.0};
    
    Rectangle rect(p0, p1, p2);
    
    EXPECT_NEAR(rect.width(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect.height(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect.center().x, 125.0, TOLERANCE);
    EXPECT_NEAR(rect.center().y, 225.0, TOLERANCE);
}

TEST_F(RectangleTest, ThreePointConstructor_TypeLimitsPreserved) {
    Point p0{0.0, 0.0};
    Point p1{100.0, 0.0};
    Point p2{100.0, 50.0};
    
    Rectangle rect(p0, p1, p2, TypeLimits::INTERNAL);
    
    EXPECT_EQ(rect.getTypeLimits(), TypeLimits::INTERNAL);
}

// ============================================================================
// isOnContour Tests
// ============================================================================

TEST_F(RectangleTest, IsOnContour_ExactlyOnEdge) {
    Rectangle rect(100.0, 50.0, 0.0, 0.0);  // 100x50 centered at origin
    
    // Point on right edge
    Point onRight{50.0, 0.0};
    EXPECT_TRUE(rect.isOnContour(onRight, 0.1));
    
    // Point on top edge
    Point onTop{0.0, 25.0};
    EXPECT_TRUE(rect.isOnContour(onTop, 0.1));
    
    // Point at corner
    Point atCorner{50.0, 25.0};
    EXPECT_TRUE(rect.isOnContour(atCorner, 0.1));
}

TEST_F(RectangleTest, IsOnContour_NearEdge) {
    Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    // Point 1 unit outside right edge
    Point nearOutside{51.0, 0.0};
    EXPECT_TRUE(rect.isOnContour(nearOutside, 2.0));
    EXPECT_FALSE(rect.isOnContour(nearOutside, 0.5));
    
    // Point 1 unit inside right edge
    Point nearInside{49.0, 0.0};
    EXPECT_TRUE(rect.isOnContour(nearInside, 2.0));
    EXPECT_FALSE(rect.isOnContour(nearInside, 0.5));
}

TEST_F(RectangleTest, IsOnContour_FarFromEdge) {
    Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    // Center (far inside)
    EXPECT_FALSE(rect.isOnContour(rect.center(), 2.0));
    
    // Far outside
    Point farAway{200.0, 200.0};
    EXPECT_FALSE(rect.isOnContour(farAway, 2.0));
}

TEST_F(RectangleTest, IsOnContour_RotatedRectangle) {
    Rectangle rect(100.0, 50.0, 0.0, 0.0, 45.0);  // Rotated 45 degrees
    
    // Get a corner and verify it's on contour
    auto corners = rect.corners();
    EXPECT_TRUE(rect.isOnContour(corners[0], 0.1));
    EXPECT_TRUE(rect.isOnContour(corners[1], 0.1));
    EXPECT_TRUE(rect.isOnContour(corners[2], 0.1));
    EXPECT_TRUE(rect.isOnContour(corners[3], 0.1));
    
    // Center should not be on contour
    EXPECT_FALSE(rect.isOnContour(rect.center(), 1.0));
}

TEST_F(RectangleTest, IsOnContour_AllEdges) {
    Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    // Test points on all four edges
    Point rightEdge{50.0, 10.0};
    Point leftEdge{-50.0, -10.0};
    Point topEdge{20.0, 25.0};
    Point bottomEdge{-20.0, -25.0};
    
    EXPECT_TRUE(rect.isOnContour(rightEdge, 0.1));
    EXPECT_TRUE(rect.isOnContour(leftEdge, 0.1));
    EXPECT_TRUE(rect.isOnContour(topEdge, 0.1));
    EXPECT_TRUE(rect.isOnContour(bottomEdge, 0.1));
}

TEST_F(RectangleTest, IsOnContour_SmallTolerance) {
    Rectangle rect(20.0, 10.0, 0.0, 0.0);
    
    // With very small tolerance, interior points should not be on contour
    Point interior{5.0, 0.0};  // Halfway to edge
    EXPECT_FALSE(rect.isOnContour(interior, 0.01));
    
    // But edge point should be
    Point edge{10.0, 0.0};
    EXPECT_TRUE(rect.isOnContour(edge, 0.01));
}

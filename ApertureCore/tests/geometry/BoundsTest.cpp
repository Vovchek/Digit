/**
 * @file BoundsTest.cpp
 * @brief Unit tests for Bounds class
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Bounds.h"
#include <sstream>

using namespace aperture;

// ============================================================================
// Construction Tests
// ============================================================================

TEST(BoundsTest, DefaultConstruction) {
    Bounds b;
    EXPECT_DOUBLE_EQ(b.left, 0.0);
    EXPECT_DOUBLE_EQ(b.top, 0.0);
    EXPECT_DOUBLE_EQ(b.right, 0.0);
    EXPECT_DOUBLE_EQ(b.bottom, 0.0);
    EXPECT_TRUE(b.isEmpty());
}

TEST(BoundsTest, ParameterizedConstruction) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    EXPECT_DOUBLE_EQ(b.left, 1.0);
    EXPECT_DOUBLE_EQ(b.top, 2.0);
    EXPECT_DOUBLE_EQ(b.right, 5.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, FromCorners) {
    Point p1{1.0, 2.0};
    Point p2{5.0, 8.0};
    Bounds b = Bounds::fromCorners(p1, p2);
    
    EXPECT_DOUBLE_EQ(b.left, 1.0);
    EXPECT_DOUBLE_EQ(b.top, 2.0);
    EXPECT_DOUBLE_EQ(b.right, 5.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, FromCornersReversed) {
    // Should normalize to left < right, top < bottom
    Point p1{5.0, 8.0};
    Point p2{1.0, 2.0};
    Bounds b = Bounds::fromCorners(p1, p2);
    
    EXPECT_DOUBLE_EQ(b.left, 1.0);
    EXPECT_DOUBLE_EQ(b.top, 2.0);
    EXPECT_DOUBLE_EQ(b.right, 5.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, FromCenterAndSize) {
    Point center{5.0, 5.0};
    Bounds b = Bounds::fromCenterAndSize(center, 4.0, 6.0);
    
    EXPECT_DOUBLE_EQ(b.left, 3.0);
    EXPECT_DOUBLE_EQ(b.top, 2.0);
    EXPECT_DOUBLE_EQ(b.right, 7.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, Infinite) {
    Bounds b = Bounds::infinite();
    EXPECT_TRUE(std::isinf(b.left));
    EXPECT_TRUE(std::isinf(b.top));
    EXPECT_TRUE(std::isinf(b.right));
    EXPECT_TRUE(std::isinf(b.bottom));
}

// ============================================================================
// Property Tests
// ============================================================================

TEST(BoundsTest, Width) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    EXPECT_DOUBLE_EQ(b.width(), 4.0);
}

TEST(BoundsTest, Height) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    EXPECT_DOUBLE_EQ(b.height(), 6.0);
}

TEST(BoundsTest, Area) {
    Bounds b{0.0, 0.0, 4.0, 5.0};
    EXPECT_DOUBLE_EQ(b.area(), 20.0);
}

TEST(BoundsTest, Center) {
    Bounds b{0.0, 0.0, 10.0, 20.0};
    Point c = b.center();
    EXPECT_DOUBLE_EQ(c.x, 5.0);
    EXPECT_DOUBLE_EQ(c.y, 10.0);
}

TEST(BoundsTest, Perimeter) {
    Bounds b{0.0, 0.0, 4.0, 5.0};
    EXPECT_DOUBLE_EQ(b.perimeter(), 18.0);  // 2*(4+5)
}

TEST(BoundsTest, Corners) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    auto corners = b.corners();
    
    EXPECT_EQ(corners.size(), 4);
    EXPECT_EQ(corners[0], Point(1.0, 2.0));   // Top-left
    EXPECT_EQ(corners[1], Point(5.0, 2.0));   // Top-right
    EXPECT_EQ(corners[2], Point(5.0, 8.0));   // Bottom-right
    EXPECT_EQ(corners[3], Point(1.0, 8.0));   // Bottom-left
}

TEST(BoundsTest, IsEmpty) {
    Bounds empty;
    EXPECT_TRUE(empty.isEmpty());
    
    Bounds notEmpty{1.0, 2.0, 5.0, 8.0};
    EXPECT_FALSE(notEmpty.isEmpty());
}

TEST(BoundsTest, IsValid) {
    Bounds valid{1.0, 2.0, 5.0, 8.0};
    EXPECT_TRUE(valid.isValid());
    
    Bounds invalid{5.0, 8.0, 1.0, 2.0};  // Inverted
    EXPECT_FALSE(invalid.isValid());
}

// ============================================================================
// Modification Tests
// ============================================================================

TEST(BoundsTest, Clear) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    b.clear();
    
    EXPECT_TRUE(b.isEmpty());
    EXPECT_DOUBLE_EQ(b.left, 0.0);
    EXPECT_DOUBLE_EQ(b.right, 0.0);
}

TEST(BoundsTest, ShiftXY) {
    Bounds b{0.0, 0.0, 4.0, 5.0};
    b.shift(2.0, 3.0);
    
    EXPECT_DOUBLE_EQ(b.left, 2.0);
    EXPECT_DOUBLE_EQ(b.top, 3.0);
    EXPECT_DOUBLE_EQ(b.right, 6.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, ShiftPoint) {
    Bounds b{0.0, 0.0, 4.0, 5.0};
    b.shift(Point{2.0, 3.0});
    
    EXPECT_DOUBLE_EQ(b.left, 2.0);
    EXPECT_DOUBLE_EQ(b.top, 3.0);
    EXPECT_DOUBLE_EQ(b.right, 6.0);
    EXPECT_DOUBLE_EQ(b.bottom, 8.0);
}

TEST(BoundsTest, ExpandWithPoint) {
    Bounds b{0.0, 0.0, 4.0, 5.0};
    b.expand(Point{10.0, 2.0});
    
    EXPECT_DOUBLE_EQ(b.left, 0.0);
    EXPECT_DOUBLE_EQ(b.top, 0.0);
    EXPECT_DOUBLE_EQ(b.right, 10.0);
    EXPECT_DOUBLE_EQ(b.bottom, 5.0);
}

TEST(BoundsTest, ExpandEmptyBounds) {
    Bounds b;
    b.expand(Point{3.0, 4.0});
    
    EXPECT_DOUBLE_EQ(b.left, 3.0);
    EXPECT_DOUBLE_EQ(b.top, 4.0);
    EXPECT_DOUBLE_EQ(b.right, 3.0);
    EXPECT_DOUBLE_EQ(b.bottom, 4.0);
}

TEST(BoundsTest, Merge) {
    Bounds b1{0.0, 0.0, 4.0, 5.0};
    Bounds b2{2.0, 3.0, 8.0, 6.0};
    b1.merge(b2);
    
    EXPECT_DOUBLE_EQ(b1.left, 0.0);
    EXPECT_DOUBLE_EQ(b1.top, 0.0);
    EXPECT_DOUBLE_EQ(b1.right, 8.0);
    EXPECT_DOUBLE_EQ(b1.bottom, 6.0);
}

TEST(BoundsTest, MergeWithEmpty) {
    Bounds b1{1.0, 2.0, 5.0, 8.0};
    Bounds b2;  // Empty
    b1.merge(b2);
    
    EXPECT_DOUBLE_EQ(b1.left, 1.0);
    EXPECT_DOUBLE_EQ(b1.top, 2.0);
    EXPECT_DOUBLE_EQ(b1.right, 5.0);
    EXPECT_DOUBLE_EQ(b1.bottom, 8.0);
}

TEST(BoundsTest, InflateUniform) {
    Bounds b{2.0, 3.0, 6.0, 8.0};
    b.inflate(1.0);
    
    EXPECT_DOUBLE_EQ(b.left, 1.0);
    EXPECT_DOUBLE_EQ(b.top, 2.0);
    EXPECT_DOUBLE_EQ(b.right, 7.0);
    EXPECT_DOUBLE_EQ(b.bottom, 9.0);
}

TEST(BoundsTest, InflateDifferent) {
    Bounds b{2.0, 3.0, 6.0, 8.0};
    b.inflate(1.0, 2.0);
    
    EXPECT_DOUBLE_EQ(b.left, 1.0);
    EXPECT_DOUBLE_EQ(b.top, 1.0);
    EXPECT_DOUBLE_EQ(b.right, 7.0);
    EXPECT_DOUBLE_EQ(b.bottom, 10.0);
}

// ============================================================================
// Query Tests
// ============================================================================

TEST(BoundsTest, ContainsPoint) {
    Bounds b{0.0, 0.0, 10.0, 10.0};
    
    EXPECT_TRUE(b.contains(Point{5.0, 5.0}));
    EXPECT_TRUE(b.contains(Point{0.0, 0.0}));    // Edge
    EXPECT_TRUE(b.contains(Point{10.0, 10.0}));  // Edge
    EXPECT_FALSE(b.contains(Point{-1.0, 5.0}));
    EXPECT_FALSE(b.contains(Point{11.0, 5.0}));
}

TEST(BoundsTest, ContainsBounds) {
    Bounds outer{0.0, 0.0, 10.0, 10.0};
    Bounds inner{2.0, 3.0, 7.0, 8.0};
    Bounds overlapping{5.0, 5.0, 15.0, 15.0};
    
    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(outer.contains(overlapping));
}

TEST(BoundsTest, Intersects) {
    Bounds b1{0.0, 0.0, 10.0, 10.0};
    Bounds b2{5.0, 5.0, 15.0, 15.0};  // Overlaps
    Bounds b3{20.0, 20.0, 30.0, 30.0};  // Separate
    
    EXPECT_TRUE(b1.intersects(b2));
    EXPECT_FALSE(b1.intersects(b3));
}

TEST(BoundsTest, IntersectsTouching) {
    Bounds b1{0.0, 0.0, 10.0, 10.0};
    Bounds b2{10.0, 0.0, 20.0, 10.0};  // Touching edge
    
    EXPECT_TRUE(b1.intersects(b2));  // Edge counts as intersection
}

TEST(BoundsTest, Intersection) {
    Bounds b1{0.0, 0.0, 10.0, 10.0};
    Bounds b2{5.0, 5.0, 15.0, 15.0};
    Bounds result = b1.intersection(b2);
    
    EXPECT_DOUBLE_EQ(result.left, 5.0);
    EXPECT_DOUBLE_EQ(result.top, 5.0);
    EXPECT_DOUBLE_EQ(result.right, 10.0);
    EXPECT_DOUBLE_EQ(result.bottom, 10.0);
}

TEST(BoundsTest, IntersectionNoOverlap) {
    Bounds b1{0.0, 0.0, 5.0, 5.0};
    Bounds b2{10.0, 10.0, 15.0, 15.0};
    Bounds result = b1.intersection(b2);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST(BoundsTest, UnionWith) {
    Bounds b1{0.0, 0.0, 5.0, 5.0};
    Bounds b2{3.0, 3.0, 10.0, 10.0};
    Bounds result = b1.unionWith(b2);
    
    EXPECT_DOUBLE_EQ(result.left, 0.0);
    EXPECT_DOUBLE_EQ(result.top, 0.0);
    EXPECT_DOUBLE_EQ(result.right, 10.0);
    EXPECT_DOUBLE_EQ(result.bottom, 10.0);
}

TEST(BoundsTest, ClampPoint) {
    Bounds b{0.0, 0.0, 10.0, 10.0};
    
    Point inside{5.0, 5.0};
    EXPECT_EQ(b.clamp(inside), Point(5.0, 5.0));
    
    Point outside{-5.0, 15.0};
    EXPECT_EQ(b.clamp(outside), Point(0.0, 10.0));
    
    Point partial{5.0, 15.0};
    EXPECT_EQ(b.clamp(partial), Point(5.0, 10.0));
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST(BoundsTest, Equality) {
    Bounds b1{1.0, 2.0, 5.0, 8.0};
    Bounds b2{1.0, 2.0, 5.0, 8.0};
    Bounds b3{1.0, 2.0, 5.0, 9.0};
    
    EXPECT_TRUE(b1 == b2);
    EXPECT_FALSE(b1 == b3);
}

TEST(BoundsTest, Inequality) {
    Bounds b1{1.0, 2.0, 5.0, 8.0};
    Bounds b2{1.0, 2.0, 5.0, 9.0};
    
    EXPECT_TRUE(b1 != b2);
    EXPECT_FALSE(b1 != b1);
}

// ============================================================================
// Stream Output Tests
// ============================================================================

TEST(BoundsTest, StreamOutput) {
    Bounds b{1.0, 2.0, 5.0, 8.0};
    std::ostringstream oss;
    oss << b;
    
    std::string output = oss.str();
    EXPECT_NE(output.find("1.00"), std::string::npos);
    EXPECT_NE(output.find("8.00"), std::string::npos);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(BoundsTest, ZeroArea) {
    Bounds b{5.0, 5.0, 5.0, 5.0};  // Point
    EXPECT_DOUBLE_EQ(b.area(), 0.0);
    EXPECT_TRUE(b.isValid());
}

TEST(BoundsTest, NegativeCoordinates) {
    Bounds b{-10.0, -5.0, -2.0, -1.0};
    EXPECT_DOUBLE_EQ(b.width(), 8.0);
    EXPECT_DOUBLE_EQ(b.height(), 4.0);
}

TEST(BoundsTest, LargeNumbers) {
    Bounds b{0.0, 0.0, 1e10, 1e10};
    EXPECT_DOUBLE_EQ(b.width(), 1e10);
}

// ============================================================================
// Hybrid Naming Approach Tests
// ============================================================================

TEST(BoundsTest, MinMaxAccessors_ScreenCoordinates) {
    // SCREEN: top < bottom (Y+ downward)
    Bounds screen{0, 0, 100, 50, CoordinateSystem::screen()};
    
    // X accessors (always unambiguous)
    EXPECT_DOUBLE_EQ(screen.minX(), 0.0);
    EXPECT_DOUBLE_EQ(screen.maxX(), 100.0);
    EXPECT_DOUBLE_EQ(screen.minX(), screen.left);
    EXPECT_DOUBLE_EQ(screen.maxX(), screen.right);
    
    // Y accessors (system-dependent mapping)
    EXPECT_DOUBLE_EQ(screen.minY(), 0.0);    // top (smaller value)
    EXPECT_DOUBLE_EQ(screen.maxY(), 50.0);   // bottom (larger value)
    EXPECT_DOUBLE_EQ(screen.minY(), screen.top);
    EXPECT_DOUBLE_EQ(screen.maxY(), screen.bottom);
    
    // Invariant: minY <= maxY
    EXPECT_LT(screen.minY(), screen.maxY());
}

TEST(BoundsTest, MinMaxAccessors_MathCoordinates) {
    // MATH: bottom < top (Y+ upward)
    Bounds math{0, 50, 100, 0, CoordinateSystem::math()};
    
    // X accessors (always unambiguous)
    EXPECT_DOUBLE_EQ(math.minX(), 0.0);
    EXPECT_DOUBLE_EQ(math.maxX(), 100.0);
    
    // Y accessors (system-dependent mapping)
    EXPECT_DOUBLE_EQ(math.minY(), 0.0);     // bottom (smaller value)
    EXPECT_DOUBLE_EQ(math.maxY(), 50.0);    // top (larger value)
    EXPECT_DOUBLE_EQ(math.minY(), math.bottom);
    EXPECT_DOUBLE_EQ(math.maxY(), math.top);
    
    // Invariant: minY <= maxY (always true!)
    EXPECT_LT(math.minY(), math.maxY());
}

TEST(BoundsTest, MinMaxAccessors_CompareSystemsWithSameLogicalBounds) {
    // Both represent the same logical rectangle [0,0] to [100,50]
    Bounds screen = Bounds::fromMinMax(0, 0, 100, 50, CoordinateSystem::screen());
    Bounds math = Bounds::fromMinMax(0, 0, 100, 50, CoordinateSystem::math());
    
    // System-agnostic accessors return same values
    EXPECT_DOUBLE_EQ(screen.minX(), math.minX());
    EXPECT_DOUBLE_EQ(screen.maxX(), math.maxX());
    EXPECT_DOUBLE_EQ(screen.minY(), math.minY());
    EXPECT_DOUBLE_EQ(screen.maxY(), math.maxY());
    
    // But field values differ!
    EXPECT_NE(screen.top, math.top);        // 0 vs 50
    EXPECT_NE(screen.bottom, math.bottom);  // 50 vs 0
}

TEST(BoundsTest, FromMinMax_ScreenCoordinates) {
    Bounds bounds = Bounds::fromMinMax(10, 20, 100, 80, CoordinateSystem::screen());
    
    // Check field mapping for SCREEN: top=minY, bottom=maxY
    EXPECT_DOUBLE_EQ(bounds.left, 10.0);
    EXPECT_DOUBLE_EQ(bounds.top, 20.0);     // min_y
    EXPECT_DOUBLE_EQ(bounds.right, 100.0);
    EXPECT_DOUBLE_EQ(bounds.bottom, 80.0);  // max_y
    
    // Verify accessors
    EXPECT_DOUBLE_EQ(bounds.minY(), 20.0);
    EXPECT_DOUBLE_EQ(bounds.maxY(), 80.0);
    
    // Verify validity
    EXPECT_TRUE(bounds.isValid());
}

TEST(BoundsTest, FromMinMax_MathCoordinates) {
    Bounds bounds = Bounds::fromMinMax(10, 20, 100, 80, CoordinateSystem::math());
    
    // Check field mapping for MATH: top=maxY, bottom=minY
    EXPECT_DOUBLE_EQ(bounds.left, 10.0);
    EXPECT_DOUBLE_EQ(bounds.top, 80.0);     // max_y
    EXPECT_DOUBLE_EQ(bounds.right, 100.0);
    EXPECT_DOUBLE_EQ(bounds.bottom, 20.0);  // min_y
    
    // Verify accessors
    EXPECT_DOUBLE_EQ(bounds.minY(), 20.0);
    EXPECT_DOUBLE_EQ(bounds.maxY(), 80.0);
    
    // Verify validity
    EXPECT_TRUE(bounds.isValid());
}

TEST(BoundsTest, IsValid_UsesMinMaxAccessors) {
    // Valid SCREEN bounds
    Bounds screenValid{0, 0, 100, 50, CoordinateSystem::screen()};
    EXPECT_TRUE(screenValid.isValid());
    EXPECT_LE(screenValid.minY(), screenValid.maxY());
    
    // Invalid SCREEN bounds
    Bounds screenInvalid{0, 50, 100, 0, CoordinateSystem::screen()};
    EXPECT_FALSE(screenInvalid.isValid());
    EXPECT_GT(screenInvalid.minY(), screenInvalid.maxY());
    
    // Valid MATH bounds
    Bounds mathValid{0, 50, 100, 0, CoordinateSystem::math()};
    EXPECT_TRUE(mathValid.isValid());
    EXPECT_LE(mathValid.minY(), mathValid.maxY());
    
    // Invalid MATH bounds
    Bounds mathInvalid{0, 0, 100, 50, CoordinateSystem::math()};
    EXPECT_FALSE(mathInvalid.isValid());
    EXPECT_GT(mathInvalid.minY(), mathInvalid.maxY());
}

TEST(BoundsTest, SystemAgnosticContainmentCheck) {
    Bounds screen = Bounds::fromMinMax(0, 0, 100, 50, CoordinateSystem::screen());
    Bounds math = Bounds::fromMinMax(0, 0, 100, 50, CoordinateSystem::math());
    
    Point testPoint{50, 25};
    
    // System-agnostic containment using min/max
    auto isInside = [](const Bounds& b, const Point& p) {
        return p.x >= b.minX() && p.x <= b.maxX() &&
               p.y >= b.minY() && p.y <= b.maxY();
    };
    
    EXPECT_TRUE(isInside(screen, testPoint));
    EXPECT_TRUE(isInside(math, testPoint));
}

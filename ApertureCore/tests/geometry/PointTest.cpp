/**
 * @file PointTest.cpp
 * @brief Unit tests for Point class
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Point.h"
#include <sstream>
#include <cmath>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace aperture;

// ============================================================================
// Construction Tests
// ============================================================================

TEST(PointTest, DefaultConstruction) {
    Point p;
    EXPECT_DOUBLE_EQ(p.x, 0.0);
    EXPECT_DOUBLE_EQ(p.y, 0.0);
}

TEST(PointTest, ParameterizedConstruction) {
    Point p{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p.x, 3.0);
    EXPECT_DOUBLE_EQ(p.y, 4.0);
}

TEST(PointTest, CopyConstruction) {
    Point p1{1.5, 2.5};
    Point p2 = p1;
    EXPECT_DOUBLE_EQ(p2.x, 1.5);
    EXPECT_DOUBLE_EQ(p2.y, 2.5);
}

// ============================================================================
// Distance Tests
// ============================================================================

TEST(PointTest, DistanceToOrigin) {
    Point p{3.0, 4.0};
    Point origin{0.0, 0.0};
    EXPECT_DOUBLE_EQ(p.distanceTo(origin), 5.0);
}

TEST(PointTest, DistanceToBetweenPoints) {
    Point p1{0.0, 0.0};
    Point p2{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p1.distanceTo(p2), 5.0);
    EXPECT_DOUBLE_EQ(p2.distanceTo(p1), 5.0);  // Symmetric
}

TEST(PointTest, DistanceSquared) {
    Point p1{0.0, 0.0};
    Point p2{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p1.distanceSquaredTo(p2), 25.0);
}

TEST(PointTest, Magnitude) {
    Point p{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p.magnitude(), 5.0);
}

TEST(PointTest, MagnitudeSquared) {
    Point p{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p.magnitudeSquared(), 25.0);
}

TEST(PointTest, ZeroDistance) {
    Point p{2.0, 3.0};
    EXPECT_DOUBLE_EQ(p.distanceTo(p), 0.0);
}

// ============================================================================
// Arithmetic Operator Tests
// ============================================================================

TEST(PointTest, Addition) {
    Point p1{1.0, 2.0};
    Point p2{3.0, 4.0};
    Point result = p1 + p2;
    EXPECT_DOUBLE_EQ(result.x, 4.0);
    EXPECT_DOUBLE_EQ(result.y, 6.0);
}

TEST(PointTest, Subtraction) {
    Point p1{5.0, 7.0};
    Point p2{2.0, 3.0};
    Point result = p1 - p2;
    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 4.0);
}

TEST(PointTest, ScalarMultiplication) {
    Point p{2.0, 3.0};
    Point result = p * 2.5;
    EXPECT_DOUBLE_EQ(result.x, 5.0);
    EXPECT_DOUBLE_EQ(result.y, 7.5);
}

TEST(PointTest, ScalarMultiplicationCommutative) {
    Point p{2.0, 3.0};
    Point result1 = p * 2.5;
    Point result2 = 2.5 * p;
    EXPECT_DOUBLE_EQ(result1.x, result2.x);
    EXPECT_DOUBLE_EQ(result1.y, result2.y);
}

TEST(PointTest, ScalarDivision) {
    Point p{10.0, 15.0};
    Point result = p / 5.0;
    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
}

TEST(PointTest, Negation) {
    Point p{3.0, -4.0};
    Point result = -p;
    EXPECT_DOUBLE_EQ(result.x, -3.0);
    EXPECT_DOUBLE_EQ(result.y, 4.0);
}

TEST(PointTest, CompoundAddition) {
    Point p{1.0, 2.0};
    p += Point{3.0, 4.0};
    EXPECT_DOUBLE_EQ(p.x, 4.0);
    EXPECT_DOUBLE_EQ(p.y, 6.0);
}

TEST(PointTest, CompoundSubtraction) {
    Point p{5.0, 7.0};
    p -= Point{2.0, 3.0};
    EXPECT_DOUBLE_EQ(p.x, 3.0);
    EXPECT_DOUBLE_EQ(p.y, 4.0);
}

TEST(PointTest, CompoundMultiplication) {
    Point p{2.0, 3.0};
    p *= 2.5;
    EXPECT_DOUBLE_EQ(p.x, 5.0);
    EXPECT_DOUBLE_EQ(p.y, 7.5);
}

TEST(PointTest, CompoundDivision) {
    Point p{10.0, 15.0};
    p /= 5.0;
    EXPECT_DOUBLE_EQ(p.x, 2.0);
    EXPECT_DOUBLE_EQ(p.y, 3.0);
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST(PointTest, Equality) {
    Point p1{1.5, 2.5};
    Point p2{1.5, 2.5};
    Point p3{1.5, 2.6};
    
    EXPECT_TRUE(p1 == p2);
    EXPECT_FALSE(p1 == p3);
}

TEST(PointTest, Inequality) {
    Point p1{1.5, 2.5};
    Point p2{1.5, 2.6};
    
    EXPECT_TRUE(p1 != p2);
    EXPECT_FALSE(p1 != p1);
}

TEST(PointTest, IsNear) {
    Point p1{1.0, 2.0};
    Point p2{1.0000001, 2.0000001};
    Point p3{1.1, 2.1};
    
    EXPECT_TRUE(p1.isNear(p2, 1e-6));
    EXPECT_TRUE(p1.isNear(p2));  // Default tolerance
    EXPECT_FALSE(p1.isNear(p3, 1e-6));
    EXPECT_TRUE(p1.isNear(p3, 0.2));  // Larger tolerance
}

// ============================================================================
// Geometric Operation Tests
// ============================================================================

TEST(PointTest, DotProduct) {
    Point p1{2.0, 3.0};
    Point p2{4.0, 5.0};
    EXPECT_DOUBLE_EQ(p1.dot(p2), 23.0);  // 2*4 + 3*5 = 8 + 15 = 23
}

TEST(PointTest, DotProductZero) {
    Point p1{1.0, 0.0};
    Point p2{0.0, 1.0};
    EXPECT_DOUBLE_EQ(p1.dot(p2), 0.0);  // Perpendicular
}

TEST(PointTest, CrossProduct) {
    Point p1{2.0, 3.0};
    Point p2{4.0, 5.0};
    EXPECT_DOUBLE_EQ(p1.cross(p2), -2.0);  // 2*5 - 3*4 = 10 - 12 = -2
}

TEST(PointTest, CrossProductParallel) {
    Point p1{2.0, 3.0};
    Point p2{4.0, 6.0};  // Parallel (2x p1)
    EXPECT_DOUBLE_EQ(p1.cross(p2), 0.0);
}

TEST(PointTest, Normalization) {
    Point p{3.0, 4.0};
    Point normalized = p.normalized();
    EXPECT_DOUBLE_EQ(normalized.magnitude(), 1.0);
    EXPECT_DOUBLE_EQ(normalized.x, 0.6);
    EXPECT_DOUBLE_EQ(normalized.y, 0.8);
}

TEST(PointTest, NormalizationZeroVector) {
    Point p{0.0, 0.0};
    Point normalized = p.normalized();
    EXPECT_DOUBLE_EQ(normalized.x, 0.0);
    EXPECT_DOUBLE_EQ(normalized.y, 0.0);
}

TEST(PointTest, Rotation90Degrees) {
    Point p{1.0, 0.0};
    Point rotated = p.rotated(M_PI / 2.0);  // 90 degrees
    EXPECT_NEAR(rotated.x, 0.0, 1e-10);
    EXPECT_NEAR(rotated.y, 1.0, 1e-10);
}

TEST(PointTest, Rotation180Degrees) {
    Point p{1.0, 0.0};
    Point rotated = p.rotated(M_PI);  // 180 degrees
    EXPECT_NEAR(rotated.x, -1.0, 1e-10);
    EXPECT_NEAR(rotated.y, 0.0, 1e-10);
}

TEST(PointTest, RotationAroundCenter) {
    Point p{2.0, 0.0};
    Point center{1.0, 0.0};
    Point rotated = p.rotatedAround(center, M_PI / 2.0);  // 90 degrees around (1,0)
    
    EXPECT_NEAR(rotated.x, 1.0, 1e-10);
    EXPECT_NEAR(rotated.y, 1.0, 1e-10);
}

// ============================================================================
// Free Function Tests
// ============================================================================

TEST(PointTest, FreeDistanceFunction) {
    Point p1{0.0, 0.0};
    Point p2{3.0, 4.0};
    EXPECT_DOUBLE_EQ(distance(p1, p2), 5.0);
}

TEST(PointTest, LinearInterpolation) {
    Point p1{0.0, 0.0};
    Point p2{10.0, 20.0};
    
    Point mid = lerp(p1, p2, 0.5);
    EXPECT_DOUBLE_EQ(mid.x, 5.0);
    EXPECT_DOUBLE_EQ(mid.y, 10.0);
    
    Point quarter = lerp(p1, p2, 0.25);
    EXPECT_DOUBLE_EQ(quarter.x, 2.5);
    EXPECT_DOUBLE_EQ(quarter.y, 5.0);
    
    Point start = lerp(p1, p2, 0.0);
    EXPECT_DOUBLE_EQ(start.x, 0.0);
    EXPECT_DOUBLE_EQ(start.y, 0.0);
    
    Point end = lerp(p1, p2, 1.0);
    EXPECT_DOUBLE_EQ(end.x, 10.0);
    EXPECT_DOUBLE_EQ(end.y, 20.0);
}

// ============================================================================
// Stream Output Tests
// ============================================================================

TEST(PointTest, StreamOutput) {
    Point p{3.5, 4.25};
    std::ostringstream oss;
    oss << p;
    
    std::string output = oss.str();
    EXPECT_NE(output.find("3.5"), std::string::npos);
    EXPECT_NE(output.find("4.25"), std::string::npos);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(PointTest, VerySmallNumbers) {
    Point p{1e-10, 2e-10};
    EXPECT_DOUBLE_EQ(p.x, 1e-10);
    EXPECT_DOUBLE_EQ(p.y, 2e-10);
}

TEST(PointTest, VeryLargeNumbers) {
    Point p{1e10, 2e10};
    EXPECT_DOUBLE_EQ(p.x, 1e10);
    EXPECT_DOUBLE_EQ(p.y, 2e10);
}

TEST(PointTest, NegativeCoordinates) {
    Point p{-3.0, -4.0};
    EXPECT_DOUBLE_EQ(p.magnitude(), 5.0);
}

TEST(PointTest, MixedSignCoordinates) {
    Point p{3.0, -4.0};
    EXPECT_DOUBLE_EQ(p.magnitude(), 5.0);
}

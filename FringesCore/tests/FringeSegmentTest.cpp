#include "gtest/gtest.h"
#include "FringeSegment.h"
#include "WavefrontFromContours.h"
#include <cmath>
#include <algorithm>

// ===== Construction Tests =====

TEST(CFringeTest, DefaultConstruction) {
    FringeSegment fringe;
    EXPECT_EQ(0.0, fringe.getNumber());
    EXPECT_EQ(0, fringe.getPointCount());
    EXPECT_FALSE(fringe.isClosed());
}

TEST(CFringeTest, ConstructionWithNumber) {
    FringeSegment fringe(2.5);
    EXPECT_EQ(2.5, fringe.getNumber());
    EXPECT_EQ(0, fringe.getPointCount());
    EXPECT_FALSE(fringe.isClosed());
}

TEST(CFringeTest, CopyConstruction) {
    FringeSegment original(1.5);
    original.addPoint(aperture::Point(10, 20));
    original.addPoint(aperture::Point(30, 40));
    original.setClosed(true);
    
    FringeSegment copy(original);
    
    EXPECT_EQ(1.5, copy.getNumber());
    EXPECT_EQ(2, copy.getPointCount());
    EXPECT_TRUE(copy.isClosed());
    EXPECT_EQ(10.0, copy.getPoint(0).x);
    EXPECT_EQ(20.0, copy.getPoint(0).y);
    EXPECT_EQ(30.0, copy.getPoint(1).x);
    EXPECT_EQ(40.0, copy.getPoint(1).y);
}

TEST(CFringeTest, AssignmentOperator) {
    FringeSegment original(2.0);
    original.addPoint(aperture::Point(5, 10));
    original.setClosed(true);
    
    FringeSegment assigned(3.0);
    assigned = original;
    
    EXPECT_EQ(2.0, assigned.getNumber());
    EXPECT_EQ(1, assigned.getPointCount());
    EXPECT_TRUE(assigned.isClosed());
    EXPECT_EQ(5.0, assigned.getPoint(0).x);
}

TEST(CFringeTest, SelfAssignment) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 20));
    
    fringe = fringe;
    
    EXPECT_EQ(1.0, fringe.getNumber());
    EXPECT_EQ(1, fringe.getPointCount());
}

// ===== Point Management Tests =====

TEST(CFringeTest, addPoint) {
    FringeSegment fringe(1.0);
    
    int idx0 = fringe.addPoint(aperture::Point(10, 20));
    EXPECT_EQ(0, idx0);
    EXPECT_EQ(1, fringe.getPointCount());
    
    int idx1 = fringe.addPoint(aperture::Point(30, 40));
    EXPECT_EQ(1, idx1);
    EXPECT_EQ(2, fringe.getPointCount());
    
    EXPECT_EQ(10.0, fringe.getPoint(0).x);
    EXPECT_EQ(20.0, fringe.getPoint(0).y);
    EXPECT_EQ(30.0, fringe.getPoint(1).x);
    EXPECT_EQ(40.0, fringe.getPoint(1).y);
}

TEST(CFringeTest, insertPointAtBeginning) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(20, 20));
    fringe.addPoint(aperture::Point(30, 30));
    
    fringe.insertPoint(0, aperture::Point(10, 10));
    
    EXPECT_EQ(3, fringe.getPointCount());
    EXPECT_EQ(10.0, fringe.getPoint(0).x);
    EXPECT_EQ(20.0, fringe.getPoint(1).x);
    EXPECT_EQ(30.0, fringe.getPoint(2).x);
}

TEST(CFringeTest, insertPointInMiddle) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(30, 30));
    
    fringe.insertPoint(1, aperture::Point(20, 20));
    
    EXPECT_EQ(3, fringe.getPointCount());
    EXPECT_EQ(10.0, fringe.getPoint(0).x);
    EXPECT_EQ(20.0, fringe.getPoint(1).x);
    EXPECT_EQ(30.0, fringe.getPoint(2).x);
}

TEST(CFringeTest, insertPointAtEnd) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(20, 20));
    
    fringe.insertPoint(2, aperture::Point(30, 30));
    
    EXPECT_EQ(3, fringe.getPointCount());
    EXPECT_EQ(30.0, fringe.getPoint(2).x);
}

TEST(CFringeTest, insertPointInvalidIndex) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    int originalCount = fringe.getPointCount();
    
    // Should not insert - index too large
    fringe.insertPoint(5, aperture::Point(50, 50));
    EXPECT_EQ(originalCount, fringe.getPointCount());
    
    // Should not insert - negative index
    fringe.insertPoint(-1, aperture::Point(5, 5));
    EXPECT_EQ(originalCount, fringe.getPointCount());
}

TEST(CFringeTest, removePoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(20, 20));
    fringe.addPoint(aperture::Point(30, 30));
    
    fringe.removePoint(1);
    
    EXPECT_EQ(2, fringe.getPointCount());
    EXPECT_EQ(10.0, fringe.getPoint(0).x);
    EXPECT_EQ(30.0, fringe.getPoint(1).x);
}

TEST(CFringeTest, RemoveFirstPoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(20, 20));
    
    fringe.removePoint(0);
    
    EXPECT_EQ(1, fringe.getPointCount());
    EXPECT_EQ(20.0, fringe.getPoint(0).x);
}

TEST(CFringeTest, RemoveLastPoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(20, 20));
    
    fringe.removePoint(1);
    
    EXPECT_EQ(1, fringe.getPointCount());
    EXPECT_EQ(10.0, fringe.getPoint(0).x);
}

TEST(CFringeTest, removePointInvalidIndex) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    int originalCount = fringe.getPointCount();
    
    fringe.removePoint(5);
    EXPECT_EQ(originalCount, fringe.getPointCount());
    
    fringe.removePoint(-1);
    EXPECT_EQ(originalCount, fringe.getPointCount());
}

TEST(CFringeTest, movePoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    fringe.addPoint(aperture::Point(20, 20));
    
    fringe.movePoint(0, aperture::Point(15, 15));
    
    EXPECT_EQ(15.0, fringe.getPoint(0).x);
    EXPECT_EQ(15.0, fringe.getPoint(0).y);
    EXPECT_EQ(20.0, fringe.getPoint(1).x);
}

TEST(CFringeTest, movePointInvalidIndex) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    fringe.movePoint(5, aperture::Point(50, 50));
    EXPECT_EQ(10.0, fringe.getPoint(0).x); // Unchanged
    
    fringe.movePoint(-1, aperture::Point(5, 5));
    EXPECT_EQ(10.0, fringe.getPoint(0).x); // Unchanged
}

TEST(CFringeTest, setPointAndgetPoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    fringe.setPoint(0, aperture::Point(25, 35));
    
    aperture::Point p = fringe.getPoint(0);
    EXPECT_EQ(25.0, p.x);
    EXPECT_EQ(35.0, p.y);
}

TEST(CFringeTest, getPointInvalidIndex) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    aperture::Point p = fringe.getPoint(5);
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
}

TEST(CFringeTest, appendPoint) {
    FringeSegment fringe1(1.0);
    fringe1.addPoint(aperture::Point(10, 10));
    fringe1.addPoint(aperture::Point(20, 20));
    
    FringeSegment fringe2(1.0);
    fringe2.addPoint(aperture::Point(30, 30));
    fringe2.addPoint(aperture::Point(40, 40));
    
    fringe1.appendPoints(fringe2);
    
    EXPECT_EQ(4, fringe1.getPointCount());
    EXPECT_EQ(10.0, fringe1.getPoint(0).x);
    EXPECT_EQ(20.0, fringe1.getPoint(1).x);
    EXPECT_EQ(30.0, fringe1.getPoint(2).x);
    EXPECT_EQ(40.0, fringe1.getPoint(3).x);
}

// ===== Property Tests =====

TEST(CFringeTest, GetsetNumber) {
    FringeSegment fringe(1.0);
    EXPECT_EQ(1.0, fringe.getNumber());
    
    fringe.setNumber(2.5);
    EXPECT_EQ(2.5, fringe.getNumber());
}

TEST(CFringeTest, GetsetClosed) {
    FringeSegment fringe(1.0);
    EXPECT_FALSE(fringe.isClosed());
    
    fringe.setClosed(true);
    EXPECT_TRUE(fringe.isClosed());
    
    fringe.setClosed(false);
    EXPECT_FALSE(fringe.isClosed());
}

// ===== Hit Testing Tests =====

TEST(CFringeTest, findNearestPointWithinTolerance) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    fringe.addPoint(aperture::Point(200, 100));
    fringe.addPoint(aperture::Point(300, 100));
    
    int idx = fringe.findNearestPoint(aperture::Point(205, 105), 10);
    EXPECT_EQ(1, idx); // Middle point
}

TEST(CFringeTest, findNearestPointExactMatch) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    fringe.addPoint(aperture::Point(200, 200));
    
    int idx = fringe.findNearestPoint(aperture::Point(200, 200), 10);
    EXPECT_EQ(1, idx);
}

TEST(CFringeTest, findNearestPointOutsideTolerance) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    fringe.addPoint(aperture::Point(200, 100));
    
    int idx = fringe.findNearestPoint(aperture::Point(500, 100), 10);
    EXPECT_EQ(-1, idx); // Too far
}

TEST(CFringeTest, findNearestPointEmptyFringe) {
    FringeSegment fringe(1.0);
    
    int idx = fringe.findNearestPoint(aperture::Point(100, 100), 10);
    EXPECT_EQ(-1, idx);
}

TEST(CFringeTest, findNearestPointSelectsClosest) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    fringe.addPoint(aperture::Point(110, 100));
    fringe.addPoint(aperture::Point(105, 100));
    
    int idx = fringe.findNearestPoint(aperture::Point(106, 100), 20);
    EXPECT_EQ(2, idx); // Index 2 is closest at x=105
}

TEST(CFringeTest, isPointOnPolyline) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    fringe.addPoint(aperture::Point(200, 100));
    
    int nearestIdx = -1;
    bool result = fringe.isPointOnPolyline(aperture::Point(105, 102), 10, nearestIdx);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(0, nearestIdx);
}

TEST(CFringeTest, isPointOnPolylineOutsideTolerance) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 100));
    
    int nearestIdx = -1;
    bool result = fringe.isPointOnPolyline(aperture::Point(500, 100), 10, nearestIdx);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(-1, nearestIdx);
}

TEST(CFringeTest, getBoundingRectEmpty) {
    FringeSegment fringe(1.0);
    
    aperture::Bounds rect = fringe.getBoundingRect();
    
    EXPECT_EQ(0, rect.left);
    EXPECT_EQ(0, rect.top);
    EXPECT_EQ(0, rect.right);
    EXPECT_EQ(0, rect.bottom);
}

TEST(CFringeTest, getBoundingRectSinglePoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(100, 200));
    
    aperture::Bounds rect = fringe.getBoundingRect();
    
    EXPECT_EQ(100, rect.left);
    EXPECT_EQ(200, rect.top);
    EXPECT_EQ(100, rect.right);
    EXPECT_EQ(200, rect.bottom);
}

TEST(CFringeTest, getBoundingRectMultiplePoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(50, 30));
    fringe.addPoint(aperture::Point(150, 20));
    fringe.addPoint(aperture::Point(100, 80));
    
    aperture::Bounds rect = fringe.getBoundingRect();
    
    EXPECT_EQ(50, rect.left);
    EXPECT_EQ(20, rect.top);
    EXPECT_EQ(150, rect.right);
    EXPECT_EQ(80, rect.bottom);
}

// ===== Advanced Operations Tests =====

TEST(CFringeTest, splitAtMiddle) {
    FringeSegment original(1.5);
    original.addPoint(aperture::Point(10, 10));
    original.addPoint(aperture::Point(20, 20));
    original.addPoint(aperture::Point(30, 30));
    original.addPoint(aperture::Point(40, 40));
    
    FringeSegment newFringe = original.split(2);
    
    EXPECT_EQ(2, original.getPointCount());
    EXPECT_EQ(10.0, original.getPoint(0).x);
    EXPECT_EQ(20.0, original.getPoint(1).x);
    
    EXPECT_EQ(2, newFringe.getPointCount());
    EXPECT_EQ(30.0, newFringe.getPoint(0).x);
    EXPECT_EQ(40.0, newFringe.getPoint(1).x);
    EXPECT_EQ(1.5, newFringe.getNumber());
}

TEST(CFringeTest, splitInvalidIndexReturnsEmpty) {
    FringeSegment original(1.0);
    original.addPoint(aperture::Point(10, 10));
    original.addPoint(aperture::Point(20, 20));
    
    FringeSegment empty1 = original.split(0);
    EXPECT_EQ(0, empty1.getPointCount());
    
    FringeSegment empty2 = original.split(2);
    EXPECT_EQ(0, empty2.getPointCount());
    
    FringeSegment empty3 = original.split(-1);
    EXPECT_EQ(0, empty3.getPointCount());
    
    // Original should be unchanged
    EXPECT_EQ(2, original.getPointCount());
}

TEST(CFringeTest, getArcLengthEmpty) {
    FringeSegment fringe(1.0);
    
    double length = fringe.getArcLength();
    EXPECT_EQ(0.0, length);
}

TEST(CFringeTest, getArcLengthSinglePoint) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10, 10));
    
    double length = fringe.getArcLength();
    EXPECT_EQ(0.0, length);
}

TEST(CFringeTest, getArcLengthTwoPoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(3, 4)); // Distance = 5
    
    double length = fringe.getArcLength();
    EXPECT_NEAR(5.0, length, 0.001);
}

TEST(CFringeTest, getArcLengthMultiplePoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(10, 0)); // Distance = 10
    fringe.addPoint(aperture::Point(10, 10)); // Distance = 10
    
    double length = fringe.getArcLength();
    EXPECT_NEAR(20.0, length, 0.001);
}

TEST(CFringeTest, getArcLengthClosedLoop) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(10, 0));
    fringe.addPoint(aperture::Point(10, 10));
    fringe.setClosed(true);
    
    double length = fringe.getArcLength();
    // 10 + 10 + sqrt(100+100) ≈ 20 + 14.142
    EXPECT_NEAR(34.142, length, 0.01);
}

TEST(CFringeTest, subdivideSegmentsNoSubdivisionNeeded) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(5, 0));
    
    fringe.subdivideSegments(10.0); // Max gap larger than segment
    
    EXPECT_EQ(2, fringe.getPointCount()); // No subdivision
}

TEST(CFringeTest, subdivideSegmentsCreatesIntermediatePoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(30, 0));
    
    fringe.subdivideSegments(10.0);
    
    EXPECT_GT(fringe.getPointCount(), 2); // Should have intermediate points
    
    // Check spacing is reasonable
    for (int i = 1; i < fringe.getPointCount(); i++) {
        double dx = fringe.getPoint(i).x - fringe.getPoint(i-1).x;
        double dy = fringe.getPoint(i).y - fringe.getPoint(i-1).y;
        double dist = sqrt(dx*dx + dy*dy);
        EXPECT_LE(dist, 11.0); // Should be close to maxGap
    }
}

TEST(CFringeTest, simplifyRemovesRedundantPoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(5, 0));  // On line
    fringe.addPoint(aperture::Point(10, 0)); // On line
    fringe.addPoint(aperture::Point(15, 0)); // On line
    fringe.addPoint(aperture::Point(20, 0));
    
    fringe.simplify(0.1); // Very small epsilon
    
    // Should keep only endpoints for a straight line
    EXPECT_EQ(2, fringe.getPointCount());
    EXPECT_EQ(0.0, fringe.getPoint(0).x);
    EXPECT_EQ(20.0, fringe.getPoint(1).x);
}

TEST(CFringeTest, simplifyKeepsSignificantPoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(10, 10)); // Significant deviation
    fringe.addPoint(aperture::Point(20, 0));
    
    fringe.simplify(0.5); // Small epsilon
    
    // Should keep all points (triangle)
    EXPECT_EQ(3, fringe.getPointCount());
}

TEST(CFringeTest, simplifyHandlesEmptyFringe) {
    FringeSegment fringe(1.0);
    
    fringe.simplify(1.0);
    
    EXPECT_EQ(0, fringe.getPointCount());
}

TEST(CFringeTest, simplifyHandlesTwoPoints) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(0, 0));
    fringe.addPoint(aperture::Point(10, 10));
    
    fringe.simplify(1.0);
    
    EXPECT_EQ(2, fringe.getPointCount()); // Always keeps 2 points
}

// ===== Edge Cases =====

TEST(CFringeTest, EmptyFringeOperations) {
    FringeSegment fringe(1.0);
    
    EXPECT_EQ(0, fringe.getPointCount());
    EXPECT_EQ(0.0, fringe.getArcLength());
    
    aperture::Bounds rect = fringe.getBoundingRect();
    EXPECT_EQ(0, rect.width());
    EXPECT_EQ(0, rect.height());
}

TEST(CFringeTest, LargeNumberOfPoints) {
    FringeSegment fringe(1.0);
    
    for (int i = 0; i < 1000; i++) {
        fringe.addPoint(aperture::Point(i, i * 2));
    }
    
    EXPECT_EQ(1000, fringe.getPointCount());
    EXPECT_EQ(0.0, fringe.getPoint(0).x);
    EXPECT_EQ(999.0, fringe.getPoint(999).x);
}

TEST(CFringeTest, NegativeCoordinates) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(-10, -20));
    fringe.addPoint(aperture::Point(-5, -10));
    
    EXPECT_EQ(-10.0, fringe.getPoint(0).x);
    EXPECT_EQ(-20.0, fringe.getPoint(0).y);
    
    aperture::Bounds rect = fringe.getBoundingRect();
    EXPECT_EQ(-10, rect.left);
    EXPECT_EQ(-20, rect.top);
}

TEST(CFringeTest, FractionalCoordinates) {
    FringeSegment fringe(1.0);
    fringe.addPoint(aperture::Point(10.5, 20.75));
    
    EXPECT_NEAR(10.5, fringe.getPoint(0).x, 0.001);
    EXPECT_NEAR(20.75, fringe.getPoint(0).y, 0.001);
}

TEST(CFringeTest, NegativeFringeNumber) {
    FringeSegment fringe(-2.5);
    EXPECT_EQ(-2.5, fringe.getNumber());
}

TEST(CFringeTest, ZeroFringeNumber) {
    FringeSegment fringe(0.0);
    EXPECT_EQ(0.0, fringe.getNumber());
}

//TEST(WavefrontBoundingCircleTest, SingleRowVisibleSpanComputesExpectedCircle)
//{
//    aperture::visibility::VisibilityMask visibilityMask(10, 6);
//    std::fill(visibilityMask.data.begin(), visibilityMask.data.end(), static_cast<uint8_t>(1));
//
//    std::vector<FringeSegment> fringeSegments;
//    aperture::Bounds bounds = aperture::Bounds::fromMinMax(0.0, 0.0, 10.0, 6.0);
//
//    WavefrontFromContoursInput input(
//        bounds,
//        visibilityMask,
//        fringeSegments,
//        1.,
//        0.,
//        10,
//        6,
//        aperture::CoordinateSystemType::SCREEN,
//        aperture::CoordinateSystemType::SCREEN);
//
//    WavefrontFromContoursContext ctx(input);
//    std::vector<char> mask(60, 0);
//
//    const int row = 2;
//    for (int col = 2; col <= 7; ++col)
//    {
//        mask[row * 10 + col] = 1;
//    }
//
//    WavefrontBoundingCircle circle = ctx.computeMaskBoundingCircle(mask);
//
//    ASSERT_TRUE(circle.valid);
//    EXPECT_NEAR(4.5, circle.center.x, 1e-9);
//    EXPECT_NEAR(2.0, circle.center.y, 1e-9);
//    EXPECT_NEAR(2.5, circle.radius, 1e-9);
//}
//
//TEST(WavefrontBoundingCircleTest, MultiRowVisibleBlockComputesExpectedCircle)
//{
//    aperture::visibility::VisibilityMask visibilityMask(10, 6);
//    std::fill(visibilityMask.data.begin(), visibilityMask.data.end(), static_cast<uint8_t>(1));
//
//    std::vector<FringeSegment> fringeSegments;
//    aperture::Bounds bounds = aperture::Bounds::fromMinMax(0.0, 0.0, 10.0, 6.0);
//
//    WavefrontFromContoursInput input(
//        bounds,
//        visibilityMask,
//        fringeSegments,
//        1.,
//        0.,
//        10,
//        6,
//        aperture::CoordinateSystemType::SCREEN,
//        aperture::CoordinateSystemType::SCREEN);
//
//    WavefrontFromContoursContext ctx(input);
//    std::vector<char> mask(60, 0);
//
//    for (int row = 1; row <= 4; ++row)
//    {
//        for (int col = 3; col <= 8; ++col)
//        {
//            mask[row * 10 + col] = 1;
//        }
//    }
//
//    WavefrontBoundingCircle circle = ctx.computeMaskBoundingCircle(mask);
//
//    ASSERT_TRUE(circle.valid);
//    EXPECT_NEAR(5.5, circle.center.x, 1e-9);
//    EXPECT_NEAR(2.5, circle.center.y, 1e-9);
//    EXPECT_NEAR(std::sqrt(8.5), circle.radius, 1e-9);
//}

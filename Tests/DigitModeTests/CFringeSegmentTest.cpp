#include "stdafx.h"
#include "gtest/gtest.h"
#include "../DigitMode/CFringeSegment.h"
#include "../DigitMode/WavefrontSolver/WavefrontFromContours.h"
#include "../MGTools/Include/Utils/BaseDataType.h"
#include <cmath>
#include <algorithm>

// ===== Construction Tests =====

TEST(CFringeTest, DefaultConstruction) {
    CFringeSegment fringe;
    EXPECT_EQ(0.0, fringe.GetNumber());
    EXPECT_EQ(0, fringe.GetPointCount());
    EXPECT_FALSE(fringe.IsClosed());
}

TEST(CFringeTest, ConstructionWithNumber) {
    CFringeSegment fringe(2.5);
    EXPECT_EQ(2.5, fringe.GetNumber());
    EXPECT_EQ(0, fringe.GetPointCount());
    EXPECT_FALSE(fringe.IsClosed());
}

TEST(CFringeTest, CopyConstruction) {
    CFringeSegment original(1.5);
    original.AddPoint(CDPoint(10, 20));
    original.AddPoint(CDPoint(30, 40));
    original.SetClosed(TRUE);
    
    CFringeSegment copy(original);
    
    EXPECT_EQ(1.5, copy.GetNumber());
    EXPECT_EQ(2, copy.GetPointCount());
    EXPECT_TRUE(copy.IsClosed());
    EXPECT_EQ(10.0, copy.GetPoint(0).x);
    EXPECT_EQ(20.0, copy.GetPoint(0).y);
    EXPECT_EQ(30.0, copy.GetPoint(1).x);
    EXPECT_EQ(40.0, copy.GetPoint(1).y);
}

TEST(CFringeTest, AssignmentOperator) {
    CFringeSegment original(2.0);
    original.AddPoint(CDPoint(5, 10));
    original.SetClosed(TRUE);
    
    CFringeSegment assigned(3.0);
    assigned = original;
    
    EXPECT_EQ(2.0, assigned.GetNumber());
    EXPECT_EQ(1, assigned.GetPointCount());
    EXPECT_TRUE(assigned.IsClosed());
    EXPECT_EQ(5.0, assigned.GetPoint(0).x);
}

TEST(CFringeTest, SelfAssignment) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 20));
    
    fringe = fringe;
    
    EXPECT_EQ(1.0, fringe.GetNumber());
    EXPECT_EQ(1, fringe.GetPointCount());
}

// ===== Point Management Tests =====

TEST(CFringeTest, AddPoint) {
    CFringeSegment fringe(1.0);
    
    int idx0 = fringe.AddPoint(CDPoint(10, 20));
    EXPECT_EQ(0, idx0);
    EXPECT_EQ(1, fringe.GetPointCount());
    
    int idx1 = fringe.AddPoint(CDPoint(30, 40));
    EXPECT_EQ(1, idx1);
    EXPECT_EQ(2, fringe.GetPointCount());
    
    EXPECT_EQ(10.0, fringe.GetPoint(0).x);
    EXPECT_EQ(20.0, fringe.GetPoint(0).y);
    EXPECT_EQ(30.0, fringe.GetPoint(1).x);
    EXPECT_EQ(40.0, fringe.GetPoint(1).y);
}

TEST(CFringeTest, InsertPointAtBeginning) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(20, 20));
    fringe.AddPoint(CDPoint(30, 30));
    
    fringe.InsertPoint(0, CDPoint(10, 10));
    
    EXPECT_EQ(3, fringe.GetPointCount());
    EXPECT_EQ(10.0, fringe.GetPoint(0).x);
    EXPECT_EQ(20.0, fringe.GetPoint(1).x);
    EXPECT_EQ(30.0, fringe.GetPoint(2).x);
}

TEST(CFringeTest, InsertPointInMiddle) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(30, 30));
    
    fringe.InsertPoint(1, CDPoint(20, 20));
    
    EXPECT_EQ(3, fringe.GetPointCount());
    EXPECT_EQ(10.0, fringe.GetPoint(0).x);
    EXPECT_EQ(20.0, fringe.GetPoint(1).x);
    EXPECT_EQ(30.0, fringe.GetPoint(2).x);
}

TEST(CFringeTest, InsertPointAtEnd) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(20, 20));
    
    fringe.InsertPoint(2, CDPoint(30, 30));
    
    EXPECT_EQ(3, fringe.GetPointCount());
    EXPECT_EQ(30.0, fringe.GetPoint(2).x);
}

TEST(CFringeTest, InsertPointInvalidIndex) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    int originalCount = fringe.GetPointCount();
    
    // Should not insert - index too large
    fringe.InsertPoint(5, CDPoint(50, 50));
    EXPECT_EQ(originalCount, fringe.GetPointCount());
    
    // Should not insert - negative index
    fringe.InsertPoint(-1, CDPoint(5, 5));
    EXPECT_EQ(originalCount, fringe.GetPointCount());
}

TEST(CFringeTest, RemovePoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(20, 20));
    fringe.AddPoint(CDPoint(30, 30));
    
    fringe.RemovePoint(1);
    
    EXPECT_EQ(2, fringe.GetPointCount());
    EXPECT_EQ(10.0, fringe.GetPoint(0).x);
    EXPECT_EQ(30.0, fringe.GetPoint(1).x);
}

TEST(CFringeTest, RemoveFirstPoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(20, 20));
    
    fringe.RemovePoint(0);
    
    EXPECT_EQ(1, fringe.GetPointCount());
    EXPECT_EQ(20.0, fringe.GetPoint(0).x);
}

TEST(CFringeTest, RemoveLastPoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(20, 20));
    
    fringe.RemovePoint(1);
    
    EXPECT_EQ(1, fringe.GetPointCount());
    EXPECT_EQ(10.0, fringe.GetPoint(0).x);
}

TEST(CFringeTest, RemovePointInvalidIndex) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    int originalCount = fringe.GetPointCount();
    
    fringe.RemovePoint(5);
    EXPECT_EQ(originalCount, fringe.GetPointCount());
    
    fringe.RemovePoint(-1);
    EXPECT_EQ(originalCount, fringe.GetPointCount());
}

TEST(CFringeTest, MovePoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    fringe.AddPoint(CDPoint(20, 20));
    
    fringe.MovePoint(0, CDPoint(15, 15));
    
    EXPECT_EQ(15.0, fringe.GetPoint(0).x);
    EXPECT_EQ(15.0, fringe.GetPoint(0).y);
    EXPECT_EQ(20.0, fringe.GetPoint(1).x);
}

TEST(CFringeTest, MovePointInvalidIndex) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    fringe.MovePoint(5, CDPoint(50, 50));
    EXPECT_EQ(10.0, fringe.GetPoint(0).x); // Unchanged
    
    fringe.MovePoint(-1, CDPoint(5, 5));
    EXPECT_EQ(10.0, fringe.GetPoint(0).x); // Unchanged
}

TEST(CFringeTest, SetPointAndGetPoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    fringe.SetPoint(0, CDPoint(25, 35));
    
    CDPoint p = fringe.GetPoint(0);
    EXPECT_EQ(25.0, p.x);
    EXPECT_EQ(35.0, p.y);
}

TEST(CFringeTest, GetPointInvalidIndex) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    CDPoint p = fringe.GetPoint(5);
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
}

TEST(CFringeTest, AppendPoints) {
    CFringeSegment fringe1(1.0);
    fringe1.AddPoint(CDPoint(10, 10));
    fringe1.AddPoint(CDPoint(20, 20));
    
    CFringeSegment fringe2(1.0);
    fringe2.AddPoint(CDPoint(30, 30));
    fringe2.AddPoint(CDPoint(40, 40));
    
    fringe1.AppendPoints(fringe2);
    
    EXPECT_EQ(4, fringe1.GetPointCount());
    EXPECT_EQ(10.0, fringe1.GetPoint(0).x);
    EXPECT_EQ(20.0, fringe1.GetPoint(1).x);
    EXPECT_EQ(30.0, fringe1.GetPoint(2).x);
    EXPECT_EQ(40.0, fringe1.GetPoint(3).x);
}

// ===== Property Tests =====

TEST(CFringeTest, GetSetNumber) {
    CFringeSegment fringe(1.0);
    EXPECT_EQ(1.0, fringe.GetNumber());
    
    fringe.SetNumber(2.5);
    EXPECT_EQ(2.5, fringe.GetNumber());
}

TEST(CFringeTest, GetSetClosed) {
    CFringeSegment fringe(1.0);
    EXPECT_FALSE(fringe.IsClosed());
    
    fringe.SetClosed(TRUE);
    EXPECT_TRUE(fringe.IsClosed());
    
    fringe.SetClosed(FALSE);
    EXPECT_FALSE(fringe.IsClosed());
}

// ===== Hit Testing Tests =====

TEST(CFringeTest, FindNearestPointWithinTolerance) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    fringe.AddPoint(CDPoint(200, 100));
    fringe.AddPoint(CDPoint(300, 100));
    
    int idx = fringe.FindNearestPoint(CPoint(205, 105), 10);
    EXPECT_EQ(1, idx); // Middle point
}

TEST(CFringeTest, FindNearestPointExactMatch) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    fringe.AddPoint(CDPoint(200, 200));
    
    int idx = fringe.FindNearestPoint(CPoint(200, 200), 10);
    EXPECT_EQ(1, idx);
}

TEST(CFringeTest, FindNearestPointOutsideTolerance) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    fringe.AddPoint(CDPoint(200, 100));
    
    int idx = fringe.FindNearestPoint(CPoint(500, 100), 10);
    EXPECT_EQ(-1, idx); // Too far
}

TEST(CFringeTest, FindNearestPointEmptyFringe) {
    CFringeSegment fringe(1.0);
    
    int idx = fringe.FindNearestPoint(CPoint(100, 100), 10);
    EXPECT_EQ(-1, idx);
}

TEST(CFringeTest, FindNearestPointSelectsClosest) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    fringe.AddPoint(CDPoint(110, 100));
    fringe.AddPoint(CDPoint(105, 100));
    
    int idx = fringe.FindNearestPoint(CPoint(106, 100), 20);
    EXPECT_EQ(2, idx); // Index 2 is closest at x=105
}

TEST(CFringeTest, IsPointOnPolyline) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    fringe.AddPoint(CDPoint(200, 100));
    
    int nearestIdx = -1;
    BOOL result = fringe.IsPointOnPolyline(CPoint(105, 102), 10, nearestIdx);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(0, nearestIdx);
}

TEST(CFringeTest, IsPointOnPolylineOutsideTolerance) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 100));
    
    int nearestIdx = -1;
    BOOL result = fringe.IsPointOnPolyline(CPoint(500, 100), 10, nearestIdx);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(-1, nearestIdx);
}

TEST(CFringeTest, GetBoundingRectEmpty) {
    CFringeSegment fringe(1.0);
    
    CDRect rect = fringe.GetBoundingRect();
    
    EXPECT_EQ(0, rect.left);
    EXPECT_EQ(0, rect.top);
    EXPECT_EQ(0, rect.right);
    EXPECT_EQ(0, rect.bottom);
}

TEST(CFringeTest, GetBoundingRectSinglePoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(100, 200));
    
    CDRect rect = fringe.GetBoundingRect();
    
    EXPECT_EQ(100, rect.left);
    EXPECT_EQ(200, rect.top);
    EXPECT_EQ(100, rect.right);
    EXPECT_EQ(200, rect.bottom);
}

TEST(CFringeTest, GetBoundingRectMultiplePoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(50, 30));
    fringe.AddPoint(CDPoint(150, 20));
    fringe.AddPoint(CDPoint(100, 80));
    
    CDRect rect = fringe.GetBoundingRect();
    
    EXPECT_EQ(50, rect.left);
    EXPECT_EQ(20, rect.top);
    EXPECT_EQ(150, rect.right);
    EXPECT_EQ(80, rect.bottom);
}

// ===== Advanced Operations Tests =====

TEST(CFringeTest, SplitAtMiddle) {
    CFringeSegment original(1.5);
    original.AddPoint(CDPoint(10, 10));
    original.AddPoint(CDPoint(20, 20));
    original.AddPoint(CDPoint(30, 30));
    original.AddPoint(CDPoint(40, 40));
    
    CFringeSegment newFringe = original.Split(2);
    
    EXPECT_EQ(2, original.GetPointCount());
    EXPECT_EQ(10.0, original.GetPoint(0).x);
    EXPECT_EQ(20.0, original.GetPoint(1).x);
    
    EXPECT_EQ(2, newFringe.GetPointCount());
    EXPECT_EQ(30.0, newFringe.GetPoint(0).x);
    EXPECT_EQ(40.0, newFringe.GetPoint(1).x);
    EXPECT_EQ(1.5, newFringe.GetNumber());
}

TEST(CFringeTest, SplitInvalidIndexReturnsEmpty) {
    CFringeSegment original(1.0);
    original.AddPoint(CDPoint(10, 10));
    original.AddPoint(CDPoint(20, 20));
    
    CFringeSegment empty1 = original.Split(0);
    EXPECT_EQ(0, empty1.GetPointCount());
    
    CFringeSegment empty2 = original.Split(2);
    EXPECT_EQ(0, empty2.GetPointCount());
    
    CFringeSegment empty3 = original.Split(-1);
    EXPECT_EQ(0, empty3.GetPointCount());
    
    // Original should be unchanged
    EXPECT_EQ(2, original.GetPointCount());
}

TEST(CFringeTest, GetArcLengthEmpty) {
    CFringeSegment fringe(1.0);
    
    double length = fringe.GetArcLength();
    EXPECT_EQ(0.0, length);
}

TEST(CFringeTest, GetArcLengthSinglePoint) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10, 10));
    
    double length = fringe.GetArcLength();
    EXPECT_EQ(0.0, length);
}

TEST(CFringeTest, GetArcLengthTwoPoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(3, 4)); // Distance = 5
    
    double length = fringe.GetArcLength();
    EXPECT_NEAR(5.0, length, 0.001);
}

TEST(CFringeTest, GetArcLengthMultiplePoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(10, 0)); // Distance = 10
    fringe.AddPoint(CDPoint(10, 10)); // Distance = 10
    
    double length = fringe.GetArcLength();
    EXPECT_NEAR(20.0, length, 0.001);
}

TEST(CFringeTest, GetArcLengthClosedLoop) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(10, 0));
    fringe.AddPoint(CDPoint(10, 10));
    fringe.SetClosed(TRUE);
    
    double length = fringe.GetArcLength();
    // 10 + 10 + sqrt(100+100) ≈ 20 + 14.142
    EXPECT_NEAR(34.142, length, 0.01);
}

TEST(CFringeTest, SubdivideSegmentsNoSubdivisionNeeded) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(5, 0));
    
    fringe.SubdivideSegments(10.0); // Max gap larger than segment
    
    EXPECT_EQ(2, fringe.GetPointCount()); // No subdivision
}

TEST(CFringeTest, SubdivideSegmentsCreatesIntermediatePoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(30, 0));
    
    fringe.SubdivideSegments(10.0);
    
    EXPECT_GT(fringe.GetPointCount(), 2); // Should have intermediate points
    
    // Check spacing is reasonable
    for (int i = 1; i < fringe.GetPointCount(); i++) {
        double dx = fringe.GetPoint(i).x - fringe.GetPoint(i-1).x;
        double dy = fringe.GetPoint(i).y - fringe.GetPoint(i-1).y;
        double dist = sqrt(dx*dx + dy*dy);
        EXPECT_LE(dist, 11.0); // Should be close to maxGap
    }
}

TEST(CFringeTest, SimplifyRemovesRedundantPoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(5, 0));  // On line
    fringe.AddPoint(CDPoint(10, 0)); // On line
    fringe.AddPoint(CDPoint(15, 0)); // On line
    fringe.AddPoint(CDPoint(20, 0));
    
    fringe.Simplify(0.1); // Very small epsilon
    
    // Should keep only endpoints for a straight line
    EXPECT_EQ(2, fringe.GetPointCount());
    EXPECT_EQ(0.0, fringe.GetPoint(0).x);
    EXPECT_EQ(20.0, fringe.GetPoint(1).x);
}

TEST(CFringeTest, SimplifyKeepsSignificantPoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(10, 10)); // Significant deviation
    fringe.AddPoint(CDPoint(20, 0));
    
    fringe.Simplify(0.5); // Small epsilon
    
    // Should keep all points (triangle)
    EXPECT_EQ(3, fringe.GetPointCount());
}

TEST(CFringeTest, SimplifyHandlesEmptyFringe) {
    CFringeSegment fringe(1.0);
    
    fringe.Simplify(1.0);
    
    EXPECT_EQ(0, fringe.GetPointCount());
}

TEST(CFringeTest, SimplifyHandlesTwoPoints) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(0, 0));
    fringe.AddPoint(CDPoint(10, 10));
    
    fringe.Simplify(1.0);
    
    EXPECT_EQ(2, fringe.GetPointCount()); // Always keeps 2 points
}

// ===== Edge Cases =====

TEST(CFringeTest, EmptyFringeOperations) {
    CFringeSegment fringe(1.0);
    
    EXPECT_EQ(0, fringe.GetPointCount());
    EXPECT_EQ(0.0, fringe.GetArcLength());
    
    CDRect rect = fringe.GetBoundingRect();
    EXPECT_EQ(0, rect.Width());
    EXPECT_EQ(0, rect.Height());
}

TEST(CFringeTest, LargeNumberOfPoints) {
    CFringeSegment fringe(1.0);
    
    for (int i = 0; i < 1000; i++) {
        fringe.AddPoint(CDPoint(i, i * 2));
    }
    
    EXPECT_EQ(1000, fringe.GetPointCount());
    EXPECT_EQ(0.0, fringe.GetPoint(0).x);
    EXPECT_EQ(999.0, fringe.GetPoint(999).x);
}

TEST(CFringeTest, NegativeCoordinates) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(-10, -20));
    fringe.AddPoint(CDPoint(-5, -10));
    
    EXPECT_EQ(-10.0, fringe.GetPoint(0).x);
    EXPECT_EQ(-20.0, fringe.GetPoint(0).y);
    
    CDRect rect = fringe.GetBoundingRect();
    EXPECT_EQ(-10, rect.left);
    EXPECT_EQ(-20, rect.top);
}

TEST(CFringeTest, FractionalCoordinates) {
    CFringeSegment fringe(1.0);
    fringe.AddPoint(CDPoint(10.5, 20.75));
    
    EXPECT_NEAR(10.5, fringe.GetPoint(0).x, 0.001);
    EXPECT_NEAR(20.75, fringe.GetPoint(0).y, 0.001);
}

TEST(CFringeTest, NegativeFringeNumber) {
    CFringeSegment fringe(-2.5);
    EXPECT_EQ(-2.5, fringe.GetNumber());
}

TEST(CFringeTest, ZeroFringeNumber) {
    CFringeSegment fringe(0.0);
    EXPECT_EQ(0.0, fringe.GetNumber());
}

TEST(WavefrontBoundingCircleTest, EmptyMaskReturnsInvalidCircle)
{
    aperture::visibility::VisibilityMask visibilityMask(10, 6);
    std::fill(visibilityMask.data.begin(), visibilityMask.data.end(), static_cast<uint8_t>(1));

    std::vector<CFringeSegment> fringeSegments;
    aperture::Bounds bounds = aperture::Bounds::fromMinMax(0.0, 0.0, 10.0, 6.0);

    WavefrontFromContoursInput input(
        bounds,
        visibilityMask,
        fringeSegments,
        10,
        6,
        aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType::SCREEN);

    WavefrontFromContoursContext ctx(input);
    std::vector<char> mask(60, 0);

    WavefrontBoundingCircle circle = ctx.computeMaskBoundingCircle(mask);

    EXPECT_FALSE(circle.valid);
}

TEST(WavefrontBoundingCircleTest, SingleRowVisibleSpanComputesExpectedCircle)
{
    aperture::visibility::VisibilityMask visibilityMask(10, 6);
    std::fill(visibilityMask.data.begin(), visibilityMask.data.end(), static_cast<uint8_t>(1));

    std::vector<CFringeSegment> fringeSegments;
    aperture::Bounds bounds = aperture::Bounds::fromMinMax(0.0, 0.0, 10.0, 6.0);

    WavefrontFromContoursInput input(
        bounds,
        visibilityMask,
        fringeSegments,
        10,
        6,
        aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType::SCREEN);

    WavefrontFromContoursContext ctx(input);
    std::vector<char> mask(60, 0);

    const int row = 2;
    for (int col = 2; col <= 7; ++col)
    {
        mask[row * 10 + col] = 1;
    }

    WavefrontBoundingCircle circle = ctx.computeMaskBoundingCircle(mask);

    ASSERT_TRUE(circle.valid);
    EXPECT_NEAR(4.5, circle.center.x, 1e-9);
    EXPECT_NEAR(2.0, circle.center.y, 1e-9);
    EXPECT_NEAR(2.5, circle.radius, 1e-9);
}

TEST(WavefrontBoundingCircleTest, MultiRowVisibleBlockComputesExpectedCircle)
{
    aperture::visibility::VisibilityMask visibilityMask(10, 6);
    std::fill(visibilityMask.data.begin(), visibilityMask.data.end(), static_cast<uint8_t>(1));

    std::vector<CFringeSegment> fringeSegments;
    aperture::Bounds bounds = aperture::Bounds::fromMinMax(0.0, 0.0, 10.0, 6.0);

    WavefrontFromContoursInput input(
        bounds,
        visibilityMask,
        fringeSegments,
        10,
        6,
        aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType::SCREEN);

    WavefrontFromContoursContext ctx(input);
    std::vector<char> mask(60, 0);

    for (int row = 1; row <= 4; ++row)
    {
        for (int col = 3; col <= 8; ++col)
        {
            mask[row * 10 + col] = 1;
        }
    }

    WavefrontBoundingCircle circle = ctx.computeMaskBoundingCircle(mask);

    ASSERT_TRUE(circle.valid);
    EXPECT_NEAR(5.5, circle.center.x, 1e-9);
    EXPECT_NEAR(2.5, circle.center.y, 1e-9);
    EXPECT_NEAR(std::sqrt(8.5), circle.radius, 1e-9);
}

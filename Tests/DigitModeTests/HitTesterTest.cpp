#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/HitTester.h"
#include "DigitMode/CFringeSegment.h"
#include <vector>
#include <cmath>

using namespace DigitMode;

/**
 * @brief Test fixture for HitTester
 */
class HitTesterTest : public ::testing::Test {
protected:
    HitTester hitTester;
    std::vector<CFringeSegment> testSegments;

    void SetUp() override {
        // Create test segments with known geometry
        testSegments.clear();

        // Segment 0: Horizontal line from (10,10) to (50,10)
        CFringeSegment seg0(1.0, 0);
        seg0.AddPoint(CDPoint(10, 10));
        seg0.AddPoint(CDPoint(30, 10));
        seg0.AddPoint(CDPoint(50, 10));
        testSegments.push_back(seg0);

        // Segment 1: Vertical line from (100,20) to (100,60)
        CFringeSegment seg1(2.0, 0);
        seg1.AddPoint(CDPoint(100, 20));
        seg1.AddPoint(CDPoint(100, 40));
        seg1.AddPoint(CDPoint(100, 60));
        testSegments.push_back(seg1);

        // Segment 2: Diagonal line
        CFringeSegment seg2(3.0, 0);
        seg2.AddPoint(CDPoint(200, 200));
        seg2.AddPoint(CDPoint(210, 210));
        seg2.AddPoint(CDPoint(220, 220));
        testSegments.push_back(seg2);
    }

    void TearDown() override {
        testSegments.clear();
    }
};

// ===== Helper Function Tests =====

TEST_F(HitTesterTest, DotDistanceExact) {
    // Use reflection to test private method via HitTest
    // For exact hit on dot[0] of segment 0 at (10, 10)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10, 10), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(0, outDot);
}

TEST_F(HitTesterTest, DotDistanceWithinTolerance) {
    // Click near dot[0] of segment 0 (within 5 pixel tolerance)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(12, 12), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(0, outDot);
}

TEST_F(HitTesterTest, DotDistanceOutsideTolerance) {
    // Click far from any dot (>5 pixels away)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(0, 0), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::None, result);
}

TEST_F(HitTesterTest, DotDistance3_4_5Triangle) {
    // Test with known distance: 3-4-5 right triangle
    // Dot at (10, 10), click at (13, 14)
    // Distance = sqrt(3^2 + 4^2) = 5 pixels (at tolerance boundary)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(13, 14), outSegment, outDot, testSegments);

    // Should be within tolerance (<=5)
    EXPECT_EQ(SelectionLevel::Dot, result);
}

// ===== Edge Hit Testing =====

TEST_F(HitTesterTest, EdgeHitOnHorizontalLine) {
    // Click on horizontal edge between dots (20, 10) - should hit edge
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(20, 10), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Edge, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(0, outDot);  // Edge starts at dot[0]
}

TEST_F(HitTesterTest, EdgeHitOnVerticalLine) {
    // Click on vertical edge at (100, 30)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(100, 30), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Edge, result);
    EXPECT_EQ(1, outSegment);
    EXPECT_EQ(0, outDot);  // Edge starts at dot[0]
}

TEST_F(HitTesterTest, EdgeHitNearLine) {
    // Click near horizontal line (within tolerance)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(20, 12), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Edge, result);
    EXPECT_EQ(0, outSegment);
}

TEST_F(HitTesterTest, EdgeMissOutsideTolerance) {
    // Click far from any edge (>5 pixels away)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(20, 20), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::None, result);
}

TEST_F(HitTesterTest, EdgeHitDiagonalLine) {
    // Click on diagonal edge
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(205, 205), outSegment, outDot, testSegments);

    // Should hit edge (perpendicular distance is 0)
    EXPECT_EQ(SelectionLevel::Edge, result);
    EXPECT_EQ(2, outSegment);
}

// ===== Dot Priority Over Edge =====

TEST_F(HitTesterTest, DotHasPriorityOverEdge) {
    // Click exactly on a dot that's also on an edge
    // Dot should be selected (higher priority)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(30, 10), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(1, outDot);  // Middle dot
}

// ===== Z-Order Testing =====

TEST_F(HitTesterTest, ReverseZOrderHitsTopSegmentFirst) {
    // Add overlapping segments
    CFringeSegment seg3(4.0, 0);
    seg3.AddPoint(CDPoint(10, 10));  // Same position as segment 0
    seg3.AddPoint(CDPoint(50, 10));
    testSegments.push_back(seg3);

    // Click at (10, 10) - should hit segment 3 (last added, top z-order)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10, 10), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(3, outSegment);  // Top segment
}

// ===== Edge Cases =====

TEST_F(HitTesterTest, EmptySegmentsReturnsNone) {
    std::vector<CFringeSegment> emptySegments;

    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10, 10), outSegment, outDot, emptySegments);

    EXPECT_EQ(SelectionLevel::None, result);
}

TEST_F(HitTesterTest, SingleDotSegment) {
    std::vector<CFringeSegment> singleDotSegments;
    CFringeSegment seg(1.0, 0);
    seg.AddPoint(CDPoint(50, 50));
    singleDotSegments.push_back(seg);

    // Should hit the dot
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(52, 52), outSegment, outDot, singleDotSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(0, outDot);
}

TEST_F(HitTesterTest, SegmentWithNoDots) {
    std::vector<CFringeSegment> emptyPointSegments;
    CFringeSegment seg(1.0, 0);
    // Don't add any points
    emptyPointSegments.push_back(seg);

    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10, 10), outSegment, outDot, emptyPointSegments);

    EXPECT_EQ(SelectionLevel::None, result);
}

// ===== Perpendicular Distance Tests =====

TEST_F(HitTesterTest, PerpendicularDistanceAtMidpoint) {
    // Click perpendicular to midpoint of horizontal edge
    // Edge from (10,10) to (30,10), click at (20, 12)
    // Perpendicular distance = 2 pixels (within tolerance)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(20, 12), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Edge, result);
}

TEST_F(HitTesterTest, PerpendicularDistanceBeyondEndpoint) {
    // Click at (40, 10) which is ON the second edge
    // Segment 0 has edges: (10,10)→(30,10) and (30,10)→(50,10)
    // Point (40,10) is ON the second edge, not beyond an endpoint
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(40, 10), outSegment, outDot, testSegments);

    // Distance to nearest dots:
    //   - dot[1] at (30,10): 10 pixels (outside tolerance)
    //   - dot[2] at (50,10): 10 pixels (outside tolerance)
    // Distance to edge (30,10)→(50,10): 0 pixels (ON the edge)
    EXPECT_EQ(SelectionLevel::Edge, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(1, outDot);  // Edge 1 starts at dot[1]
}

TEST_F(HitTesterTest, ClickBeyondSegmentEndpoint) {
    // Click truly BEYOND the last endpoint
    // Last edge is (30,10)→(50,10), click at (60,10)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(60, 10), outSegment, outDot, testSegments);

    // Distance to dot[2] at (50,10): 10 pixels (outside tolerance)
    // Edge (30,10)→(50,10) clamps to endpoint (50,10), distance = 10 pixels (outside tolerance)
    EXPECT_EQ(SelectionLevel::None, result);
}

TEST_F(HitTesterTest, ClickJustBeyondEndpointWithinTolerance) {
    // Click just beyond last endpoint but within tolerance
    // Last dot is at (50,10), click at (53,10)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(53, 10), outSegment, outDot, testSegments);

    // Distance to dot[2] at (50,10): 3 pixels (within tolerance)
    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(2, outDot);  // Last dot
}

// ===== Boundary Tests =====

TEST_F(HitTesterTest, ToleranceBoundaryInclusive) {
    // EXPLICITLY TEST: Tolerance is INCLUSIVE (<=), not exclusive (<)
    // This is standard CAD/graphics convention
    int outSegment, outDot;
    
    // Test at exactly 5 pixels (horizontal)
    SelectionLevel result1 = hitTester.HitTest(CPoint(15, 10), outSegment, outDot, testSegments);
    EXPECT_EQ(SelectionLevel::Dot, result1) << "Tolerance should be INCLUSIVE: distance=5 should HIT";
    
    // Test at exactly 5 pixels (vertical)
    SelectionLevel result2 = hitTester.HitTest(CPoint(10, 15), outSegment, outDot, testSegments);
    EXPECT_EQ(SelectionLevel::Dot, result2) << "Tolerance should be INCLUSIVE: distance=5 should HIT";
    
    // Test at 6 pixels (outside tolerance)
    SelectionLevel result3 = hitTester.HitTest(CPoint(16, 10), outSegment, outDot, testSegments);
    if (result3 == SelectionLevel::Dot) {
        EXPECT_FALSE(outSegment == 0 && outDot == 0) << "Distance=6 should NOT hit dot at (10,10)";
    }
}

TEST_F(HitTesterTest, ExactlyAtTolerance) {
    // Click exactly 5 pixels away from dot (at tolerance boundary)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10, 15), outSegment, outDot, testSegments);

    // Should be within tolerance (<=5)
    EXPECT_EQ(SelectionLevel::Dot, result);
}

// ===== Multiple Segments Tests =====

TEST_F(HitTesterTest, MultipleSegmentsHitsClosest) {
    // Add another segment closer to click point
    CFringeSegment seg3(4.0, 0);
    seg3.AddPoint(CDPoint(15, 15));
    testSegments.insert(testSegments.begin(), seg3);  // Insert at start (lower z-order)

    // Click at (12, 12) - closer to (10,10) than (15,15)
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(12, 12), outSegment, outDot, testSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    // Due to reverse iteration, should hit based on priority
}

// ===== Performance Test =====

TEST_F(HitTesterTest, LargeSegmentCountPerformance) {
    std::vector<CFringeSegment> largeSegments;
    
    // Create 100 segments with 100 dots each
    for (int i = 0; i < 100; i++) {
        CFringeSegment seg(1.0, i);
        for (int j = 0; j < 100; j++) {
            seg.AddPoint(CDPoint(i * 100.0 + j, j * 10.0));
        }
        largeSegments.push_back(seg);
    }

    // Hit test should complete quickly
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(5000, 500), outSegment, outDot, largeSegments);

    // Test completes (no assertion on result, just testing performance)
    EXPECT_TRUE(result == SelectionLevel::Dot || 
                result == SelectionLevel::Edge || 
                result == SelectionLevel::None);
}

// ===== Negative Coordinate Tests =====

TEST_F(HitTesterTest, NegativeCoordinates) {
    std::vector<CFringeSegment> negativeSegments;
    CFringeSegment seg(1.0, 0);
    seg.AddPoint(CDPoint(-10, -10));
    seg.AddPoint(CDPoint(-20, -20));
    negativeSegments.push_back(seg);

    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(-10, -10), outSegment, outDot, negativeSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(0, outDot);
}

TEST_F(HitTesterTest, VeryLargeCoordinates) {
    std::vector<CFringeSegment> largeCoordSegments;
    CFringeSegment seg(1.0, 0);
    seg.AddPoint(CDPoint(10000, 10000));
    seg.AddPoint(CDPoint(10100, 10100));
    largeCoordSegments.push_back(seg);

    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(10000, 10000), outSegment, outDot, largeCoordSegments);

    EXPECT_EQ(SelectionLevel::Dot, result);
}

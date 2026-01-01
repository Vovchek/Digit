/// <summary>
/// Google Test suite for ConnectSegments function
/// Tests the algorithm that connects broken line segments into continuous contour polygons
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/CalcContour.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Include/Int_Cons.h"
#include <cmath>

// ============================================================================
// Test Fixture for ConnectSegments
// ============================================================================

class ConnectSegmentsTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;
    static constexpr double DEFAULT_EPS = 0.1;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }

    // Helper to compare doubles with tolerance
    bool IsNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }

    // Helper to verify polygon is closed
    bool IsClosed(const XYPolygon& polygon, double eps = DEFAULT_EPS) const {
        if (polygon.GetSize() < 2) return false;
        return Distance(polygon[0], polygon[polygon.GetSize() - 1]) < eps;
    }

    // Helper to create a broken line from points
    XYBrokenLine CreateBrokenLine(const std::vector<XYPoint>& points) const {
        XYBrokenLine line;
        for (const auto& pt : points) {
            line.Add(pt);
        }
        return line;
    }

    // Helper to count total points in all contours
    int CountTotalPoints(const CArrayXYPolygon& contours) const {
        int total = 0;
        for (int i = 0; i < contours.GetSize(); i++) {
            total += contours[i].GetSize();
        }
        return total;
    }
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, EmptyInput_ProducesEmptyOutput) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0);
}

TEST_F(ConnectSegmentsTest, SingleSegment_ProducesOnePolygon) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    XYBrokenLine segment = CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0),
        XYPoint(1.0, 1.0)
    });
    arrBLn.Add(segment);
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    // Contour may be closed if endpoints are within 2*eps
    EXPECT_GE(arrCont[0].GetSize(), 3);
}

TEST_F(ConnectSegmentsTest, TwoDisconnectedSegments_BothFilteredAsDegenerate) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Two 2-point segments far apart (both degenerate)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(10.0, 10.0),
        XYPoint(11.0, 10.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two disconnected 2-point segments should both be filtered as degenerate";
}

// ============================================================================
// Segment Connection Tests - Forward Connections
// ============================================================================

TEST_F(ConnectSegmentsTest, TwoSegments_ConnectAtEnd_AppendForward) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Segment 1: [0,0] -> [1,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    // Segment 2: [1,0] -> [2,0] (connects to end of segment 1)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GE(arrCont[0].GetSize(), 3); // At least 3 points after merging
}

TEST_F(ConnectSegmentsTest, TwoSegments_ConnectAtEnd_AppendReverse) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Segment 1: [0,0] -> [1,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    // Segment 2: [2,0] -> [1,0] (end connects to end of segment 1, needs reverse)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GE(arrCont[0].GetSize(), 3);
}

TEST_F(ConnectSegmentsTest, TwoSegments_ConnectAtStart_PrependForward) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Segment 1: [1,0] -> [2,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    // Segment 2: [0,0] -> [1,0] (end connects to start of segment 1)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GE(arrCont[0].GetSize(), 3);
}

TEST_F(ConnectSegmentsTest, TwoSegments_ConnectAtStart_PrependReverse) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Segment 1: [1,0] -> [2,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    // Segment 2: [1,0] -> [0,0] (start connects to start of segment 1, needs reverse)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GE(arrCont[0].GetSize(), 3);
}

// ============================================================================
// Multi-Segment Connection Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, ThreeSegments_FormClosedTriangle) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create three segments forming a triangle
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.5, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, 1.0),
        XYPoint(0.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(ConnectSegmentsTest, FourSegments_FormClosedRectangle) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create four segments forming a rectangle
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),
        XYPoint(2.0, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 1.0),
        XYPoint(0.0, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 1.0),
        XYPoint(0.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(ConnectSegmentsTest, SegmentsInRandomOrder_StillConnect) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Add segments in non-sequential order
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 1.0),
        XYPoint(0.0, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 1.0),
        XYPoint(0.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),
        XYPoint(2.0, 1.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

// ============================================================================
// Tolerance Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, EndpointsWithinTolerance_AreConnected) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // Segment 1: [0,0] -> [1,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    // Segment 2: [1.05,0] -> [2,0] (0.05 < eps, should connect)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.05, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 1); // Should connect
}

TEST_F(ConnectSegmentsTest, EndpointsBeyondTolerance_AreNotConnected) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // Segment 1: [0,0] -> [1,0]
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    // Segment 2: [1.2,0] -> [2,0] (0.2 > eps, should not connect)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.2, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two disconnected 2-point segments should both be filtered as degenerate";
}

TEST_F(ConnectSegmentsTest, SmallTolerance_RequiresPreciseMatch) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.01;
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.005, 0.0),
        XYPoint(2.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 1); // 0.005 < 0.01, should connect
}

TEST_F(ConnectSegmentsTest, LargeTolerance_ConnectsDistantPoints) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 1.0;
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.5, 0.0),
        XYPoint(2.5, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 1); // 0.5 < 1.0, should connect
}

// ============================================================================
// Contour Closure Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, ClosedContour_DuplicatesFirstPoint) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // Triangle with endpoints very close
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.5, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, 1.0),
        XYPoint(0.0, 0.0)  // Exactly matches start
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    
    // Check that first and last points are the same
    int size = arrCont[0].GetSize();
    EXPECT_TRUE(Distance(arrCont[0][0], arrCont[0][size-1]) < TOLERANCE);
}

TEST_F(ConnectSegmentsTest, AlmostClosedContour_ClosesWithinTolerance) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.2;
    
    // Triangle with endpoints close but not exact
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.5, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, 1.0),
        XYPoint(0.05, 0.05)  // Close to (0,0) within 2*eps
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    
    // Should be closed
    EXPECT_TRUE(IsClosed(arrCont[0], eps * 2.5));
}

// NOTE: XYPolygon constructor automatically closes polygons, so we can't test
// for truly open contours. The constructor adds the first point at the end
// if the distance between first and last points is > PRECISION.

// ============================================================================
// Complex Scenarios
// ============================================================================

TEST_F(ConnectSegmentsTest, MultipleContours_TwoSeparateTriangles) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // First triangle
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.5, 1.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, 1.0),
        XYPoint(0.0, 0.0)
    }));
    
    // Second triangle (far away)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(10.0, 10.0),
        XYPoint(11.0, 10.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(11.0, 10.0),
        XYPoint(10.5, 11.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(10.5, 11.0),
        XYPoint(10.0, 10.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 2);
    EXPECT_TRUE(IsClosed(arrCont[0]));
    EXPECT_TRUE(IsClosed(arrCont[1]));
}

TEST_F(ConnectSegmentsTest, BrokenCircle_ReconnectsSegments) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.15;
    
    // Create a circle broken into 4 arcs
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.707, 0.707)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.707, 0.707),
        XYPoint(0.0, 1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 1.0),
        XYPoint(-0.707, 0.707)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.707, 0.707),
        XYPoint(-1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-1.0, 0.0),
        XYPoint(-0.707, -0.707)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.707, -0.707),
        XYPoint(0.0, -1.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, -1.0),
        XYPoint(0.707, -0.707)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.707, -0.707),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(ConnectSegmentsTest, ManySmallSegments_FormValidRectangle) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // Create segments forming a rectangle (non-degenerate, has area)
    // Bottom edge
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 0.0), XYPoint(2.5, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(2.5, 0.0), XYPoint(5.0, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(5.0, 0.0), XYPoint(7.5, 0.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(7.5, 0.0), XYPoint(10.0, 0.0) }));
    
    // Right edge
    arrBLn.Add(CreateBrokenLine({ XYPoint(10.0, 0.0), XYPoint(10.0, 2.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(10.0, 2.0), XYPoint(10.0, 4.0) }));
    
    // Top edge
    arrBLn.Add(CreateBrokenLine({ XYPoint(10.0, 4.0), XYPoint(7.5, 4.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(7.5, 4.0), XYPoint(5.0, 4.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(5.0, 4.0), XYPoint(2.5, 4.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(2.5, 4.0), XYPoint(0.0, 4.0) }));
    
    // Left edge
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 4.0), XYPoint(0.0, 2.0) }));
    arrBLn.Add(CreateBrokenLine({ XYPoint(0.0, 2.0), XYPoint(0.0, 0.0) }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    EXPECT_EQ(arrCont.GetSize(), 1) 
        << "Rectangle segments should form single non-degenerate polygon";
    EXPECT_GE(arrCont[0].GetSize(), 12) << "Should have at least 12 points";
    EXPECT_FALSE(arrCont[0].isDegenerate()) << "Rectangle should not be degenerate";
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ConnectSegmentsTest, SinglePointSegment_IsFilteredAsDegenerate) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Single-point segment should be filtered as degenerate (< 3 points)";
}

TEST_F(ConnectSegmentsTest, TwoPointSegment_IsFilteredAsDegenerate) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Two-point segment (line) should be filtered as degenerate (zero area)";
}

TEST_F(ConnectSegmentsTest, IdenticalSegments_ProducesOneContour) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Two identical segments
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    // Both segments should be processed
    EXPECT_GE(arrCont.GetSize(), 1);
}

TEST_F(ConnectSegmentsTest, SegmentWithManyPoints_PreservesPoints) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create a segment with many intermediate points
    XYBrokenLine segment;
    for (int i = 0; i <= 100; i++) {
        segment.Add(XYPoint(i * 0.1, 0.0));
    }
    arrBLn.Add(segment);
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    // All points preserved, plus possibly a closing point
    EXPECT_GE(arrCont[0].GetSize(), 101);
}

// ============================================================================
// Segment Reversal Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, BackwardSegment_IsReversedForConnection) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Segment 1: forward
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(0.5, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    // Segment 2: backward (needs reversal to append)
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),
        XYPoint(1.5, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GE(arrCont[0].GetSize(), 5); // Combined points
}

TEST_F(ConnectSegmentsTest, MixedDirectionSegments_AllConnect) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Mix of forward and backward segments
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 0.0),
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),  // Backward
        XYPoint(1.0, 0.0)
    }));
    
    arrBLn.Add(CreateBrokenLine({
        XYPoint(2.0, 0.0),
        XYPoint(3.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 1);
}

// ============================================================================
// Performance and Stress Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, ManySegments_HandlesEfficiently) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create 100 sequential segments
    for (int i = 0; i < 100; i++) {
        arrBLn.Add(CreateBrokenLine({
            XYPoint(i * 1.0, 0.0),
            XYPoint((i + 1) * 1.0, 0.0)
        }));
    }
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 1);
}

TEST_F(ConnectSegmentsTest, ManyDisconnectedSegments_AllFilteredAsDegenerate) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create 50 separate 2-point segments (all degenerate)
    for (int i = 0; i < 50; i++) {
        arrBLn.Add(CreateBrokenLine({
            XYPoint(i * 10.0, i * 10.0),
            XYPoint(i * 10.0 + 1.0, i * 10.0)
        }));
    }
    
    ConnectSegments(arrBLn, arrCont, DEFAULT_EPS);
    
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "50 disconnected 2-point segments should all be filtered as degenerate";
}

// ============================================================================
// Geometric Pattern Tests
// ============================================================================

TEST_F(ConnectSegmentsTest, Star_FormsSingleContour) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // 5-pointed star outer points
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, 1.0),
        XYPoint(0.2, 0.2)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.2, 0.2),
        XYPoint(1.0, 0.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.2, -0.2)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.2, -0.2),
        XYPoint(0.0, -1.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.0, -1.0),
        XYPoint(-0.2, -0.2)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.2, -0.2),
        XYPoint(-1.0, 0.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-1.0, 0.0),
        XYPoint(-0.2, 0.2)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.2, 0.2),
        XYPoint(0.0, 1.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(ConnectSegmentsTest, Hexagon_SixSegments_FormsClosed) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    double eps = 0.1;
    
    // Regular hexagon
    arrBLn.Add(CreateBrokenLine({
        XYPoint(1.0, 0.0),
        XYPoint(0.5, 0.866)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, 0.866),
        XYPoint(-0.5, 0.866)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.5, 0.866),
        XYPoint(-1.0, 0.0)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-1.0, 0.0),
        XYPoint(-0.5, -0.866)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(-0.5, -0.866),
        XYPoint(0.5, -0.866)
    }));
    arrBLn.Add(CreateBrokenLine({
        XYPoint(0.5, -0.866),
        XYPoint(1.0, 0.0)
    }));
    
    ConnectSegments(arrBLn, arrCont, eps);
    
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

// ============================================================================
// Validation Tests
// ===================================================

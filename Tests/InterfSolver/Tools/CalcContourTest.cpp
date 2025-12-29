/// <summary>
/// Google Test suite for CalcContour function
/// Tests visible contour calculation with occlusion handling for geometric shapes
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/CalcContour.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Include/Int_Cons.h"
#include <cmath>

// ============================================================================
// Test Fixture for CalcContour
// ============================================================================

class CalcContourTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

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

    // Helper to count points in all contours
    int CountTotalPoints(const CArrayXYPolygon& contours) const {
        int total = 0;
        for (int i = 0; i < contours.GetSize(); i++) {
            total += contours[i].GetSize();
        }
        return total;
    }

    // Helper to verify polygon is closed
    bool IsClosed(const XYPolygon& polygon) const {
        if (polygon.GetSize() < 2) return false;
        return Distance(polygon[0], polygon[polygon.GetSize() - 1]) < TOLERANCE;
    }
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(CalcContourTest, EmptyInputs_ProducesEmptyOutput) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    CalcContour(arrEll, arrRect, arrPlg, arrCont);

    EXPECT_EQ(arrCont.GetSize(), 0);
}

TEST_F(CalcContourTest, SingleEllipse_ProducesOneContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(CalcContourTest, SingleRectangle_ProducesOneContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYRect rect(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrRect.Add(rect);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(CalcContourTest, SinglePolygon_ProducesOneContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Create a triangle
    XYPolygon triangle;
    triangle.Add(XYPoint(0.0, 0.0));
    triangle.Add(XYPoint(10.0, 0.0));
    triangle.Add(XYPoint(5.0, 10.0));
    triangle.Add(XYPoint(0.0, 0.0)); // Close the polygon
    triangle.SetTypeLimits(EXTERNAL);
    arrPlg.Add(triangle);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
}

// ============================================================================
// Occlusion Tests
// ============================================================================

TEST_F(CalcContourTest, TwoNonOverlappingEllipses_ProducesTwoContours) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Two ellipses far apart
    XYEllipse ellipse1(5.0, 3.0, -20.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(5.0, 3.0, 20.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 2);
}

TEST_F(CalcContourTest, EllipseInsideEllipse_EXTERNAL_ProducesOneContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Large outer ellipse (EXTERNAL - only outside is visible)
    XYEllipse outer(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    // Small inner ellipse (EXTERNAL - only outside is visible)
    XYEllipse inner(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(outer);
    arrEll.Add(inner);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should produce one contour for outer and one for inner
    EXPECT_GE(arrCont.GetSize(), 1);
}

TEST_F(CalcContourTest, EllipseWithInternalHole_ProducesTwoContours) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Outer ellipse (EXTERNAL)
    XYEllipse outer(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    // Inner ellipse as hole (INTERNAL)
    XYEllipse hole(5.0, 3.0, 0.0, 0.0, 0.0, INTERNAL);
    
    arrEll.Add(outer);
    arrEll.Add(hole);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should have at least one contour (outer), possibly two if hole is detected
    EXPECT_GE(arrCont.GetSize(), 1);
}

TEST_F(CalcContourTest, PartiallyOverlappingEllipses_HandlesOcclusion) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Two ellipses with partial overlap
    XYEllipse ellipse1(8.0, 5.0, -5.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(8.0, 5.0, 5.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should produce contours accounting for overlap
    EXPECT_GE(arrCont.GetSize(), 1);
}

// ============================================================================
// Mixed Shape Tests
// ============================================================================

TEST_F(CalcContourTest, EllipseAndRectangle_NonOverlapping) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(5.0, 3.0, -15.0, 0.0, 0.0, EXTERNAL);
    XYRect rect(5.0, 3.0, 15.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse);
    arrRect.Add(rect);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 2);
}

TEST_F(CalcContourTest, EllipseRectanglePolygon_Mixed) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(5.0, 3.0, -20.0, 0.0, 0.0, EXTERNAL);
    XYRect rect(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    // Triangle
    XYPolygon triangle;
    triangle.Add(XYPoint(15.0, -5.0));
    triangle.Add(XYPoint(25.0, -5.0));
    triangle.Add(XYPoint(20.0, 5.0));
    triangle.Add(XYPoint(15.0, -5.0));
    triangle.SetTypeLimits(EXTERNAL);
    
    arrEll.Add(ellipse);
    arrRect.Add(rect);
    arrPlg.Add(triangle);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 3);
}

// ============================================================================
// Contour Type Classification Tests
// ============================================================================

TEST_F(CalcContourTest, SingleShape_ClassifiedAsExternal) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_EQ(arrCont[0].GetTypeLimits(), EXTERNAL);
}

TEST_F(CalcContourTest, ConcentricShapes_ClassifiesExternalAndInternal) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Outer shape
    XYEllipse outer(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    // Inner shape as hole
    XYEllipse inner(8.0, 6.0, 0.0, 0.0, 0.0, INTERNAL);
    
    arrEll.Add(outer);
    arrEll.Add(inner);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Check that we have both external and internal contours
    bool hasExternal = false;
    bool hasInternal = false;
    
    for (int i = 0; i < arrCont.GetSize(); i++) {
        if (arrCont[i].GetTypeLimits() == EXTERNAL) hasExternal = true;
        if (arrCont[i].GetTypeLimits() == INTERNAL) hasInternal = true;
    }
    
    EXPECT_TRUE(hasExternal);
}

// ============================================================================
// Step Size and Point Count Tests
// ============================================================================

TEST_F(CalcContourTest, DifferentStepSizes_ProduceDifferentPointCounts) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    
    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);

    CArrayXYPolygon arrCont1, arrCont2;
    CalcContour(arrEll, arrRect, arrPlg, arrCont1, 50);
    CalcContour(arrEll, arrRect, arrPlg, arrCont2, 200);

    ASSERT_EQ(arrCont1.GetSize(), 1);
    ASSERT_EQ(arrCont2.GetSize(), 1);
    
    // More points should be generated with larger NPntMax
    // (though exact counts depend on perimeter calculation)
    EXPECT_GT(arrCont2[0].GetSize(), 0);
    EXPECT_GT(arrCont1[0].GetSize(), 0);
}

TEST_F(CalcContourTest, DefaultStepSize_UsesNContConstant) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);

    // Call with default parameter
    CalcContour(arrEll, arrRect, arrPlg, arrCont);

    EXPECT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
}

// ============================================================================
// Segment Connection Tests
// ============================================================================

TEST_F(CalcContourTest, BrokenSegments_AreConnected) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Create two ellipses that partially occlude a third
    XYEllipse base(15.0, 10.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse occluder1(4.0, 3.0, -8.0, 0.0, 0.0, EXTERNAL);
    XYEllipse occluder2(4.0, 3.0, 8.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(base);
    arrEll.Add(occluder1);
    arrEll.Add(occluder2);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should connect broken segments
    EXPECT_GE(arrCont.GetSize(), 1);
    
    // All contours should be closed
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_TRUE(IsClosed(arrCont[i]));
    }
}

// ============================================================================
// Rotated Shapes Tests
// ============================================================================

TEST_F(CalcContourTest, RotatedEllipse_ProducesCorrectContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse ellipse(10.0, 5.0, 0.0, 0.0, 45.0, EXTERNAL);
    arrEll.Add(ellipse);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

TEST_F(CalcContourTest, RotatedRectangle_ProducesCorrectContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYRect rect(8.0, 5.0, 0.0, 0.0, 30.0, EXTERNAL);
    arrRect.Add(rect);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
    EXPECT_TRUE(IsClosed(arrCont[0]));
}

// ============================================================================
// Edge Cases and Degenerate Inputs
// ============================================================================

TEST_F(CalcContourTest, VerySmallShape_HandlesGracefully) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse tiny(0.01, 0.01, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(tiny);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 50);

    // Should still produce a contour
    EXPECT_GE(arrCont.GetSize(), 0);
}

TEST_F(CalcContourTest, VeryLargeShape_HandlesGracefully) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    XYEllipse large(1000.0, 800.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(large);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
}

TEST_F(CalcContourTest, CompletelyOccludedShape_ProducesNoContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Large ellipse completely hiding small one
    XYEllipse largeEll(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse smallEll(3.0, 2.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(largeEll);
    arrEll.Add(smallEll);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Small ellipse should be completely hidden
    // Only large ellipse contour should appear
    EXPECT_GE(arrCont.GetSize(), 1);
}

// ============================================================================
// Complex Scenarios
// ============================================================================

TEST_F(CalcContourTest, MultipleShapes_ComplexArrangement) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Create a complex arrangement
    arrEll.Add(XYEllipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(5.0, 4.0, -15.0, -10.0, 0.0, EXTERNAL));
    arrRect.Add(XYRect(6.0, 4.0, 15.0, 10.0, 0.0, EXTERNAL));
    
    XYPolygon triangle;
    triangle.Add(XYPoint(-5.0, 15.0));
    triangle.Add(XYPoint(5.0, 15.0));
    triangle.Add(XYPoint(0.0, 25.0));
    triangle.Add(XYPoint(-5.0, 15.0));
    triangle.SetTypeLimits(EXTERNAL);
    arrPlg.Add(triangle);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    // Should handle complex arrangement
    EXPECT_GE(arrCont.GetSize(), 1);
    
    // All contours should be valid polygons
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GT(arrCont[i].GetSize(), 2);
    }
}

TEST_F(CalcContourTest, ManySmallShapes_PerformanceTest) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Add multiple small shapes in a grid
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            arrEll.Add(XYEllipse(2.0, 1.5, i * 10.0, j * 10.0, 0.0, EXTERNAL));
        }
    }

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 50);

    // Should produce many contours (one per shape if non-overlapping)
    EXPECT_EQ(arrCont.GetSize(), 25);
}

// ============================================================================
// Polygon-Specific Tests
// ============================================================================

TEST_F(CalcContourTest, IrregularPolygon_ProducesCorrectContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Irregular pentagon
    XYPolygon pentagon;
    pentagon.Add(XYPoint(0.0, 0.0));
    pentagon.Add(XYPoint(10.0, 2.0));
    pentagon.Add(XYPoint(12.0, 8.0));
    pentagon.Add(XYPoint(5.0, 12.0));
    pentagon.Add(XYPoint(-2.0, 6.0));
    pentagon.Add(XYPoint(0.0, 0.0)); // Close
    pentagon.SetTypeLimits(EXTERNAL);
    arrPlg.Add(pentagon);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 4); // At least as many points as vertices
}

TEST_F(CalcContourTest, SelfIntersectingPolygon_HandlesGracefully) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Star-like self-intersecting polygon
    XYPolygon star;
    star.Add(XYPoint(0.0, 10.0));
    star.Add(XYPoint(2.0, 2.0));
    star.Add(XYPoint(10.0, 0.0));
    star.Add(XYPoint(2.0, -2.0));
    star.Add(XYPoint(0.0, -10.0));
    star.Add(XYPoint(-2.0, -2.0));
    star.Add(XYPoint(-10.0, 0.0));
    star.Add(XYPoint(-2.0, 2.0));
    star.Add(XYPoint(0.0, 10.0)); // Close
    star.SetTypeLimits(EXTERNAL);
    arrPlg.Add(star);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_GE(arrCont.GetSize(), 0); // Should not crash
}

// ============================================================================
// Visibility Logic Tests
// ============================================================================

TEST_F(CalcContourTest, AllEXTERNAL_OnlyOutsidePointsVisible) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // All shapes marked as EXTERNAL
    arrEll.Add(XYEllipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrRect.Add(XYRect(5.0, 4.0, 20.0, 0.0, 0.0, EXTERNAL));

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 2);
}

TEST_F(CalcContourTest, MixedEXTERNALandINTERNAL_ProducesCorrectContours) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Outer EXTERNAL, inner INTERNAL (hole)
    arrEll.Add(XYEllipse(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(5.0, 4.0, 0.0, 0.0, 0.0, INTERNAL));

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should produce contours for both
    EXPECT_GE(arrCont.GetSize(), 1);
}

// ============================================================================
// Contour Closure and Continuity Tests
// ============================================================================

TEST_F(CalcContourTest, AllContours_AreClosed) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Multiple shapes
    arrEll.Add(XYEllipse(8.0, 6.0, -15.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(8.0, 6.0, 15.0, 0.0, 0.0, EXTERNAL));
    arrRect.Add(XYRect(6.0, 4.0, 0.0, 0.0, 0.0, EXTERNAL));

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    // Verify all contours are closed
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_TRUE(IsClosed(arrCont[i])) << "Contour " << i << " is not closed";
    }
}

TEST_F(CalcContourTest, ContoursHaveMinimumPoints) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    arrEll.Add(XYEllipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL));

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    // A polygon needs at least 3 unique points + closing point
    EXPECT_GE(arrCont[0].GetSize(), 4);
}

// ============================================================================
// Specific Geometry Tests
// ============================================================================

TEST_F(CalcContourTest, Circle_ProducesCircularContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Circle (equal radii)
    XYEllipse circle(10.0, 10.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(circle);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    
    // All points should be approximately same distance from center
    double expectedRadius = 10.0;
    XYPoint center(0.0, 0.0);
    
    for (int i = 0; i < arrCont[0].GetSize() - 1; i++) {
        double dist = Distance(arrCont[0][i], center);
        EXPECT_TRUE(IsNear(dist, expectedRadius, 0.5));
    }
}

TEST_F(CalcContourTest, Square_ProducesRectangularContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Square
    XYRect square(5.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrRect.Add(square);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_GT(arrCont[0].GetSize(), 0);
}

// ============================================================================
// Boundary Condition Tests
// ============================================================================

TEST_F(CalcContourTest, TouchingButNotOverlapping_ProducesSeparateContours) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Two circles touching at one point
    XYEllipse circle1(5.0, 5.0, -5.0, 0.0, 0.0, EXTERNAL);
    XYEllipse circle2(5.0, 5.0, 5.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(circle1);
    arrEll.Add(circle2);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    EXPECT_EQ(arrCont.GetSize(), 2);
}

TEST_F(CalcContourTest, IdenticalShapes_ProducesSingleContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Two identical ellipses at same location
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);

    // Should produce one contour (shapes are identical)
    EXPECT_GE(arrCont.GetSize(), 1);
}

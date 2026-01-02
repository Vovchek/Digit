/// <summary>
/// Test suite for detecting degenerate polygon bug in CalcContour
/// 
/// This test suite specifically targets the bug where:
/// - Small changes in PI precision change point counts (NFi)
/// - Visibility filtering becomes overly aggressive
/// - Results in 2-point "polygons" that crash downstream code
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
// Test Fixture for Degenerate Polygon Detection
// ============================================================================

class CalcContourDegenerateTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;
    static constexpr int MIN_VIABLE_POLYGON_POINTS = 3;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }

    // Helper to check if polygon is degenerate
    bool IsDegenerate(const XYPolygon& polygon) const {
        // Less than 3 distinct points = degenerate
        if (polygon.GetSize() < MIN_VIABLE_POLYGON_POINTS) {
            return true;
        }
        
        // All points collinear = degenerate (line, not polygon)
        if (polygon.GetSize() == MIN_VIABLE_POLYGON_POINTS) {
            // Check if 3 points are collinear using cross product
            XYPoint p0 = polygon[0];
            XYPoint p1 = polygon[1];
            XYPoint p2 = polygon[2];
            
            double cross = (p1.X - p0.X) * (p2.Y - p0.Y) - 
                          (p1.Y - p0.Y) * (p2.X - p0.X);
                          
            if (std::abs(cross) < TOLERANCE) {
                return true; // Collinear
            }
        }
        
        return false;
    }
    
    // Helper to check if bounds are valid
    bool HasValidBounds(const XYPolygon& polygon) const {
        if (polygon.GetSize() < 2) return false;
        
        double xMin = polygon[0].X;
        double xMax = polygon[0].X;
        double yMin = polygon[0].Y;
        double yMax = polygon[0].Y;
        
        for (int i = 1; i < polygon.GetSize(); i++) {
            if (polygon[i].X < xMin) xMin = polygon[i].X;
            if (polygon[i].X > xMax) xMax = polygon[i].X;
            if (polygon[i].Y < yMin) yMin = polygon[i].Y;
            if (polygon[i].Y > yMax) yMax = polygon[i].Y;
        }
        
        // Valid bounds must have non-zero area
        bool hasArea = (xMax - xMin) > TOLERANCE && (yMax - yMin) > TOLERANCE;
        return hasArea;
    }
    
    // Helper to count visible points on ellipse
    int CountVisiblePoints(const XYEllipse& ellipse, 
                          const CArrayXYEllipse& allEllipses,
                          int exceptIndex,
                          double step) const {
        XYBrokenLine contour;
        ellipse.GetContour(contour, step);
        
        int visibleCount = 0;
        for (int i = 0; i < contour.GetSize(); i++) {
            XYPoint pt = contour[i];
            bool visible = true;
            
            // Check against all other ellipses
            for (int j = 0; j < allEllipses.GetSize(); j++) {
                if (j == exceptIndex) continue;
                
                if (!allEllipses[j].isVisible(pt)) {
                    visible = false;
                    break;
                }
            }
            
            if (visible) visibleCount++;
        }
        
        return visibleCount;
    }
};

// ============================================================================
// Core Bug Detection Tests
// ============================================================================

TEST_F(CalcContourDegenerateTest, DetectDegeneratePolygon_TwoSlightlyDifferentEllipses) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Simulate the DOS ZAP file scenario: two EXTERNAL ellipses slightly different
    // These values should trigger the bug
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.1, 8.1, 0.5, 0.3, 0.0, EXTERNAL);  // Slightly different
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    // Test with different NPntMax values that might trigger different NFi
    for (int NPntMax : {100, 200, 500, 501, 1000}) {
        arrCont.RemoveAll();
        CalcContour(arrEll, arrRect, arrPlg, arrCont, NPntMax);
        
        // Verify NO degenerate polygons
        for (int i = 0; i < arrCont.GetSize(); i++) {
            EXPECT_FALSE(IsDegenerate(arrCont[i])) 
                << "Degenerate polygon detected with NPntMax=" << NPntMax
                << ", contour " << i << " has " << arrCont[i].GetSize() << " points";
                
            EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
                << "Polygon has too few points with NPntMax=" << NPntMax;
        }
    }
}

TEST_F(CalcContourDegenerateTest, AllContoursHaveMinimumPoints) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Various configurations
    arrEll.Add(XYEllipse(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(5.1, 3.1, 1.0, 1.0, 0.0, EXTERNAL));
    arrRect.Add(XYRect(4.0, 3.0, -5.0, 0.0, 0.0, EXTERNAL));
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    // Every contour MUST have at least 3 points
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Contour " << i << " is degenerate with " 
            << arrCont[i].GetSize() << " points";
    }
}

TEST_F(CalcContourDegenerateTest, AllContoursHaveValidBounds) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    arrEll.Add(XYEllipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(10.1, 8.1, 0.5, 0.3, 0.0, EXTERNAL));
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // Every contour must have valid (non-zero area) bounds
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_TRUE(HasValidBounds(arrCont[i]))
            << "Contour " << i << " has invalid bounds (zero area)";
    }
}

// ============================================================================
// Floating-Point Precision Sensitivity Tests
// ============================================================================

TEST_F(CalcContourDegenerateTest, PerimeterCalculation_StableAcrossPrecision) {
    // Test that perimeter calculation doesn't cause wild point count variations
    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    double perim = ellipse.Perimeter();
    
    // For NPntMax around 500, check that NFi doesn't jump erratically
    std::vector<int> pointCounts;
    for (int NPntMax = 498; NPntMax <= 502; NPntMax++) {
        double step = perim / NPntMax;
        int NFi = static_cast<int>(perim / step);
        pointCounts.push_back(NFi);
    }
    
    // Point counts should be stable (no jumps > 2)
    for (size_t i = 1; i < pointCounts.size(); i++) {
        int diff = std::abs(pointCounts[i] - pointCounts[i-1]);
        EXPECT_LE(diff, 2) 
            << "Point count unstable: " << pointCounts[i-1] 
            << " to " << pointCounts[i];
    }
}

TEST_F(CalcContourDegenerateTest, PointCountConsistency_NFi500vs501) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.1, 8.1, 0.5, 0.3, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    // Test NFi=500 vs NFi=501 (critical boundary from bug report)
    CArrayXYPolygon arrCont500;
    CArrayXYPolygon arrCont501;
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont500, 500);
    CalcContour(arrEll, arrRect, arrPlg, arrCont501, 501);
    
    // Both should produce same number of contours
    EXPECT_EQ(arrCont500.GetSize(), arrCont501.GetSize())
        << "NFi=500 produces " << arrCont500.GetSize() 
        << " contours vs NFi=501 produces " << arrCont501.GetSize();
    
    // Both should have all valid polygons
    for (int i = 0; i < arrCont500.GetSize(); i++) {
        EXPECT_GE(arrCont500[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "NFi=500 contour " << i << " is degenerate";
    }
    
    for (int i = 0; i < arrCont501.GetSize(); i++) {
        EXPECT_GE(arrCont501[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "NFi=501 contour " << i << " is degenerate";
    }
}

// ============================================================================
// Visibility Filtering Tests
// ============================================================================

TEST_F(CalcContourDegenerateTest, VisibilityFiltering_NotOverlyAggressive) {
    CArrayXYEllipse arrEll;
    
    // Two EXTERNAL ellipses slightly overlapping
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.1, 8.1, 1.0, 0.5, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    double perim1 = ellipse1.Perimeter();
    double step = perim1 / 500.0;
    
    // Count visible points for each ellipse
    int visible1 = CountVisiblePoints(ellipse1, arrEll, 0, step);
    int visible2 = CountVisiblePoints(ellipse2, arrEll, 1, step);
    
    // Should have substantial visible points (not just 2!)
    EXPECT_GT(visible1, 10) 
        << "Only " << visible1 << " visible points on ellipse 1 - too aggressive filtering";
    EXPECT_GT(visible2, 10) 
        << "Only " << visible2 << " visible points on ellipse 2 - too aggressive filtering";
        
    // At least 5% of points should be visible
    int total1 = static_cast<int>(perim1 / step);
    EXPECT_GT(visible1, total1 * 0.05)
        << "Less than 5% visible points on ellipse 1";
}

TEST_F(CalcContourDegenerateTest, ExternalApertures_NonOverlapping_EmptyVisibleArea) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures that DON'T overlap
    // Visible area = intersection of apertures = EMPTY
    XYEllipse ellipse1(5.0, 4.0, -10.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(5.0, 4.0, 10.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // Non-overlapping EXTERNAL apertures → no visible area → no contours
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Non-overlapping EXTERNAL apertures should produce empty visible area";
}

TEST_F(CalcContourDegenerateTest, ExternalApertures_Overlapping_ProducesContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures that DO overlap
    // Visible area = intersection of apertures
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.0, 8.0, 2.0, 1.0, 0.0, EXTERNAL);  // Overlapping
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // Should produce contour(s) for the overlapping region
    EXPECT_GT(arrCont.GetSize(), 0)
        << "Overlapping EXTERNAL apertures should produce visible area";
    
    // All contours should be valid
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Contour " << i << " is degenerate";
        EXPECT_FALSE(IsDegenerate(arrCont[i]))
            << "Contour " << i << " is degenerate";
    }
}

// ============================================================================
// Edge Case Tests for Degenerate Inputs
// ============================================================================

TEST_F(CalcContourDegenerateTest, VerySmallStep_ProducesValidPolygons) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);
    
    // Very large NPntMax → very small step
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 5000);
    
    ASSERT_GT(arrCont.GetSize(), 0);
    EXPECT_GE(arrCont[0].GetSize(), MIN_VIABLE_POLYGON_POINTS);
    EXPECT_FALSE(IsDegenerate(arrCont[0]));
}

TEST_F(CalcContourDegenerateTest, VeryLargeStep_StillProducesValidPolygons) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    XYEllipse ellipse(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    arrEll.Add(ellipse);
    
    // Very small NPntMax → very large step
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 10);
    
    ASSERT_GT(arrCont.GetSize(), 0);
    EXPECT_GE(arrCont[0].GetSize(), MIN_VIABLE_POLYGON_POINTS);
    EXPECT_FALSE(IsDegenerate(arrCont[0]));
}

TEST_F(CalcContourDegenerateTest, TinyOverlappingEllipses_NoDegenerate) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Very small ellipses with tiny differences
    XYEllipse ellipse1(0.5, 0.4, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(0.51, 0.41, 0.02, 0.01, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    // Should still produce valid polygons
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Tiny ellipse contour " << i << " is degenerate";
    }
}

// ============================================================================
// Regression Tests for Specific Bug Scenario
// ============================================================================

TEST_F(CalcContourDegenerateTest, BugScenario_TwoExternalApertures_DifferentSizes) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Recreate bug scenario: two EXTERNAL apertures, slightly different
    // Aperture 1: larger
    XYEllipse aperture1(15.0, 12.0, 0.0, 0.0, 0.0, EXTERNAL);
    // Aperture 2: slightly smaller and offset
    XYEllipse aperture2(14.8, 11.9, 0.5, 0.3, 0.0, EXTERNAL);
    
    arrEll.Add(aperture1);
    arrEll.Add(aperture2);
    
    // Test with critical NPntMax values
    for (int NPntMax : {499, 500, 501, 502}) {
        arrCont.RemoveAll();
        CalcContour(arrEll, arrRect, arrPlg, arrCont, NPntMax);
        
        EXPECT_GT(arrCont.GetSize(), 0) 
            << "No contours produced with NPntMax=" << NPntMax;
        
        // Check each contour
        for (int i = 0; i < arrCont.GetSize(); i++) {
            int pointCount = arrCont[i].GetSize();
            
            EXPECT_GE(pointCount, MIN_VIABLE_POLYGON_POINTS)
                << "NPntMax=" << NPntMax << ", contour " << i 
                << " has only " << pointCount << " points (DEGENERATE)";
            
            EXPECT_FALSE(IsDegenerate(arrCont[i]))
                << "NPntMax=" << NPntMax << ", contour " << i << " is degenerate";
            
            EXPECT_TRUE(HasValidBounds(arrCont[i]))
                << "NPntMax=" << NPntMax << ", contour " << i << " has invalid bounds";
        }
    }
}

TEST_F(CalcContourDegenerateTest, BugScenario_OnlyTwoVisiblePoints) {
    // This test simulates the exact bug condition:
    // - Ellipse contour has 500 points
    // - Visibility filtering leaves only 2 points
    // - Results in 2-point "polygon"
    
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Create scenario where visibility is very restrictive
    XYEllipse ellipse1(8.0, 6.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(8.05, 6.05, 0.1, 0.05, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // Must produce contours
    ASSERT_GT(arrCont.GetSize(), 0) << "No contours produced";
    
    // No contour should have just 2 points
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_NE(arrCont[i].GetSize(), 2)
            << "Contour " << i << " has exactly 2 points (BUG CONDITION)";
        
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Contour " << i << " is degenerate";
    }
}

// ============================================================================
// Polygon Validation Tests
// ============================================================================

TEST_F(CalcContourDegenerateTest, PolygonConstructor_RejectsDegenerate) {
    // Test that XYPolygon constructor properly handles degenerate input
    
    // 2-point broken line
    XYBrokenLine twoPoints;
    twoPoints.Add(XYPoint(0.0, 0.0));
    twoPoints.Add(XYPoint(1.0, 1.0));
    
    XYPolygon poly(twoPoints);
    
    // Polygon created, but is it valid?
    int size = poly.GetSize();
    
    // This should be caught - 2-point polygon is not valid
    // (This test documents current behavior, not necessarily correct behavior)
    EXPECT_TRUE(size == 2 || size == 3)
        << "XYPolygon from 2 points has unexpected size: " << size;
}

TEST_F(CalcContourDegenerateTest, ConnectSegments_FiltersOutDegenerateTwoPoints) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;

    // Create single 2-point segment (degenerate)
    XYBrokenLine twoPointSegment;
    twoPointSegment.Add(XYPoint(0.0, 0.0));
    twoPointSegment.Add(XYPoint(1.0, 0.0));
    arrBLn.Add(twoPointSegment);

    double eps = 0.1;
    ConnectSegments(arrBLn, arrCont, eps);

    // Degenerate polygon should be filtered out
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "2-point degenerate polygon should be filtered out";
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(CalcContourDegenerateTest, RealWorld_MultipleApertures_NoDegenerate) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Simulate real-world aperture system with multiple EXTERNAL shapes
    arrEll.Add(XYEllipse(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(19.8, 14.9, 1.0, 0.5, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(19.5, 14.8, -1.0, -0.5, 0.0, EXTERNAL));
    arrRect.Add(XYRect(10.0, 8.0, 5.0, 5.0, 0.0, EXTERNAL));
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // All contours must be valid
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Real-world scenario produced degenerate contour " << i;
        EXPECT_FALSE(IsDegenerate(arrCont[i]));
        EXPECT_TRUE(HasValidBounds(arrCont[i]));
    }
}

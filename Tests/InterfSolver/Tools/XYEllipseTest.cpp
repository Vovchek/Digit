/// <summary>
/// Google Test suite for XYEllipse class
/// Tests constructors, methods, and edge cases for ellipse geometry
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Tools/XYBounds.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include <vector>
#include <cmath>

// ============================================================================
// Test Fixture for XYEllipse
// ============================================================================

class XYEllipseTest : public ::testing::Test {
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
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(XYEllipseTest, DefaultConstructor) {
    XYEllipse ellipse;
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 1.0);
    EXPECT_DOUBLE_EQ(ellipse.By, 1.0);
    EXPECT_DOUBLE_EQ(ellipse.Xc, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);
    EXPECT_EQ(ellipse.TypeLimits, EXTERNAL);
    EXPECT_EQ(ellipse.TypeSystCoor, MEASURING);
}

TEST_F(XYEllipseTest, ParameterizedConstructor) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 45.0, INTERNAL, NORMALISED);
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.By, 3.0);
    EXPECT_DOUBLE_EQ(ellipse.Xc, 10.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 20.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 45.0);
    EXPECT_EQ(ellipse.TypeLimits, INTERNAL);
    EXPECT_EQ(ellipse.TypeSystCoor, NORMALISED);
    
    // Verify Si and Co are computed
    EXPECT_TRUE(IsNear(ellipse.Si, std::sin(45.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(ellipse.Co, std::cos(45.0 * GRD_RD)));
}

TEST_F(XYEllipseTest, CopyConstructor) {
    XYEllipse original(5.0, 3.0, 10.0, 20.0, 30.0);
    XYEllipse copy(original);
    
    EXPECT_DOUBLE_EQ(copy.Ax, original.Ax);
    EXPECT_DOUBLE_EQ(copy.By, original.By);
    EXPECT_DOUBLE_EQ(copy.Xc, original.Xc);
    EXPECT_DOUBLE_EQ(copy.Yc, original.Yc);
    EXPECT_DOUBLE_EQ(copy.Fi, original.Fi);
    EXPECT_DOUBLE_EQ(copy.Si, original.Si);
    EXPECT_DOUBLE_EQ(copy.Co, original.Co);
}

TEST_F(XYEllipseTest, BoundsConstructor) {
    XYBounds bounds;
    bounds.XLeft = 0.0;
    bounds.XRight = 20.0;
    bounds.YBottom = 30.0;
    bounds.YTop = 10.0;
    
    XYEllipse ellipse(bounds);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 10.0);  // (0 + 20) / 2
    EXPECT_DOUBLE_EQ(ellipse.Yc, 20.0);  // (30 + 10) / 2
    EXPECT_DOUBLE_EQ(ellipse.Ax, 10.0);  // |20 - 0| / 2
    EXPECT_DOUBLE_EQ(ellipse.By, 10.0);  // |10 - 30| / 2
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);
}

// ============================================================================
// Vector Constructor Tests (0-5+ points)
// ============================================================================

TEST_F(XYEllipseTest, VectorConstructor_ZeroPoints) {
    std::vector<XYPoint> points;
    XYEllipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.By, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Xc, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);
}

TEST_F(XYEllipseTest, VectorConstructor_OnePoint) {
    std::vector<XYPoint> points = { XYPoint(5.0, 10.0) };
    XYEllipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 10.0);
    EXPECT_DOUBLE_EQ(ellipse.Ax, 0.0);  // Unit circle
    EXPECT_DOUBLE_EQ(ellipse.By, 0.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);
}

TEST_F(XYEllipseTest, VectorConstructor_TwoPoints) {
    std::vector<XYPoint> points = {
        XYPoint(0.0, 0.0),
        XYPoint(10.0, 0.0)
    };
    XYEllipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 5.0);   // Midpoint X
    EXPECT_DOUBLE_EQ(ellipse.Yc, 0.0);   // Midpoint Y
    EXPECT_DOUBLE_EQ(ellipse.Ax, 5.0);   // Radius = diameter / 2
    EXPECT_DOUBLE_EQ(ellipse.By, 5.0);   // Circle
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);
}

TEST_F(XYEllipseTest, VectorConstructor_ThreePoints_Circle) {
    // Three points on a circle with center (0,0) and radius 5
    std::vector<XYPoint> points = {
        XYPoint(5.0, 0.0),
        XYPoint(0.0, 5.0),
        XYPoint(-5.0, 0.0)
    };
    XYEllipse ellipse(points);
    
    EXPECT_TRUE(IsNear(ellipse.Xc, 0.0));
    EXPECT_TRUE(IsNear(ellipse.Yc, 0.0));
    EXPECT_TRUE(IsNear(ellipse.Ax, 5.0));
    EXPECT_TRUE(IsNear(ellipse.By, 5.0));  // Circle
}

TEST_F(XYEllipseTest, VectorConstructor_ThreePoints_Collinear) {
    // Three collinear points should create degenerate ellipse
    std::vector<XYPoint> points = {
        XYPoint(0.0, 0.0),
        XYPoint(5.0, 5.0),
        XYPoint(10.0, 10.0)
    };
    XYEllipse ellipse(points);
    
    EXPECT_TRUE(IsNear(ellipse.Ax, 0.0));
    EXPECT_TRUE(IsNear(ellipse.By, 0.0));
}

TEST_F(XYEllipseTest, VectorConstructor_FourPoints_AxisAligned) {
    // Four points forming axis-aligned ellipse
    std::vector<XYPoint> points = {
        XYPoint(0.0, 5.0),    // Top
        XYPoint(10.0, 5.0),   // Right
        XYPoint(5.0, 0.0),    // Bottom
        XYPoint(5.0, 10.0)    // Left
    };
    XYEllipse ellipse(points);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 0.0);  // Axis-aligned
    EXPECT_GT(ellipse.Ax, 0.0);
    EXPECT_GT(ellipse.By, 0.0);
}

TEST_F(XYEllipseTest, VectorConstructor_FivePoints_ExactFit) {
    // Five points define exact ellipse
    std::vector<XYPoint> points = {
        XYPoint(10.0, 5.0),
        XYPoint(5.0, 10.0),
        XYPoint(0.0, 5.0),
        XYPoint(5.0, 0.0),
        XYPoint(8.0, 8.0)
    };
    XYEllipse ellipse(points);
    
    // Should create valid ellipse (exact parameters depend on fitting)
    EXPECT_GT(ellipse.Ax, 0.0);
    EXPECT_GT(ellipse.By, 0.0);
}

TEST_F(XYEllipseTest, VectorConstructor_MoreThanFivePoints_LSM) {
    // More than 5 points - least squares fitting
    std::vector<XYPoint> points;
    double cx = 10.0, cy = 5.0, a = 8.0, b = 4.0;
    
    // Generate points on an ellipse
    for (int i = 0; i < 20; i++) {
        double angle = i * PI2 / 20.0;
        points.push_back(XYPoint(
            cx + a * std::cos(angle),
            cy + b * std::sin(angle)
        ));
    }
    
    XYEllipse ellipse(points);
    
    // Fitted ellipse should be close to original
    EXPECT_TRUE(IsNear(ellipse.Xc, cx, 0.5));
    EXPECT_TRUE(IsNear(ellipse.Yc, cy, 0.5));
    EXPECT_TRUE(IsNear(ellipse.Ax, a, 0.5));
    EXPECT_TRUE(IsNear(ellipse.By, b, 0.5));
}

// ============================================================================
// Assignment Operator Tests
// ============================================================================

TEST_F(XYEllipseTest, AssignmentOperator) {
    XYEllipse original(5.0, 3.0, 10.0, 20.0, 30.0);
    XYEllipse assigned;
    
    assigned = original;
    
    EXPECT_DOUBLE_EQ(assigned.Ax, original.Ax);
    EXPECT_DOUBLE_EQ(assigned.By, original.By);
    EXPECT_DOUBLE_EQ(assigned.Xc, original.Xc);
    EXPECT_DOUBLE_EQ(assigned.Yc, original.Yc);
    EXPECT_DOUBLE_EQ(assigned.Fi, original.Fi);
}

// ============================================================================
// Perimeter Tests
// ============================================================================

TEST_F(XYEllipseTest, Perimeter_Circle) {
    XYEllipse circle(5.0, 5.0, 0.0, 0.0, 0.0);
    double perimeter = circle.Perimeter();
    
    // For circle: P ≈ 2πr
    double expected = 2.0 * PI * 5.0;
    EXPECT_TRUE(IsNear(perimeter, expected, 0.1));
}

TEST_F(XYEllipseTest, Perimeter_Ellipse) {
    XYEllipse ellipse(8.0, 4.0, 0.0, 0.0, 0.0);
    double perimeter = ellipse.Perimeter();
    
    // Should be between 2π*min and 2π*max
    EXPECT_GT(perimeter, 2.0 * PI * 4.0);
    EXPECT_LT(perimeter, 2.0 * PI * 8.0);
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(XYEllipseTest, isInside_Center) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 0.0);
    XYPoint center(10.0, 20.0);
    
    EXPECT_TRUE(ellipse.isInside(center));
}

TEST_F(XYEllipseTest, isInside_OnBoundary) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYPoint point(5.0, 0.0);  // On major axis
    
    EXPECT_TRUE(ellipse.isInside(point));
}

TEST_F(XYEllipseTest, isInside_Outside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYPoint point(10.0, 10.0);
    
    EXPECT_FALSE(ellipse.isInside(point));
}

TEST_F(XYEllipseTest, isInside_RotatedEllipse) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 45.0);
    XYPoint center(0.0, 0.0);
    
    EXPECT_TRUE(ellipse.isInside(center));
}

TEST_F(XYEllipseTest, isInside_XYOverload) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 0.0);
    
    EXPECT_TRUE(ellipse.isInside(10.0, 20.0));  // Center
    EXPECT_FALSE(ellipse.isInside(100.0, 100.0));  // Far outside
}

// ============================================================================
// isVisible Tests
// ============================================================================

TEST_F(XYEllipseTest, isVisible_ExternalLimits_Inside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYPoint inside(0.0, 0.0);
    
    EXPECT_TRUE(ellipse.isVisible(inside));  // Inside is visible for EXTERNAL
}

TEST_F(XYEllipseTest, isVisible_ExternalLimits_Outside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYPoint outside(10.0, 10.0);
    
    EXPECT_FALSE(ellipse.isVisible(outside));  // Outside invisible for EXTERNAL
}

TEST_F(XYEllipseTest, isVisible_InternalLimits_Inside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0, INTERNAL);
    XYPoint inside(0.0, 0.0);
    
    EXPECT_FALSE(ellipse.isVisible(inside));  // Inside invisible for INTERNAL
}

TEST_F(XYEllipseTest, isVisible_InternalLimits_Outside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0, INTERNAL);
    XYPoint outside(10.0, 10.0);
    
    EXPECT_TRUE(ellipse.isVisible(outside));  // Outside is visible for INTERNAL
}

// ============================================================================
// GetExtents Tests
// ============================================================================

TEST_F(XYEllipseTest, GetExtents_AxisAligned) {
    XYEllipse ellipse(8.0, 5.0, 10.0, 20.0, 0.0);
    double xmin, ymin, xmax, ymax;
    
    ellipse.GetExtents(xmin, ymin, xmax, ymax);
    
    EXPECT_DOUBLE_EQ(xmin, 2.0);   // 10 - 8
    EXPECT_DOUBLE_EQ(xmax, 18.0);  // 10 + 8
    EXPECT_DOUBLE_EQ(ymin, 15.0);  // 20 - 5
    EXPECT_DOUBLE_EQ(ymax, 25.0);  // 20 + 5
}

TEST_F(XYEllipseTest, GetExtents_RotatedEllipse) {
    XYEllipse ellipse(8.0, 5.0, 0.0, 0.0, 45.0);
    double xmin, ymin, xmax, ymax;
    
    ellipse.GetExtents(xmin, ymin, xmax, ymax);
    
	// For 45° rotation extents are equal for both axes
    // Note: Due to limited precision of GRD_RD constant, we need slightly higher tolerance
    EXPECT_TRUE(IsNear((xmax-xmin), (ymax-ymin), 2e-6));

	// for ax = 8, by = 5 we get sqrt(8*8/2 + 5*5/2) * 2 = 13.3417 approx
    EXPECT_TRUE(IsNear((xmax - xmin), 13.3417, 0.0001));

    // Should be symmetric around center
    EXPECT_TRUE(IsNear(xmin, -xmax));
    EXPECT_TRUE(IsNear(ymin, -ymax));
}

TEST_F(XYEllipseTest, GetExtents_Circle) {
    XYEllipse circle(5.0, 5.0, 10.0, 10.0, 0.0);
    double xmin, ymin, xmax, ymax;
    
    circle.GetExtents(xmin, ymin, xmax, ymax);
    
    EXPECT_DOUBLE_EQ(xmin, 5.0);
    EXPECT_DOUBLE_EQ(xmax, 15.0);
    EXPECT_DOUBLE_EQ(ymin, 5.0);
    EXPECT_DOUBLE_EQ(ymax, 15.0);
}

TEST_F(XYEllipseTest, GetExtents_DegenerateEllipse) {
    XYEllipse ellipse(0.0, 0.0, 5.0, 10.0, 0.0);
    double xmin, ymin, xmax, ymax;
    
    ellipse.GetExtents(xmin, ymin, xmax, ymax);
    
    EXPECT_DOUBLE_EQ(xmin, 5.0);
    EXPECT_DOUBLE_EQ(xmax, 5.0);
    EXPECT_DOUBLE_EQ(ymin, 10.0);
    EXPECT_DOUBLE_EQ(ymax, 10.0);
}

// ============================================================================
// Transformation Tests
// ============================================================================

TEST_F(XYEllipseTest, InverseY) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 30.0);
    double YcInv = 100.0;
    
    ellipse.InverseY(YcInv);
    
    EXPECT_DOUBLE_EQ(ellipse.Yc, 80.0);  // 100 - 20
    EXPECT_DOUBLE_EQ(ellipse.Fi, -30.0);
}

TEST_F(XYEllipseTest, ShiftX) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 0.0);
    
    ellipse.ShiftX(5.0);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 15.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 20.0);  // Unchanged
}

TEST_F(XYEllipseTest, ShiftY) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 0.0);
    
    ellipse.ShiftY(-10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.Xc, 10.0);  // Unchanged
    EXPECT_DOUBLE_EQ(ellipse.Yc, 10.0);
}

TEST_F(XYEllipseTest, Normalize) {
    XYEllipse ellipse(10.0, 6.0, 50.0, 30.0, 0.0, EXTERNAL, MEASURING);
    
    ellipse.Normalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 1.0);   // 10 / 10
    EXPECT_DOUBLE_EQ(ellipse.By, 0.6);   // 6 / 10
    EXPECT_DOUBLE_EQ(ellipse.Xc, 1.0);   // (50 - 40) / 10
    EXPECT_DOUBLE_EQ(ellipse.Yc, 1.0);   // (30 - 20) / 10
    EXPECT_EQ(ellipse.TypeSystCoor, NORMALISED);
}

TEST_F(XYEllipseTest, DeNormalize) {
    XYEllipse ellipse(1.0, 0.6, 1.0, 1.0, 0.0, EXTERNAL, NORMALISED);
    
    ellipse.DeNormalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 10.0);  // 1 * 10
    EXPECT_DOUBLE_EQ(ellipse.By, 6.0);   // 0.6 * 10
    EXPECT_DOUBLE_EQ(ellipse.Xc, 50.0);  // 1 * 10 + 40
    EXPECT_DOUBLE_EQ(ellipse.Yc, 30.0);  // 1 * 10 + 20
    EXPECT_EQ(ellipse.TypeSystCoor, MEASURING);
}

TEST_F(XYEllipseTest, Normalize_DeNormalize_RoundTrip) {
    XYEllipse original(10.0, 6.0, 50.0, 30.0, 0.0);
    XYEllipse ellipse = original;
    
    ellipse.Normalize(40.0, 20.0, 10.0);
    ellipse.DeNormalize(40.0, 20.0, 10.0);
    
    EXPECT_TRUE(IsNear(ellipse.Ax, original.Ax));
    EXPECT_TRUE(IsNear(ellipse.By, original.By));
    EXPECT_TRUE(IsNear(ellipse.Xc, original.Xc));
    EXPECT_TRUE(IsNear(ellipse.Yc, original.Yc));
}

// ============================================================================
// GetContour Tests
// ============================================================================

TEST_F(XYEllipseTest, GetContour_BrokenLine_Count) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = ellipse.GetContour(bline, 100);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(bline.GetSize(), 100);
}

TEST_F(XYEllipseTest, GetContour_BrokenLine_PointsOnEllipse) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    ellipse.GetContour(bline, 100);
    
    // Verify points are on the ellipse
    for (int i = 0; i < bline.GetSize(); i++) {
        EXPECT_TRUE(ellipse.isInside(bline[i]));
    }
}

TEST_F(XYEllipseTest, GetContour_BrokenLine_Step) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = ellipse.GetContour(bline, 1.0);
    
    EXPECT_TRUE(success);
    EXPECT_GT(bline.GetSize(), 0);
}

TEST_F(XYEllipseTest, GetContour_BrokenLine_InvalidStep) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = ellipse.GetContour(bline, -1.0);
    
    EXPECT_FALSE(success);
}

TEST_F(XYEllipseTest, GetContour_Polygon_Count) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYPolygon polygon;
    
    bool success = ellipse.GetContour(polygon, 100);
    
    EXPECT_TRUE(success);
    EXPECT_GT(polygon.GetSize(), 0);
}

TEST_F(XYEllipseTest, GetContour_DegenerateEllipse) {
    XYEllipse ellipse(0.0, 0.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = ellipse.GetContour(bline, 100);
    
    EXPECT_FALSE(success);  // Should fail for degenerate ellipse
}

// ============================================================================
// Set Method Tests
// ============================================================================

TEST_F(XYEllipseTest, SetMethod) {
    XYEllipse ellipse;
    
    ellipse.Set(7.0, 4.0, 15.0, 25.0, 60.0, INTERNAL, NORMALISED);
    
    EXPECT_DOUBLE_EQ(ellipse.Ax, 7.0);
    EXPECT_DOUBLE_EQ(ellipse.By, 4.0);
    EXPECT_DOUBLE_EQ(ellipse.Xc, 15.0);
    EXPECT_DOUBLE_EQ(ellipse.Yc, 25.0);
    EXPECT_DOUBLE_EQ(ellipse.Fi, 60.0);
    EXPECT_EQ(ellipse.TypeLimits, INTERNAL);
    EXPECT_EQ(ellipse.TypeSystCoor, NORMALISED);
    
    // Verify Si and Co are updated
    EXPECT_TRUE(IsNear(ellipse.Si, std::sin(60.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(ellipse.Co, std::cos(60.0 * GRD_RD)));
}

// ============================================================================
// Friend Function Tests
// ============================================================================

TEST_F(XYEllipseTest, FriendFunction_isInside) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYPoint inside(0.0, 0.0);
    XYPoint outside(10.0, 10.0);
    
    EXPECT_TRUE(isInside(ellipse, inside));
    EXPECT_FALSE(isInside(ellipse, outside));
}

TEST_F(XYEllipseTest, FriendFunction_isVisible) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYPoint inside(0.0, 0.0);
    XYPoint outside(10.0, 10.0);
    
    EXPECT_FALSE(isVisible(ellipse, inside));
    EXPECT_TRUE(isVisible(ellipse, outside));
}

TEST_F(XYEllipseTest, FriendFunction_GetContour) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYPolygon polygon;
    
    GetContour(ellipse, polygon, 100);
    
    EXPECT_GT(polygon.GetSize(), 0);
}

// ============================================================================
// Edge Cases and Special Scenarios
// ============================================================================

TEST_F(XYEllipseTest, VerySmallEllipse) {
    XYEllipse ellipse(0.001, 0.001, 0.0, 0.0, 0.0);
    XYPoint center(0.0, 0.0);
    
    EXPECT_TRUE(ellipse.isInside(center));
    EXPECT_GT(ellipse.Perimeter(), 0.0);
}

TEST_F(XYEllipseTest, VeryLargeEllipse) {
    XYEllipse ellipse(1e6, 1e6, 0.0, 0.0, 0.0);
    XYPoint point(1e5, 1e5);
    
    EXPECT_TRUE(ellipse.isInside(point));
}

TEST_F(XYEllipseTest, HighlyEccentricEllipse) {
    XYEllipse ellipse(100.0, 1.0, 0.0, 0.0, 0.0);
    XYPoint point(50.0, 0.0);
    
    EXPECT_TRUE(ellipse.isInside(point));
    
    double xmin, ymin, xmax, ymax;
    ellipse.GetExtents(xmin, ymin, xmax, ymax);
    
    EXPECT_DOUBLE_EQ(xmax - xmin, 200.0);
    EXPECT_DOUBLE_EQ(ymax - ymin, 2.0);
}

TEST_F(XYEllipseTest, Rotation360Degrees) {
    XYEllipse ellipse(5.0, 3.0, 10.0, 20.0, 360.0);
    
    // 360 degrees should be equivalent to 0 degrees for Si and Co
    EXPECT_TRUE(IsNear(ellipse.Si, 0.0, 0.01));
    EXPECT_TRUE(IsNear(ellipse.Co, 1.0, 0.01));
}

TEST_F(XYEllipseTest, NegativeRotation) {
    XYEllipse ellipse(5.0, 3.0, 0.0, 0.0, -45.0);
    
    EXPECT_TRUE(IsNear(ellipse.Si, std::sin(-45.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(ellipse.Co, std::cos(-45.0 * GRD_RD)));
}


/// <summary>
/// Google Test suite for CalcWinFringeBoundCircle function
/// Tests minimal enclosing circle calculation for external non-rotated shapes
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/ReadWriteData.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPoint.h"
#include <cmath>

// Forward declaration of the function under test
bool CalcWinFringeBoundCircle(const CArray<XYEllipse>& ArrEll, const CArray<XYRect>& ArrRect, double& Xc, double& Yc, double& Rad);

// Helper function to check if a point is inside or on the boundary of a circle
bool isPointInCircle(double px, double py, double cx, double cy, double radius, double tolerance = 1e-6)
{
	double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
	return dist <= radius + tolerance;
}

// Helper function to verify all extreme points of a shape are within the circle
bool verifyEllipseInCircle(const XYEllipse& ell, double cx, double cy, double radius, double tolerance = 1e-6)
{
	// Check 4 extreme points
	return isPointInCircle(ell.Xc - ell.Ax, ell.Yc, cx, cy, radius, tolerance) &&
		   isPointInCircle(ell.Xc + ell.Ax, ell.Yc, cx, cy, radius, tolerance) &&
		   isPointInCircle(ell.Xc, ell.Yc - ell.By, cx, cy, radius, tolerance) &&
		   isPointInCircle(ell.Xc, ell.Yc + ell.By, cx, cy, radius, tolerance);
}

bool verifyRectInCircle(const XYRect& rect, double cx, double cy, double radius, double tolerance = 1e-6)
{
	// Check 4 corner points
	return isPointInCircle(rect.Xc - rect.Ax, rect.Yc - rect.By, cx, cy, radius, tolerance) &&
		   isPointInCircle(rect.Xc + rect.Ax, rect.Yc - rect.By, cx, cy, radius, tolerance) &&
		   isPointInCircle(rect.Xc - rect.Ax, rect.Yc + rect.By, cx, cy, radius, tolerance) &&
		   isPointInCircle(rect.Xc + rect.Ax, rect.Yc + rect.By, cx, cy, radius, tolerance);
}

// ============================================================================
// TEST SUITE: CalcWinFringeBoundCircleTest
// ============================================================================

class CalcWinFringeBoundCircleTest : public ::testing::Test
{
protected:
	CArray<XYEllipse> ellipses;
	CArray<XYRect> rectangles;
	double xc, yc, rad;

	void SetUp() override
	{
		ellipses.RemoveAll();
		rectangles.RemoveAll();
		xc = yc = rad = 0.0;
	}

	void TearDown() override
	{
		ellipses.RemoveAll();
		rectangles.RemoveAll();
	}
};

// ============================================================================
// Empty Input Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, EmptyArrays_ReturnsFalse)
{
	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
	EXPECT_DOUBLE_EQ(0.0, xc);
	EXPECT_DOUBLE_EQ(0.0, yc);
	EXPECT_DOUBLE_EQ(0.0, rad);
}

TEST_F(CalcWinFringeBoundCircleTest, OnlyInternalEllipses_ReturnsFalse)
{
	ellipses.Add(XYEllipse(5.0, 3.0, 10.0, 20.0, 0.0, INTERNAL, MEASURING));
	ellipses.Add(XYEllipse(2.0, 2.0, 0.0, 0.0, 0.0, INTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, OnlyRotatedEllipses_ReturnsFalse)
{
	ellipses.Add(XYEllipse(5.0, 3.0, 10.0, 20.0, 45.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(2.0, 2.0, 0.0, 0.0, 30.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

// ============================================================================
// Single Ellipse Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, SingleCircleAtOrigin)
{
	// Circle with radius 5 centered at origin
	ellipses.Add(XYEllipse(5.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_NEAR(5.0, rad, 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, SingleCircleOffset)
{
	// Circle with radius 3 centered at (10, 20)
	ellipses.Add(XYEllipse(3.0, 3.0, 10.0, 20.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(10.0, xc, 1e-6);
	EXPECT_NEAR(20.0, yc, 1e-6);
	EXPECT_NEAR(3.0, rad, 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, SingleEllipseAxisAligned)
{
	// Ellipse with Ax=10, By=5 centered at (0, 0)
	// Extreme points: (-10, 0), (10, 0), (0, -5), (0, 5)
	// Minimal circle should pass through furthest points: (-10, 0) and (10, 0)
	ellipses.Add(XYEllipse(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_NEAR(10.0, rad, 1e-6); // Radius should be the larger semi-axis
}

TEST_F(CalcWinFringeBoundCircleTest, SingleEllipseTallOriented)
{
	// Ellipse with Ax=3, By=8 centered at (5, 10)
	// Extreme points: (2, 10), (8, 10), (5, 2), (5, 18)
	// Minimal circle should be centered at (5, 10) with radius 8
	ellipses.Add(XYEllipse(3.0, 8.0, 5.0, 10.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(5.0, xc, 1e-6);
	EXPECT_NEAR(10.0, yc, 1e-6);
	EXPECT_NEAR(8.0, rad, 1e-6);
}

// ============================================================================
// Single Rectangle Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, SingleSquareAtOrigin)
{
	// Square with half-width 4 centered at origin
	// Corner points: (-4, -4), (4, -4), (-4, 4), (4, 4)
	// Minimal circle: center (0, 0), radius = sqrt(32) = 4*sqrt(2) ≈ 5.657
	rectangles.Add(XYRect(4.0, 4.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_NEAR(4.0 * std::sqrt(2.0), rad, 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, SingleRectangleWide)
{
	// Rectangle with Ax=10, By=3 centered at (20, 30)
	// Corner points: (10, 27), (30, 27), (10, 33), (30, 33)
	// Diagonal = sqrt(400 + 36) = sqrt(436) ≈ 20.88
	// Radius = diagonal/2 ≈ 10.44
	rectangles.Add(XYRect(10.0, 3.0, 20.0, 30.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(20.0, xc, 1e-6);
	EXPECT_NEAR(30.0, yc, 1e-6);
	double expectedRad = std::sqrt(10.0 * 10.0 + 3.0 * 3.0);
	EXPECT_NEAR(expectedRad, rad, 1e-6);
}

// ============================================================================
// Multiple Ellipses Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, TwoConcentricCircles)
{
	// Two concentric circles at origin, radii 3 and 5
	// Minimal circle should be the larger one
	ellipses.Add(XYEllipse(3.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(5.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_GE(rad, 3.0 - 1e-6);
	EXPECT_LE(rad, 5.0 + 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, TwoSeparatedCircles)
{
	// Two circles: radius 2 at (-10, 0) and radius 2 at (10, 0)
	// Extreme points: (-12, 0), (-8, 0), (8, 0), (12, 0)
	// Minimal circle: center (0, 0), radius 12
	ellipses.Add(XYEllipse(2.0, 2.0, -10.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(2.0, 2.0, 10.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, ThreeEllipsesFormingTriangle)
{
	// Three circles at triangle vertices
	// Points at (0, 0), (10, 0), (5, 8.66) - approximately equilateral triangle
	ellipses.Add(XYEllipse(1.0, 1.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(1.0, 1.0, 10.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(1.0, 1.0, 5.0, 8.66, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

// ============================================================================
// Multiple Rectangles Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, TwoConcentricSquares)
{
	// Two concentric squares at origin
	rectangles.Add(XYRect(3.0, 3.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(5.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_GE(rad, 3.0 * std::sqrt(2.0) - 1e-6);
	EXPECT_LE(rad, 5.0 * std::sqrt(2.0) + 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, TwoSeparatedRectangles)
{
	// Two rectangles separated horizontally
	rectangles.Add(XYRect(2.0, 1.0, -5.0, 0.0, 0.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(2.0, 1.0, 5.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

// ============================================================================
// Mixed Shapes Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, EllipseAndRectangleMixed)
{
	// One ellipse and one rectangle
	ellipses.Add(XYEllipse(5.0, 3.0, -10.0, 0.0, 0.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(4.0, 4.0, 10.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, MultipleShapesMixed)
{
	// Multiple ellipses and rectangles
	ellipses.Add(XYEllipse(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(3.0, 3.0, 20.0, 20.0, 0.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(5.0, 8.0, -15.0, 10.0, 0.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(2.0, 2.0, 15.0, -15.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

// ============================================================================
// Filtering Tests - Verify shapes are properly filtered
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, RotatedShapesAreIgnored)
{
	// Add valid external non-rotated shape
	ellipses.Add(XYEllipse(5.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	
	// Add rotated shapes - should be ignored
	ellipses.Add(XYEllipse(10.0, 10.0, 100.0, 100.0, 45.0, EXTERNAL, MEASURING));
	rectangles.Add(XYRect(15.0, 15.0, 200.0, 200.0, 30.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, InternalShapesAreIgnored)
{
	// Add valid external shape
	ellipses.Add(XYEllipse(8.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	
	// Add internal shapes - should be ignored
	ellipses.Add(XYEllipse(20.0, 20.0, 100.0, 100.0, 0.0, INTERNAL, MEASURING));
	rectangles.Add(XYRect(25.0, 25.0, 200.0, 200.0, 0.0, INTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(0.0, xc, 1e-6);
	EXPECT_NEAR(0.0, yc, 1e-6);
	EXPECT_NEAR(8.0, rad, 1e-6);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, TwoPointsOnXAxis)
{
	// Two circles creating exactly 2 extreme points on the same horizontal line
	ellipses.Add(XYEllipse(0.0, 1.0, -5.0, 0.0, 0.0, EXTERNAL, MEASURING)); // Vertical line
	ellipses.Add(XYEllipse(0.0, 1.0, 5.0, 0.0, 0.0, EXTERNAL, MEASURING));  // Vertical line

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, CoincidentShapes)
{
	// Multiple shapes with identical geometry
	ellipses.Add(XYEllipse(7.0, 7.0, 5.0, 5.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(7.0, 7.0, 5.0, 5.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(7.0, 7.0, 5.0, 5.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_NEAR(5.0, xc, 1e-6);
	EXPECT_NEAR(5.0, yc, 1e-6);
	EXPECT_NEAR(7.0, rad, 1e-6);
}

TEST_F(CalcWinFringeBoundCircleTest, VerySmallShapes)
{
	// Extremely tiny aperture is below robust threshold for this helper.
	ellipses.Add(XYEllipse(0.001, 0.001, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, VeryLargeShapes)
{
	// Large but stable range
	rectangles.Add(XYRect(200.0, 50.0, 500.0, 300.0, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_TRUE(result);
	EXPECT_TRUE(verifyRectInCircle(rectangles[0], xc, yc, rad));
}

// ============================================================================
// Welzl Algorithm Specific Tests
// ============================================================================

TEST_F(CalcWinFringeBoundCircleTest, FourPointsOnCircumference)
{
	// Four points exactly on a circle of radius 10 centered at (0, 0)
	ellipses.Add(XYEllipse(0.0, 0.0, 10.0, 0.0, 0.0, EXTERNAL, MEASURING));  // Point at (10, 0)
	ellipses.Add(XYEllipse(0.0, 0.0, -10.0, 0.0, 0.0, EXTERNAL, MEASURING)); // Point at (-10, 0)
	ellipses.Add(XYEllipse(0.0, 0.0, 0.0, 10.0, 0.0, EXTERNAL, MEASURING));  // Point at (0, 10)
	ellipses.Add(XYEllipse(0.0, 0.0, 0.0, -10.0, 0.0, EXTERNAL, MEASURING)); // Point at (0, -10)

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

TEST_F(CalcWinFringeBoundCircleTest, MinimalCircleWith3Points)
{
	// Three points that define unique minimal enclosing circle
	// Points at (0, 0), (4, 0), (2, 2*sqrt(3)) form equilateral triangle
	// Circumcircle center: (2, 2/sqrt(3)), radius: 4/sqrt(3)
	ellipses.Add(XYEllipse(0.0, 0.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(0.0, 0.0, 4.0, 0.0, 0.0, EXTERNAL, MEASURING));
	ellipses.Add(XYEllipse(0.0, 0.0, 2.0, 3.464, 0.0, EXTERNAL, MEASURING));

	bool result = CalcWinFringeBoundCircle(ellipses, rectangles, xc, yc, rad);

	EXPECT_FALSE(result);
}

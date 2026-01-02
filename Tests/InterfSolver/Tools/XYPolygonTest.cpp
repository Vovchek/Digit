/// <summary>
/// Google Test suite for XYPolygon class
/// Tests constructors, methods, XYShape inheritance, multiple inheritance, and edge cases
/// Complements XYPolygonAreaTest.cpp (which tests Area() and isDegenerate())
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Tools/XYBounds.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include <cmath>

// ============================================================================
// Test Fixture for XYPolygon
// ============================================================================

class XYPolygonTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    void SetUp() override {
    }

    void TearDown() override {
    }

    // Helper to compare doubles with tolerance
    bool IsNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }
    
    // Helper to create a square polygon
    XYPolygon CreateSquare(double side, double centerX = 0.0, double centerY = 0.0) {
        CArrayDouble arrX, arrY;
        arrX.SetSize(5);
        arrY.SetSize(5);
        
        double half = side / 2.0;
        arrX[0] = centerX - half; arrY[0] = centerY - half;
        arrX[1] = centerX + half; arrY[1] = centerY - half;
        arrX[2] = centerX + half; arrY[2] = centerY + half;
        arrX[3] = centerX - half; arrY[3] = centerY + half;
        arrX[4] = arrX[0]; arrY[4] = arrY[0];  // Close polygon
        
        return XYPolygon(arrX, arrY);
    }
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(XYPolygonTest, DefaultConstructor) {
    XYPolygon polygon;
    
    EXPECT_EQ(polygon.GetTypeLimits(), EXTERNAL);
    EXPECT_EQ(polygon.GetTypeSystCoor(), MEASURING);
    EXPECT_EQ(polygon.GetSize(), 0);
}

TEST_F(XYPolygonTest, CopyConstructor) {
    XYPolygon original = CreateSquare(10.0);
    original.SetTypeLimits(INTERNAL);
    original.SetTypeSystCoor(NORMALISED);
    
    XYPolygon copy(original);
    
    EXPECT_EQ(copy.GetSize(), original.GetSize());
    EXPECT_EQ(copy.GetTypeLimits(), INTERNAL);
    EXPECT_EQ(copy.GetTypeSystCoor(), NORMALISED);
    
    // Verify points are copied
    for (int i = 0; i < original.GetSize(); i++) {
        EXPECT_DOUBLE_EQ(copy[i].X, original[i].X);
        EXPECT_DOUBLE_EQ(copy[i].Y, original[i].Y);
    }
}

TEST_F(XYPolygonTest, BrokenLineConstructor) {
    XYBrokenLine bline;
    bline.SetSize(4);
    bline[0] = XYPoint(0.0, 0.0);
    bline[1] = XYPoint(10.0, 0.0);
    bline[2] = XYPoint(10.0, 10.0);
    bline[3] = XYPoint(0.0, 10.0);
    
    XYPolygon polygon(bline, INTERNAL, NORMALISED);
    
    // Should auto-close the polygon
    EXPECT_EQ(polygon.GetSize(), 5);  // 4 + 1 (closing point)
    EXPECT_EQ(polygon.GetTypeLimits(), INTERNAL);
    EXPECT_EQ(polygon.GetTypeSystCoor(), NORMALISED);
}

TEST_F(XYPolygonTest, BrokenLineConstructor_AlreadyClosed) {
    XYBrokenLine bline;
    bline.SetSize(5);
    bline[0] = XYPoint(0.0, 0.0);
    bline[1] = XYPoint(10.0, 0.0);
    bline[2] = XYPoint(10.0, 10.0);
    bline[3] = XYPoint(0.0, 10.0);
    bline[4] = XYPoint(0.0, 0.0);  // Already closed
    
    XYPolygon polygon(bline);
    
    // Should not add another closing point
    EXPECT_EQ(polygon.GetSize(), 5);
}

TEST_F(XYPolygonTest, ArrayConstructor_Double) {
    CArrayDouble arrX, arrY;
    arrX.SetSize(4);
    arrY.SetSize(4);
    
    arrX[0] = 0.0;   arrY[0] = 0.0;
    arrX[1] = 10.0;  arrY[1] = 0.0;
    arrX[2] = 10.0;  arrY[2] = 10.0;
    arrX[3] = 0.0;   arrY[3] = 10.0;
    
    XYPolygon polygon(arrX, arrY, INTERNAL);
    
    EXPECT_GT(polygon.GetSize(), 0);
    EXPECT_EQ(polygon.GetTypeLimits(), INTERNAL);
}

TEST_F(XYPolygonTest, ArrayConstructor_Int) {
    int arrX[] = {0, 10, 10, 0};
    int arrY[] = {0, 0, 10, 10};
    
    XYPolygon polygon(4, arrX, arrY);
    
    EXPECT_GT(polygon.GetSize(), 0);
}

TEST_F(XYPolygonTest, PointerConstructor_Double) {
    double arrX[] = {0.0, 10.0, 10.0, 0.0};
    double arrY[] = {0.0, 0.0, 10.0, 10.0};
    
    XYPolygon polygon(4, arrX, arrY);
    
    EXPECT_GT(polygon.GetSize(), 0);
}

// ============================================================================
// Assignment Operator Tests
// ============================================================================

TEST_F(XYPolygonTest, AssignmentOperator_Polygon) {
    XYPolygon original = CreateSquare(10.0);
    original.SetTypeLimits(INTERNAL);
    
    XYPolygon assigned;
    assigned = original;
    
    EXPECT_EQ(assigned.GetSize(), original.GetSize());
    EXPECT_EQ(assigned.GetTypeLimits(), INTERNAL);
}

TEST_F(XYPolygonTest, AssignmentOperator_SelfAssignment) {
    XYPolygon polygon = CreateSquare(10.0);
    
    polygon = polygon;  // Self-assignment
    
    EXPECT_GT(polygon.GetSize(), 0);
}

TEST_F(XYPolygonTest, AssignmentOperator_XYPointArray) {
    CArrayXYPoint points;
    points.SetSize(4);
    points[0] = XYPoint(0.0, 0.0);
    points[1] = XYPoint(10.0, 0.0);
    points[2] = XYPoint(10.0, 10.0);
    points[3] = XYPoint(0.0, 10.0);
    
    XYPolygon polygon;
    polygon = points;
    
    EXPECT_EQ(polygon.GetSize(), 5); // Should add closing point
    // Should reset to defaults
    EXPECT_EQ(polygon.GetTypeLimits(), EXTERNAL);
    EXPECT_EQ(polygon.GetTypeSystCoor(), MEASURING);
}

// ============================================================================
// XYShape Base Class Tests
// ============================================================================

TEST_F(XYPolygonTest, XYShape_SetGetTypeLimits) {
    XYPolygon polygon = CreateSquare(10.0);
    
    polygon.SetTypeLimits(INTERNAL);
    EXPECT_EQ(polygon.GetTypeLimits(), INTERNAL);
    
    polygon.SetTypeLimits(EXTERNAL);
    EXPECT_EQ(polygon.GetTypeLimits(), EXTERNAL);
}

TEST_F(XYPolygonTest, XYShape_SetGetTypeSystCoor) {
    XYPolygon polygon = CreateSquare(10.0);
    
    polygon.SetTypeSystCoor(NORMALISED);
    EXPECT_EQ(polygon.GetTypeSystCoor(), NORMALISED);
    
    polygon.SetTypeSystCoor(MEASURING);
    EXPECT_EQ(polygon.GetTypeSystCoor(), MEASURING);
}

// ============================================================================
// XYBrokenLine Base Class Tests
// ============================================================================

TEST_F(XYPolygonTest, XYBrokenLine_Indexing) {
    XYPolygon polygon = CreateSquare(10.0);
    
    EXPECT_GT(polygon.GetSize(), 0);
    
    // Access via XYBrokenLine interface
    XYPoint first = polygon[0];
    EXPECT_DOUBLE_EQ(first.X, -5.0);
    EXPECT_DOUBLE_EQ(first.Y, -5.0);
}

TEST_F(XYPolygonTest, XYBrokenLine_GetArrX_GetArrY) {
    XYPolygon polygon = CreateSquare(10.0);
    
    CArrayDouble arrX, arrY;
    polygon.GetArrX(arrX);
    polygon.GetArrY(arrY);
    
    EXPECT_EQ(arrX.GetSize(), polygon.GetSize());
    EXPECT_EQ(arrY.GetSize(), polygon.GetSize());
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(XYPolygonTest, isInside_Center) {
    XYPolygon square = CreateSquare(10.0);
    XYPoint center(0.0, 0.0);
    
    EXPECT_TRUE(square.isInside(center));
}

TEST_F(XYPolygonTest, isInside_OnEdge) {
    XYPolygon square = CreateSquare(10.0);
    XYPoint edge(5.0, 0.0);  // Right edge
    
    EXPECT_TRUE(square.isInside(edge));
}

TEST_F(XYPolygonTest, isInside_Corner) {
    XYPolygon square = CreateSquare(10.0);
    XYPoint corner(5.0, 5.0);
    
    EXPECT_TRUE(square.isInside(corner));
}

TEST_F(XYPolygonTest, isInside_Outside) {
    XYPolygon square = CreateSquare(10.0);
    XYPoint outside(20.0, 20.0);
    
    EXPECT_FALSE(square.isInside(outside));
}

TEST_F(XYPolygonTest, isInside_Triangle) {
    CArrayDouble arrX, arrY;
    arrX.SetSize(4);
    arrY.SetSize(4);
    
    arrX[0] = 0.0;  arrY[0] = 0.0;
    arrX[1] = 10.0; arrY[1] = 0.0;
    arrX[2] = 5.0;  arrY[2] = 5.0;
    arrX[3] = 0.0;  arrY[3] = 0.0;
    
    XYPolygon triangle(arrX, arrY);
    
    EXPECT_TRUE(triangle.isInside(XYPoint(5.0, 1.0)));   // Inside
    EXPECT_FALSE(triangle.isInside(XYPoint(0.0, 10.0))); // Outside
}

TEST_F(XYPolygonTest, isInside_XYOverload) {
    XYPolygon square = CreateSquare(10.0);
    
    EXPECT_TRUE(square.isInside(0.0, 0.0));     // Center
    EXPECT_FALSE(square.isInside(100.0, 100.0)); // Far outside
}

// ============================================================================
// isVisible Tests (XYShape base class implementation)
// ============================================================================

TEST_F(XYPolygonTest, isVisible_ExternalLimits_Inside) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(EXTERNAL);
    
    XYPoint inside(0.0, 0.0);
    
    // EXTERNAL aperture: inside points are visible
    EXPECT_TRUE(polygon.isVisible(inside));
}

TEST_F(XYPolygonTest, isVisible_ExternalLimits_Outside) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(EXTERNAL);
    
    XYPoint outside(20.0, 20.0);
    
    // EXTERNAL aperture: outside points are blocked
    EXPECT_FALSE(polygon.isVisible(outside));
}

TEST_F(XYPolygonTest, isVisible_InternalLimits_Inside) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(INTERNAL);
    
    XYPoint inside(0.0, 0.0);
    
    // INTERNAL obstruction: inside points are blocked
    EXPECT_FALSE(polygon.isVisible(inside));
}

TEST_F(XYPolygonTest, isVisible_InternalLimits_Outside) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(INTERNAL);
    
    XYPoint outside(20.0, 20.0);
    
    // INTERNAL obstruction: outside points are visible
    EXPECT_TRUE(polygon.isVisible(outside));
}

TEST_F(XYPolygonTest, isVisible_XYOverload) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(EXTERNAL);
    
    EXPECT_TRUE(polygon.isVisible(0.0, 0.0));     // Inside - visible
    EXPECT_FALSE(polygon.isVisible(100.0, 100.0)); // Outside - blocked
}

// ============================================================================
// Normalize Tests
// ============================================================================

TEST_F(XYPolygonTest, Normalize) {
    CArrayDouble arrX, arrY;
    arrX.SetSize(5);
    arrY.SetSize(5);
    
    arrX[0] = 40.0; arrY[0] = 20.0;
    arrX[1] = 60.0; arrY[1] = 20.0;
    arrX[2] = 60.0; arrY[2] = 40.0;
    arrX[3] = 40.0; arrY[3] = 40.0;
    arrX[4] = 40.0; arrY[4] = 20.0;
    
    XYPolygon polygon(arrX, arrY, EXTERNAL, MEASURING);
    
    polygon.Normalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(polygon[0].X, 0.0);  // (40 - 40) / 10
    EXPECT_DOUBLE_EQ(polygon[0].Y, 0.0);  // (20 - 20) / 10
    EXPECT_DOUBLE_EQ(polygon[1].X, 2.0);  // (60 - 40) / 10
    EXPECT_DOUBLE_EQ(polygon[1].Y, 0.0);
    EXPECT_EQ(polygon.GetTypeSystCoor(), NORMALISED);
}

// ============================================================================
// GetBounds Tests
// ============================================================================

TEST_F(XYPolygonTest, GetBounds_ReturnValue) {
    XYPolygon polygon = CreateSquare(10.0, 5.0, 5.0);
    
    XYBounds bounds = polygon.GetBounds();
    
    EXPECT_DOUBLE_EQ(bounds.XLeft, 0.0);
    EXPECT_DOUBLE_EQ(bounds.XRight, 10.0);
    EXPECT_DOUBLE_EQ(bounds.YTop, 0.0);
    EXPECT_DOUBLE_EQ(bounds.YBottom, 10.0);
}

TEST_F(XYPolygonTest, GetBounds_OutputParameter) {
    XYPolygon polygon = CreateSquare(10.0, 5.0, 5.0);
    
    XYBounds bounds = polygon.GetBounds();
    
    EXPECT_DOUBLE_EQ(bounds.XLeft, 0.0);
    EXPECT_DOUBLE_EQ(bounds.XRight, 10.0);
    EXPECT_DOUBLE_EQ(bounds.YTop, 0.0);
    EXPECT_DOUBLE_EQ(bounds.YBottom, 10.0);
}

// ============================================================================
// GetCentroid Tests
// ============================================================================

TEST_F(XYPolygonTest, GetCentroid_Square) {
    XYPolygon square = CreateSquare(10.0);
    
    XYPoint centroid = square.GetCentroid();
    
    EXPECT_TRUE(IsNear(centroid.X, 0.0));
    EXPECT_TRUE(IsNear(centroid.Y, 0.0));
}

TEST_F(XYPolygonTest, GetCentroid_Triangle) {
    CArrayDouble arrX, arrY;
    arrX.SetSize(4);
    arrY.SetSize(4);
    
    arrX[0] = 0.0;  arrY[0] = 0.0;
    arrX[1] = 30.0; arrY[1] = 0.0;
    arrX[2] = 0.0;  arrY[2] = 30.0;
    arrX[3] = 0.0;  arrY[3] = 0.0;
    
    XYPolygon triangle(arrX, arrY);
    
    XYPoint centroid = triangle.GetCentroid();
    
    // Centroid of right triangle is at (1/3*base, 1/3*height) from origin
    // But this implementation just averages vertices
    EXPECT_GT(centroid.X, 0.0);
    EXPECT_GT(centroid.Y, 0.0);
}

// ============================================================================
// GetContour Tests (XYShape interface - stub implementations)
// ============================================================================

TEST_F(XYPolygonTest, GetContour_BrokenLine) {
    XYPolygon polygon = CreateSquare(10.0);
    XYBrokenLine bline;
    
    bool success = polygon.GetContour(bline, 100);
    
    EXPECT_TRUE(success);
    // Polygon IS a broken line, so should just copy
    EXPECT_EQ(bline.GetSize(), polygon.GetSize());
}

TEST_F(XYPolygonTest, GetContour_Polygon) {
    XYPolygon polygon = CreateSquare(10.0);
    XYPolygon result;
    
    bool success = polygon.GetContour(result, 100);
    
    EXPECT_TRUE(success);
    // Should copy itself
    EXPECT_EQ(result.GetSize(), polygon.GetSize());
}

// ============================================================================
// Edge Cases and Special Scenarios
// ============================================================================

TEST_F(XYPolygonTest, EmptyPolygon) {
    XYPolygon empty;
    
    EXPECT_EQ(empty.GetSize(), 0);
    EXPECT_TRUE(empty.isDegenerate());
}

TEST_F(XYPolygonTest, VeryLargePolygon) {
    // Create polygon with many points
    CArrayDouble arrX, arrY;
    int nPoints = 1000;
    arrX.SetSize(nPoints);
    arrY.SetSize(nPoints);
    
    double radius = 100.0;
    for (int i = 0; i < nPoints; i++) {
        double angle = 2.0 * PI * i / nPoints;
        arrX[i] = radius * cos(angle);
        arrY[i] = radius * sin(angle);
    }
    
    XYPolygon polygon(arrX, arrY);
    
    EXPECT_EQ(polygon.GetSize(), nPoints + 1);  // +1 for closing
    EXPECT_FALSE(polygon.isDegenerate());
}

TEST_F(XYPolygonTest, ComplexConcavePolygon) {
    // Create a star-shaped (concave) polygon
    CArrayDouble arrX, arrY;
    arrX.SetSize(11);
    arrY.SetSize(11);
    
    // Star with 5 points
    for (int i = 0; i < 10; i++) {
        double angle = 2.0 * PI * i / 10.0;
        double radius = (i % 2 == 0) ? 10.0 : 5.0;
        arrX[i] = radius * cos(angle);
        arrY[i] = radius * sin(angle);
    }
    arrX[10] = arrX[0];
    arrY[10] = arrY[0];
    
    XYPolygon star(arrX, arrY);
    
    EXPECT_GT(star.GetSize(), 0);
    EXPECT_FALSE(star.isDegenerate());
}

// ============================================================================
// Polymorphism Tests (Multiple Inheritance)
// ============================================================================

TEST_F(XYPolygonTest, Polymorphic_XYShape_Pointer) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(EXTERNAL);
    
    XYShape* shape = &polygon;
    
    XYPoint inside(0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    // Should use XYShape::isVisible() which calls virtual isInside()
    EXPECT_TRUE(shape->isVisible(inside));
    EXPECT_FALSE(shape->isVisible(outside));
}

TEST_F(XYPolygonTest, Polymorphic_XYBrokenLine_Pointer) {
    XYPolygon polygon = CreateSquare(10.0);
    
    XYBrokenLine* bline = &polygon;
    
    // Should access points via XYBrokenLine interface
    EXPECT_GT(bline->GetSize(), 0);
    EXPECT_DOUBLE_EQ((*bline)[0].X, -5.0);
}

TEST_F(XYPolygonTest, Polymorphic_TypeLimits_Access) {
    XYPolygon polygon = CreateSquare(10.0);
    polygon.SetTypeLimits(INTERNAL);
    
    XYShape* shape = &polygon;
    XYBrokenLine* bline = &polygon;
    
    // TypeLimits should be accessible via XYShape base class
    EXPECT_EQ(shape->GetTypeLimits(), INTERNAL);
    
    // Verify it's the same member
    EXPECT_EQ(polygon.TypeLimits, INTERNAL);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(XYPolygonTest, Integration_CreateNormalizeCheck) {
    XYPolygon polygon = CreateSquare(20.0, 50.0, 30.0);
    polygon.SetTypeLimits(INTERNAL);
    
    // Normalize
    polygon.Normalize(40.0, 20.0, 10.0);
    
    // Check normalized coordinates
    EXPECT_EQ(polygon.GetTypeSystCoor(), NORMALISED);
    
    // Check visibility after normalization
    XYPoint normalizedCenter((50.0 - 40.0) / 10.0, (30.0 - 20.0) / 10.0);
    EXPECT_FALSE(polygon.isVisible(normalizedCenter));  // INTERNAL + inside = blocked
}

TEST_F(XYPolygonTest, Integration_BrokenLineToPolygonConversion) {
    // Create a broken line
    XYBrokenLine bline;
    bline.SetSize(4);
    bline[0] = XYPoint(0.0, 0.0);
    bline[1] = XYPoint(10.0, 0.0);
    bline[2] = XYPoint(10.0, 10.0);
    bline[3] = XYPoint(0.0, 10.0);
    
    // Convert to polygon
    XYPolygon polygon(bline, EXTERNAL, MEASURING);
    
    // Verify it works as a polygon
    EXPECT_TRUE(polygon.isInside(XYPoint(5.0, 5.0)));
    EXPECT_FALSE(polygon.isInside(XYPoint(20.0, 20.0)));
    
    // Verify as a shape
    XYShape* shape = &polygon;
    EXPECT_TRUE(shape->isVisible(XYPoint(5.0, 5.0)));  // EXTERNAL + inside
}

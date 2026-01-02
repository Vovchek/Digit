/// <summary>
/// Google Test suite for XYRect class
/// Tests constructors, methods, XYShape inheritance, and edge cases
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Tools/XYBounds.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include <cmath>

// ============================================================================
// Test Fixture for XYRect
// ============================================================================

class XYRectTest : public ::testing::Test {
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

TEST_F(XYRectTest, DefaultConstructor) {
    XYRect rect;
    
    EXPECT_DOUBLE_EQ(rect.Ax, 1.0);
    EXPECT_DOUBLE_EQ(rect.By, 1.0);
    EXPECT_DOUBLE_EQ(rect.Xc, 0.0);
    EXPECT_DOUBLE_EQ(rect.Yc, 0.0);
    EXPECT_DOUBLE_EQ(rect.Fi, 0.0);
    EXPECT_EQ(rect.TypeLimits, EXTERNAL);
    EXPECT_EQ(rect.TypeSystCoor, MEASURING);
}

TEST_F(XYRectTest, ParameterizedConstructor) {
    XYRect rect(10.0, 5.0, 20.0, 30.0, 45.0, INTERNAL, NORMALISED);
    
    EXPECT_DOUBLE_EQ(rect.Ax, 10.0);
    EXPECT_DOUBLE_EQ(rect.By, 5.0);
    EXPECT_DOUBLE_EQ(rect.Xc, 20.0);
    EXPECT_DOUBLE_EQ(rect.Yc, 30.0);
    EXPECT_DOUBLE_EQ(rect.Fi, 45.0);
    EXPECT_EQ(rect.TypeLimits, INTERNAL);
    EXPECT_EQ(rect.TypeSystCoor, NORMALISED);
    
    // Verify Si and Co are computed
    EXPECT_TRUE(IsNear(rect.Si, std::sin(45.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(rect.Co, std::cos(45.0 * GRD_RD)));
}

TEST_F(XYRectTest, CopyConstructor) {
    XYRect original(10.0, 5.0, 20.0, 30.0, 60.0);
    XYRect copy(original);
    
    EXPECT_DOUBLE_EQ(copy.Ax, original.Ax);
    EXPECT_DOUBLE_EQ(copy.By, original.By);
    EXPECT_DOUBLE_EQ(copy.Xc, original.Xc);
    EXPECT_DOUBLE_EQ(copy.Yc, original.Yc);
    EXPECT_DOUBLE_EQ(copy.Fi, original.Fi);
    EXPECT_DOUBLE_EQ(copy.Si, original.Si);
    EXPECT_DOUBLE_EQ(copy.Co, original.Co);
    EXPECT_EQ(copy.TypeLimits, original.TypeLimits);
    EXPECT_EQ(copy.TypeSystCoor, original.TypeSystCoor);
}

TEST_F(XYRectTest, BoundsConstructor) {
    XYBounds bounds;
    bounds.XLeft = 0.0;
    bounds.XRight = 20.0;
    bounds.YBottom = 30.0;
    bounds.YTop = 10.0;
    
    XYRect rect(bounds);
    
    EXPECT_DOUBLE_EQ(rect.Xc, 10.0);  // (0 + 20) / 2
    EXPECT_DOUBLE_EQ(rect.Yc, 20.0);  // (30 + 10) / 2
    EXPECT_DOUBLE_EQ(rect.Ax, 10.0);  // |20 - 0| / 2
    EXPECT_DOUBLE_EQ(rect.By, 10.0);  // |10 - 30| / 2
    EXPECT_DOUBLE_EQ(rect.Fi, 0.0);
}

// ============================================================================
// Assignment Operator Tests
// ============================================================================

TEST_F(XYRectTest, AssignmentOperator) {
    XYRect original(10.0, 5.0, 20.0, 30.0, 45.0, INTERNAL);
    XYRect assigned;
    
    assigned = original;
    
    EXPECT_DOUBLE_EQ(assigned.Ax, original.Ax);
    EXPECT_DOUBLE_EQ(assigned.By, original.By);
    EXPECT_DOUBLE_EQ(assigned.Xc, original.Xc);
    EXPECT_DOUBLE_EQ(assigned.Yc, original.Yc);
    EXPECT_DOUBLE_EQ(assigned.Fi, original.Fi);
    EXPECT_EQ(assigned.TypeLimits, original.TypeLimits);
    EXPECT_EQ(assigned.TypeSystCoor, original.TypeSystCoor);
}

TEST_F(XYRectTest, AssignmentOperator_SelfAssignment) {
    XYRect rect(10.0, 5.0, 20.0, 30.0, 45.0);
    
    rect = rect;  // Self-assignment
    
    EXPECT_DOUBLE_EQ(rect.Ax, 10.0);
    EXPECT_DOUBLE_EQ(rect.By, 5.0);
}

// ============================================================================
// XYShape Base Class Tests (TypeLimits/TypeSystCoor)
// ============================================================================

TEST_F(XYRectTest, XYShape_SetGetTypeLimits) {
    XYRect rect;
    
    rect.SetTypeLimits(INTERNAL);
    EXPECT_EQ(rect.GetTypeLimits(), INTERNAL);
    
    rect.SetTypeLimits(EXTERNAL);
    EXPECT_EQ(rect.GetTypeLimits(), EXTERNAL);
}

TEST_F(XYRectTest, XYShape_SetGetTypeSystCoor) {
    XYRect rect;
    
    rect.SetTypeSystCoor(NORMALISED);
    EXPECT_EQ(rect.GetTypeSystCoor(), NORMALISED);
    
    rect.SetTypeSystCoor(MEASURING);
    EXPECT_EQ(rect.GetTypeSystCoor(), MEASURING);
}

// ============================================================================
// Perimeter Tests
// ============================================================================

TEST_F(XYRectTest, Perimeter_Square) {
    XYRect square(5.0, 5.0, 0.0, 0.0, 0.0);
    
    double perimeter = square.Perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 40.0);  // 4 * (5 + 5)
}

TEST_F(XYRectTest, Perimeter_Rectangle) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    
    double perimeter = rect.Perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 60.0);  // 4 * (10 + 5)
}

TEST_F(XYRectTest, Perimeter_RotatedRectangle) {
    // Rotation shouldn't affect perimeter
    XYRect rect(10.0, 5.0, 0.0, 0.0, 45.0);
    
    double perimeter = rect.Perimeter();
    
    EXPECT_DOUBLE_EQ(perimeter, 60.0);
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(XYRectTest, isInside_Center) {
    XYRect rect(10.0, 5.0, 20.0, 30.0, 0.0);
    XYPoint center(20.0, 30.0);
    
    EXPECT_TRUE(rect.isInside(center));
}

TEST_F(XYRectTest, isInside_Corner) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPoint corner(10.0, 5.0);  // Top-right corner
    
    EXPECT_TRUE(rect.isInside(corner));
}

TEST_F(XYRectTest, isInside_OnEdge) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPoint edge(10.0, 0.0);  // Right edge
    
    EXPECT_TRUE(rect.isInside(edge));
}

TEST_F(XYRectTest, isInside_Outside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    EXPECT_FALSE(rect.isInside(outside));
}

TEST_F(XYRectTest, isInside_RotatedRectangle) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 45.0);
    XYPoint center(0.0, 0.0);
    
    EXPECT_TRUE(rect.isInside(center));
}

TEST_F(XYRectTest, isInside_XYOverload) {
    XYRect rect(10.0, 5.0, 20.0, 30.0, 0.0);
    
    EXPECT_TRUE(rect.isInside(20.0, 30.0));   // Center
    EXPECT_FALSE(rect.isInside(100.0, 100.0)); // Far outside
}

// ============================================================================
// isVisible Tests (XYShape base class implementation)
// ============================================================================

TEST_F(XYRectTest, isVisible_ExternalLimits_Inside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYPoint inside(0.0, 0.0);
    
    // EXTERNAL aperture: inside points are visible
    EXPECT_TRUE(rect.isVisible(inside));
}

TEST_F(XYRectTest, isVisible_ExternalLimits_Outside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYPoint outside(20.0, 20.0);
    
    // EXTERNAL aperture: outside points are blocked
    EXPECT_FALSE(rect.isVisible(outside));
}

TEST_F(XYRectTest, isVisible_InternalLimits_Inside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, INTERNAL);
    XYPoint inside(0.0, 0.0);
    
    // INTERNAL obstruction: inside points are blocked
    EXPECT_FALSE(rect.isVisible(inside));
}

TEST_F(XYRectTest, isVisible_InternalLimits_Outside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, INTERNAL);
    XYPoint outside(20.0, 20.0);
    
    // INTERNAL obstruction: outside points are visible
    EXPECT_TRUE(rect.isVisible(outside));
}

TEST_F(XYRectTest, isVisible_XYOverload) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    EXPECT_TRUE(rect.isVisible(0.0, 0.0));     // Inside - visible
    EXPECT_FALSE(rect.isVisible(100.0, 100.0)); // Outside - blocked
}

// ============================================================================
// Transformation Tests
// ============================================================================

TEST_F(XYRectTest, Normalize) {
    XYRect rect(20.0, 10.0, 50.0, 30.0, 0.0, EXTERNAL, MEASURING);
    
    rect.Normalize(40.0, 20.0, 10.0);
    
    EXPECT_DOUBLE_EQ(rect.Ax, 2.0);   // 20 / 10
    EXPECT_DOUBLE_EQ(rect.By, 1.0);   // 10 / 10
    EXPECT_DOUBLE_EQ(rect.Xc, 1.0);   // (50 - 40) / 10
    EXPECT_DOUBLE_EQ(rect.Yc, 1.0);   // (30 - 20) / 10
    EXPECT_EQ(rect.TypeSystCoor, NORMALISED);
}

// ============================================================================
// GetContour Tests
// ============================================================================

TEST_F(XYRectTest, GetContour_BrokenLine_MinimumPoints) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, 4);  // Request 4 points
    
    EXPECT_TRUE(success);
    EXPECT_GT(bline.GetSize(), 0);
}

TEST_F(XYRectTest, GetContour_BrokenLine_ManyPoints) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, 100);
    
    EXPECT_TRUE(success);
    EXPECT_GT(bline.GetSize(), 0);
    
    // All points should be inside or on the rectangle
    for (int i = 0; i < bline.GetSize(); i++) {
        EXPECT_TRUE(rect.isInside(bline[i])) 
            << "Point " << i << " at (" << bline[i].X << "," << bline[i].Y 
            << ") should be on rectangle boundary";
    }
}

TEST_F(XYRectTest, GetContour_BrokenLine_Step) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, 1.0);
    
    EXPECT_TRUE(success);
    EXPECT_GT(bline.GetSize(), 0);
}

TEST_F(XYRectTest, GetContour_BrokenLine_InvalidStep) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, -1.0);
    
    EXPECT_FALSE(success);
}

TEST_F(XYRectTest, GetContour_BrokenLine_TooFewPoints) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, 1);  // Too few
    
    EXPECT_FALSE(success);
}

TEST_F(XYRectTest, GetContour_Polygon_Count) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPolygon polygon;
    
    bool success = rect.GetContour(polygon, 100);
    
    EXPECT_TRUE(success);
    EXPECT_GT(polygon.GetSize(), 0);
}

TEST_F(XYRectTest, GetContour_Polygon_TypesPreserved) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, INTERNAL, NORMALISED);
    XYPolygon polygon;
    
    rect.GetContour(polygon, 50);
    
    EXPECT_EQ(polygon.GetTypeLimits(), INTERNAL);
    EXPECT_EQ(polygon.GetTypeSystCoor(), NORMALISED);
}

TEST_F(XYRectTest, GetContour_DegenerateRectangle) {
    XYRect rect(0.0, 0.0, 0.0, 0.0, 0.0);
    XYBrokenLine bline;
    
    bool success = rect.GetContour(bline, 100);
    
    EXPECT_FALSE(success);  // Should fail for degenerate rectangle
}

// ============================================================================
// Set Method Tests
// ============================================================================

TEST_F(XYRectTest, SetMethod) {
    XYRect rect;
    
    rect.Set(15.0, 8.0, 25.0, 35.0, 30.0, INTERNAL, NORMALISED);
    
    EXPECT_DOUBLE_EQ(rect.Ax, 15.0);
    EXPECT_DOUBLE_EQ(rect.By, 8.0);
    EXPECT_DOUBLE_EQ(rect.Xc, 25.0);
    EXPECT_DOUBLE_EQ(rect.Yc, 35.0);
    EXPECT_DOUBLE_EQ(rect.Fi, 30.0);
    EXPECT_EQ(rect.TypeLimits, INTERNAL);
    EXPECT_EQ(rect.TypeSystCoor, NORMALISED);
    
    // Verify Si and Co are updated
    EXPECT_TRUE(IsNear(rect.Si, std::sin(30.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(rect.Co, std::cos(30.0 * GRD_RD)));
}

// ============================================================================
// Friend Function Tests
// ============================================================================

TEST_F(XYRectTest, FriendFunction_isInside) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPoint inside(0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    EXPECT_TRUE(isInside(rect, inside));
    EXPECT_FALSE(isInside(rect, outside));
}

TEST_F(XYRectTest, FriendFunction_isInside_BUG_UsesEllipseFormula) {
  
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    
    // Corner point (10, 5) should be inside rectangle
    
    XYPoint corner(10.0, 5.0);
    
    EXPECT_TRUE(rect.isInside(corner))         // Member function - CORRECT
        << "Member function correctly identifies corner as inside";
    
    EXPECT_TRUE(isInside(rect, corner))       // Friend function
        << "Friend function CORRECTLY identifies corner as inside";
}

TEST_F(XYRectTest, FriendFunction_GetContour) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0);
    XYPolygon polygon;
    
    GetContour(rect, polygon, 100);
    
    EXPECT_GT(polygon.GetSize(), 0);
}

// ============================================================================
// Edge Cases and Special Scenarios
// ============================================================================

TEST_F(XYRectTest, VerySmallRectangle) {
    XYRect rect(0.001, 0.001, 0.0, 0.0, 0.0);
    XYPoint center(0.0, 0.0);
    
    EXPECT_TRUE(rect.isInside(center));
    EXPECT_GT(rect.Perimeter(), 0.0);
}

TEST_F(XYRectTest, VeryLargeRectangle) {
    XYRect rect(1e6, 1e6, 0.0, 0.0, 0.0);
    XYPoint point(1e5, 1e5);
    
    EXPECT_TRUE(rect.isInside(point));
}

TEST_F(XYRectTest, HighlyAsymmetricRectangle) {
    XYRect rect(100.0, 1.0, 0.0, 0.0, 0.0);
    XYPoint point(50.0, 0.0);
    
    EXPECT_TRUE(rect.isInside(point));
}

TEST_F(XYRectTest, Rotation90Degrees) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 90.0);
    
    // At 90 degrees, Si ≈ 1, Co ≈ 0
    EXPECT_TRUE(IsNear(rect.Si, std::sin(90.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(rect.Co, std::cos(90.0 * GRD_RD)));
}

TEST_F(XYRectTest, Rotation180Degrees) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 180.0);
    
    // At 180 degrees, Si ≈ 0, Co ≈ -1
    EXPECT_TRUE(IsNear(rect.Si, std::sin(180.0 * GRD_RD), 0.01));
    EXPECT_TRUE(IsNear(rect.Co, std::cos(180.0 * GRD_RD), 0.01));
}

TEST_F(XYRectTest, Rotation360Degrees) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 360.0);
    
    // 360 degrees should be equivalent to 0 degrees
    EXPECT_TRUE(IsNear(rect.Si, 0.0, 0.01));
    EXPECT_TRUE(IsNear(rect.Co, 1.0, 0.01));
}

TEST_F(XYRectTest, NegativeRotation) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, -45.0);
    
    EXPECT_TRUE(IsNear(rect.Si, std::sin(-45.0 * GRD_RD)));
    EXPECT_TRUE(IsNear(rect.Co, std::cos(-45.0 * GRD_RD)));
}

// ============================================================================
// Polymorphism Tests (XYShape base class)
// ============================================================================

TEST_F(XYRectTest, Polymorphic_BasePointer_isVisible) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYShape* shape = &rect;
    
    XYPoint inside(0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    // Should use XYShape::isVisible() which calls virtual isInside()
    EXPECT_TRUE(shape->isVisible(inside));
    EXPECT_FALSE(shape->isVisible(outside));
}

TEST_F(XYRectTest, Polymorphic_BasePointer_GetTypeLimits) {
    XYRect rect(10.0, 5.0, 0.0, 0.0, 0.0, INTERNAL);
    XYShape* shape = &rect;
    
    EXPECT_EQ(shape->GetTypeLimits(), INTERNAL);
    
    shape->SetTypeLimits(EXTERNAL);
    EXPECT_EQ(shape->GetTypeLimits(), EXTERNAL);
    EXPECT_EQ(rect.TypeLimits, EXTERNAL);  // Verify it's the same member
}

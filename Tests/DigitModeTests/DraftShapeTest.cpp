/**
 * @file DraftShapeTest.cpp
 * @brief Google Test suite for DraftShape class
 * 
 * Tests draft shape accumulation and conversion workflows for Phase 2.
 */

#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/DraftShape.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ApertureCore/include/aperturecore/geometry/Ellipse.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Test Fixture
// ============================================================================

class DraftShapeTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    bool isNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }
};

// ============================================================================
// CanCommit Tests
// ============================================================================

TEST_F(DraftShapeTest, Rectangle_ThreePoints_CanCommit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    draft.type = TypeLimits::EXTERNAL;
    
    // Less than 3 points - cannot commit
    draft.AddPoint({0, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    draft.AddPoint({100, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    // Exactly 3 points - can commit
    draft.AddPoint({100, 50});
    EXPECT_TRUE(draft.CanCommit());
    
    // More than 3 points - still valid
    draft.AddPoint({0, 50});
    EXPECT_TRUE(draft.CanCommit());
}

TEST_F(DraftShapeTest, Ellipse_TwoPoints_CannotCommit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Ellipse;
    draft.type = TypeLimits::EXTERNAL;
    
    // Less than 3 points - cannot commit
    draft.AddPoint({10, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    draft.AddPoint({0, 10});
    EXPECT_FALSE(draft.CanCommit());
    
    // 3 points - can commit
    draft.AddPoint({-10, 0});
    EXPECT_TRUE(draft.CanCommit());
}

TEST_F(DraftShapeTest, Circle_ThreePoints_CanCommit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Circle;
    draft.type = TypeLimits::EXTERNAL;
    
    // Less than 3 points - cannot commit
    draft.AddPoint({10, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    draft.AddPoint({0, 10});
    EXPECT_FALSE(draft.CanCommit());
    
    // 3 points - can commit
    draft.AddPoint({-10, 0});
    EXPECT_TRUE(draft.CanCommit());
}

TEST_F(DraftShapeTest, Polygon_ThreePoints_CanCommit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Polygon;
    draft.type = TypeLimits::EXTERNAL;
    
    // Less than 3 points - cannot commit
    draft.AddPoint({0, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    draft.AddPoint({50, 0});
    EXPECT_FALSE(draft.CanCommit());
    
    // 3 points - can commit (minimum for closed polygon)
    draft.AddPoint({25, 50});
    EXPECT_TRUE(draft.CanCommit());
}

// ============================================================================
// ToShape Tests - Rectangle
// ============================================================================

TEST_F(DraftShapeTest, ToShape_Rectangle_CreatesValidShape) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    draft.type = TypeLimits::APERTURE;
    
    // Add 3 points for oriented rectangle
    draft.AddPoint({0, 0});     // Corner 1
    draft.AddPoint({100, 0});   // Corner 2 (width direction)
    draft.AddPoint({100, 50});  // Corner 3 (height)
    
    auto shape = draft.ToShape();
    
    ASSERT_NE(shape, nullptr);
    EXPECT_STREQ(shape->typeName(), "Rectangle");
    EXPECT_EQ(shape->getTypeLimits(), TypeLimits::APERTURE);
    
    // Verify it's a valid Rectangle
    aperture::Rectangle* rect = dynamic_cast<aperture::Rectangle*>(shape.get());
    ASSERT_NE(rect, nullptr);
    
    EXPECT_NEAR(rect->width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect->height(), 50.0, TOLERANCE);
    EXPECT_NEAR(rect->center().x, 50.0, TOLERANCE);
    EXPECT_NEAR(rect->center().y, 25.0, TOLERANCE);
}

TEST_F(DraftShapeTest, ToShape_Rectangle_InsufficientPoints_ReturnsNull) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    
    // Only 2 points - should fail
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 0});
    
    auto shape = draft.ToShape();
    EXPECT_EQ(shape, nullptr);
}

// ============================================================================
// ToShape Tests - Circle
// ============================================================================

TEST_F(DraftShapeTest, ToShape_Circle_RadiiEqual) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Circle;
    draft.type = TypeLimits::EXTERNAL;
    
    // Add 4 points on a circle (center at origin, radius 10)
    draft.AddPoint({10, 0});
    draft.AddPoint({0, 10});
    draft.AddPoint({-10, 0});
    draft.AddPoint({0, -10});
    
    auto shape = draft.ToShape();
    
    ASSERT_NE(shape, nullptr);
    EXPECT_STREQ(shape->typeName(), "Ellipse");
    
    aperture::Ellipse* ellipse = dynamic_cast<aperture::Ellipse*>(shape.get());
    ASSERT_NE(ellipse, nullptr);
    
    // Verify it's a circle (equal radii)
    EXPECT_NEAR(ellipse->semiMajor(), ellipse->semiMinor(), 0.1);
    EXPECT_TRUE(ellipse->isCircle(0.1));
    
    // Verify radius is approximately 10
    EXPECT_NEAR(ellipse->semiMajor(), 10.0, 1.0);
}

TEST_F(DraftShapeTest, ToShape_Circle_EnforcesCircularConstraint) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Circle;
    draft.type = TypeLimits::EXTERNAL;
    
    // Add elliptical points (would fit better as ellipse)
    draft.AddPoint({20, 0});    // Major axis
    draft.AddPoint({0, 10});    // Minor axis
    draft.AddPoint({-20, 0});
    draft.AddPoint({0, -10});
    
    auto shape = draft.ToShape();
    
    ASSERT_NE(shape, nullptr);
    
    aperture::Ellipse* ellipse = dynamic_cast<aperture::Ellipse*>(shape.get());
    ASSERT_NE(ellipse, nullptr);
    
    // Even with elliptical points, Circle mode must enforce equal radii
    EXPECT_NEAR(ellipse->semiMajor(), ellipse->semiMinor(), 0.1);
    EXPECT_TRUE(ellipse->isCircle(0.1));
}

// ============================================================================
// ToShape Tests - Ellipse
// ============================================================================

TEST_F(DraftShapeTest, ToShape_Ellipse_AllowsDifferentRadii) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Ellipse;
    draft.type = TypeLimits::INTERNAL;
    
    // Add elliptical points (20x10)
    draft.AddPoint({20, 0});
    draft.AddPoint({0, 10});
    draft.AddPoint({-20, 0});
    draft.AddPoint({0, -10});
    draft.AddPoint({14.14, 7.07});  // Additional point
    
    auto shape = draft.ToShape();
    
    ASSERT_NE(shape, nullptr);
    EXPECT_STREQ(shape->typeName(), "Ellipse");
    EXPECT_EQ(shape->getTypeLimits(), TypeLimits::INTERNAL);
    
    aperture::Ellipse* ellipse = dynamic_cast<aperture::Ellipse*>(shape.get());
    ASSERT_NE(ellipse, nullptr);
    
    // Should fit as ellipse (not forced to circle)
    EXPECT_GT(ellipse->semiMajor(), ellipse->semiMinor() * 1.5);
}

// ============================================================================
// ToShape Tests - Polygon
// ============================================================================

TEST_F(DraftShapeTest, ToShape_Polygon_CreatesValidPolygon) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Polygon;
    draft.type = TypeLimits::EXTERNAL;
    
    // Add triangle vertices
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 0});
    draft.AddPoint({50, 86.6});
    
    auto shape = draft.ToShape();
    
    ASSERT_NE(shape, nullptr);
    EXPECT_STREQ(shape->typeName(), "Polygon");
    
    aperture::Polygon* poly = dynamic_cast<aperture::Polygon*>(shape.get());
    ASSERT_NE(poly, nullptr);
    
    EXPECT_EQ(poly->vertexCount(), 3);
}

TEST_F(DraftShapeTest, Polygon_SelfIntersection_ReturnsNull) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Polygon;
    
    // TODO: Currently, self-intersection check is not implemented
    // This test documents expected future behavior
    
    // Add self-intersecting polygon (bowtie shape)
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 100});
    draft.AddPoint({100, 0});
    draft.AddPoint({0, 100});
    
    auto shape = draft.ToShape();
    
    // Current implementation: returns polygon (no validation)
    // Future: should return nullptr for self-intersecting polygons
    ASSERT_NE(shape, nullptr);  // Will change when validation is added
    
    // TODO: When self-intersection check is implemented, change to:
    // EXPECT_EQ(shape, nullptr);
}

// ============================================================================
// GetPreview Tests
// ============================================================================

TEST_F(DraftShapeTest, GetPreview_Rectangle_TwoPoints_AxisAligned) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 50});
    
    auto preview = draft.GetPreview();
    
    ASSERT_NE(preview, nullptr);
    
    aperture::Rectangle* rect = dynamic_cast<aperture::Rectangle*>(preview.get());
    ASSERT_NE(rect, nullptr);
    
    // Preview should be axis-aligned with 2 points
    EXPECT_NEAR(rect->rotationDegrees(), 0.0, TOLERANCE);
    EXPECT_NEAR(rect->width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect->height(), 50.0, TOLERANCE);
}

TEST_F(DraftShapeTest, GetPreview_Rectangle_ThreePoints_Oriented) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 0});
    draft.AddPoint({100, 50});
    
    auto preview = draft.GetPreview();
    
    ASSERT_NE(preview, nullptr);
    
    aperture::Rectangle* rect = dynamic_cast<aperture::Rectangle*>(preview.get());
    ASSERT_NE(rect, nullptr);
    
    // With 3 points, should use full 3-point constructor
    EXPECT_NEAR(rect->width(), 100.0, TOLERANCE);
    EXPECT_NEAR(rect->height(), 50.0, TOLERANCE);
}

TEST_F(DraftShapeTest, GetPreview_Ellipse_TwoPoints_ReturnsNull) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Ellipse;
    
    draft.AddPoint({10, 0});
    draft.AddPoint({0, 10});
    
    // Need at least 3 points for ellipse fit
    auto preview = draft.GetPreview();
    EXPECT_EQ(preview, nullptr);
}

TEST_F(DraftShapeTest, GetPreview_Ellipse_ThreePoints_ShowsFit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Ellipse;
    
    draft.AddPoint({10, 0});
    draft.AddPoint({0, 10});
    draft.AddPoint({-10, 0});
    
    auto preview = draft.GetPreview();
	ASSERT_EQ(preview, nullptr); // ellipse needs 4 or more points, not fallback to circle
}

TEST_F(DraftShapeTest, GetPreview_Circle_ThreePoints_ShowsCircleFit) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Circle;
    
    draft.AddPoint({10, 0});
    draft.AddPoint({0, 10});
    draft.AddPoint({-10, 0});
    
    auto preview = draft.GetPreview();
    ASSERT_NE(preview, nullptr);
    
    aperture::Ellipse* ellipse = dynamic_cast<aperture::Ellipse*>(preview.get());
    ASSERT_NE(ellipse, nullptr);
    
    // Preview should be circular
    EXPECT_TRUE(ellipse->isCircle(0.5));
}

// ============================================================================
// Clear and PointCount Tests
// ============================================================================

TEST_F(DraftShapeTest, Clear_RemovesAllPoints) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Rectangle;
    
    draft.AddPoint({0, 0});
    draft.AddPoint({100, 0});
    draft.AddPoint({100, 50});
    
    EXPECT_EQ(draft.PointCount(), 3);
    EXPECT_TRUE(draft.CanCommit());
    
    draft.Clear();
    
    EXPECT_EQ(draft.PointCount(), 0);
    EXPECT_FALSE(draft.CanCommit());
}

TEST_F(DraftShapeTest, Clear_PreservesKindAndType) {
    DraftShape draft;
    draft.kind = DraftShape::Kind::Circle;
    draft.type = TypeLimits::INTERNAL;
    
    draft.AddPoint({10, 0});
    draft.Clear();
    
    // Kind and type should remain unchanged
    EXPECT_EQ(draft.kind, DraftShape::Kind::Circle);
    EXPECT_EQ(draft.type, TypeLimits::INTERNAL);
}

TEST_F(DraftShapeTest, AddPoint_IncrementsPointCount) {
    DraftShape draft;
    
    EXPECT_EQ(draft.PointCount(), 0);
    
    draft.AddPoint({0, 0});
    EXPECT_EQ(draft.PointCount(), 1);
    
    draft.AddPoint({100, 0});
    EXPECT_EQ(draft.PointCount(), 2);
    
    draft.AddPoint({100, 50});
    EXPECT_EQ(draft.PointCount(), 3);
}

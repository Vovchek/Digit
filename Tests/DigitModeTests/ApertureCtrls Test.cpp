/**
 * @file ApertureCtrlsTest.cpp
 * @brief Unit tests for CApertureCtrls (modern aperture control)
 * 
 * Tests written from scratch using modern C++ and ApertureCore shapes.
 * No legacy XYShape or GetExtRealBound patterns.
 */
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Controls\CApertureCtrls.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Test Fixture
// ============================================================================

class ApertureCtrlsTest : public ::testing::Test {
protected:
    void SetUp() override {
        apertureCtrls.Init();
    }
    
    void TearDown() override {
        apertureCtrls.Clear();
    }
    
    CApertureCtrls apertureCtrls;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, InitialState_IsEmpty) {
    EXPECT_EQ(0, apertureCtrls.GetShapeCount());
    EXPECT_EQ(0, apertureCtrls.GetExternalCount());
    EXPECT_EQ(0, apertureCtrls.GetInternalCount());
    EXPECT_EQ(0, apertureCtrls.GetApertureCount());
}

TEST_F(ApertureCtrlsTest, Clear_RemovesAllShapes) {
    // Add some shapes
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    apertureCtrls.AddExternalShape(std::move(ellipse));
    
    auto rect = std::make_unique<aperture::Rectangle>(50.0, 50.0, 200.0, 200.0);
    apertureCtrls.AddInternalShape(std::move(rect));
    
    ASSERT_EQ(2, apertureCtrls.GetShapeCount());
    
    // Clear
    apertureCtrls.Clear();
    
    EXPECT_EQ(0, apertureCtrls.GetShapeCount());
}

// ============================================================================
// Shape Management Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, AddExternalShape_IncreasesCount) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(ellipse));
    
    EXPECT_EQ(TypeLimits::EXTERNAL, handle.type);
    EXPECT_EQ(0, handle.index);
    EXPECT_EQ(1, apertureCtrls.GetExternalCount());
    EXPECT_EQ(1, apertureCtrls.GetShapeCount());
}

TEST_F(ApertureCtrlsTest, AddInternalShape_IncreasesCount) {
    auto rect = std::make_unique<aperture::Rectangle>(20.0, 20.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddInternalShape(std::move(rect));
    
    EXPECT_EQ(TypeLimits::INTERNAL, handle.type);
    EXPECT_EQ(0, handle.index);
    EXPECT_EQ(1, apertureCtrls.GetInternalCount());
}

TEST_F(ApertureCtrlsTest, AddApertureShape_IncreasesCount) {
    auto polygon = std::make_unique<aperture::Polygon>(std::vector<Point>{
        {0, 0}, {10, 0}, {10, 10}, {0, 10}
    });
    ShapeHandle handle = apertureCtrls.AddApertureShape(std::move(polygon));
    
    EXPECT_EQ(TypeLimits::APERTURE, handle.type);
    EXPECT_EQ(0, handle.index);
    EXPECT_EQ(1, apertureCtrls.GetApertureCount());
}

TEST_F(ApertureCtrlsTest, AddMultipleShapes_TracksCountsCorrectly) {
    apertureCtrls.AddExternalShape(std::make_unique<aperture::Ellipse>(50, 50, 100, 100));
    apertureCtrls.AddExternalShape(std::make_unique<aperture::Rectangle>(50, 50, 200, 200));
    apertureCtrls.AddInternalShape(std::make_unique<aperture::Ellipse>(10, 10, 100, 100));
    
    EXPECT_EQ(2, apertureCtrls.GetExternalCount());
    EXPECT_EQ(1, apertureCtrls.GetInternalCount());
    EXPECT_EQ(0, apertureCtrls.GetApertureCount());
    EXPECT_EQ(3, apertureCtrls.GetShapeCount());
}

TEST_F(ApertureCtrlsTest, AddNullShape_ReturnsInvalidHandle) {
    ShapeHandle handle = apertureCtrls.AddExternalShape(nullptr);
    // Null shape returns default handle, GetShape will return nullptr
    EXPECT_EQ(nullptr, apertureCtrls.GetShape(handle));
}

// ============================================================================
// Shape Access Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, GetShape_ValidHandle_ReturnsShape) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(ellipse));
    
    const Shape* shape = apertureCtrls.GetShape(handle);
    
    ASSERT_NE(nullptr, shape);
    EXPECT_STREQ("Ellipse", shape->typeName());  // Use EXPECT_STREQ for C-string comparison
}

TEST_F(ApertureCtrlsTest, GetShape_InvalidHandle_ReturnsNull) {
    ShapeHandle invalid{999, TypeLimits::EXTERNAL};  // Index out of range
    
    const Shape* shape = apertureCtrls.GetShape(invalid);
    
    EXPECT_EQ(nullptr, shape);
}

TEST_F(ApertureCtrlsTest, GetShape_AfterAddingMore_StillValid) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(ellipse));
    
    // Add another shape - handle should still be valid (just index + type)
    apertureCtrls.AddExternalShape(std::make_unique<aperture::Ellipse>(30, 30, 50, 50));
    
    // Original handle is still valid
    const Shape* shape = apertureCtrls.GetShape(handle);
    EXPECT_NE(nullptr, shape);
}

TEST_F(ApertureCtrlsTest, GetShapeForEdit_ReturnsNonConstPointer) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(ellipse));
    
    Shape* shape = apertureCtrls.GetShapeForEdit(handle);
    
    ASSERT_NE(nullptr, shape);
    // Can modify (this is compile-time check)
    shape->setTypeLimits(TypeLimits::EXTERNAL);
}

// ============================================================================
// Version Tracking Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, Version_InitiallyNonZero) {
    EXPECT_GT(apertureCtrls.GetVersion(), 0);
}

TEST_F(ApertureCtrlsTest, AddShape_IncrementsVersion) {
    uint64_t v1 = apertureCtrls.GetVersion();
    
    apertureCtrls.AddExternalShape(std::make_unique<aperture::Ellipse>(50, 50, 100, 100));
    
    uint64_t v2 = apertureCtrls.GetVersion();
    EXPECT_GT(v2, v1);
}

TEST_F(ApertureCtrlsTest, NotifyShapeModified_IncrementsVersion) {
    auto ellipse = std::make_unique<aperture::Ellipse>(50.0, 50.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(ellipse));
    
    uint64_t v1 = apertureCtrls.GetVersion();
    
    apertureCtrls.NotifyShapeModified();
    
    uint64_t v2 = apertureCtrls.GetVersion();
    EXPECT_GT(v2, v1);
}

// ============================================================================
// Hit Testing Tests
// ============================================================================

TEST_F(ApertureCtrlsTest, HitTest_PointInsideShape_ReturnsHit) {
    // Add a rectangle at (50, 50) with size 100x100
    auto rect = std::make_unique<aperture::Rectangle>(100.0, 100.0, 100.0, 100.0);
    ShapeHandle handle = apertureCtrls.AddExternalShape(std::move(rect));
    
    // Test point at center
    Point testPoint{100.0, 100.0};
    auto result = apertureCtrls.HitTest(testPoint, 5.0);
    
    EXPECT_TRUE(result.hitShape());
    EXPECT_TRUE(result.hitBody());
    EXPECT_FALSE(result.hitControlPoint());
}

TEST_F(ApertureCtrlsTest, HitTest_PointOutsideShape_ReturnsNoHit) {
    auto rect = std::make_unique<aperture::Rectangle>(100.0, 100.0, 100.0, 100.0);
    apertureCtrls.AddExternalShape(std::move(rect));
    
    // Test point far outside
    Point testPoint{500.0, 500.0};
    auto result = apertureCtrls.HitTest(testPoint, 5.0);
    
    EXPECT_FALSE(result.hitShape());
}

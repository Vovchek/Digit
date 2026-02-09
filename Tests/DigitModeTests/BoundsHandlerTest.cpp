#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/BoundsHandler.h"
#include "ImageTempl/ViewTransform.h"
#include "Controls/BoundCtrls.h"
#include "Controls/ImageCtrls.h"

using namespace DigitMode;

/**
 * @brief Test fixture for BoundsHandler
 * 
 * Phase 1 tests focus on:
 * - Initialization
 * - ViewTransform coordinate conversions
 * - Basic state management
 */
class BoundsHandlerTest : public ::testing::Test {
protected:
    BoundsHandler handler;
    ViewTransform viewTransform;

    void SetUp() override {
        // Default ViewTransform state (scale=1.0, offset=0)
    }

    void TearDown() override {
        // No cleanup needed for this phase
    }
};

// ===== Initialization Tests =====

TEST_F(BoundsHandlerTest, DefaultConstruction) {
    EXPECT_FALSE(handler.IsInitialized());
}

TEST_F(BoundsHandlerTest, InitializationWithViewTransform) {
    handler.SetViewTransform(&viewTransform);
    EXPECT_TRUE(handler.IsInitialized());
}

// ===== Coordinate Transformation Tests =====

TEST_F(BoundsHandlerTest, ScreenToWorldIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0)
    handler.SetViewTransform(&viewTransform);
    CPoint screenPt(100, 200);
    
    // Act
    CPoint worldPt = handler.ScreenToWorld(screenPt);
    
    // Assert: With identity transform, screen == world
    EXPECT_EQ(100, worldPt.x);
    EXPECT_EQ(200, worldPt.y);
}

TEST_F(BoundsHandlerTest, ScreenToWorldDoubleIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0)
    handler.SetViewTransform(&viewTransform);
    CPoint screenPt(100, 200);
    
    // Act
    CPoint2d worldPt = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: With identity transform, screen == world
    EXPECT_DOUBLE_EQ(100.0, worldPt.x);
    EXPECT_DOUBLE_EQ(200.0, worldPt.y);
}

TEST_F(BoundsHandlerTest, WorldToScreenIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0)
    handler.SetViewTransform(&viewTransform);
    CPoint2d worldPt = {150.0, 250.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(worldPt);
    
    // Assert: With identity transform, world == screen
    EXPECT_EQ(150, screenPt.x);
    EXPECT_EQ(250, screenPt.y);
}

TEST_F(BoundsHandlerTest, ScreenToWorldWithOffset) {
    // Arrange: ViewTransform with offset (pan)
    CPoint2d offset = {50.0, 75.0};
    viewTransform.SetOffset(offset);
    handler.SetViewTransform(&viewTransform);
    
    CPoint screenPt(0, 0);
    
    // Act
    CPoint2d worldPt = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: screen(0,0) with offset(50,75) -> world(-50,-75)
    // Formula: world = (screen - offset) / scale
    // world.x = (0 - 50) / 1.0 = -50
    EXPECT_DOUBLE_EQ(-50.0, worldPt.x);
    EXPECT_DOUBLE_EQ(-75.0, worldPt.y);
}

TEST_F(BoundsHandlerTest, WorldToScreenWithOffset) {
    // Arrange: ViewTransform with offset
    CPoint2d offset = {50.0, 75.0};
    viewTransform.SetOffset(offset);
    handler.SetViewTransform(&viewTransform);
    
    CPoint2d worldPt = {100.0, 100.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(worldPt);
    
    // Assert: world(100,100) with offset(50,75) -> screen(150,175)
    // Formula: screen = world * scale + offset
    // screen.x = 100 * 1.0 + 50 = 150
    EXPECT_EQ(150, screenPt.x);
    EXPECT_EQ(175, screenPt.y);
}

TEST_F(BoundsHandlerTest, ScreenToWorldWithZoom) {
    // Arrange: ViewTransform with zoom (scale > 1)
    CRect clientRect(0, 0, 800, 600);
    CPoint center(400, 300);
    viewTransform.ZoomAt(center, 2.0);  // Zoom in 2x at center
    handler.SetViewTransform(&viewTransform);
    
    // Test a point at center (should remain at same position in world coords)
    // After zoom, center point in screen space should map to same world point
    CPoint2d worldBeforeZoom = {400.0, 300.0};
    
    // Act: Convert center point from screen to world after zoom
    CPoint screenCenter(400, 300);
    CPoint2d worldAfterZoom = handler.ScreenToWorldDouble(screenCenter);
    
    // Assert: Center point should stay approximately at same world position
    // (within tolerance due to zoom calculation)
    EXPECT_NEAR(worldBeforeZoom.x, worldAfterZoom.x, 5.0);
    EXPECT_NEAR(worldBeforeZoom.y, worldAfterZoom.y, 5.0);
}

TEST_F(BoundsHandlerTest, RoundTripConversion) {
    // Arrange: ViewTransform with offset
    CPoint2d offset = {100.0, 150.0};
    viewTransform.SetOffset(offset);
    handler.SetViewTransform(&viewTransform);
    
    CPoint2d originalWorld = {200.0, 250.0};
    
    // Act: Convert world -> screen -> world
    CPoint screen = handler.WorldToScreen(originalWorld);
    CPoint2d finalWorld = handler.ScreenToWorldDouble(screen);
    
    // Assert: Round-trip should give back original (within rounding tolerance)
    EXPECT_NEAR(originalWorld.x, finalWorld.x, 1.0);
    EXPECT_NEAR(originalWorld.y, finalWorld.y, 1.0);
}

TEST_F(BoundsHandlerTest, MultiplePointsWithSameTransform) {
    // Arrange: Transform with offset
    CPoint2d offset = {20.0, 30.0};
    viewTransform.SetOffset(offset);
    handler.SetViewTransform(&viewTransform);
    
    std::vector<CPoint2d> worldPoints = {
        {0.0, 0.0},
        {100.0, 100.0},
        {-50.0, -50.0},
        {500.0, 400.0}
    };
    
    // Act & Assert: Each point converts consistently
    for (const auto& worldPt : worldPoints) {
        CPoint screenPt = handler.WorldToScreen(worldPt);
        CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
        
        EXPECT_NEAR(worldPt.x, backToWorld.x, 1.0);
        EXPECT_NEAR(worldPt.y, backToWorld.y, 1.0);
    }
}

// ===== Boundary Tests =====

TEST_F(BoundsHandlerTest, LargeCoordinateValues) {
    // Arrange: Test with large image coordinates
    handler.SetViewTransform(&viewTransform);
    
    CPoint2d largeWorldPt = {10000.0, 8000.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(largeWorldPt);
    CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: Should handle large values correctly
    EXPECT_NEAR(largeWorldPt.x, backToWorld.x, 10.0);  // Larger tolerance for large coords
    EXPECT_NEAR(largeWorldPt.y, backToWorld.y, 10.0);
}

TEST_F(BoundsHandlerTest, NegativeCoordinates) {
    // Arrange: Test with negative coordinates
    handler.SetViewTransform(&viewTransform);
    
    CPoint2d negativeWorld = {-100.0, -200.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(negativeWorld);
    CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: Should handle negative values correctly
    EXPECT_NEAR(negativeWorld.x, backToWorld.x, 1.0);
    EXPECT_NEAR(negativeWorld.y, backToWorld.y, 1.0);
}

// ===== Edge Cases =====

TEST_F(BoundsHandlerTest, ZeroCoordinates) {
    // Arrange
    handler.SetViewTransform(&viewTransform);
    CPoint2d zeroWorld = {0.0, 0.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(zeroWorld);
    CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
    
    // Assert
    EXPECT_DOUBLE_EQ(0.0, backToWorld.x);
    EXPECT_DOUBLE_EQ(0.0, backToWorld.y);
}

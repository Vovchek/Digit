#include "stdafx.h"
#include "gtest/gtest.h"
#include "AppDef.h"  // For BOUND_RECT, BOUND_ROUND, etc.
#include "InterfSolver/Tools/XYRect.h"  // For XYRect
#include "InterfSolver/Tools/XYEllipse.h"  // For XYEllipse
#include "InterfSolver/Tools/XYPolygon.h"  // For XYPolygon
#include "InterfSolver/Tools/XYBounds.h"  // For XYBounds

// ===== Mock Classes for Testing =====
// Define these BEFORE including BoundsHandler.h which forward-declares them

// Mock CBoundCtrls - minimal implementation for testing BoundsHandler

#include "Tests/MockControls.h"

// NOW include BoundsHandler.h after mock definitions
#include "DigitMode/BoundsHandler.h"
#include "ImageTempl/ViewTransform.h"

using namespace DigitMode;

/**
 * @brief Test fixture for BoundsHandler
 * 
 * Phase 1 tests focus on:
 * - Initialization
 * - ViewTransform coordinate conversions
 * - Hit-testing for bounds handles
 */
class BoundsHandlerTest : public ::testing::Test {
protected:
    BoundsHandler handler;
    ViewTransform viewTransform;
    CBoundCtrls boundCtrls;
    CImageCtrls imageCtrls;

    void SetUp() override {
        // Default ViewTransform state (scale=1.0, offset=0)
        // Initialize bounds and image controls
        boundCtrls.Init();
        imageCtrls.ImageSize = CSize(800, 600);  // Mock image size
        
        // Initialize handler with bounds and view data (required for all tests)
        ASSERT_VALID(&boundCtrls.ArrRect);
        void* addrBefore = &boundCtrls;
        TRACE("Before call: boundCtrls at %p\n", addrBefore);

        handler.SetBoundsData(&boundCtrls, &imageCtrls);
        handler.SetViewTransform(&viewTransform);
    }

    void TearDown() override {
        // No cleanup needed for this phase
    }
    
    // Helper: Set up a rectangular external bound for testing
    void SetupExternalRectBound(const CRect& bound) {
        boundCtrls.ExtBoundType = BOUND_RECT;
        
        // Create XYBounds from CRect
        XYBounds bnd;
        bnd.XLeft = bound.left;
        bnd.YTop = bound.top;
        bnd.XRight = bound.right;
        bnd.YBottom = bound.bottom;
        
        // Create XYRect from bounds (external, measuring coordinates)
        XYRect rect(bnd, EXTERNAL, MEASURING);
        
        ASSERT_VALID(&boundCtrls.ArrRect);
        boundCtrls.ArrRect.RemoveAll();
        boundCtrls.ArrRect.Add(rect);
    }
};

/**
 * @brief Test fixture for BoundsHandler with dynamic image size
 * 
 * Phase 2 tests focus on:
 * - Initialization with dynamic image size
 * - ViewTransform coordinate conversions
 * - Hit-testing for bounds handles
 */
class BoundsHandlerDynamicImageTest : public BoundsHandlerTest {
protected:
    void SetUp() override {
        // Call base class SetUp
        BoundsHandlerTest::SetUp();
        
        // Set a different image size for dynamic testing
        imageCtrls.ImageSize = CSize(1024, 768);  // Mock HD image size
    }
};

// ===== Initialization Tests =====

TEST_F(BoundsHandlerTest, DefaultConstruction) {
    // Create a fresh handler without using SetUp (which initializes it)
    BoundsHandler freshHandler;
    EXPECT_FALSE(freshHandler.IsInitialized());
}

TEST_F(BoundsHandlerTest, InitializationWithViewTransform) {
    // Note: SetUp() already calls SetViewTransform(), so IsInitialized() should return true
    EXPECT_TRUE(handler.IsInitialized());
}

// ===== Coordinate Transformation Tests =====

TEST_F(BoundsHandlerTest, ScreenToWorldIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0) (set in SetUp)
    CPoint screenPt(100, 200);
    
    // Act
    CPoint worldPt = handler.ScreenToWorld(screenPt);
    
    // Assert: With identity transform, screen == world
    EXPECT_EQ(100, worldPt.x);
    EXPECT_EQ(200, worldPt.y);
}

TEST_F(BoundsHandlerTest, ScreenToWorldDoubleIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0) (set in SetUp)
    CPoint screenPt(100, 200);
    
    // Act
    CPoint2d worldPt = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: With identity transform, screen == world
    EXPECT_DOUBLE_EQ(100.0, worldPt.x);
    EXPECT_DOUBLE_EQ(200.0, worldPt.y);
}

TEST_F(BoundsHandlerTest, WorldToScreenIdentityTransform) {
    // Arrange: ViewTransform with scale=1.0, offset=(0,0) (set in SetUp)
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
    // Note: handler already has viewTransform from SetUp, so changes apply
    
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
    // Note: handler already has viewTransform from SetUp, so changes apply
    
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
    // Note: handler already has viewTransform from SetUp, so changes apply
    
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
    // Note: handler already has viewTransform from SetUp, so changes apply
    
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
    // Note: handler already has viewTransform from SetUp, so changes apply
    
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
    // Arrange: Test with large image coordinates (ViewTransform already set in SetUp)
    CPoint2d largeWorldPt = {10000.0, 8000.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(largeWorldPt);
    CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
    
    // Assert: Should handle large values correctly
    EXPECT_NEAR(largeWorldPt.x, backToWorld.x, 10.0);  // Larger tolerance for large coords
    EXPECT_NEAR(largeWorldPt.y, backToWorld.y, 10.0);
}

TEST_F(BoundsHandlerTest, NegativeCoordinates) {
    // Arrange: Test with negative coordinates (ViewTransform already set in SetUp)
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
    // Arrange (ViewTransform already set in SetUp)
    CPoint2d zeroWorld = {0.0, 0.0};
    
    // Act
    CPoint screenPt = handler.WorldToScreen(zeroWorld);
    CPoint2d backToWorld = handler.ScreenToWorldDouble(screenPt);
    
    // Assert
    EXPECT_DOUBLE_EQ(0.0, backToWorld.x);
    EXPECT_DOUBLE_EQ(0.0, backToWorld.y);
}

// ===== Hit-Testing Helper Tests =====

TEST_F(BoundsHandlerTest, GetHandleWorldPosTopLeft) {
    // Arrange (handler already initialized in SetUp)
    CRect bound(100, 200, 300, 400);
    SetupExternalRectBound(bound);
    
    // Act - Using private method through public interface (we'll use a test-only accessor later if needed)
    // For now, we'll test this indirectly through HitTestBoundHandle
    // Direct test would require friend class or test accessor
    
    // This test validates the concept via manual calculation
    CPoint2d expectedTL = {100.0, 200.0};
    CPoint2d expectedTR = {300.0, 200.0};
    CPoint2d expectedBR = {300.0, 400.0};
    CPoint2d expectedBL = {100.0, 400.0};
    
    // Convert to screen for verification
    CPoint screenTL = handler.WorldToScreen(expectedTL);
    CPoint screenTR = handler.WorldToScreen(expectedTR);
    CPoint screenBR = handler.WorldToScreen(expectedBR);
    CPoint screenBL = handler.WorldToScreen(expectedBL);
    
    // Assert - Verify screen positions are correct
    EXPECT_EQ(100, screenTL.x);
    EXPECT_EQ(200, screenTL.y);
    EXPECT_EQ(300, screenTR.x);
    EXPECT_EQ(200, screenTR.y);
    EXPECT_EQ(300, screenBR.x);
    EXPECT_EQ(400, screenBR.y);
    EXPECT_EQ(100, screenBL.x);
    EXPECT_EQ(400, screenBL.y);
}

// ===== Hit-Testing Integration Tests =====

TEST_F(BoundsHandlerTest, HitTestBoundHandleNoBoundsSet) {
    // Arrange (handler already initialized in SetUp)
    // No bounds set (ExtBoundType == -1 by default after Init)
    CPoint screenPt(100, 100);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - No bounds, so no hit
    EXPECT_FALSE(hit);
    EXPECT_EQ(-1, boundIdx);
    EXPECT_EQ(-1, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleDirectBoundCheck) {
    // Test to verify that direct bound calling works
    CRect bound;
    CArray<CPoint, CPoint> plgPoints;
    
    // First, set up a bound
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Now try to get it back via GetExtRealBound
    BOOL result = boundCtrls.GetExtRealBound(BOUND_RECT, 800, 600, bound, plgPoints);
    EXPECT_TRUE(result);
    EXPECT_EQ(100, bound.left);
    EXPECT_EQ(200, bound.top);
    EXPECT_EQ(300, bound.right);
    EXPECT_EQ(400, bound.bottom);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleVerifyWorldToScreen) {
    // Test that WorldToScreen works correctly with identity transform
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    CRect bound;
    CArray<CPoint, CPoint> plgPoints;
    boundCtrls.GetExtRealBound(BOUND_RECT, 800, 600, bound, plgPoints);
    
    // With identity transform, TL corner (100, 200) should map to screen (100, 200)
    CPoint2d tlWorld = {(double)bound.left, (double)bound.top};
    CPoint tlScreen = handler.WorldToScreen(tlWorld);
    
    EXPECT_EQ(100, tlScreen.x);
    EXPECT_EQ(200, tlScreen.y);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleTopLeftCorner) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // DEBUG: Verify the bound was set up correctly
    EXPECT_EQ(BOUND_RECT, boundCtrls.ExtBoundType);
    EXPECT_EQ(1, boundCtrls.ArrRect.GetSize());
    
    // Click exactly on top-left corner (100, 200)
    CPoint screenPt(100, 200);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should hit handle 0 (TL)
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);  // External bound
    EXPECT_EQ(0, handleIdx);  // TL corner
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleTopRightCorner) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click on top-right corner (300, 200)
    CPoint screenPt(300, 200);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should hit handle 1 (TR)
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(1, handleIdx);  // TR corner
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleBottomRightCorner) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click on bottom-right corner (300, 400)
    CPoint screenPt(300, 400);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should hit handle 2 (BR)
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(2, handleIdx);  // BR corner
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleBottomLeftCorner) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click on bottom-left corner (100, 400)
    CPoint screenPt(100, 400);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should hit handle 3 (BL)
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(3, handleIdx);  // BL corner
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleWithinTolerance) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click 3 pixels away from top-left corner (within 5px tolerance)
    CPoint screenPt(103, 203);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should hit handle 0 (TL) due to tolerance
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(0, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleOutsideTolerance) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click 10 pixels away from top-left corner (outside 5px tolerance)
    CPoint screenPt(110, 210);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should NOT hit any handle
    EXPECT_FALSE(hit);
    EXPECT_EQ(-1, boundIdx);
    EXPECT_EQ(-1, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleMissBetweenHandles) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Click in the middle of the top edge (200, 200) - between TL and TR
    CPoint screenPt(200, 200);
    int boundIdx, handleIdx;
    
    // Act
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // Assert - Should NOT hit any handle (not close to corners)
    EXPECT_FALSE(hit);
    EXPECT_EQ(-1, boundIdx);
    EXPECT_EQ(-1, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleWithZoom) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Zoom 2x at center
    viewTransform.ZoomAt(CPoint(400, 300), 2.0);
    // Note: handler already has viewTransform from SetUp, so changes apply
    
    // Calculate where TL corner (100, 200) appears on screen after zoom
    CPoint2d tlWorld = {100.0, 200.0};
    CPoint tlScreen = handler.WorldToScreen(tlWorld);
    
    int boundIdx, handleIdx;
    
    // Act - Click on the transformed position
    bool hit = handler.HitTestBoundHandle(tlScreen, boundIdx, handleIdx);
    
    // Assert - Should hit TL corner at its screen position
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(0, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleWithPan) {
    // Arrange (handler already initialized in SetUp)
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Pan offset
    CPoint2d offset = {50.0, 75.0};
    viewTransform.SetOffset(offset);
    // Note: handler already has viewTransform from SetUp, so changes apply
    
    // Calculate where TL corner (100, 200) appears on screen after pan
    CPoint2d tlWorld = {100.0, 200.0};
    CPoint tlScreen = handler.WorldToScreen(tlWorld);
    
    // Expected: screen = world + offset = (100+50, 200+75) = (150, 275)
    EXPECT_EQ(150, tlScreen.x);
    EXPECT_EQ(275, tlScreen.y);
    
    int boundIdx, handleIdx;
    
    // Act - Click on the transformed position
    bool hit = handler.HitTestBoundHandle(tlScreen, boundIdx, handleIdx);
    
    // Assert - Should hit TL corner at its screen position
    EXPECT_TRUE(hit);
    EXPECT_EQ(0, boundIdx);
    EXPECT_EQ(0, handleIdx);
}

TEST_F(BoundsHandlerTest, HitTestBoundHandleSimpleDebugTest) {
    // Setup a bound
    SetupExternalRectBound(CRect(100, 200, 300, 400));
    
    // Verify it was set up
    int ext_type = boundCtrls.ExtBoundType;
    int arr_size = boundCtrls.ArrRect.GetSize();
    ASSERT_EQ(2, ext_type);  // BOUND_RECT = 2
    ASSERT_EQ(1, arr_size);
    
    // Call HitTestBoundHandle on the TL corner
    CPoint screenPt(100, 200);
    int boundIdx = -99, handleIdx = -99;
    bool hit = handler.HitTestBoundHandle(screenPt, boundIdx, handleIdx);
    
    // This should be true but we'll see
    //ASSERT_TRUE(hit) << "HitTestBoundHandle returned false when it should have returned true";
    //ASSERT_EQ(0, boundIdx) << "boundIdx=" << boundIdx << " but expected 0";
    //ASSERT_EQ(0, handleIdx) << "handleIdx=" << handleIdx << " but expected 0";
}

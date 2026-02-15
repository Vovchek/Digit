/**
 * @file ShapeDrawStyleTest.cpp
 * @brief Unit tests for ShapeDrawStyle (Phase 3)
 * 
 * Tests visual appearance computation methods for different states and types.
 */

#include "stdafx.h"
#include <gtest/gtest.h>
#include "DigitMode/Rendering/ShapeDrawStyle.h"
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Outline Color Tests (UX Spec §3)
// ============================================================================

TEST(ShapeDrawStyleTest, OutlineColor_External_IsGreen) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    COLORREF color = style.GetOutlineColor();
    
    // Green: RGB(0, 255, 0)
    EXPECT_EQ(GetRValue(color), 0);
    EXPECT_EQ(GetGValue(color), 255);
    EXPECT_EQ(GetBValue(color), 0);
}

TEST(ShapeDrawStyleTest, OutlineColor_Aperture_IsCyan) {
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    
    COLORREF color = style.GetOutlineColor();
    
    // Cyan: RGB(0, 255, 255)
    EXPECT_EQ(GetRValue(color), 0);
    EXPECT_EQ(GetGValue(color), 255);
    EXPECT_EQ(GetBValue(color), 255);
}

TEST(ShapeDrawStyleTest, OutlineColor_Internal_IsRed) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    
    COLORREF color = style.GetOutlineColor();
    
    // Red: RGB(255, 0, 0)
    EXPECT_EQ(GetRValue(color), 255);
    EXPECT_EQ(GetGValue(color), 0);
    EXPECT_EQ(GetBValue(color), 0);
}

TEST(ShapeDrawStyleTest, OutlineColor_StateDoesNotAffect) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    // Color should be same regardless of state
    style.state = ShapeDrawStyle::State::Idle;
    COLORREF idleColor = style.GetOutlineColor();
    
    style.state = ShapeDrawStyle::State::Selected;
    COLORREF selectedColor = style.GetOutlineColor();
    
    EXPECT_EQ(idleColor, selectedColor);
}

// ============================================================================
// Outline Width Tests (State-based scaling)
// ============================================================================

TEST(ShapeDrawStyleTest, OutlineWidth_Idle_IsNarrow) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Idle;
    
    int width = style.GetOutlineWidth();
    
    EXPECT_EQ(width, 1);  // Narrow line for idle state
}

TEST(ShapeDrawStyleTest, OutlineWidth_Hovered_IsWider) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Hovered;
    
    int width = style.GetOutlineWidth();
    
    EXPECT_GT(width, 1);  // Wider than idle
    EXPECT_LE(width, 3);  // But not too wide
}

TEST(ShapeDrawStyleTest, OutlineWidth_Selected_IsWider) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Selected;
    
    int width = style.GetOutlineWidth();
    
    EXPECT_GT(width, 1);  // Wider than idle
}

TEST(ShapeDrawStyleTest, OutlineWidth_Dragging_IsWidest) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Dragging;
    
    int width = style.GetOutlineWidth();
    
    EXPECT_GE(width, 2);  // Wide line during drag
}

TEST(ShapeDrawStyleTest, OutlineWidth_Draft_IsMedium) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Draft;
    
    int width = style.GetOutlineWidth();
    
    EXPECT_GE(width, 1);
    EXPECT_LE(width, 2);
}

// ============================================================================
// Outline Style Tests (Solid vs Dashed)
// ============================================================================

TEST(ShapeDrawStyleTest, OutlineStyle_External_IsSolid) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    int lineStyle = style.GetOutlineStyle();
    
    EXPECT_EQ(lineStyle, PS_SOLID);
}

TEST(ShapeDrawStyleTest, OutlineStyle_Aperture_IsSolid) {
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    style.state = ShapeDrawStyle::State::Idle;
    
    int lineStyle = style.GetOutlineStyle();
    
    EXPECT_EQ(lineStyle, PS_SOLID);
}

TEST(ShapeDrawStyleTest, OutlineStyle_Internal_IsDashed) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    int lineStyle = style.GetOutlineStyle();
    
    EXPECT_EQ(lineStyle, PS_DASH);
}

TEST(ShapeDrawStyleTest, OutlineStyle_Draft_IsDashed) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Draft;
    
    int lineStyle = style.GetOutlineStyle();
    
    EXPECT_EQ(lineStyle, PS_DASH);
}

TEST(ShapeDrawStyleTest, OutlineStyle_InternalDraft_IsDashed) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Draft;
    
    int lineStyle = style.GetOutlineStyle();
    
    // Both INTERNAL and Draft use dashed
    EXPECT_EQ(lineStyle, PS_DASH);
}

// ============================================================================
// Fill Tests (INTERNAL shapes only)
// ============================================================================

TEST(ShapeDrawStyleTest, HasFill_External_ReturnsFalse) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    EXPECT_FALSE(style.HasFill());
}

TEST(ShapeDrawStyleTest, HasFill_Aperture_ReturnsFalse) {
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    
    EXPECT_FALSE(style.HasFill());
}

TEST(ShapeDrawStyleTest, HasFill_Internal_ReturnsTrue) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    
    EXPECT_TRUE(style.HasFill());
}

TEST(ShapeDrawStyleTest, FillColor_Internal_IsSemiTransparentRed) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    
    COLORREF fillColor = style.GetFillColor();
    
    // Semi-transparent red (actual color may vary, but should be reddish)
    EXPECT_GT(GetRValue(fillColor), 0);
    // Note: GDI doesn't support true alpha, so this is just a red tint
}

TEST(ShapeDrawStyleTest, FillColor_WhenNoFill_CanStillCall) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    // Should not crash even if HasFill() is false
    COLORREF fillColor = style.GetFillColor();
    
    // Color doesn't matter since HasFill() is false
    SUCCEED();
}

// ============================================================================
// Handle Color Tests
// ============================================================================

TEST(ShapeDrawStyleTest, HandleColor_Inactive_IsWhite) {
    ShapeDrawStyle style;
    style.activeHandleIndex = 2;  // Active is index 2
    
    COLORREF color = style.GetHandleColor(0);  // Ask for index 0
    
    // White: RGB(255, 255, 255)
    EXPECT_EQ(GetRValue(color), 255);
    EXPECT_EQ(GetGValue(color), 255);
    EXPECT_EQ(GetBValue(color), 255);
}

TEST(ShapeDrawStyleTest, HandleColor_Active_IsYellow) {
    ShapeDrawStyle style;
    style.activeHandleIndex = 2;
    
    COLORREF color = style.GetHandleColor(2);  // Ask for active handle
    
    // Yellow: RGB(255, 255, 0)
    EXPECT_EQ(GetRValue(color), 255);
    EXPECT_EQ(GetGValue(color), 255);
    EXPECT_EQ(GetBValue(color), 0);
}

TEST(ShapeDrawStyleTest, HandleColor_NoActiveHandle_AllWhite) {
    ShapeDrawStyle style;
    style.activeHandleIndex = -1;  // No active handle
    
    COLORREF color0 = style.GetHandleColor(0);
    COLORREF color1 = style.GetHandleColor(1);
    COLORREF color2 = style.GetHandleColor(2);
    
    // All should be white
    EXPECT_EQ(color0, RGB(255, 255, 255));
    EXPECT_EQ(color1, RGB(255, 255, 255));
    EXPECT_EQ(color2, RGB(255, 255, 255));
}

TEST(ShapeDrawStyleTest, HandleColor_FirstHandle_CanBeActive) {
    ShapeDrawStyle style;
    style.activeHandleIndex = 0;
    
    COLORREF color = style.GetHandleColor(0);
    
    EXPECT_EQ(color, RGB(255, 255, 0));  // Yellow
}

// ============================================================================
// Handle Size Tests
// ============================================================================

TEST(ShapeDrawStyleTest, HandleSize_Idle_IsSmall) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Idle;
    
    int size = style.GetHandleSize();
    
    EXPECT_GE(size, 4);
    EXPECT_LE(size, 6);
}

TEST(ShapeDrawStyleTest, HandleSize_Hovered_IsLarger) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Hovered;
    
    int size = style.GetHandleSize();
    
    EXPECT_GE(size, 5);  // Slightly larger when hovered
}

TEST(ShapeDrawStyleTest, HandleSize_IsConsistent) {
    ShapeDrawStyle style;
    style.state = ShapeDrawStyle::State::Selected;
    
    int size1 = style.GetHandleSize();
    int size2 = style.GetHandleSize();
    
    EXPECT_EQ(size1, size2);  // Consistent across calls
}

// ============================================================================
// Combined State Tests
// ============================================================================

TEST(ShapeDrawStyleTest, ExternalIdleStyle_FullConfiguration) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    style.showHandles = false;
    
    // Verify complete appearance
    EXPECT_EQ(style.GetOutlineColor(), RGB(0, 255, 0));  // Green
    EXPECT_EQ(style.GetOutlineWidth(), 1);
    EXPECT_EQ(style.GetOutlineStyle(), PS_SOLID);
    EXPECT_FALSE(style.HasFill());
    EXPECT_FALSE(style.showHandles);
}

TEST(ShapeDrawStyleTest, InternalSelectedStyle_FullConfiguration) {
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Selected;
    style.showHandles = true;
    style.activeHandleIndex = 1;
    
    // Verify complete appearance
    EXPECT_EQ(style.GetOutlineColor(), RGB(255, 0, 0));  // Red
    EXPECT_GT(style.GetOutlineWidth(), 1);  // Wider when selected
    EXPECT_EQ(style.GetOutlineStyle(), PS_DASH);  // Dashed for INTERNAL
    EXPECT_TRUE(style.HasFill());
    EXPECT_TRUE(style.showHandles);
    
    // Verify handle colors
    EXPECT_EQ(style.GetHandleColor(0), RGB(255, 255, 255));  // White
    EXPECT_EQ(style.GetHandleColor(1), RGB(255, 255, 0));    // Yellow (active)
    EXPECT_EQ(style.GetHandleColor(2), RGB(255, 255, 255));  // White
}

TEST(ShapeDrawStyleTest, DraftStyle_LooksLikeDraft) {
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Draft;
    
    // Draft shapes should be visually distinct
    EXPECT_EQ(style.GetOutlineStyle(), PS_DASH);  // Dashed line
    EXPECT_LE(style.GetOutlineWidth(), 2);  // Thinner line
}

// ============================================================================
// Default Constructor Tests
// ============================================================================

TEST(ShapeDrawStyleTest, DefaultConstructor_SaneDefaults) {
    ShapeDrawStyle style;
    
    // Verify defaults make sense
    EXPECT_EQ(style.state, ShapeDrawStyle::State::Idle);
    EXPECT_EQ(style.type, TypeLimits::EXTERNAL);
    EXPECT_FALSE(style.showHandles);
    EXPECT_EQ(style.activeHandleIndex, -1);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ShapeDrawStyleTest, HandleColor_NegativeIndex_DoesNotCrash) {
    ShapeDrawStyle style;
    style.activeHandleIndex = -1;
    
    COLORREF color = style.GetHandleColor(-1);
    
    // Should return some color (white since no active handle)
    SUCCEED();
}

TEST(ShapeDrawStyleTest, HandleColor_LargeIndex_DoesNotCrash) {
    ShapeDrawStyle style;
    style.activeHandleIndex = 0;
    
    COLORREF color = style.GetHandleColor(999);
    
    // Should return white (not active)
    EXPECT_EQ(color, RGB(255, 255, 255));
}

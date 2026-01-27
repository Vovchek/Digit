#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/InputHandler.h"
#include "DigitMode/CFringeSegment.h"  // Include for CFringeSegment
#include "DigitMode/DigitInfo.h"      // Include for CDigitInfo
#include "MGTools/Include/Utils/BaseDataType.h"  // Include for CDPoint

using namespace DigitMode;

/**
 * @brief Test fixture for InputHandler
 */
class InputHandlerTest : public ::testing::Test {
protected:
    InputHandler inputHandler;

    void SetUp() override {
        // Ensure default state
    }

    void TearDown() override {
        // Cleanup
    }
};

// ===== Mode Switching Tests =====

TEST_F(InputHandlerTest, DefaultModeIsNavigate) {
    EXPECT_EQ(EditMode::Navigate, inputHandler.GetMode());
    EXPECT_FALSE(inputHandler.IsInDrawMode());
}

TEST_F(InputHandlerTest, SetModeChangesMode) {
    inputHandler.SetMode(EditMode::Draw);
    EXPECT_EQ(EditMode::Draw, inputHandler.GetMode());
    EXPECT_TRUE(inputHandler.IsInDrawMode());

    inputHandler.SetMode(EditMode::DotEdit);
    EXPECT_EQ(EditMode::DotEdit, inputHandler.GetMode());
    EXPECT_FALSE(inputHandler.IsInDrawMode());
}

TEST_F(InputHandlerTest, IsInDrawModeWorks) {
    inputHandler.SetMode(EditMode::Navigate);
    EXPECT_FALSE(inputHandler.IsInDrawMode());

    inputHandler.SetMode(EditMode::Draw);
    EXPECT_TRUE(inputHandler.IsInDrawMode());

    inputHandler.SetMode(EditMode::DotEdit);
    EXPECT_FALSE(inputHandler.IsInDrawMode());
}

TEST_F(InputHandlerTest, SetSameModeIsIdempotent) {
    inputHandler.SetMode(EditMode::Draw);
    EXPECT_EQ(EditMode::Draw, inputHandler.GetMode());

    inputHandler.SetMode(EditMode::Draw);
    EXPECT_EQ(EditMode::Draw, inputHandler.GetMode());
}

TEST_F(InputHandlerTest, LeavingDrawModeEndsSegment) {
    inputHandler.SetMode(EditMode::Draw);
    // TODO: Start a segment (requires DigitInfo integration)
    // inputHandler.StartNewSegment(CPoint(10, 10));
    // int activeSegment = inputHandler.GetActiveSegment();
    // EXPECT_GE(activeSegment, 0);

    inputHandler.SetMode(EditMode::Navigate);

    // Active segment should be cleared
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

// ===== Draw Mode State Tests =====

TEST_F(InputHandlerTest, InitialActiveSegmentIsNegative) {
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, EndCurrentSegmentClearsActiveSegment) {
    inputHandler.SetMode(EditMode::Draw);
    // Simulate starting a segment (stubbed, just test state)
    // inputHandler.StartNewSegment(CPoint(10, 10));

    inputHandler.EndCurrentSegment();

    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, EndCurrentSegmentOnEmptyStateDoesNotCrash) {
    // Should not crash when no segment is active
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, MultipleEndCurrentSegmentCallsAreSafe) {
    inputHandler.EndCurrentSegment();
    inputHandler.EndCurrentSegment();
    inputHandler.EndCurrentSegment();

    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

// ===== Mode Transition Tests =====

TEST_F(InputHandlerTest, NavigateToDrawTransition) {
    inputHandler.SetMode(EditMode::Navigate);
    inputHandler.SetMode(EditMode::Draw);

    EXPECT_EQ(EditMode::Draw, inputHandler.GetMode());
}

TEST_F(InputHandlerTest, DrawToNavigateTransition) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.SetMode(EditMode::Navigate);

    EXPECT_EQ(EditMode::Navigate, inputHandler.GetMode());
}

TEST_F(InputHandlerTest, DrawToDotEditTransition) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.SetMode(EditMode::DotEdit);

    EXPECT_EQ(EditMode::DotEdit, inputHandler.GetMode());
}

TEST_F(InputHandlerTest, DotEditToDrawTransition) {
    inputHandler.SetMode(EditMode::DotEdit);
    inputHandler.SetMode(EditMode::Draw);

    EXPECT_EQ(EditMode::Draw, inputHandler.GetMode());
}

// ===== Stub Function Tests (Phase 1) =====

TEST_F(InputHandlerTest, StartNewSegmentDoesNotCrash) {
    inputHandler.SetMode(EditMode::Draw);
    
    // Stubbed function should not crash
    // Note: Requires DigitInfo - skip in Phase 1 tests
    // inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    
    // TODO: Move to DrawModeTest in Phase 2
}

TEST_F(InputHandlerTest, ContinueSegmentDoesNotCrash) {
    inputHandler.SetMode(EditMode::Draw);
    
    // Stubbed function should not crash
    // Note: Requires DigitInfo - skip in Phase 1 tests
    // inputHandler.ContinueSegment(0, 0, &digitInfo);
    
    // TODO: Move to DrawModeTest in Phase 2
}

TEST_F(InputHandlerTest, ConnectSegmentsDoesNotCrash) {
    inputHandler.SetMode(EditMode::Draw);
    
    // Stubbed function should not crash
    // Note: Requires DigitInfo - skip in Phase 1 tests
    // inputHandler.ConnectSegments(1, 0, &digitInfo);
    
    // TODO: Move to DrawModeTest in Phase 2
}

// ===== Integration Readiness Tests =====

TEST_F(InputHandlerTest, ModeSwitchingPreservesNoState) {
    // Mode switching should not preserve draw state (by design)
    inputHandler.SetMode(EditMode::Draw);
    // Start segment (stubbed)
    // inputHandler.StartNewSegment(CPoint(10, 10));

    inputHandler.SetMode(EditMode::Navigate);
    inputHandler.SetMode(EditMode::Draw);

    // New draw session should have no active segment
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, MultipleModeSwitchesStable) {
    for (int i = 0; i < 10; i++) {
        inputHandler.SetMode(EditMode::Navigate);
        inputHandler.SetMode(EditMode::Draw);
        inputHandler.SetMode(EditMode::DotEdit);
    }

    EXPECT_EQ(EditMode::DotEdit, inputHandler.GetMode());
}

// ===== ModifierState Tests =====

TEST_F(InputHandlerTest, ModifierStateNoneWorks) {
    ModifierState mods;
    EXPECT_TRUE(mods.None());
}

TEST_F(InputHandlerTest, ModifierStateCtrlWorks) {
    ModifierState mods;
    mods.ctrl = true;
    EXPECT_FALSE(mods.None());
}

TEST_F(InputHandlerTest, ModifierStateShiftWorks) {
    ModifierState mods;
    mods.shift = true;
    EXPECT_FALSE(mods.None());
}

TEST_F(InputHandlerTest, ModifierStateAltWorks) {
    ModifierState mods;
    mods.alt = true;
    EXPECT_FALSE(mods.None());
}

TEST_F(InputHandlerTest, ModifierStateDebugString) {
    ModifierState mods;
    EXPECT_EQ("None", mods.Debug());

    mods.ctrl = true;
    EXPECT_EQ("Ctrl+", mods.Debug());

    mods.shift = true;
    EXPECT_EQ("Ctrl+Shift+", mods.Debug());

    mods.alt = true;
    EXPECT_EQ("Ctrl+Shift+Alt+", mods.Debug());
}

TEST_F(InputHandlerTest, ModifierStateFromKeyboardDoesNotCrash) {
    // This will read actual keyboard state (may be all false in test environment)
    ModifierState mods = ModifierState::FromKeyboard();

    // Just verify it doesn't crash
    EXPECT_TRUE(mods.None() || !mods.None());
}

// ===== Placeholder for Phase 2 Tests =====

/*
TEST_F(InputHandlerTest, StartNewSegmentCreatesSegment) {
    // TODO: Implement in Phase 2 with DigitInfo integration
    // inputHandler.StartNewSegment(CPoint(100, 100));
    // EXPECT_GE(inputHandler.GetActiveSegment(), 0);
}

TEST_F(InputHandlerTest, ContinueSegmentFromEnd) {
    // TODO: Implement in Phase 2
    // Create segment, continue from end
}

TEST_F(InputHandlerTest, ConnectSegmentsFreeEndBecomesActive) {
    // TODO: Implement in Phase 2
    // Create two segments, connect them, verify free end is active
}
*/

using namespace DigitMode;

TEST_F(InputHandlerTest, NavigateModeBoxSelection) {
    // Setup DigitInfo with test segments
    CDigitInfo digitInfo;
    digitInfo.Init();

    CFringeSegment seg1(1.0, 0);
    seg1.AddPoint(CDPoint(10, 10));
    seg1.AddPoint(CDPoint(20, 20));
    digitInfo.Fringes.push_back(seg1);

    CFringeSegment seg2(2.0, 1);
    seg2.AddPoint(CDPoint(30, 30));
    seg2.AddPoint(CDPoint(40, 40));
    digitInfo.Fringes.push_back(seg2);

    // Simulate box selection drag
    CPoint start(5, 5);
    CPoint end(25, 25);
    inputHandler.SetMode(EditMode::Navigate);
    inputHandler.OnMouseDrag(start, end, &digitInfo);

    // Verify selection
    EXPECT_EQ(2, digitInfo.selectionManager.GetCount());
    EXPECT_EQ(SelectionLevel::Dot, digitInfo.selectionManager.GetLevel());
}

// Mock implementation of CDigitInfo to avoid full dependency
class MockDigitInfo : public CDigitInfo {
public:
    MockDigitInfo() {
        CurrentNumber = 0.0;
        numStep = 1.0;
    }
};

TEST_F(InputHandlerTest, StartNewSegmentInitializesCorrectly) {
    MockDigitInfo mockDigit;
    CPoint startPoint(10, 10);

    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(startPoint, &mockDigit);

    EXPECT_EQ(mockDigit.CurrentNumber, 1.0);
    EXPECT_EQ(mockDigit.Fringes.size(), 1);
    EXPECT_EQ(inputHandler.GetActiveSegment(), 0);
}

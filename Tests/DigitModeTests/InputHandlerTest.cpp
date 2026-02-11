#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/InputHandler.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/Commands/AllCommands.h"
#include "DigitMode/CFringeSegment.h"
#include "DigitMode/DigitInfo.h"
#include "MGTools/Include/Utils/BaseDataType.h"
#include "ImageTempl/ViewTransform.h"

using namespace DigitMode;

/**
 * @brief Test fixture for InputHandler
 * 
 * Tests InputHandler functionality including:
 * - Mode switching
 * - Draw mode operations (via CommandDispatcher)
 * - Keyboard shortcuts
 * - Active segment state management
 */
class InputHandlerTest : public ::testing::Test {
protected:
    InputHandler inputHandler;
    CDigitInfo digitInfo;
    CommandDispatcher cmdDispatcher;

    void SetUp() override {
        digitInfo.Init();
        digitInfo.CurrentNumber = 0.0;
        digitInfo.numStep = 1.0;
    }

    void TearDown() override {
        digitInfo.Fringes.clear();
    }

    // Helper: Create a test segment with dots
    int CreateTestSegment(double number, const std::vector<CDPoint>& points) {
        CFringeSegment seg(number, digitInfo.Fringes.size());
        for (const auto& p : points) {
            seg.AddPoint(p);
        }
        digitInfo.Fringes.push_back(seg);
        return digitInfo.Fringes.size() - 1;
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

TEST_F(InputHandlerTest, LeavingDrawModeFinalizesSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));

    inputHandler.SetMode(EditMode::Navigate);
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(InputHandlerTest, MultipleModeSwitchesStable) {
    for (int i = 0; i < 10; i++) {
        inputHandler.SetMode(EditMode::Navigate);
        inputHandler.SetMode(EditMode::Draw);
        inputHandler.SetMode(EditMode::DotEdit);
    }
    EXPECT_EQ(EditMode::DotEdit, inputHandler.GetMode());
}

// ===== Draw Mode State Tests =====

TEST_F(InputHandlerTest, InitialActiveSegmentIsNegative) {
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, EndCurrentSegmentClearsActiveEnd) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();

    inputHandler.EndCurrentSegment();
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(InputHandlerTest, EndCurrentSegmentOnEmptyStateDoesNotCrash) {
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, MultipleEndCurrentSegmentCallsAreSafe) {
    inputHandler.EndCurrentSegment();
    inputHandler.EndCurrentSegment();
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

// ===== OnLButtonDown Tests =====

TEST_F(InputHandlerTest, DrawMode_EmptyClickStartsNewSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.OnLButtonDown(0, CPoint(100, 100), &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(1, digitInfo.Fringes.size());
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    EXPECT_EQ(1, digitInfo.Fringes[0].GetPointCount());
}

TEST_F(InputHandlerTest, DrawMode_EmptyClickAddsToActiveSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    
    inputHandler.OnLButtonDown(0, CPoint(20, 20), &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(2, digitInfo.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(20.0, digitInfo.Fringes[0].GetPoint(1).x);
}

TEST_F(InputHandlerTest, DrawMode_ClickDotContinuesSegment) {
    inputHandler.SetMode(EditMode::Draw);
    int iSeg = CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    
    inputHandler.OnLButtonDown(0, CPoint(20, 20), &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== OnKeyDown Tests =====

TEST_F(InputHandlerTest, BackspaceRemovesLastDot) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, 0, 1, CDPoint(20, 20)));
    
    EXPECT_EQ(2, digitInfo.Fringes[0].GetPointCount());
    
    inputHandler.OnKeyDown(VK_BACK, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(1, digitInfo.Fringes[0].GetPointCount());
    EXPECT_TRUE(cmdDispatcher.CanUndo());
}

TEST_F(InputHandlerTest, EscapeCancelsActiveSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.OnKeyDown(VK_ESCAPE, &digitInfo, &cmdDispatcher);
    
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(InputHandlerTest, EnterFinalizesActiveSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    inputHandler.OnKeyDown(VK_RETURN, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(InputHandlerTest, KeyB_TogglesRubberBand) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    
    EXPECT_FALSE(inputHandler.GetRubberBand(&digitInfo));
    
    inputHandler.OnKeyDown('b', &digitInfo, &cmdDispatcher);
    EXPECT_TRUE(inputHandler.GetRubberBand(&digitInfo));
    
    inputHandler.OnKeyDown('B', &digitInfo, &cmdDispatcher);
    EXPECT_FALSE(inputHandler.GetRubberBand(&digitInfo));
}

TEST_F(InputHandlerTest, KeyDownOutsideDrawModeIgnored) {
    inputHandler.SetMode(EditMode::Navigate);
    
    inputHandler.OnKeyDown(VK_BACK, &digitInfo, &cmdDispatcher);
    inputHandler.OnKeyDown(VK_ESCAPE, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(EditMode::Navigate, inputHandler.GetMode());
}

// ===== ContinueSegment Tests =====

TEST_F(InputHandlerTest, ContinueSegmentFromTail) {
    inputHandler.SetMode(EditMode::Draw);
    int iSeg = CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    
    inputHandler.ContinueSegment(iSeg, 1, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(InputHandlerTest, ContinueSegmentFromHead) {
    inputHandler.SetMode(EditMode::Draw);
    int iSeg = CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    
    inputHandler.ContinueSegment(iSeg, 0, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(InputHandlerTest, ContinueSegmentRejectsMiddleDot) {
    inputHandler.SetMode(EditMode::Draw);
    int iSeg = CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20), CDPoint(30, 30)});
    
    inputHandler.ContinueSegment(iSeg, 1, &digitInfo, &cmdDispatcher);
    
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== ConnectSegments Tests =====

TEST_F(InputHandlerTest, ConnectSegmentsMergesTwoSegments) {
    inputHandler.SetMode(EditMode::Draw);
    CreateTestSegment(1.0, {CDPoint(0, 0), CDPoint(10, 10)});
    CreateTestSegment(2.0, {CDPoint(20, 20), CDPoint(30, 30)});
    
    inputHandler.ContinueSegment(0, 1, &digitInfo, &cmdDispatcher);
    inputHandler.ConnectSegments(1, 0, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(1, digitInfo.Fringes.size());
    EXPECT_TRUE(cmdDispatcher.CanUndo());
}

// ===== Drag & Drop Tests =====

TEST_F(InputHandlerTest, DotDragCommitsMoveDotCommand) {
    inputHandler.SetMode(EditMode::Draw);
    CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    
    inputHandler.OnLButtonDown(0, CPoint(10, 10), &digitInfo, &cmdDispatcher);
    inputHandler.OnLButtonUp(CPoint(15, 15), &digitInfo, &cmdDispatcher);
    
    EXPECT_DOUBLE_EQ(15.0, digitInfo.Fringes[0].GetPoint(0).x);
    EXPECT_TRUE(cmdDispatcher.CanUndo());
}

TEST_F(InputHandlerTest, NavigateMode_BoxSelection) {
    inputHandler.SetMode(EditMode::Navigate);
    CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    CreateTestSegment(2.0, {CDPoint(30, 30), CDPoint(40, 40)});
    
    inputHandler.OnLButtonDown(0, CPoint(5, 5), &digitInfo, &cmdDispatcher);
    inputHandler.OnLButtonUp(CPoint(35, 35), &digitInfo, &cmdDispatcher);
    
    EXPECT_GT(digitInfo.selectionManager.GetCount(), 0u);
}

// ===== IsActiveSegmentValid Tests =====

TEST_F(InputHandlerTest, IsActiveSegmentValid_RequiresActiveEnd) {
    CreateTestSegment(1.0, {CDPoint(10, 10)});
    
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.ContinueSegment(0, 0, &digitInfo);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== GetActiveDot Tests =====

TEST_F(InputHandlerTest, GetActiveDot_NoActiveSegment) {
    EXPECT_EQ(CPoint(-1, -1), inputHandler.GetActiveDot(&digitInfo));
}

TEST_F(InputHandlerTest, GetActiveDot_ActiveTail) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, 0, 1, CDPoint(20, 20)));
    
    CPoint activeDot = inputHandler.GetActiveDot(&digitInfo);
    EXPECT_EQ(20, activeDot.x);
}

TEST_F(InputHandlerTest, GetActiveDot_ActiveHead) {
    inputHandler.SetMode(EditMode::Draw);
    int iSeg = CreateTestSegment(1.0, {CDPoint(10, 10), CDPoint(20, 20)});
    
    inputHandler.ContinueSegment(iSeg, 0, &digitInfo);
    
    CPoint activeDot = inputHandler.GetActiveDot(&digitInfo);
    EXPECT_EQ(10, activeDot.x);
}

// ===== GetRubberBand Tests =====

TEST_F(InputHandlerTest, GetRubberBand_FalseByDefault) {
    EXPECT_FALSE(inputHandler.GetRubberBand(&digitInfo));
}

TEST_F(InputHandlerTest, GetRubberBand_RequiresActiveSegment) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.OnKeyDown('b', &digitInfo, &cmdDispatcher);
    
    EXPECT_FALSE(inputHandler.GetRubberBand(&digitInfo));
    
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    EXPECT_TRUE(inputHandler.GetRubberBand(&digitInfo));
}

TEST_F(InputHandlerTest, GetRubberBand_HidesOutsideDrawMode) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    inputHandler.OnKeyDown('b', &digitInfo, &cmdDispatcher);
    
    EXPECT_TRUE(inputHandler.GetRubberBand(&digitInfo));
    
    inputHandler.SetMode(EditMode::Navigate);
    EXPECT_FALSE(inputHandler.GetRubberBand(&digitInfo));
}

// ===== Pan/Zoom Tests =====

TEST_F(InputHandlerTest, BeginPan_SetsPanningState) {
    inputHandler.BeginPan(CPoint(100, 100));
    EXPECT_TRUE(inputHandler.m_isPanning);
}

TEST_F(InputHandlerTest, EndPan_ClearsPanningState) {
    inputHandler.BeginPan(CPoint(100, 100));
    inputHandler.EndPan();
    EXPECT_FALSE(inputHandler.m_isPanning);
}

TEST_F(InputHandlerTest, ContinuePan_UpdatesTransform) {
    ViewTransform vt;
    inputHandler.BeginPan(CPoint(100, 100));
    inputHandler.ContinuePan(CPoint(110, 110), &vt);
    
    ::CPoint2d offset = vt.GetOffset();  // Explicit namespace
    EXPECT_DOUBLE_EQ(10.0, offset.x);
    EXPECT_DOUBLE_EQ(10.0, offset.y);
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

TEST_F(InputHandlerTest, ModifierStateDebugString) {
    ModifierState mods;
    EXPECT_EQ("None", mods.Debug());

    mods.ctrl = true;
    EXPECT_EQ("Ctrl+", mods.Debug());

    mods.shift = true;
    EXPECT_EQ("Ctrl+Shift+", mods.Debug());
}

// ===== Undo Support Tests =====

TEST_F(InputHandlerTest, AllOperationsAreUndoable) {
    inputHandler.SetMode(EditMode::Draw);
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, 0, 1, CDPoint(20, 20)));
    
    inputHandler.OnKeyDown(VK_BACK, &digitInfo, &cmdDispatcher);
    EXPECT_EQ(1, digitInfo.Fringes[0].GetPointCount());
    
    cmdDispatcher.Undo();
    EXPECT_EQ(2, digitInfo.Fringes[0].GetPointCount());
}

// ===== Complex Workflows =====

TEST_F(InputHandlerTest, DrawContinueConnectWorkflow) {
    inputHandler.SetMode(EditMode::Draw);
    
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, 0, 1, CDPoint(20, 20)));
    inputHandler.EndCurrentSegment();
    
    inputHandler.StartNewSegment(CPoint(30, 30), &digitInfo, &cmdDispatcher);
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, 1, 1, CDPoint(40, 40)));
    
    EXPECT_EQ(2, digitInfo.Fringes.size());
    
    inputHandler.ContinueSegment(0, 1, &digitInfo, &cmdDispatcher);
    inputHandler.ConnectSegments(1, 0, &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(1, digitInfo.Fringes.size());
    
    cmdDispatcher.Undo();
    EXPECT_EQ(2, digitInfo.Fringes.size());
}

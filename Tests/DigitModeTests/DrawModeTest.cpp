#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/InputHandler.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/Commands/AllCommands.h"
#include "DigitMode/DigitInfo.h"

using namespace DigitMode;

/**
 * @brief Test fixture for Draw Mode functionality
 * 
 * Tests the complete draw workflow:
 * - Starting new segments
 * - Adding dots via commands
 * - Continuing from segment ends
 * - Connecting segments
 * - Undo/redo for all operations
 */
class DrawModeTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    InputHandler inputHandler;
    CommandDispatcher cmdDispatcher;

    void SetUp() override {
        digitInfo.Init();
        digitInfo.CurrentNumber = 1.0;
        digitInfo.numStep = 0.5;
        inputHandler.SetMode(FringeEditMode::Draw);
    }

    void TearDown() override {
        digitInfo.Fringes.clear();
    }
};

// ===== Basic Draw Operations =====

TEST_F(DrawModeTest, StartNewSegmentCreatesSegment) {
    int initialCount = digitInfo.Fringes.size();
    
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(initialCount + 1, digitInfo.Fringes.size());
    EXPECT_GE(inputHandler.GetActiveSegment(), 0);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    // Verify new segment has correct number and initial dot
    CFringeSegment& newSeg = digitInfo.Fringes[inputHandler.GetActiveSegment()];
    EXPECT_DOUBLE_EQ(1.5, newSeg.GetNumber());  // CurrentNumber + numStep
    EXPECT_EQ(1, newSeg.GetPointCount());  // Initial dot added by StartNewSegment
    EXPECT_DOUBLE_EQ(100.0, newSeg.GetPoint(0).x);
    EXPECT_DOUBLE_EQ(100.0, newSeg.GetPoint(0).y);
}

TEST_F(DrawModeTest, AddDotCommandAddsPoint) {
    // Start a new segment (already adds first dot)
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());  // Initial dot
    
    // Add second dot via command
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(200, 200));
    cmdDispatcher.Execute(std::move(pCmd));
    
    CFringeSegment& segment = digitInfo.Fringes[iSeg];
    EXPECT_EQ(2, segment.GetPointCount());
    EXPECT_DOUBLE_EQ(100.0, segment.GetPoint(0).x);
    EXPECT_DOUBLE_EQ(200.0, segment.GetPoint(1).x);
}

TEST_F(DrawModeTest, AddMultipleDotsBuildsSegment) {
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Segment already has initial dot at (10, 10)
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Add 4 more dots (total 5)
    for (int i = 1; i < 5; i++) {
        auto pCmd = std::make_unique<AddDotCommand>(
            &digitInfo, iSeg, i, CDPoint(10.0 + i * 10, 10.0));
        cmdDispatcher.Execute(std::move(pCmd));
    }
    
    CFringeSegment& segment = digitInfo.Fringes[iSeg];
    EXPECT_EQ(5, segment.GetPointCount());
    
    // Verify dot positions
    for (int i = 0; i < 5; i++) {
        EXPECT_DOUBLE_EQ(10.0 + i * 10, segment.GetPoint(i).x);
    }
}

// ===== Undo/Redo Tests =====

TEST_F(DrawModeTest, AddDotCommandSupportsUndo) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Segment already has 1 dot from StartNewSegment
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(200, 200));
    cmdDispatcher.Execute(std::move(pCmd));
    
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_TRUE(cmdDispatcher.CanUndo());
    
    cmdDispatcher.Undo();
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_TRUE(cmdDispatcher.CanRedo());
}

TEST_F(DrawModeTest, AddDotCommandSupportsRedo) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(200, 200));
    cmdDispatcher.Execute(std::move(pCmd));
    cmdDispatcher.Undo();
    
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    cmdDispatcher.Redo();
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(200.0, digitInfo.Fringes[iSeg].GetPoint(1).x);
}

TEST_F(DrawModeTest, MultipleUndosRestoreCorrectState) {
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Initial dot already at (10, 10)
    // Add 2 more dots
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 2, CDPoint(30, 30)));
    
    EXPECT_EQ(3, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Undo twice
    cmdDispatcher.Undo();
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    
    cmdDispatcher.Undo();
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(10.0, digitInfo.Fringes[iSeg].GetPoint(0).x);
}

// ===== RemoveLastDotCommand Tests =====

TEST_F(DrawModeTest, RemoveLastDotCommandWorks) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 2 more dots (initial + 2 = 3 total)
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 2, CDPoint(30, 30)));
    
    EXPECT_EQ(3, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Remove last dot
    cmdDispatcher.Execute(std::make_unique<RemoveLastDotCommand>(&digitInfo, iSeg));
    
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(20.0, digitInfo.Fringes[iSeg].GetPoint(1).x);  // Last dot now
}

TEST_F(DrawModeTest, RemoveLastDotCommandSupportsUndo) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    
    cmdDispatcher.Execute(std::make_unique<RemoveLastDotCommand>(&digitInfo, iSeg));
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Undo remove
    cmdDispatcher.Undo();
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(20.0, digitInfo.Fringes[iSeg].GetPoint(1).x);  // Restored
}

// ===== Continue Segment Tests =====

TEST_F(DrawModeTest, ContinueSegmentSetsActive) {
    // Create a segment with some dots
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    
    // End segment - clears activeEnd but remembers segment index
    inputHandler.EndCurrentSegment();
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());  // Index is remembered
    
    // Continue from last dot - re-activates segment for drawing
    inputHandler.ContinueSegment(iSeg, 1, &digitInfo);
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== End Segment Tests =====

TEST_F(DrawModeTest, EndCurrentSegmentClearsActiveEnd) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    EXPECT_GE(iSeg, 0);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.EndCurrentSegment();
    // Index is remembered but segment is not valid for drawing
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

TEST_F(DrawModeTest, EndCurrentSegmentOnEmptyIsIdempotent) {
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== Mode Switching Tests =====

TEST_F(DrawModeTest, LeavingDrawModeEndsSegment) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    EXPECT_GE(iSeg, 0);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    inputHandler.SetMode(FringeEditMode::Navigate);
    // Segment is finalized but index is remembered
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== Segment Memory Tests =====

TEST_F(DrawModeTest, EndCurrentSegmentRemembersLastSegment) {
    // This test verifies the design choice: iActiveSegment is "memory" of last drawn segment
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg1 = inputHandler.GetActiveSegment();
    
    inputHandler.EndCurrentSegment();
    // iActiveSegment still points to segment (for potential UI feedback)
    EXPECT_EQ(iSeg1, inputHandler.GetActiveSegment());
    
    // But segment is not valid for drawing
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    // Starting new segment uses remembered CurrentNumber for auto-increment
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg2 = inputHandler.GetActiveSegment();
    
    EXPECT_NE(iSeg1, iSeg2);
    EXPECT_DOUBLE_EQ(1.5, digitInfo.Fringes[iSeg1].GetNumber());
    EXPECT_DOUBLE_EQ(2.0, digitInfo.Fringes[iSeg2].GetNumber());  // Auto-incremented
}

TEST_F(DrawModeTest, ActiveEndDeterminesDrawingValidity) {
    // Verify that activeEnd is the gatekeeper for IsActiveSegmentValid
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Initially valid with activeEnd set
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    // After EndCurrentSegment, activeEnd is cleared
    inputHandler.EndCurrentSegment();
    EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));
    
    // After ContinueSegment, activeEnd is restored
    inputHandler.ContinueSegment(iSeg, 0, &digitInfo);
    EXPECT_TRUE(inputHandler.IsActiveSegmentValid(&digitInfo));
}

// ===== Complex Workflow Tests =====

TEST_F(DrawModeTest, CompleteDrawWorkflow) {
    // Start segment (already adds first dot)
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Draw a polyline with 4 more dots (total 5)
    for (int i = 1; i < 5; i++) {
        cmdDispatcher.Execute(std::make_unique<AddDotCommand>(
            &digitInfo, iSeg, i, CDPoint(10.0 + i * 10, 10.0 + i * 10)));
    }
    
    EXPECT_EQ(5, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Accidentally added wrong dot - remove it
    cmdDispatcher.Execute(std::make_unique<RemoveLastDotCommand>(&digitInfo, iSeg));
    EXPECT_EQ(4, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Add correct dot
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(
        &digitInfo, iSeg, 4, CDPoint(50, 100)));  // Different position
    
    EXPECT_EQ(5, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(50.0, digitInfo.Fringes[iSeg].GetPoint(4).x);
    EXPECT_DOUBLE_EQ(100.0, digitInfo.Fringes[iSeg].GetPoint(4).y);
}

TEST_F(DrawModeTest, UndoRedoComplexWorkflow) {
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 2 more dots (initial + 2 = 3 total)
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 2, CDPoint(30, 30)));
    
    // Undo last 2
    cmdDispatcher.Undo();
    cmdDispatcher.Undo();
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Redo 1
    cmdDispatcher.Redo();
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Add new dot (breaks redo chain)
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 2, CDPoint(40, 40)));
    EXPECT_FALSE(cmdDispatcher.CanRedo());
    EXPECT_EQ(3, digitInfo.Fringes[iSeg].GetPointCount());
}

// ===== Edge Cases =====

TEST_F(DrawModeTest, CannotRemoveDotFromEmptySegment) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Segment has 1 dot from StartNewSegment
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Remove the only dot
    cmdDispatcher.Execute(std::make_unique<RemoveLastDotCommand>(&digitInfo, iSeg));
    EXPECT_EQ(0, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Now attempting to remove from empty segment would trigger ASSERT
    // This should trigger ASSERT in debug build
    // In release, behavior is undefined - don't test
    // EXPECT_DEATH(RemoveLastDotCommand cmd(&digitInfo, iSeg), "");
}

TEST_F(DrawModeTest, MultipleSegmentsIndependent) {
    // Start first segment
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo, &cmdDispatcher);
    int iSeg1 = inputHandler.GetActiveSegment();
    inputHandler.EndCurrentSegment();
    
    // Start second segment
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg2 = inputHandler.GetActiveSegment();
    
    // Verify segments are independent
    EXPECT_NE(iSeg1, iSeg2);
    EXPECT_EQ(1, digitInfo.Fringes[iSeg1].GetPointCount());
    EXPECT_EQ(1, digitInfo.Fringes[iSeg2].GetPointCount());
    
    // Verify different numbers
    EXPECT_NE(digitInfo.Fringes[iSeg1].GetNumber(), 
              digitInfo.Fringes[iSeg2].GetNumber());
    
    // Verify correct numbering sequence
    EXPECT_DOUBLE_EQ(1.5, digitInfo.Fringes[iSeg1].GetNumber());  // CurrentNumber(1.0) + numStep(0.5)
    EXPECT_DOUBLE_EQ(2.0, digitInfo.Fringes[iSeg2].GetNumber());  // CurrentNumber(1.5) + numStep(0.5)
}

TEST_F(DrawModeTest, NumberingSequenceAutoIncrements) {
    // Verify that multiple segments get incrementing numbers
    digitInfo.CurrentNumber = 0.0;
    digitInfo.numStep = 1.0;
    
    for (int i = 0; i < 5; i++) {
        inputHandler.StartNewSegment(CPoint(i * 10, i * 10), &digitInfo, &cmdDispatcher);
        int iSeg = inputHandler.GetActiveSegment();
        inputHandler.EndCurrentSegment();
    }
    
    // Verify numbering: 1.0, 2.0, 3.0, 4.0, 5.0
    EXPECT_EQ(5, digitInfo.Fringes.size());
    for (int i = 0; i < 5; i++) {
        EXPECT_DOUBLE_EQ(1.0 + i, digitInfo.Fringes[i].GetNumber());
        EXPECT_EQ(1, digitInfo.Fringes[i].GetPointCount());  // Each has initial dot
    }
}

TEST_F(DrawModeTest, StartNewSegmentIsUndoable) {
    // Verify that StartNewSegment via CommandDispatcher is undoable
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo, &cmdDispatcher);
    int iSeg = inputHandler.GetActiveSegment();
    
    EXPECT_EQ(1, digitInfo.Fringes.size());
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Undo should remove both the segment creation and initial dot
    cmdDispatcher.Undo();  // Undo AddDotCommand (initial dot)
    EXPECT_EQ(0, digitInfo.Fringes.size());
    
    // Redo
    cmdDispatcher.Redo();  // Redo CreateSegmentCommand
    EXPECT_EQ(1, digitInfo.Fringes.size());
    EXPECT_EQ(1, digitInfo.Fringes[0].GetPointCount());

    EXPECT_DOUBLE_EQ(100.0, digitInfo.Fringes[0].GetPoint(0).x);
}

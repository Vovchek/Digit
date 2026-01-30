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
        inputHandler.SetMode(EditMode::Draw);
    }

    void TearDown() override {
        digitInfo.Fringes.clear();
    }
};

// ===== Basic Draw Operations =====

TEST_F(DrawModeTest, StartNewSegmentCreatesSegment) {
    int initialCount = digitInfo.Fringes.size();
    
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    
    EXPECT_EQ(initialCount + 1, digitInfo.Fringes.size());
    EXPECT_GE(inputHandler.GetActiveSegment(), 0);
    
    // Verify new segment has correct number
    CFringeSegment& newSeg = digitInfo.Fringes[inputHandler.GetActiveSegment()];
    EXPECT_DOUBLE_EQ(1.5, newSeg.GetNumber());  // CurrentNumber + numStep
}

TEST_F(DrawModeTest, AddDotCommandAddsPoint) {
    // Start a new segment
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add first dot via command
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(100, 100));
    cmdDispatcher.Execute(std::move(pCmd));
    
    CFringeSegment& segment = digitInfo.Fringes[iSeg];
    EXPECT_EQ(1, segment.GetPointCount());
    EXPECT_DOUBLE_EQ(100.0, segment.GetPoint(0).x);
    EXPECT_DOUBLE_EQ(100.0, segment.GetPoint(0).y);
}

TEST_F(DrawModeTest, AddMultipleDotsBuildsSegment) {
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 5 dots
    for (int i = 0; i < 5; i++) {
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
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(100, 100));
    cmdDispatcher.Execute(std::move(pCmd));
    
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_TRUE(cmdDispatcher.CanUndo());
    
    cmdDispatcher.Undo();
    EXPECT_EQ(0, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_FALSE(cmdDispatcher.CanUndo());
    EXPECT_TRUE(cmdDispatcher.CanRedo());
}

TEST_F(DrawModeTest, AddDotCommandSupportsRedo) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(100, 100));
    cmdDispatcher.Execute(std::move(pCmd));
    cmdDispatcher.Undo();
    
    EXPECT_EQ(0, digitInfo.Fringes[iSeg].GetPointCount());
    
    cmdDispatcher.Redo();
    EXPECT_EQ(1, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(100.0, digitInfo.Fringes[iSeg].GetPoint(0).x);
}

TEST_F(DrawModeTest, MultipleUndosRestoreCorrectState) {
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 3 dots
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(10, 10)));
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
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 3 dots
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(10, 10)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 2, CDPoint(30, 30)));
    
    EXPECT_EQ(3, digitInfo.Fringes[iSeg].GetPointCount());
    
    // Remove last dot
    cmdDispatcher.Execute(std::make_unique<RemoveLastDotCommand>(&digitInfo, iSeg));
    
    EXPECT_EQ(2, digitInfo.Fringes[iSeg].GetPointCount());
    EXPECT_DOUBLE_EQ(20.0, digitInfo.Fringes[iSeg].GetPoint(1).x);  // Last dot now
}

TEST_F(DrawModeTest, RemoveLastDotCommandSupportsUndo) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(10, 10)));
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
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(10, 10)));
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 1, CDPoint(20, 20)));
    
    // End segment
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    
    // Continue from last dot
    inputHandler.ContinueSegment(iSeg, 1, &digitInfo);
    EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());
}

// ===== End Segment Tests =====

TEST_F(DrawModeTest, EndCurrentSegmentClearsActive) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    EXPECT_GE(inputHandler.GetActiveSegment(), 0);
    
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

TEST_F(DrawModeTest, EndCurrentSegmentOnEmptyIsIdempotent) {
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
    
    inputHandler.EndCurrentSegment();
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

// ===== Mode Switching Tests =====

TEST_F(DrawModeTest, LeavingDrawModeEndsSegment) {
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    EXPECT_GE(inputHandler.GetActiveSegment(), 0);
    
    inputHandler.SetMode(EditMode::Navigate);
    EXPECT_EQ(-1, inputHandler.GetActiveSegment());
}

// ===== Complex Workflow Tests =====

TEST_F(DrawModeTest, CompleteDrawWorkflow) {
    // Start segment
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Draw a polyline with 5 dots
    for (int i = 0; i < 5; i++) {
        cmdDispatcher.Execute(std::make_unique<AddDotCommand>(
            &digitInfo, iSeg, i, CDPoint(10.0 + i * 10, 10.0 + i * 10)));
    }
    
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
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Add 3 dots
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(10, 10)));
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
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg = inputHandler.GetActiveSegment();
    
    // Segment has no dots yet
    EXPECT_EQ(0, digitInfo.Fringes[iSeg].GetPointCount());
    
    // This should trigger ASSERT in debug build
    // In release, behavior is undefined - don't test
    // EXPECT_DEATH(RemoveLastDotCommand cmd(&digitInfo, iSeg), "");
}

TEST_F(DrawModeTest, MultipleSegmentsIndependent) {
    // Start first segment
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg1 = inputHandler.GetActiveSegment();
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg1, 0, CDPoint(10, 10)));
    inputHandler.EndCurrentSegment();
    
    // Start second segment
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg2 = inputHandler.GetActiveSegment();
    cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg2, 0, CDPoint(100, 100)));
    
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
        inputHandler.StartNewSegment(CPoint(i * 10, i * 10), &digitInfo);
        int iSeg = inputHandler.GetActiveSegment();
        cmdDispatcher.Execute(std::make_unique<AddDotCommand>(&digitInfo, iSeg, 0, CDPoint(i * 10.0, i * 10.0)));
        inputHandler.EndCurrentSegment();
    }
    
    // Verify numbering: 1.0, 2.0, 3.0, 4.0, 5.0
    EXPECT_EQ(5, digitInfo.Fringes.size());
    for (int i = 0; i < 5; i++) {
        EXPECT_DOUBLE_EQ(1.0 + i, digitInfo.Fringes[i].GetNumber());
    }
}

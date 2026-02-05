#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/SelectionManager.h"
#include "DigitMode/CFringeSegment.h"
#include <vector>

using namespace DigitMode;

/**
 * @brief Test fixture for SelectionManager
 * 
 * Creates a test environment with multiple segments for selection testing
 */
class SelectionManagerTest : public ::testing::Test {
protected:
    SelectionManager selectionMgr;
    std::vector<CFringeSegment> testSegments;

    void SetUp() override {
        // Create test segments with different Numbers
        // Segments 0, 1, 2: Number = 1.0
        // Segments 3, 4: Number = 2.0
        // Segment 5: Number = 1.5

        testSegments.clear();
        
        CFringeSegment seg0(1.0, 0);
        seg0.AddPoint(CDPoint(10, 10));
        seg0.AddPoint(CDPoint(20, 20));
        seg0.AddPoint(CDPoint(30, 30));
        testSegments.push_back(seg0);

        CFringeSegment seg1(1.0, 1);
        seg1.AddPoint(CDPoint(40, 10));
        seg1.AddPoint(CDPoint(50, 20));
        testSegments.push_back(seg1);

        CFringeSegment seg2(1.0, 2);
        seg2.AddPoint(CDPoint(60, 10));
        seg2.AddPoint(CDPoint(70, 20));
        testSegments.push_back(seg2);

        CFringeSegment seg3(2.0, 0);
        seg3.AddPoint(CDPoint(80, 10));
        seg3.AddPoint(CDPoint(90, 20));
        testSegments.push_back(seg3);

        CFringeSegment seg4(2.0, 1);
        seg4.AddPoint(CDPoint(100, 10));
        seg4.AddPoint(CDPoint(110, 20));
        testSegments.push_back(seg4);

        CFringeSegment seg5(1.5, 0);
        seg5.AddPoint(CDPoint(120, 10));
        seg5.AddPoint(CDPoint(130, 20));
        testSegments.push_back(seg5);
    }

    void TearDown() override {
        selectionMgr.Clear();
        testSegments.clear();
    }
};

// ===== Basic Selection Tests =====

TEST_F(SelectionManagerTest, InitialStateIsEmpty) {
    EXPECT_TRUE(selectionMgr.IsEmpty());
    EXPECT_EQ(0, selectionMgr.GetCount());
    EXPECT_EQ(SelectionLevel::None, selectionMgr.GetLevel());
}

TEST_F(SelectionManagerTest, SelectDotWorks) {
    selectionMgr.SelectDot(0, 5);

    EXPECT_FALSE(selectionMgr.IsEmpty());
    EXPECT_EQ(1, selectionMgr.GetCount());
    EXPECT_EQ(SelectionLevel::Dot, selectionMgr.GetLevel());

    const auto& obj = selectionMgr.GetAt(0);
    EXPECT_EQ(SelectionLevel::Dot, obj.level);
    EXPECT_EQ(0, obj.iSegment);
    EXPECT_EQ(5, obj.iDot);
    EXPECT_TRUE(obj.IsValid());
}

TEST_F(SelectionManagerTest, SelectEdgeWorks) {
    selectionMgr.SelectEdge(2, 3);

    EXPECT_EQ(1, selectionMgr.GetCount());
    EXPECT_EQ(SelectionLevel::Edge, selectionMgr.GetLevel());

    const auto& obj = selectionMgr.GetAt(0);
    EXPECT_EQ(SelectionLevel::Edge, obj.level);
    EXPECT_EQ(2, obj.iSegment);
    EXPECT_EQ(3, obj.iEdge);
}

TEST_F(SelectionManagerTest, SelectSegmentWorks) {
    selectionMgr.SelectSegment(4);

    EXPECT_EQ(1, selectionMgr.GetCount());
    EXPECT_EQ(SelectionLevel::Segment, selectionMgr.GetLevel());

    const auto& obj = selectionMgr.GetAt(0);
    EXPECT_EQ(SelectionLevel::Segment, obj.level);
    EXPECT_EQ(4, obj.iSegment);
}

// ===== Fringe Selection Tests (Segment-Primary Model) =====

TEST_F(SelectionManagerTest, SelectFringeSelectsAllSegmentsWithSameNumber) {
    // Select fringe with Number = 1.0 (should select segments 0, 1, 2)
    selectionMgr.SelectFringe(1.0, testSegments);

    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(3, selectionMgr.GetCount());  // 3 segments with Number=1.0

    // Verify all selected segments have Number=1.0
    for (size_t i = 0; i < selectionMgr.GetCount(); i++) {
        const auto& obj = selectionMgr.GetAt(i);
        EXPECT_EQ(SelectionLevel::Fringe, obj.level);
        EXPECT_DOUBLE_EQ(1.0, obj.Number);
        EXPECT_DOUBLE_EQ(1.0, testSegments[obj.iSegment].GetNumber());
    }
}

TEST_F(SelectionManagerTest, SelectFringeWithDifferentNumber) {
    // Select fringe with Number = 2.0 (should select segments 3, 4)
    selectionMgr.SelectFringe(2.0, testSegments);

    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(2, selectionMgr.GetCount());

    for (size_t i = 0; i < selectionMgr.GetCount(); i++) {
        const auto& obj = selectionMgr.GetAt(i);
        EXPECT_DOUBLE_EQ(2.0, obj.Number);
    }
}

TEST_F(SelectionManagerTest, SelectFringeWithNoMatchingSegments) {
    // Try to select fringe with Number that doesn't exist
    selectionMgr.SelectFringe(99.0, testSegments);

    EXPECT_TRUE(selectionMgr.IsEmpty());
}

// ===== Clear Tests =====

TEST_F(SelectionManagerTest, ClearRemovesSelection) {
    selectionMgr.SelectDot(0, 0);
    EXPECT_FALSE(selectionMgr.IsEmpty());

    selectionMgr.Clear();
    EXPECT_TRUE(selectionMgr.IsEmpty());
    EXPECT_EQ(0, selectionMgr.GetCount());
}

TEST_F(SelectionManagerTest, NewSelectionClearsPrevious) {
    selectionMgr.SelectDot(0, 0);
    EXPECT_EQ(1, selectionMgr.GetCount());

    selectionMgr.SelectSegment(1);
    EXPECT_EQ(1, selectionMgr.GetCount());  // Should have only new selection
    EXPECT_EQ(SelectionLevel::Segment, selectionMgr.GetLevel());
}

// ===== Multi-Selection Tests =====

TEST_F(SelectionManagerTest, AddToSelectionSameLevel) {
    selectionMgr.SelectDot(0, 0);

    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 0;
    obj.iDot = 1;

    bool success = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success);
    EXPECT_EQ(2, selectionMgr.GetCount());
}

TEST_F(SelectionManagerTest, AddToSelectionDifferentLevelFails) {
    selectionMgr.SelectDot(0, 0);

    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Segment;  // Different level
    obj.iSegment = 1;

    bool success = selectionMgr.AddToSelection(obj);
    EXPECT_FALSE(success);  // Should fail
    EXPECT_EQ(1, selectionMgr.GetCount());  // Count unchanged
}

TEST_F(SelectionManagerTest, AddToSelectionTogglesDuplicate) {
    selectionMgr.SelectDot(0, 0);

    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 0;
    obj.iDot = 1;

    // Add first time
    bool success1 = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success1);
    EXPECT_EQ(2, selectionMgr.GetCount());

    // Add same object again (should toggle off)
    bool success2 = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success2);
    EXPECT_EQ(1, selectionMgr.GetCount());  // Back to 1
}

// ===== Promotion Tests =====

TEST_F(SelectionManagerTest, PromoteToFringeSingleSegment) {
    // Select a dot in segment 0 (Number=1.0)
    selectionMgr.SelectDot(0, 0);

    // Promote to fringe
    selectionMgr.PromoteToFringe(testSegments);

    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(3, selectionMgr.GetCount());  // All segments with Number=1.0 (0, 1, 2)
}

TEST_F(SelectionManagerTest, PromoteToFringeMultipleSegmentsSameNumber) {
    // Select dots from segments 0 and 1 (both have Number=1.0)
    selectionMgr.SelectDot(0, 0);

    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 1;
    obj.iDot = 0;
    selectionMgr.AddToSelection(obj);

    EXPECT_EQ(2, selectionMgr.GetCount());

    // Promote to fringe
    selectionMgr.PromoteToFringe(testSegments);

    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(3, selectionMgr.GetCount());  // All segments with Number=1.0
}

TEST_F(SelectionManagerTest, PromoteToFringeMultipleNumbers) {
    // Select dots from segments with different Numbers
    selectionMgr.SelectDot(0, 0);  // Number=1.0

    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 3;  // Number=2.0
    obj.iDot = 0;
    selectionMgr.AddToSelection(obj);

    // Promote to fringe
    selectionMgr.PromoteToFringe(testSegments);

    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(5, selectionMgr.GetCount());  // 3 segments with 1.0 + 2 with 2.0
}

// ===== Edge Cases =====

TEST_F(SelectionManagerTest, SelectDotWithNegativeIndices) {
    // Should still work (no validation in SelectionManager)
    selectionMgr.SelectDot(-1, -1);

    EXPECT_EQ(1, selectionMgr.GetCount());
    const auto& obj = selectionMgr.GetAt(0);
    EXPECT_EQ(-1, obj.iSegment);
    EXPECT_EQ(-1, obj.iDot);
}

TEST_F(SelectionManagerTest, AddToEmptySelectionWorks) {
    // Adding to empty selection should work
    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 0;
    obj.iDot = 0;

    bool success = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success);
    EXPECT_EQ(1, selectionMgr.GetCount());
}

TEST_F(SelectionManagerTest, PromoteEmptySelectionDoesNothing) {
    selectionMgr.PromoteToFringe(testSegments);

    EXPECT_TRUE(selectionMgr.IsEmpty());
}

// ===== Performance Test =====

TEST_F(SelectionManagerTest, LargeSelectionPerformance) {
    // Create 1000 segments
    std::vector<CFringeSegment> largeSegments;
    for (int i = 0; i < 1000; i++) {
        CFringeSegment seg(1.0, i);
        seg.AddPoint(CDPoint(i * 10.0, i * 10.0));
        largeSegments.push_back(seg);
    }

    // Select entire fringe (all 1000 segments)
    selectionMgr.SelectFringe(1.0, largeSegments);

    EXPECT_EQ(1000, selectionMgr.GetCount());
}

// ===== Box Selection Tests =====

TEST_F(SelectionManagerTest, BoxSelectionSelectsDots) {
    // Use existing testSegments from SetUp (member variable) instead of local copy
    // This ensures vectors stay valid throughout the test
    
    // Define selection box that contains points (10,10) and (20,20)
    // but NOT (30,30), (40,40), etc.
    CRect box(5, 5, 25, 25);  // (left=5, top=5, right=25, bottom=25)

    // Perform box selection
    size_t count = selectionMgr.SelectBox(box, testSegments);  // ← Use testSegments (member), not local

    // Verify selection count
    EXPECT_EQ(2, count);  // Should select 2 dots: (10,10) and (20,20) from segment 0
    
    EXPECT_EQ(SelectionLevel::Dot, selectionMgr.GetLevel());
    EXPECT_EQ(2, selectionMgr.GetCount());

    // Verify first selected dot
    const auto& obj1 = selectionMgr.GetAt(0);
    EXPECT_EQ(SelectionLevel::Dot, obj1.level);
    EXPECT_EQ(0, obj1.iSegment);  // Segment 0
    EXPECT_EQ(0, obj1.iDot);       // Dot 0 at (10,10)

    // Verify second selected dot
    const auto& obj2 = selectionMgr.GetAt(1);
    EXPECT_EQ(SelectionLevel::Dot, obj2.level);
    EXPECT_EQ(0, obj2.iSegment);  // Segment 0
    EXPECT_EQ(1, obj2.iDot);       // Dot 1 at (20,20)
}

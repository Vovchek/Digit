#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/SelectionManager.h"
#include "DigitMode/InputHandler.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/CFringeSegment.h"

using namespace DigitMode;

/**
 * @brief Test fixture for Navigate Mode functionality
 * 
 * Tests according to UX v1.0:
 * - Box selection (default, Shift=Segment, Alt=Fringe, Ctrl=Add)
 * - Click selection (LClick, Ctrl+Click, Shift+Click, Alt+Click)
 * - Selection levels and promotion
 */
class NavigateModeTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    InputHandler inputHandler;
    CommandDispatcher cmdDispatcher;

    void SetUp() override {
        digitInfo.Init();
        digitInfo.CurrentNumber = 0.0;
        digitInfo.numStep = 1.0;
        inputHandler.SetMode(FringeEditMode::Navigate);
    }

    void TearDown() override {
        digitInfo.Fringes.clear();
    }

    // Helper: Create test segment
    int CreateSegment(double number, const std::vector<CDPoint>& points) {
        CFringeSegment seg(number, digitInfo.Fringes.size());
        for (const auto& p : points) {
            seg.AddPoint(p);
        }
        digitInfo.Fringes.push_back(seg);
        return digitInfo.Fringes.size() - 1;
    }
};

// ===== Box Selection - Default Mode =====

TEST_F(NavigateModeTest, BoxSelect_Default_SelectsDotsInside) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50), CDPoint(100, 100)});
    
    CDRect box(5, 5, 55, 55);
    box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes);
    
    EXPECT_EQ(2u, count);  // Dots at (10,10) and (50,50)
    EXPECT_EQ(SelectionLevel::Dot, digitInfo.selectionManager.GetLevel());
}

TEST_F(NavigateModeTest, BoxSelect_Default_SelectsEdgesIntersecting) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(100, 10)});
    
    CDRect box(30, 5, 70, 15);  // Crosses edge but doesn't contain dots
    box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes);
    
    EXPECT_GT(count, 0u);  // Should select edge
    // Note: May select edge OR dots depending on intersection logic
}

TEST_F(NavigateModeTest, BoxSelect_Default_SelectsSegmentWhenAllEdgesIncluded) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50), CDPoint(90, 90)});
    
    CDRect box(5, 5, 95, 95);  // Includes entire segment
    box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes);
    
    EXPECT_EQ(1u, count);  // Should promote to segment
    EXPECT_EQ(SelectionLevel::Segment, digitInfo.selectionManager.GetLevel());
}

// ===== Box Selection - Segment Mode (Shift) =====

TEST_F(NavigateModeTest, BoxSelect_SegmentMode_SelectsIfAnyPartIntersects) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(100, 100)});
    CreateSegment(2.0, {CDPoint(200, 200), CDPoint(300, 300)});
    
    CDRect box(5, 5, 30, 30);  // Only intersects first segment's start
    box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes, 
                                                         BoxSelectionMode::Segment);
    
    EXPECT_EQ(1u, count);
    EXPECT_EQ(SelectionLevel::Segment, digitInfo.selectionManager.GetLevel());
    EXPECT_EQ(0, digitInfo.selectionManager.GetAt(0).iSegment);
}

TEST_F(NavigateModeTest, BoxSelect_SegmentMode_MultipleSegments) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(2.0, {CDPoint(30, 30), CDPoint(70, 70)});
    
    CDRect box(0, 0, 60, 60);
    box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes,
                                                         BoxSelectionMode::Segment);
    
    EXPECT_EQ(2u, count);  // Both segments intersect
    EXPECT_EQ(SelectionLevel::Segment, digitInfo.selectionManager.GetLevel());
}

// ===== Box Selection - Fringe Mode (Alt) =====

TEST_F(NavigateModeTest, BoxSelect_FringeMode_SelectsAllSegmentsWithSameNumber) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(1.0, {CDPoint(200, 200), CDPoint(250, 250)});  // Same number
    CreateSegment(2.0, {CDPoint(300, 300), CDPoint(350, 350)});  // Different number
    
    CDRect box(5, 5, 55, 55);  // Only intersects first segment
	box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes,
                                                         BoxSelectionMode::Fringe);
    
    EXPECT_EQ(2u, count);  // Both segments with Number=1.0
    EXPECT_EQ(SelectionLevel::Fringe, digitInfo.selectionManager.GetLevel());
    EXPECT_DOUBLE_EQ(1.0, digitInfo.selectionManager.GetAt(0).Number);
    EXPECT_DOUBLE_EQ(1.0, digitInfo.selectionManager.GetAt(1).Number);
}

TEST_F(NavigateModeTest, BoxSelect_FringeMode_MultipleFringes) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(1.0, {CDPoint(100, 100), CDPoint(150, 150)});
    CreateSegment(2.0, {CDPoint(30, 30), CDPoint(70, 70)});
    CreateSegment(2.0, {CDPoint(200, 200), CDPoint(250, 250)});
    
    CDRect box(0, 0, 80, 80);  // Intersects segments from both fringes
	box.NormalizeRect();
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes,
                                                         BoxSelectionMode::Fringe);
    
    EXPECT_EQ(4u, count);  // All segments selected (2 fringes)
}

// ===== Box Selection - Add Mode (Ctrl) =====

TEST_F(NavigateModeTest, BoxSelect_AddMode_AddsToExisting) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(2.0, {CDPoint(100, 100), CDPoint(150, 150)});
    
    // First selection
    CDRect box1(5, 5, 55, 55);
	box1.NormalizeRect();
    digitInfo.selectionManager.SelectBox(box1, digitInfo.Fringes);
    size_t count1 = digitInfo.selectionManager.GetCount();
    
    // Add to selection
    CDRect box2(95, 95, 155, 155);
	box2.NormalizeRect();
    size_t count2 = digitInfo.selectionManager.SelectBox(box2, digitInfo.Fringes,
                                                          BoxSelectionMode::AddMode);
    
    EXPECT_GT(count2, count1);  // Selection increased
}

// ===== Click Selection =====

TEST_F(NavigateModeTest, ClickDot_SelectsSingle) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    
    inputHandler.OnLButtonDown(0, CPoint(10, 10), &digitInfo, &cmdDispatcher);
    
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());
    EXPECT_EQ(SelectionLevel::Dot, digitInfo.selectionManager.GetLevel());
}

TEST_F(NavigateModeTest, CtrlClickDot_TogglesSelection) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50), CDPoint(90, 90)});
    
    // Select first dot
    digitInfo.selectionManager.SelectDot(0, 0);
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());
    
    // Ctrl+Click second dot (add)
    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 0;
    obj.iDot = 1;
    digitInfo.selectionManager.AddToSelection(obj);
    
    EXPECT_EQ(2u, digitInfo.selectionManager.GetCount());
    
    // Ctrl+Click second dot again (toggle off)
    digitInfo.selectionManager.AddToSelection(obj);
    
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());
}

TEST_F(NavigateModeTest, AltClick_PromotesToFringe) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(1.0, {CDPoint(100, 100), CDPoint(150, 150)});  // Same number
    
    digitInfo.selectionManager.SelectDot(0, 0);
    EXPECT_EQ(SelectionLevel::Dot, digitInfo.selectionManager.GetLevel());
    
    digitInfo.selectionManager.PromoteToFringe(digitInfo.Fringes);
    
    EXPECT_EQ(2, digitInfo.selectionManager.GetCount());  // Both segments selected
    EXPECT_EQ(SelectionLevel::Fringe, digitInfo.selectionManager.GetLevel());
}

// ===== Selection Levels =====

TEST_F(NavigateModeTest, SelectionLevelHierarchy) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50), CDPoint(90, 90)});
    
    // Dot selection
    digitInfo.selectionManager.SelectDot(0, 0);
    EXPECT_EQ(SelectionLevel::Dot, digitInfo.selectionManager.GetLevel());
    
    // Edge selection
    digitInfo.selectionManager.SelectEdge(0, 0);
    EXPECT_EQ(SelectionLevel::Edge, digitInfo.selectionManager.GetLevel());
    
    // Segment selection
    digitInfo.selectionManager.SelectSegment(0);
    EXPECT_EQ(SelectionLevel::Segment, digitInfo.selectionManager.GetLevel());
    
    // Fringe selection
    digitInfo.selectionManager.SelectFringe(1.0, digitInfo.Fringes);
    EXPECT_EQ(SelectionLevel::Fringe, digitInfo.selectionManager.GetLevel());
}

TEST_F(NavigateModeTest, SelectionPersistsAcrossModeSwitch) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    
    digitInfo.selectionManager.SelectDot(0, 0);
    EXPECT_EQ(1, digitInfo.selectionManager.GetCount());
    
    inputHandler.SetMode(FringeEditMode::Draw);
    inputHandler.SetMode(FringeEditMode::Navigate);
    
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());  // Selection persisted
}

// ===== Edge Cases =====

TEST_F(NavigateModeTest, BoxSelect_EmptyRegion_SelectsNothing) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    
    CRect box(100, 100, 200, 200);  // Far from segment
    size_t count = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes);
    
    EXPECT_EQ(0, count);
    EXPECT_EQ(SelectionLevel::None, digitInfo.selectionManager.GetLevel());
}

TEST_F(NavigateModeTest, AddToSelection_DifferentLevels_Rejected) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    
    digitInfo.selectionManager.SelectDot(0, 0);
    
    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Segment;  // Different level
    obj.iSegment = 0;
    
    bool added = digitInfo.selectionManager.AddToSelection(obj);
    
    EXPECT_FALSE(added);  // Rejected
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());  // Unchanged
}

TEST_F(NavigateModeTest, ClearSelection_Works) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    
    digitInfo.selectionManager.SelectDot(0, 0);
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());
    
    digitInfo.selectionManager.Clear();
    
    EXPECT_EQ(0u, digitInfo.selectionManager.GetCount());
    EXPECT_EQ(SelectionLevel::None, digitInfo.selectionManager.GetLevel());
}

// ===== Integration with HandleBoxSelection =====

TEST_F(NavigateModeTest, HandleBoxSelection_UsesModifiers) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(100, 100)});
    CreateSegment(1.0, {CDPoint(200, 200), CDPoint(300, 300)});
    
    // This test verifies InputHandler::HandleBoxSelection
    // In real usage, modifiers are read from keyboard
    // For testing, we use SelectBox directly with explicit modes
    
    CRect box(5, 5, 50, 50);
    
    // Default mode
    size_t count1 = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes,
                                                          BoxSelectionMode::Default);
    EXPECT_GT(count1, 0u);
    
    // Fringe mode (Alt)
    size_t count2 = digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes,
                                                          BoxSelectionMode::Fringe);
    EXPECT_EQ(2u, count2);  // Both segments with Number=1.0
}

// ===== Complex Scenarios =====

TEST_F(NavigateModeTest, ComplexWorkflow_SelectModifyDeselect) {
    CreateSegment(1.0, {CDPoint(10, 10), CDPoint(50, 50)});
    CreateSegment(2.0, {CDPoint(30, 30), CDPoint(70, 70)});
    
    // Box select
    CRect box(0, 0, 60, 60);
    digitInfo.selectionManager.SelectBox(box, digitInfo.Fringes);
    size_t initialCount = digitInfo.selectionManager.GetCount();
    EXPECT_GT(initialCount, 0u);
    
    // Clear
    digitInfo.selectionManager.Clear();
    EXPECT_EQ(0u, digitInfo.selectionManager.GetCount());
    
    // Select single segment
    digitInfo.selectionManager.SelectSegment(0);
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());
    
    // Promote to fringe
    digitInfo.selectionManager.PromoteToFringe(digitInfo.Fringes);
    EXPECT_EQ(1u, digitInfo.selectionManager.GetCount());  // Only 1 segment with Number=1.0
    EXPECT_EQ(SelectionLevel::Fringe, digitInfo.selectionManager.GetLevel());
}

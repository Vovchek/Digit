#include "stdafx.h"
#include "gtest/gtest.h"

#include "DigitMode/Commands/AllCommands.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/CFringeSegment.h"

using namespace DigitMode;

// ===== CreateSegmentCommand Tests =====

TEST(AllCommands, CreateSegmentCommand_CreatesAndUndoes) {
    CDigitInfo doc; doc.Fringes.clear();
    std::vector<CDPoint> pts = { CDPoint(1.0,2.0), CDPoint(3.0,4.0) };
    CreateSegmentCommand cmd(doc, pts, 5.0);
    cmd.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[0].GetNumber());
    cmd.Undo();
    EXPECT_EQ(0u, doc.Fringes.size());
}

TEST(AllCommands, CreateSegmentCommand_PreservesOtherSegments) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment existing(1.0, 0);
    existing.AddPoint(CDPoint(100.0, 100.0));
    doc.Fringes.push_back(existing);

    std::vector<CDPoint> pts = { CDPoint(1.0,2.0), CDPoint(3.0,4.0) };
    CreateSegmentCommand cmd(doc, pts, 5.0);
    cmd.Execute();
    ASSERT_EQ(2u, doc.Fringes.size());
    EXPECT_DOUBLE_EQ(100.0, doc.Fringes[0].GetPoint(0).x);
    EXPECT_DOUBLE_EQ(1.0, doc.Fringes[1].GetPoint(0).x);
}

// ===== ExtendSegmentCommand Tests =====

TEST(AllCommands, ExtendSegmentCommand_AddsAtTailAndHead) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(1.0, 0);
    seg.AddPoint(CDPoint(10.0, 10.0));
    doc.Fringes.push_back(seg);

    ExtendSegmentCommand extTail(doc, 0, false, CDPoint(20.0, 20.0));
    extTail.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(20.0, doc.Fringes[0].GetPoint(1).x);
    extTail.Undo();
    EXPECT_EQ(1, doc.Fringes[0].GetPointCount());

    ExtendSegmentCommand extHead(doc, 0, true, CDPoint(5.0, 5.0));
    extHead.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[0].GetPoint(0).x);
    extHead.Undo();
    EXPECT_EQ(1, doc.Fringes[0].GetPointCount());
}

TEST(AllCommands, ExtendSegmentCommand_MultipleExtensions) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(1.0, 0);
    seg.AddPoint(CDPoint(10.0, 10.0));
    doc.Fringes.push_back(seg);

    ExtendSegmentCommand ext1(doc, 0, false, CDPoint(20.0, 20.0));
    ExtendSegmentCommand ext2(doc, 0, false, CDPoint(30.0, 30.0));
    
    ext1.Execute();
    ext2.Execute();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(30.0, doc.Fringes[0].GetPoint(2).x);
    
    ext2.Undo();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    ext1.Undo();
    EXPECT_EQ(1, doc.Fringes[0].GetPointCount());
}

// ===== MoveDotCommand Tests =====

TEST(AllCommands, MoveDotCommand_MovesAndUndoes) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0,0);
    seg.AddPoint(CDPoint(0.0,0.0));
    seg.AddPoint(CDPoint(5.0,5.0));
    doc.Fringes.push_back(seg);

    CDPoint oldPos = doc.Fringes[0].GetPoint(1);
    CDPoint newPos(9.0,9.0);
    MoveDotCommand mv(doc, 0, 1, oldPos, newPos);
    mv.Execute();
    EXPECT_DOUBLE_EQ(9.0, doc.Fringes[0].GetPoint(1).x);
    mv.Undo();
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[0].GetPoint(1).x);
}

TEST(AllCommands, MoveDotCommand_PreservesOtherDots) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0,0);
    seg.AddPoint(CDPoint(0.0,0.0));
    seg.AddPoint(CDPoint(5.0,5.0));
    seg.AddPoint(CDPoint(10.0,10.0));
    doc.Fringes.push_back(seg);

    MoveDotCommand mv(doc, 0, 1, CDPoint(5.0,5.0), CDPoint(7.0,7.0));
    mv.Execute();
    EXPECT_DOUBLE_EQ(0.0, doc.Fringes[0].GetPoint(0).x);
    EXPECT_DOUBLE_EQ(7.0, doc.Fringes[0].GetPoint(1).x);
    EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(2).x);
}

// ===== DeleteDotCommand Tests =====

TEST(AllCommands, DeleteDotCommand_RemovesAndRestores) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0,0);
    seg.AddPoint(CDPoint(1.0,1.0));
    seg.AddPoint(CDPoint(2.0,2.0));
    seg.AddPoint(CDPoint(3.0,3.0));
    doc.Fringes.push_back(seg);

    DeleteDotCommand del(doc, 0, 1);
    del.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(3.0, doc.Fringes[0].GetPoint(1).x);
    del.Undo();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[0].GetPoint(1).x);
}

TEST(AllCommands, DeleteDotCommand_FirstAndLastDots) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0,0);
    seg.AddPoint(CDPoint(1.0,1.0));
    seg.AddPoint(CDPoint(2.0,2.0));
    seg.AddPoint(CDPoint(3.0,3.0));
    doc.Fringes.push_back(seg);

    // Delete first
    DeleteDotCommand del0(doc, 0, 0);
    del0.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[0].GetPoint(0).x);
    del0.Undo();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(1.0, doc.Fringes[0].GetPoint(0).x);

    // Delete last
    DeleteDotCommand del2(doc, 0, 2);
    del2.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[0].GetPoint(1).x);
    del2.Undo();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
}

// ===== SplitSegmentCommand Tests =====

TEST(AllCommands, SplitSegmentCommand_SplitsAndUndoes) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment s(0.0,0);
    // points 0..4
    for (int i=0;i<5;i++) s.AddPoint(CDPoint(i*1.0, i*1.0));
    doc.Fringes.push_back(s);

    SplitSegmentCommand sp(doc, 0, 2);
    sp.Execute();
    // original should have first two points
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    // new segment appended
    EXPECT_EQ(3, doc.Fringes[1].GetPointCount());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[1].GetPoint(0).x);

    sp.Undo();
    EXPECT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(5, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(4.0, doc.Fringes[0].GetPoint(4).x);
}

TEST(AllCommands, SplitSegmentCommand_SplitAtDifferentIndices) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment s(1.5,0);
    for (int i=0;i<6;i++) s.AddPoint(CDPoint(i*10.0, i*10.0));
    doc.Fringes.push_back(s);

    SplitSegmentCommand sp(doc, 0, 4);
    sp.Execute();
    EXPECT_EQ(4, doc.Fringes[0].GetPointCount());
    EXPECT_EQ(2, doc.Fringes[1].GetPointCount());
    EXPECT_DOUBLE_EQ(40.0, doc.Fringes[1].GetPoint(0).x);
    EXPECT_DOUBLE_EQ(50.0, doc.Fringes[1].GetPoint(1).x);
}

// ===== ConnectSegmentsCommand Comprehensive Tests =====

class ConnectSegmentsTest : public ::testing::Test {
protected:
    CDigitInfo doc;
    
    void SetUp() override {
        doc.Fringes.clear();
    }

    // Helper to create two standard test segments
    void MakeStandardSegments(CFringeSegment& A, CFringeSegment& B) {
        A = CFringeSegment(1.0, 0);
        A.AddPoint(CDPoint(0,0));
        A.AddPoint(CDPoint(1,1));
        A.AddPoint(CDPoint(2,2));

        B = CFringeSegment(2.0, 1);
        B.AddPoint(CDPoint(10,10));
        B.AddPoint(CDPoint(11,11));
    }

    void AddSegments(const CFringeSegment& A, const CFringeSegment& B) {
        doc.Fringes.clear();
        doc.Fringes.push_back(A);
        doc.Fringes.push_back(B);
    }

    // Verify segment count and point values
    void ExpectSegmentPoints(int segIdx, const std::vector<double>& expectedX) {
        ASSERT_LT(segIdx, (int)doc.Fringes.size());
        auto& seg = doc.Fringes[segIdx];
        ASSERT_EQ((int)expectedX.size(), seg.GetPointCount()) 
            << "Segment " << segIdx << " point count mismatch";
        for (size_t i = 0; i < expectedX.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedX[i], seg.GetPoint((int)i).x) 
                << "Segment " << segIdx << " point " << i << " x mismatch";
        }
    }
};

TEST_F(ConnectSegmentsTest, TailToHead_AEndBStart_Merge) {
    CFringeSegment A, B;
    MakeStandardSegments(A, B);
    AddSegments(A, B);

    // Connect A.tail to B.head: result = [A points][B points]
    ConnectSegmentsCommand cmd(doc, 0, true, 1, false);
    cmd.Execute();
    
    ASSERT_EQ(1u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2, 10, 11});

    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2});
    ExpectSegmentPoints(1, {10, 11});
}

TEST_F(ConnectSegmentsTest, TailToTail_AEndBEnd_Reverse) {
    CFringeSegment A, B;
    MakeStandardSegments(A, B);
    AddSegments(A, B);

    // Connect A.tail to B.tail: result = [A points][B reversed]
    ConnectSegmentsCommand cmd(doc, 0, true, 1, true);
    cmd.Execute();
    
    ASSERT_EQ(1u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2, 11, 10});

    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2});
    ExpectSegmentPoints(1, {10, 11});
}

TEST_F(ConnectSegmentsTest, HeadToHead_AStartBStart_Insert) {
    CFringeSegment A, B;
    MakeStandardSegments(A, B);
    AddSegments(A, B);

    // Connect A.head to B.head: result = [B points][A points]
    ConnectSegmentsCommand cmd(doc, 0, false, 1, false);
    cmd.Execute();
    
    ASSERT_EQ(1u, doc.Fringes.size());
    ExpectSegmentPoints(0, {10, 11, 0, 1, 2});

    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2});
    ExpectSegmentPoints(1, {10, 11});
}

TEST_F(ConnectSegmentsTest, HeadToTail_AStartBEnd_InsertReversed) {
    CFringeSegment A, B;
    MakeStandardSegments(A, B);
    AddSegments(A, B);

    // Connect A.head to B.tail: result = [B reversed][A points]
    ConnectSegmentsCommand cmd(doc, 0, false, 1, true);
    cmd.Execute();
    
    ASSERT_EQ(1u, doc.Fringes.size());
    ExpectSegmentPoints(0, {11, 10, 0, 1, 2});

    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 1, 2});
    ExpectSegmentPoints(1, {10, 11});
}

TEST_F(ConnectSegmentsTest, UndoPreservesOriginalNumbers) {
    CFringeSegment A(3.5, 0), B(7.2, 1);
    A.AddPoint(CDPoint(0,0));
    B.AddPoint(CDPoint(10,10));
    AddSegments(A, B);

    ConnectSegmentsCommand cmd(doc, 0, true, 1, false);
    cmd.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    
    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    EXPECT_DOUBLE_EQ(3.5, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(7.2, doc.Fringes[1].GetNumber());
}

TEST_F(ConnectSegmentsTest, ConnectWhenBIndexLessThanA_UndoRestoresCorrectly) {
    // Bug scenario: when segB index < segA index, undo must account for shifted indices
    CFringeSegment seg0(1.0, 0), seg1(2.0, 1), seg2(3.0, 2);
    seg0.AddPoint(CDPoint(0,0));
    seg1.AddPoint(CDPoint(10,10));
    seg2.AddPoint(CDPoint(20,20));
    doc.Fringes.push_back(seg0);
    doc.Fringes.push_back(seg1);
    doc.Fringes.push_back(seg2);

    // Connect seg2 (index 2) to seg0 (index 0): B < A
    ConnectSegmentsCommand cmd(doc, 2, true, 0, false);
    cmd.Execute();
    
    ASSERT_EQ(2u, doc.Fringes.size());
    // After execution: seg0 is removed, seg2 is modified
    // Expected: seg1 at index 0, merged seg at index 1
    
    cmd.Undo();
    ASSERT_EQ(3u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0});
    ExpectSegmentPoints(1, {10});
    ExpectSegmentPoints(2, {20});
}

TEST_F(ConnectSegmentsTest, MultipleConnections_ChainSegments) {
    CFringeSegment s0(1.0,0), s1(2.0,1), s2(3.0,2);
    s0.AddPoint(CDPoint(0,0));
    s1.AddPoint(CDPoint(10,10));
    s2.AddPoint(CDPoint(20,20));
    doc.Fringes.push_back(s0);
    doc.Fringes.push_back(s1);
    doc.Fringes.push_back(s2);

    // Connect s0 tail -> s1 head
    ConnectSegmentsCommand cmd1(doc, 0, true, 1, false);
    cmd1.Execute();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 10});

    // Connect merged tail -> s2 head
    ConnectSegmentsCommand cmd2(doc, 0, true, 1, false);
    cmd2.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 10, 20});

    cmd2.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0, 10});
    ExpectSegmentPoints(1, {20});

    cmd1.Undo();
    ASSERT_EQ(3u, doc.Fringes.size());
    ExpectSegmentPoints(0, {0});
    ExpectSegmentPoints(1, {10});
}

TEST_F(ConnectSegmentsTest, EdgeCase_SinglePointSegments) {
    CFringeSegment A(1.0, 0), B(2.0, 1);
    A.AddPoint(CDPoint(5,5));
    B.AddPoint(CDPoint(15,15));
    AddSegments(A, B);

    ConnectSegmentsCommand cmd(doc, 0, true, 1, false);
    cmd.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());

    cmd.Undo();
    ASSERT_EQ(2u, doc.Fringes.size());
    EXPECT_EQ(1, doc.Fringes[0].GetPointCount());
    EXPECT_EQ(1, doc.Fringes[1].GetPointCount());
}

TEST_F(ConnectSegmentsTest, ConnectPreservesNumberOfMergedSegment) {
    CFringeSegment A(100.5, 0), B(200.7, 1);
    A.AddPoint(CDPoint(0,0));
    B.AddPoint(CDPoint(10,10));
    AddSegments(A, B);

    ConnectSegmentsCommand cmd(doc, 0, true, 1, false);
    cmd.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    // Merged segment should keep A's number
    EXPECT_DOUBLE_EQ(100.5, doc.Fringes[0].GetNumber());
}

// ===== RenumberSegmentsCommand Tests =====

TEST(AllCommands, RenumberSegmentsCommand_ChangesAndRestores) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment s1(1.0,0); s1.AddPoint(CDPoint(0,0));
    CFringeSegment s2(2.0,1); s2.AddPoint(CDPoint(1,1));
    doc.Fringes.push_back(s1); doc.Fringes.push_back(s2);

    std::vector<size_t> idxs = {0,1};
    RenumberSegmentsCommand ren(doc, idxs, 9.9);
    ren.Execute();
    EXPECT_DOUBLE_EQ(9.9, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(9.9, doc.Fringes[1].GetNumber());
    ren.Undo();
    EXPECT_DOUBLE_EQ(1.0, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[1].GetNumber());
}

TEST(AllCommands, RenumberSegmentsCommand_SingleSegment) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment s(5.5, 0);
    s.AddPoint(CDPoint(0,0));
    doc.Fringes.push_back(s);

    std::vector<size_t> idxs = {0};
    RenumberSegmentsCommand ren(doc, idxs, 42.0);
    ren.Execute();
    EXPECT_DOUBLE_EQ(42.0, doc.Fringes[0].GetNumber());
    ren.Undo();
    EXPECT_DOUBLE_EQ(5.5, doc.Fringes[0].GetNumber());
}

TEST(AllCommands, RenumberSegmentsCommand_NonContiguousIndices) {
    CDigitInfo doc; doc.Fringes.clear();
    for (int i = 0; i < 5; ++i) {
        CFringeSegment s(i + 1.0, i);
        s.AddPoint(CDPoint(i*10.0, i*10.0));
        doc.Fringes.push_back(s);
    }

    std::vector<size_t> idxs = {1, 3};
    RenumberSegmentsCommand ren(doc, idxs, 99.0);
    ren.Execute();
    EXPECT_DOUBLE_EQ(1.0, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(99.0, doc.Fringes[1].GetNumber());
    EXPECT_DOUBLE_EQ(3.0, doc.Fringes[2].GetNumber());
    EXPECT_DOUBLE_EQ(99.0, doc.Fringes[3].GetNumber());
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[4].GetNumber());

    ren.Undo();
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[1].GetNumber());
    EXPECT_DOUBLE_EQ(4.0, doc.Fringes[3].GetNumber());
}

// ===== DeleteSegmentsCommand Tests =====

TEST(AllCommands, DeleteSegmentsCommand_RemovesAndRestores) {
    CDigitInfo doc; doc.Fringes.clear();
    for (int i=0;i<4;i++) {
        CFringeSegment s(static_cast<double>(i+1), i);
        s.AddPoint(CDPoint(i*10.0, i*10.0));
        doc.Fringes.push_back(s);
    }
    std::vector<size_t> toRemove = {1,3};
    DeleteSegmentsCommand del(doc, toRemove);
    del.Execute();
    EXPECT_EQ(2u, doc.Fringes.size());
    del.Undo();
    EXPECT_EQ(4u, doc.Fringes.size());
    EXPECT_DOUBLE_EQ(10.0, doc.Fringes[1].GetPoint(0).x);
    EXPECT_DOUBLE_EQ(30.0, doc.Fringes[3].GetPoint(0).x);
}

TEST(AllCommands, DeleteSegmentsCommand_DeleteAll) {
    CDigitInfo doc; doc.Fringes.clear();
    for (int i = 0; i < 3; ++i) {
        CFringeSegment s(i + 1.0, i);
        s.AddPoint(CDPoint(i*10.0, i*10.0));
        doc.Fringes.push_back(s);
    }

    std::vector<size_t> toRemove = {0, 1, 2};
    DeleteSegmentsCommand del(doc, toRemove);
    del.Execute();
    EXPECT_EQ(0u, doc.Fringes.size());

    del.Undo();
    EXPECT_EQ(3u, doc.Fringes.size());
    EXPECT_DOUBLE_EQ(1.0, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[1].GetNumber());
    EXPECT_DOUBLE_EQ(3.0, doc.Fringes[2].GetNumber());
}

TEST(AllCommands, DeleteSegmentsCommand_OrderIndependence) {
    CDigitInfo doc; doc.Fringes.clear();
    for (int i = 0; i < 5; ++i) {
        CFringeSegment s(i + 1.0, i);
        s.AddPoint(CDPoint(i*10.0, i*10.0));
        doc.Fringes.push_back(s);
    }

    // Delete in non-sorted order
    std::vector<size_t> toRemove = {3, 0, 2};
    DeleteSegmentsCommand del(doc, toRemove);
    del.Execute();
    EXPECT_EQ(2u, doc.Fringes.size());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[0].GetNumber());
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[1].GetNumber());

    del.Undo();
    EXPECT_EQ(5u, doc.Fringes.size());
    for (int i = 0; i < 5; ++i) {
        EXPECT_DOUBLE_EQ((i + 1.0), doc.Fringes[i].GetNumber());
    }
}

// ===== AddDotCommand / RemoveLastDotCommand Tests =====

TEST(AllCommands, AddDotCommand_InsertsAtMiddle) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0, 0);
    seg.AddPoint(CDPoint(0,0));
    seg.AddPoint(CDPoint(10,10));
    doc.Fringes.push_back(seg);

    AddDotCommand add(&doc, 0, 1, CDPoint(5,5));
    add.Execute();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(5.0, doc.Fringes[0].GetPoint(1).x);

    add.Undo();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(1).x);
}

TEST(AllCommands, RemoveLastDotCommand_RemovesAndRestores) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment seg(0.0, 0);
    seg.AddPoint(CDPoint(1,1));
    seg.AddPoint(CDPoint(2,2));
    seg.AddPoint(CDPoint(3,3));
    doc.Fringes.push_back(seg);

    RemoveLastDotCommand rem(&doc, 0);
    rem.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(2.0, doc.Fringes[0].GetPoint(1).x);

    rem.Undo();
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(3.0, doc.Fringes[0].GetPoint(2).x);
}

// ===== Command Composition Tests =====

TEST(AllCommands, CommandSequence_CreateExtendDelete) {
    CDigitInfo doc; doc.Fringes.clear();
    
    std::vector<CDPoint> pts = { CDPoint(0,0) };
    CreateSegmentCommand create(doc, pts, 1.0);
    create.Execute();
    EXPECT_EQ(1u, doc.Fringes.size());

    ExtendSegmentCommand extend(doc, 0, false, CDPoint(10,10));
    extend.Execute();
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());

    std::vector<size_t> toDel = {0};
    DeleteSegmentsCommand del(doc, toDel);
    del.Execute();
    EXPECT_EQ(0u, doc.Fringes.size());

    // Undo in reverse order
    del.Undo();
    EXPECT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(2, doc.Fringes[0].GetPointCount());

    extend.Undo();
    EXPECT_EQ(1, doc.Fringes[0].GetPointCount());

    create.Undo();
    EXPECT_EQ(0u, doc.Fringes.size());
}

TEST(AllCommands, CommandSequence_ConnectThenSplit) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment A(1.0, 0), B(2.0, 1);
    for (int i = 0; i < 3; ++i) A.AddPoint(CDPoint(i, i));
    for (int i = 0; i < 3; ++i) B.AddPoint(CDPoint(i+10, i+10));
    doc.Fringes.push_back(A);
    doc.Fringes.push_back(B);

    ConnectSegmentsCommand conn(doc, 0, true, 1, false);
    conn.Execute();
    ASSERT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(6, doc.Fringes[0].GetPointCount());

    // Split merged segment back
    SplitSegmentCommand split(doc, 0, 3);
    split.Execute();
    EXPECT_EQ(2u, doc.Fringes.size());
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_EQ(3, doc.Fringes[1].GetPointCount());

    split.Undo();
    EXPECT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(6, doc.Fringes[0].GetPointCount());

    conn.Undo();
    EXPECT_EQ(2u, doc.Fringes.size());
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());
    EXPECT_EQ(3, doc.Fringes[1].GetPointCount());
}

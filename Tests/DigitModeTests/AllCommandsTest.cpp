#include "stdafx.h"
#include "gtest/gtest.h"

#include "DigitMode/Commands/AllCommands.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/CFringeSegment.h"

using namespace DigitMode;

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
    EXPECT_EQ(5, doc.Fringes[0].GetPointCount());
    EXPECT_DOUBLE_EQ(4.0, doc.Fringes[0].GetPoint(4).x);
}

TEST(AllCommands, ConnectSegmentsCommand_AllJoinModes) {
    // Prepare baseline A and B
    auto makeAB = [](CDigitInfo& doc, CFringeSegment& A, CFringeSegment& B){
        doc.Fringes.clear();
        doc.Fringes.push_back(A);
        doc.Fringes.push_back(B);
    };

    // Create A and B with distinct points
    CFringeSegment A(0.0,0); A.AddPoint(CDPoint(0,0)); A.AddPoint(CDPoint(1,1));
    CFringeSegment B(1.0,1); B.AddPoint(CDPoint(10,10)); B.AddPoint(CDPoint(11,11));

    // Test cases: (endA, endB)
    {
        CDigitInfo doc; makeAB(doc, A, B);
        ConnectSegmentsCommand cmd(doc, 0, true, 1, false); // A end -> B start
        cmd.Execute();
        ASSERT_EQ(1u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(0.0, doc.Fringes[0].GetPoint(0).x);
        EXPECT_DOUBLE_EQ(1.0, doc.Fringes[0].GetPoint(1).x);
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(2).x);
        EXPECT_DOUBLE_EQ(11.0, doc.Fringes[0].GetPoint(3).x);
        cmd.Undo();
        ASSERT_EQ(2u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(0.0, doc.Fringes[0].GetPoint(0).x);
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[1].GetPoint(0).x);
    }

    {
        CDigitInfo doc; makeAB(doc, A, B);
        ConnectSegmentsCommand cmd(doc, 0, true, 1, true); // A end -> B end (B reversed)
        cmd.Execute();
        ASSERT_EQ(1u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(11.0, doc.Fringes[0].GetPoint(2).x);
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(3).x);
        cmd.Undo();
        ASSERT_EQ(2u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[1].GetPoint(0).x);
    }

    {
        CDigitInfo doc; makeAB(doc, A, B);
        ConnectSegmentsCommand cmd(doc, 0, false, 1, false); // A start <- B start (insert at start)
        cmd.Execute();
        ASSERT_EQ(1u, doc.Fringes.size());
        // After insert at start, B points should precede A points
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(0).x);
        EXPECT_DOUBLE_EQ(11.0, doc.Fringes[0].GetPoint(1).x);
        EXPECT_DOUBLE_EQ(0.0, doc.Fringes[0].GetPoint(2).x);
        cmd.Undo();
        ASSERT_EQ(2u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[1].GetPoint(0).x);
    }

    {
        CDigitInfo doc; makeAB(doc, A, B);
        ConnectSegmentsCommand cmd(doc, 0, false, 1, true); // A start <- B end (insert reversed at start)
        cmd.Execute();
        ASSERT_EQ(1u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(11.0, doc.Fringes[0].GetPoint(0).x);
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[0].GetPoint(1).x);
        EXPECT_DOUBLE_EQ(0.0, doc.Fringes[0].GetPoint(2).x);
        cmd.Undo();
        ASSERT_EQ(2u, doc.Fringes.size());
        EXPECT_DOUBLE_EQ(10.0, doc.Fringes[1].GetPoint(0).x);
    }
}

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

TEST(AllCommands, ConnectSegmentsCommand_MergeAndUndo) {
    CDigitInfo doc; doc.Fringes.clear();
    CFringeSegment A(0.0,0);
    A.AddPoint(CDPoint(0.0,0.0));
    A.AddPoint(CDPoint(1.0,1.0));
    CFringeSegment B(1.0,1);
    B.AddPoint(CDPoint(2.0,2.0));
    doc.Fringes.push_back(A);
    doc.Fringes.push_back(B);

    std::vector<CDPoint> aPts, bPts;
    for (int i=0;i<A.GetPointCount();++i) aPts.push_back(A.GetPoint(i));
    for (int i=0;i<B.GetPointCount();++i) bPts.push_back(B.GetPoint(i));

    ConnectSegmentsCommand conn(doc, 0, false, 1, false);
    conn.Execute();
    EXPECT_EQ(1u, doc.Fringes.size());
    EXPECT_EQ(3, doc.Fringes[0].GetPointCount());

    conn.Undo();
    EXPECT_EQ(2u, doc.Fringes.size());
    // Verify points restored for first segment
    EXPECT_EQ((int)aPts.size(), doc.Fringes[0].GetPointCount());
    for (size_t i=0;i<aPts.size();++i) EXPECT_DOUBLE_EQ(aPts[i].x, doc.Fringes[0].GetPoint((int)i).x);
    // Verify points restored for second segment
    EXPECT_EQ((int)bPts.size(), doc.Fringes[1].GetPointCount());
    for (size_t i=0;i<bPts.size();++i) EXPECT_DOUBLE_EQ(bPts[i].x, doc.Fringes[1].GetPoint((int)i).x);
}

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

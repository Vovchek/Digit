/// <summary>
/// Test suite exposing the FATAL BUG in CalcContour's classification logic
/// 
/// THE BUG: Classification checks if point from contour A is inside contour B
/// to determine EXTERNAL vs INTERNAL. This is COMPLETELY WRONG because:
/// 
/// 1. Two non-overlapping EXTERNAL shapes → both should be EXTERNAL
///    But if point from A is not inside B's bounds, one gets marked INTERNAL!
/// 
/// 2. Degenerate contours (1-2 points) → isInside() returns wrong results
///    A 1-point "contour" can never contain another contour's point
///    → Second contour gets marked INTERNAL even when both are EXTERNAL
/// 
/// 3. The logic assumes: "if not inside any other → EXTERNAL"
///    This is BACKWARDS! Should be based on SOURCE SHAPE TypeLimits, not spatial containment!
/// 
/// CORRECT LOGIC: Preserve the TypeLimits from the SOURCE shapes, don't re-classify
/// based on spatial relationships between resulting contours!
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/CalcContour.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYBrokenLine.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Include/Int_Cons.h"
#include <cmath>

// ============================================================================
// Test Fixture for Classification Bug
// ============================================================================

class CalcContourClassificationBugTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    void SetUp() override {
    }

    void TearDown() override {
    }

    // Helper to count contours by type
    struct ContourCounts {
        int total = 0;
        int external = 0;
        int internal = 0;
        int unknown = 0;
    };
    
    ContourCounts CountContourTypes(CArrayXYPolygon& contours) {
        ContourCounts counts;
        counts.total = contours.GetSize();
        
        for (int i = 0; i < contours.GetSize(); i++) {
            int type = contours[i].GetTypeLimits();
            if (type == EXTERNAL) {
                counts.external++;
            } else if (type == INTERNAL) {
                counts.internal++;
            } else {
                counts.unknown++;
            }
        }
        
        return counts;
    }
};

// ============================================================================
// FATAL BUG #1: Two Non-Overlapping EXTERNAL Shapes
// ============================================================================

TEST_F(CalcContourClassificationBugTest, FATAL_TwoNonOverlappingEXTERNAL_BothShouldBeEXTERNAL) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL ellipses far apart (no overlap)
    XYEllipse ellipse1(10.0, 10.0, -50.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.0, 10.0,  50.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    auto counts = CountContourTypes(arrCont);
    
    // BUG: One gets marked INTERNAL because its point is not inside the other
    // CORRECT: Both should be EXTERNAL (they came from EXTERNAL shapes!)
    
    EXPECT_EQ(counts.total, 2) << "Should produce 2 contours";
    EXPECT_EQ(counts.external, 2) << "FATAL BUG: Both should be EXTERNAL, but got " 
                                  << counts.external << " EXTERNAL and " 
                                  << counts.internal << " INTERNAL";
    EXPECT_EQ(counts.internal, 0) << "Should have NO INTERNAL contours";
}

TEST_F(CalcContourClassificationBugTest, FATAL_TwoNonOverlappingRects_BothShouldBeEXTERNAL) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL rectangles far apart
    XYRect rect1(8.0, 6.0, -30.0, 0.0, 0.0, EXTERNAL);
    XYRect rect2(8.0, 6.0,  30.0, 0.0, 0.0, EXTERNAL);
    
    arrRect.Add(rect1);
    arrRect.Add(rect2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    auto counts = CountContourTypes(arrCont);
    
    EXPECT_EQ(counts.total, 2);
    EXPECT_EQ(counts.external, 2) << "FATAL: Both rectangles are EXTERNAL sources";
    EXPECT_EQ(counts.internal, 0);
}

// ============================================================================
// FATAL BUG #2: Degenerate Contour Classification
// ============================================================================

TEST_F(CalcContourClassificationBugTest, FATAL_DegenerateContour_DoesNotMakeOthersINTERNAL) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Recreate YOUR EXACT BUG SCENARIO:
    // Two slightly different EXTERNAL ellipses
    // First gets reduced to 1 point by isPupil
    // Second is valid but gets marked INTERNAL!
    
    XYEllipse ellipse1(233.891, 233.891, 280.233, 267.611, 0.0, EXTERNAL);
    XYEllipse ellipse2(233.423218, 233.423218, 279.765218, 267.611, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    auto counts = CountContourTypes(arrCont);
    
    // THE BUG:
    // - Ellipse1 becomes 1-point contour
    // - Point from Ellipse2 is NOT inside 1-point "contour"
    // - Therefore Ellipse2 contour marked INTERNAL
    // - WRONG! Both source shapes are EXTERNAL!
    
    EXPECT_GT(counts.total, 0) << "Should produce at least some contours";
    
    // All resulting contours should be EXTERNAL (source shapes are EXTERNAL)
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_EQ(arrCont[i].GetTypeLimits(), EXTERNAL)
            << "FATAL BUG: Contour " << i << " marked INTERNAL but source was EXTERNAL!"
            << " Contour has " << arrCont[i].GetSize() << " points";
    }
}

TEST_F(CalcContourClassificationBugTest, FATAL_SinglePointContour_DoesNotAffectOthers) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Force scenario where one contour is degenerate
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.05, 8.05, 0.5, 0.3, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // If we get contours, they should ALL be EXTERNAL
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_EQ(arrCont[i].GetTypeLimits(), EXTERNAL)
            << "Contour " << i << " should be EXTERNAL (source shape is EXTERNAL)";
    }
}

// ============================================================================
// FATAL BUG #3: Multiple EXTERNAL Shapes
// ============================================================================

TEST_F(CalcContourClassificationBugTest, FATAL_ThreeEXTERNALShapes_AllShouldBeEXTERNAL) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Three EXTERNAL shapes in a row
    XYEllipse ellipse1(5.0, 5.0, -20.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(5.0, 5.0,   0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse3(5.0, 5.0,  20.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    arrEll.Add(ellipse3);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    auto counts = CountContourTypes(arrCont);
    
    EXPECT_EQ(counts.total, 3) << "Should produce 3 contours";
    EXPECT_EQ(counts.external, 3) << "FATAL: ALL should be EXTERNAL";
    EXPECT_EQ(counts.internal, 0) << "Should have NO INTERNAL";
}

TEST_F(CalcContourClassificationBugTest, FATAL_MixedShapeTypes_PreserveSourceTypes) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Mix of EXTERNAL shapes
    arrEll.Add(XYEllipse(8.0, 6.0, -15.0, 0.0, 0.0, EXTERNAL));
    arrRect.Add(XYRect(7.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL));
    arrEll.Add(XYEllipse(8.0, 6.0, 15.0, 0.0, 0.0, EXTERNAL));
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    auto counts = CountContourTypes(arrCont);
    
    EXPECT_EQ(counts.external, counts.total) 
        << "ALL source shapes are EXTERNAL, so ALL contours should be EXTERNAL";
}

// ============================================================================
// FATAL BUG #4: Current Logic is Backwards
// ============================================================================

TEST_F(CalcContourClassificationBugTest, FATAL_LogicIsBackwards_ShouldInheritFromSource) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Current logic: "If point not inside any other contour → EXTERNAL"
    // This means it's classifying based on SPATIAL RELATIONSHIP between contours
    // WRONG! Should classify based on SOURCE SHAPE TypeLimits!
    
    // Test case: Two EXTERNAL shapes where one point IS inside other's bounds
    XYEllipse outer(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse inner(5.0, 4.0, 0.0, 0.0, 0.0, EXTERNAL);  // Inside outer
    
    arrEll.Add(outer);
    arrEll.Add(inner);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // BOTH source shapes are EXTERNAL
    // Therefore BOTH resulting contours should be EXTERNAL
    // Even though inner's point IS inside outer's contour!
    
    auto counts = CountContourTypes(arrCont);
    
    // Current buggy logic would mark inner as INTERNAL
    // Correct logic: BOTH are EXTERNAL (from source)
    EXPECT_EQ(counts.external, counts.total)
        << "FATAL: Classification should come from SOURCE shapes, not spatial containment!"
        << " Both sources are EXTERNAL, so both results should be EXTERNAL";
}

// ============================================================================
// FATAL BUG #5: The isInside() Check is Meaningless
// ============================================================================

TEST_F(CalcContourClassificationBugTest, FATAL_isInsideCheck_IsMeaninglessForClassification) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // The current code does:
    // if (ArrCont[i].isInside(P)) → mark as INTERNAL
    // else → mark as EXTERNAL
    //
    // This is NONSENSE! The classification should depend on:
    // - What TYPE was the SOURCE shape? (EXTERNAL or INTERNAL)
    // - NOT whether contours spatially contain each other!
    
    // Proof: Two separate EXTERNAL apertures
    XYRect aperture1(10.0, 8.0, -25.0, 0.0, 0.0, EXTERNAL);
    XYRect aperture2(10.0, 8.0,  25.0, 0.0, 0.0, EXTERNAL);
    
    arrRect.Add(aperture1);
    arrRect.Add(aperture2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 100);
    
    ASSERT_EQ(arrCont.GetSize(), 2);
    
    // Point from aperture1 will NOT be inside aperture2's contour
    // So current logic marks one as INTERNAL
    // But BOTH apertures are EXTERNAL sources!
    
    EXPECT_EQ(arrCont[0].GetTypeLimits(), EXTERNAL) 
        << "Aperture 1 is EXTERNAL source → contour should be EXTERNAL";
    EXPECT_EQ(arrCont[1].GetTypeLimits(), EXTERNAL) 
        << "Aperture 2 is EXTERNAL source → contour should be EXTERNAL";
}

// ============================================================================
// CORRECT BEHAVIOR: Holes (INTERNAL) Inside EXTERNAL
// ============================================================================

TEST_F(CalcContourClassificationBugTest, CORRECT_ExternalWithInternalHole_PreservesTypes) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // THIS is the correct use case for EXTERNAL/INTERNAL:
    // Outer aperture (EXTERNAL) with obstruction hole (INTERNAL)
    
    XYEllipse outer(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);   // Aperture
    XYEllipse hole(5.0, 4.0, 0.0, 0.0, 0.0, INTERNAL);      // Obstruction
    
    arrEll.Add(outer);
    arrEll.Add(hole);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // Should get 2 contours:
    // 1. Outer aperture → EXTERNAL (from source)
    // 2. Hole → INTERNAL (from source)
    
    auto counts = CountContourTypes(arrCont);
    
    EXPECT_GE(counts.total, 2);
    EXPECT_GE(counts.external, 1) << "Should have EXTERNAL contour from outer aperture";
    EXPECT_GE(counts.internal, 1) << "Should have INTERNAL contour from hole";
}

// ============================================================================
// THE ROOT CAUSE
// ============================================================================

TEST_F(CalcContourClassificationBugTest, ROOT_CAUSE_ClassificationLogicIsFundamentallyWrong) {
    // THE CURRENT CODE (WRONG):
    // -------------------------
    // for (iElm = 0; iElm < NCont; iElm++) {
    //     P = ArrCont[iElm][0];
    //     isInsideAny = false;
    //     for (i = 0; i < NCont; i++) {
    //         if (i == iElm) continue;
    //         if (ArrCont[i].isInside(P)) {
    //             isInsideAny = true;
    //             break;
    //         }
    //     }
    //     ArrCont[iElm].SetTypeLimits(isInsideAny ? INTERNAL : EXTERNAL);
    // }
    //
    // PROBLEMS:
    // 1. Classifies based on SPATIAL CONTAINMENT between resulting contours
    // 2. Ignores the TypeLimits of SOURCE shapes
    // 3. Assumes "inside another → INTERNAL, not inside → EXTERNAL"
    // 4. Breaks with degenerate contours (1-2 points)
    // 5. Breaks with non-overlapping EXTERNAL shapes
    //
    // CORRECT APPROACH:
    // -----------------
    // The TypeLimits should be INHERITED from the source shape!
    // If an ellipse has TypeLimits=EXTERNAL, its contour should be EXTERNAL
    // If an ellipse has TypeLimits=INTERNAL, its contour should be INTERNAL
    // Spatial relationships between contours are IRRELEVANT!
    
    // This test documents the issue
    FAIL() << "Classification logic in CalcContour is fundamentally broken.\n"
           << "It classifies contours based on spatial containment instead of source shape types.\n"
           << "FIX: Preserve TypeLimits from source shapes, don't re-classify based on contour relationships!";
}

// ============================================================================
// REGRESSION TEST: Your Exact Bug Scenario
// ============================================================================

TEST_F(CalcContourClassificationBugTest, YOUR_BUG_TwoSlightlyDifferentEllipses_BothEXTERNAL) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // YOUR EXACT DATA:
    // 1) Ax=233.891, By=233.891, Xc=280.233, Yc=267.611
    // 2) Ax=233.423218, By=233.423218, Xc=279.765218, Yc=267.611
    
    XYEllipse ellipse1(233.891, 233.891, 280.233, 267.611, 0.0, EXTERNAL);
    XYEllipse ellipse2(233.423218, 233.423218, 279.765218, 267.611, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // THE BUG YOU DESCRIBED:
    // - Ellipse1: isPupil kills 499/500 points → 1-point contour
    // - Ellipse2: Valid contour but point not inside 1-point contour
    // - Result: Ellipse2 marked INTERNAL (WRONG!)
    // - CORRECT: Both should be EXTERNAL (source shapes are EXTERNAL)
    
    auto counts = CountContourTypes(arrCont);
    
    EXPECT_GT(counts.total, 0) << "Should produce some contours";
    
    // CRITICAL: All contours should be EXTERNAL (both source ellipses are EXTERNAL)
    EXPECT_EQ(counts.external, counts.total) 
        << "YOUR BUG: Got " << counts.external << " EXTERNAL and " 
        << counts.internal << " INTERNAL, but BOTH source ellipses are EXTERNAL!";
    
    EXPECT_EQ(counts.internal, 0) 
        << "Should have NO INTERNAL contours (no source shapes are INTERNAL)";
}

// ============================================================================
// PROOF: The Logic Makes Valid Shapes Invisible
// ============================================================================

TEST_F(CalcContourClassificationBugTest, PROOF_ValidShapeBecomesInvisible_DueToWrongClassification) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures
    // App needs BOTH to determine valid pupil region
    // But bug makes second one INTERNAL → wrong visibility calculations!
    
    XYEllipse aperture1(15.0, 12.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse aperture2(14.8, 11.9, 1.0, 0.5, 0.0, EXTERNAL);
    
    arrEll.Add(aperture1);
    arrEll.Add(aperture2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 500);
    
    // If second aperture gets marked INTERNAL:
    // - It will be treated as obstruction instead of aperture
    // - Visibility calculations will be WRONG
    // - App will crash or produce garbage results
    
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_EQ(arrCont[i].GetTypeLimits(), EXTERNAL)
            << "PROOF OF BUG: Contour " << i << " marked " 
            << (arrCont[i].GetTypeLimits() == INTERNAL ? "INTERNAL" : "other")
            << " but source is EXTERNAL aperture!"
            << " This breaks visibility calculations!";
    }
}

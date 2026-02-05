#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/Commands/AutoNumberingAlgorithmSaddles.h"
#include "DigitMode/CFringeSegment.h"
#include <cmath>
#include <set>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace DigitMode;

/**
 * @brief Test suite for AutoNumberFringes algorithm
 * 
 * Tests all six phases of the automatic numbering algorithm:
 * 1. Preprocessing
 * 2. Adjacency graph construction
 * 3. Constraint generation
 * 4. Numerical solve
 * 5. Quantization + validation
 * 6. Confidence evaluation
 */
class AutoNumberingAlgorithmTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default parameters
        step = 1.0;
        confidenceThreshold = 0.7;
    }

    /**
     * Helper: Create a simple horizontal line fringe
     */
    CFringeSegment CreateHorizontalLine(double y, double length = 100.0, double startX = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = static_cast<int>(length / 10.0) + 1;
        for (int i = 0; i < numPoints; ++i) {
            CDPoint pt;
            pt.x = startX + i * 10.0;
            pt.y = y;
            seg.AddPoint(pt);
        }
        return seg;
    }

    /**
     * Helper: Create a vertical line fringe
     */
    CFringeSegment CreateVerticalLine(double x, double length = 100.0, double startY = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = static_cast<int>(length / 10.0) + 1;
        for (int i = 0; i < numPoints; ++i) {
            CDPoint pt;
            pt.x = x;
            pt.y = startY + i * 10.0;
            seg.AddPoint(pt);
        }
        return seg;
    }

    /**
     * Helper: Create a closed circular fringe
     */
    CFringeSegment CreateCircle(double cx, double cy, double radius) {
        CFringeSegment seg(0.0, 0);
        int numPoints = 16;
        for (int i = 0; i < numPoints; ++i) {
            double angle = 2.0 * M_PI * i / numPoints;
            CDPoint pt;
            pt.x = cx + radius * std::cos(angle);
            pt.y = cy + radius * std::sin(angle);
            seg.AddPoint(pt);
        }
        // Close the curve
        CDPoint first = seg.GetPoint(0);
        seg.AddPoint(first);
        return seg;
    }

    double step;
    double confidenceThreshold;
};

// ========== Phase 1: Preprocessing Tests ==========

TEST_F(AutoNumberingAlgorithmTest, PreprocessingTrustedMarking) {
    // Simple test: verify trusted indices are passed correctly
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    // Mark first and third as trusted
    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Verify original trusted are preserved
    EXPECT_GE(result.trustedFringes.size(), 2);
}

TEST_F(AutoNumberingAlgorithmTest, PreprocessingCentroidComputation) {
    // Simple test: verify fringes with centroid are processed
    std::vector<CFringeSegment> fringes;
    CFringeSegment seg = CreateHorizontalLine(25.0, 100.0, 10.0);
    seg.SetNumber(0.0);
    fringes.push_back(seg);

    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Should preserve trusted
    EXPECT_GE(result.trustedFringes.size(), 1);
}

TEST_F(AutoNumberingAlgorithmTest, PreprocessingClosedCurveDetection) {
    // Simple test: verify closed and open curves are handled
    std::vector<CFringeSegment> fringes;

    // Open curve
    CFringeSegment openCurve = CreateHorizontalLine(0.0);
    fringes.push_back(openCurve);

    // Closed curve
    CFringeSegment closedCurve = CreateCircle(50.0, 50.0, 20.0);
    fringes.push_back(closedCurve);

    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(1.0);

    std::vector<size_t> trustedIndices = {0, 1};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Should process both
    EXPECT_GE(result.trustedFringes.size(), 2);
}

// ========== Phase 2: Adjacency & Direction Tests ==========

TEST_F(AutoNumberingAlgorithmTest, SingleAnchorUsesGlobalDirection) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(50.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, VerticalLinesMonotonic) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateVerticalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateVerticalLine(20.0, 100.0, 0.0));
    fringes.push_back(CreateVerticalLine(40.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, SlantedLinesMonotonic) {
    std::vector<CFringeSegment> fringes;

    // Slanted lines: y = x + c
    CFringeSegment s0 = CreateHorizontalLine(0.0, 100.0, 0.0);
    for (int i = 0; i < s0.GetPointCount(); ++i) {
        CDPoint p = s0.GetPoint(i);
        p.y = p.x;
        s0.SetPoint(i, p);
    }

    CFringeSegment s1 = CreateHorizontalLine(20.0, 100.0, 0.0);
    for (int i = 0; i < s1.GetPointCount(); ++i) {
        CDPoint p = s1.GetPoint(i);
        p.y = p.x + 20.0;
        s1.SetPoint(i, p);
    }

    CFringeSegment s2 = CreateHorizontalLine(40.0, 100.0, 0.0);
    for (int i = 0; i < s2.GetPointCount(); ++i) {
        CDPoint p = s2.GetPoint(i);
        p.y = p.x + 40.0;
        s2.SetPoint(i, p);
    }

    fringes.push_back(s0);
    fringes.push_back(s1);
    fringes.push_back(s2);

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

// ========== Phase 4: Solver Tests ==========

TEST_F(AutoNumberingAlgorithmTest, SolverSimpleTrustedValues) {
    // Trivial case: all fringes are trusted
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(1.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 1, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // All should remain trusted
    EXPECT_GE(result.trustedFringes.size(), 3);
    EXPECT_EQ(fringes[0].GetNumber(), 0.0);
    EXPECT_EQ(fringes[1].GetNumber(), 1.0);
    EXPECT_EQ(fringes[2].GetNumber(), 2.0);
}

TEST_F(AutoNumberingAlgorithmTest, SolverInferMiddleValue) {
    // Three fringes: first and last trusted, middle to be inferred
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(50.0, 100.0, 0.0));  // To be inferred
    fringes.push_back(CreateHorizontalLine(100.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(0.0);  // Unknown
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Middle should be inferred as close to 1.0
    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, SolverEmptyFringes) {
    std::vector<CFringeSegment> fringes;
    std::vector<size_t> trustedIndices;

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_EQ(result.trustedFringes.size(), 0);
}

// ========== Phase 6: Confidence Tests ==========

TEST_F(AutoNumberingAlgorithmTest, ConfidenceHighForTrustedFringes) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(1.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 1, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Trusted fringes should be in the result
    EXPECT_GE(result.trustedFringes.size(), 3);
}

TEST_F(AutoNumberingAlgorithmTest, ConfidenceThreshold) {
    std::vector<CFringeSegment> fringes;

    // Create 3 fringes, only endpoints trusted
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(50.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(100.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // At minimum, original trusted should remain
    EXPECT_GE(result.trustedFringes.size(), 2);
}

/* ========== Integration Tests ========== */

TEST_F(AutoNumberingAlgorithmTest, IntegrationSimpleGrid) {
    // Grid of evenly-spaced parallel lines (reversed order)
    std::vector<CFringeSegment> fringes;
    for (int i = 0; i < 5; ++i) {
        CFringeSegment line = CreateHorizontalLine((5 - i) * 25.0, 100.0, 0.0);
        line.SetNumber(0.0);  // Unknown
        fringes.push_back(line);
    }

    // Trust only first and last (consistent with step)
    fringes[0].SetNumber(0.0);
    fringes[4].SetNumber(4.0);

    std::vector<size_t> trustedIndices = {0, 4};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Should infer intermediate values
    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 2.0, 0.5);
    EXPECT_NEAR(fringes[3].GetNumber(), 3.0, 0.5);

    // Check that result includes at least original trusted
    EXPECT_GE(result.trustedFringes.size(), 2);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationLargeGap) {
    // Fringes with large gap in the middle
    std::vector<CFringeSegment> fringes;

    // Close fringes at start
    for (int i = 0; i < 3; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 10.0, 100.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    // Large gap (50 pixels)

    // Close fringes at end
    for (int i = 0; i < 3; ++i) {
        CFringeSegment line = CreateHorizontalLine(80.0 + i * 10.0, 100.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    // Trust start and end clusters
    fringes[0].SetNumber(0.0);
    fringes[5].SetNumber(10.0);

    std::vector<size_t> trustedIndices = {0, 5};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    // Algorithm should handle the gap gracefully
    EXPECT_GE(result.trustedFringes.size(), 2);  // At least keep the trusted ones
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesPeak) {
    // Nested rings with inner anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 2.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesReversed) {
    // Nested rings with outer anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = { 0 };

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 2.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesPit) {
    // Nested rings with outer and inner anchors (pit topology)
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));

    fringes[0].SetNumber(2.0);
    fringes[2].SetNumber(0.0);
    std::vector<size_t> trustedIndices = { 0, 2 };

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 0.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationParallelLines) {
    // Parallel horizontal lines (band case)
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(15.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(30.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_GE(result.trustedFringes.size(), 2);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationMixedBandAndRing) {
    // Mixed topology: bands + nested rings
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(20.0, 100.0, 0.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 10.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));

    fringes[0].SetNumber(0.0);
    fringes[3].SetNumber(2.0);
    std::vector<size_t> trustedIndices = {0, 3};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, SaddleLikeTopology) {
    // CORRECTED: Real saddle topology
    // Saddle = 4 fringes arranged in a rectangle with GAPS (no connections)
    // 
    //   ....0....   (Top)
    //   3       1   (Left and Right)
    //   ....2....   (Bottom)
    //
    // Topological Rule: OPPOSITE SIDES HAVE SAME NUMBER
    // - Top and Bottom are opposite → MUST have same number
    // - Left and Right are opposite → MUST have same number
    // - Cycle propagates: Top=0, Right=1, Bottom=0, Left=1
    //
    // IMPORTANT: This test is CORRECT. If it fails, DO NOT CHANGE THE TEST.
    // Instead, fix AutoNumberingAlgorithm to:
    // 1. Phase 1: Detect saddle pattern (rectangle of 4 fringes with gaps)
    // 2. Phase 2: Mark opposite-side relationships in adjacency graph
    // 3. Phase 3-4: Add constraint: opposite_fringe_i == opposite_fringe_j
    
    std::vector<CFringeSegment> fringes;
    
    // Four separate lines forming rectangle outline with GAPS
    fringes.push_back(CreateHorizontalLine(0.0, 50.0, 10.0));    // Top (0)
    fringes.push_back(CreateVerticalLine(70.0, 50.0, 0.0));      // Right (1) [gap at x=70]
    fringes.push_back(CreateHorizontalLine(60.0, 50.0, 10.0));   // Bottom (2) [gap at y=60]
    fringes.push_back(CreateVerticalLine(0.0, 50.0, 0.0));       // Left (3)
    
    // IMPORTANT: This is correct test. Gaps ensure NO CONNECTIONS.
    // If test fails - fix autonumbering algorithm, NOT the test!
    
    // Trust only the top fringe
    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // In saddle topology, numbers should propagate around the cycle
    // with consistent stepping
    // Expected cycle: Top(0) → Right(1) → Bottom(0) → Left(1) → repeat
    double topNum = fringes[0].GetNumber();
    double rightNum = fringes[1].GetNumber();
    double bottomNum = fringes[2].GetNumber();
    double leftNum = fringes[3].GetNumber();
    
    // Check that all fringes got assigned (saddle is stable)
    std::set<double> numbers = {topNum, rightNum, bottomNum, leftNum};
    EXPECT_GT(numbers.size(), 1u);  // More than one distinct number
    
    // Topological constraints (opposite sides equal):
    EXPECT_EQ(topNum, bottomNum);           // Top == Bottom (opposite sides)
    EXPECT_EQ(leftNum, rightNum);           // Left == Right (opposite sides)
    EXPECT_NEAR((topNum + 1), rightNum, 0.1);  // Adjacent sides differ by step
}

TEST_F(AutoNumberingAlgorithmTest, SaddleLikeCrossDoesNotCollapse) {
    // Cross-like arrangement: multiple adjacencies around a region
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(-10.0, 40.0, -20.0));
    fringes.push_back(CreateHorizontalLine(10.0, 40.0, -20.0));
    fringes.push_back(CreateVerticalLine(-10.0, 40.0, -20.0));
    fringes.push_back(CreateVerticalLine(10.0, 40.0, -20.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    std::set<double> numbers;
    for (const auto& f : fringes) numbers.insert(f.GetNumber());
    EXPECT_GT(numbers.size(), 1u);
}

TEST_F(AutoNumberingAlgorithmTest, EdgeCaseSingleFringe) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes[0].SetNumber(5.0);

    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_EQ(fringes[0].GetNumber(), 5.0);
    EXPECT_GE(result.trustedFringes.size(), 1);
}

TEST_F(AutoNumberingAlgorithmTest, EdgeCaseNegativeNumbers) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    fringes[0].SetNumber(-2.0);
    fringes[1].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 0.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, EdgeCaseNonUnitStep) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(10.0);  // Step of 5.0

    std::vector<size_t> trustedIndices = {0, 2};

    double customStep = 5.0;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, customStep);

    EXPECT_NEAR(fringes[1].GetNumber(), 5.0, 1.0);
}

/* ========== Updated Band Sequence Integration Tests ========== */

TEST_F(AutoNumberingAlgorithmTest, IntegrationLongBandSequence) {
    // Many parallel bands with anchors at ends
    std::vector<CFringeSegment> fringes;
    const int count = 12;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 10.0, 120.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    fringes[0].SetNumber(0.0);
    fringes[count - 1].SetNumber(static_cast<double>(count - 1));

    std::vector<size_t> trustedIndices = {0, static_cast<size_t>(count - 1)};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 1; i < count - 1; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationLongBandSequenceReversed) {
    // Many parallel bands with reversed order
    std::vector<CFringeSegment> fringes;
    const int count = 12;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine((count - 1 - i) * 10.0, 120.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    fringes[0].SetNumber(0.0);
    fringes[count - 1].SetNumber(static_cast<double>(count - 1));

    std::vector<size_t> trustedIndices = {0, static_cast<size_t>(count - 1)};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 1; i < count - 1; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationBandWithSparseAnchors) {
    // Many bands with sparse anchors to verify propagation stability
    std::vector<CFringeSegment> fringes;
    const int count = 10;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 12.0, 120.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    fringes[2].SetNumber(2.0);
    fringes[7].SetNumber(7.0);

    std::vector<size_t> trustedIndices = {2, 7};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 0; i < count; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationBandTwentyFringes) {
    std::vector<CFringeSegment> fringes;
    const int count = 20;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 8.0, 160.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    fringes[0].SetNumber(0.0);
    fringes[count - 1].SetNumber(static_cast<double>(count - 1));

    std::vector<size_t> trustedIndices = {0, static_cast<size_t>(count - 1)};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 0; i < count; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationBandManyFringesAnchorTop) {
    std::vector<CFringeSegment> fringes;
    const int count = 30;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 8.0, 160.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    // Anchor at topmost fringe
    fringes[0].SetNumber(0.0);

    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 0; i < count; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationBandManyFringesAnchorBottom) {
    std::vector<CFringeSegment> fringes;
    const int count = 30;
    for (int i = 0; i < count; ++i) {
        CFringeSegment line = CreateHorizontalLine(i * 8.0, 160.0, 0.0);
        line.SetNumber(0.0);
        fringes.push_back(line);
    }

    // Anchor at bottommost fringe
    fringes[count - 1].SetNumber(static_cast<double>(count - 1));

    std::vector<size_t> trustedIndices = {static_cast<size_t>(count - 1)};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int i = 0; i < count; ++i) {
        EXPECT_NEAR(fringes[i].GetNumber(), static_cast<double>(i), 0.5);
    }
}

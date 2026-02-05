#include "stdafx.h"
#include "gtest/gtest.h"
#include "DigitMode/Commands/AutoNumberingAlgorithm.h"
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
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    // Mark first and third as trusted
    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = {0, 2};

    // Process
    std::vector<FringeNode> nodes;
    for (size_t i = 0; i < fringes.size(); ++i) {
        FringeNode node;
        node.index = i;
        node.isTrusted = (std::find(trustedIndices.begin(), trustedIndices.end(), i) != trustedIndices.end());
        node.knownValue = fringes[i].GetNumber();
        node.centroid_x = 50.0;
        node.centroid_y = (i * 50.0);
        node.isClosed = false;
        node.confidence = 0.0;
        nodes.push_back(node);
    }

    // Verify trusted marking
    EXPECT_TRUE(nodes[0].isTrusted);
    EXPECT_FALSE(nodes[1].isTrusted);
    EXPECT_TRUE(nodes[2].isTrusted);
}

TEST_F(AutoNumberingAlgorithmTest, PreprocessingCentroidComputation) {
    std::vector<CFringeSegment> fringes;
    CFringeSegment seg = CreateHorizontalLine(25.0, 100.0, 10.0);
    seg.SetNumber(0.0);
    fringes.push_back(seg);

    std::vector<size_t> trustedIndices;

    // Centroid should be approximately (60, 25) for a line from (10,25) to (110,25)
    FringeNode node;
    node.index = 0;
    double sumX = 0.0, sumY = 0.0;
    for (int p = 0; p < seg.GetPointCount(); ++p) {
        CDPoint pt = seg.GetPoint(p);
        sumX += pt.x;
        sumY += pt.y;
    }
    node.centroid_x = sumX / seg.GetPointCount();
    node.centroid_y = sumY / seg.GetPointCount();

    EXPECT_NEAR(node.centroid_x, 60.0, 1.0);
    EXPECT_NEAR(node.centroid_y, 25.0, 1.0);
}

TEST_F(AutoNumberingAlgorithmTest, PreprocessingClosedCurveDetection) {
    std::vector<CFringeSegment> fringes;

    // Open curve
    CFringeSegment openCurve = CreateHorizontalLine(0.0);
    fringes.push_back(openCurve);

    // Closed curve
    CFringeSegment closedCurve = CreateCircle(50.0, 50.0, 20.0);
    fringes.push_back(closedCurve);

    FringeNode openNode;
    openNode.isClosed = false;
    if (openCurve.GetPointCount() >= 3) {
        CDPoint first = openCurve.GetPoint(0);
        CDPoint last = openCurve.GetPoint(openCurve.GetPointCount() - 1);
        double dist = std::sqrt((first.x - last.x) * (first.x - last.x) +
                               (first.y - last.y) * (first.y - last.y));
        openNode.isClosed = (dist < 5.0);
    }

    FringeNode closedNode;
    closedNode.isClosed = false;
    if (closedCurve.GetPointCount() >= 3) {
        CDPoint first = closedCurve.GetPoint(0);
        CDPoint last = closedCurve.GetPoint(closedCurve.GetPointCount() - 1);
        double dist = std::sqrt((first.x - last.x) * (first.x - last.x) +
                               (first.y - last.y) * (first.y - last.y));
        closedNode.isClosed = (dist < 5.0);
    }

    EXPECT_FALSE(openNode.isClosed);
    EXPECT_TRUE(closedNode.isClosed);
}

// ========== Phase 2: Adjacency & Direction Tests ==========

TEST_F(AutoNumberingAlgorithmTest, SingleAnchorUsesGlobalDirection) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateHorizontalLine(50.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

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
    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

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
    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    // All should remain trusted
    EXPECT_EQ(result.size(), 3);
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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    // Middle should be inferred as close to 1.0
    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, SolverEmptyFringes) {
    std::vector<CFringeSegment> fringes;
    std::vector<size_t> trustedIndices;

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    EXPECT_EQ(result.size(), 0);
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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    // Trusted fringes should be in the result
    EXPECT_EQ(result.size(), 3);
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

    double highThreshold = 0.95;  // Very strict
    auto resultHigh = AutoNumberFringes(fringes, trustedIndices, step, highThreshold);

    // With high threshold, middle fringe may not qualify
    // At minimum, original trusted should remain
    EXPECT_GE(resultHigh.size(), 2);
}

// ========== Integration Tests ==========

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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    // Should infer intermediate values
    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 2.0, 0.5);
    EXPECT_NEAR(fringes[3].GetNumber(), 3.0, 0.5);

    // Check that result includes at least original trusted
    EXPECT_GE(result.size(), 2);
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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    // Algorithm should handle the gap gracefully
    EXPECT_GE(result.size(), 2);  // At least keep the trusted ones
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesPeak) {
    // Nested rings with inner anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {0};

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 2.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesReversed) {
    // Nested rings with inner anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = { 0 };

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    EXPECT_NEAR(fringes[1].GetNumber(), -1.0, 0.5); // defaults to lump
    EXPECT_NEAR(fringes[2].GetNumber(), -2.0, 0.5); // i.e. outer rings lower
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesPit) {
    // Nested rings with inner anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));

    fringes[0].SetNumber(2.0);
    fringes[2].SetNumber(0.0);
    std::vector<size_t> trustedIndices = { 0, 2 };

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

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

    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_GE(result.size(), 2);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationMixedBandAndRing) {
    // CORRECTED: Bands and rings that CONNECT must have the same number
    // Ring must actually TOUCH Band1 (not just be nearby)
    
    std::vector<CFringeSegment> fringes;
    
    // Two horizontal bands
    fringes.push_back(CreateHorizontalLine(0.0, 100.0, 0.0));    // Band 1 at y=0
    fringes.push_back(CreateHorizontalLine(50.0, 100.0, 0.0));   // Band 2 at y=50
    
    // Ring that ACTUALLY INTERSECTS with Band 1
    // Ring centered at (50, 5) with radius 8 → y in [-3, 13]
    // This DOES cross y=0 where Band1 is
    CFringeSegment ring = CreateCircle(50.0, 5.0, 8.0);
    fringes.push_back(ring);
    
    // Trust Band 1 and Band 2
    fringes[0].SetNumber(0.0);  // Band 1
    fringes[1].SetNumber(1.0);  // Band 2 (different from band 1)
    
    std::vector<size_t> trustedIndices = {0, 1};
    
    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);
    
    // Ring CONNECTS to Band 1 (y=0), so it should get number ≈ 0.0
    // NOT number between 0 and 1
    EXPECT_NEAR(fringes[2].GetNumber(), fringes[0].GetNumber(), 0.2);
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
    
    auto result = AutoNumberFringes(fringes, trustedIndices, step, confidenceThreshold);
    
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

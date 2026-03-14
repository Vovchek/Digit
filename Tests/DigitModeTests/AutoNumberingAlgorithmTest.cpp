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
 * @brief Test suite for AutoNumberFringes merging/connectivity logic
 * 
 * Tests the FringesConnected() function and BFS-based merge group creation.
 * Verifies that connected fringes are grouped together and separate fringes
 * remain in different groups.
 */
class AutoNumberingMergingTest : public ::testing::Test {
protected:
    void SetUp() override {
        step = 1.0;
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
     * Helper: Create a line fringe @ arbitrary slant with optional jitter
     */
    CFringeSegment CreateSlantLine(double startX = 0.0, double startY = 0.0,
        double lengthX = 100.0, double lengthY = 100.0,
        double jitter = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = static_cast<int>((std::max)(lengthX, lengthY) / 10.0) + 1;
        double stepX = lengthX / (numPoints - 1);
        double stepY = lengthY / (numPoints - 1);
        for (int i = 0; i < numPoints; ++i) {
            CDPoint pt;
            pt.x = startX + i * stepX + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            pt.y = startY + i * stepY + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
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
    CFringeSegment CreateCircle(double cx, double cy, double radius, double jitter = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = 16;
        for (int i = 0; i < numPoints; ++i) {
            double angle = 2.0 * M_PI * i / numPoints;
            CDPoint pt;
            pt.x = cx + radius * std::cos(angle) + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            pt.y = cy + radius * std::sin(angle) + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            seg.AddPoint(pt);
        }
        // Close the curve
        CDPoint first = seg.GetPoint(0);
        seg.AddPoint(first);
        return seg;
    }

    /**
     * Helper: Check if two fringes ended up in the same merge group
     * by verifying they have the same assigned number
     */
    bool AreMerged(const std::vector<CFringeSegment>& fringes, size_t i, size_t j) {
        return std::abs(fringes[i].GetNumber() - fringes[j].GetNumber()) < 0.01;
    }

    double step;
};

// ========== Merge Connectivity Tests ==========

TEST_F(AutoNumberingMergingTest, TwoHorizontalLinesTouching) {
    // Two horizontal lines touching at endpoints
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 50.0, 0.0);  // 0.0 to 50.0
    CFringeSegment line2 = CreateHorizontalLine(0.0, 50.0, 50.0); // 50.0 to 100.0 (touching)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Both should be merged into the same group
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Touching horizontal lines should be merged";
}

TEST_F(AutoNumberingMergingTest, TwoHorizontalLinesSeparate) {
    // Two horizontal lines with gap
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 50.0, 0.0);   // 0.0 to 50.0
    CFringeSegment line2 = CreateHorizontalLine(0.0, 50.0, 60.0);  // 60.0 to 110.0 (gap of 10)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should remain in separate groups
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Separated horizontal lines should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, TwoVerticalLinesTouching) {
    // Two vertical lines touching at endpoints
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateVerticalLine(0.0, 50.0, 0.0);  // y: 0.0 to 50.0
    CFringeSegment line2 = CreateVerticalLine(0.0, 50.0, 50.0); // y: 50.0 to 100.0 (touching)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Both should be merged
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Touching vertical lines should be merged";
}

TEST_F(AutoNumberingMergingTest, TwoVerticalLinesSeparate) {
    // Two vertical lines with gap
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateVerticalLine(0.0, 50.0, 0.0);   // y: 0.0 to 50.0
    CFringeSegment line2 = CreateVerticalLine(0.0, 50.0, 60.0);  // y: 60.0 to 110.0 (gap)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Separated vertical lines should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, OverlappingHorizontalLines) {
    // Two overlapping horizontal lines
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 60.0, 0.0);  // 0.0 to 60.0
    CFringeSegment line2 = CreateHorizontalLine(0.0, 60.0, 40.0); // 40.0 to 100.0 (overlap 20 units)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (many overlapping points)
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Overlapping lines should be merged";
}

TEST_F(AutoNumberingMergingTest, SlantedLinesTouching) {
    // Two slanted lines (y = x) touching at endpoints
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1(0.0, 0);
    for (int i = 0; i <= 5; ++i) {
        CDPoint pt;
        pt.x = i * 10.0;
        pt.y = i * 10.0;
        line1.AddPoint(pt);
    }
    
    CFringeSegment line2(0.0, 0);
    for (int i = 5; i <= 10; ++i) {
        CDPoint pt;
        pt.x = i * 10.0;
        pt.y = i * 10.0;
        line2.AddPoint(pt);
    }
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (touching at (50, 50))
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Slanted lines touching at endpoint should be merged";
}

TEST_F(AutoNumberingMergingTest, ParallelSlantedLinesSeparate) {
    // Two parallel slanted lines (y = x and y = x + 20) - not touching
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1(0.0, 0);
    for (int i = 0; i <= 10; ++i) {
        CDPoint pt;
        pt.x = i * 10.0;
        pt.y = i * 10.0;
        line1.AddPoint(pt);
    }
    
    CFringeSegment line2(0.0, 0);
    for (int i = 0; i <= 10; ++i) {
        CDPoint pt;
        pt.x = i * 10.0;
        pt.y = i * 10.0 + 20.0;
        line2.AddPoint(pt);
    }
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged (parallel, separated by 20 units)
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Parallel separated slanted lines should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, CircleIntersectingLine) {
    // Circle and horizontal line that pass through each other
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment circle = CreateCircle(50.0, 50.0, 10.0);
    CFringeSegment line = CreateHorizontalLine(50.0, 100.0, 30.0); // Passes through circle center
    
    fringes.push_back(circle);
    fringes.push_back(line);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (circle points within proximity of line)
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Circle intersecting line should be merged";
}

TEST_F(AutoNumberingMergingTest, CircleAndLineSeparate) {
    // Circle and line that don't touch
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment circle = CreateCircle(50.0, 50.0, 5.0);
    CFringeSegment line = CreateHorizontalLine(80.0, 100.0, 0.0); // Far from circle
    
    fringes.push_back(circle);
    fringes.push_back(line);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged (far apart)
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Circle and line far apart should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, TwoCirclesTouchingExternally) {
    // Two circles touching at a single point
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment circle1 = CreateCircle(0.0, 0.0, 10.0);
    CFringeSegment circle2 = CreateCircle(20.0, 0.0, 10.0); // Touching at x=10
    
    fringes.push_back(circle1);
    fringes.push_back(circle2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (touching point within proximity threshold)
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Circles touching externally should be merged";
}

TEST_F(AutoNumberingMergingTest, TwoCirclesSeparate) {
    // Two circles not touching
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment circle1 = CreateCircle(0.0, 0.0, 10.0);
    CFringeSegment circle2 = CreateCircle(30.0, 0.0, 10.0); // Gap of 10 units
    
    fringes.push_back(circle1);
    fringes.push_back(circle2);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Separated circles should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, ChainOfThreeFringes) {
    // Three lines in a chain: A touches B, B touches C
    // Tests BFS transitivity: if A connected to B and B connected to C, then all three merged
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 50.0, 0.0);   // 0 to 50
    CFringeSegment line2 = CreateHorizontalLine(0.0, 50.0, 50.0);  // 50 to 100 (touches line1)
    CFringeSegment line3 = CreateHorizontalLine(0.0, 50.0, 100.0); // 100 to 150 (touches line2)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    fringes.push_back(line3);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // All three should be merged into one group via BFS
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Line 1 and 2 should be merged";
    EXPECT_TRUE(AreMerged(fringes, 1u, 2u)) << "Line 2 and 3 should be merged";
    EXPECT_TRUE(AreMerged(fringes, 0u, 2u)) << "Line 1 and 3 should be merged (transitivity)";
}

TEST_F(AutoNumberingMergingTest, LShapeConnection) {
    // L-shaped: horizontal line touching vertical line at corner
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment hLine = CreateHorizontalLine(0.0, 50.0, 0.0);  // Horizontal at y=0, x: 0-50
    CFringeSegment vLine = CreateVerticalLine(50.0, 50.0, 0.0);   // Vertical at x=50, y: 0-50
    
    fringes.push_back(hLine);
    fringes.push_back(vLine);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (touching at corner (50, 0))
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "L-shaped lines should be merged at corner";
}

TEST_F(AutoNumberingMergingTest, TwoSeparateGroups) {
    // Two separate groups: (A, B) and (C, D)
    // Tests that BFS correctly identifies multiple connected components
    std::vector<CFringeSegment> fringes;
    
    // Group 1: touching at x=50
    CFringeSegment lineA = CreateHorizontalLine(0.0, 50.0, 0.0);   // 0 to 50
    CFringeSegment lineB = CreateHorizontalLine(0.0, 50.0, 50.0);  // 50 to 100
    
    // Group 2: touching at y=50, different location (x=150)
    CFringeSegment lineC = CreateVerticalLine(150.0, 50.0, 0.0);   // y: 0 to 50
    CFringeSegment lineD = CreateVerticalLine(150.0, 50.0, 50.0);  // y: 50 to 100
    
    fringes.push_back(lineA);
    fringes.push_back(lineB);
    fringes.push_back(lineC);
    fringes.push_back(lineD);
    
    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(10.0);
    std::vector<size_t> trustedIndices = {0u, 2u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Group 1: A and B merged
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Group 1: A and B should be merged";
    
    // Group 2: C and D merged
    EXPECT_TRUE(AreMerged(fringes, 2u, 3u)) << "Group 2: C and D should be merged";
    
    // Groups should be separate
    EXPECT_FALSE(AreMerged(fringes, 0u, 2u)) << "Group 1 and 2 should NOT be merged";
    EXPECT_FALSE(AreMerged(fringes, 1u, 3u)) << "Group 1 and 2 should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, StarPatternAllConnected) {
    // Star pattern: center line with 4 lines radiating out (all touching center)
    // Tests complex connectivity graph
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment center = CreateHorizontalLine(50.0, 20.0, 40.0); // Center at (40-60, 50)
    
    CFringeSegment north = CreateVerticalLine(50.0, 30.0, 20.0);    // Up from center
    CFringeSegment south = CreateVerticalLine(50.0, 30.0, 50.0);    // Down from center
    CFringeSegment west = CreateHorizontalLine(50.0, 30.0, 10.0);   // Left from center
    CFringeSegment east = CreateHorizontalLine(50.0, 30.0, 60.0);   // Right from center
    
    fringes.push_back(center);
    fringes.push_back(north);
    fringes.push_back(south);
    fringes.push_back(west);
    fringes.push_back(east);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // All should be in the same merged group (all connect to center)
    for (size_t i = 1u; i < fringes.size(); ++i) {
        EXPECT_TRUE(AreMerged(fringes, 0u, i)) << "Fringe " << i << " should be merged with center";
    }
}

TEST_F(AutoNumberingMergingTest, EmptyFringe) {
    // Edge case: fringe with no points
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment empty(0.0, 0);
    CFringeSegment normal = CreateHorizontalLine(0.0);
    
    fringes.push_back(empty);
    fringes.push_back(normal);
    
    fringes[1].SetNumber(1.0);
    std::vector<size_t> trustedIndices = {1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Empty fringe should not crash; should be in its own group
    EXPECT_GE(result.trustedFringes.size(), 1u) << "Should handle empty fringe gracefully";
}

TEST_F(AutoNumberingMergingTest, SinglePointFringe) {
    // Edge case: fringe with single point touching a line
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment single(0.0, 0);
    CDPoint pt;
    pt.x = 50.0;
    pt.y = 50.0;
    single.AddPoint(pt);
    
    CFringeSegment line = CreateHorizontalLine(50.0, 100.0, 45.0); // Line at y=50, close to point
    
    fringes.push_back(single);
    fringes.push_back(line);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should merge if within proximity threshold
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Single point near line should be merged";
}

TEST_F(AutoNumberingMergingTest, ConcentricCirclesNotTouching) {
    // Concentric circles (nested but not touching, should be separate)
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment inner = CreateCircle(50.0, 50.0, 10.0);
    CFringeSegment outer = CreateCircle(50.0, 50.0, 30.0);
    
    fringes.push_back(inner);
    fringes.push_back(outer);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged (concentric, gap of 20 units)
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Concentric circles with gap should NOT be merged";
}

TEST_F(AutoNumberingMergingTest, MixedOrientationsAtCommonPoint) {
    // Horizontal, vertical, and slanted lines all meeting at (50, 50)
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment hLine = CreateHorizontalLine(50.0, 50.0, 0.0);  // y=50, x: 0-50
    CFringeSegment vLine = CreateVerticalLine(50.0, 50.0, 0.0);    // x=50, y: 0-50
    
    CFringeSegment slant(0.0, 0);
    for (int i = 0; i <= 5; ++i) {
        CDPoint pt;
        pt.x = 50.0 + i * 10.0;
        pt.y = 50.0 + i * 10.0;
        slant.AddPoint(pt);
    }
    
    fringes.push_back(hLine);
    fringes.push_back(vLine);
    fringes.push_back(slant);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // All should merge (all meet at (50, 50))
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Horizontal and vertical should be merged";
    EXPECT_TRUE(AreMerged(fringes, 1u, 2u)) << "Vertical and slant should be merged";
    EXPECT_TRUE(AreMerged(fringes, 0u, 2u)) << "Horizontal and slant should be merged";
}

TEST_F(AutoNumberingMergingTest, ProximityThresholdJustInside) {
    // Test proximity threshold: points just within 0.5 threshold
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 50.0, 0.0);
    CFringeSegment line2 = CreateHorizontalLine(0.0, 50.0, 50.4); // Gap of 0.4 (within 0.5)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    std::vector<size_t> trustedIndices;
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should be merged (within proximity threshold of 0.5)
    EXPECT_TRUE(AreMerged(fringes, 0u, 1u)) << "Lines within proximity threshold should be merged";
}

TEST_F(AutoNumberingMergingTest, ProximityThresholdJustOutside) {
    // Test proximity threshold: points just beyond 0.5 threshold
    std::vector<CFringeSegment> fringes;
    
    CFringeSegment line1 = CreateHorizontalLine(0.0, 50.0, 0.0);
    CFringeSegment line2 = CreateHorizontalLine(0.0, 50.0, 51.0); // Gap of 1.0 (beyond 0.5)
    
    fringes.push_back(line1);
    fringes.push_back(line2);
    
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(5.0);
    std::vector<size_t> trustedIndices = {0u, 1u};
    
    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);
    
    // Should NOT be merged (beyond proximity threshold)
    EXPECT_FALSE(AreMerged(fringes, 0u, 1u)) << "Lines beyond proximity threshold should NOT be merged";
}

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
	 * Helper: Create a line fringe @ arbitrary slant with optional jitter
     */
    CFringeSegment CreateSlantLine(double startX = 0.0, double startY = 0.0, 
                                   double lengthX = 100.0, double lengthY = 100.0,
                                    double jitter = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = static_cast<int>((std::max)(lengthX, lengthY) / 10.0) + 1;
		double stepX = lengthX / (numPoints - 1);
		double stepY = lengthY / (numPoints - 1);
        for (int i = 0; i < numPoints; ++i) {
            CDPoint pt;
            pt.x = startX + i * stepX + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            pt.y = startY + i * stepY + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            seg.AddPoint(pt);
        }
        return seg;
    }

    /**
     * Helper: Create a closed circular fringe
     */
    CFringeSegment CreateCircle(double cx, double cy, double radius, double jitter = 0.0) {
        CFringeSegment seg(0.0, 0);
        int numPoints = 16;
        for (int i = 0; i < numPoints; ++i) {
            double angle = 2.0 * M_PI * i / numPoints;
            CDPoint pt;
            pt.x = cx + radius * std::cos(angle) + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
            pt.y = cy + radius * std::sin(angle) + (jitter > 0.0 ? (std::rand() % 1000 / 1000.0 * jitter) - (jitter / 2.0) : 0.0);
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
    EXPECT_GE(result.trustedFringes.size(), 2u);
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
    EXPECT_GE(result.trustedFringes.size(), 1u);
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
    EXPECT_GE(result.trustedFringes.size(), 2u);
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
    EXPECT_GE(result.trustedFringes.size(), 3u);
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
    EXPECT_GE(result.trustedFringes.size(), 3u);
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
    EXPECT_GE(result.trustedFringes.size(), 2u);
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
    EXPECT_GE(result.trustedFringes.size(), 2u);
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
    EXPECT_GE(result.trustedFringes.size(), 2u);  // At least keep the trusted ones
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationCircularFringesPeak) {
    // Nested rings with inner anchor
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateCircle(0.0, 0.0, 5.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 15.0));
    fringes.push_back(CreateCircle(0.0, 0.0, 25.0));

    fringes[0].SetNumber(0.0);
    std::vector<size_t> trustedIndices = {2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[0].GetNumber(), 2.0, 0.5);
    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 0.0, 0.5);
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

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, -step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_NEAR(fringes[2].GetNumber(), 0.0, 0.5);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationHorizontalLines) {
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
    EXPECT_GE(result.trustedFringes.size(), 2u);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationVerticalLines) {
    // Parallel horizontal lines (band case)
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateVerticalLine(0.0, 100.0, 0.0));
    fringes.push_back(CreateVerticalLine(15.0, 100.0, 0.0));
    fringes.push_back(CreateVerticalLine(30.0, 100.0, 0.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = { 0, 2 };

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_GE(result.trustedFringes.size(), 2u);
}

TEST_F(AutoNumberingAlgorithmTest, IntegrationSlantLines) {
    // Parallel horizontal lines (band case)
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateSlantLine(0.0, 0.0, 100.0));
    fringes.push_back(CreateSlantLine(15.0, 0.0, 100.0));
    fringes.push_back(CreateSlantLine(30.0, 0.0, 100.0));

    fringes[0].SetNumber(0.0);
    fringes[2].SetNumber(2.0);

    std::vector<size_t> trustedIndices = { 0, 2 };

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), 1.0, 0.5);
    EXPECT_GE(result.trustedFringes.size(), 2u);
}


TEST_F(AutoNumberingAlgorithmTest, MergeMixedBandAndRing) {
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

    EXPECT_EQ(fringes[0].GetNumber(), fringes[1].GetNumber());
    EXPECT_EQ(fringes[1].GetNumber(), fringes[2].GetNumber());
    EXPECT_EQ(fringes[2].GetNumber(), fringes[3].GetNumber());

}

TEST_F(AutoNumberingAlgorithmTest, IntegrationMixedBandsAndRings) {
    // Mixed topology: bands + nested rings
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateSlantLine(0.0, 0.0, 10.0, 100.0, 5));
    fringes.push_back(CreateSlantLine(10.0, 0.0, 20.0, 100.0, 5));
    fringes.push_back(CreateCircle(50.0, 50.0, 20.0, 3));
    fringes.push_back(CreateCircle(50.0, 50.0, 10.0, 3));
    fringes.push_back(CreateCircle(50.0, 50.0, 5.0, 3));

    double step = -1.0;
    fringes[0].SetNumber(0.0);
    fringes[1].SetNumber(step);
    std::vector<size_t> trustedIndices = { 0, 1 };

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    for (int idx = 0; idx < fringes.size(); ++idx) {
        EXPECT_NEAR(fringes[idx].GetNumber(), idx*step, 0.1);
    }

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
    EXPECT_GE(result.trustedFringes.size(), 1u);
}

TEST_F(AutoNumberingAlgorithmTest, EdgeCaseNegativeNumbers) {
    std::vector<CFringeSegment> fringes;
    fringes.push_back(CreateHorizontalLine(0.0));
    fringes.push_back(CreateHorizontalLine(50.0));
    fringes.push_back(CreateHorizontalLine(100.0));

    fringes[0].SetNumber(-3.0);
    fringes[1].SetNumber(0.0);
    fringes[2].SetNumber(-1.0);

    std::vector<size_t> trustedIndices = {0, 2};

    auto result = AutoNumberFringesSaddles(fringes, trustedIndices, step);

    EXPECT_NEAR(fringes[1].GetNumber(), -2.0, 0.5);
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

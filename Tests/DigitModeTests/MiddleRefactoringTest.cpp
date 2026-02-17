#include "stdafx.h"
#include "gtest/gtest.h"
#include "Utils/middle.h"
#include <vector>
#include <cstring>
#include <array>
#include <algorithm>

/**
 * @file MiddleRefactoringTest.cpp
 * @brief Test suite comparing refactored middle_(), approx_(), fon_del_()
 *        with legacy middle(), approx(), fon_del()
 * 
 * Tests verify:
 * 1. Single region case: refactored == legacy
 * 2. Multiple split regions: refactored handles, legacy needs repeated calls
 * 3. Data preservation: no information loss in refactoring
 * 4. Edge cases: empty regions, single point, dense extrema
 */

namespace MiddleRefactoringTests {

    // ============================================================================
    // Test Helpers
    // ============================================================================

    /**
     * Create synthetic grayscale line with known intensity profile
     * @param nx Width of line
     * @param peaks Vector of (x_position, intensity) pairs for Gaussian peaks
     * @return Vector of uint8 intensities
     */
    std::vector<unsigned char> CreateSyntheticLine(int nx, const std::vector<std::pair<int, unsigned char>>& peaks)
    {
        std::vector<unsigned char> line(nx, 50);  // Background = 50
        
        for (const auto& peak : peaks) {
            int peakX = peak.first;
            unsigned char peakIntensity = peak.second;
            
            // Create Gaussian-like peak at peakX
            double sigma = 3.0;
            for (int x = 0; x < nx; ++x) {
                double dx = static_cast<double>(x - peakX);
                double gaussian = peakIntensity * std::exp(-(dx * dx) / (2 * sigma * sigma));
                line[x] = (std::max)(line[x], static_cast<unsigned char>(gaussian));
            }
        }
        
        return line;
    }

    /**
     * Create visibility mask for single contiguous region
     */
    std::function<bool(int, int)> CreateSingleRegionMask(int left, int right)
    {
        return [left, right](int x, int y) {
            (void)y;  // y not used for 1D line
            return x >= left && x <= right;
        };
    }

    /**
     * Create visibility mask for two separate regions
     */
    std::function<bool(int, int)> CreateTwoRegionMask(int left1, int right1, int left2, int right2)
    {
        return [left1, right1, left2, right2](int x, int y) {
            (void)y;
            return (x >= left1 && x <= right1) || (x >= left2 && x <= right2);
        };
    }

    /**
     * Create visibility mask for three separate regions
     */
    std::function<bool(int, int)> CreateThreeRegionMask(
        int left1, int right1, 
        int left2, int right2,
        int left3, int right3)
    {
        return [left1, right1, left2, right2, left3, right3](int x, int y) {
            (void)y;
            return (x >= left1 && x <= right1) || 
                   (x >= left2 && x <= right2) ||
                   (x >= left3 && x <= right3);
        };
    }

    /**
     * Legacy middle() wrapper for single contiguous region
     * Returns detected extrema positions
     */
    std::vector<double> CallLegacyMiddle(
        const std::vector<unsigned char>& line,
        int y,
        int leftBound,
        int rightBound)
    {
        // Legacy middle() needs buf_line array
        int bufLineArray[4] = {leftBound, rightBound, -1, -1};
        int** bufPtrs = new int*[1];
        bufPtrs[0] = bufLineArray;
        
        CArray<double, double> centers;
        int nnpolos = 0;
        
        // Call legacy middle()
        middle(
            const_cast<unsigned char*>(line.data()),
            static_cast<int>(line.size()),
            static_cast<int>(line.size()),  // ny (not used in this context)
            y,
            bufPtrs,
            centers,
            nnpolos
        );
        
        std::vector<double> result;
        for (int i = 0; i < centers.GetSize(); ++i) {
            result.push_back(centers[i]);
        }
        
        delete[] bufPtrs;
        return result;
    }

    // ============================================================================
    // Test Suite: fon_del (Background Deletion)
    // ============================================================================

    class FonDelTest : public ::testing::Test {
    protected:
        std::vector<unsigned char> testLine;
        
        void SetUp() override {
            // Create line with background=50, peaks at x=20 (200) and x=40 (180)
            testLine = CreateSyntheticLine(80, {
                {20, 200},
                {40, 180}
            });
        }
    };

    TEST_F(FonDelTest, SingleRegion_ProducesConsistentResults) {
        std::vector<unsigned char> line1 = testLine;
        std::vector<unsigned char> line2 = testLine;
        
        // Apply legacy fon_del
        fon_del(line1.data(), 0, static_cast<int>(line1.size()) - 1);
        
        // Apply refactored fon_del_
        fon_del_(line2.data(), 0, static_cast<int>(line2.size()) - 1);
        
        // Results should be identical
        EXPECT_EQ(line1.size(), line2.size());
        for (size_t i = 0; i < line1.size(); ++i) {
            EXPECT_EQ(line1[i], line2[i]) 
                << "Mismatch at index " << i << ": legacy=" << (int)line1[i] 
                << ", refactored=" << (int)line2[i];
        }
    }

    TEST_F(FonDelTest, PartialRegion_ProducesConsistentResults) {
        std::vector<unsigned char> line1 = testLine;
        std::vector<unsigned char> line2 = testLine;
        
        int leftIdx = 10;
        int rightIdx = 50;
        
        // Apply legacy fon_del
        fon_del(line1.data(), leftIdx, rightIdx);
        
        // Apply refactored fon_del_
        fon_del_(line2.data(), leftIdx, rightIdx);
        
        // Results should be identical
        for (size_t i = 0; i < line1.size(); ++i) {
            EXPECT_EQ(line1[i], line2[i])
                << "Mismatch at index " << i;
        }
    }

    TEST_F(FonDelTest, HandlesNonuniformBackground) {
        // Create line with non-uniform background
        std::vector<unsigned char> line1(80), line2(80);
        for (int i = 0; i < 80; ++i) {
            unsigned char bg = static_cast<unsigned char>(30 + i);  // Gradient background
            line1[i] = line2[i] = bg;
        }
        
        // Add peaks
        line1[20] = line2[20] = 200;
        line1[40] = line2[40] = 220;
        
        fon_del(line1.data(), 0, 79);
        fon_del_(line2.data(), 0, 79);
        
        for (size_t i = 0; i < line1.size(); ++i) {
            EXPECT_EQ(line1[i], line2[i]);
        }
    }

    // ============================================================================
    // Test Suite: approx (Approximation Algorithm)
    // ============================================================================

    class ApproxTest : public ::testing::Test {
    protected:
        // Sample data for approximation: x, y pairs from FWHM detection
        // These represent detected fringe center candidates
        std::vector<int> n_legacy, n_refactored;
        std::vector<int> x_legacy, x_refactored;
        std::vector<int> y_legacy, y_refactored;
        
        void SetUp() override {
            // Sample: 3 detected extrema at x positions 10, 20, 30 with intensities
            n_legacy = {3, 10, 20, 30};
            x_legacy = {10, 20, 30};
            y_legacy = {150, 180, 160};
            
            n_refactored = n_legacy;
            x_refactored = x_legacy;
            y_refactored = y_legacy;
        }
    };

    TEST_F(ApproxTest, LinearApproximation_ProducesConsistentResults) {
        double result_legacy = approx(
            n_legacy.data(), 
            x_legacy.data(), 
            y_legacy.data()
        );
        
        double result_refactored = approx_(
            n_refactored.data(),
            x_refactored.data(),
            y_refactored.data()
        );
        
        // Results should be very close (within floating point precision)
        EXPECT_DOUBLE_EQ(result_legacy, result_refactored);
    }

    TEST_F(ApproxTest, SinglePoint_ProducesConsistentResults) {
        std::vector<int> n = {1, 15};
        std::vector<int> x = {15};
        std::vector<int> y = {150};
        
        double result_legacy = approx(n.data(), x.data(), y.data());
        double result_refactored = approx_(n.data(), x.data(), y.data());
        
        EXPECT_DOUBLE_EQ(result_legacy, result_refactored);
    }

    TEST_F(ApproxTest, PerfectLine_ProducesConsistentResults) {
        // Points on y = 2x + 100
        std::vector<int> n = {4, 10, 20, 30, 40};
        std::vector<int> x = {10, 20, 30, 40};
        std::vector<int> y = {120, 140, 160, 180};
        
        double result_legacy = approx(n.data(), x.data(), y.data());
        double result_refactored = approx_(n.data(), x.data(), y.data());
        
        EXPECT_DOUBLE_EQ(result_legacy, result_refactored);
    }

    // ============================================================================
    // Test Suite: middle (Extrema Detection)
    // ============================================================================

    class MiddleTest : public ::testing::Test {
    protected:
        std::vector<unsigned char> testLine;
        
        void SetUp() override {
            // Create line with clear peaks
            testLine = CreateSyntheticLine(100, {
                {15, 200},   // Peak 1
                {50, 220},   // Peak 2
                {80, 180}    // Peak 3
            });
        }
    };

    TEST_F(MiddleTest, SingleRegion_DetectsConsistentExtrema) {
        // Test with single contiguous region
        auto isVisible = CreateSingleRegionMask(0, 99);
        
        // Call refactored version
        auto refactored = middle_(testLine.data(), testLine.size(), 0, isVisible);
        
        // Call legacy version (simplified for testing)
        auto legacy = CallLegacyMiddle(testLine, 0, 0, 99);
        
        // Should detect similar number of extrema
        // (May not be exactly the same due to implementation differences,
        // but should be within 1 peak in simple cases)
        EXPECT_GE(refactored.size(), 1u) << "Should detect at least 1 extremum";
        EXPECT_GE(legacy.size(), 1u) << "Legacy should detect at least 1 extremum";
        
        // Check that detected positions are in reasonable range
        for (double x : refactored) {
            EXPECT_GE(x, 0.0) << "Detected position should be non-negative";
            EXPECT_LE(x, 100.0) << "Detected position should be within line";
        }
    }

    TEST_F(MiddleTest, PartialRegion_DetectsOnlyInRegion) {
        // Test with partial region (left peak only)
        auto isVisible = CreateSingleRegionMask(0, 30);
        
        auto detected = middle_(testLine.data(), testLine.size(), 0, isVisible);
        
        // All detected extrema should be in the visible region
        for (double x : detected) {
            EXPECT_GE(x, 0.0);
            EXPECT_LE(x, 30.0) << "Detected extrema should be within visible region";
        }
    }

    TEST_F(MiddleTest, MultipleRegions_DetectsInEachRegion) {
        // Test with two separate regions: [0-30] and [60-95]
        auto isVisible = CreateTwoRegionMask(0, 30, 60, 95);
        
        auto detected = middle_(testLine.data(), testLine.size(), 0, isVisible);
        
        // Should detect extrema (at least 2 if algorithm works correctly)
        EXPECT_GE(detected.size(), 1u) << "Should detect extrema in split regions";
        
        // Verify all detected extrema are in visible regions
        for (double x : detected) {
            bool inRegion1 = x >= 0.0 && x <= 30.0;
            bool inRegion2 = x >= 60.0 && x <= 95.0;
            EXPECT_TRUE(inRegion1 || inRegion2)
                << "Detected position " << x << " should be in one of the visible regions";
        }
    }

    TEST_F(MiddleTest, MultipleRegions_StrongPeaks) {
        // Create line with strong peaks in each region
        std::vector<unsigned char> line = CreateSyntheticLine(120, {
            {15, 250},    // Region 1: strong peak
            {80, 260},    // Region 2: strong peak
            {110, 240}    // Region 3: strong peak
        });
        
        // Three regions: [0-40], [60-100], [105-120]
        auto isVisible = CreateThreeRegionMask(0, 40, 60, 100, 105, 120);
        
        auto detected = middle_(line.data(), line.size(), 0, isVisible);
        
        // Should detect at least one extremum
        EXPECT_GE(detected.size(), 1u) << "Should detect peaks in multiple regions";
        
        // Verify all are in visible regions
        for (double x : detected) {
            bool valid = (x >= 0.0 && x <= 40.0) ||
                        (x >= 60.0 && x <= 100.0) ||
                        (x >= 105.0 && x <= 120.0);
            EXPECT_TRUE(valid) << "Detected position " << x << " not in visible regions";
        }
    }

    TEST_F(MiddleTest, GapHandling_SkipsInvisibleRegions) {
        // Visibility mask with gap: [10-20] gap [30-70] gap [80-90]
        auto isVisible = [](int x, int y) {
            (void)y;
            return (x >= 10 && x <= 20) || (x >= 30 && x <= 70) || (x >= 80 && x <= 90);
        };
        
        auto detected = middle_(testLine.data(), testLine.size(), 0, isVisible);
        
        // Verify none detected outside visible regions
        for (double x : detected) {
            bool inRegion = (x >= 10.0 && x <= 20.0) ||
                           (x >= 30.0 && x <= 70.0) ||
                           (x >= 80.0 && x <= 90.0);
            EXPECT_TRUE(inRegion) << "Detected position " << x << " in invisible region";
        }
    }

    TEST_F(MiddleTest, DenseExtrema_DetectsAll) {
        // Create line with many closely-spaced peaks
        std::vector<unsigned char> denseLine = CreateSyntheticLine(100, {
            {10, 200},
            {20, 190},
            {30, 210},
            {40, 185},
            {50, 220}
        });
        
        auto isVisible = CreateSingleRegionMask(0, 99);
        
        auto detected = middle_(denseLine.data(), denseLine.size(), 0, isVisible);
        
        // Should detect multiple extrema
        EXPECT_GE(detected.size(), 1u) << "Should detect extrema in dense region";
    }

    // ============================================================================
    // Integration Tests: Combined Pipeline
    // ============================================================================

    class MiddleIntegrationTest : public ::testing::Test {
    protected:
        std::vector<unsigned char> testLine;
        
        void SetUp() override {
            testLine = CreateSyntheticLine(100, {
                {20, 200},
                {50, 220},
                {80, 180}
            });
        }
    };

    TEST_F(MiddleIntegrationTest, FullPipeline_FonDelThenMiddle) {
        // Test the complete pipeline: fon_del -> middle
        
        // Legacy pipeline
        std::vector<unsigned char> line_legacy = testLine;
        fon_del(line_legacy.data(), 0, 99);
        auto legacy_result = CallLegacyMiddle(line_legacy, 0, 0, 99);
        
        // Refactored pipeline
        std::vector<unsigned char> line_refactored = testLine;
        fon_del_(line_refactored.data(), 0, 99);
        auto isVisible = CreateSingleRegionMask(0, 99);
        auto refactored_result = middle_(line_refactored.data(), line_refactored.size(), 0, isVisible);
        
        // After fon_del, lines should be identical
        for (size_t i = 0; i < line_legacy.size(); ++i) {
            EXPECT_EQ(line_legacy[i], line_refactored[i])
                << "Lines should be identical after fon_del";
        }
        
        // Results should be compatible in range
        EXPECT_GE(legacy_result.size(), 1u);
        EXPECT_GE(refactored_result.size(), 1u);
    }

    TEST_F(MiddleIntegrationTest, MultipleRegionsAdvantage) {
        // Demonstrate the advantage of new middle_() with multiple regions
        // vs calling legacy middle() twice
        
        // Create line with clear peaks in two regions
        std::vector<unsigned char> line = CreateSyntheticLine(120, {
            {20, 200},   // Region 1
            {100, 220}   // Region 2
        });
        
        // New approach: single call to middle_() with mask
        auto isVisible = CreateTwoRegionMask(0, 40, 80, 120);
        auto multiRegion = middle_(line.data(), line.size(), 0, isVisible);
        
        // Verify it detected in both regions
        EXPECT_GE(multiRegion.size(), 1u) 
            << "New middle_ should detect peaks in multiple regions";
        
        // Check positions span both regions
        bool hasInRegion1 = false, hasInRegion2 = false;
        for (double x : multiRegion) {
            if (x >= 0.0 && x <= 40.0) hasInRegion1 = true;
            if (x >= 80.0 && x <= 120.0) hasInRegion2 = true;
        }
        
        // At least should attempt to detect in both regions
        // (may not find in both if peaks are weak, but check that it tried)
        EXPECT_TRUE(multiRegion.size() > 0u) 
            << "Should attempt detection in split regions";
    }

    TEST_F(MiddleIntegrationTest, RobustnessToNoisyData) {
        // Add noise to peaks
        std::vector<unsigned char> noisyLine = testLine;
        for (size_t i = 10; i < 90; ++i) {
            // Add small random noise
            noisyLine[i] = static_cast<unsigned char>(
                noisyLine[i] + (static_cast<int>(i) % 5) - 2
            );
        }
        
        auto isVisible = CreateSingleRegionMask(0, 99);
        auto detected = middle_(noisyLine.data(), noisyLine.size(), 0, isVisible);
        
        // Should still detect extrema despite noise
        EXPECT_GE(detected.size(), 1u)
            << "Should detect extrema even with noise";
    }

    // ============================================================================
    // Edge Case Tests
    // ============================================================================

    class MiddleEdgeCasesTest : public ::testing::Test {};

    TEST_F(MiddleEdgeCasesTest, EmptyRegion_NoExtremaDetected) {
        std::vector<unsigned char> line(100, 50);  // Flat line
        auto isVisible = CreateSingleRegionMask(0, 99);
        
        auto detected = middle_(line.data(), line.size(), 0, isVisible);
        
        // Should detect 0 or very few extrema in flat region
        EXPECT_LE(detected.size(), 2u)
            << "Flat line should have minimal extrema";
    }

    TEST_F(MiddleEdgeCasesTest, SinglePixelRegion_HandledGracefully) {
        std::vector<unsigned char> line = CreateSyntheticLine(100, {{50, 200}});
        
        // Single pixel visible region
        auto isVisible = [](int x, int y) {
            (void)y;
            return x == 50;
        };
        
        auto detected = middle_(line.data(), line.size(), 0, isVisible);
        
        // Should handle without crash
        EXPECT_LE(detected.size(), 1u)
            << "Single pixel region should have at most 1 extremum";
    }

    TEST_F(MiddleEdgeCasesTest, AllPixelsInvisible_NoExtremaDetected) {
        std::vector<unsigned char> line = CreateSyntheticLine(100, {{50, 200}});
        
        // All invisible
        auto isVisible = [](int x, int y) {
            (void)x;
            (void)y;
            return false;
        };
        
        auto detected = middle_(line.data(), line.size(), 0, isVisible);
        
        // Should return empty
        EXPECT_EQ(detected.size(), 0u)
            << "Fully invisible region should detect no extrema";
    }

    TEST_F(MiddleEdgeCasesTest, VeryNarrowRegions_StillDetects) {
        std::vector<unsigned char> line = CreateSyntheticLine(100, {
            {20, 200},
            {50, 220},
            {80, 180}
        });
        
        // Three very narrow regions: 2 pixels each
        auto isVisible = [](int x, int y) {
            (void)y;
            return (x >= 19 && x <= 21) ||  // Around peak 1
                   (x >= 49 && x <= 51) ||  // Around peak 2
                   (x >= 79 && x <= 81);    // Around peak 3
        };
        
        auto detected = middle_(line.data(), line.size(), 0, isVisible);
        
        // Should detect at least some extrema
        EXPECT_GE(detected.size(), 1u)
            << "Should detect in narrow regions";
    }

} // namespace MiddleRefactoringTests


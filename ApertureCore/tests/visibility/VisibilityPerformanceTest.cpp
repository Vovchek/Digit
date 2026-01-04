/**
 * @file VisibilityPerformanceTest.cpp
 * @brief Performance benchmarks for VisibilityChecker
 * 
 * Tests visibility checking performance with:
 * - Multi-megapixel images (realistic image sizes)
 * - Multiple polygon shapes (worst case for performance)
 * - Various shape type combinations
 * - Real-world configurations
 */

#include <gtest/gtest.h>
#include "aperturecore/visibility/VisibilityChecker.h"
#include "aperturecore/visibility/ShapeCollection.h"
#include "aperturecore/geometry/Ellipse.h"
#include "aperturecore/geometry/Rectangle.h"
#include "aperturecore/geometry/Polygon.h"
#include <chrono>
#include <iostream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace aperture;
using namespace std::chrono;

// ============================================================================
// Performance Test Fixture
// ============================================================================

class VisibilityPerformanceTest : public ::testing::Test {
protected:
    // Standard image sizes
    static constexpr int IMG_1MP = 1024 * 1024;         // 1 megapixel (1024x1024)
    static constexpr int IMG_2MP = 1920 * 1080;         // ~2 megapixels (1920x1080, Full HD)
    static constexpr int IMG_4MP = 2048 * 2048;         // 4 megapixels (2048x2048)
    static constexpr int IMG_8MP = 3840 * 2160;         // ~8 megapixels (3840x2160, 4K)
    
    // Performance thresholds (pixels per second)
    static constexpr double MIN_THROUGHPUT_PPS = 10'000'000.0;  // 10 million pixels/sec
    static constexpr double TARGET_THROUGHPUT_PPS = 50'000'000.0; // 50 million pixels/sec
    
    // Helper to measure performance
    struct BenchmarkResult {
        size_t pixelCount;
        double elapsedMs;
        double pixelsPerSecond;
        size_t visibleCount;
        VisibilityChecker::Stats stats;
    };
    
    BenchmarkResult benchmarkVisibility(VisibilityChecker& checker, 
                                       int width, int height,
                                       const std::string& testName = "") {
        auto start = high_resolution_clock::now();
        
        size_t visibleCount = 0;
        size_t pixelCount = 0;
        
        // Check every pixel
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Point p{static_cast<double>(x), static_cast<double>(y)};
                if (checker.isVisible(p)) {
                    visibleCount++;
                }
                pixelCount++;
            }
        }
        
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        double elapsedMs = duration.count() / 1000.0;
        double pixelsPerSecond = (pixelCount * 1'000'000.0) / duration.count();
        
        BenchmarkResult result{
            pixelCount,
            elapsedMs,
            pixelsPerSecond,
            visibleCount,
            checker.getStats()
        };
        
        // Print results
        if (!testName.empty()) {
            std::cout << "\n=== " << testName << " ===" << std::endl;
        }
        std::cout << "  Image size: " << width << "x" << height 
                  << " (" << (pixelCount / 1'000'000.0) << " MP)" << std::endl;
        std::cout << "  Pixels checked: " << pixelCount << std::endl;
        std::cout << "  Visible pixels: " << visibleCount 
                  << " (" << (100.0 * visibleCount / pixelCount) << "%)" << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(2) << elapsedMs << " ms" << std::endl;
        std::cout << "  Throughput: " << std::fixed << std::setprecision(2) 
                  << (pixelsPerSecond / 1'000'000.0) << " Mpixels/sec" << std::endl;
        std::cout << "  Checks per pixel: INTERNAL=" << (result.stats.internalChecks / (double)pixelCount)
                  << ", EXTERNAL=" << (result.stats.externalChecks / (double)pixelCount)
                  << ", APERTURE=" << (result.stats.apertureChecks / (double)pixelCount) << std::endl;
        std::cout << "  Early exits: " << result.stats.earlyExits 
                  << " (" << (100.0 * result.stats.earlyExits / pixelCount) << "%)" << std::endl;
        
        return result;
    }
    
    // Helper to create polygon with N vertices
    std::unique_ptr<Polygon> createNGon(int n, double radius, double centerX, double centerY, TypeLimits  typeLimits) {
        std::vector<Point> vertices;
        for (int i = 0; i < n; i++) {
            double angle = 2.0 * M_PI * i / n;
            vertices.push_back({
                centerX + radius * std::cos(angle),
                centerY + radius * std::sin(angle)
            });
        }
        return std::make_unique<Polygon>(vertices, typeLimits);
    }
};

// ============================================================================
// Baseline Performance Tests (Simple Shapes)
// ============================================================================

TEST_F(VisibilityPerformanceTest, Baseline_1MP_SimpleAnnulus) {
    // Baseline: 1MP image with simple annular aperture
    
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(500.0, 500.0, 512.0, 512.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 512.0, 512.0, 0.0,
                    TypeLimits::INTERNAL));
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "Baseline: 1MP Simple Annulus");
    
    // Performance assertion
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS)
        << "Throughput below minimum threshold!";
    
    std::cout << "  V Performance: " 
              << (result.pixelsPerSecond >= TARGET_THROUGHPUT_PPS ? "EXCELLENT" : "ACCEPTABLE")
              << std::endl;
}

TEST_F(VisibilityPerformanceTest, Baseline_2MP_FullHD) {
    // 2MP (1920x1080, Full HD) with simple configuration
    
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(800.0, 500.0, 960.0, 540.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(150.0, 150.0, 960.0, 540.0, 0.0,
                    TypeLimits::INTERNAL));
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1920, 1080, 
                                      "Baseline: 2MP Full HD");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

// ============================================================================
// Polygon Performance Tests (Critical for Performance)
// ============================================================================

TEST_F(VisibilityPerformanceTest, Polygons_1MP_MultipleComplexPolygons) {
    // CRITICAL: Multiple complex polygons (worst case for performance)
    
    ShapeCollection shapes;
    
    // EXTERNAL: 20-sided polygon
    shapes.addShape(createNGon(20, 500.0, 512.0, 512.0, TypeLimits::EXTERNAL));
    
    // INTERNAL: 12-sided polygon (central obstruction)
    shapes.addShape(createNGon(12, 100.0, 512.0, 512.0, TypeLimits::INTERNAL));
    
    // INTERNAL: Spider vanes (4 elongated polygons)
    for (int i = 0; i < 4; i++) {
        double angle = M_PI * i / 2.0;
        std::vector<Point> vane = {
            {512.0 + 50.0 * std::cos(angle - 0.05), 512.0 + 50.0 * std::sin(angle - 0.05)},
            {512.0 + 500.0 * std::cos(angle - 0.05), 512.0 + 500.0 * std::sin(angle - 0.05)},
            {512.0 + 500.0 * std::cos(angle + 0.05), 512.0 + 500.0 * std::sin(angle + 0.05)},
            {512.0 + 50.0 * std::cos(angle + 0.05), 512.0 + 50.0 * std::sin(angle + 0.05)}
        };
        shapes.addInternal(std::make_unique<Polygon>(vane));
    }
    
    // APERTURE: 3 slit openings (8-sided polygons each)
    for (int i = 0; i < 3; i++) {
        double offsetX = (i - 1) * 200.0;
        shapes.addShape(createNGon(8, 30.0, 512.0 + offsetX, 512.0, 
                       TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "WORST CASE: 1MP Multiple Complex Polygons");
    
    // Even with complex polygons, should meet minimum threshold
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS)
        << "Polygon performance below minimum threshold!";
    
    std::cout << "  Total shapes: " << (1 + 1 + 4 + 3) << " polygons" << std::endl;
}

TEST_F(VisibilityPerformanceTest, Polygons_2MP_RealisticTelescopeAperture) {
    // Realistic telescope aperture with polygon obstructions
    
    ShapeCollection shapes;
    
    // EXTERNAL: 30-sided polygon (approximates circle)
    shapes.addShape(createNGon(30, 900.0, 960.0, 540.0, TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction (20-sided)
    shapes.addShape(createNGon(20, 200.0, 960.0, 540.0, TypeLimits::INTERNAL));
    
    // INTERNAL: 4 spider vanes
    for (int i = 0; i < 4; i++) {
        double angle = M_PI * i / 2.0 + M_PI / 4.0; // 45° offset
        std::vector<Point> vane;
        for (int j = 0; j < 4; j++) {
            double r = (j < 2) ? 200.0 : 900.0;
            double a = angle + ((j % 2 == 0) ? -0.02 : 0.02);
            vane.push_back({960.0 + r * std::cos(a), 540.0 + r * std::sin(a)});
        }
        shapes.addInternal(std::make_unique<Polygon>(vane));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1920, 1080, 
                                      "Realistic: 2MP Telescope Aperture (Polygons)");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

// ============================================================================
// Scaling Tests (Multi-Megapixel)
// ============================================================================

TEST_F(VisibilityPerformanceTest, Scaling_4MP_AnnulusWithSlits) {
    // 4MP image with annulus + slit apertures
    
    ShapeCollection shapes;
    
    // EXTERNAL: Large circle
    shapes.addShape(std::make_unique<Ellipse>(1000.0, 1000.0, 1024.0, 1024.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction
    shapes.addShape(std::make_unique<Ellipse>(200.0, 200.0, 1024.0, 1024.0, 0.0,
                    TypeLimits::INTERNAL));
    
    // APERTURE: 4 radial slits
    for (int i = 0; i < 4; i++) {
        double angle = M_PI * i / 2.0;
        double cx = 1024.0 + 600.0 * std::cos(angle);
        double cy = 1024.0 + 600.0 * std::sin(angle);
        shapes.addShape(std::make_unique<Rectangle>(20.0, 100.0, cx, cy, angle * 180.0 / M_PI, 
                       TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 2048, 2048, 
                                      "Scaling: 4MP Annulus with Slits");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

TEST_F(VisibilityPerformanceTest, DISABLED_Scaling_8MP_4K_ComplexConfiguration) {
    // 8MP (4K resolution) - DISABLED by default due to long runtime
    // Run with: --gtest_also_run_disabled_tests
    
    ShapeCollection shapes;
    
    // Complex configuration with multiple shape types
    shapes.addShape(std::make_unique<Ellipse>(1800.0, 1000.0, 1920.0, 1080.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(createNGon(16, 300.0, 1920.0, 1080.0, TypeLimits::INTERNAL));
    
    // Multiple aperture openings
    for (int i = 0; i < 6; i++) {
        double angle = 2.0 * M_PI * i / 6.0;
        double cx = 1920.0 + 1200.0 * std::cos(angle);
        double cy = 1080.0 + 800.0 * std::sin(angle);
        shapes.addShape(createNGon(6, 50.0, cx, cy, TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 3840, 2160, 
                                      "Scaling: 8MP (4K) Complex Configuration");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

// ============================================================================
// Early Exit Optimization Tests
// ============================================================================

TEST_F(VisibilityPerformanceTest, Optimization_InternalEarlyExit) {
    // Test that INTERNAL shapes trigger early exits efficiently
    
    ShapeCollection shapes;
    
    shapes.addShape(std::make_unique<Ellipse>(500.0, 500.0, 512.0, 512.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // Large INTERNAL shape (covers ~50% of EXTERNAL)
    shapes.addShape(std::make_unique<Ellipse>(350.0, 350.0, 512.0, 512.0, 0.0,
                    TypeLimits::INTERNAL));
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "Optimization: INTERNAL Early Exit");
    
    // With large INTERNAL, should have high early exit rate
    double earlyExitRate = (double)result.stats.earlyExits / result.pixelCount;
    std::cout << "  Early exit rate: " << (earlyExitRate * 100.0) << "%" << std::endl;
    
    // Should have > 30% early exits (INTERNAL blocks many pixels)
    EXPECT_GT(earlyExitRate, 0.30) 
        << "Expected high early exit rate with large INTERNAL";
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

TEST_F(VisibilityPerformanceTest, Optimization_ApertureEarlyExit) {
    // Test that APERTURE shapes trigger early exits when found
    
    ShapeCollection shapes;
    
    shapes.addShape(std::make_unique<Ellipse>(500.0, 500.0, 512.0, 512.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // Multiple APERTURE shapes (first one should cause early exit)
    for (int i = 0; i < 5; i++) {
        shapes.addShape(std::make_unique<Rectangle>(100.0, 100.0, 
                                                    512.0 + i * 50.0, 512.0, 0.0,
                       TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "Optimization: APERTURE Early Exit");
    
    // Average APERTURE checks per pixel should be < number of APERTURE shapes
    double avgApertureChecks = (double)result.stats.apertureChecks / result.pixelCount;
    std::cout << "  Avg APERTURE checks/pixel: " << avgApertureChecks << std::endl;
    
    EXPECT_LT(avgApertureChecks, 5.0) 
        << "Should early exit when APERTURE found";
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

// ============================================================================
// Shape Count Scaling Tests
// ============================================================================

TEST_F(VisibilityPerformanceTest, ShapeCount_10Polygons) {
    // Test with 10 polygon shapes
    
    ShapeCollection shapes;
    
    shapes.addShape(createNGon(20, 500.0, 512.0, 512.0, TypeLimits::EXTERNAL));
    
    for (int i = 0; i < 9; i++) {
        double angle = 2.0 * M_PI * i / 9.0;
        double cx = 512.0 + 300.0 * std::cos(angle);
        double cy = 512.0 + 300.0 * std::sin(angle);
        int sides = 6 + (i % 4) * 2; // 6, 8, 10, or 12 sides
        shapes.addShape(createNGon(sides, 40.0, cx, cy, 
                       (i < 4) ? TypeLimits::INTERNAL : TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "Shape Count: 10 Polygons");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

TEST_F(VisibilityPerformanceTest, ShapeCount_20Polygons) {
    // Test with 20 polygon shapes (stress test)
    
    ShapeCollection shapes;
    
    shapes.addShape(createNGon(24, 500.0, 512.0, 512.0, TypeLimits::EXTERNAL));
    
    for (int i = 0; i < 19; i++) {
        double angle = 2.0 * M_PI * i / 19.0;
        double radius = 150.0 + (i % 3) * 50.0;
        double cx = 512.0 + radius * std::cos(angle);
        double cy = 512.0 + radius * std::sin(angle);
        int sides = 5 + (i % 5);
        
        TypeLimits type;
        if (i < 6) type = TypeLimits::INTERNAL;
        else if (i < 12) type = TypeLimits::APERTURE;
        else type = TypeLimits::INTERNAL;
        
        shapes.addShape(createNGon(sides, 30.0, cx, cy, type));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1024, 1024, 
                                      "Shape Count: 20 Polygons (STRESS TEST)");
    
    // Even with 20 polygons, should still meet minimum
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS)
        << "Performance degraded with 20 polygons!";
    
    std::cout << "  Total polygon shapes: 20" << std::endl;
}

// ============================================================================
// Real-World Scenario Tests
// ============================================================================

TEST_F(VisibilityPerformanceTest, RealWorld_JWSTAperture) {
    // James Webb Space Telescope-like aperture (hexagonal segments)
    
    ShapeCollection shapes;
    
    // EXTERNAL: Large hexagonal outer boundary
    shapes.addShape(createNGon(6, 900.0, 960.0, 540.0, TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction (secondary mirror)
    shapes.addShape(createNGon(6, 250.0, 960.0, 540.0, TypeLimits::INTERNAL));
    
    // INTERNAL: 3 spider vanes
    for (int i = 0; i < 3; i++) {
        double angle = 2.0 * M_PI * i / 3.0;
        std::vector<Point> vane;
        for (int j = 0; j < 4; j++) {
            double r = (j < 2) ? 250.0 : 900.0;
            double a = angle + ((j % 2 == 0) ? -0.015 : 0.015);
            vane.push_back({960.0 + r * std::cos(a), 540.0 + r * std::sin(a)});
        }
        shapes.addInternal(std::make_unique<Polygon>(vane));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1920, 1080, 
                                      "Real-World: JWST-like Aperture");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

TEST_F(VisibilityPerformanceTest, RealWorld_InterferometryAperture) {
    // Interferometry aperture with multiple sub-apertures
    
    ShapeCollection shapes;
    
    // EXTERNAL: Large boundary
    shapes.addShape(std::make_unique<Ellipse>(900.0, 900.0, 960.0, 540.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // APERTURE: 7 circular sub-apertures in hexagonal pattern
    std::vector<std::pair<double, double>> positions = {
        {0.0, 0.0},      // Center
        {300.0, 0.0},    // Right
        {150.0, 260.0},  // Top-right
        {-150.0, 260.0}, // Top-left
        {-300.0, 0.0},   // Left
        {-150.0, -260.0}, // Bottom-left
        {150.0, -260.0}  // Bottom-right
    };
    
    for (const auto& [dx, dy] : positions) {
        shapes.addShape(createNGon(12, 80.0, 960.0 + dx, 540.0 + dy, 
                       TypeLimits::APERTURE));
    }
    
    VisibilityChecker checker(shapes);
    
    auto result = benchmarkVisibility(checker, 1920, 1080, 
                                      "Real-World: Interferometry Aperture");
    
    EXPECT_GT(result.pixelsPerSecond, MIN_THROUGHPUT_PPS);
}

// ============================================================================
// Summary Test (All Scenarios)
// ============================================================================

TEST_F(VisibilityPerformanceTest, DISABLED_PerformanceSummary) {
    // Comprehensive summary of all performance scenarios
    // Run with: --gtest_also_run_disabled_tests
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "PERFORMANCE SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "\nMinimum acceptable throughput: " 
              << (MIN_THROUGHPUT_PPS / 1'000'000.0) << " Mpixels/sec" << std::endl;
    std::cout << "Target throughput: " 
              << (TARGET_THROUGHPUT_PPS / 1'000'000.0) << " Mpixels/sec" << std::endl;
    std::cout << "\nAll performance tests should exceed minimum threshold." << std::endl;
    std::cout << "Run individual tests for detailed metrics." << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

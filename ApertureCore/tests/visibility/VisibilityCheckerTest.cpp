/**
 * @file VisibilityCheckerTest.cpp
 * @brief Tests for VisibilityChecker visibility logic correctness
 * 
 * These tests verify the correct implementation of the 3-type visibility algorithm,
 * specifically testing that:
 * 1. APERTURE shapes can override EXTERNAL blocking
 * 2. INTERNAL shapes are absolute blockers (nothing overrides)
 * 3. Algorithm order is correct (INTERNAL first for performance)
 */

#include <gtest/gtest.h>
#include "aperturecore/visibility/VisibilityChecker.h"
#include "aperturecore/visibility/ShapeCollection.h"
#include "aperturecore/geometry/Ellipse.h"
#include "aperturecore/geometry/Rectangle.h"
#include "aperturecore/geometry/Polygon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace aperture;

// ============================================================================
// Test Fixture
// ============================================================================

class VisibilityCheckerTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;
};

// ============================================================================
// CRITICAL: APERTURE Override Tests (Tests the Bug Fix!)
// ============================================================================

TEST_F(VisibilityCheckerTest, ApertureCanOverrideExternalBlocking) {
    // This is the CRITICAL test that would FAIL with the old bug!
    // 
    // Setup: Point is OUTSIDE EXTERNAL but INSIDE APERTURE
    // Expected: visible = TRUE (APERTURE opens visibility)
    // Old bug: would return FALSE (EXTERNAL blocked immediately)
    
    ShapeCollection shapes;
    
    // EXTERNAL: Circle centered at (100, 100) with radius 50
    shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0));
    
    // APERTURE: Rectangle extending outside EXTERNAL boundary
    // Rectangle: width=100, height=30, center=(170, 100)
    // X range: [170-50, 170+50] = [120, 220]
    // Y range: [100-15, 100+15] = [85, 115]
    shapes.addAperture(std::make_unique<Rectangle>(100.0, 30.0, 170.0, 100.0));
    
    VisibilityChecker checker(shapes);
    
    // Point outside EXTERNAL circle but inside APERTURE rectangle
    Point outsideExternalInsideAperture{170.0, 100.0};
    
    // Distance from EXTERNAL center: sqrt((170-100)^2 + 0) = 70 > 50 (outside EXTERNAL)
    // Inside APERTURE rectangle: x=170 in [120,220], y=100 in [85,115] ✓
    
    // ❌ OLD BUG: Would return false (EXTERNAL check returned immediately)
    // ✅ CORRECT: Should return true (APERTURE overrides EXTERNAL blocking)
    EXPECT_TRUE(checker.isVisible(outsideExternalInsideAperture))
        << "APERTURE must be able to override EXTERNAL blocking!";
}

TEST_F(VisibilityCheckerTest, ApertureOpensHoleInExternalBoundary) {
    // Real-world scenario: Slit aperture extending through boundary
    
    ShapeCollection shapes;
    
    // EXTERNAL: Large circle (main aperture)
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // APERTURE: Thin vertical slit extending beyond boundary
    shapes.addShape(std::make_unique<Rectangle>(5.0, 150.0, 100.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    // Points along the slit
    Point inSlit1{100.0, 50.0};   // Inside EXTERNAL, inside slit
    Point inSlit2{100.0, 150.0};  // Outside EXTERNAL, inside slit
    Point inSlit3{100.0, 200.0};  // Far outside EXTERNAL, inside slit
    
    // All should be visible (slit opens visibility)
    EXPECT_TRUE(checker.isVisible(inSlit1)) << "Point in slit (inside EXTERNAL)";
    EXPECT_TRUE(checker.isVisible(inSlit2)) << "Point in slit (outside EXTERNAL) - APERTURE opens!";
    EXPECT_TRUE(checker.isVisible(inSlit3)) << "Point in slit (far outside) - APERTURE opens!";
}

TEST_F(VisibilityCheckerTest, MultipleAperturesCreateMultipleOpenings) {
    // Multiple APERTURE shapes create union of openings
    
    ShapeCollection shapes;
    
    // EXTERNAL: Circle
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // APERTURE 1: Opening on the right
    shapes.addShape(std::make_unique<Rectangle>(20.0, 30.0, 160.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    // APERTURE 2: Opening on the left
    shapes.addShape(std::make_unique<Rectangle>(20.0, 30.0, 40.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    // Points in each opening (outside EXTERNAL)
    Point rightOpening{165.0, 100.0};
    Point leftOpening{35.0, 100.0};
    
    EXPECT_TRUE(checker.isVisible(rightOpening)) << "Right opening via APERTURE 1";
    EXPECT_TRUE(checker.isVisible(leftOpening)) << "Left opening via APERTURE 2";
}

// ============================================================================
// INTERNAL Absolute Blocking Tests
// ============================================================================

TEST_F(VisibilityCheckerTest, InternalBlocksEvenIfInsideAperture) {
    // INTERNAL shapes are ABSOLUTE blockers
    // Even APERTURE cannot override INTERNAL blocking
    
    ShapeCollection shapes;
    
    // EXTERNAL: Large circle
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction
    shapes.addShape(std::make_unique<Ellipse>(30.0, 30.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    
    // APERTURE: Overlaps INTERNAL region
    shapes.addShape(std::make_unique<Rectangle>(40.0, 40.0, 100.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    // Point inside INTERNAL and APERTURE
    Point insideBoth{100.0, 100.0};
    
    // INTERNAL always wins (absolute blocker)
    EXPECT_FALSE(checker.isVisible(insideBoth))
        << "INTERNAL must block even if inside APERTURE!";
}

TEST_F(VisibilityCheckerTest, InternalBlocksInsideExternalAndAperture) {
    // INTERNAL has highest priority - blocks everything
    
    ShapeCollection shapes;
    
	shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0, 0.0,
                    TypeLimits::INTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    Point insideInternal{10.0, 10.0};  // Inside INTERNAL (radius 20)
    Point outsideInternal{30.0, 30.0}; // Outside INTERNAL, inside others
    
    EXPECT_FALSE(checker.isVisible(insideInternal)) << "Blocked by INTERNAL";
    EXPECT_TRUE(checker.isVisible(outsideInternal)) << "Not blocked (outside INTERNAL)";
}

// ============================================================================
// Algorithm Order Tests
// ============================================================================

TEST_F(VisibilityCheckerTest, InternalCheckedFirstForPerformance) {
    // Verify INTERNAL is checked first (early exit optimization)
    
    ShapeCollection shapes;
    
    // Add shapes in specific order
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(20.0, 20.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    shapes.addShape(std::make_unique<Rectangle>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    Point insideInternal{100.0, 100.0};
    
    // Check visibility
    bool visible = checker.isVisible(insideInternal);
    EXPECT_FALSE(visible);
    
    // Check that we got early exit (INTERNAL blocked)
    auto stats = checker.getStats();
    EXPECT_EQ(stats.internalChecks, 1u) << "Should check INTERNAL first";
    EXPECT_EQ(stats.earlyExits, 1u) << "Should exit early on INTERNAL block";
    EXPECT_EQ(stats.externalChecks, 0u) << "Should NOT check EXTERNAL (early exit)";
    EXPECT_EQ(stats.apertureChecks, 0u) << "Should NOT check APERTURE (early exit)";
}

TEST_F(VisibilityCheckerTest, ApertureEarlyExitOptimization) {
    // When APERTURE found, should break (early exit)
    
    ShapeCollection shapes;
    
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Rectangle>(30.0, 30.0, 120.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    shapes.addShape(std::make_unique<Rectangle>(30.0, 30.0, 80.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    checker.resetStats();
    
    Point insideFirstAperture{125.0, 100.0};
    
    bool visible = checker.isVisible(insideFirstAperture);
    EXPECT_TRUE(visible);
    
    auto stats = checker.getStats();
    EXPECT_EQ(stats.apertureChecks, 0u) << "Should exit without testing APERTURE";
    EXPECT_EQ(stats.earlyExits, 1u) << "Inside EXTERNAL should trigger early exit";
}

// ============================================================================
// Standard Annular Aperture Tests
// ============================================================================

TEST_F(VisibilityCheckerTest, StandardAnnularAperture) {
    // Classic use case: Outer boundary with central obstruction
    
    ShapeCollection shapes;
    
    // EXTERNAL: Outer circle (radius 100)
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction (radius 30)
    shapes.addShape(std::make_unique<Ellipse>(30.0, 30.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    
    VisibilityChecker checker(shapes);
    
    // Test points at various positions
    Point center{100.0, 100.0};           // Center (inside INTERNAL)
    Point inAnnulus{60.0, 100.0};         // In annular region
    Point onInternalEdge{130.0, 100.0};   // On INTERNAL boundary
    Point onExternalEdge{200.0, 100.0};   // On EXTERNAL boundary
    Point outside{250.0, 100.0};          // Outside everything
    
    EXPECT_FALSE(checker.isVisible(center)) << "Blocked by INTERNAL";
    EXPECT_TRUE(checker.isVisible(inAnnulus)) << "Visible in annulus";
    EXPECT_FALSE(checker.isVisible(onInternalEdge)) << "On INTERNAL edge (outside)";
    EXPECT_TRUE(checker.isVisible(onExternalEdge)) << "On EXTERNAL edge (inside)";
    EXPECT_FALSE(checker.isVisible(outside)) << "Outside EXTERNAL";
}

TEST_F(VisibilityCheckerTest, AnnularApertureWithSlits) {
    // Annular aperture with additional slit openings
    
    ShapeCollection shapes;
    
    // EXTERNAL: Outer circle
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // INTERNAL: Central obstruction
    shapes.addShape(std::make_unique<Ellipse>(30.0, 30.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    
    // APERTURE: Vertical slit extending beyond boundary
    shapes.addShape(std::make_unique<Rectangle>(5.0, 250.0, 100.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    Point inAnnulus{60.0, 100.0};         // Normal annulus region
    Point inSlit{100.0, 220.0};           // In slit (outside EXTERNAL)
    Point inSlitAndInternal{100.0, 100.0}; // In slit but blocked by INTERNAL
    
    EXPECT_TRUE(checker.isVisible(inAnnulus)) << "Visible in annulus";
    EXPECT_TRUE(checker.isVisible(inSlit)) << "Slit opens visibility";
    EXPECT_FALSE(checker.isVisible(inSlitAndInternal)) << "INTERNAL blocks even in slit";
}

// ============================================================================
// Edge Cases and Boundary Conditions
// ============================================================================

TEST_F(VisibilityCheckerTest, NoShapes_AlwaysInvisible) {
    ShapeCollection shapes;
    VisibilityChecker checker(shapes);
    
    Point anywhere{100.0, 100.0};
    
    EXPECT_FALSE(checker.isVisible(anywhere)) 
        << "No EXTERNAL shapes -> everything invisible";
}

TEST_F(VisibilityCheckerTest, OnlyExternal_InsideVisible) {
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    VisibilityChecker checker(shapes);
    
    Point inside{100.0, 100.0};
    Point outside{200.0, 200.0};
    
    EXPECT_TRUE(checker.isVisible(inside)) << "Inside EXTERNAL -> visible";
    EXPECT_FALSE(checker.isVisible(outside)) << "Outside EXTERNAL -> invisible";
}

TEST_F(VisibilityCheckerTest, OnlyAperture_AlwaysInsideApertureVisible) {
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    Point inside{100.0, 100.0};
    Point outside{200.0, 200.0};
    
    EXPECT_TRUE(checker.isVisible(inside)) << "Inside APERTURE -> visible";
    EXPECT_FALSE(checker.isVisible(outside)) << "Outside APERTURE -> invisible (no EXTERNAL)";
}

TEST_F(VisibilityCheckerTest, OnlyInternal_AlwaysBlocked) {
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    
    VisibilityChecker checker(shapes);
    
    Point inside{100.0, 100.0};
    Point outside{200.0, 200.0};
    
    // No EXTERNAL -> everything invisible (INTERNAL just blocks additionally)
    EXPECT_FALSE(checker.isVisible(inside)) << "Inside INTERNAL -> blocked";
    EXPECT_FALSE(checker.isVisible(outside)) << "No EXTERNAL -> invisible";
}

// ============================================================================
// Complex Multi-Shape Scenarios
// ============================================================================

TEST_F(VisibilityCheckerTest, ComplexMultiShapeConfiguration) {
    // Real-world complex configuration
    
    ShapeCollection shapes;
    
    // Main aperture (large circle)
    shapes.addShape(std::make_unique<Ellipse>(150.0, 150.0, 200.0, 200.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // Central obstruction
    shapes.addShape(std::make_unique<Ellipse>(40.0, 40.0, 200.0, 200.0, 0.0,
                    TypeLimits::INTERNAL));
    
    // Spider vanes (obstructions)
    shapes.addShape(std::make_unique<Rectangle>(5.0, 300.0, 200.0, 200.0, 0.0,
                    TypeLimits::INTERNAL));
    shapes.addShape(std::make_unique<Rectangle>(300.0, 5.0, 200.0, 200.0, 0.0,
                    TypeLimits::INTERNAL));
    
    // Openings/slits
    shapes.addShape(std::make_unique<Rectangle>(10.0, 50.0, 100.0, 200.0, 0.0,
                    TypeLimits::APERTURE));
    shapes.addShape(std::make_unique<Rectangle>(10.0, 50.0, 300.0, 200.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    // Test various points
    Point inAnnulus{100.0, 100.0};           // In clear annulus
    Point onSpider{200.0, 200.0};            // On spider vane
    Point inLeftSlit{96.0, 200.0};           // In left opening
    Point inRightSlit{304.0, 200.0};         // In right opening
    Point inCentralObstruction{200.0, 200.0}; // Center
    
    EXPECT_TRUE(checker.isVisible(inAnnulus)) << "Clear annulus region";
    EXPECT_FALSE(checker.isVisible(onSpider)) << "Blocked by spider vane";
    EXPECT_FALSE(checker.isVisible(inLeftSlit)) << "Slit opening cannot open internal";
    EXPECT_FALSE(checker.isVisible(inRightSlit)) << "Right slit same";
    EXPECT_FALSE(checker.isVisible(inCentralObstruction)) << "Central obstruction";
}

// ============================================================================
// Statistics Tracking Tests
// ============================================================================

TEST_F(VisibilityCheckerTest, StatisticsTracking) {
    ShapeCollection shapes;
    
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    shapes.addShape(std::make_unique<Ellipse>(20.0, 20.0, 100.0, 100.0, 0.0,
                    TypeLimits::INTERNAL));
    shapes.addShape(std::make_unique<Rectangle>(30.0, 30.0, 150.0, 100.0, 0.0,
                    TypeLimits::APERTURE));
    
    VisibilityChecker checker(shapes);
    
    // Check a point
    Point p{100.0, 100.0};
    checker.isVisible(p);
    
    auto stats = checker.getStats();
    EXPECT_EQ(stats.totalChecks, 1u);
    EXPECT_GT(stats.internalChecks, 0u);
    
    // Reset and check again
    checker.resetStats();
    stats = checker.getStats();
    EXPECT_EQ(stats.totalChecks, 0u);
    EXPECT_EQ(stats.internalChecks, 0u);
}

// ============================================================================
// Batch Processing Tests
// ============================================================================

TEST_F(VisibilityCheckerTest, BatchCheckPoints) {
    ShapeCollection shapes;
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    VisibilityChecker checker(shapes);
    
    std::vector<Point> points = {
        {100.0, 100.0},  // Inside
        {200.0, 200.0},  // Outside
        {120.0, 100.0},  // Inside
        {50.0, 50.0},    // Outside
    };
    
    auto results = checker.checkPoints(points);
    
    ASSERT_EQ(results.size(), 4u);
    EXPECT_TRUE(results[0]);   // Inside
    EXPECT_FALSE(results[1]);  // Outside
    EXPECT_TRUE(results[2]);   // Inside
    EXPECT_FALSE(results[3]);  // Outside
}

// ============================================================================
// Polygon Shape Tests (Important for Performance)
// ============================================================================

TEST_F(VisibilityCheckerTest, PolygonApertureOverridesExternal) {
    // Test with polygon APERTURE shape
    
    ShapeCollection shapes;
    
    // EXTERNAL: Circle
    shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // APERTURE: Polygon extending outside EXTERNAL
    std::vector<Point> vertices = {
        {140.0, 80.0},
        {160.0, 80.0},
        {160.0, 120.0},
        {140.0, 120.0}
    };
    shapes.addAperture(std::make_unique<Polygon>(vertices));
    
    VisibilityChecker checker(shapes);
    
    Point insidePolygonOutsideCircle{150.0, 100.0};
    
    // Should be visible (polygon APERTURE overrides EXTERNAL blocking)
    EXPECT_TRUE(checker.isVisible(insidePolygonOutsideCircle))
        << "Polygon APERTURE must override EXTERNAL blocking";
}

TEST_F(VisibilityCheckerTest, ComplexPolygonInternal) {
    // Test with complex polygon INTERNAL shape
    
    ShapeCollection shapes;
    
    shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0, 0.0,
                    TypeLimits::EXTERNAL));
    
    // Complex polygon obstruction
    std::vector<Point> vertices = {
        {80.0, 80.0},
        {120.0, 80.0},
        {120.0, 120.0},
        {100.0, 130.0},
        {80.0, 120.0}
    };
    shapes.addInternal(std::make_unique<Polygon>(vertices));
    
    VisibilityChecker checker(shapes);
    
    Point insidePolygon{100.0, 100.0};
    Point outsidePolygon{70.0, 100.0};
    
    EXPECT_FALSE(checker.isVisible(insidePolygon)) << "Blocked by polygon INTERNAL";
    EXPECT_TRUE(checker.isVisible(outsidePolygon)) << "Outside polygon, inside EXTERNAL";
}

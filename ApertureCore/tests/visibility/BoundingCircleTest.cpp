 /**
  * @file BoundingCircleTest.cpp
  * @brief Unit tests for ShapeCollection::getBoundingCircle()
  *
  * ## Visibility Model (Critical for Understanding Tests)
  *
  * ApertureCore uses a 3-type shape system with specific priority and semantics:
  *
  * ### Shape Types and Application Order (Construction Priority)
  * 1. **EXTERNAL** (Lowest Priority)
  *    - Masks OUTSIDE the shape as invisible
  *    - Applied first during construction
  *    - Defines main aperture boundary
  *    - Effect: Makes inside visible (if no other shape blocks)
  *
  * 2. **APERTURE** (Middle Priority)
  *    - Opens INSIDE the shape (makes visible)
  *    - Applied second
  *    - Local openings through obstructions
  *    - Can override EXTERNAL rejection
  *
  * 3. **INTERNAL** (Highest Priority)
  *    - Masks INSIDE the shape as invisible
  *    - Applied last (checked first during visibility testing!)
  *    - Obstructions/blockages
  *    - Cannot be overridden by APERTURE
  *
  * ### Visibility Algorithm (Runtime Checking - Inverse Order)
  * Point is visible if ALL conditions met:
  * 1. NOT inside any INTERNAL shape (early exit if true - blocked)
  * 2. (Inside ALL EXTERNAL shapes) OR (Inside ANY APERTURE shape)
  *
  * This means:
  * - INTERNAL checked FIRST (highest priority - acts as veto)
  * - EXTERNAL/APERTURE checked AFTER
  * - APERTURE can override EXTERNAL rejection only if INTERNAL doesn't block
  *
  * ### Critical Default Behavior
  * - **If NO EXTERNAL shapes**: Area starts INVISIBLE (only APERTURE can open it)
  * - **If YES EXTERNAL shapes**: Area starts VISIBLE (INTERNAL/APERTURE can modify)
  *
  * This enables flexibility:
  * - Option A: Use EXTERNAL to define main aperture, INTERNAL for obstructions
  * - Option B: Use APERTURE shapes as "lights" opening dark area
  * - Option C: Mix EXTERNAL + APERTURE for complex apertures with local openings
  *
  * ### For getBoundingCircle()
  * The bounding circle encompasses only VISIBLE area:
  * - Visible = points not blocked by INTERNAL and meeting EXTERNAL/APERTURE rules
  * - INTERNAL shapes shrink bounding circle (their area is never visible)
  * - EXTERNAL shapes define main boundary
  * - APERTURE shapes extend visible area (relative to EXTERNAL-only case)
  */

#include <gtest/gtest.h>
#include "aperturecore/geometry/BoundingCircle.h"
#include "aperturecore/geometry/Ellipse.h"
#include "aperturecore/geometry/Rectangle.h"
#include "aperturecore/geometry/Polygon.h"
#include "aperturecore/visibility/ShapeCollection.h"
#include "aperturecore/visibility/VisibilityChecker.h"
#include <cmath>

using namespace aperture;

namespace {
    // Tolerance for floating point comparisons
    constexpr double TOLERANCE = 1e-5;

    // Helper to check if point is approximately equal
    bool pointsNear(const Point& a, const Point& b, double tol = TOLERANCE) {
        return std::abs(a.x - b.x) < tol && std::abs(a.y - b.y) < tol;
    }

    // Helper to check if circles are approximately equal
    bool circlesNear(const BoundingCircle& a, const BoundingCircle& b, double tol = TOLERANCE) {
        if (a.valid != b.valid) return false;
        if (!a.valid) return true; // Both invalid
        return pointsNear(a.center, b.center, tol) && std::abs(a.radius - b.radius) < tol;
    }

    // Helper to check if point is inside circle (with margin for numerical errors)
    bool isInsideCircle(const Point& p, const Point& center, double radius, double margin = TOLERANCE) {
        double dist = p.distanceTo(center);
        return dist <= radius + margin;
    }
}

// ============================================================================
// Empty Collection Tests
// ============================================================================

TEST(BoundingCircleTest, EmptyCollectionReturnsInvalid) {
    ShapeCollection shapes;
    auto circle = shapes.getBoundingCircle();
    EXPECT_FALSE(circle.valid);
}

// ============================================================================
// EXTERNAL-Only Tests
// ============================================================================

TEST(BoundingCircleTest, SingleCircleEllipse) {
    ShapeCollection shapes;
    double cx = 50.0, cy = 50.0, a = 30.0, b = 30.0;
    shapes.addExternal(std::make_unique<Ellipse>(a, b, cx, cy));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_TRUE(pointsNear(circle.center, Point{ cx, cy }));
    EXPECT_NEAR(circle.radius, a, TOLERANCE);
}

TEST(BoundingCircleTest, SingleEllipseWithRotation) {
    ShapeCollection shapes;
    double cx = 0.0, cy = 0.0, a = 50.0, b = 30.0;
    shapes.addExternal(std::make_unique<Ellipse>(a, b, cx, cy, M_PI / 4.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    // Bounding circle of rotated ellipse should have radius = semi-major axis
    EXPECT_NEAR(circle.radius, a, TOLERANCE);
}

TEST(BoundingCircleTest, SingleRectangle) {
    ShapeCollection shapes;
    double cx = 0.0, cy = 0.0, w = 40.0, h = 30.0;
    shapes.addExternal(std::make_unique<Rectangle>(w, h, cx, cy));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_TRUE(pointsNear(circle.center, Point{ cx, cy }));

    // For axis-aligned rectangle, bounding circle radius = diagonal/2
    double expectedRadius = std::sqrt(w * w + h * h) / 2.0;
    EXPECT_NEAR(circle.radius, expectedRadius, TOLERANCE);
}

TEST(BoundingCircleTest, SingleRectangleRotated) {
    ShapeCollection shapes;
    double cx = 10.0, cy = 20.0, w = 40.0, h = 30.0;
    shapes.addExternal(std::make_unique<Rectangle>(w, h, cx, cy, M_PI / 6.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_TRUE(pointsNear(circle.center, Point{ cx, cy }));

    // Bounding circle radius should equal half the diagonal
    double expectedRadius = std::sqrt(w * w + h * h) / 2.0;
    EXPECT_NEAR(circle.radius, expectedRadius, TOLERANCE);
}

TEST(BoundingCircleTest, SingleSquarePolygon) {
    ShapeCollection shapes;
    std::vector<Point> vertices = {
        Point{0.0, 0.0},
        Point{10.0, 0.0},
        Point{10.0, 10.0},
        Point{0.0, 10.0}
    };
    shapes.addExternal(std::make_unique<Polygon>(vertices));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // All vertices should be inside the circle
    for (const auto& v : vertices) {
        EXPECT_TRUE(isInsideCircle(v, circle.center, circle.radius));
    }
}

TEST(BoundingCircleTest, TwoNonOverlappingExternals) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(20.0, 20.0, -50.0, 0.0));
    shapes.addExternal(std::make_unique<Ellipse>(20.0, 20.0, 50.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Should contain both circles
    EXPECT_TRUE(isInsideCircle(Point{ -70.0, 0.0 }, circle.center, circle.radius));
    EXPECT_TRUE(isInsideCircle(Point{ 70.0, 0.0 }, circle.center, circle.radius));
}

// ============================================================================
// INTERNAL-Only Tests (No EXTERNAL)
// ============================================================================

TEST(BoundingCircleTest, OnlyInternalShapesReturnsInvalid) {
    // Key: If NO EXTERNAL shapes exist, area is INVISIBLE (default)
    // INTERNAL shapes can only make things MORE invisible (shrink area)
    // Result: No visible area → Invalid bounding circle
    ShapeCollection shapes;
    shapes.addInternal(std::make_unique<Ellipse>(30.0, 30.0, 0.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_FALSE(circle.valid);  // No visible area!
}

TEST(BoundingCircleTest, OnlyMultipleInternalShapesReturnsInvalid) {
    // Multiple INTERNAL shapes without EXTERNAL: still invisible
    ShapeCollection shapes;
    shapes.addInternal(std::make_unique<Ellipse>(30.0, 30.0, 0.0, 0.0));
    shapes.addInternal(std::make_unique<Rectangle>(40.0, 40.0, 60.0, 60.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_FALSE(circle.valid);
}

// ============================================================================
// APERTURE-Only Tests (No EXTERNAL)
// ============================================================================

TEST(BoundingCircleTest, OnlyApertureShapesValid) {
    // Key: If NO EXTERNAL shapes, area is INVISIBLE (default)
    // But APERTURE shapes OPEN area, making those regions visible
    // Result: Visible area = union of APERTURE shapes
    ShapeCollection shapes;
    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);  // APERTURE opened the area!
}

TEST(BoundingCircleTest, MultipleApertureShapesNoExternal) {
    // Key: Multiple APERTUREs without EXTERNAL create union of visible regions
    ShapeCollection shapes;
    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, -30.0, -30.0));
    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, 30.0, 30.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Bounding circle should encompass both aperture regions
    EXPECT_TRUE(isInsideCircle(Point{ -30.0, -30.0 }, circle.center, circle.radius));
    EXPECT_TRUE(isInsideCircle(Point{ 30.0, 30.0 }, circle.center, circle.radius));
}

// ============================================================================
// EXTERNAL + INTERNAL Tests (Main Aperture with Obstruction)
// ============================================================================

TEST(BoundingCircleTest, ExternalWithCentralInternalObstruction) {
    // Key Scenario: Main aperture with central obstruction
    // - EXTERNAL defines boundary (area inside is visible initially)
    // - INTERNAL blocks area inside it (has highest priority in checking)
    // - Result: Visible = EXTERNAL minus INTERNAL
    ShapeCollection shapes;

    // External: Large circle - marks INSIDE as visible
    shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0));

    // Internal: Small circle at center - blocks its area
    shapes.addInternal(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Center should be blocked (inside INTERNAL)
    // But edges should be visible
    EXPECT_TRUE(isInsideCircle(Point{ 100.0, 0.0 }, circle.center, circle.radius));
    EXPECT_TRUE(isInsideCircle(Point{ -100.0, 0.0 }, circle.center, circle.radius));
}

TEST(BoundingCircleTest, ExternalWithPartialInternalObstruction) {
    // Obstruction blocks only part of external (e.g., right side)
    ShapeCollection shapes;

    shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
    // Rectangle blocks right half
    shapes.addInternal(std::make_unique<Rectangle>(60.0, 60.0, 30.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Left side should be fully visible
    Point leftEdge{ -50.0, 0.0 };
    EXPECT_TRUE(isInsideCircle(leftEdge, circle.center, circle.radius));

    // Bounding circle should be smaller than full external
    // (right side pruned by obstruction)
    double fullExternalRadius = 50.0;
    EXPECT_LT(circle.radius, fullExternalRadius);
}

TEST(BoundingCircleTest, MultipleInternalsProduceSmallerBoundingCircle) {
    // Multiple obstructions shrink visible area
    ShapeCollection withoutInternal, withInternal;

    // Both have same external
    withoutInternal.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
    withInternal.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));

    // withInternal has obstruction
    withInternal.addInternal(std::make_unique<Rectangle>(80.0, 80.0, 30.0, 0.0));

    auto circle1 = withoutInternal.getBoundingCircle();
    auto circle2 = withInternal.getBoundingCircle();

    EXPECT_TRUE(circle1.valid);
    EXPECT_TRUE(circle2.valid);

    // Obstruction should reduce bounding circle size
    EXPECT_LE(circle2.radius, circle1.radius + TOLERANCE);
}

// ============================================================================
// EXTERNAL + APERTURE Tests (Complex Apertures)
// ============================================================================

TEST(BoundingCircleTest, ExternalWithApertureOpening) {
    // Key: APERTURE cannot extend beyond EXTERNAL
    // - EXTERNAL defines main boundary
    // - APERTURE is additional opening (union logic)
    ShapeCollection shapes;

    shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
    shapes.addAperture(std::make_unique<Rectangle>(20.0, 40.0, 60.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Aperture should extend visible area beyond what EXTERNAL alone would give
    // if positioned at boundary
    Point apertureCenter{ 60.0, 0.0 };
    EXPECT_TRUE(isInsideCircle(apertureCenter, circle.center, circle.radius));
}

// ============================================================================
// EXTERNAL + INTERNAL + APERTURE Tests (Full Complexity)
// ============================================================================

TEST(BoundingCircleTest, ExternalWithObstructionAndOpening) {
    // Most complex scenario: All three types
    // - EXTERNAL: main boundary
    // - INTERNAL: obstruction
    // - APERTURE: opening through obstruction
    ShapeCollection shapes;

    // Main aperture
    shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0));

    // Central obstruction
    shapes.addInternal(std::make_unique<Ellipse>(40.0, 40.0, 0.0, 0.0));

    // Small opening through obstruction
    shapes.addAperture(std::make_unique<Rectangle>(5.0, 30.0, 0.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);

    // Outside boundary should be blocked (outside EXTERNAL)
    // Inside obstruction should be blocked (inside INTERNAL)
    // But opening should let through

    // Edge of external should be visible
    EXPECT_TRUE(isInsideCircle(Point{ 100.0, 0.0 }, circle.center, circle.radius));
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(BoundingCircleTest, VerySmallRadius) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(1e-6, 1e-6, 0.0, 0.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_GT(circle.radius, 0.0);
}

TEST(BoundingCircleTest, LargeCoordinates) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 1e6, 1e6));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_NEAR(circle.center.x, 1e6, 100.0);
    EXPECT_NEAR(circle.center.y, 1e6, 100.0);
}

TEST(BoundingCircleTest, NegativeCoordinates) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Rectangle>(40.0, 30.0, -50.0, -60.0));

    auto circle = shapes.getBoundingCircle();
    EXPECT_TRUE(circle.valid);
    EXPECT_TRUE(pointsNear(circle.center, Point{ -50.0, -60.0 }));
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST(BoundingCircleTest, VersionNotModifiedByGetBoundingCircle) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));
    uint64_t v1 = shapes.getVersion();

    auto circle = shapes.getBoundingCircle();
    uint64_t v2 = shapes.getVersion();

    // getBoundingCircle should not modify collection
    EXPECT_EQ(v1, v2);
}

TEST(BoundingCircleTest, RepeatedCallsProduceSameResult) {
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(30.0, 30.0, 10.0, 20.0));
    shapes.addInternal(std::make_unique<Rectangle>(20.0, 20.0, -10.0, -20.0));

    auto circle1 = shapes.getBoundingCircle();
    auto circle2 = shapes.getBoundingCircle();

    EXPECT_TRUE(circlesNear(circle1, circle2));
}

TEST(BoundingCircleTest, DeterministicShuffling) {
    // Fixed seed ensures reproducible results across calls
    ShapeCollection shapes;
    shapes.addExternal(std::make_unique<Ellipse>(40.0, 40.0, 5.0, 10.0));
    shapes.addExternal(std::make_unique<Rectangle>(30.0, 30.0, -30.0, -30.0));

    auto circle1 = shapes.getBoundingCircle();
    auto circle2 = shapes.getBoundingCircle();

    // Fixed seed (0xD16D1234u) ensures deterministic Welzl results
    EXPECT_TRUE(circlesNear(circle1, circle2));
}

//using namespace aperture;
//
//namespace {
//    // Tolerance for floating point comparisons
//    constexpr double TOLERANCE = 1e-2;
//
//    // Helper to check if point is approximately equal
//    bool pointsNear(const Point& a, const Point& b, double tol = TOLERANCE) {
//        return std::abs(a.x - b.x) < tol && std::abs(a.y - b.y) < tol;
//    }
//
//    // Helper to check if circles are approximately equal
//    bool circlesNear(const BoundingCircle& a, const BoundingCircle& b, double tol = TOLERANCE) {
//        if (a.valid != b.valid) return false;
//        if (!a.valid) return true; // Both invalid
//        return pointsNear(a.center, b.center, tol) && std::abs(a.radius - b.radius) < tol;
//    }
//
//    // Helper to check if point is inside circle (with margin for numerical errors)
//    bool isInsideCircle(const Point& p, const Point& center, double radius, double margin = TOLERANCE) {
//        double dist = p.distanceTo(center);
//        return dist <= radius + margin;
//    }
//}
//
//// ============================================================================
//// Empty Collection Tests
//// ============================================================================
//
//TEST(BoundingCircleTest, EmptyCollectionReturnsInvalid) {
//    ShapeCollection shapes;
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_FALSE(circle.valid);
//}
//
//// ============================================================================
//// Single Shape Tests
//// ============================================================================
//
//TEST(BoundingCircleTest, SingleCircleEllipse) {
//    ShapeCollection shapes;
//    double cx = 50.0, cy = 50.0, a = 30.0, b = 30.0;
//    shapes.addExternal(std::make_unique<Ellipse>(a, b, cx, cy));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_TRUE(pointsNear(circle.center, Point{cx, cy}));
//    EXPECT_NEAR(circle.radius, a, TOLERANCE);
//}
//
//TEST(BoundingCircleTest, SingleEllipseWithRotation) {
//    ShapeCollection shapes;
//    double cx = 0.0, cy = 0.0, a = 50.0, b = 30.0;
//    shapes.addExternal(std::make_unique<Ellipse>(a, b, cx, cy, M_PI / 4.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    // Bounding circle of rotated ellipse should have radius = semi-major axis
//    EXPECT_NEAR(circle.radius, a, TOLERANCE);
//}
//
//TEST(BoundingCircleTest, SingleRectangle) {
//    ShapeCollection shapes;
//    double cx = 0.0, cy = 0.0, w = 40.0, h = 30.0;
//    shapes.addExternal(std::make_unique<Rectangle>(w, h, cx, cy));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_TRUE(pointsNear(circle.center, Point{cx, cy}));
//
//    // For axis-aligned rectangle, bounding circle radius = diagonal/2
//    double expectedRadius = std::sqrt(w * w + h * h) / 2.0;
//    EXPECT_NEAR(circle.radius, expectedRadius, TOLERANCE);
//}
//
//TEST(BoundingCircleTest, SingleRectangleRotated) {
//    ShapeCollection shapes;
//    double cx = 10.0, cy = 20.0, w = 40.0, h = 30.0;
//    shapes.addExternal(std::make_unique<Rectangle>(w, h, cx, cy, M_PI / 6.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_TRUE(pointsNear(circle.center, Point{cx, cy}));
//
//    // Bounding circle radius should equal half the diagonal
//    double expectedRadius = std::sqrt(w * w + h * h) / 2.0;
//    EXPECT_NEAR(circle.radius, expectedRadius, TOLERANCE);
//}
//
//TEST(BoundingCircleTest, SingleSquarePolygon) {
//    ShapeCollection shapes;
//    std::vector<Point> vertices = {
//        Point{0.0, 0.0},
//        Point{10.0, 0.0},
//        Point{10.0, 10.0},
//        Point{0.0, 10.0}
//    };
//    shapes.addExternal(std::make_unique<Polygon>(vertices));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // All vertices should be inside the circle
//    for (const auto& v : vertices) {
//        EXPECT_TRUE(isInsideCircle(v, circle.center, circle.radius));
//    }
//}
//
//// ============================================================================
//// Multiple Shapes Tests
//// ============================================================================
//
//TEST(BoundingCircleTest, TwoNonOverlappingCircles) {
//    ShapeCollection shapes;
//    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, -50.0, 0.0));
//    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, 50.0, 0.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // Should contain both circles
//    EXPECT_TRUE(isInsideCircle(Point{-69.9, 0.0}, circle.center, circle.radius));
//    EXPECT_TRUE(isInsideCircle(Point{69.9, 0.0}, circle.center, circle.radius));
//}
//
//TEST(BoundingCircleTest, ThreeRectangles) {
//    ShapeCollection shapes;
//    shapes.addAperture(std::make_unique<Rectangle>(20.0, 20.0, 0.0, 0.0));
//    shapes.addAperture(std::make_unique<Rectangle>(20.0, 20.0, 50.0, 50.0));
//    shapes.addAperture(std::make_unique<Rectangle>(20.0, 20.0, -50.0, -50.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // All rectangles should be enclosed
//    Point corners[] = {
//        Point{0.0, 0.0},    // Center rect
//        Point{50.0, 50.0},  // Second rect
//        Point{-50.0, -50.0} // Aperture rect
//    };
//
//    for (const auto& c : corners) {
//        EXPECT_TRUE(isInsideCircle(c, circle.center, circle.radius));
//    }
//}
//
//TEST(BoundingCircleTest, MixedShapeTypes) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(30.0, 30.0, 0.0, 0.0));
//    shapes.addAperture(std::make_unique<Rectangle>(20.0, 20.0, 40.0, 40.0));
//
//    std::vector<Point> polyVerts = {
//        Point{-50.0, 0.0},
//        Point{-30.0, 30.0},
//        Point{-10.0, -30.0}
//    };
//    shapes.addExternal(std::make_unique<Polygon>(polyVerts));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // All shape centers should be enclosed
//    EXPECT_TRUE(isInsideCircle(Point{0.0, 0.0}, circle.center, circle.radius));
//    EXPECT_TRUE(isInsideCircle(Point{40.0, 40.0}, circle.center, circle.radius));
//}
//
//// ============================================================================
//// Internal Shapes (Obstruction) Tests - Key Insight
//// ============================================================================
//
//TEST(BoundingCircleTest, InternalShapesNotIncluded) {
//    // INTERNAL shapes shrink the visible area, they don't extend it.
//    // Therefore, they should NOT contribute to the bounding circle.
//    ShapeCollection shapes;
//    
//    // External: Large circle
//    shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0));
//    
//    // Internal obstruction: Small circle in center
//    shapes.addInternal(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // Bounding circle should be determined by external shape only
//    // Internal shape points should be pruned by VisibilityChecker
//    EXPECT_TRUE(isInsideCircle(Point{100.0, 0.0}, circle.center, circle.radius));
//    EXPECT_TRUE(isInsideCircle(Point{-100.0, 0.0}, circle.center, circle.radius));
//}
//
//TEST(BoundingCircleTest, ObstructionPrunesContourPoints) {
//    ShapeCollection shapes;
//    
//    // External: circle at (0, 0)
//    shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
//    
//    // Internal: obstruction that blocks right half
//    shapes.addInternal(std::make_unique<Rectangle>(100.0, 100.0, 30.0, 0.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//
//    // Left side of external should be visible and included
//    Point leftEdge{-50.0, 0.0};
//    EXPECT_TRUE(isInsideCircle(leftEdge, circle.center, circle.radius));
//    
//    // Bounding circle should be smaller than if computed from full external shape
//    // (because right side is pruned)
//    double fullExternalRadius = 50.0;
//    EXPECT_LT(circle.radius, fullExternalRadius);
//}
//
//TEST(BoundingCircleTest, MultipleObstructionsProduceSmallerBoundingCircle) {
//    ShapeCollection shapes1, shapes2;
//    
//    // Both have external circle
//    shapes1.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
//    shapes2.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 0.0, 0.0));
//    
//    // shapes2 has obstruction blocking part of external
//    shapes2.addInternal(std::make_unique<Rectangle>(80.0, 80.0, 30.0, 0.0));
//
//    auto circle1 = shapes1.getBoundingCircle();
//    auto circle2 = shapes2.getBoundingCircle();
//
//    EXPECT_TRUE(circle1.valid);
//    EXPECT_TRUE(circle2.valid);
//    
//    // Circle with obstruction should be no larger than without
//    EXPECT_LE(circle2.radius, circle1.radius + TOLERANCE);
//}
//
//// ============================================================================
//// Type-Specific Tests
//// ============================================================================
//
//TEST(BoundingCircleTest, OnlyExternalShapes) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(25.0, 25.0, -20.0, 0.0));
//    shapes.addExternal(std::make_unique<Ellipse>(25.0, 25.0, 20.0, 0.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_TRUE(circle.radius > 0.0);
//}
//
//TEST(BoundingCircleTest, OnlyApertureShapes) {
//    ShapeCollection shapes;
//    shapes.addAperture(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));
//    shapes.addAperture(std::make_unique<Rectangle>(30.0, 30.0, 50.0, 50.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//}
//
//TEST(BoundingCircleTest, OnlyInternalShapesReturnsInvalid) {
//    // INTERNAL-only collection has no visible area
//    ShapeCollection shapes;
//    shapes.addInternal(std::make_unique<Ellipse>(30.0, 30.0, 0.0, 0.0));
//    shapes.addInternal(std::make_unique<Rectangle>(40.0, 40.0, 60.0, 60.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_FALSE(circle.valid);  // No visible area = no bounding circle
//}
//
//// ============================================================================
//// Edge Cases
//// ============================================================================
//
//TEST(BoundingCircleTest, VerySmallRadius) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(1e-5, 1e-5, 0.0, 0.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_GT(circle.radius, 0.0);
//}
//
//TEST(BoundingCircleTest, LargeCoordinates) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 1e6, 1e6));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_NEAR(circle.center.x, 1e6, 100.0);
//    EXPECT_NEAR(circle.center.y, 1e6, 100.0);
//}
//
//TEST(BoundingCircleTest, NegativeCoordinates) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Rectangle>(40.0, 30.0, -50.0, -60.0));
//
//    auto circle = shapes.getBoundingCircle();
//    EXPECT_TRUE(circle.valid);
//    EXPECT_TRUE(pointsNear(circle.center, Point{-50.0, -60.0}));
//}
//
//// ============================================================================
//// Consistency Tests
//// ============================================================================
//
//TEST(BoundingCircleTest, VersionNotAffected) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(20.0, 20.0, 0.0, 0.0));
//    uint64_t v1 = shapes.getVersion();
//
//    auto circle = shapes.getBoundingCircle();
//    uint64_t v2 = shapes.getVersion();
//
//    // getBoundingCircle should not modify collection
//    EXPECT_EQ(v1, v2);
//}
//
//TEST(BoundingCircleTest, RepeatedCallsSameResult) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(30.0, 30.0, 10.0, 20.0));
//    shapes.addInternal(std::make_unique<Rectangle>(20.0, 20.0, -10.0, -20.0));
//
//    auto circle1 = shapes.getBoundingCircle();
//    auto circle2 = shapes.getBoundingCircle();
//
//    EXPECT_TRUE(circlesNear(circle1, circle2));
//}
//
//TEST(BoundingCircleTest, DeterministicShuffling) {
//    ShapeCollection shapes;
//    shapes.addExternal(std::make_unique<Ellipse>(40.0, 40.0, 5.0, 10.0));
//    shapes.addExternal(std::make_unique<Rectangle>(30.0, 30.0, -30.0, -30.0));
//
//    auto circle1 = shapes.getBoundingCircle();
//    auto circle2 = shapes.getBoundingCircle();
//
//    // Fixed seed (0xD16D1234u) ensures deterministic results
//    EXPECT_TRUE(circlesNear(circle1, circle2));
//}

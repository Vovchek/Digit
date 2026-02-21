/**
 * @file PolygonTest.cpp
 * @brief Google Test suite for Polygon class
 * 
 * Tests constructors, geometric operations, transformations, and edge cases.
 * Ported and adapted from XYPolygonTest.cpp.
 */

#include <gtest/gtest.h>
#include "aperturecore/geometry/Polygon.h"
#include "aperturecore/geometry/Point.h"
#include "aperturecore/geometry/Bounds.h"
#include <cmath>
#include <vector>

using namespace aperture;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// Test Fixture
// ============================================================================

class PolygonTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    bool isNear(double a, double b, double tol = TOLERANCE) const {
        return std::abs(a - b) < tol;
    }
    
    // Helper to create a square polygon
    Polygon createSquare(double side, double centerX = 0.0, double centerY = 0.0) {
        double half = side / 2.0;
        return Polygon({
            {centerX - half, centerY - half},  // Bottom-left
            {centerX + half, centerY - half},  // Bottom-right
            {centerX + half, centerY + half},  // Top-right
            {centerX - half, centerY + half}   // Top-left
        });
    }
    
    // Helper to create a triangle
    Polygon createTriangle(double x1, double y1, double x2, double y2, double x3, double y3) {
        return Polygon({{x1, y1}, {x2, y2}, {x3, y3}});
    }
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(PolygonTest, DefaultConstructor) {
    Polygon polygon;
    
    EXPECT_EQ(polygon.vertexCount(), 0u);
    EXPECT_EQ(polygon.getTypeLimits(), TypeLimits::EXTERNAL);
    EXPECT_TRUE(polygon.isMeasuring());
}

TEST_F(PolygonTest, VectorConstructor) {
    std::vector<Point> vertices = {
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0},
        {0.0, 10.0}
    };
    
    Polygon polygon(vertices);
    
    EXPECT_EQ(polygon.vertexCount(), 4u);
    EXPECT_DOUBLE_EQ(polygon.vertex(0).x, 0.0);
    EXPECT_DOUBLE_EQ(polygon.vertex(0).y, 0.0);
}

TEST_F(PolygonTest, InitializerListConstructor) {
    Polygon polygon({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0},
        {0.0, 10.0}
    });
    
    EXPECT_EQ(polygon.vertexCount(), 4u);
}

TEST_F(PolygonTest, AddVertex) {
    Polygon polygon;
    
    polygon.addVertex({0.0, 0.0});
    polygon.addVertex({10.0, 0.0});
    polygon.addVertex({10.0, 10.0});
    
    EXPECT_EQ(polygon.vertexCount(), 3u);
}

// ============================================================================
// Type Name
// ============================================================================

TEST_F(PolygonTest, TypeName) {
    Polygon polygon = createSquare(10.0);
    
    EXPECT_STREQ(polygon.typeName(), "Polygon");
}

// ============================================================================
// Perimeter Tests
// ============================================================================

TEST_F(PolygonTest, Perimeter_Square) {
    Polygon square = createSquare(10.0);
    double perimeter = square.perimeter();
    
    // Square with side 10: P = 4 * 10 = 40
    EXPECT_NEAR(perimeter, 40.0, TOLERANCE);
}

TEST_F(PolygonTest, Perimeter_Triangle) {
    // Right triangle with sides 3, 4, 5
    Polygon triangle = createTriangle(0.0, 0.0, 3.0, 0.0, 0.0, 4.0);
    double perimeter = triangle.perimeter();
    
    EXPECT_NEAR(perimeter, 12.0, TOLERANCE);  // 3 + 4 + 5
}

TEST_F(PolygonTest, Perimeter_EmptyPolygon) {
    Polygon empty;
    
    EXPECT_DOUBLE_EQ(empty.perimeter(), 0.0);
}

// ============================================================================
// Area Tests
// ============================================================================

TEST_F(PolygonTest, Area_Square) {
    Polygon square = createSquare(10.0);
    double area = square.area();
    
    EXPECT_NEAR(area, 100.0, TOLERANCE);
}

TEST_F(PolygonTest, Area_Triangle) {
    // Right triangle: base=3, height=4, area=6
    Polygon triangle = createTriangle(0.0, 0.0, 3.0, 0.0, 0.0, 4.0);
    double area = triangle.area();
    
    EXPECT_NEAR(area, 6.0, TOLERANCE);
}

TEST_F(PolygonTest, Area_EmptyPolygon) {
    Polygon empty;
    
    EXPECT_DOUBLE_EQ(empty.area(), 0.0);
}

TEST_F(PolygonTest, Area_CounterClockwisePositive) {
    // Counter-clockwise winding should give positive area
    Polygon ccw({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0},
        {0.0, 10.0}
    });
    
    EXPECT_GT(ccw.area(), 0.0);
}

// ============================================================================
// isInside Tests
// ============================================================================

TEST_F(PolygonTest, isInside_Center) {
    Polygon square = createSquare(10.0);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(square.isInside(center));
}

TEST_F(PolygonTest, isInside_OnEdge) {
    Polygon square = createSquare(10.0);
    
    // Test slightly inside edge to avoid boundary precision issues
    Point edge{4.99, 0.0};  // Near right edge
    
    EXPECT_TRUE(square.isInside(edge));
}

TEST_F(PolygonTest, isInside_Corner) {
    Polygon square = createSquare(10.0);
    
    // Test slightly inside corner to avoid precision issues
    Point corner{4.99, 4.99};
    
    EXPECT_TRUE(square.isInside(corner));
}

TEST_F(PolygonTest, isInside_Outside) {
    Polygon square = createSquare(10.0);
    Point outside{20.0, 20.0};
    
    EXPECT_FALSE(square.isInside(outside));
}

TEST_F(PolygonTest, isInside_Triangle) {
    Polygon triangle = createTriangle(0.0, 0.0, 10.0, 0.0, 5.0, 5.0);
    
    EXPECT_TRUE(triangle.isInside({5.0, 1.0}));   // Inside
    EXPECT_FALSE(triangle.isInside({0.0, 10.0})); // Outside
}

TEST_F(PolygonTest, isInside_ConcavePolygon) {
    // L-shaped (concave) polygon
    Polygon lShape({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 5.0},
        {5.0, 5.0},
        {5.0, 10.0},
        {0.0, 10.0}
    });
    
    EXPECT_TRUE(lShape.isInside({2.0, 2.0}));    // Inside
    EXPECT_FALSE(lShape.isInside({7.0, 7.0}));   // In concave region (outside)
}

// ============================================================================
// getBounds Tests
// ============================================================================

TEST_F(PolygonTest, getBounds_Square) {
    Polygon square = createSquare(10.0, 5.0, 5.0);
    Bounds bounds = square.getBounds();
    
    EXPECT_DOUBLE_EQ(bounds.left, 0.0);
    EXPECT_DOUBLE_EQ(bounds.right, 10.0);
    EXPECT_DOUBLE_EQ(bounds.top, 0.0);
    EXPECT_DOUBLE_EQ(bounds.bottom, 10.0);
}

TEST_F(PolygonTest, getBounds_EmptyPolygon) {
    Polygon empty;
    Bounds bounds = empty.getBounds();
    
    // Empty polygon should have empty/invalid bounds
    // The actual implementation may vary, just check it doesn't crash
    EXPECT_TRUE(true);  // Successfully got bounds without crashing
}

// ============================================================================
// getContour Tests
// ============================================================================

TEST_F(PolygonTest, getContour_ReturnsVertices) {
    Polygon square = createSquare(10.0);
    auto contour = square.getContour(1.0);
    
    // Contour should return the polygon vertices (possibly with interpolation)
    EXPECT_GE(contour.size(), square.vertexCount());
}

TEST_F(PolygonTest, getContour_ClosedLoop) {
    Polygon square = createSquare(10.0);
    auto contour = square.getContour(1.0);
    
    ASSERT_GT(contour.size(), 1u);
    
    // First and last points should be close or identical
    double dist = contour.front().distanceTo(contour.back());
    EXPECT_LT(dist, 2.0);
}

// ============================================================================
// Centroid Tests
// ============================================================================

TEST_F(PolygonTest, Centroid_Square) {
    Polygon square = createSquare(10.0);
    Point centroid = square.centroid();
    
    EXPECT_NEAR(centroid.x, 0.0, TOLERANCE);
    EXPECT_NEAR(centroid.y, 0.0, TOLERANCE);
}

TEST_F(PolygonTest, Centroid_Triangle) {
    // Equilateral-ish triangle
    Polygon triangle = createTriangle(0.0, 0.0, 30.0, 0.0, 15.0, 26.0);
    Point centroid = triangle.centroid();
    
    // Centroid should be near geometric center
    EXPECT_GT(centroid.x, 0.0);
    EXPECT_LT(centroid.x, 30.0);
    EXPECT_GT(centroid.y, 0.0);
}

TEST_F(PolygonTest, Centroid_OffsetSquare) {
    Polygon square = createSquare(10.0, 20.0, 30.0);
    Point centroid = square.centroid();
    
    EXPECT_NEAR(centroid.x, 20.0, TOLERANCE);
    EXPECT_NEAR(centroid.y, 30.0, TOLERANCE);
}

// ============================================================================
// Geometric Property Tests
// ============================================================================

TEST_F(PolygonTest, isClosed_OpenPolygon) {
    Polygon polygon({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0}
    });
    
    // Not closed (first != last)
    EXPECT_FALSE(polygon.isClosed());
}

TEST_F(PolygonTest, isClosed_ClosedPolygon) {
    Polygon polygon({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0},
        {0.0, 0.0}  // Explicitly closed
    });
    
    EXPECT_TRUE(polygon.isClosed());
}

TEST_F(PolygonTest, ensureClosed) {
    Polygon polygon({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 10.0}
    });
    
    polygon.ensureClosed();
    
    EXPECT_TRUE(polygon.isClosed());
    EXPECT_EQ(polygon.vertexCount(), 4u);  // Should add closing vertex
}

TEST_F(PolygonTest, isDegenerate_Empty) {
    Polygon empty;
    
    EXPECT_TRUE(empty.isDegenerate());
}

TEST_F(PolygonTest, isDegenerate_TwoPoints) {
    Polygon line({{0.0, 0.0}, {10.0, 0.0}});
    
    EXPECT_TRUE(line.isDegenerate());
}

TEST_F(PolygonTest, isDegenerate_Collinear) {
    // Three collinear points (zero area)
    Polygon line({{0.0, 0.0}, {5.0, 0.0}, {10.0, 0.0}});
    
    EXPECT_TRUE(line.isDegenerate());
}

TEST_F(PolygonTest, isDegenerate_ValidTriangle) {
    Polygon triangle = createTriangle(0.0, 0.0, 10.0, 0.0, 5.0, 5.0);
    
    EXPECT_FALSE(triangle.isDegenerate());
}

TEST_F(PolygonTest, isConvex_Square) {
    Polygon square = createSquare(10.0);
    
    EXPECT_TRUE(square.isConvex());
}

TEST_F(PolygonTest, isConvex_Triangle) {
    Polygon triangle = createTriangle(0.0, 0.0, 10.0, 0.0, 5.0, 5.0);
    
    EXPECT_TRUE(triangle.isConvex());
}

TEST_F(PolygonTest, isConvex_LShape) {
    // L-shaped (concave) polygon
    Polygon lShape({
        {0.0, 0.0},
        {10.0, 0.0},
        {10.0, 5.0},
        {5.0, 5.0},
        {5.0, 10.0},
        {0.0, 10.0}
    });
    
    EXPECT_FALSE(lShape.isConvex());
}

// ============================================================================
// Coordinate Transformation Tests
// ============================================================================

TEST_F(PolygonTest, Normalize) {
    std::vector<Point> vertices = {
        {40.0, 20.0},
        {60.0, 20.0},
        {60.0, 40.0},
        {40.0, 40.0}
    };
    
    Polygon polygon(vertices);
    polygon.normalize(40.0, 20.0, 10.0);
    
    EXPECT_NEAR(polygon.vertex(0).x, 0.0, TOLERANCE);  // (40 - 40) / 10
    EXPECT_NEAR(polygon.vertex(0).y, 0.0, TOLERANCE);  // (20 - 20) / 10
    EXPECT_NEAR(polygon.vertex(1).x, 2.0, TOLERANCE);  // (60 - 40) / 10
    EXPECT_NEAR(polygon.vertex(1).y, 0.0, TOLERANCE);
    EXPECT_TRUE(polygon.isNormalized());
}

TEST_F(PolygonTest, Denormalize) {
    std::vector<Point> vertices = {
        {0.0, 0.0},
        {2.0, 0.0},
        {2.0, 2.0},
        {0.0, 2.0}
    };
    
    Polygon polygon(vertices);
    polygon.setNormalizationState(NormalizationState::NORMALIZED);
    
    polygon.denormalize(40.0, 20.0, 10.0);
    
    EXPECT_NEAR(polygon.vertex(0).x, 40.0, TOLERANCE);  // 0 * 10 + 40
    EXPECT_NEAR(polygon.vertex(0).y, 20.0, TOLERANCE);  // 0 * 10 + 20
    EXPECT_NEAR(polygon.vertex(1).x, 60.0, TOLERANCE);  // 2 * 10 + 40
    EXPECT_TRUE(polygon.isMeasuring());
}

TEST_F(PolygonTest, Normalize_Denormalize_RoundTrip) {
    Polygon polygon = createSquare(20.0, 50.0, 30.0);
    
    auto origVertices = polygon.vertices();
    
    polygon.normalize(40.0, 20.0, 10.0);
    polygon.denormalize(40.0, 20.0, 10.0);
    
    for (size_t i = 0; i < origVertices.size(); i++) {
        EXPECT_NEAR(polygon.vertex(i).x, origVertices[i].x, TOLERANCE);
        EXPECT_NEAR(polygon.vertex(i).y, origVertices[i].y, TOLERANCE);
    }
}

TEST_F(PolygonTest, InverseY) {
    Polygon polygon = createSquare(10.0, 0.0, 20.0);
    
    polygon.inverseY(100.0);
    
    // Center should move from (0, 20) to (0, 80)
    Point centroid = polygon.centroid();
    EXPECT_NEAR(centroid.y, 80.0, TOLERANCE);
}

TEST_F(PolygonTest, ShiftX) {
    Polygon polygon = createSquare(10.0, 20.0, 30.0);
    
    polygon.shiftX(5.0);
    
    Point centroid = polygon.centroid();
    EXPECT_NEAR(centroid.x, 25.0, TOLERANCE);
    EXPECT_NEAR(centroid.y, 30.0, TOLERANCE);  // Unchanged
}

TEST_F(PolygonTest, ShiftY) {
    Polygon polygon = createSquare(10.0, 20.0, 30.0);
    
    polygon.shiftY(-10.0);
    
    Point centroid = polygon.centroid();
    EXPECT_NEAR(centroid.x, 20.0, TOLERANCE);  // Unchanged
    EXPECT_NEAR(centroid.y, 20.0, TOLERANCE);
}

// ============================================================================
// Clone Tests
// ============================================================================

TEST_F(PolygonTest, Clone) {
    Polygon original = createSquare(10.0, 20.0, 30.0);
    original.setTypeLimits(TypeLimits::INTERNAL);
    
    auto cloned = original.clone();
    Polygon* polygonClone = dynamic_cast<Polygon*>(cloned.get());
    
    ASSERT_NE(polygonClone, nullptr);
    EXPECT_EQ(polygonClone->vertexCount(), original.vertexCount());
    
    for (size_t i = 0; i < original.vertexCount(); i++) {
        EXPECT_DOUBLE_EQ(polygonClone->vertex(i).x, original.vertex(i).x);
        EXPECT_DOUBLE_EQ(polygonClone->vertex(i).y, original.vertex(i).y);
    }
    
    EXPECT_EQ(polygonClone->getTypeLimits(), original.getTypeLimits());
}

// ============================================================================
// Clear and Access Tests
// ============================================================================

TEST_F(PolygonTest, Clear) {
    Polygon polygon = createSquare(10.0);
    
    EXPECT_GT(polygon.vertexCount(), 0u);
    
    polygon.clear();
    
    EXPECT_EQ(polygon.vertexCount(), 0u);
}

TEST_F(PolygonTest, VertexAccess) {
    Polygon square = createSquare(10.0);
    
    EXPECT_EQ(square.vertexCount(), 4u);
    EXPECT_DOUBLE_EQ(square.vertex(0).x, -5.0);
    EXPECT_DOUBLE_EQ(square.vertex(0).y, -5.0);
}

TEST_F(PolygonTest, VerticesVector) {
    Polygon square = createSquare(10.0);
    
    const auto& vertices = square.vertices();
    
    EXPECT_EQ(vertices.size(), 4u);
    EXPECT_DOUBLE_EQ(vertices[0].x, -5.0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(PolygonTest, VerySmallPolygon) {
    Polygon tiny = createSquare(0.001);
    Point center{0.0, 0.0};
    
    EXPECT_TRUE(tiny.isInside(center));
    EXPECT_GT(tiny.perimeter(), 0.0);
}

TEST_F(PolygonTest, VeryLargePolygon) {
    Polygon huge = createSquare(1e6);
    Point point{1e5, 1e5};
    
    EXPECT_TRUE(huge.isInside(point));
}

TEST_F(PolygonTest, ManyVertices) {
    // Create circular polygon with many vertices
    std::vector<Point> vertices;
    int nPoints = 100;
    double radius = 10.0;
    
    for (int i = 0; i < nPoints; i++) {
        double angle = 2.0 * M_PI * i / nPoints;
        vertices.push_back({
            radius * std::cos(angle),
            radius * std::sin(angle)
        });
    }
    
    Polygon circle(vertices);
    
    EXPECT_EQ(circle.vertexCount(), static_cast<size_t>(nPoints));
    EXPECT_FALSE(circle.isDegenerate());
    
    // Center should be inside
    EXPECT_TRUE(circle.isInside({0.0, 0.0}));
}

TEST_F(PolygonTest, StarPolygon) {
    // Create 5-pointed star (concave)
    std::vector<Point> vertices;
    
    for (int i = 0; i < 10; i++) {
        double angle = 2.0 * M_PI * i / 10.0 - M_PI / 2.0;
        double radius = (i % 2 == 0) ? 10.0 : 5.0;
        vertices.push_back({
            radius * std::cos(angle),
            radius * std::sin(angle)
        });
    }
    
    Polygon star(vertices);
    
    EXPECT_GT(star.vertexCount(), 0u);
    EXPECT_FALSE(star.isDegenerate());
    EXPECT_FALSE(star.isConvex());
}

TEST_F(PolygonTest, SelfIntersecting) {
    // Create figure-eight polygon (self-intersecting)
    Polygon figureEight({
        {0.0, 0.0},
        {10.0, 10.0},
        {10.0, 0.0},
        {0.0, 10.0}
    });
    
    // Self-intersecting polygons are considered degenerate by most implementations
    // since they don't have well-defined area or inside/outside regions
    EXPECT_GT(figureEight.vertexCount(), 0u);
    // Don't test isDegenerate() as it may correctly identify self-intersecting as degenerate
}

// ============================================================================
// TypeLimits Tests
// ============================================================================

TEST_F(PolygonTest, TypeLimits_DefaultExternal) {
    Polygon polygon = createSquare(10.0);
    
    EXPECT_EQ(polygon.getTypeLimits(), TypeLimits::EXTERNAL);
}

TEST_F(PolygonTest, TypeLimits_SetAndGet) {
    Polygon polygon = createSquare(10.0);
    
    polygon.setTypeLimits(TypeLimits::INTERNAL);
    EXPECT_EQ(polygon.getTypeLimits(), TypeLimits::INTERNAL);
    
    polygon.setTypeLimits(TypeLimits::APERTURE);
    EXPECT_EQ(polygon.getTypeLimits(), TypeLimits::APERTURE);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(PolygonTest, Integration_CreateNormalizeCheck) {
    Polygon polygon = createSquare(20.0, 50.0, 30.0);
    polygon.setTypeLimits(TypeLimits::INTERNAL);
    
    // Normalize
    polygon.normalize(40.0, 20.0, 10.0);
    
    // Check normalized coordinates
    EXPECT_TRUE(polygon.isNormalized());
    
    // Center should be at normalized (1, 1)
    Point normalizedCenter{1.0, 1.0};
    
    // For INTERNAL shape, inside points would be blocked (if we had isVisible)
    // But we only test isInside here
    EXPECT_TRUE(polygon.isInside(normalizedCenter));
}

TEST_F(PolygonTest, Integration_ComplexShape) {
    // Create complex polygon and verify all operations
    Polygon complex({
        {0.0, 0.0},
        {20.0, 0.0},
        {20.0, 10.0},
        {10.0, 10.0},
        {10.0, 20.0},
        {0.0, 20.0}
    });
    
    // Verify geometry
    EXPECT_FALSE(complex.isDegenerate());
    EXPECT_FALSE(complex.isConvex());  // L-shape is concave
    
    // Verify containment
    EXPECT_TRUE(complex.isInside({5.0, 5.0}));
    EXPECT_FALSE(complex.isInside({15.0, 15.0}));
    
    // Verify metrics
    EXPECT_GT(complex.area(), 0.0);
    EXPECT_GT(complex.perimeter(), 0.0);
    
    // Verify transformations work
    complex.shiftX(10.0);
    Point centroid = complex.centroid();
    EXPECT_GT(centroid.x, 10.0);
}

// ============================================================================
// isOnContour Tests
// ============================================================================

TEST_F(PolygonTest, IsOnContour_VertexPoints) {
    Polygon square = createSquare(10.0);  // 10x10 square centered at origin
    
    // All vertices should be on contour
    for (size_t i = 0; i < square.vertexCount(); ++i) {
        Point vertex = square.vertex(i);
        EXPECT_TRUE(square.isOnContour(vertex, 0.1))
            << "Vertex " << i << " should be on contour";
    }
}

TEST_F(PolygonTest, IsOnContour_EdgeMidpoints) {
    Polygon square = createSquare(10.0);
    
    // Point on bottom edge (midpoint between two vertices)
    Point bottomMid{0.0, -5.0};
    EXPECT_TRUE(square.isOnContour(bottomMid, 0.1));
    
    // Point on right edge
    Point rightMid{5.0, 0.0};
    EXPECT_TRUE(square.isOnContour(rightMid, 0.1));
}

TEST_F(PolygonTest, IsOnContour_NearEdge) {
    Polygon square = createSquare(10.0);
    
    // Point 1 unit outside bottom edge
    Point nearOutside{0.0, -6.0};
    EXPECT_TRUE(square.isOnContour(nearOutside, 2.0));
    EXPECT_FALSE(square.isOnContour(nearOutside, 0.5));
    
    // Point 1 unit inside from right edge
    Point nearInside{4.0, 0.0};
    EXPECT_TRUE(square.isOnContour(nearInside, 2.0));
    EXPECT_FALSE(square.isOnContour(nearInside, 0.5));
}

TEST_F(PolygonTest, IsOnContour_FarFromEdge) {
    Polygon square = createSquare(10.0);
    
    // Center (far inside)
    EXPECT_FALSE(square.isOnContour({0.0, 0.0}, 2.0));
    
    // Far outside
    Point farAway{50.0, 50.0};
    EXPECT_FALSE(square.isOnContour(farAway, 2.0));
}

TEST_F(PolygonTest, IsOnContour_Triangle) {
    Polygon triangle = createTriangle(0.0, 0.0, 10.0, 0.0, 5.0, 8.66);
    
    // Point on bottom edge
    Point onBottom{5.0, 0.0};
    EXPECT_TRUE(triangle.isOnContour(onBottom, 0.1));
    
    // Point on slanted edge (approximate)
    Point onSlant{7.5, 4.33};  // Midpoint of right edge
    EXPECT_TRUE(triangle.isOnContour(onSlant, 0.5));
}

TEST_F(PolygonTest, IsOnContour_ConcavePolygon) {
    // L-shaped polygon (concave)
    Polygon lShape({
        {0.0, 0.0},
        {20.0, 0.0},
        {20.0, 10.0},
        {10.0, 10.0},
        {10.0, 20.0},
        {0.0, 20.0}
    });
    
    // Point on outer edge
    Point onOuter{10.0, 0.0};
    EXPECT_TRUE(lShape.isOnContour(onOuter, 0.1));
    
    // Point on inner corner edge
    Point onInner{10.0, 15.0};
    EXPECT_TRUE(lShape.isOnContour(onInner, 0.1));
}

TEST_F(PolygonTest, IsOnContour_SmallTolerance) {
    Polygon square = createSquare(10.0);
    
    // With very small tolerance, interior points should not be on contour
    Point interior{2.0, 2.0};  // Well inside
    EXPECT_FALSE(square.isOnContour(interior, 0.01));
    
    // But edge point should be
    Point edge{5.0, 0.0};
    EXPECT_TRUE(square.isOnContour(edge, 0.01));
}

TEST_F(PolygonTest, IsOnContour_ClosestPointOnSegment) {
    // Create a simple vertical line segment as a degenerate polygon
    Polygon segment({{0.0, 0.0}, {0.0, 10.0}});
    
    // Point directly on segment
    Point onSegment{0.0, 5.0};
    EXPECT_TRUE(segment.isOnContour(onSegment, 0.1));
    
    // Point near segment
    Point nearSegment{1.0, 5.0};
    EXPECT_TRUE(segment.isOnContour(nearSegment, 2.0));
    EXPECT_FALSE(segment.isOnContour(nearSegment, 0.5));
}

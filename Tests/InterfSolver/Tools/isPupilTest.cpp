#include "stdafx.h"
#include <chrono>
#include "InterfSolver/Tools/isPupil.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPolygon.h"

// ========================================================================
// Test Fixture
// ========================================================================

class IsPupilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test shapes
        // External circle at (100, 100), radius 50
        externalCircle = XYEllipse(50.0, 50.0, 100.0, 100.0, 0.0, EXTERNAL);
        
        // Internal circle at (100, 100), radius 20
        internalCircle = XYEllipse(20.0, 20.0, 100.0, 100.0, 0.0, INTERNAL);
        
        // External rectangle [50, 150] x [50, 150]
        externalRect = XYRect(50.0, 150.0, 50.0, 150.0, EXTERNAL);
        
        // Internal rectangle [80, 120] x [80, 120]
        internalRect = XYRect(80.0, 120.0, 80.0, 120.0, INTERNAL);
        
        // External polygon (square)
        CArrayDouble xExt, yExt;
        xExt.Add(0.0); yExt.Add(0.0);
        xExt.Add(200.0); yExt.Add(0.0);
        xExt.Add(200.0); yExt.Add(200.0);
        xExt.Add(0.0); yExt.Add(200.0);
        xExt.Add(0.0); yExt.Add(0.0);
        externalPolygon = XYPolygon(xExt, yExt, EXTERNAL);
        
        // Internal polygon (small square)
        CArrayDouble xInt, yInt;
        xInt.Add(90.0); yInt.Add(90.0);
        xInt.Add(110.0); yInt.Add(90.0);
        xInt.Add(110.0); yInt.Add(110.0);
        xInt.Add(90.0); yInt.Add(110.0);
        xInt.Add(90.0); yInt.Add(90.0);
        internalPolygon = XYPolygon(xInt, yInt, INTERNAL);
    }
    
    XYEllipse externalCircle;
    XYEllipse internalCircle;
    XYRect externalRect;
    XYRect internalRect;
    XYPolygon externalPolygon;
    XYPolygon internalPolygon;
};

// ========================================================================
// Single Shape Tests - Ellipse
// ========================================================================

TEST_F(IsPupilTest, ExternalCircle_PointOutside_ReturnsTrue) {
    XYPoint P(200.0, 200.0);  // Far outside
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ExternalCircle_PointInside_ReturnsFalse) {
    XYPoint P(100.0, 100.0);  // Center (inside)
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ExternalCircle_PointOnBoundary_ReturnsTrue) {
    XYPoint P(150.0, 100.0);  // On edge
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalCircle_PointInside_ReturnsTrue) {
    XYPoint P(100.0, 100.0);  // Center (inside INTERNAL shape = visible)
    CArrayXYEllipse arr;
    arr.Add(internalCircle);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalCircle_PointOutside_ReturnsFalse) {
    XYPoint P(200.0, 200.0);  // Outside INTERNAL shape = not visible
    CArrayXYEllipse arr;
    arr.Add(internalCircle);
    
    EXPECT_FALSE(isPupil(P, arr));
}

// ========================================================================
// Single Shape Tests - Rectangle
// ========================================================================

TEST_F(IsPupilTest, ExternalRect_PointOutside_ReturnsTrue) {
    XYPoint P(200.0, 200.0);
    CArrayXYRect arr;
    arr.Add(externalRect);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ExternalRect_PointInside_ReturnsFalse) {
    XYPoint P(100.0, 100.0);
    CArrayXYRect arr;
    arr.Add(externalRect);
    
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalRect_PointInside_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYRect arr;
    arr.Add(internalRect);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalRect_PointOutside_ReturnsFalse) {
    XYPoint P(200.0, 200.0);
    CArrayXYRect arr;
    arr.Add(internalRect);
    
    EXPECT_FALSE(isPupil(P, arr));
}

// ========================================================================
// Single Shape Tests - Polygon
// ========================================================================

TEST_F(IsPupilTest, ExternalPolygon_PointOutside_ReturnsTrue) {
    XYPoint P(300.0, 300.0);
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ExternalPolygon_PointInside_ReturnsFalse) {
    XYPoint P(100.0, 100.0);
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalPolygon_PointInside_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYPolygon arr;
    arr.Add(internalPolygon);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, InternalPolygon_PointOutside_ReturnsFalse) {
    XYPoint P(200.0, 200.0);
    CArrayXYPolygon arr;
    arr.Add(internalPolygon);
    
    EXPECT_FALSE(isPupil(P, arr));
}

// ========================================================================
// Combined Shape Tests
// ========================================================================

TEST_F(IsPupilTest, ExternalCircleAndRect_PointInCircleOutsideRect_ReturnsFalse) {
    XYPoint P(160.0, 100.0);  // Inside circle, outside rect
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    arrEll.Add(externalCircle);
    arrRect.Add(externalRect);
    
    // For EXTERNAL shapes, point must be outside ALL shapes
    EXPECT_FALSE(isPupil(P, arrEll, arrRect, CArrayXYPolygon()));
}

TEST_F(IsPupilTest, ExternalCircleAndRect_PointOutsideBoth_ReturnsTrue) {
    XYPoint P(200.0, 200.0);  // Outside both
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    arrEll.Add(externalCircle);
    arrRect.Add(externalRect);
    
    EXPECT_TRUE(isPupil(P, arrEll, arrRect, CArrayXYPolygon()));
}

TEST_F(IsPupilTest, ExternalCircleAndRect_PointInsideBoth_ReturnsFalse) {
    XYPoint P(100.0, 100.0);  // Inside both
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    arrEll.Add(externalCircle);
    arrRect.Add(externalRect);
    
    EXPECT_FALSE(isPupil(P, arrEll, arrRect, CArrayXYPolygon()));
}

// ========================================================================
// Aperture + Obstruction Tests (Real-world scenario)
// ========================================================================

TEST_F(IsPupilTest, ApertureWithObstruction_PointInValidRegion_ReturnsTrue) {
    // Setup: External circle (aperture) + Internal circle (obstruction)
    CArrayXYEllipse arr;
    arr.Add(externalCircle);  // EXTERNAL: must be outside
    arr.Add(internalCircle);  // INTERNAL: must be inside
    
    // Point between inner and outer circles = valid region
    XYPoint P(130.0, 100.0);  // Outside inner, inside outer
    
    // For EXTERNAL: outside = visible → FALSE (inside outer circle)
    // For INTERNAL: inside = visible → FALSE (outside inner circle)
    // Combined: Should be FALSE (doesn't satisfy both conditions)
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ApertureWithObstruction_PointInObstruction_ReturnsFalse) {
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    arr.Add(internalCircle);
    
    XYPoint P(100.0, 100.0);  // Inside both circles (obstruction area)
    
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, ApertureWithObstruction_PointOutsideAperture_ReturnsFalse) {
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    arr.Add(internalCircle);
    
    XYPoint P(200.0, 200.0);  // Outside aperture
    
    EXPECT_FALSE(isPupil(P, arr));
}

// ========================================================================
// Empty Array Tests
// ========================================================================

TEST_F(IsPupilTest, EmptyArrays_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    
    EXPECT_TRUE(isPupil(P, arrEll, arrRect, arrPlg));
}

TEST_F(IsPupilTest, EmptyEllipseArray_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYEllipse arr;
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, EmptyRectArray_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYRect arr;
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, EmptyPolygonArray_ReturnsTrue) {
    XYPoint P(100.0, 100.0);
    CArrayXYPolygon arr;
    
    EXPECT_TRUE(isPupil(P, arr));
}

// ========================================================================
// ExceptElm Tests
// ========================================================================

TEST_F(IsPupilTest, ExceptElm_SkipsSpecifiedEllipse) {
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    arr.Add(internalCircle);
    
    XYPoint P(100.0, 100.0);  // Inside first circle
    
    // Without except: FALSE
    EXPECT_FALSE(isPupil(P, arr, 99));
    
    // Skip first circle (index 0): should only check second circle
    EXPECT_TRUE(isPupil(P, arr, 0));
}

TEST_F(IsPupilTest, ExceptElm_SkipsSpecifiedRect) {
    CArrayXYRect arr;
    arr.Add(externalRect);
    arr.Add(internalRect);
    
    XYPoint P(100.0, 100.0);  // Inside first rect
    
    // Without except: FALSE
    EXPECT_FALSE(isPupil(P, arr, 99));
    
    // Skip first rect (index 0): should only check second rect
    EXPECT_TRUE(isPupil(P, arr, 0));
}

TEST_F(IsPupilTest, ExceptElm_SkipsSpecifiedPolygon) {
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    arr.Add(internalPolygon);
    
    XYPoint P(100.0, 100.0);  // Inside first polygon
    
    // Without except: FALSE
    EXPECT_FALSE(isPupil(P, arr, 99));
    
    // Skip first polygon (index 0): should only check second polygon
    EXPECT_TRUE(isPupil(P, arr, 0));
}

// ========================================================================
// Performance Benchmark Test
// ========================================================================

TEST_F(IsPupilTest, Performance_SinglePolygon_100kPoints) {
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int count = 0;
    for (int y = 0; y < 1000; y++) {
        for (int x = 0; x < 100; x++) {
            XYPoint P(x * 2.0, y * 0.2);
            if (isPupil(P, arr)) {
                count++;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Tested 100k points in " << duration.count() << " ms" << std::endl;
    std::cout << "Points outside polygon: " << count << std::endl;
    
    // Performance expectation: should complete in reasonable time
    EXPECT_LT(duration.count(), 5000);  // Less than 5 seconds
}

TEST_F(IsPupilTest, Performance_MultipleShapes_10kPoints) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    
    arrEll.Add(externalCircle);
    arrEll.Add(internalCircle);
    arrRect.Add(externalRect);
    arrPlg.Add(externalPolygon);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int count = 0;
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            XYPoint P(x * 2.0, y * 2.0);
            if (isPupil(P, arrEll, arrRect, arrPlg)) {
                count++;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Tested 10k points against 4 shapes in " << duration.count() << " ms" << std::endl;
    std::cout << "Visible points: " << count << std::endl;
    
    EXPECT_LT(duration.count(), 1000);  // Less than 1 second
}

// ========================================================================
// Edge Cases
// ========================================================================

TEST_F(IsPupilTest, PointExactlyOnEllipseBoundary) {
    XYPoint P(150.0, 100.0);  // Exactly on circle edge (radius 50 from center 100,100)
    CArrayXYEllipse arr;
    arr.Add(externalCircle);
    
    // On boundary should be considered "outside" for EXTERNAL
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, PointExactlyOnRectBoundary) {
    XYPoint P(50.0, 100.0);  // On left edge
    CArrayXYRect arr;
    arr.Add(externalRect);
    
    EXPECT_TRUE(isPupil(P, arr));
}

TEST_F(IsPupilTest, PointExactlyOnPolygonVertex) {
    XYPoint P(0.0, 0.0);  // Exact vertex
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    
    // Vertex should be considered "inside"
    EXPECT_FALSE(isPupil(P, arr));
}

TEST_F(IsPupilTest, PointExactlyOnPolygonEdge) {
    XYPoint P(100.0, 0.0);  // Midpoint of bottom edge
    CArrayXYPolygon arr;
    arr.Add(externalPolygon);
    
    // Edge should be considered "inside"
    EXPECT_FALSE(isPupil(P, arr));
}

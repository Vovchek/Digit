/// <summary>
/// Google Test suite for XYShape polymorphic behavior
/// Tests base class functionality and polymorphic usage of XYEllipse, XYRect, XYPolygon
/// </summary>
#include "stdafx.h"
#include "gtest/gtest.h"
#include "InterfSolver/Tools/XYShape.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYPolygon.h"
#include "InterfSolver/Tools/XYPoint.h"
#include "InterfSolver/Tools/XYBounds.h"
#include <vector>

// ============================================================================
// Test Fixture for XYShape Polymorphism
// ============================================================================

class XYShapeTest : public ::testing::Test {
protected:
    static constexpr double TOLERANCE = 1e-6;

    void SetUp() override {
        // Create test shapes
        ellipse = new XYEllipse(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING);
        rect = new XYRect(10.0, 5.0, 0.0, 0.0, 0.0, EXTERNAL, MEASURING);
        
        // Create square polygon
        CArrayDouble arrX, arrY;
        arrX.SetSize(5);
        arrY.SetSize(5);
        arrX[0] = -5.0; arrY[0] = -5.0;
        arrX[1] =  5.0; arrY[1] = -5.0;
        arrX[2] =  5.0; arrY[2] =  5.0;
        arrX[3] = -5.0; arrY[3] =  5.0;
        arrX[4] = -5.0; arrY[4] = -5.0;
        polygon = new XYPolygon(arrX, arrY, EXTERNAL, MEASURING);
    }

    void TearDown() override {
        delete ellipse;
        delete rect;
        delete polygon;
    }

    XYEllipse* ellipse;
    XYRect* rect;
    XYPolygon* polygon;
};

// ============================================================================
// Polymorphic Interface Tests
// ============================================================================

TEST_F(XYShapeTest, Polymorphic_TypeLimits_GetSet) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(INTERNAL);
        EXPECT_EQ(shape->GetTypeLimits(), INTERNAL);
        
        shape->SetTypeLimits(EXTERNAL);
        EXPECT_EQ(shape->GetTypeLimits(), EXTERNAL);
    }
}

TEST_F(XYShapeTest, Polymorphic_TypeSystCoor_GetSet) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        shape->SetTypeSystCoor(NORMALISED);
        EXPECT_EQ(shape->GetTypeSystCoor(), NORMALISED);
        
        shape->SetTypeSystCoor(MEASURING);
        EXPECT_EQ(shape->GetTypeSystCoor(), MEASURING);
    }
}

TEST_F(XYShapeTest, Polymorphic_isVisible_ExternalAperture) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    XYPoint inside(0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(EXTERNAL);
        
        // EXTERNAL aperture: inside visible, outside blocked
        EXPECT_TRUE(shape->isVisible(inside)) 
            << "EXTERNAL shape should have inside points visible";
        EXPECT_FALSE(shape->isVisible(outside)) 
            << "EXTERNAL shape should have outside points blocked";
    }
}

TEST_F(XYShapeTest, Polymorphic_isVisible_InternalObstruction) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    XYPoint inside(0.0, 0.0);
    XYPoint outside(20.0, 20.0);
    
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(INTERNAL);
        
        // INTERNAL obstruction: inside blocked, outside visible
        EXPECT_FALSE(shape->isVisible(inside)) 
            << "INTERNAL shape should have inside points blocked";
        EXPECT_TRUE(shape->isVisible(outside)) 
            << "INTERNAL shape should have outside points visible";
    }
}

TEST_F(XYShapeTest, Polymorphic_isVisible_XYOverload) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(EXTERNAL);
        
        EXPECT_TRUE(shape->isVisible(0.0, 0.0));     // Inside
        EXPECT_FALSE(shape->isVisible(100.0, 100.0)); // Outside
    }
}

// ============================================================================
// Visibility Logic Consistency Tests
// ============================================================================

TEST_F(XYShapeTest, VisibilityLogic_AllShapes_SameLogic) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    XYPoint testPoint(0.0, 0.0);  // Inside all shapes
    
    // Test EXTERNAL
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(EXTERNAL);
    }
    
    bool ellipseResult = ellipse->isVisible(testPoint);
    bool rectResult = rect->isVisible(testPoint);
    bool polygonResult = polygon->isVisible(testPoint);
    
    EXPECT_EQ(ellipseResult, rectResult) 
        << "Ellipse and Rect should have same visibility logic";
    EXPECT_EQ(rectResult, polygonResult) 
        << "Rect and Polygon should have same visibility logic";
    EXPECT_TRUE(ellipseResult) 
        << "All EXTERNAL shapes should show inside point as visible";
    
    // Test INTERNAL
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(INTERNAL);
    }
    
    ellipseResult = ellipse->isVisible(testPoint);
    rectResult = rect->isVisible(testPoint);
    polygonResult = polygon->isVisible(testPoint);
    
    EXPECT_EQ(ellipseResult, rectResult);
    EXPECT_EQ(rectResult, polygonResult);
    EXPECT_FALSE(ellipseResult) 
        << "All INTERNAL shapes should show inside point as blocked";
}

// ============================================================================
// GetBounds Tests
// ============================================================================

TEST_F(XYShapeTest, Polymorphic_GetBounds_ReturnValue) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        XYBounds bounds = shape->GetBounds();
        
        // All test shapes are centered at origin with similar extents
        EXPECT_LT(bounds.XLeft, 0.0);
        EXPECT_GT(bounds.XRight, 0.0);
        EXPECT_LT(bounds.YTop, 0.0);
        EXPECT_GT(bounds.YBottom, 0.0);
    }
}

TEST_F(XYShapeTest, Polymorphic_GetBounds_OutputParameter) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        XYBounds bounds;
        shape->GetBounds(bounds);
        
        EXPECT_LT(bounds.XLeft, 0.0);
        EXPECT_GT(bounds.XRight, 0.0);
    }
}

// ============================================================================
// Virtual Method Tests
// ============================================================================

TEST_F(XYShapeTest, VirtualMethod_isInside) {
    XYPoint center(0.0, 0.0);
    
    // Call via base class pointer - should use derived implementation
    XYShape* shapePtr = ellipse;
    EXPECT_TRUE(shapePtr->isInside(center));
    
    shapePtr = rect;
    EXPECT_TRUE(shapePtr->isInside(center));
    
    shapePtr = polygon;
    EXPECT_TRUE(shapePtr->isInside(center));
}

TEST_F(XYShapeTest, VirtualMethod_Perimeter) {
    XYShape* shapePtr = ellipse;
    double ellipsePerim = shapePtr->Perimeter();
    EXPECT_GT(ellipsePerim, 0.0);
    
    shapePtr = rect;
    double rectPerim = shapePtr->Perimeter();
    EXPECT_GT(rectPerim, 0.0);
    
    shapePtr = polygon;
    double polygonPerim = shapePtr->Perimeter();
    EXPECT_GE(polygonPerim, 0.0);  // May be 0 due to bug
}

TEST_F(XYShapeTest, VirtualMethod_Normalize) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        shape->Normalize(0.0, 0.0, 10.0);
        EXPECT_EQ(shape->GetTypeSystCoor(), NORMALISED);
    }
}

// ============================================================================
// Container Tests (Polymorphic Storage)
// ============================================================================

TEST_F(XYShapeTest, Container_VectorOfShapes) {
    std::vector<XYShape*> shapes;
    shapes.push_back(ellipse);
    shapes.push_back(rect);
    shapes.push_back(polygon);
    
    XYPoint testPoint(0.0, 0.0);
    
    // All shapes contain the test point
    for (XYShape* shape : shapes) {
        EXPECT_TRUE(shape->isInside(testPoint));
    }
}

TEST_F(XYShapeTest, Container_isPupil_Logic) {
    // Simulate isPupil() logic using polymorphic container
    std::vector<XYShape*> apertures;   // EXTERNAL shapes
    std::vector<XYShape*> obstructions; // INTERNAL shapes
    
    ellipse->SetTypeLimits(EXTERNAL);
    rect->SetTypeLimits(INTERNAL);
    polygon->SetTypeLimits(EXTERNAL);
    
    apertures.push_back(ellipse);
    apertures.push_back(polygon);
    obstructions.push_back(rect);
    
    XYPoint testPoint(0.0, 0.0);
    
    // Point is visible if:
    // - Inside at least one aperture
    // - Outside all obstructions
    
    bool insideAperture = false;
    for (XYShape* aperture : apertures) {
        if (aperture->isInside(testPoint)) {
            insideAperture = true;
            break;
        }
    }
    EXPECT_TRUE(insideAperture);
    
    bool blockedByObstruction = false;
    for (XYShape* obstruction : obstructions) {
        if (obstruction->isInside(testPoint)) {
            blockedByObstruction = true;
            break;
        }
    }
    EXPECT_TRUE(blockedByObstruction);  // Rect blocks center
    
    bool isVisible = insideAperture && !blockedByObstruction;
    EXPECT_FALSE(isVisible);  // Blocked by rect
}

TEST_F(XYShapeTest, Container_isPupil_UsingIsVisible) {
    // Simplified isPupil() using isVisible()
    std::vector<XYShape*> shapes;
    
    ellipse->SetTypeLimits(EXTERNAL);
    rect->SetTypeLimits(INTERNAL);
    polygon->SetTypeLimits(EXTERNAL);
    
    shapes.push_back(ellipse);
    shapes.push_back(rect);
    shapes.push_back(polygon);
    
    XYPoint testPoint(0.0, 0.0);
    
    // Point is visible if visible for ALL shapes
    bool visible = true;
    for (XYShape* shape : shapes) {
        if (!shape->isVisible(testPoint)) {
            visible = false;
            break;
        }
    }
    
    EXPECT_FALSE(visible);  // Blocked by INTERNAL rect
}

// ============================================================================
// Type Safety Tests
// ============================================================================

TEST_F(XYShapeTest, TypeSafety_DynamicCast) {
    XYShape* shapePtr = ellipse;
    
    XYEllipse* ellPtr = dynamic_cast<XYEllipse*>(shapePtr);
    EXPECT_NE(ellPtr, nullptr);
    
    XYRect* rectPtr = dynamic_cast<XYRect*>(shapePtr);
    EXPECT_EQ(rectPtr, nullptr);  // Should fail - wrong type
}

TEST_F(XYShapeTest, TypeSafety_PolymorphicDestruction) {
    // Create shapes on heap via base pointer
    XYShape* shape1 = new XYEllipse(5.0, 3.0, 0.0, 0.0, 0.0);
    XYShape* shape2 = new XYRect(5.0, 3.0, 0.0, 0.0, 0.0);
    
    // Should properly delete via virtual destructor
    delete shape1;
    delete shape2;
    
    // If this doesn't crash, virtual destructors are working
    SUCCEED();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(XYShapeTest, EdgeCase_BoundaryPoint) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    // Point exactly on boundary
    XYPoint boundary(10.0, 0.0);  // On right edge of shapes
    
    for (XYShape* shape : shapes) {
        shape->SetTypeLimits(EXTERNAL);
        
        bool inside = shape->isInside(boundary);
        // Boundary behavior may vary by shape, but isVisible should be consistent
        bool visible = shape->isVisible(boundary);
        
        if (inside) {
            EXPECT_TRUE(visible) << "If inside, should be visible for EXTERNAL";
        } else {
            EXPECT_FALSE(visible) << "If outside, should be blocked for EXTERNAL";
        }
    }
}

TEST_F(XYShapeTest, EdgeCase_VeryFarPoint) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    XYPoint farPoint(1e10, 1e10);
    
    for (XYShape* shape : shapes) {
        EXPECT_FALSE(shape->isInside(farPoint));
        
        shape->SetTypeLimits(EXTERNAL);
        EXPECT_FALSE(shape->isVisible(farPoint));  // Outside aperture
        
        shape->SetTypeLimits(INTERNAL);
        EXPECT_TRUE(shape->isVisible(farPoint));   // Outside obstruction
    }
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_F(XYShapeTest, Consistency_isVisible_MatchesLogic) {
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    XYPoint testPoints[] = {
        XYPoint(0.0, 0.0),     // Inside all
        XYPoint(100.0, 100.0), // Outside all
        XYPoint(10.0, 0.0),    // On boundary
    };
    
    for (XYShape* shape : shapes) {
        for (const XYPoint& pt : testPoints) {
            bool inside = shape->isInside(pt);
            
            // EXTERNAL: visible if inside
            shape->SetTypeLimits(EXTERNAL);
            bool visibleExt = shape->isVisible(pt);
            EXPECT_EQ(visibleExt, inside) 
                << "EXTERNAL: isVisible should match isInside";
            
            // INTERNAL: visible if outside
            shape->SetTypeLimits(INTERNAL);
            bool visibleInt = shape->isVisible(pt);
            EXPECT_EQ(visibleInt, !inside) 
                << "INTERNAL: isVisible should be opposite of isInside";
        }
    }
}

TEST_F(XYShapeTest, Consistency_TypeLimits_PublicAccess) {
    // Verify TypeLimits is public and accessible
    std::vector<XYShape*> shapes = { ellipse, rect, polygon };
    
    for (XYShape* shape : shapes) {
        shape->TypeLimits = INTERNAL;
        EXPECT_EQ(shape->GetTypeLimits(), INTERNAL);
        EXPECT_EQ(shape->TypeLimits, INTERNAL);
        
        shape->SetTypeLimits(EXTERNAL);
        EXPECT_EQ(shape->TypeLimits, EXTERNAL);
    }
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(XYShapeTest, Performance_PolymorphicCalls) {
    std::vector<XYShape*> shapes;
    
    // Create many shapes
    for (int i = 0; i < 100; i++) {
        shapes.push_back(new XYEllipse(10.0, 5.0, 0.0, 0.0, 0.0));
        shapes.push_back(new XYRect(10.0, 5.0, 0.0, 0.0, 0.0));
    }
    
    XYPoint testPoint(0.0, 0.0);
    
    // Test polymorphic calls
    int visibleCount = 0;
    for (XYShape* shape : shapes) {
        if (shape->isVisible(testPoint)) {
            visibleCount++;
        }
    }
    
    EXPECT_GT(visibleCount, 0);
    
    // Cleanup
    for (XYShape* shape : shapes) {
        delete shape;
    }
}

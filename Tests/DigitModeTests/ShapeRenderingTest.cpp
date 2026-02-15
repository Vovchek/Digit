/**
 * @file ShapeRenderingTest.cpp
 * @brief Integration tests for shape renderers and dispatcher (Phase 3)
 * 
 * Tests concrete renderers and dispatcher routing.
 * Note: These tests verify that rendering methods can be called without
 * crashing, but cannot verify actual pixel output without a real device context.
 */

#include "stdafx.h"
#include <gtest/gtest.h>
#include "DigitMode/Rendering/ShapeDrawDispatcher.h"
#include "DigitMode/Rendering/ShapeDrawStyle.h"
#include "DigitMode/Rendering/RectangleRenderer.h"
#include "DigitMode/Rendering/EllipseRenderer.h"
#include "DigitMode/Rendering/PolygonRenderer.h"
#include "ApertureCore/include/aperturecore/geometry/Rectangle.h"
#include "ApertureCore/include/aperturecore/geometry/Ellipse.h"
#include "ApertureCore/include/aperturecore/geometry/Polygon.h"
#include "ImageTempl/ViewTransform.h"

using namespace DigitMode;
using namespace aperture;

// ============================================================================
// Test Fixture with Memory DC
// ============================================================================

class ShapeRenderingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create memory DC for testing
        CWnd* desktop = CWnd::GetDesktopWindow();
        if (desktop) {
            CDC* desktopDC = desktop->GetDC();
            if (desktopDC) {
                m_memDC.CreateCompatibleDC(desktopDC);
                desktop->ReleaseDC(desktopDC);
            }
        }
        
        // Create bitmap for memory DC
        m_bitmap.CreateCompatibleBitmap(&m_memDC, 800, 600);
        m_memDC.SelectObject(&m_bitmap);
        
        // Set up transform (identity with some offset)
        m_transform.SetScale(1.0);
        m_transform.SetOffset(CPoint2d{100.0, 100.0});
    }
    
    void TearDown() override {
        m_bitmap.DeleteObject();
        m_memDC.DeleteDC();
    }
    
    CDC m_memDC;
    CBitmap m_bitmap;
    ViewTransform m_transform;
};

// ============================================================================
// ShapeDrawDispatcher Tests - Type Routing
// ============================================================================

TEST_F(ShapeRenderingTest, Dispatcher_Rectangle_RoutesToRectangleRenderer) {
    ShapeDrawDispatcher dispatcher;
    
    aperture::Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    // Should not crash
    EXPECT_NO_THROW({
        dispatcher.Draw(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Dispatcher_Ellipse_RoutesToEllipseRenderer) {
    ShapeDrawDispatcher dispatcher;
    
    aperture::Ellipse ellipse(50.0, 30.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    style.state = ShapeDrawStyle::State::Idle;
    
    // Should not crash
    EXPECT_NO_THROW({
        dispatcher.Draw(ellipse, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Dispatcher_Polygon_RoutesToPolygonRenderer) {
    ShapeDrawDispatcher dispatcher;
    
    std::vector<Point> vertices = {
        {0, 0}, {100, 0}, {100, 100}, {0, 100}
    };
    aperture::Polygon polygon(vertices);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    // Should not crash
    EXPECT_NO_THROW({
        dispatcher.Draw(polygon, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Dispatcher_TypeNameMatching_IsCaseSensitive) {
    ShapeDrawDispatcher dispatcher;
    
    // Create shapes and verify they have expected type names
    aperture::Rectangle rect(10, 10, 0, 0);
    EXPECT_STREQ(rect.typeName(), "Rectangle");
    
    aperture::Ellipse ellipse(10, 10, 0, 0);
    EXPECT_STREQ(ellipse.typeName(), "Ellipse");
    
    aperture::Polygon poly({{0,0}, {10,0}, {5,10}});
    EXPECT_STREQ(poly.typeName(), "Polygon");
}

// ============================================================================
// Dispatcher Handle Drawing Tests
// ============================================================================

TEST_F(ShapeRenderingTest, Dispatcher_DrawHandles_WhenShowHandlesTrue) {
    ShapeDrawDispatcher dispatcher;
    
    aperture::Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Selected;
    style.showHandles = true;
    style.activeHandleIndex = 0;
    
    // Should not crash
    EXPECT_NO_THROW({
        dispatcher.DrawHandles(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Dispatcher_DrawHandles_WhenShowHandlesFalse_SkipsRendering) {
    ShapeDrawDispatcher dispatcher;
    
    aperture::Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.showHandles = false;  // Should early exit
    
    // Should not crash and should return quickly
    EXPECT_NO_THROW({
        dispatcher.DrawHandles(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Dispatcher_DrawHandles_AllShapeTypes) {
    ShapeDrawDispatcher dispatcher;
    
    ShapeDrawStyle style;
    style.showHandles = true;
    style.activeHandleIndex = 1;
    
    // Rectangle handles
    aperture::Rectangle rect(100.0, 50.0, 0.0, 0.0);
    EXPECT_NO_THROW({
        dispatcher.DrawHandles(rect, m_memDC, style, m_transform);
    });
    
    // Ellipse handles
    aperture::Ellipse ellipse(50.0, 30.0, 0.0, 0.0);
    EXPECT_NO_THROW({
        dispatcher.DrawHandles(ellipse, m_memDC, style, m_transform);
    });
    
    // Polygon handles
    std::vector<Point> vertices = {{0,0}, {100,0}, {50,100}};
    aperture::Polygon polygon(vertices);
    EXPECT_NO_THROW({
        dispatcher.DrawHandles(polygon, m_memDC, style, m_transform);
    });
}

// ============================================================================
// RectangleRenderer Direct Tests
// ============================================================================

TEST_F(ShapeRenderingTest, RectangleRenderer_AxisAligned_RendersWithoutCrash) {
    RectangleRenderer renderer;
    
    aperture::Rectangle rect(100.0, 50.0, 200.0, 150.0);  // Centered at (200,150)
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    EXPECT_NO_THROW({
        renderer.Draw(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, RectangleRenderer_Rotated_RendersWithoutCrash) {
    RectangleRenderer renderer;
    
    // Create rotated rectangle using 3-point constructor
    Point p1(0, 0);
    Point p2(100, 0);
    Point p3(0, 50);
    aperture::Rectangle rect(p1, p2, p3);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    style.state = ShapeDrawStyle::State::Selected;
    
    EXPECT_NO_THROW({
        renderer.Draw(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, RectangleRenderer_WithFill_InternalType) {
    RectangleRenderer renderer;
    
    aperture::Rectangle rect(80.0, 60.0, 100.0, 100.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Idle;
    
    // INTERNAL shapes have fill
    ASSERT_TRUE(style.HasFill());
    
    EXPECT_NO_THROW({
        renderer.Draw(rect, m_memDC, style, m_transform);
    });
}

// ============================================================================
// EllipseRenderer Direct Tests
// ============================================================================

TEST_F(ShapeRenderingTest, EllipseRenderer_Circle_RendersWithoutCrash) {
    EllipseRenderer renderer;
    
    aperture::Ellipse circle(50.0, 50.0, 100.0, 100.0);  // Perfect circle
    ASSERT_TRUE(circle.isCircle());
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    EXPECT_NO_THROW({
        renderer.Draw(circle, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, EllipseRenderer_AxisAlignedEllipse_RendersWithoutCrash) {
    EllipseRenderer renderer;
    
    aperture::Ellipse ellipse(60.0, 40.0, 150.0, 150.0, 0.0);  // No rotation
    ASSERT_FALSE(ellipse.isCircle());
    
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    
    EXPECT_NO_THROW({
        renderer.Draw(ellipse, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, EllipseRenderer_RotatedEllipse_RendersWithoutCrash) {
    EllipseRenderer renderer;
    
    aperture::Ellipse ellipse(70.0, 35.0, 200.0, 150.0, 45.0);  // 45° rotation
    ASSERT_GT(std::abs(ellipse.rotationRadians()), 0.001);  // Significant rotation
    
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    style.state = ShapeDrawStyle::State::Dragging;
    
    EXPECT_NO_THROW({
        renderer.Draw(ellipse, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, EllipseRenderer_HandleDrawing_CircularHandles) {
    EllipseRenderer renderer;
    
    aperture::Ellipse ellipse(50.0, 30.0, 100.0, 100.0);
    
    ShapeDrawStyle style;
    style.showHandles = true;
    style.activeHandleIndex = 2;
    
    EXPECT_NO_THROW({
        renderer.DrawHandles(ellipse, m_memDC, style, m_transform);
    });
}

// ============================================================================
// PolygonRenderer Direct Tests
// ============================================================================

TEST_F(ShapeRenderingTest, PolygonRenderer_Triangle_RendersWithoutCrash) {
    PolygonRenderer renderer;
    
    std::vector<Point> triangle = {
        {0, 0},
        {100, 0},
        {50, 86.6}
    };
    aperture::Polygon poly(triangle);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    EXPECT_NO_THROW({
        renderer.Draw(poly, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, PolygonRenderer_ComplexPolygon_RendersWithoutCrash) {
    PolygonRenderer renderer;
    
    // Octagon
    std::vector<Point> vertices;
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4.0;
        vertices.push_back(Point(50.0 * std::cos(angle), 50.0 * std::sin(angle)));
    }
    aperture::Polygon poly(vertices);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    style.state = ShapeDrawStyle::State::Hovered;
    
    EXPECT_NO_THROW({
        renderer.Draw(poly, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, PolygonRenderer_DegeneratePolygon_HandlesGracefully) {
    PolygonRenderer renderer;
    
    // Empty polygon
    std::vector<Point> empty;
    aperture::Polygon poly(empty);
    
    ShapeDrawStyle style;
    
    // Should not crash on empty polygon
    EXPECT_NO_THROW({
        renderer.Draw(poly, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, PolygonRenderer_VertexHandles_OnePerVertex) {
    PolygonRenderer renderer;
    
    std::vector<Point> square = {
        {0, 0}, {100, 0}, {100, 100}, {0, 100}
    };
    aperture::Polygon poly(square);
    
    ShapeDrawStyle style;
    style.showHandles = true;
    style.activeHandleIndex = 1;
    
    // Should draw 4 handles (one per vertex)
    EXPECT_NO_THROW({
        renderer.DrawHandles(poly, m_memDC, style, m_transform);
    });
}

// ============================================================================
// State-Based Rendering Tests
// ============================================================================

TEST_F(ShapeRenderingTest, AllStates_RenderWithoutCrash) {
    ShapeDrawDispatcher dispatcher;
    aperture::Rectangle rect(100.0, 50.0, 0.0, 0.0);
    
    std::vector<ShapeDrawStyle::State> states = {
        ShapeDrawStyle::State::Idle,
        ShapeDrawStyle::State::Hovered,
        ShapeDrawStyle::State::Selected,
        ShapeDrawStyle::State::Dragging,
        ShapeDrawStyle::State::Draft
    };
    
    for (auto state : states) {
        ShapeDrawStyle style;
        style.state = state;
        style.type = TypeLimits::EXTERNAL;
        
        EXPECT_NO_THROW({
            dispatcher.Draw(rect, m_memDC, style, m_transform);
        }) << "Failed for state: " << static_cast<int>(state);
    }
}

TEST_F(ShapeRenderingTest, AllTypes_RenderWithoutCrash) {
    ShapeDrawDispatcher dispatcher;
    aperture::Ellipse ellipse(50.0, 50.0, 0.0, 0.0);
    
    std::vector<TypeLimits> types = {
        TypeLimits::EXTERNAL,
        TypeLimits::INTERNAL,
        TypeLimits::APERTURE
    };
    
    for (auto type : types) {
        ShapeDrawStyle style;
        style.state = ShapeDrawStyle::State::Idle;
        style.type = type;
        
        EXPECT_NO_THROW({
            dispatcher.Draw(ellipse, m_memDC, style, m_transform);
        }) << "Failed for type: " << static_cast<int>(type);
    }
}

// ============================================================================
// ViewTransform Integration Tests
// ============================================================================

TEST_F(ShapeRenderingTest, Transform_ZoomedIn_RendersLarger) {
    ShapeDrawDispatcher dispatcher;
    aperture::Rectangle rect(50.0, 50.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::EXTERNAL;
    
    // Set high zoom
    m_transform.SetScale(5.0);
    
    EXPECT_NO_THROW({
        dispatcher.Draw(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Transform_ZoomedOut_RendersSmaller) {
    ShapeDrawDispatcher dispatcher;
    aperture::Rectangle rect(200.0, 200.0, 100.0, 100.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::APERTURE;
    
    // Set low zoom
    m_transform.SetScale(0.2);
    
    EXPECT_NO_THROW({
        dispatcher.Draw(rect, m_memDC, style, m_transform);
    });
}

TEST_F(ShapeRenderingTest, Transform_Panned_RendersAtOffset) {
    ShapeDrawDispatcher dispatcher;
    aperture::Ellipse ellipse(40.0, 40.0, 0.0, 0.0);
    
    ShapeDrawStyle style;
    style.type = TypeLimits::INTERNAL;
    
    // Pan view
    m_transform.SetOffset(CPoint2d{-200.0, -150.0});
    
    EXPECT_NO_THROW({
        dispatcher.Draw(ellipse, m_memDC, style, m_transform);
    });
}

// ============================================================================
// Regression Tests
// ============================================================================

TEST_F(ShapeRenderingTest, MultipleShapes_SequentialRendering_NoStateLeakage) {
    ShapeDrawDispatcher dispatcher;
    
    // Draw multiple shapes in sequence
    aperture::Rectangle rect(100, 50, 0, 0);
    aperture::Ellipse ellipse(60, 40, 200, 0);
    std::vector<Point> triangle = {{300,0}, {350,0}, {325,50}};
    aperture::Polygon poly(triangle);
    
    ShapeDrawStyle externalStyle;
    externalStyle.type = TypeLimits::EXTERNAL;
    
    ShapeDrawStyle internalStyle;
    internalStyle.type = TypeLimits::INTERNAL;
    
    // Render all without crashing
    EXPECT_NO_THROW({
        dispatcher.Draw(rect, m_memDC, externalStyle, m_transform);
        dispatcher.Draw(ellipse, m_memDC, internalStyle, m_transform);
        dispatcher.Draw(poly, m_memDC, externalStyle, m_transform);
    });
}

TEST_F(ShapeRenderingTest, SameShape_MultipleStyles_RendersCorrectly) {
    ShapeDrawDispatcher dispatcher;
    aperture::Rectangle rect(80, 60, 100, 100);
    
    // Draw same shape with different styles
    ShapeDrawStyle idle;
    idle.state = ShapeDrawStyle::State::Idle;
    idle.type = TypeLimits::EXTERNAL;
    
    ShapeDrawStyle selected;
    selected.state = ShapeDrawStyle::State::Selected;
    selected.type = TypeLimits::INTERNAL;
    selected.showHandles = true;
    
    EXPECT_NO_THROW({
        dispatcher.Draw(rect, m_memDC, idle, m_transform);
        dispatcher.Draw(rect, m_memDC, selected, m_transform);
        dispatcher.DrawHandles(rect, m_memDC, selected, m_transform);
    });
}

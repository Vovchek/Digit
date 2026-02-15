/**
 * @file IShapeRenderer.h
 * @brief Abstract renderer interface for shape drawing (Phase 3)
 * 
 * ## Architecture Overview
 * 
 * The rendering system has three distinct layers:
 * 1. **Geometry** (ApertureCore) - Pure shapes (Rectangle, Ellipse, Polygon)
 * 2. **Appearance** (ShapeDrawStyle) - Visual state (colors, line width, fill)
 * 3. **Rendering** (IShapeRenderer) - GDI drawing logic
 * 
 * This separation ensures:
 * - ApertureCore remains UI-free and testable
 * - Visual appearance can change without geometry changes
 * - Renderers can be swapped (GDI → Direct2D → OpenGL)
 * 
 * ## Usage Pattern
 * 
 * ```cpp
 * // Dispatcher selects appropriate renderer
 * const IShapeRenderer* renderer = dispatcher.GetRenderer(shape);
 * 
 * // Compute visual style based on UI state
 * ShapeDrawStyle style;
 * style.state = ShapeDrawStyle::State::Selected;
 * style.type = aperture::TypeLimits::EXTERNAL;
 * style.showHandles = true;
 * 
 * // Render shape + handles
 * renderer->Draw(shape, dc, style, worldToScreen);
 * renderer->DrawHandles(shape, dc, style, worldToScreen);
 * ```
 * 
 * ## Coordinate Transformation
 * 
 * Shapes store geometry in **world coordinates** (engineering units).
 * GDI requires **screen coordinates** (pixels).
 * 
 * ViewTransform bridges these:
 * - `WorldToScreen(Point)` → CPoint for GDI calls
 * - `ScreenToWorld(CPoint)` → Point for hit-testing
 * 
 * ## Renderer Responsibilities
 * 
 * **Draw():**
 * 1. Downcast Shape& to concrete type (safe via dispatcher)
 * 2. Extract geometry (corners, radii, vertices)
 * 3. Transform to screen coordinates
 * 4. Create GDI resources (CPen, CBrush) from ShapeDrawStyle
 * 5. Draw primitives (Polygon, Ellipse, Polyline)
 * 6. Restore device context
 * 
 * **DrawHandles():**
 * 1. Check `style.showHandles` (early exit if false)
 * 2. Enumerate handles via `shape.enumerateHandles()`
 * 3. Transform handle positions to screen coordinates
 * 4. Draw handle rectangles/circles
 * 5. Highlight `style.activeHandleIndex` with different color
 * 
 * ## Performance Notes
 * 
 * - Renderers are **stateless** (const methods, no member variables)
 * - Single renderer instance per shape type (reused across frames)
 * - GDI resource creation/destruction happens per Draw() call
 * - For high-performance rendering, consider caching pens/brushes
 */
#pragma once

#include <afxwin.h>  // For CDC, CPen, CBrush

// Forward declarations to avoid circular dependencies
namespace aperture { 
    class Shape; 
}

namespace DigitMode { 
    struct ShapeDrawStyle; 
}

class ViewTransform;  // Coordinate transformation utility (ImageTempl layer)

namespace DigitMode {

/**
 * @brief Abstract renderer for a specific shape type
 * 
 * Concrete implementations exist for Rectangle, Ellipse, Polygon.
 * Lives in UI layer (knows about MFC/GDI), pulls geometry from Shape.
 * 
 * ## Design Principles
 * - **Stateless**: No member variables, all rendering driven by parameters
 * - **Const Methods**: Renderers don't modify shapes or styles
 * - **Single Responsibility**: Only responsible for drawing, not hit-testing or editing
 * - **Type-Safe Downcasting**: Dispatcher ensures shape type matches renderer
 */
class IShapeRenderer {
public:
    virtual ~IShapeRenderer() = default;
    
    /**
     * @brief Draw shape outline and fill (if applicable)
     * 
     * @param shape Shape to render (geometry in world coordinates)
     * @param dc Device context (screen, printer, or memory DC)
     * @param style Visual appearance (colors, line width, fill)
     * @param worldToScreen Transform from world to screen coordinates
     * 
     * ## Implementation Steps
     * 1. Downcast `shape` to concrete type (Rectangle/Ellipse/Polygon)
     * 2. Extract geometry (corners, center, radii, vertices)
     * 3. Transform all points from world → screen coordinates
     * 4. Create CPen from `style.GetOutlineColor/Width/Style()`
     * 5. Create CBrush if `style.HasFill()` is true
     * 6. Draw GDI primitive (Polygon, Ellipse, Polyline)
     * 7. Restore device context (SelectObject old pen/brush)
     * 
     * ## Example (Rectangle)
     * ```cpp
     * const auto& rect = static_cast<const aperture::Rectangle&>(shape);
     * auto corners = rect.corners();  // 4 world-space points
     * 
     * CPoint screenPts[4];
     * for (int i = 0; i < 4; ++i) {
     *     screenPts[i] = worldToScreen.WorldToScreen(corners[i]);
     * }
     * 
     * CPen pen(style.GetOutlineStyle(), style.GetOutlineWidth(), style.GetOutlineColor());
     * CPen* oldPen = dc.SelectObject(&pen);
     * dc.Polygon(screenPts, 4);
     * dc.SelectObject(oldPen);
     * ```
     */
    virtual void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const = 0;
    
    /**
     * @brief Draw interactive handles for shape manipulation
     * 
     * @param shape Shape to render handles for
     * @param dc Device context (screen or memory DC)
     * @param style Visual appearance (handle color, active handle highlight)
     * @param worldToScreen Transform from world to screen coordinates
     * 
     * ## Implementation Steps
     * 1. Early exit if `!style.showHandles`
     * 2. Enumerate handles via `shape.enumerateHandles(callback)`
     * 3. Transform each handle position from world → screen
     * 4. Draw handle rectangle/circle (size from `style.GetHandleSize()`)
     * 5. Use `style.GetHandleColor(index)` to highlight active handle
     * 
     * ## Handle Types (from HandleDesc)
     * - **Move**: Single handle at shape center (translates shape)
     * - **Rotate**: Handle at rotation anchor (changes angle)
     * - **Corner**: 4 handles at rectangle corners (resizes)
     * - **Edge**: 4 handles at edge midpoints (resizes one dimension)
     * - **Axis**: 2 handles at ellipse major/minor axis endpoints
     * - **Vertex**: N handles at polygon vertices
     * 
     * ## Visual Style
     * - Normal handles: White square (5x5 or 6x6 pixels)
     * - Active handle: Yellow square (highlighted during drag)
     * - Handle outline: Black 1-pixel border for contrast
     * 
     * ## Example (Rectangle Handles)
     * ```cpp
     * if (!style.showHandles) return;
     * 
     * const auto& rect = static_cast<const aperture::Rectangle&>(shape);
     * int handleIdx = 0;
     * 
     * rect.enumerateHandles([&](const aperture::HandleDesc& desc) {
     *     CPoint screenPos = worldToScreen.WorldToScreen(desc.position);
     *     COLORREF color = style.GetHandleColor(handleIdx);
     *     int size = style.GetHandleSize();
     *     
     *     CBrush brush(color);
     *     CBrush* oldBrush = dc.SelectObject(&brush);
     *     dc.Rectangle(screenPos.x - size, screenPos.y - size,
     *                  screenPos.x + size, screenPos.y + size);
     *     dc.SelectObject(oldBrush);
     *     
     *     handleIdx++;
     * });
     * ```
     */
    virtual void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const = 0;
};

} // namespace DigitMode

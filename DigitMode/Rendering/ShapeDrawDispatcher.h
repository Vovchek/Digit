/**
 * @file ShapeDrawDispatcher.h
 * @brief Central shape rendering dispatcher (Phase 3)
 * 
 * ## Overview
 * 
 * ShapeDrawDispatcher is the single entry point for rendering shapes.
 * It owns all concrete renderers and dispatches to the appropriate one
 * based on the shape's type (typeName()).
 * 
 * ## Usage Pattern
 * 
 * ```cpp
 * // In CApertureCtrls::OnDraw() or similar UI code
 * ShapeDrawDispatcher dispatcher;
 * 
 * // Iterate through shapes in collection
 * for (const auto& shape : shapes.getExternal()) {
 *     // Compute style based on selection/hover state
 *     ShapeDrawStyle style;
 *     style.state = isSelected ? State::Selected : State::Idle;
 *     style.type = TypeLimits::EXTERNAL;
 *     style.showHandles = isSelected;
 *     
 *     // Dispatch to appropriate renderer
 *     dispatcher.Draw(*shape, dc, style, worldToScreen);
 *     dispatcher.DrawHandles(*shape, dc, style, worldToScreen);
 * }
 * ```
 * 
 * ## Performance
 * 
 * - Renderer selection: O(1) string comparison
 * - Renderers are created once in constructor (not per-frame)
 * - Renderers are stateless (reusable across calls)
 * - No heap allocations during rendering (renderers owned by dispatcher)
 */
#pragma once

#include "IShapeRenderer.h"
#include <memory>
#include <string>

namespace DigitMode {

// Forward declarations of concrete renderers
class RectangleRenderer;
class EllipseRenderer;
class PolygonRenderer;
struct ShapeDrawStyle;

/**
 * @brief Central shape rendering dispatcher
 * 
 * Selects appropriate renderer based on shape type.
 * Owns all concrete renderers.
 */
class ShapeDrawDispatcher {
public:
    /**
     * @brief Constructor - creates all concrete renderers
     */
    ShapeDrawDispatcher();
    
    /**
     * @brief Destructor - cleans up renderers
     */
    ~ShapeDrawDispatcher();
    
    // Disable copying (dispatcher owns renderers)
    ShapeDrawDispatcher(const ShapeDrawDispatcher&) = delete;
    ShapeDrawDispatcher& operator=(const ShapeDrawDispatcher&) = delete;
    
    /**
     * @brief Draw shape with specified style
     * 
     * @param shape Shape to render (type determined via typeName())
     * @param dc Device context (screen, printer, or memory DC)
     * @param style Visual appearance parameters
     * @param worldToScreen Transform from world to screen coordinates
     * 
     * ## Dispatching
     * 1. Call `shape.typeName()` to determine type
     * 2. Match type string ("Rectangle", "Ellipse", "Polygon")
     * 3. Delegate to `renderer->Draw()`
     * 4. Unknown types are ignored (no crash)
     */
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const;
    
    /**
     * @brief Draw handles for shape (if style.showHandles == true)
     * 
     * @param shape Shape to render handles for
     * @param dc Device context
     * @param style Visual appearance (controls handle visibility/color)
     * @param worldToScreen Transform from world to screen coordinates
     * 
     * ## Dispatching
     * Same logic as Draw(), but calls `renderer->DrawHandles()`.
     * Early exit if `!style.showHandles`.
     */
    void DrawHandles(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style,
        const ViewTransform& worldToScreen
    ) const;
    
private:
    /**
     * @brief Get renderer for specific shape type name
     * @param typeName Shape type string ("Rectangle", "Ellipse", "Polygon")
     * @return Pointer to renderer, or nullptr if unknown type
     */
    const IShapeRenderer* GetRenderer(const std::string& typeName) const;
    
    // Owned renderers (one per shape type)
    std::unique_ptr<RectangleRenderer> m_rectangleRenderer;
    std::unique_ptr<EllipseRenderer> m_ellipseRenderer;
    std::unique_ptr<PolygonRenderer> m_polygonRenderer;
};

} // namespace DigitMode

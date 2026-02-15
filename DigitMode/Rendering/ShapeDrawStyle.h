/**
 * @file ShapeDrawStyle.h
 * @brief Visual appearance parameters for shape rendering (Phase 3)
 * 
 * ## Overview
 * 
 * ShapeDrawStyle is a pure data structure that describes HOW to render a shape,
 * completely decoupled from WHAT to render (the geometry).
 * 
 * ## Architecture Principle (CRITICAL)
 * 
 * **Shapes describe geometry.**
 * **ShapeDrawStyle describes appearance.**
 * **Renderers combine both.**
 * 
 * This separation keeps ApertureCore UI-free and testable.
 * 
 * ## Usage
 * 
 * ```cpp
 * // UI layer computes style based on selection/hover state
 * ShapeDrawStyle style;
 * style.state = ShapeDrawStyle::State::Selected;
 * style.type = aperture::TypeLimits::EXTERNAL;
 * style.showHandles = true;
 * 
 * // Renderer applies style to shape geometry
 * renderer.Draw(shape, dc, style, worldToScreen);
 * ```
 * 
 * ## Color & Style Rules (UX Spec §3)
 * 
 * | TypeLimits | Outline     | Fill             |
 * |------------|-------------|------------------|
 * | EXTERNAL   | Solid green | None             |
 * | APERTURE   | Solid cyan  | None             |
 * | INTERNAL   | Dashed red  | Semi-transparent |
 * 
 * | State    | Outline | Handles |
 * |----------|---------|---------|
 * | Idle     | Thin    | Hidden  |
 * | Hovered  | Thicker | Hidden  |
 * | Selected | Thick   | Visible |
 * | Dragging | Thick   | Active  |
 * | Draft    | Dashed  | Dots    |
 */
#pragma once

#include <afxwin.h>  // For COLORREF, PS_* constants
#include "ApertureCore/include/aperturecore/visibility/TypeLimits.h"

namespace DigitMode {

/**
 * @brief Visual appearance parameters for shape rendering
 * 
 * Pure data structure - no logic, no dependencies on Shape.
 * Computed by UI layer based on selection/interaction state.
 */
struct ShapeDrawStyle {
    /**
     * @brief Interaction state determining visual appearance
     */
    enum class State {
        Idle,      ///< Not selected, not hovered
        Hovered,   ///< Mouse cursor over shape
        Selected,  ///< Currently selected
        Dragging,  ///< Being dragged (move/resize/rotate)
        Draft      ///< Draft shape (not yet committed)
    };
    
    // ========================================================================
    // State Properties
    // ========================================================================
    
    State state = State::Idle;
    aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
    bool showHandles = false;      ///< True if handles should be drawn
    int activeHandleIndex = -1;    ///< Index of handle being dragged (-1 = none)
    
    // ========================================================================
    // Computed Style Methods
    // ========================================================================
    
    /**
     * @brief Get outline color based on TypeLimits
     * @return Color for shape outline
     * 
     * - EXTERNAL → Green (RGB 0, 255, 0)
     * - APERTURE → Cyan (RGB 0, 255, 255)
     * - INTERNAL → Red (RGB 255, 0, 0)
     */
    COLORREF GetOutlineColor() const;
    
    /**
     * @brief Get outline width based on State
     * @return Pen width in pixels
     * 
     * - Idle → 1 pixel
     * - Hovered → 2 pixels
     * - Selected → 3 pixels
     * - Dragging → 3 pixels
     * - Draft → 2 pixels (dashed)
     */
    int GetOutlineWidth() const;
    
    /**
     * @brief Get outline style (solid/dashed)
     * @return GDI pen style (PS_SOLID, PS_DASH, etc.)
     * 
     * - INTERNAL → PS_DASH (dashed line)
     * - Draft → PS_DASH (dashed line)
     * - Others → PS_SOLID (solid line)
     */
    int GetOutlineStyle() const;
    
    /**
     * @brief Get fill color for INTERNAL shapes
     * @return Semi-transparent red for INTERNAL, transparent otherwise
     */
    COLORREF GetFillColor() const;
    
    /**
     * @brief Check if shape should be filled
     * @return true only for INTERNAL shapes
     */
    bool HasFill() const;
    
    /**
     * @brief Get handle color
     * @return White for normal handles, yellow for active handle
     */
    COLORREF GetHandleColor(int handleIndex) const;
    
    /**
     * @brief Get handle size in pixels
     * @return Handle radius (typically 5-7 pixels)
     */
    int GetHandleSize() const;
};

} // namespace DigitMode

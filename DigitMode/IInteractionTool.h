/**
 * @file IInteractionTool.h
 * @brief Tool interface for interaction system
 * 
 * Replaces IInputHandler with context-aware semantics.
 * Tools no longer see raw MFC events; instead they receive:
 * - Arbitrated hit results
 * - Keyboard state captured at event time
 * - Previous state for deltas
 * - View state generation for rendering
 */

#pragma once

#include "ToolCapabilities.h"
#include "ToolContext.h"
#include "DigitMode/Rendering/ShapeDrawStyle.h"
#include <afxwin.h>
#include <vector>

// Forward declarations
namespace aperture {
    class Shape;
}
class ViewTransform;

namespace DigitMode {

/**
 * @brief Result of hit-testing a tool
 * 
 * Unified result structure used by manager to arbitrate between tools.
 */
struct HitResult {
    bool hit = false;
    
    /// Which tool reported this hit (set by manager, not tool)
    class IInteractionTool* tool = nullptr;
    
    /// Tool-specific context (what was hit: shape index, handle, etc.)
    void* toolContext = nullptr;
    
    /// Distance from query point (for tie-breaking)
    double distance = std::numeric_limits<double>::max();
    
    /// Priority for arbitration (from tool capabilities)
    int priority = 0;
    
    /// Comparison for sorting (higher priority first, closer distance second)
    bool operator<(const HitResult& other) const {
        if (priority != other.priority) return priority > other.priority;
        if (distance != other.distance) return distance < other.distance;
        return false;
    }
};

/**
 * @brief Tool interface for interaction system
 * 
 * All interaction tools (Bounds, Fringe, Fiducials, etc.) implement this.
 * Replaces IInputHandler with added semantics for:
 * - Capability declaration
 * - Context-aware event handling
 * - View state generation
 * - Tool activation lifecycle
 */
class IInteractionTool {
public:
    virtual ~IInteractionTool() = default;
    
    // ========================================================================
    // Capability & Identity
    // ========================================================================
    
    /// Declare what this tool can do
    /// Called by InteractionManager during initialization and arbitration
    virtual ToolCapabilities GetCapabilities() const = 0;
    
    /// Get human-readable name for debugging/status
    virtual const char* GetName() const = 0;
    
    // ========================================================================
    // Hit-Testing (used by InteractionManager for arbitration)
    // ========================================================================
    
    /// Hit-test at screen point
    /// 
    /// @param screenPt Point in screen coordinates
    /// @param tolerance Hit tolerance in screen pixels (default 5)
    /// @return HitResult with hit=true if something hit, false otherwise
    /// 
    /// Note: Do NOT set hit.tool or hit.priority; manager does that.
    /// Just set: hit.hit, hit.toolContext, hit.distance
    virtual HitResult HitTest(CPoint screenPt, int tolerance = 5) = 0;
    
    // ========================================================================
    // Event Handling
    // ========================================================================
    
    /// Mouse button down
    /// 
    /// @param ctx Event context with mouse state, hit result, modifiers
    /// @return true if event consumed (prevents fallback), false to allow fallback
    /// 
    /// Manager calls this on either:
    /// - activeTool (if hit belongs to active tool)
    /// - captureTool (if foreign drag allowed and hit belongs to foreign tool)
    virtual bool OnMouseDown(const ToolContext& ctx) = 0;
    
    /// Mouse move (with or without button held)
    /// 
    /// Called for both:
    /// - activeTool (for mode-specific behavior)
    /// - captureTool (for drag operations)
    /// 
    /// @param ctx Event context with current position and deltas
    virtual void OnMouseMove(const ToolContext& ctx) = 0;
    
    /// Mouse button up
    /// 
    /// Called on whichever tool was handling the drag.
    /// 
    /// @param ctx Event context
    virtual void OnMouseUp(const ToolContext& ctx) = 0;
    
    /// Mouse wheel
    /// 
    /// @param ctx Event context
    /// @param zDelta Wheel delta (positive = up, negative = down)
    /// @return true if handled, false to allow fallback
    virtual bool OnMouseWheel(const ToolContext& ctx, short zDelta) = 0;
    
    /// Keyboard down
    /// 
    /// @param ctx Event context (ctx.hit may be empty for key-only events)
    /// @param nChar Virtual key code
    /// @return true if handled, false to allow other handlers
    virtual bool OnKeyDown(const ToolContext& ctx, UINT nChar) = 0;
    
    /// Cancel current operation (ESC key)
    /// 
    /// Tool should clean up any in-progress operations:
    /// - Cancel draft shapes
    /// - Release captures
    /// - Revert temporary previews
    virtual void Cancel() = 0;
    
    // ========================================================================
    // Activation Lifecycle (Tool Switching)
    // ========================================================================
    
    /// Called when tool becomes active
    /// 
    /// Safe place to:
    /// - Initialize state
    /// - Update UI (status bar, cursor)
    /// - Reset modes
    virtual void OnActivate() = 0;
    
    /// Called when tool becomes inactive
    /// 
    /// Safe place to:
    /// - Cancel drafts
    /// - Clear selections
    /// - Cleanup temporary state
    virtual void OnDeactivate() = 0;
    
    // ========================================================================
    // Visual Feedback (for rendering in OnDraw)
    // ========================================================================
    
    /// @brief Visual elements to render for this tool
    struct ViewState {
        /// Shape to render with its style
        struct Shape {
            const aperture::Shape* shape = nullptr;
            ShapeDrawStyle style;
        };
        std::vector<Shape> shapes;
        
        /// Cursor to display (nullptr = default arrow)
        HCURSOR cursor = nullptr;
        
        /// Status bar text (empty = no update)
        CString statusText;
        
        /// Tooltip text (empty = no tooltip)
        CString tooltip;
        
        /// Request another invalidation pass?
        /// Set true if animation or continuous update needed
        bool requestInvalidate = false;
    };
    
    /// Gather all visual elements for rendering
    /// 
    /// Called by InteractionManager during view state collection.
    /// 
    /// @param isActive true if this is the currently active tool
    /// @param isCapturing true if this tool currently has capture
    /// @return ViewState with shapes, cursor, text, tooltip
    /// 
    /// Example:
    /// @code
    /// ViewState state;
    /// 
    /// // Only show shapes if active or capturing
    /// if (isActive || isCapturing) {
    ///     if (IsDrafting()) {
    ///         state.shapes.push_back({
    ///             .shape = GetDraftPreview(),
    ///             .style = ShapeDrawStyle{.state = Draft}
    ///         });
    ///     }
    ///     if (IsDragging()) {
    ///         state.shapes.push_back({
    ///             .shape = GetPreviewShape(),
    ///             .style = ShapeDrawStyle{.state = Selected, .showHandles = true}
    ///         });
    ///     }
    /// }
    /// 
    /// // Status bar always available
    /// state.statusText = GetStatusMessage();
    /// state.cursor = GetCursor();
    /// 
    /// return state;
    /// @endcode
    virtual ViewState GetViewState(bool isActive, bool isCapturing) const = 0;
};

}  // namespace DigitMode

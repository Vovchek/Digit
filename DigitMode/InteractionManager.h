/**
 * @file InteractionManager.h
 * @brief Central tool manager and event router
 * 
 * Owns the tool set, maintains active/capture state, and routes input events
 * to the appropriate tool using capability-based arbitration.
 */

#pragma once

#include "IInteractionTool.h"
#include <afxwin.h>
#include <vector>
#include <memory>

namespace DigitMode {

/**
 * @brief Central tool manager and event router
 * 
 * Responsibilities:
 * - Maintain active tool and temporary capture tool state
 * - Arbitrate hit-testing between all tools
 * - Route input events to appropriate tool
 * - Coordinate visual feedback (cursor, status, shapes)
 * - Handle tool activation/deactivation
 * 
 * Architecture:
 * - activeTool: The currently selected tool (mode)
 * - captureTool: Tool that has temporary input capture (may differ from active)
 * - hoveredTool: Tool whose element is under mouse (for hover highlighting)
 * 
 * Example usage:
 * @code
 * // In CImageView::OnInitialUpdate:
 * manager.RegisterTool(&boundsToolAdapter);
 * manager.RegisterTool(&fringeToolAdapter);
 * manager.SetActiveTool(&fringeToolAdapter);
 * 
 * // In CBaseImageView::OnLButtonDown:
 * manager.OnMouseDown(nFlags, point);
 * 
 * // In CImageView::OnDraw:
 * auto viewState = manager.GetViewState();
 * for (auto& shape : viewState.shapes) Render(shape);
 * SetCursor(viewState.cursor);
 * @endcode
 */
class InteractionManager {
public:
    InteractionManager();
    ~InteractionManager();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /// Register a tool in the system (does not activate)
    /// 
    /// @param tool Pointer to tool (owned externally, not copied)
    /// @pre tool != nullptr
    void RegisterTool(IInteractionTool* tool);
    
    /// Set the currently active tool (mode switch)
    /// 
    /// @param tool Tool to activate (must be registered)
    /// @pre tool was registered or tool == nullptr
    /// 
    /// Calls:
    /// - old tool.OnDeactivate()
    /// - new tool.OnActivate()
    void SetActiveTool(IInteractionTool* tool);
    
    /// Get current active tool
    IInteractionTool* GetActiveTool() const { return m_activeTool; }
    
    /// Get tool that currently has capture (may differ from active)
    IInteractionTool* GetCaptureTool() const { return m_captureTool; }
    
    /// Get tool that is currently hovered (for hover highlighting)
    IInteractionTool* GetHoveredTool() const { return m_hoveredTool; }
    
    // ========================================================================
    // Event Routing (called by CBaseImageView message handlers)
    // ========================================================================
    
    /// Mouse button down
    /// 
    /// Arbitrates hit-testing across all tools and routes to appropriate handler.
    /// May set captureTool if foreign drag is allowed.
    /// 
    /// @param flags Mouse flags (MK_LBUTTON, etc.)
    /// @param pt Point in screen coordinates
    /// @return true if event consumed by tool (prevents fallback to navigation)
    bool OnMouseDown(UINT flags, CPoint pt);
    
    /// Mouse move
    /// 
    /// Routes to captureTool (if set) or activeTool.
    /// Updates hover state for all tools.
    /// 
    /// @param flags Mouse flags
    /// @param pt Point in screen coordinates
    /// @return true if event consumed (triggers invalidation request)
    bool OnMouseMove(UINT flags, CPoint pt);
    
    /// Mouse button up
    /// 
    /// Routes to captureTool (if set) or activeTool.
    /// Clears captureTool after handling.
    /// 
    /// @param flags Mouse flags
    /// @param pt Point in screen coordinates
    /// @return true if event consumed
    bool OnMouseUp(UINT flags, CPoint pt);
    
    /// Mouse wheel
    /// 
    /// Routes to activeTool only (wheel typically not used for cross-tool capture).
    /// 
    /// @param flags Mouse flags
    /// @param zDelta Wheel delta
    /// @param pt Point in screen coordinates
    /// @return true if handled
    bool OnMouseWheel(UINT flags, short zDelta, CPoint pt);
    
    /// Keyboard down
    /// 
    /// Routes to activeTool only.
    /// 
    /// @param nChar Virtual key code
    /// @return true if handled
    bool OnKeyDown(UINT nChar);
    
    /// Cancel (ESC key)
    /// 
    /// Called by NavigationInputHandler or explicit cancel request.
    /// Clears all active operations.
    void Cancel();
    
    // ========================================================================
    // View State (called by CImageView::OnDraw)
    // ========================================================================
    
    /// Composite view state from all active tools
    struct CompositeViewState {
        /// View state from each tool (in render order)
        struct Layer {
            IInteractionTool::ViewState toolState;
            IInteractionTool* tool = nullptr;
            bool isActive = false;
            bool isCapturing = false;
        };
        std::vector<Layer> layers;
        
        /// Resolved cursor (highest priority, or nullptr)
        HCURSOR cursor = nullptr;
        
        /// Resolved status text (from active or capture tool)
        CString statusText;
        
        /// Resolved tooltip (from hovered tool if not active)
        CString tooltip;
        
        /// If true, view should request invalidation for animation
        bool requestInvalidate = false;
    };
    
    /// Gather unified view state from all active tools
    /// 
    /// Called during OnDraw to get all rendering elements.
    /// Merges states from activeTool, captureTool, and hoveredTool.
    /// 
    /// @return CompositeViewState with shapes, cursor, text, etc.
    CompositeViewState GetViewState() const;
    
    // ========================================================================
    // State Queries
    // ========================================================================
    
    /// Is any tool currently performing a drag operation?
    /// Can be overridden by tools as needed.
    bool IsDragging() const;
    
    /// Is any tool currently creating a draft shape?
    /// Can be overridden by tools as needed.
    bool IsDrafting() const;
    
    // ========================================================================
    // Internal Implementation
    // ========================================================================
    
private:
    /// Find best hit across all tools
    /// @param screenPt Point to test
    /// @param tolerance Hit tolerance in pixels
    /// @return Best HitResult (may have hit=false if nothing hit)
    HitResult BestHit(CPoint screenPt, int tolerance = 5) const;
    
    /// Build tool context from event data
    /// @param flags Mouse flags
    /// @param pt Screen point
    /// @param hit Hit result to include
    ToolContext MakeContext(UINT flags, CPoint pt, const HitResult& hit = HitResult());
    
    /// Update modifier key state in context
    static void UpdateModifiers(ToolContext& ctx);
    
    /// Activate a tool (internal, handles lifecycle)
    void ActivateTool(IInteractionTool* tool);
    
    /// Clear capture (release temp tool)
    void ClearCapture();
    
    /// State
    std::vector<IInteractionTool*> m_tools;
    
    IInteractionTool* m_activeTool = nullptr;    // Current mode (Fringe, Bounds, etc.)
    IInteractionTool* m_captureTool = nullptr;   // Who has temporary capture
    IInteractionTool* m_hoveredTool = nullptr;   // Who is under mouse
    
    HitResult m_lastHit;                         // Last hit for comparing with new
    CPoint m_lastMousePoint = {0, 0};            // Last mouse position
    bool m_requestInvalidate = false;            // Should view be invalidated?
};

}  // namespace DigitMode

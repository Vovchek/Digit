/**
 * @file InputHandlerAdapter.h
 * @brief Adapter for wrapping existing IInputHandler into IInteractionTool
 * 
 * Allows incremental migration: existing handlers work without modification
 * by being wrapped in this adapter until they're refactored natively.
 */

#pragma once

#include "IInteractionTool.h"
#include "IInputHandler.h"
#include <memory>

namespace DigitMode {

/**
 * @brief Adapter to reuse existing IInputHandler as IInteractionTool
 * 
 * Bridges old input handler architecture to new interaction tool architecture.
 * 
 * Responsibilities:
 * - Wrap an IInputHandler (owns or borrows reference)
 * - Translate IInteractionTool calls to IInputHandler calls
 * - Provide minimal hit-testing (delegates if possible)
 * - Generate view state (can be extended per tool)
 * 
 * Usage:
 * @code
 * // Wrap existing handler
 * auto adapter = std::make_unique<InputHandlerAdapter>(
 *     &m_boundsInputHandler,
 *     "BoundsTool"
 * );
 * m_interactionManager.RegisterTool(adapter.get());
 * @endcode
 * 
 * Notes:
 * - Does NOT own the wrapped handler (caller is responsible)
 * - Can be extended per tool (derived classes override GetViewState, etc.)
 * - Temporary bridge during Phase 1; replace with native tools in Phase 2
 */
class InputHandlerAdapter : public IInteractionTool {
public:
    /// Construct adapter
    /// 
    /// @param handler Pointer to IInputHandler (not owned, must outlive adapter)
    /// @param name Human-readable tool name (for debugging)
    InputHandlerAdapter(IInputHandler* handler, const char* name);
    
    virtual ~InputHandlerAdapter() = default;
    
    // ========================================================================
    // IInteractionTool Implementation
    // ========================================================================
    
    /// Default capabilities for wrapped handler
    /// Can be overridden in derived classes
    ToolCapabilities GetCapabilities() const override;
    
    const char* GetName() const override { return m_name; }
    
    /// Hit-test delegation (may be empty if wrapped handler doesn't support)
    /// Can be overridden in derived classes for specific handlers
    HitResult HitTest(CPoint screenPt, int tolerance = 5) override;
    
    /// Translate to OnMouseDown on wrapped handler
    bool OnMouseDown(const ToolContext& ctx) override;
    
    /// Translate to OnMouseMove on wrapped handler
    void OnMouseMove(const ToolContext& ctx) override;
    
    /// Translate to OnMouseUp on wrapped handler
    void OnMouseUp(const ToolContext& ctx) override;
    
    /// Translate to OnMouseWheel on wrapped handler
    bool OnMouseWheel(const ToolContext& ctx, short zDelta) override;
    
    /// Translate to OnKeyDown on wrapped handler
    bool OnKeyDown(const ToolContext& ctx, UINT nChar) override;
    
    /// Translate to Cancel on wrapped handler
    void Cancel() override;
    
    /// Called when tool becomes active
    /// Default: empty (can be overridden)
    void OnActivate() override;
    
    /// Called when tool becomes inactive
    /// Default: calls Cancel() (can be overridden)
    void OnDeactivate() override;
    
    /// Get view state (default: empty)
    /// Derived classes should override to provide real view state
    ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
    // ========================================================================
    // Internal
    // ========================================================================
    
protected:
    /// Get wrapped handler
    IInputHandler* GetHandler() { return m_handler; }
    const IInputHandler* GetHandler() const { return m_handler; }
    
private:
    IInputHandler* m_handler;
    const char* m_name;
};

}  // namespace DigitMode

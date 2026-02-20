/**
 * @file InputHandlerAdapter.cpp
 * @brief Adapter implementation for wrapping IInputHandler as IInteractionTool
 */

#include "stdafx.h"
#include "InputHandlerAdapter.h"
#include "IInputHandler.h"

namespace DigitMode {

InputHandlerAdapter::InputHandlerAdapter(IInputHandler* handler, const char* name)
    : m_handler(handler)
    , m_name(name)
{
    ASSERT(handler && "Handler must not be null");
}

// ========================================================================
// Capability & Identity
// ========================================================================

ToolCapabilities InputHandlerAdapter::GetCapabilities() const
{
    // Default capabilities for wrapped handler
    // Can be overridden in derived classes for specific handlers
    ToolCapabilities caps;
    caps.supportsHover = true;
    caps.allowForeignDrags = true;
    caps.requiresExplicitActivation = false;
    caps.isExclusive = false;
    caps.hitTestPriority = 100;
    caps.name = m_name;
    return caps;
}

// ========================================================================
// Hit-Testing
// ========================================================================

HitResult InputHandlerAdapter::HitTest(CPoint screenPt, int tolerance)
{
    // Default: no hit (can be overridden in derived classes)
    // If wrapped handler has HitTest capability, override this method
    // and implement handler-specific logic
    return HitResult();
}

// ========================================================================
// Event Handling (Translate to IInputHandler)
// ========================================================================

bool InputHandlerAdapter::OnMouseDown(const ToolContext& ctx)
{
    if (!m_handler) return false;
    return m_handler->OnMouseDown(ctx.mouseFlags, ctx.screenPoint);
}

void InputHandlerAdapter::OnMouseMove(const ToolContext& ctx)
{
    if (!m_handler) return;
    m_handler->OnMouseMove(ctx.mouseFlags, ctx.screenPoint);
}

void InputHandlerAdapter::OnMouseUp(const ToolContext& ctx)
{
    if (!m_handler) return;
    m_handler->OnMouseUp(ctx.mouseFlags, ctx.screenPoint);
}

bool InputHandlerAdapter::OnMouseWheel(const ToolContext& ctx, short zDelta)
{
    if (!m_handler) return false;
    return m_handler->OnMouseWheel(ctx.mouseFlags, zDelta, ctx.screenPoint);
}

bool InputHandlerAdapter::OnKeyDown(const ToolContext& ctx, UINT nChar)
{
    if (!m_handler) return false;
    return m_handler->OnKeyDown(nChar);
}

void InputHandlerAdapter::Cancel()
{
    if (!m_handler) return;
    m_handler->Cancel();
}

// ========================================================================
// Activation Lifecycle
// ========================================================================

void InputHandlerAdapter::OnActivate()
{
    // Default: empty
    // Derived classes can override to initialize state specific to this tool
}

void InputHandlerAdapter::OnDeactivate()
{
    // Default: cancel wrapped handler
    // This ensures any active operations are cleaned up
    if (m_handler) {
        m_handler->Cancel();
    }
}

// ========================================================================
// Visual Feedback
// ========================================================================

IInteractionTool::ViewState InputHandlerAdapter::GetViewState(bool isActive, bool isCapturing) const
{
    // Default: empty state
    // Derived classes should override with actual tool-specific state:
    // - Shapes to render (draft, preview)
    // - Cursor to display
    // - Status text
    // - Tooltip text
    
    return ViewState();
}

}  // namespace DigitMode

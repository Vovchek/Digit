/**
 * @file InputRouter.cpp
 * @brief Implementation of central event routing
 */
#include "stdafx.h"
#include "InputRouter.h"

namespace DigitMode {

InputRouter::InputRouter()
    : m_activeTool(nullptr)
    , m_navigation(nullptr)
{
}

InputRouter::~InputRouter()
{
    // Pointers are not owned - no cleanup needed
}

// ========================================================================
// Configuration
// ========================================================================

void InputRouter::SetActiveTool(IInputHandler* tool)
{
    // Cancel previous tool before switching
    if (m_activeTool && m_activeTool != tool) {
        m_activeTool->Cancel();
    }
    
    m_activeTool = tool;
}

void InputRouter::SetNavigationHandler(IInputHandler* nav)
{
    m_navigation = nav;
}

// ========================================================================
// Event Routing (CANONICAL RULE: tool → navigation)
// ========================================================================

bool InputRouter::OnMouseDown(UINT flags, CPoint pt)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnMouseDown(flags, pt)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnMouseDown(flags, pt);
}

bool InputRouter::OnMouseMove(UINT flags, CPoint pt)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnMouseMove(flags, pt)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnMouseMove(flags, pt);
}

bool InputRouter::OnMouseUp(UINT flags, CPoint pt)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnMouseUp(flags, pt)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnMouseUp(flags, pt);
}

bool InputRouter::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnMouseWheel(flags, delta, pt)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnMouseWheel(flags, delta, pt);
}

bool InputRouter::OnKeyDown(UINT nChar)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnKeyDown(nChar)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnKeyDown(nChar);
}

bool InputRouter::OnKeyUp(UINT nChar)
{
    // Try active tool first
    if (m_activeTool && m_activeTool->OnKeyUp(nChar)) {
        return true;  // Consumed by tool
    }
    
    // Fallback to navigation
    return m_navigation && m_navigation->OnKeyUp(nChar);
}

// ========================================================================
// Lifecycle
// ========================================================================

void InputRouter::Cancel()
{
    // Cancel active tool
    if (m_activeTool) {
        m_activeTool->Cancel();
    }
    
    // Cancel navigation (if it has state)
    if (m_navigation) {
        m_navigation->Cancel();
    }
}

} // namespace DigitMode

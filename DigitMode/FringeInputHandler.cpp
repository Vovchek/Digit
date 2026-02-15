/**
 * @file FringeInputHandler.cpp
 * @brief Implementation of fringe input handler
 */
#include "stdafx.h"
#include "FringeInputHandler.h"
#include "DigitMode/DigitInfo.h"
#include "CommandDispatcher.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

FringeInputHandler::FringeInputHandler(CWnd* pView)
    : m_pView(pView)
    , m_pDigit(nullptr)
    , m_pTransform(nullptr)
    , m_pDispatcher(nullptr)
{
    ASSERT(m_pView && "View must not be null");
}

FringeInputHandler::~FringeInputHandler()
{
    // Pointers are not owned
}

// ========================================================================
// Initialization
// ========================================================================

void FringeInputHandler::Initialize(
    CDigitInfo* pDigit,
    ViewTransform* pTransform,
    CommandDispatcher* pDispatcher)
{
    m_pDigit = pDigit;
    m_pTransform = pTransform;
    m_pDispatcher = pDispatcher;
}

bool FringeInputHandler::IsInitialized() const
{
    return m_pDigit != nullptr && m_pTransform != nullptr && m_pDispatcher != nullptr;
}

// ========================================================================
// Mode Management
// ========================================================================

void FringeInputHandler::SetMode(EditMode mode)
{
    // Cancel current operation before switching
    Cancel();
    
    m_inputHandler.SetMode(mode);
    
    Invalidate();
}

EditMode FringeInputHandler::GetMode() const
{
    return m_inputHandler.GetMode();
}

// ========================================================================
// IInputHandler Implementation
// ========================================================================

bool FringeInputHandler::OnMouseDown(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;  // Not initialized, allow fallback
    }
    
    // Check for pan gesture (Space + LButton)
    // Return false to allow NavigationInputHandler to handle it
    if (IsSpacePressed() && (flags & MK_LBUTTON)) {
        return false;  // Don't consume, let navigation handle pan
    }
    
    // Left button
    if (flags & MK_LBUTTON) {
        m_inputHandler.OnLButtonDown(flags, pt, m_pDigit, m_pDispatcher);
        Invalidate();
        return true;  // Consumed
    }
    
    // Right button
    if (flags & MK_RBUTTON) {
        m_inputHandler.OnRButtonDown(flags, pt, m_pDigit, m_pDispatcher);
        Invalidate();
        return true;  // Consumed
    }
    
    return false;  // Other buttons not handled
}

bool FringeInputHandler::OnMouseMove(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Check for pan gesture (Space held during move)
    // Return false to allow NavigationInputHandler to handle it
    if (IsSpacePressed()) {
        return false;  // Don't consume, let navigation handle pan
    }
    
    // Build modifier state
    ModifierState mods;
    mods.ctrl = (flags & MK_CONTROL) != 0;
    mods.shift = (flags & MK_SHIFT) != 0;
    mods.alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    
    // Delegate to InputHandler
    m_inputHandler.OnMouseMove(pt, mods, m_pDigit, m_pDispatcher);
    Invalidate();
    
    // Don't consume mouse move (allow cursor updates, tooltips, etc.)
    return false;
}

bool FringeInputHandler::OnMouseUp(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Left button up
    if ((flags & MK_LBUTTON) == 0) {  // Button was just released
        m_inputHandler.OnLButtonUp(pt, m_pDigit, m_pDispatcher);
        Invalidate();
        return true;  // Consumed
    }
    
    return false;
}

bool FringeInputHandler::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    // Check for zoom gesture (Ctrl + Wheel)
    // Return false to allow NavigationInputHandler to handle it
    if (IsCtrlPressed()) {
        return false;  // Don't consume, let navigation handle zoom
    }
    
    // Fringe handler doesn't handle mouse wheel without Ctrl
    // Delegate to InputHandler
    if (IsInitialized()) {
        m_inputHandler.OnMouseWheel(pt, delta, m_pTransform);
        Invalidate();
    }
    
    return false;  // Don't consume (allow navigation fallback)
}

bool FringeInputHandler::OnKeyDown(UINT nChar)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Delegate to InputHandler (handles arrows, backspace, escape, etc.)
    m_inputHandler.OnKeyDown(nChar, m_pDigit, m_pDispatcher);
    Invalidate();
    
    // Consume key events that InputHandler handles
    switch (nChar) {
        case VK_ESCAPE:
        case VK_RETURN:
        case VK_BACK:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            return true;  // Consumed
        
        default:
            return false;  // Not handled, allow fallback
    }
}

bool FringeInputHandler::OnKeyUp(UINT nChar)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Delegate to InputHandler
    m_inputHandler.OnKeyUp(nChar, m_pDigit, m_pDispatcher);
    Invalidate();
    
    return false;  // Don't consume key-up events
}

void FringeInputHandler::Cancel()
{
    if (!IsInitialized()) {
        return;
    }
    
    // Cancel any active drawing
    m_inputHandler.CancelDraw(m_pDigit);
    
    // End pan if active (handled by InputHandler)
    if (m_inputHandler.m_isPanning) {
        m_inputHandler.EndPan();
    }
    
    Invalidate();
}

// ========================================================================
// Helpers
// ========================================================================

void FringeInputHandler::Invalidate()
{
    if (m_pView) {
        m_pView->Invalidate(FALSE);
    }
}

bool FringeInputHandler::IsSpacePressed() const
{
    return (::GetKeyState(VK_SPACE) & 0x8000) != 0;
}

bool FringeInputHandler::IsCtrlPressed() const
{
    return (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
}

} // namespace DigitMode

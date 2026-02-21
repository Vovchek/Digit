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

FringeInputHandler::FringeInputHandler()
    : m_pDigit(nullptr)
    , m_pTransform(nullptr)
    , m_pDispatcher(nullptr)
{
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

void FringeInputHandler::SetMode(FringeEditMode mode)
{
    // Cancel current operation before switching
    Cancel();
    
    m_inputHandler.SetMode(mode);
}

FringeEditMode FringeInputHandler::GetEditMode() const
{
    return m_inputHandler.GetEditMode();
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
    
    CPoint worldPt = pt;
    if (m_pTransform) {
        worldPt = m_pTransform->ScreenToWorld(pt);
    }

    // Left button
    if (flags & MK_LBUTTON) {
        m_inputHandler.OnLButtonDown(flags, worldPt, m_pDigit, m_pDispatcher);
        return true;  // Consumed
    }
    
    // Right button
    if (flags & MK_RBUTTON) {
        m_inputHandler.OnRButtonDown(flags, worldPt, m_pDigit, m_pDispatcher);
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

    CPoint worldPt = pt;
    if (m_pTransform) {
        worldPt = m_pTransform->ScreenToWorld(pt);
    }
    
    // Delegate to InputHandler
    m_inputHandler.OnMouseMove(worldPt, mods, m_pDigit, m_pDispatcher);
    
    return true;  // Consumed
}

bool FringeInputHandler::OnMouseUp(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }

    CPoint worldPt = pt;
    if (m_pTransform) {
        worldPt = m_pTransform->ScreenToWorld(pt);
    }
    
    // Left button up
    m_inputHandler.OnLButtonUp(worldPt, m_pDigit, m_pDispatcher);
    return true;  // Consumed
}

bool FringeInputHandler::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    // Check for zoom gesture (Ctrl + Wheel)
    // Return false to allow NavigationInputHandler to handle it
    if (IsCtrlPressed()) {
        return false;  // Don't consume, let navigation handle zoom
    }
    
    // Fringe handler doesn't handle mouse wheel without Ctrl
    return false;
}

bool FringeInputHandler::OnKeyDown(UINT nChar)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Delegate to InputHandler (handles arrows, backspace, escape, etc.)
    m_inputHandler.OnKeyDown(nChar, m_pDigit, m_pDispatcher);
    
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
    
    return false;  // Don't consume key-up events
}

bool FringeInputHandler::Cancel()
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Cancel any active drawing or drag via escape handling
    m_inputHandler.OnKeyDown(VK_ESCAPE, m_pDigit, m_pDispatcher);
    return true;
}

// ========================================================================
// Helpers
// ========================================================================

bool FringeInputHandler::IsSpacePressed() const
{
    return (::GetKeyState(VK_SPACE) & 0x8000) != 0;
}

bool FringeInputHandler::IsCtrlPressed() const
{
    return (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
}

} // namespace DigitMode

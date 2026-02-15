/**
 * @file NavigationInputHandler.cpp
 * @brief Implementation of navigation handler
 */
#include "stdafx.h"
#include "NavigationInputHandler.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

NavigationInputHandler::NavigationInputHandler(ViewTransform* pTransform, CWnd* pView)
    : m_pTransform(pTransform)
    , m_pView(pView)
    , m_isPanning(false)
    , m_panStart(0, 0)
{
    ASSERT(m_pTransform && "ViewTransform must not be null");
    ASSERT(m_pView && "View must not be null");
    
    m_panStartOffset.x = 0.0;
    m_panStartOffset.y = 0.0;
}

NavigationInputHandler::~NavigationInputHandler()
{
    // Pointers are not owned
}

// ========================================================================
// IInputHandler Implementation
// ========================================================================

bool NavigationInputHandler::OnMouseDown(UINT flags, CPoint pt)
{
    // Pan gesture: Space + Left Button
    if (IsSpacePressed() && (flags & MK_LBUTTON)) {
        m_isPanning = true;
        m_panStart = pt;
        m_panStartOffset = m_pTransform->GetOffset();
        
        // Change cursor to indicate panning mode
        ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
        
        return true;  // Consumed
    }
    
    return false;  // Not handled
}

bool NavigationInputHandler::OnMouseMove(UINT flags, CPoint pt)
{
    // Update pan
    if (m_isPanning) {
        CPoint delta = pt - m_panStart;
        
        // Apply pan offset
        CPoint2d newOffset;
        newOffset.x = m_panStartOffset.x + delta.x;
        newOffset.y = m_panStartOffset.y + delta.y;
        
        m_pTransform->SetOffset(newOffset);
        
        Invalidate();
        
        return true;  // Consumed
    }
    
    // Update cursor if Space is held (pan-ready state)
    if (IsSpacePressed()) {
        ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
        return true;  // Indicate we're in navigation mode
    }
    
    return false;  // Not handled
}

bool NavigationInputHandler::OnMouseUp(UINT flags, CPoint pt)
{
    // End pan
    if (m_isPanning) {
        m_isPanning = false;
        
        // Final pan update
        CPoint delta = pt - m_panStart;
        CPoint2d newOffset;
        newOffset.x = m_panStartOffset.x + delta.x;
        newOffset.y = m_panStartOffset.y + delta.y;
        
        m_pTransform->SetOffset(newOffset);
        
        Invalidate();
        
        // Restore cursor
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        
        return true;  // Consumed
    }
    
    return false;  // Not handled
}

bool NavigationInputHandler::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    // Zoom gesture: Ctrl + Wheel
    if (IsCtrlPressed()) {
        double currentScale = m_pTransform->GetScale();
        double factor = (delta > 0) ? 1.1 : (1.0 / 1.1);
        double newScale = currentScale * factor;
        
        // Clamp to reasonable range
        const double MIN_SCALE = 0.02;
        const double MAX_SCALE = 22.0;
        if (newScale < MIN_SCALE) newScale = MIN_SCALE;
        if (newScale > MAX_SCALE) newScale = MAX_SCALE;
        
        // Zoom centered on mouse position (world point stays under cursor)
        // Convert mouse point to world before zoom
        CPoint2d worldPt;
        CPoint2d offset = m_pTransform->GetOffset();
        worldPt.x = (pt.x - offset.x) / currentScale;
        worldPt.y = (pt.y - offset.y) / currentScale;
        
        // Set new scale
        m_pTransform->SetScale(newScale);
        
        // Adjust offset so worldPt remains at pt in screen coordinates
        CPoint2d newOffset;
        newOffset.x = pt.x - worldPt.x * newScale;
        newOffset.y = pt.y - worldPt.y * newScale;
        m_pTransform->SetOffset(newOffset);
        
        Invalidate();
        
        return true;  // Consumed
    }
    
    return false;  // Not handled (allow tool to handle wheel without Ctrl)
}

bool NavigationInputHandler::OnKeyDown(UINT nChar)
{
    // Escape cancels navigation state
    if (nChar == VK_ESCAPE) {
        Cancel();
        return true;  // Consumed
    }
    
    // Space key - just update cursor (no consumption, allow tools to see it)
    if (nChar == VK_SPACE) {
        ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
        return false;  // Don't consume - let tools know about Space too
    }
    
    return false;  // Not handled
}

bool NavigationInputHandler::OnKeyUp(UINT nChar)
{
    // Space released - end pan if active
    if (nChar == VK_SPACE && m_isPanning) {
        m_isPanning = false;
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        Invalidate();
        return true;  // Consumed
    }
    
    return false;  // Not handled
}

void NavigationInputHandler::Cancel()
{
    // Reset pan state
    if (m_isPanning) {
        m_isPanning = false;
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        Invalidate();
    }
}

// ========================================================================
// Helpers
// ========================================================================

bool NavigationInputHandler::IsSpacePressed() const
{
    return (::GetKeyState(VK_SPACE) & 0x8000) != 0;
}

bool NavigationInputHandler::IsCtrlPressed() const
{
    return (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
}

void NavigationInputHandler::Invalidate()
{
    if (m_pView) {
        m_pView->Invalidate(FALSE);  // FALSE = don't erase background
    }
}

} // namespace DigitMode

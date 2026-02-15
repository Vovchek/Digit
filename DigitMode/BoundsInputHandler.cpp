/**
 * @file BoundsInputHandler.cpp
 * @brief Implementation of bounds input handler
 */
#include "stdafx.h"
#include "BoundsInputHandler.h"
#include "Controls/CApertureCtrls.h"
#include "CommandDispatcher.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

BoundsInputHandler::BoundsInputHandler(CWnd* pView)
    : m_pView(pView)
{
    ASSERT(m_pView && "View must not be null");
}

BoundsInputHandler::~BoundsInputHandler()
{
    // Pointers are not owned
}

// ========================================================================
// Initialization
// ========================================================================

void BoundsInputHandler::Initialize(
    CApertureCtrls* pApertureCtrls,
    IImageData* pImage,
    ViewTransform* pTransform,
    CommandDispatcher* pDispatcher)
{
    m_boundsHandler.SetApertureCtrls(pApertureCtrls, pImage);
    m_boundsHandler.SetViewTransform(pTransform);
    m_boundsHandler.SetCommandDispatcher(pDispatcher);
}

bool BoundsInputHandler::IsInitialized() const
{
    return m_boundsHandler.IsInitialized();
}

// ========================================================================
// Mode Management
// ========================================================================

void BoundsInputHandler::SetEditMode(ShapeEditMode mode)
{
    // Cancel current operation before switching
    Cancel();
    
    m_boundsHandler.SetEditMode(mode);
    
    Invalidate();
}

ShapeEditMode BoundsInputHandler::GetEditMode() const
{
    return m_boundsHandler.GetEditMode();
}

// ========================================================================
// IInputHandler Implementation
// ========================================================================

bool BoundsInputHandler::OnMouseDown(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;  // Not initialized, allow fallback
    }
    
    // Route based on current mode
    ShapeEditMode mode = GetEditMode();
    
    if (mode == ShapeEditMode::Select) {
        return HandleSelectModeMouseDown(flags, pt);
    }
    else {
        // Add modes (Rectangle, Ellipse, Circle, Polygon)
        return HandleAddModeMouseDown(flags, pt);
    }
}

bool BoundsInputHandler::OnMouseMove(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Update drag preview if dragging
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.UpdateDrag(pt);
        Invalidate();
        return true;  // Consumed
    }
    
    // Update hover state for handle highlighting
    if (m_boundsHandler.UpdateHoveredHandle(pt)) {
        Invalidate();  // Redraw to show hover feedback
    }
    
    // Don't consume mouse move (allow cursor updates, tooltips, etc.)
    return false;
}

bool BoundsInputHandler::OnMouseUp(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // End drag operation (commit or cancel)
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.EndDrag(true);  // Commit = true
        Invalidate();
        return true;  // Consumed
    }
    
    return false;  // Not dragging, allow fallback
}

bool BoundsInputHandler::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
    // Bounds handler doesn't handle mouse wheel
    // Return false to allow NavigationInputHandler to zoom
    return false;
}

bool BoundsInputHandler::OnKeyDown(UINT nChar)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Delegate to BoundsHandler (handles Escape, Enter)
    return m_boundsHandler.OnKeyDown(nChar, 0, 0);
}

bool BoundsInputHandler::OnKeyUp(UINT nChar)
{
    // Currently no key-up handling needed
    return false;
}

void BoundsInputHandler::Cancel()
{
    if (!IsInitialized()) {
        return;
    }
    
    // Cancel any active drag
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.CancelDrag();
    }
    
    // Cancel any active draft
    if (m_boundsHandler.IsDrafting()) {
        m_boundsHandler.CancelDraft();
    }
    
    // Clear hover state
    m_boundsHandler.ClearHover();
    
    Invalidate();
}

// ========================================================================
// Helpers
// ========================================================================

void BoundsInputHandler::Invalidate()
{
    if (m_pView) {
        m_pView->Invalidate(FALSE);
    }
}

bool BoundsInputHandler::HandleSelectModeMouseDown(UINT flags, CPoint pt)
{
    // Only handle left button in Select mode
    if (!(flags & MK_LBUTTON)) {
        return false;  // Right-click, middle-click → allow fallback
    }
    
    // Hit-test to find shape/handle
    auto hit = m_boundsHandler.HitTest(pt);
    
    if (hit.hit && hit.isControlPoint()) {
        // Begin dragging a handle
        m_boundsHandler.BeginDrag(hit.type, hit.shapeIndex, hit.controlPointIndex, pt);
        Invalidate();
        return true;  // Consumed
    }
    
    if (hit.hit && hit.isBody()) {
        // Clicked on shape body (future: selection, move entire shape)
        // For now, just consume the event
        return true;  // Consumed
    }
    
    // Clicked on empty space → deselect (future implementation)
    return false;  // Not consumed, allow navigation fallback
}

bool BoundsInputHandler::HandleAddModeMouseDown(UINT flags, CPoint pt)
{
    // Only handle left button in Add modes
    if (!(flags & MK_LBUTTON)) {
        return false;  // Allow fallback
    }
    
    // Convert to world coordinates
    aperture::Point worldPt = m_boundsHandler.ScreenToAperturePoint(pt);
    
    // Add point to draft shape
    if (m_boundsHandler.AddDraftPoint(worldPt)) {
        Invalidate();
        return true;  // Consumed
    }
    
    return false;  // Draft not accepting points (shouldn't happen)
}

} // namespace DigitMode

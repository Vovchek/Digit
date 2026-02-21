/**
 * @file BoundsInputHandler.cpp
 * @brief Implementation of bounds input handler
 */
#include "stdafx.h"
#include "BoundsInputHandler.h"
#include "Controls/CApertureCtrls.h"
#include "CommandDispatcher.h"
#include "Commands/RemoveShapeCommand.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

BoundsInputHandler::BoundsInputHandler()
{
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

    // In add modes, treat right-button click as "commit draft" (finalize
    // current circle/ellipse/polygon) instead of adding another point.
    // This matches the UX spec: Enter / right-click commit, Esc cancels.
    if ((flags & MK_RBUTTON) != 0)
    {
        // Only commit in Add modes; in Select mode, let right-click fall
        // through so view can show context menus or other behavior.
        ShapeEditMode mode = GetEditMode();
        if (mode != ShapeEditMode::Select && m_boundsHandler.IsDrafting())
        {
            // CommitDraft() returns true if a valid shape was created and
            // dispatched via AddShapeCommand.
            if (m_boundsHandler.CommitDraft())
            {
                // Successfully committed - exit Add mode back to Select
                SetEditMode(ShapeEditMode::Select);
                return true; // consumed
            }
        }
        
        // Right-click in Add mode without valid draft - exit to Select mode
        if (mode != ShapeEditMode::Select && mode != ShapeEditMode::Delete) {
            SetEditMode(ShapeEditMode::Select);
            return true; // consumed
        }
        
        return false; // not handled, allow fallback (context menu, etc.)
    }

    // Route based on current mode
    ShapeEditMode mode = GetEditMode();
    
    if (mode == ShapeEditMode::Select) {
        return HandleSelectModeMouseDown(flags, pt);
    }
    else if (mode == ShapeEditMode::Delete) {
        return HandleDeleteModeMouseDown(flags, pt);  // Fix for Issue #2
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
    
    // Phase B: Update draft drag preview if dragging
    if (m_boundsHandler.IsDraftDragging()) {
        m_boundsHandler.UpdateDraftDrag(pt);
        // Invalidate view for live preview (caller should handle this via return value)
        return true;  // Consumed
    }
    
    // Update drag preview if dragging handle
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.UpdateDrag(pt);
        return true;  // Consumed
    }
    
    // Update hover state for handle highlighting
    if (m_boundsHandler.UpdateHoveredHandle(pt)) {
        return true;  // Consumed (state changed)
    }
    
    // Don't consume mouse move (allow cursor updates, tooltips, etc.)
    return false;
}

bool BoundsInputHandler::OnMouseUp(UINT flags, CPoint pt)
{
    if (!IsInitialized()) {
        return false;
    }
    
    // Phase B: Handle draft drag completion
    if (m_boundsHandler.IsDraftDragging()) {
        CPoint anchor = m_boundsHandler.GetDragAnchor();
        int dx = abs(pt.x - anchor.x);
        int dy = abs(pt.y - anchor.y);

        const int DRAG_THRESHOLD = 3;  // pixels (defined in BoundsHandler.h)

        if (dx < DRAG_THRESHOLD && dy < DRAG_THRESHOLD) {
            // Click (not drag): interpret as first perimeter sample only.
            // BeginDraftDrag() already seeded the draft with the anchor
            // point, so we just end drag mode and keep that single point.
            // Do NOT add another point here, otherwise the first click
            // produces two identical samples.
            m_boundsHandler.EndDraftDrag();  // leave draft with 1 point
        } else {
            // Drag detected - auto-commit bounding box shape
            m_boundsHandler.CommitDraftDrag();
            
            // Successfully committed - exit Add mode back to Select
            SetEditMode(ShapeEditMode::Select);
        }
        return true;  // Consumed
    }
    
    // End handle drag operation (commit or cancel)
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.EndDrag(true);  // Commit = true
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
    
    // Handle Esc key - cancel current operation and exit to Select mode
    if (nChar == VK_ESCAPE) {
        ShapeEditMode currentMode = GetEditMode();
        
        // Cancel any active operations first
        if (m_boundsHandler.IsDraftDragging()) {
            m_boundsHandler.EndDraftDrag();
            m_boundsHandler.CancelDraft();
        }
        else if (m_boundsHandler.IsDrafting()) {
            m_boundsHandler.CancelDraft();
        }
        else if (m_boundsHandler.IsDragging()) {
            m_boundsHandler.CancelDrag();
        }
        
        // Exit to Select mode from any mode
        if (currentMode != ShapeEditMode::Select) {
            SetEditMode(ShapeEditMode::Select);
        }
        
        return true;  // Consumed
    }
    
    // Handle Enter key - commit draft and exit to Select mode
    if (nChar == VK_RETURN) {
        if (m_boundsHandler.IsDrafting()) {
            // Try to commit the draft
            if (m_boundsHandler.CommitDraft()) {
                // Successfully committed - exit to Select mode
                SetEditMode(ShapeEditMode::Select);
                return true;  // Consumed
            }
        }
        return false;  // Not handled or commit failed
    }
    
    // Delegate other keys to BoundsHandler
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
    
    // Cancel draft drag if active (Phase B)
    if (m_boundsHandler.IsDraftDragging()) {
        m_boundsHandler.EndDraftDrag();
        m_boundsHandler.CancelDraft();
    }
    
    // Cancel any active handle drag
    if (m_boundsHandler.IsDragging()) {
        m_boundsHandler.CancelDrag();
    }
    
    // Cancel any active draft
    if (m_boundsHandler.IsDrafting()) {
        m_boundsHandler.CancelDraft();
    }
    
    // Clear hover state
    m_boundsHandler.ClearHover();
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
        return true;  // Consumed
    }
    
    if (hit.hit && hit.isBody()) {
        // Clicked on shape body → begin dragging to move entire shape
        // controlPointIndex = -1 indicates body drag (not a specific control point)
        m_boundsHandler.BeginDrag(hit.type, hit.shapeIndex, -1, pt);
        return true;  // Consumed
    }
    
    // Clicked on empty space → allow navigation fallback
    return false;  // Not consumed, allow navigation fallback
}

bool BoundsInputHandler::HandleDeleteModeMouseDown(UINT flags, CPoint pt)
{
    // Fix for Issue #2: Delete mode implementation
    
    // Only handle left button in Delete mode
    if (!(flags & MK_LBUTTON)) {
        return false;  // Right-click, middle-click → allow fallback
    }
    
    // Hit-test to find shape
    auto hit = m_boundsHandler.HitTest(pt);
    
    if (hit.hit) {
        // Found a shape - delete it
        auto cmd = std::make_unique<RemoveShapeCommand>(
            *m_boundsHandler.GetApertureCtrls(),
            hit.type,
            hit.shapeIndex
        );
        
        if (m_boundsHandler.GetDispatcher()) {
            m_boundsHandler.GetDispatcher()->Execute(std::move(cmd));
        }
        
        return true;  // Consumed
    }
    
    // Clicked on empty space
    return false;  // Not consumed, allow navigation fallback
}

bool BoundsInputHandler::HandleAddModeMouseDown(UINT flags, CPoint pt)
{
    // Only handle left button in Add modes
    if (!(flags & MK_LBUTTON)) {
        return false;  // Allow fallback
    }
    
    // Phase B: Drag-based creation vs point-sequence creation
    // If no draft points exist yet, begin drag mode
    // If draft already has points, continue with point-sequence mode
    
    if (!m_boundsHandler.HasDraftPoints()) {
        // No points yet - begin drag mode (user can drag or click)
        m_boundsHandler.BeginDraftDrag(pt);
        return true;  // Consumed
    } else {
        // Already have points - this is point-sequence mode
        aperture::Point worldPt = m_boundsHandler.ScreenToAperturePoint(pt);
        
        // Add point to draft shape
        if (m_boundsHandler.AddDraftPoint(worldPt)) {
            return true;  // Consumed
        }
    }
    
    return false;  // Draft not accepting points (shouldn't happen)
}

} // namespace DigitMode

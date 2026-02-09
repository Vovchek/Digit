# Bounds Editing Implementation Guide

## Complete Code Snippets & Templates

This document provides ready-to-implement code templates for the bounds editing restoration project.

---

## 1. BoundsHandler Header Template

**File**: `DigitMode/BoundsHandler.h`

```cpp
#pragma once

#include "MGTools/Include/Utils/BaseDataType.h"
#include "Controls/BoundCtrls.h"
#include "Controls/ImageCtrls.h"
#include "ViewTransform.h"

// Forward declarations
class CDigitInfo;

namespace DigitMode {

/**
 * @brief Handles interactive editing of image bounds (apertures)
 * 
 * Encapsulates all logic for:
 * - Hit-testing bound handles
 * - Drag state management
 * - Coordinate transformations via ViewTransform
 * - Preview bound calculation
 * - Handle rendering
 */
class BoundsHandler {
public:
    BoundsHandler();
    ~BoundsHandler();
    
    // =========== Initialization ===========
    /**
     * Set pointers to required data/view objects
     * Must call before any interaction
     */
    void SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage);
    void SetViewTransform(ViewTransform* pView);
    
    // =========== Hit Testing ===========
    /**
     * Check if screen point is on a bound handle
     * @param screenPt Point in client/screen coordinates
     * @param outBoundIdx Output: which bound (0-N)
     * @param outHandleIdx Output: which handle corner (0-3 for rect)
     * @return true if hit a handle; indices are valid
     */
    bool HitTestBoundHandle(const CPoint& screenPt, 
                            int& outBoundIdx, 
                            int& outHandleIdx) const;
    
    /**
     * Check if screen point is inside a bound (not just handle)
     */
    bool HitTestBoundInterior(const CPoint& screenPt, 
                              int& outBoundIdx) const;
    
    // =========== Drag Lifecycle ===========
    /**
     * Start dragging a bound handle
     * @param boundIdx Which bound to drag
     * @param handleIdx Which corner/handle (0=TL, 1=TR, 2=BR, 3=BL)
     * @param screenStart Initial screen position of cursor
     */
    void BeginDrag(int boundIdx, int handleIdx, const CPoint& screenStart);
    
    /**
     * Update drag to new cursor position
     * Computes new bound based on drag delta
     * @param screenCurrent Current screen position
     */
    void UpdateDrag(const CPoint& screenCurrent);
    
    /**
     * Finish dragging
     * @param bCommit true=apply changes, false=revert to original
     * @note Caller is responsible for creating undo command if desired
     */
    void EndDrag(bool bCommit);
    
    /**
     * Cancel dragging and revert to original state
     */
    void CancelDrag();
    
    // =========== State Query ===========
    bool IsDragging() const { return m_isDragging; }
    int GetActiveBoundIndex() const { return m_boundIndex; }
    int GetActiveHandleIndex() const { return m_handleIndex; }
    
    /**
     * Get the preview bound (world coordinates)
     * Valid during drag; returns snapshot outside of drag
     */
    CRect GetPreviewBound() const { return m_previewBound; }
    CRect GetOriginalBound() const { return m_boundSnapshot; }
    
    // =========== Rendering ===========
    /**
     * Draw handle indicators on DC
     * Called during drag to show interactive elements
     */
    void DrawHandles(CDC* pDC);
    
    /**
     * Draw preview of new bound during drag
     * Shows where bound will be if user releases now
     */
    void DrawPreviewBound(CDC* pDC);
    
    // =========== Coordinate Transforms ===========
    /**
     * Convert screen (client window) coordinates to world (image) coordinates
     * Accounts for current zoom/pan in ViewTransform
     */
    CPoint2d ScreenToWorld(const CPoint& screenPt) const;
    
    /**
     * Convert world (image) coordinates to screen (client window) coordinates
     * Accounts for current zoom/pan in ViewTransform
     */
    CPoint WorldToScreen(const CPoint2d& worldPt) const;

private:
    // =========== Internal Helpers ===========
    /**
     * Get current bound from CBoundCtrls
     * @return Bound rectangle in world coordinates
     */
    CRect GetCurrentBound() const;
    
    /**
     * Set bound in CBoundCtrls
     */
    void SetBound(const CRect& newBound);
    
    /**
     * Compute new bound based on drag handle movement
     * @param original Original bound rectangle
     * @param handleIdx Which corner is being dragged
     * @param worldDx World coordinate delta X
     * @param worldDy World coordinate delta Y
     * @return New bound after applying drag
     */
    CRect ComputeNewBoundFromDrag(const CRect& original, 
                                  int handleIdx,
                                  double worldDx, 
                                  double worldDy) const;
    
    /**
     * Get corner position in world coordinates
     * @param bound Rectangle in world coordinates
     * @param handleIdx Corner index (0=TL, 1=TR, 2=BR, 3=BL)
     * @return Corner position in world coordinates
     */
    CPoint2d GetHandleWorldPos(const CRect& bound, int handleIdx) const;
    
    /**
     * Check if screen point is near a handle position
     * @param screenPt Screen point to test
     * @param handleScreenPos Screen position of handle
     * @param tolerance Pixel tolerance (default 5)
     */
    bool IsNearHandle(const CPoint& screenPt, 
                      const CPoint& handleScreenPos,
                      int tolerance = 5) const;
    
    /**
     * Draw a single handle (small square/circle)
     */
    void DrawHandle(CDC* pDC, const CPoint& screenPos, 
                    bool bActive = false) const;
    
    /**
     * Draw rectangle outline from world coordinates
     */
    void DrawWorldRect(CDC* pDC, const CRect& worldRect,
                       COLORREF color = RGB(255, 0, 0)) const;

private:
    // =========== State ===========
    CBoundCtrls* m_pBounds = nullptr;
    CImageCtrls* m_pImage = nullptr;
    ViewTransform* m_pView = nullptr;
    
    // Drag state
    bool m_isDragging = false;
    int m_boundIndex = -1;      // Which bound being dragged
    int m_handleIndex = -1;     // Which corner (0-3)
    
    // Cursor tracking
    CPoint m_dragStart;         // Screen coords where drag began
    CPoint m_dragCurrent;       // Current screen position during drag
    
    // Bound snapshots
    CRect m_boundSnapshot;      // Original bound before drag
    CRect m_previewBound;       // Computed new bound during drag
    
    // Constants
    static constexpr int HANDLE_SIZE = 8;       // Pixels
    static constexpr int HANDLE_TOLERANCE = 5;  // Pixels for hit testing
};

} // namespace DigitMode
```

---

## 2. BoundsHandler Implementation Skeleton

**File**: `DigitMode/BoundsHandler.cpp`

```cpp
#include "stdafx.h"
#include "BoundsHandler.h"
#include "MGTools/Include/Graph/GraphTools.h"

namespace DigitMode {

BoundsHandler::BoundsHandler()
    : m_pBounds(nullptr), m_pImage(nullptr), m_pView(nullptr),
      m_isDragging(false), m_boundIndex(-1), m_handleIndex(-1),
      m_dragStart(0, 0), m_dragCurrent(0, 0)
{
}

BoundsHandler::~BoundsHandler()
{
    // No cleanup needed (pointers are not owned)
}

// ========================================================================
// Initialization
// ========================================================================

void BoundsHandler::SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage)
{
    m_pBounds = pBounds;
    m_pImage = pImage;
}

void BoundsHandler::SetViewTransform(ViewTransform* pView)
{
    m_pView = pView;
}

// ========================================================================
// Hit Testing
// ========================================================================

bool BoundsHandler::HitTestBoundHandle(const CPoint& screenPt,
                                       int& outBoundIdx,
                                       int& outHandleIdx) const
{
    ASSERT(m_pBounds && m_pImage && m_pView);
    
    // TODO: Implement based on current bound type
    // For now: assume BOUND_RECT, check all 4 corners
    
    CRect bound = GetCurrentBound();
    if (bound.IsRectEmpty()) return false;
    
    // Check each corner handle
    for (int i = 0; i < 4; ++i) {
        CPoint2d handleWorldPos = GetHandleWorldPos(bound, i);
        CPoint handleScreenPos = WorldToScreen(handleWorldPos);
        
        if (IsNearHandle(screenPt, handleScreenPos, HANDLE_TOLERANCE)) {
            outBoundIdx = 0;  // TODO: support multiple bounds
            outHandleIdx = i;
            return true;
        }
    }
    
    return false;
}

bool BoundsHandler::HitTestBoundInterior(const CPoint& screenPt,
                                         int& outBoundIdx) const
{
    // TODO: Implement
    return false;
}

// ========================================================================
// Drag Lifecycle
// ========================================================================

void BoundsHandler::BeginDrag(int boundIdx, int handleIdx, 
                              const CPoint& screenStart)
{
    ASSERT(!m_isDragging);
    ASSERT(m_pBounds && m_pImage && m_pView);
    
    m_isDragging = true;
    m_boundIndex = boundIdx;
    m_handleIndex = handleIdx;
    m_dragStart = screenStart;
    m_dragCurrent = screenStart;
    
    // Snapshot current bound
    m_boundSnapshot = GetCurrentBound();
    m_previewBound = m_boundSnapshot;
}

void BoundsHandler::UpdateDrag(const CPoint& screenCurrent)
{
    if (!m_isDragging) return;
    ASSERT(m_pBounds && m_pImage && m_pView);
    
    m_dragCurrent = screenCurrent;
    
    // Convert screen points to world coordinates
    CPoint2d worldStart = ScreenToWorld(m_dragStart);
    CPoint2d worldCurr = ScreenToWorld(screenCurrent);
    
    // Calculate delta in world coordinates
    double worldDx = worldCurr.x - worldStart.x;
    double worldDy = worldCurr.y - worldStart.y;
    
    // Compute new bound based on which handle is being dragged
    m_previewBound = ComputeNewBoundFromDrag(m_boundSnapshot, 
                                             m_handleIndex,
                                             worldDx, worldDy);
}

void BoundsHandler::EndDrag(bool bCommit)
{
    if (!m_isDragging) return;
    
    if (bCommit && m_previewBound != m_boundSnapshot) {
        SetBound(m_previewBound);
        // TODO: CommandDispatcher::Dispatch(MoveBoundCommand(...))
    }
    
    m_isDragging = false;
    m_boundIndex = -1;
    m_handleIndex = -1;
}

void BoundsHandler::CancelDrag()
{
    EndDrag(false);  // false = revert
}

// ========================================================================
// Coordinate Transforms
// ========================================================================

CPoint2d BoundsHandler::ScreenToWorld(const CPoint& screenPt) const
{
    ASSERT(m_pView);
    return m_pView->ScreenToWorld(screenPt);
}

CPoint BoundsHandler::WorldToScreen(const CPoint2d& worldPt) const
{
    ASSERT(m_pView);
    return m_pView->WorldToScreen(worldPt);
}

// ========================================================================
// Rendering
// ========================================================================

void BoundsHandler::DrawHandles(CDC* pDC)
{
    ASSERT(pDC && m_pView);
    
    CRect bound = m_isDragging ? m_previewBound : GetCurrentBound();
    if (bound.IsRectEmpty()) return;
    
    for (int i = 0; i < 4; ++i) {
        CPoint2d handleWorldPos = GetHandleWorldPos(bound, i);
        CPoint handleScreenPos = WorldToScreen(handleWorldPos);
        bool bActive = m_isDragging && (m_handleIndex == i);
        DrawHandle(pDC, handleScreenPos, bActive);
    }
}

void BoundsHandler::DrawPreviewBound(CDC* pDC)
{
    if (!m_isDragging) return;
    ASSERT(pDC && m_pView);
    
    DrawWorldRect(pDC, m_previewBound, RGB(255, 255, 0));  // Yellow preview
}

void BoundsHandler::DrawHandle(CDC* pDC, const CPoint& screenPos, bool bActive) const
{
    ASSERT(pDC);
    
    int size = HANDLE_SIZE;
    COLORREF color = bActive ? RGB(255, 0, 0) : RGB(0, 255, 0);
    
    CBrush brush(color);
    CBrush* oldBrush = pDC->SelectObject(&brush);
    
    CRect rcHandle(screenPos.x - size/2, screenPos.y - size/2,
                   screenPos.x + size/2, screenPos.y + size/2);
    pDC->Rectangle(rcHandle);
    
    if (oldBrush) pDC->SelectObject(oldBrush);
}

void BoundsHandler::DrawWorldRect(CDC* pDC, const CRect& worldRect, COLORREF color) const
{
    ASSERT(pDC && m_pView);
    
    // Convert world corners to screen
    CPoint tl = WorldToScreen(CPoint2d{(double)worldRect.left, (double)worldRect.top});
    CPoint tr = WorldToScreen(CPoint2d{(double)worldRect.right, (double)worldRect.top});
    CPoint br = WorldToScreen(CPoint2d{(double)worldRect.right, (double)worldRect.bottom});
    CPoint bl = WorldToScreen(CPoint2d{(double)worldRect.left, (double)worldRect.bottom});
    
    CPen pen(PS_SOLID, 2, color);
    CPen* oldPen = pDC->SelectObject(&pen);
    
    pDC->MoveTo(tl); pDC->LineTo(tr);
    pDC->LineTo(br); pDC->LineTo(bl); pDC->LineTo(tl);
    
    if (oldPen) pDC->SelectObject(oldPen);
}

// ========================================================================
// Internal Helpers
// ========================================================================

CRect BoundsHandler::GetCurrentBound() const
{
    ASSERT(m_pBounds);
    
    // TODO: Query current bound from CBoundCtrls
    // Placeholder:
    CRect bound;
    if (!m_pBounds->GetBoundRect(bound)) {
        return CRect(0, 0, 0, 0);
    }
    return bound;
}

void BoundsHandler::SetBound(const CRect& newBound)
{
    ASSERT(m_pBounds);
    // TODO: Call CBoundCtrls::SetBound() or similar
    // m_pBounds->SetBound(BOUND_RECT, newBound);
}

CPoint2d BoundsHandler::GetHandleWorldPos(const CRect& bound, int handleIdx) const
{
    switch (handleIdx) {
        case 0: return CPoint2d{(double)bound.left, (double)bound.top};      // TL
        case 1: return CPoint2d{(double)bound.right, (double)bound.top};     // TR
        case 2: return CPoint2d{(double)bound.right, (double)bound.bottom};  // BR
        case 3: return CPoint2d{(double)bound.left, (double)bound.bottom};   // BL
        default: return CPoint2d{0, 0};
    }
}

bool BoundsHandler::IsNearHandle(const CPoint& screenPt,
                                 const CPoint& handleScreenPos,
                                 int tolerance) const
{
    int dx = screenPt.x - handleScreenPos.x;
    int dy = screenPt.y - handleScreenPos.y;
    return (dx*dx + dy*dy) <= (tolerance*tolerance);
}

CRect BoundsHandler::ComputeNewBoundFromDrag(const CRect& original,
                                             int handleIdx,
                                             double worldDx,
                                             double worldDy) const
{
    CRect newBound = original;
    
    // Clamp deltas to integers for now
    int dx = (int)round(worldDx);
    int dy = (int)round(worldDy);
    
    switch (handleIdx) {
        case 0:  // TL: move top-left
            newBound.left += dx;
            newBound.top += dy;
            break;
        case 1:  // TR: move top-right
            newBound.right += dx;
            newBound.top += dy;
            break;
        case 2:  // BR: move bottom-right
            newBound.right += dx;
            newBound.bottom += dy;
            break;
        case 3:  // BL: move bottom-left
            newBound.left += dx;
            newBound.bottom += dy;
            break;
    }
    
    // Validate bound (ensure left < right, top < bottom)
    if (newBound.left >= newBound.right) {
        newBound.left = newBound.right - 1;
    }
    if (newBound.top >= newBound.bottom) {
        newBound.top = newBound.bottom - 1;
    }
    
    return newBound;
}

} // namespace DigitMode
```

---

## 3. InputHandler Extensions

**Changes to**: `DigitMode/InputHandler.h`

```cpp
// In EditMode enum, add:
enum class EditMode {
    Navigate,      // Existing
    Draw,          // Existing
    DotEdit,       // Existing
    BoundsExt,     // NEW: External aperture editing
    BoundsIns      // NEW: Internal obstruction editing
};

// In InputHandler class, add members:
private:
    // Bounds editing state
    struct BoundsEditState {
        bool isDragging = false;
        int boundIndex = -1;
        int handleIndex = -1;
        CPoint dragStart;
    } m_boundsEdit;
    
    BoundsHandler m_boundsHandler;  // NEW

public:
    // Accessors for bounds handler
    BoundsHandler& GetBoundsHandler() { return m_boundsHandler; }
    const BoundsHandler& GetBoundsHandler() const { return m_boundsHandler; }
    
    // Initialize bounds handler before use
    void InitBoundsHandler(CBoundCtrls* pBounds, CImageCtrls* pImage, ViewTransform* pView);
```

**Changes to**: `DigitMode/InputHandler.cpp`

```cpp
void InputHandler::InitBoundsHandler(CBoundCtrls* pBounds, CImageCtrls* pImage, ViewTransform* pView)
{
    m_boundsHandler.SetBoundsData(pBounds, pImage);
    m_boundsHandler.SetViewTransform(pView);
}

void InputHandler::SetMode(EditMode newMode)
{
    if (newMode == currentMode) return;
    
    // Finalize previous mode
    switch (currentMode) {
        case EditMode::BoundsExt:
        case EditMode::BoundsIns:
            // Abort any in-progress drag
            if (m_boundsHandler.IsDragging()) {
                m_boundsHandler.CancelDrag();
            }
            m_boundsEdit.isDragging = false;
            m_boundsEdit.boundIndex = -1;
            m_boundsEdit.handleIndex = -1;
            break;
            
        case EditMode::Draw:
            // Finalize active segment if in draw mode
            if (IsActiveSegmentValid()) {
                EndCurrentSegment();
            }
            break;
            
        default:
            break;
    }
    
    // Switch mode
    currentMode = newMode;
    
    // Initialize new mode
    switch (currentMode) {
        case EditMode::BoundsExt:
        case EditMode::BoundsIns:
            // Ready for bounds editing
            // Caller will trigger interactive mode via mouse events
            break;
            
        case EditMode::Navigate:
        case EditMode::Draw:
        case EditMode::DotEdit:
            // Handled in existing code
            break;
    }
}
```

---

## 4. ImageView Integration

**Changes to**: `ImageTempl/ImageView.cpp`

```cpp
// In OnInitialUpdate() or similar initialization:
void CImageView::OnInitialUpdate()
{
    CBaseImageView::OnInitialUpdate();
    
    // ... existing code ...
    
    // Initialize InputHandler with bounds data
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    if (pDoc) {
        m_inputHandler.InitBoundsHandler(&pDoc->boundCtrls, 
                                        GetImageCtrls(this),
                                        &m_viewTransform);
    }
}

// Message handlers for bounds mode
void CImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // Route through InputHandler
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        
        int boundIdx, handleIdx;
        if (m_inputHandler.GetBoundsHandler().HitTestBoundHandle(point, boundIdx, handleIdx)) {
            m_inputHandler.GetBoundsHandler().BeginDrag(boundIdx, handleIdx, point);
            SetCapture();
            return;
        }
    }
    
    // Default handling
    m_inputHandler.OnMouseDown(point, nFlags, &GetDocument()->Digit);
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        
        if (m_inputHandler.GetBoundsHandler().IsDragging()) {
            m_inputHandler.GetBoundsHandler().UpdateDrag(point);
            Invalidate(FALSE);
            return;
        }
    }
    
    // Default handling
    m_inputHandler.OnMouseMove(point, &GetDocument()->Digit);
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        
        if (m_inputHandler.GetBoundsHandler().IsDragging()) {
            m_inputHandler.GetBoundsHandler().EndDrag(true);
            ReleaseCapture();
            Invalidate(FALSE);
            return;
        }
    }
    
    // Default handling
    m_inputHandler.OnMouseUp(point, &GetDocument()->Digit, m_cmdDispatcher);
}

void CImageView::OnDraw(CDC* pDC)
{
    // ... existing drawing code ...
    
    // Draw bounds feedback
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        m_inputHandler.GetBoundsHandler().DrawPreviewBound(pDC);
        m_inputHandler.GetBoundsHandler().DrawHandles(pDC);
    }
}

// Escape key support (cancel drag)
void CImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE) {
        if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
            m_inputHandler.GetMode() == EditMode::BoundsIns) {
            m_inputHandler.GetBoundsHandler().CancelDrag();
            Invalidate(FALSE);
            return;
        }
    }
    
    // Default handling
    CBaseImageView::OnKeyDown(nChar, nRepCnt, nFlags);
}
```

---

## 5. Mode Activation (ImageDoc)

**Changes to**: `ImageTempl/ImageDoc.cpp`

```cpp
void CImageDoc::ActivateExtBounds(BOOL key)
{
    CImageView* pView = GetView();
    if (!pView) return;
    
    if (key) {
        // Enter bounds editing mode
        pView->GetInputHandler().SetMode(EditMode::BoundsExt);
        SetInfo(I_BOUNDS_EXT, TRUE);
    } else {
        // Exit bounds editing mode
        pView->GetInputHandler().SetMode(EditMode::Navigate);
        SetInfo(I_BOUNDS_EXT, FALSE);
    }
    
    Invalidate(FALSE);
}

void CImageDoc::ActivateInsBounds(BOOL key)
{
    CImageView* pView = GetView();
    if (!pView) return;
    
    if (key) {
        pView->GetInputHandler().SetMode(EditMode::BoundsIns);
        SetInfo(I_BOUNDS_INS, TRUE);
    } else {
        pView->GetInputHandler().SetMode(EditMode::Navigate);
        SetInfo(I_BOUNDS_INS, FALSE);
    }
    
    Invalidate(FALSE);
}
```

---

## 6. Unit Test Template

**File**: `Tests/DigitModeTests/BoundsHandlerTest.cpp`

```cpp
#include "CppUnitTest.h"
#include "DigitMode/BoundsHandler.h"
#include "ViewTransform.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace DigitMode;

TEST_CLASS(BoundsHandlerTest) {
private:
    ViewTransform m_view;
    BoundsHandler m_handler;
    // Mock objects would go here
    
public:
    TEST_METHOD(ScreenToWorldNoTransform) {
        // Arrange
        m_view.SetOffset(CPoint2d{0, 0});
        // Assuming scale = 1.0 by default
        m_handler.SetViewTransform(&m_view);
        
        CPoint screenPt(100, 200);
        
        // Act
        CPoint2d worldPt = m_handler.ScreenToWorld(screenPt);
        
        // Assert
        Assert::AreEqual(100.0, worldPt.x, 0.1);
        Assert::AreEqual(200.0, worldPt.y, 0.1);
    }
    
    TEST_METHOD(ScreenToWorldWithZoom) {
        // Arrange
        // TODO: Add scale method to ViewTransform if needed
        // For now, assume scale=2.0
        m_handler.SetViewTransform(&m_view);
        
        CPoint screenPt(100, 100);
        
        // Act
        // With scale 2.0 and no offset, screen(100,100) → world(50,50)
        // TODO: Implement test once ViewTransform has public zoom setting
        
        // Assert
    }
    
    TEST_METHOD(DragGeometryTopLeft) {
        // Arrange
        // Set up a bound at (100, 100) → (200, 200)
        CRect originalBound(100, 100, 200, 200);
        
        // Drag TL corner by (10, 20) in world coordinates
        CRect expected(110, 120, 200, 200);
        
        // Act
        // TODO: Mock CBoundCtrls and test via public interface
        
        // Assert
    }
    
    TEST_METHOD(IsDraggingStateTransition) {
        // Arrange
        Assert::IsFalse(m_handler.IsDragging());
        
        // Act
        m_handler.BeginDrag(0, 0, CPoint(50, 50));
        
        // Assert
        Assert::IsTrue(m_handler.IsDragging());
        
        // Act
        m_handler.EndDrag(false);
        
        // Assert
        Assert::IsFalse(m_handler.IsDragging());
    }
};
```

---

## 7. Integration Test Sketch

```cpp
TEST_CLASS(BoundsEditingIntegrationTest) {
public:
    TEST_METHOD(FullDragWorkflowWithUndo) {
        // Setup
        CImageDoc doc;
        CImageView view;
        doc.OnNewDocument();
        // Load test image...
        
        // Activate bounds editing
        doc.ActivateExtBounds(TRUE);
        Assert::AreEqual((int)EditMode::BoundsExt, (int)view.GetInputHandler().GetMode());
        
        // Simulate user clicking on bound handle
        int boundIdx, handleIdx;
        CPoint handleScreenPos(150, 150);
        
        if (view.GetInputHandler().GetBoundsHandler()
            .HitTestBoundHandle(handleScreenPos, boundIdx, handleIdx)) {
            
            // Simulate drag
            view.GetInputHandler().GetBoundsHandler()
                .BeginDrag(boundIdx, handleIdx, handleScreenPos);
            
            view.GetInputHandler().GetBoundsHandler()
                .UpdateDrag(CPoint(200, 200));
            
            // Check preview
            CRect preview = view.GetInputHandler().GetBoundsHandler().GetPreviewBound();
            Assert::IsFalse(preview.IsRectEmpty());
            
            // Commit
            view.GetInputHandler().GetBoundsHandler().EndDrag(true);
            
            // Verify command was created (check undo buffer)
            // Assert: undo buffer has MoveBoundCommand
        }
    }
};
```

---

## 8. Checklist for Implementation

### Phase 1: BoundsHandler Core
- [ ] Create `BoundsHandler.h` with interface
- [ ] Create `BoundsHandler.cpp` with implementations
- [ ] Implement `ScreenToWorld()` and `WorldToScreen()`
- [ ] Implement `HitTestBoundHandle()`
- [ ] Implement `BeginDrag/UpdateDrag/EndDrag` state machine
- [ ] Implement `DrawHandles()` and `DrawPreviewBound()`
- [ ] Unit test coordinate transforms
- [ ] Unit test drag geometry
- [ ] Unit test state machine

### Phase 2: InputHandler Integration
- [ ] Extend `EditMode` enum
- [ ] Add bounds-related members to `InputHandler`
- [ ] Implement `SetMode()` with bounds cleanup
- [ ] Add `InitBoundsHandler()` method
- [ ] Unit test mode switching
- [ ] Integration test mode transitions

### Phase 3: ImageView Integration
- [ ] Route `OnLButtonDown` through InputHandler
- [ ] Route `OnMouseMove` through InputHandler
- [ ] Route `OnLButtonUp` through InputHandler
- [ ] Add bounds rendering to `OnDraw()`
- [ ] Update `ImageDoc.ActivateExtBounds/ActivateInsBounds`
- [ ] Test with real UI interactions

### Phase 4: Undo/Redo
- [ ] Create `MoveBoundCommand` class
- [ ] Integrate with `CommandDispatcher`
- [ ] Test undo/redo workflow
- [ ] Test preview vs committed state

### Phase 5: Testing & Documentation
- [ ] Visual regression tests
- [ ] Stress tests (extreme zoom/pan)
- [ ] Document public API
- [ ] Create migration guide
- [ ] Code review with team

---

## 9. Common Pitfalls & Solutions

| Pitfall | Cause | Solution |
|---------|-------|----------|
| Handles not clickable | ViewTransform coordinate conversion error | Verify ScreenToWorld is inverse of WorldToScreen; add unit test |
| Preview jumps when dragging | Mouse capture or event ordering issue | Ensure capture/release are paired; log drag deltas |
| Undo doesn't work | Command not dispatched | Verify EndDrag calls CommandDispatcher; check undo buffer |
| Mode not switching | SetMode not called from ImageDoc | Add logging in SetMode to verify it's called |
| Crash on null pointer | BoundsHandler not initialized | Call InitBoundsHandler() before use; add asserts |
| Coordinate off by N pixels | Rounding errors in transforms | Use double precision throughout; round only at final screen draw |

---

## 10. Future Enhancements (Post-Phase 1)

1. **Polygon bound editing**: Extend `BoundsHandler` to support per-vertex dragging
2. **Snapping**: Snap handles to image boundaries or grid
3. **Constraints**: Keep bounds within image, maintain aspect ratio
4. **Multi-bound editing**: Edit multiple bounds in one operation
5. **Keyboard shortcuts**: Arrow keys to nudge bounds
6. **Copy/paste bounds**: Between images or documents
7. **Bound templates**: Pre-defined bound shapes and sizes


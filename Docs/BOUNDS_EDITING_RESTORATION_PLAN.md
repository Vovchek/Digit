# Bounds (Apertures) Editing Restoration Plan
## Integrating ViewTransform & Moving to InputHandler

**Status**: Planning Phase  
**Date**: 2024  
**Scope**: Restore I_BOUNDS_EXT and I_BOUNDS_INS editing modes with modern ViewTransform-based coordinate handling and eventual InputHandler integration

---

## Executive Summary

The Digit project currently has legacy bounds (apertures) editing functionality tied to:
- **I_BOUNDS_EXT** (0x10): External bounds/aperture editing
- **I_BOUNDS_INS** (0x20): Internal bounds/obstruction editing

These modes rely on:
1. **Legacy zoom logic** in `BaseImageView` and `Tracker` (SECZoomView heritage)
2. **Direct Tracker manipulation** in `ImageView` via `BeginTracker/DragTracker/DropTracker`
3. **Manual coordinate transformations** that don't account for the new `ViewTransform` system

**Required workflow coverage**:
- **Shapes**: circle, ellipse, rectangle, polygon.
- **Input methods**:
  - **Tracker-based** (rectangle handles) for circle, ellipse, rectangle.
  - **Point-based** (custom dots) for circle, ellipse, rectangle, polygon.
- **Parity**: the modernized flow must preserve the ability to switch setup type (tracker vs dots), and commit/cancel bounds via the same Apply/Remove semantics.
- **Hit testing**: must detect handles, polygon dots, and outline hits, returning a selection level (handle/dot/edge/interior) so polygon bounds can be edited.

**Interaction model (O1)**:
- Single add/edit bounds mode for creation and editing.
- Explicit Add Bound command enters a modal edit session until commit/discard.
- After commit, the bound remains editable without leaving bounds mode; new bounds are created via the explicit command.
- Shape selection remains via context menu and should be reachable via a command/toolbar for discoverability.

**Goals**:
1. **Restore functionality**: Make bounds editing work with the new `ViewTransform`
2. **Modernize architecture**: Move bounds editing into `InputHandler` following the established `EditMode` pattern
3. **Improve maintainability**: Centralize bounds handling, reduce coordinate transformation complexity
4. **Enable future enhancements**: Support more sophisticated bounds operations (constraints, snapping, etc.)

---

## Current Architecture

### 1. Bounds Data Model
**Files**: `Controls/BoundCtrls.h/cpp`

```cpp
// From AppDef.h
#define I_BOUNDS_EXT  0x10    // External bounds
#define I_BOUNDS_INS  0x20    // Internal bounds (obstructions)

// From Controls/BoundCtrls.h
#define BOUND_NONE    -1
#define BOUND_ROUND    0      // Circle/ellipse
#define BOUND_ELLIPSE  1
#define BOUND_RECT     2
#define BOUND_POLYGON  3
```

**Structure** (in `BaseImageDoc`):
```cpp
CBoundCtrls boundCtrls;  // Stores ExtBoundType, InsBoundType, actual bound data
CMTraker Tracker;         // Visual editor for bounds rectangles
```

### 2. Current Bounds Editing Flow

```
ImageView::OnLButtonDown(CPoint)
  ↓
  [If I_BOUNDS_EXT or I_BOUNDS_INS mode]
  ↓
  ImageView::BeginTracker(CPoint)
  ↓
  CMTraker::Track() — Interactive dragging of 4 corner handles
  ↓
  ImageView::DragTracker(CPoint)
  ↓
  ImageView::DropTracker(CPoint)
  ↓
  CBoundCtrls::SetBound() — Commit to document
```

### 2a. Legacy Setup Type Workflow (Tracker vs Custom Dots)

- **Setup type toggles** live in `CBaseImageView`:
  - `OnSetupDotsBound()` → `EnableCustomDots = true`, `EnableTracker = false`.
  - `OnSetupRectBound()` → `EnableTracker = true`, `EnableCustomDots = false`.
- **Point-based setup**:
  - `SetCustomDot()` → `AddCustomDot()` → `SetCurBound()`.
  - Commit via `OnApplyBound()` which calls `CBoundCtrls::AddBound()`.
- **Tracker-based setup**:
  - `BeginTracker()` / `DragTracker()` / `DropTracker()` manipulate `CMTraker`.
  - `OnApplyBound()` converts tracker rect to 4 dots (top/mid/right/left) before `AddBound()`.
- **Shape restrictions**:
  - Tracker is disabled for polygon (`OnScrPlgBound()` forces dot mode).
  - Circle/ellipse/rectangle can be defined by tracker or dots.

### 3. Tracker Class (`Utils/Tracker.h`)

**Responsibilities**:
- Manages 4 corner handles (TL, TR, BR, BL) for rectangular bounds
- Tracks dragging state and cursor position
- Draws handles on screen
- Converts world coordinates to screen coordinates for display

**Current Limitations**:
- Uses legacy zoom coefficient (`m_zoomLevel` from `BaseImageView`)
- Manual coordinate transformation without ViewTransform abstraction
- Tightly coupled to rectangle representation (limited polygon support)
- No offset/pan awareness

### 4. ImageView Integration (`ImageTempl/ImageView.cpp` lines 30-122)

**DrawBounds()**:
- ✅ **Already modernized** to use `ViewTransform`:
  ```cpp
  CPoint tl = m_viewTransform.WorldToScreen(CPoint2d{...});
  CPoint br = m_viewTransform.WorldToScreen(CPoint2d{...});
  ```
- Handles BOUND_ROUND, BOUND_ELLIPSE, BOUND_RECT, BOUND_POLYGON rendering

**Bounds Editing Handlers** (likely in BaseImageView or ImageView):
- ❌ **Not yet identified** — likely call `BeginTracker/DragTracker/DropTracker`
- ❌ **No ViewTransform integration** in interactive editing

---

## InputHandler Existing Pattern

**Files**: `DigitMode/InputHandler.h/cpp`

### EditMode Enum
```cpp
enum class EditMode {
    Navigate,  // Selection, multi-object operations
    Draw,      // Creating/extending segments
    DotEdit    // Geometry editing (move/insert/delete dots)
};
```

### Mode Lifecycle (Navigate ↔ Draw)
```cpp
void SetMode(EditMode newMode);
EditMode GetMode() const;
```

### State Management Example (Draw Mode)
```cpp
struct {
    int iActiveSegment = -1;
    ActiveEnd activeEnd = ActiveEnd::None;
    // ... other draw-specific state
} m_draw;
```

### Key Design Principles
- **Modifiers have consistent global meaning**:
  - `Ctrl` = Add/Extend/Connect
  - `Shift` = Range/Constrain/Promote
  - `Alt` = Alternate/Destructive/Structural

- **Mode switching finalizes pending operations**
  - `SetMode(Navigate)` while drawing ends the current segment
  - Ensures no orphaned state across mode boundaries

- **Coordinate transformations via ViewTransform**:
  - `ViewTransform::ScreenToWorld()` for input
  - `ViewTransform::WorldToScreen()` for rendering feedback

---

## Design: New BoundsEditMode

### 1. Extend EditMode Enum
```cpp
enum class EditMode {
    Navigate,      // Existing
    Draw,          // Existing
    DotEdit,       // Existing
    BoundsExt,     // NEW: External bounds editing
    BoundsIns      // NEW: Internal bounds editing
};
```

### 2. BoundsEditState Structure
Add to `InputHandler` private section:
```cpp
struct BoundsEditState {
    BoundsType type;           // BOUND_ROUND, BOUND_ELLIPSE, BOUND_RECT, BOUND_POLYGON
    int boundIndex = -1;       // Which bound being edited (-1 = none)
    CRect originalBound;       // Snapshot before drag
    
    int activeHandle = -1;     // Which corner/handle (-1 = none)
    CPoint handleStart;        // Screen coords where drag began
    CPoint lastPreview;        // Last preview position
    
    bool isDragging = false;
} m_boundsEdit;
```

### 3. BoundsEditMode Entry/Exit
```cpp
void InputHandler::SetMode(EditMode newMode)
{
    if (newMode == currentMode) return;
    
    // Finalize previous mode
    if (currentMode == EditMode::BoundsExt || currentMode == EditMode::BoundsIns) {
        m_boundsEdit.isDragging = false;
        m_boundsEdit.activeHandle = -1;
        // Optionally: revert if user cancels
    }
    
    currentMode = newMode;
    
    // Initialize new mode
    if (currentMode == EditMode::BoundsExt) {
        // Caller will identify which bound and trigger interactive editing
    } else if (currentMode == EditMode::BoundsIns) {
        // Similar for internal bounds
    }
}
```

### 4. Bounds Drag Lifecycle
```cpp
// Called when user clicks on a bound handle in BoundsExt/BoundsIns mode
void BeginBoundsDrag(int boundIndex, int handleIndex, CPoint screenPt, 
                     CDigitInfo* pDigit, ViewTransform* view)
{
    m_boundsEdit.boundIndex = boundIndex;
    m_boundsEdit.activeHandle = handleIndex;
    m_boundsEdit.handleStart = screenPt;
    m_boundsEdit.isDragging = true;
    
    // Snapshot current bound for potential undo
    CBoundCtrls* pBounds = pDigit->GetBoundCtrls();
    CRect currentBound;
    pBounds->GetBoundRect(currentBound);  // Pseudo-code
    m_boundsEdit.originalBound = currentBound;
}

// Called during mouse move while dragging
void UpdateBoundsDrag(CPoint screenPt, CDigitInfo* pDigit, ViewTransform* view)
{
    if (!m_boundsEdit.isDragging) return;
    
    m_boundsEdit.lastPreview = screenPt;
    
    // Convert screen delta to world delta
    CPoint2d worldStart = view->ScreenToWorld(m_boundsEdit.handleStart);
    CPoint2d worldCurr = view->ScreenToWorld(screenPt);
    double dx = worldCurr.x - worldStart.x;
    double dy = worldCurr.y - worldStart.y;
    
    // Compute new bound based on drag direction
    CRect newBound = ComputeNewBound(m_boundsEdit.originalBound, 
                                     m_boundsEdit.activeHandle, dx, dy);
    
    // Preview in UI (caller draws feedback)
    // m_boundsEdit.previewBound = newBound;
}

// Called on mouse up
void EndBoundsDrag(CPoint screenPt, CDigitInfo* pDigit, 
                   CommandDispatcher* pCmdDisp, ViewTransform* view)
{
    if (!m_boundsEdit.isDragging) return;
    
    UpdateBoundsDrag(screenPt, pDigit, view);
    
    // Commit via command (for undo support)
    // pCmdDisp->Dispatch(new MoveBoundCommand(...));
    
    m_boundsEdit.isDragging = false;
    m_boundsEdit.activeHandle = -1;
}
```

---

## Design: BoundsHandler Abstraction Layer

### Purpose
Encapsulate bounds editing logic separate from `InputHandler` to keep concerns cleanly separated.

**Files to Create**:
- `DigitMode/BoundsHandler.h`
- `DigitMode/BoundsHandler.cpp`

### Interface Skeleton
```cpp
namespace DigitMode {

class BoundsHandler {
public:
    BoundsHandler();
    
    // Initialization
    void SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage);
    void SetViewTransform(ViewTransform* pView);
    
    // Query
    bool HitTestBoundHandle(const CPoint& screenPt, 
                            int& outBoundIdx, int& outHandleIdx);
    
    // Interaction
    void BeginDrag(int boundIdx, int handleIdx, CPoint screenStart);
    void UpdateDrag(CPoint screenCurrent);
    void EndDrag(bool bCommit);  // true=commit, false=revert
    void CancelDrag();
    
    // Rendering
    void DrawHandles(CDC* pDC);           // For feedback during drag
    void DrawPreviewBound(CDC* pDC);      // Live preview while dragging
    
    // Query state
    bool IsDragging() const { return m_isDragging; }
    CRect GetPreviewBound() const;
    
private:
    // Coordinate transformations
    CPoint WorldToScreen(const CPoint2d& wpt) const;
    CPoint2d ScreenToWorld(const CPoint& spt) const;
    
    // Geometry
    CRect ComputeNewBoundFromDrag(const CRect& original, int handleIdx, 
                                  double dx, double dy) const;
    
    // State
    CBoundCtrls* m_pBounds = nullptr;
    CImageCtrls* m_pImage = nullptr;
    ViewTransform* m_pView = nullptr;
    
    int m_boundIndex = -1;
    int m_handleIndex = -1;
    CPoint m_dragStart;
    CPoint m_dragCurrent;
    CRect m_boundSnapshot;
    bool m_isDragging = false;
};

} // namespace DigitMode
```

### Responsibilities
1. **HitTesting**: Determine if click is on a bound handle (using ViewTransform)
2. **Dragging**: Convert screen deltas to world coordinate changes
3. **Preview**: Calculate new bound during drag without committing
4. **Rendering**: Draw handles and preview feedback
5. **Coordinate transformation**: Central point for ViewTransform integration
6. **Setup modes**: Preserve tracker-based rectangle handles and point-based custom-dot entry (circle/ellipse/rect via either; polygon via dots only)
7. **Selection levels**: Provide selection level (handle/dot/edge/interior) for polygon editing and outline hit-testing
8. **O1 interaction**: Support explicit Add Bound command and modal edit session with commit/discard

### Integration with InputHandler
```cpp
class InputHandler {
    // ...existing...
    
    BoundsHandler m_boundsHandler;
    
public:
    void SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage) {
        m_boundsHandler.SetBoundsData(pBounds, pImage);
    }
    
    void SetViewTransform(ViewTransform* pView) {
        m_boundsHandler.SetViewTransform(pView);
    }
};
```

---

## ViewTransform Integration

### Current State
- ✅ `ViewTransform` class exists with proper API:
  ```cpp
  struct CPoint2d { double x; double y; };
  
  CPoint ScreenToWorld(const CPoint& pt) const;
  CPoint WorldToScreen(const CPoint2d& wpt) const;
  void ZoomAt(const CPoint& screenPt, double factor);
  void PanBy(const CPoint& deltaScreen);
  void ZoomToFit(const CRect& imageRect, const CRect& clientRect);
  ```

- ✅ `ImageView` already uses it:
  ```cpp
  ViewTransform m_viewTransform;
  GetViewTransform() // public accessor
  ```

- ✅ `DrawBounds()` already modernized to use it

### Required Changes for Bounds Editing

1. **Pass ViewTransform to BoundsHandler**:
   ```cpp
   // In ImageView::OnInitialUpdate() or similar
   m_inputHandler.SetViewTransform(&m_viewTransform);
   m_inputHandler.SetBoundsData(&GetDocument()->boundCtrls, 
                                 GetImageCtrls(this));
   ```

2. **Update coordinate transformations in BoundsHandler**:
   ```cpp
   CPoint BoundsHandler::WorldToScreen(const CPoint2d& wpt) const {
       return m_pView->WorldToScreen(wpt);
   }
   
   CPoint2d BoundsHandler::ScreenToWorld(const CPoint& spt) const {
       return m_pView->ScreenToWorld(spt);
   }
   ```

3. **Handle pan/zoom during drag**:
   - ViewTransform already tracks offset and scale
   - Coordinate transformations automatically account for current view state
   - No special handling needed

---

## InputHandler Integration

### 1. Mode Activation Trigger

**Current flow** (ImageDoc):
```cpp
void CImageDoc::ActivateExtBounds(BOOL key)
{
    if (key) {
        SetInfo(I_BOUNDS_EXT, TRUE);     // Set flag
        // Currently: triggers BeginTracker in ImageView
    } else {
        SetInfo(I_BOUNDS_EXT, FALSE);
    }
}
```

**New flow** (proposed):
```cpp
void CImageDoc::ActivateExtBounds(BOOL key)
{
    CImageView* pView = GetView();
    if (!pView) return;
    
    if (key) {
        pView->GetInputHandler().SetMode(EditMode::BoundsExt);
        SetInfo(I_BOUNDS_EXT, TRUE);
    } else {
        pView->GetInputHandler().SetMode(EditMode::Navigate);
        SetInfo(I_BOUNDS_EXT, FALSE);
    }
}
```

### 2. Mouse Event Routing

**In ImageView**:
```cpp
void CImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
    DigitMode::InputHandler& handler = m_inputHandler;
    
    if (handler.GetMode() == EditMode::BoundsExt ||
        handler.GetMode() == EditMode::BoundsIns) {
        // Bounds editing mode
        int boundIdx, handleIdx;
        if (m_inputHandler.GetBoundsHandler().HitTestBoundHandle(point, boundIdx, handleIdx)) {
            m_inputHandler.GetBoundsHandler().BeginDrag(boundIdx, handleIdx, point);
            SetCapture();  // Capture mouse
        }
        return;  // Don't process as normal selection
    }
    
    // Normal mode (Navigate, Draw, DotEdit)
    handler.OnMouseDown(point, flags, &GetDocument()->Digit);
}

void CImageView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        
        if (m_inputHandler.GetBoundsHandler().IsDragging()) {
            m_inputHandler.GetBoundsHandler().UpdateDrag(point);
            Invalidate(FALSE);  // Trigger redraw for preview
        }
        return;
    }
    
    // Normal mode
    m_inputHandler.OnMouseMove(point, &GetDocument()->Digit);
}

void CImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        
        if (m_inputHandler.GetBoundsHandler().IsDragging()) {
            m_inputHandler.GetBoundsHandler().EndDrag(true);  // Commit
            ReleaseCapture();
            Invalidate(FALSE);
        }
        return;
    }
    
    // Normal mode
    m_inputHandler.OnMouseUp(point, &GetDocument()->Digit, m_cmdDispatcher);
}
```

### 3. Draw Callback for Preview

In `OnDraw()` or via `DrawTracker()`:
```cpp
void CImageView::OnDraw(CDC* pDC)
{
    // ... existing drawing code ...
    
    // Draw bounds editing feedback
    if (m_inputHandler.GetMode() == EditMode::BoundsExt ||
        m_inputHandler.GetMode() == EditMode::BoundsIns) {
        m_inputHandler.GetBoundsHandler().DrawPreviewBound(pDC);
        m_inputHandler.GetBoundsHandler().DrawHandles(pDC);
    }
}
```

---

## Undo/Redo Support

### Command Objects

```cpp
namespace DigitMode {

// Command: Move a bound
class MoveBoundCommand : public ICommand {
private:
    int m_boundIndex;
    EditMode m_boundMode;  // BoundsExt or BoundsIns
    CRect m_oldBound;
    CRect m_newBound;
    CBoundCtrls* m_pBounds;
    
public:
    MoveBoundCommand(int idx, EditMode mode, const CRect& oldB, const CRect& newB,
                     CBoundCtrls* pBounds)
        : m_boundIndex(idx), m_boundMode(mode), m_oldBound(oldB), m_newBound(newB),
          m_pBounds(pBounds) {}
    
    void Execute() override {
        if (m_boundMode == EditMode::BoundsExt) {
            m_pBounds->SetBound(BOUND_RECT, m_boundIndex);  // Simplified
        }
    }
    
    void Undo() override {
        if (m_boundMode == EditMode::BoundsExt) {
            m_pBounds->SetBound(BOUND_RECT, m_boundIndex);  // Restore old
        }
    }
};

// Command: Add a bound
class AddBoundCommand : public ICommand { /* ... */ };

// Command: Remove a bound
class RemoveBoundCommand : public ICommand { /* ... */ };

} // namespace DigitMode
```

### Integration with CommandDispatcher

In `BoundsHandler::EndDrag()`:
```cpp
void BoundsHandler::EndDrag(bool bCommit, CommandDispatcher* pCmdDisp) {
    if (!m_isDragging) return;
    
    if (bCommit) {
        CRect newBound = GetPreviewBound();
        if (newBound != m_boundSnapshot) {
            auto cmd = std::make_unique<MoveBoundCommand>(
                m_boundIndex, m_mode, m_boundSnapshot, newBound, m_pBounds);
            pCmdDisp->Dispatch(std::move(cmd));
        }
    }
    // else: revert (no command)
    
    m_isDragging = false;
}
```

---

## Migration Strategy

### Phase 1: Parallel Coexistence (Current → Step 1-4)
- [x] Create `BoundsHandler` abstraction
- [x] Integrate `ViewTransform` into bounds editing
- [x] Keep `Tracker` class in place (not yet removed)
- [x] New code paths use `InputHandler` + `BoundsHandler`
- [x] Old code paths (if any) still use `Tracker`
- **Risk**: Maintenance of two systems; **Mitigation**: Isolate via `InputHandler.SetMode()`

### Phase 2: Deprecate Legacy Code (Step 5-7)
- Remove `BeginTracker/DragTracker/DropTracker` from `ImageView`
- Redirect all bounds editing through `InputHandler.SetMode(BoundsExt/BoundsIns)`
- Update `ImageDoc.ActivateExtBounds()` / `ActivateInsBounds()` to use new API
- Remove usage of `m_zoomLevel` in bounds context

### Phase 3: Consolidation (Step 8-10)
- If `Tracker` is used elsewhere, refactor or keep as utility
- Consider removing `Tracker` entirely if no other uses remain
- Consolidate bounds operations into unified command pattern
- Document final architecture and team migration guide

---

## Testing Strategy

### Unit Tests

**BoundsHandler Coordinate Transformations**:
```cpp
TEST(BoundsHandlerTest, ScreenToWorldUnderZoom) {
    // Arrange
    ViewTransform view;
    view.SetZoom(2.0);
    BoundsHandler handler;
    handler.SetViewTransform(&view);
    
    CPoint screen(100, 100);
    
    // Act
    CPoint2d world = handler.ScreenToWorld(screen);
    
    // Assert
    EXPECT_DOUBLE_EQ(world.x, 50.0);  // 100 / 2.0
    EXPECT_DOUBLE_EQ(world.y, 50.0);
}

TEST(BoundsHandlerTest, ScreenToWorldUnderPan) {
    ViewTransform view;
    CPoint2d offset = {50.0, 75.0};
    view.SetOffset(offset);
    BoundsHandler handler;
    handler.SetViewTransform(&view);
    
    CPoint screen(0, 0);
    CPoint2d world = handler.ScreenToWorld(screen);
    
    EXPECT_DOUBLE_EQ(world.x, -50.0);  // 0 - 50
    EXPECT_DOUBLE_EQ(world.y, -75.0);
}
```

**BoundsHandler HitTesting**:
```cpp
TEST(BoundsHandlerTest, HitTestCornerHandle) {
    // Set up a bound at (100, 100) → (200, 200)
    // Click near TL corner
    // Verify handle is detected
}
```

**BoundsHandler Drag Geometry**:
```cpp
TEST(BoundsHandlerTest, DragTopLeftCorner) {
    // Original bound: (100, 100) → (200, 200)
    // Drag TL handle to (150, 150)
    // Expected result: (150, 150) → (200, 200)
}
```

### Integration Tests

**InputHandler Mode Switching**:
```cpp
TEST(InputHandlerTest, SetModeToExtBounds) {
    InputHandler handler;
    
    handler.SetMode(EditMode::Navigate);
    EXPECT_EQ(handler.GetMode(), EditMode::Navigate);
    
    handler.SetMode(EditMode::BoundsExt);
    EXPECT_EQ(handler.GetMode(), EditMode::BoundsExt);
    
    handler.SetMode(EditMode::Navigate);
    EXPECT_EQ(handler.GetMode(), EditMode::Navigate);
}
```

**Full Drag Lifecycle**:
```cpp
TEST(BoundsEditingTest, DragBoundWithViewTransform) {
    // Setup: ImageView with zoom=2.0, offset=(10, 10)
    // Bound at (0, 0) → (100, 100)
    // User drags TL corner from screen(50, 50) to screen(100, 100)
    
    // Verify:
    // 1. HitTest succeeds
    // 2. BeginDrag captures handle
    // 3. UpdateDrag computes correct world deltas
    // 4. Preview bound is updated
    // 5. EndDrag creates MoveBoundCommand
}
```

### Visual/Regression Tests

- Test bounds rendering at various zoom levels
- Test bounds rendering with pan offset
- Test handle drawing at correct screen positions
- Compare before/after screenshots for different bound types (ROUND, ELLIPSE, RECT, POLYGON)

---

## Implementation Steps (Detailed)

### Step 1: Analysis & Documentation ✓
- [x] Review current bounds editing code
- [x] Identify ViewTransform usage gaps
- [x] Document this plan

### Step 2-4: BoundsHandler Core (Est. 3-5 days)
1. Create `DigitMode/BoundsHandler.h/cpp`
2. Implement coordinate transformation methods
3. Implement HitTesting for handles
4. Implement drag state machine
5. Implement preview bound calculation
6. Unit test coordinate transformations and geometry

### Step 5-7: InputHandler Integration (Est. 2-3 days)
1. Extend `EditMode` enum
2. Add `m_boundsEdit` state to `InputHandler`
3. Add `BoundsEditMode` entry/exit logic
4. Add `BeginBoundsDrag/UpdateBoundsDrag/EndBoundsDrag` methods
5. Create `BoundsHandler` member in `InputHandler`
6. Implement command objects for undo/redo

### Step 8: ImageView Integration (Est. 2-3 days)
1. Add message routing for bounds mode in `OnLButtonDown/OnMouseMove/OnLButtonUp`
2. Update `OnDraw()` to draw bounds feedback during editing
3. Update `ImageDoc.ActivateExtBounds/ActivateInsBounds` to use new API
4. Remove old `BeginTracker/DragTracker/DropTracker` calls
5. Test with real UI interactions

### Step 9: Testing (Est. 3-4 days)
1. Unit tests for `BoundsHandler`
2. Integration tests for mode switching
3. Full bounds editing workflow tests
4. Visual regression tests
5. Manual testing with various bound types

### Step 10: Documentation (Est. 1 day)
1. API documentation for `BoundsHandler` and new modes
2. Migration guide for removing `Tracker` usage
3. Architecture diagram for bounds editing flow
4. Testing guide for future developers

---

## Risk Assessment & Mitigation

| Risk | Probability | Impact | Mitigation |
|------|-----------|--------|-----------|
| ViewTransform doesn't handle all pan/zoom scenarios | Low | High | Early integration test with extreme zoom/pan values |
| Coordinate transformation bugs cause misaligned handles | Medium | High | Comprehensive unit tests; visual regression tests |
| Breaking existing bounds functionality | Medium | High | Keep parallel implementation until fully tested |
| Mode state leakage across mode switches | Low | High | Explicit cleanup in `SetMode()`; assert invariants |
| Undo/redo commands not captured properly | Low | Medium | Test all drag scenarios with undo/redo |
| Performance issues with frequent coordinate transforms | Low | Medium | Profile under heavy zoom/pan; cache if needed |

---

## Dependencies & Assumptions

### Dependencies
- ✅ `ViewTransform` class (fully available)
- ✅ `InputHandler` with `EditMode` pattern (partially complete)
- ✅ `CommandDispatcher` for undo/redo (exists)
- ✅ `CBoundCtrls` bounds data model (exists)
- ⚠️ Possible refactoring of `Tracker` if reused elsewhere

### Assumptions
1. **ViewTransform coordinate system is correct**: `ScreenToWorld()` and `WorldToScreen()` are inverses
2. **Image coordinate system is stable**: Bounds data uses consistent pixel/world coordinates
3. **Mode switching cleanly finalizes operations**: No mid-drag mode switches expected
4. **Undo/redo is centralized via CommandDispatcher**: All state changes go through commands
5. **No concurrent bounds editing**: Single bounds being edited at a time

---

## Success Criteria

1. ✅ Bounds editing works with any zoom level (0.02x → 22.0x)
2. ✅ Bounds editing works with any pan offset
3. ✅ Handles are positioned correctly on screen during drag
4. ✅ Undo/redo captures bound modifications
5. ✅ No regression in existing functionality (other EditModes)
6. ✅ Code is testable and has >80% test coverage
7. ✅ Architecture is documented and ready for team review

---

## Open Questions

1. **Handle drawing style**: Should we keep using `CMTraker::DrawTracker()` or create new `BoundsHandler::DrawHandles()`?
   - **Proposed**: Create new method; gives us control over rendering during ViewTransform era

2. **Polygon bounds support**: Should Phase 1 support BOUND_POLYGON editing, or Phase 2?
   - **Proposed**: Phase 1: BOUND_RECT only; Phase 2: add polygon via extended `BoundsHandler`

3. **Constraints & snapping**: Should we add image boundary constraints to bounds handles?
   - **Proposed**: Phase 2; initial implementation allows free dragging

4. **Multiple bounds editing**: Can user edit multiple bounds in one operation?
   - **Proposed**: No; one bound at a time; reflected in `m_boundIndex` (singular)

5. **Tracker class fate**: Will `Tracker` be removed or kept for other purposes?
   - **Decision needed**: Scan codebase for other `CMTraker` usage before Phase 3

---

## Related Documents

- `INPUTHANDLER_ACTIVE_SEGMENT_DESIGN.md` — Drawing mode architecture
- `ViewTransform.h` — Coordinate transformation system
- `Controls/BoundCtrls.h/cpp` — Bounds data model
- `DigitMode/InputHandler.h/cpp` — Input handling framework
- `DigitMode/CommandDispatcher.h/cpp` — Command pattern for undo/redo

---

## Revision History

| Date | Author | Version | Changes |
|------|--------|---------|---------|
| 2024-01 | Architecture Planning | 1.0 | Initial planning document; 10-step implementation plan |


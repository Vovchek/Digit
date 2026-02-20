# Bounds Editing - Gap Analysis & Implementation Plan

## Executive Summary

Current implementation has **good foundation** but is **missing critical workflow functionality**. The architecture (Commands, DraftShape, BoundsHandler) is solid, but user-facing interactions are incomplete.

---

## ✅ What's Already Implemented (Solid Foundation)

### Architecture ✓
- [x] IInputHandler interface with clean routing
- [x] BoundsHandler (state machine for editing)
- [x] BoundsInputHandler (IInputHandler wrapper)
- [x] InputRouter integration
- [x] Command pattern (AddShapeCommand, RemoveShapeCommand, ReplaceShapeCommand)
- [x] DraftShape system with ToShape() and GetPreview()
- [x] ShapeDrawDispatcher rendering system (Phase 4 complete)
- [x] ViewTransform coordinate conversion

### Shape Creation Infrastructure ✓
- [x] DraftShape with Kind (Rectangle, Ellipse, Circle, Polygon)
- [x] AddDraftPoint() method
- [x] CommitDraft() method
- [x] CanCommit() validation
- [x] LSM fitting for Ellipse (FitEllipse, FitCircle)
- [x] Rectangle 3-point constructor
- [x] Polygon vertex construction

### Handle Editing Infrastructure ✓
- [x] EnumerateHandles() in Shape classes
- [x] ApplyHandleDrag() in Shape classes
- [x] BeginDrag/UpdateDrag/EndDrag cycle
- [x] HitTest with handle detection
- [x] ReplaceShapeCommand for edits

---

## ❌ Critical Gaps (Missing Workflow Pieces)

### Gap 1: Drag-Based Bounding Box Creation (§2.0 Spec) ❌

**Requirement**: Rectangle/Ellipse/Circle should support **drag-creation**:
1. Click-and-hold → anchor point
2. Drag → grow bounding box
3. Release → commit shape immediately

**Current State**: 
- ❌ No drag detection logic
- ❌ No bounding box tracking
- ❌ No mouse-up auto-commit in Add modes
- ✓ Point-sequence creation works (but no drag shortcut)

**Impact**: **HIGH** - Primary creation workflow missing

---

### Gap 2: Draft Preview During Mouse Move ❌

**Requirement**: Show live preview as user drags or moves mouse

**Current State**:
- ❌ OnMouseMove doesn't update draft preview
- ❌ No visual feedback while dragging bounding box
- ❌ No preview edge following cursor (polygon mode)
- ✓ GetPreview() exists but not used in real-time

**Impact**: **HIGH** - No visual feedback = poor UX

---

### Gap 3: Delete Mode Implementation ❌

**Requirement**: Click shape → remove it (with RemoveShapeCommand)

**Current State**:
- ❌ Delete mode exists in enum but not implemented
- ❌ No shape removal logic in BoundsInputHandler
- ✓ RemoveShapeCommand exists and tested

**Impact**: **MEDIUM** - Missing core editing mode

---

### Gap 4: Shape Selection & Body Drag ❌

**Requirement**: Click shape body → select, drag body → move entire shape

**Current State**:
- ❌ No selection state tracking
- ❌ Body drag detected but not handled
- ❌ No MoveShapeCommand (needs creation)
- ✓ HitTest detects body clicks

**Impact**: **MEDIUM** - Can't move shapes after creation

---

### Gap 5: Visual Integration (OnDraw) ⚠️

**Requirement**: Draw draft preview, selection highlights, handles

**Current State**:
- ⚠️ RenderPreview() exists in BoundsHandler
- ❌ Not called from CImageView::OnDraw()
- ❌ No integration with ShapeDrawDispatcher
- ✓ Rendering infrastructure complete (Phase 4)

**Impact**: **HIGH** - Users can't see what they're doing

---

### Gap 6: Modifier Keys (Shift/Alt) ⚠️

**Requirement**: 
- Shift: constrain aspect ratio, snap angles, force circle
- Alt: resize from center

**Current State**:
- ⚠️ DragContext has shiftKey/altKey fields
- ❌ Not read from GetKeyState() in UpdateDrag()
- ❌ No constraint logic applied
- ✓ Infrastructure exists

**Impact**: **LOW** - Nice-to-have, but spec requires it

---

### Gap 7: Auto-Commit on Mouse Up (Drag Creation) ❌

**Requirement**: After drag-creation, auto-commit on mouse-up

**Current State**:
- ❌ OnMouseUp doesn't commit draft shapes
- ❌ No distinction between drag-creation vs point-sequence
- ✓ CommitDraft() works manually

**Impact**: **HIGH** - Drag-creation incomplete without this

---

## 📋 Implementation Plan (Priority Order)

### Phase A: Visual Feedback (Immediate - 2 hours)

**Goal**: Make draft shapes visible to users

#### A1. Integrate RenderPreview into CImageView::OnDraw
```cpp
// In CImageView::OnDraw(), after drawing committed shapes:
if (m_boundsHandler.IsDrafting()) {
    const auto* draftPreview = m_boundsHandler.GetDraftPreview();
    if (draftPreview) {
        ShapeDrawStyle style;
        style.state = ShapeDrawStyle::State::Draft;
        style.type = m_boundsHandler.GetShapeType();
        m_shapeDrawDispatcher.Draw(*draftPreview, dc, style, m_viewTransform);
    }
}

if (m_boundsHandler.IsDragging()) {
    const auto* previewShape = m_boundsHandler.GetPreviewShape();
    if (previewShape) {
        ShapeDrawStyle style;
        style.state = ShapeDrawStyle::State::Selected;
        style.showHandles = true;
        m_shapeDrawDispatcher.Draw(*previewShape, dc, style, m_viewTransform);
    }
}
```

**Files**:
- `ImageTempl/ImageView.cpp` - Modify OnDraw()

**Test**: Start Add mode, click points, verify preview appears

---

### Phase B: Drag-Based Creation (Core - 4 hours)

**Goal**: Implement drag-to-create workflow for Rectangle/Ellipse/Circle

#### B1. Add Drag State to BoundsHandler
```cpp
// In BoundsHandler.h:
private:
    bool m_isDraftDragging = false;
    CPoint m_dragAnchor;      // First click point (screen coords)
    CPoint m_dragCurrent;     // Current mouse position
```

#### B2. Detect Drag vs Click in OnMouseDown
```cpp
// In BoundsInputHandler::HandleAddModeMouseDown():
if (flags & MK_LBUTTON) {
    // If no draft points yet, prepare for potential drag
    if (!m_boundsHandler.HasDraftPoints()) {
        m_boundsHandler.BeginDraftDrag(pt);
        // Don't add point yet - wait for drag or click-release
        return true;
    } else {
        // Already have points - this is point-sequence mode
        aperture::Point worldPt = m_boundsHandler.ScreenToAperturePoint(pt);
        m_boundsHandler.AddDraftPoint(worldPt);
        return true;
    }
}
```

#### B3. Update Draft During Mouse Move
```cpp
// In BoundsInputHandler::OnMouseMove():
if (m_boundsHandler.IsDraftDragging()) {
    m_boundsHandler.UpdateDraftDrag(pt);
    return true;  // Consumed - invalidate view
}
```

#### B4. Commit or Add Point on Mouse Up
```cpp
// In BoundsInputHandler::OnMouseUp():
if (m_boundsHandler.IsDraftDragging()) {
    CPoint anchor = m_boundsHandler.GetDragAnchor();
    int dx = abs(pt.x - anchor.x);
    int dy = abs(pt.y - anchor.y);
    
    if (dx < 3 && dy < 3) {
        // Click (not drag) - add as point-sequence
        aperture::Point worldPt = m_boundsHandler.ScreenToAperturePoint(pt);
        m_boundsHandler.AddDraftPoint(worldPt);
        m_boundsHandler.EndDraftDrag();
    } else {
        // Drag detected - auto-commit bounding box
        m_boundsHandler.CommitDraftDrag();
    }
    return true;
}
```

**Files**:
- `DigitMode/BoundsHandler.h` - Add drag state members
- `DigitMode/BoundsHandler.cpp` - Implement BeginDraftDrag, UpdateDraftDrag, CommitDraftDrag
- `DigitMode/BoundsInputHandler.cpp` - Update OnMouseDown, OnMouseMove, OnMouseUp
- `DigitMode/DraftShape.h` - Add FromBoundingBox() static method
- `DigitMode/DraftShape.cpp` - Implement box→shape conversion

**Test**:
1. Start Add Rectangle mode
2. Click-drag → shape appears, release → committed
3. Click once → point added for sequence mode

---

### Phase C: Delete Mode (Simple - 1 hour)

**Goal**: Click shape → remove it

#### C1. Implement Delete Handling
```cpp
// In BoundsInputHandler::OnMouseDown():
if (mode == ShapeEditMode::Delete) {
    auto hit = m_boundsHandler.HitTest(pt);
    if (hit.hit) {
        // Create RemoveShapeCommand
        auto cmd = std::make_unique<RemoveShapeCommand>(
            *m_apertureCtrls,
            hit.type,
            hit.shapeIndex
        );
        m_dispatcher->Execute(std::move(cmd));
        return true;
    }
}
```

**Files**:
- `DigitMode/BoundsInputHandler.cpp` - Add delete mode case

**Test**: Add shape, switch to Delete mode, click shape, verify removal + undo

---

### Phase D: Shape Selection & Move (Medium - 3 hours)

**Goal**: Select shape, drag body to move

#### D1. Add Selection State
```cpp
// In BoundsHandler.h:
struct SelectedShape {
    aperture::TypeLimits type;
    size_t index;
    bool valid() const { return index != size_t(-1); }
};

private:
    SelectedShape m_selectedShape = { aperture::TypeLimits::EXTERNAL, size_t(-1) };
```

#### D2. Implement Body Drag → Move
```cpp
// In HandleSelectModeMouseDown():
if (hit.hit && hit.isBody()) {
    m_selectedShape = { hit.type, hit.shapeIndex };
    m_boundsHandler.BeginBodyDrag(hit.type, hit.shapeIndex, pt);
    return true;
}
```

#### D3. Create MoveShapeCommand
```cpp
// New file: DigitMode/Commands/MoveShapeCommand.h/cpp
class MoveShapeCommand : public ICommand {
    // Stores before/after center positions
    // On Execute: translates shape by delta
    // On Undo: restores original position
};
```

**Files**:
- `DigitMode/BoundsHandler.h` - Add selection state
- `DigitMode/BoundsHandler.cpp` - Implement BeginBodyDrag, UpdateBodyDrag
- `DigitMode/Commands/MoveShapeCommand.h` (new)
- `DigitMode/Commands/MoveShapeCommand.cpp` (new)

**Test**: Click shape body → drag → shape moves, undo works

---

### Phase E: Modifier Keys (Polish - 2 hours)

**Goal**: Shift/Alt constraints work

#### E1. Read Modifier State in UpdateDrag
```cpp
// In BoundsHandler::UpdateDrag():
dragContext.shiftKey = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
dragContext.altKey = (GetKeyState(VK_MENU) & 0x8000) != 0;

// In UpdateDraftDrag() for bounding box:
if (dragContext.shiftKey) {
    // Force square/circle
    int size = std::min(width, height);
    width = height = size;
}

if (dragContext.altKey) {
    // Resize from center
    // Adjust anchor point to keep center fixed
}
```

**Files**:
- `DigitMode/BoundsHandler.cpp` - Add modifier logic

**Test**: Drag with Shift → square/circle, drag with Alt → center-based

---

### Phase F: Polish & Testing (Final - 3 hours)

#### F1. Add Missing Features
- [ ] Polygon: preview edge following cursor
- [ ] Right-click to close polygon (spec §2.4)
- [ ] Self-intersection validation (polygon)
- [ ] Status bar messages during modes

#### F2. End-to-End Tests
```cpp
TEST(BoundsWorkflow, DragCreateRectangle) {
    // Simulate drag-creation workflow
    SimulateMouseDown(100, 100);
    SimulateMouseMove(200, 150);
    SimulateMouseUp(200, 150);
    
    ASSERT_EQ(1, shapes.getExternal().size());
    // Verify rectangle dimensions
}

TEST(BoundsWorkflow, PointSequenceEllipse) {
    // Simulate point-sequence workflow
    SimulateClick(100, 100);
    SimulateClick(150, 120);
    SimulateClick(180, 110);
    SimulateKeyPress(VK_RETURN);
    
    ASSERT_EQ(1, shapes.getExternal().size());
}

TEST(BoundsWorkflow, DeleteShape) {
    // Create, switch to delete, remove
    CreateTestShape();
    SetEditMode(ShapeEditMode::Delete);
    SimulateClick(shapeCenter);
    
    ASSERT_EQ(0, shapes.getExternal().size());
}
```

**Files**:
- `Tests/DigitModeTests/BoundsWorkflowTest.cpp` (new)

---

## 🎯 Implementation Priority Matrix

| Feature | Priority | Effort | Impact | Status |
|---------|----------|--------|--------|--------|
| **Visual Integration (OnDraw)** | P0 | 2h | HIGH | ❌ |
| **Drag-Based Creation** | P0 | 4h | HIGH | ❌ |
| **Draft Preview on MouseMove** | P0 | 1h | HIGH | ❌ |
| **Delete Mode** | P1 | 1h | MED | ❌ |
| **Shape Selection** | P1 | 2h | MED | ❌ |
| **Body Drag (Move)** | P1 | 1h | MED | ❌ |
| **Modifier Keys** | P2 | 2h | LOW | ❌ |
| **Polygon Edge Preview** | P2 | 1h | LOW | ❌ |

**Total Estimated Effort**: 14 hours (~2 days)

---

## 🚧 Known Issues to Address

### Issue 1: No Drag Detection Threshold
**Problem**: Currently treats all mouse-down as point addition
**Solution**: Add DRAG_THRESHOLD (3 pixels) to distinguish click vs drag

### Issue 2: No Visual Feedback
**Problem**: Draft shapes not visible during creation
**Solution**: Call RenderPreview() from CImageView::OnDraw()

### Issue 3: OnMouseUp Ignored in Add Modes
**Problem**: Drag-creation can't auto-commit
**Solution**: Handle OnMouseUp to detect drag-end

### Issue 4: No Selection State
**Problem**: Can't track which shape is selected
**Solution**: Add m_selectedShape to BoundsHandler

### Issue 5: No MoveShapeCommand
**Problem**: Body drag has no undo support
**Solution**: Create new command class

---

## 📝 Implementation Checklist

### Phase A: Visual Feedback ⏳
- [ ] Add RenderPreview() call in CImageView::OnDraw()
- [ ] Test draft shapes appear during creation
- [ ] Verify dashed outline for draft state

### Phase B: Drag Creation ⏳
- [ ] Add m_isDraftDragging state to BoundsHandler
- [ ] Implement BeginDraftDrag()
- [ ] Implement UpdateDraftDrag()
- [ ] Implement CommitDraftDrag()
- [ ] Add DraftShape::FromBoundingBox()
- [ ] Update OnMouseDown to detect no-points case
- [ ] Update OnMouseMove to update drag preview
- [ ] Update OnMouseUp to commit or add point
- [ ] Test Rectangle drag-creation
- [ ] Test Ellipse drag-creation
- [ ] Test Circle drag-creation

### Phase C: Delete Mode ⏳
- [ ] Add delete case in HandleSelectModeMouseDown()
- [ ] Test shape removal
- [ ] Test undo/redo

### Phase D: Selection & Move ⏳
- [ ] Add SelectedShape struct
- [ ] Implement selection on body click
- [ ] Create MoveShapeCommand
- [ ] Implement BeginBodyDrag()
- [ ] Implement UpdateBodyDrag()
- [ ] Test shape movement
- [ ] Test undo/redo

### Phase E: Modifiers ⏳
- [ ] Read Shift/Alt in UpdateDrag()
- [ ] Apply square/circle constraint
- [ ] Apply center-resize constraint
- [ ] Test modifier behavior

### Phase F: Polish ⏳
- [ ] Polygon edge preview
- [ ] Self-intersection check
- [ ] Status bar integration
- [ ] End-to-end workflow tests
- [ ] Documentation update

---

## 🎓 Key Architectural Decisions

### 1. Drag vs Click Detection
**Decision**: Use 3-pixel threshold on mouse-up
**Rationale**: Distinguishes intentional drag from jitter

### 2. Auto-Commit on Drag
**Decision**: Drag-creation commits immediately on mouse-up
**Rationale**: Spec §2.0 requires this, matches Photoshop/CAD UX

### 3. Point-Sequence Fallback
**Decision**: After first point, clicks add points (no more drag)
**Rationale**: Allows mixed workflow (start drag, continue clicking)

### 4. Visual Feedback Priority
**Decision**: Implement OnDraw integration first
**Rationale**: Without visuals, nothing else is testable

### 5. Selection State in BoundsHandler
**Decision**: Store selected shape in handler, not view
**Rationale**: Handler owns editing logic, view just renders

---

## 🔧 Code Patterns to Follow

### Pattern 1: State Machine Transitions
```cpp
// Always check current state before transitioning
if (m_isDragging || m_isDraftDragging) {
    Cancel();  // Clean up before new operation
}
```

### Pattern 2: Coordinate Conversion
```cpp
// Always convert screen → world for geometry
aperture::Point worldPt = ScreenToAperturePoint(screenPt);
```

### Pattern 3: Command Dispatch
```cpp
// Always use Commands for mutations
auto cmd = std::make_unique<AddShapeCommand>(...);
m_dispatcher->Execute(std::move(cmd));
```

### Pattern 4: Preview Updates
```cpp
// Always update preview on state change
m_draftPreview = m_draft->GetPreview();
// Caller invalidates view
```

---

## 📊 Success Criteria

### Minimum Viable (MVP)
- [x] Architecture complete
- [ ] Drag-creation works (Rectangle, Ellipse, Circle)
- [ ] Point-sequence works (all shapes)
- [ ] Visual feedback during creation
- [ ] Delete mode functional
- [ ] Undo/redo works for all operations

### Full Specification Compliance
- [ ] All §2.x creation modes working
- [ ] All §4.x handle editing modes working
- [ ] Modifier keys (Shift/Alt) functional
- [ ] Selection and move working
- [ ] Visual appearance matches §3.x spec
- [ ] Drawing order correct (§5)

### Production Ready
- [ ] End-to-end tests passing
- [ ] No regressions in Phase 1-4
- [ ] Documentation updated
- [ ] Performance acceptable (<16ms frame time)

---

## 🚀 Next Actions

1. **Immediate (Today)**:
   - Implement Phase A (Visual Integration) - 2 hours
   - Test draft preview appears
   - Verify no regressions

2. **Short Term (Tomorrow)**:
   - Implement Phase B (Drag Creation) - 4 hours
   - Test all three drag-creation shapes
   - Verify point-sequence still works

3. **Medium Term (This Week)**:
   - Implement Phase C (Delete) - 1 hour
   - Implement Phase D (Selection & Move) - 3 hours
   - Test complete editing workflow

4. **Long Term (Next Week)**:
   - Implement Phase E (Modifiers) - 2 hours
   - Implement Phase F (Polish & Tests) - 3 hours
   - Final integration testing

---

**Status**: Gap analysis complete ✅  
**Estimated Total Work**: 14 hours (2 days)  
**Priority**: P0 - Blocking user workflows  
**Owner**: Implementation team  
**Review Date**: After Phase A completion

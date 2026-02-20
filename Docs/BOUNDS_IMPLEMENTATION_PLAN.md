# Bounds Editing - Implementation Status & Action Plan

## 🎯 Executive Summary

**Current State**: ✅ Excellent architecture, ❌ Missing user-facing workflows  
**Assessment**: 60% complete (infrastructure done, interactions missing)  
**Estimated Work**: 14 hours (2 days) to MVP  
**Priority**: P0 - Blocking user functionality

---

## ✅ What Works Today

### Architecture (100% Complete)
- ✅ IInputHandler interface + routing
- ✅ Command pattern (Add/Remove/Replace)
- ✅ DraftShape system with LSM fitting
- ✅ Shape rendering (Phase 4 complete)
- ✅ Handle enumeration + ApplyHandleDrag
- ✅ Coordinate transformations (ViewTransform)

### Partial Functionality (50% Complete)
- ⚠️ Point-sequence creation (works but no preview)
- ⚠️ Handle dragging (backend works, no UI integration)
- ⚠️ Hit-testing (works, but selection not tracked)

---

## ❌ Critical Gaps (What's Missing)

### Gap 1: Visual Feedback ❌ **P0**
**Issue**: Draft shapes not visible during creation  
**Impact**: Users can't see what they're doing  
**Effort**: 2 hours  
**Fix**: Call RenderPreview() from CImageView::OnDraw()

### Gap 2: Drag-Based Creation ❌ **P0**
**Issue**: No click-drag-release workflow  
**Impact**: Primary UX pattern missing (§2.0 spec)  
**Effort**: 4 hours  
**Fix**: Add drag detection + bounding box tracking

### Gap 3: Delete Mode ❌ **P1**
**Issue**: Can create shapes but not delete  
**Impact**: No way to remove mistakes  
**Effort**: 1 hour  
**Fix**: Add delete case in OnMouseDown

### Gap 4: Shape Movement ❌ **P1**
**Issue**: Can't move shapes after creation  
**Impact**: Limited editing capability  
**Effort**: 3 hours  
**Fix**: Implement body drag + MoveShapeCommand

### Gap 5: Modifier Keys ❌ **P2**
**Issue**: Shift/Alt constraints don't work  
**Impact**: Missing UX polish (spec requires it)  
**Effort**: 2 hours  
**Fix**: Read GetKeyState() in UpdateDrag()

---

## 📋 6-Phase Implementation Plan

### **Phase A: Visual Feedback** (P0 - 2 hours)
**Goal**: Make draft shapes visible

```cpp
// In CImageView::OnDraw():
if (m_boundsHandler.IsDrafting()) {
    const auto* preview = m_boundsHandler.GetDraftPreview();
    if (preview) {
        ShapeDrawStyle style;
        style.state = ShapeDrawStyle::State::Draft;
        m_shapeDrawDispatcher.Draw(*preview, dc, style, m_viewTransform);
    }
}
```

**Files**: ImageView.cpp  
**Test**: Click points → preview appears  
**Deliverable**: Users can see draft shapes

---

### **Phase B: Drag Creation** (P0 - 4 hours)
**Goal**: Click-drag-release creates shape

```cpp
// In BoundsInputHandler:
OnMouseDown → BeginDraftDrag(pt)
OnMouseMove → UpdateDraftDrag(pt)  // updates preview
OnMouseUp → CommitDraftDrag()      // auto-commit if dragged
```

**Files**: BoundsHandler.h/cpp, BoundsInputHandler.cpp, DraftShape.cpp  
**Test**: Drag rectangle → shape created on release  
**Deliverable**: Primary creation workflow functional

---

### **Phase C: Delete Mode** (P1 - 1 hour)
**Goal**: Click shape → remove it

```cpp
// In BoundsInputHandler:
if (mode == Delete && hit.hit) {
    auto cmd = std::make_unique<RemoveShapeCommand>(...)  
    m_dispatcher->Execute(std::move(cmd));
}
```

**Files**: BoundsInputHandler.cpp  
**Test**: Delete shape + undo  
**Deliverable**: Delete mode functional

---

### **Phase D: Selection & Move** (P1 - 3 hours)
**Goal**: Drag shape body → translate

```cpp
// Add to BoundsHandler:
struct SelectedShape { TypeLimits type; size_t index; };
SelectedShape m_selectedShape;

// New command:
class MoveShapeCommand { /* translate shape by delta */ };
```

**Files**: BoundsHandler.h/cpp, MoveShapeCommand.h/cpp  
**Test**: Drag shape → moves, undo works  
**Deliverable**: Shape movement functional

---

### **Phase E: Modifiers** (P2 - 2 hours)
**Goal**: Shift/Alt constraints work

```cpp
// In UpdateDrag():
if (GetKeyState(VK_SHIFT) & 0x8000) {
    // Force square/circle
}
if (GetKeyState(VK_MENU) & 0x8000) {
    // Resize from center
}
```

**Files**: BoundsHandler.cpp  
**Test**: Drag with Shift → square  
**Deliverable**: Modifier keys functional

---

### **Phase F: Polish** (P2 - 3 hours)
**Goal**: Complete spec compliance

- [ ] Polygon edge preview (cursor tracking)
- [ ] Self-intersection validation
- [ ] Status bar messages
- [ ] End-to-end workflow tests

**Files**: BoundsHandler.cpp, BoundsWorkflowTest.cpp  
**Test**: Full spec compliance  
**Deliverable**: Production-ready implementation

---

## 🎯 Success Criteria

### Minimum Viable (After Phase A + B)
- [ ] Drag-creation works (Rectangle, Ellipse, Circle)
- [ ] Visual feedback during creation
- [ ] Point-sequence still works
- [ ] No regressions

### Full MVP (After Phase A-D)
- [ ] All creation modes functional
- [ ] Delete mode works
- [ ] Shape movement works
- [ ] Undo/redo complete

### Spec Compliant (After Phase A-F)
- [ ] All §2.x modes implemented
- [ ] All §3.x visual styles correct
- [ ] All §4.x handle editing functional
- [ ] End-to-end tests passing

---

## 🚀 Recommended Execution Order

### Day 1 (Morning - 4 hours)
1. **Phase A: Visual Feedback** (2 hours)
   - Integrate RenderPreview in OnDraw
   - Test draft visibility
   - **Milestone**: Users can see drafts ✓

2. **Phase B: Drag Creation** (Start - 2 hours)
   - Add drag state tracking
   - Implement BeginDraftDrag
   - **Checkpoint**: Drag detection working

### Day 1 (Afternoon - 4 hours)
3. **Phase B: Drag Creation** (Complete - 2 hours)
   - Implement UpdateDraftDrag
   - Implement CommitDraftDrag
   - **Milestone**: Drag-creation functional ✓

4. **Phase C: Delete Mode** (1 hour)
   - Add delete handler
   - Test removal + undo
   - **Milestone**: Delete works ✓

5. **Prep Phase D** (1 hour)
   - Design MoveShapeCommand
   - Add selection state

### Day 2 (Morning - 3 hours)
6. **Phase D: Selection & Move** (3 hours)
   - Implement body drag
   - Create MoveShapeCommand
   - Test movement + undo
   - **Milestone**: Move works ✓

### Day 2 (Afternoon - 3 hours)
7. **Phase E: Modifiers** (2 hours)
   - Add Shift/Alt handling
   - Test constraints
   - **Milestone**: Modifiers work ✓

8. **Phase F: Testing** (1 hour)
   - End-to-end workflow tests
   - Regression verification
   - **Milestone**: MVP complete ✓

---

## 📊 Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Drag detection threshold wrong | Medium | Low | Test with different screen DPIs |
| Preview invalidation too slow | Low | Medium | Profile OnDraw, optimize if needed |
| Mouse capture conflicts | Low | High | Test with other tools active |
| Undo/redo state corruption | Low | High | Comprehensive unit tests |
| Modifier keys OS-dependent | Low | Low | Test on Win7/10/11 |

---

## 🔧 Technical Debt & Future Work

### Defer to V2
- [ ] Advanced polygon editing (vertex insertion/deletion)
- [ ] Shape rotation handles
- [ ] Multi-shape selection (Ctrl+click)
- [ ] Group operations (move multiple shapes)
- [ ] Snap-to-grid / snap-to-shape
- [ ] Shape templates / presets

### Architecture Improvements
- [ ] Separate DragHandler from BoundsHandler
- [ ] Extract SelectionManager
- [ ] Add InputStateValidator (debug tool)

---

## 📈 Progress Tracking

### Week 1 Status
- [x] Phase 1: Architecture design (complete)
- [x] Phase 2: DraftShape system (complete)
- [x] Phase 3: Rendering system (complete)
- [x] Phase 4: Handle editing (complete)
- [ ] **Phase 5: User workflows** ← **Current**

### Phase 5 Sub-Tasks
- [ ] A: Visual feedback (P0) - **NEXT**
- [ ] B: Drag creation (P0)
- [ ] C: Delete mode (P1)
- [ ] D: Selection & move (P1)
- [ ] E: Modifiers (P2)
- [ ] F: Polish (P2)

---

## 📝 Code Review Checklist

Before merging each phase:

### Functionality
- [ ] Feature works as specified
- [ ] Edge cases handled (empty selection, invalid shapes)
- [ ] Undo/redo verified
- [ ] No console errors/warnings

### Code Quality
- [ ] Follows existing patterns (Command, Handler, etc.)
- [ ] No RTTI abuse (Shape::GetKind() only)
- [ ] Coordinate conversion consistent (screen→world)
- [ ] Comments explain why, not what

### Testing
- [ ] Unit tests added/updated
- [ ] Integration test covers workflow
- [ ] Manual testing complete
- [ ] No regressions in other modes

### Performance
- [ ] OnDraw < 16ms (60 FPS target)
- [ ] No unnecessary allocations in hot path
- [ ] Preview updates throttled if needed

---

## 🎓 Key Design Decisions

### 1. Drag vs Click Threshold: 3 pixels
**Rationale**: Standard Windows drag threshold  
**Source**: GetSystemMetrics(SM_CXDRAG)

### 2. Auto-commit on Drag Release
**Rationale**: Spec §2.0 requires immediate commit  
**UX Precedent**: Photoshop, AutoCAD, Illustrator

### 3. Visual Feedback Priority
**Rationale**: Can't test without seeing drafts  
**Impact**: Phase A must be first

### 4. Selection State in BoundsHandler
**Rationale**: Handler owns editing logic  
**Alternative**: Separate SelectionManager (V2)

### 5. MoveShapeCommand vs ReplaceShapeCommand
**Rationale**: Move is specialized (translate only)  
**Performance**: Faster than full shape replacement

---

## 📚 Reference Documentation

- **Gap Analysis**: `BOUNDS_GAP_ANALYSIS.md` (this file)
- **Specification**: `copilot_xxx.md` (UX requirements)
- **Architecture**: `copilot_yyy.md` (input routing)
- **Phase 4 Report**: `Phase4_CompletionReport.md` (rendering)
- **Quick Reference**: `BOUNDS_QUICK_REFERENCE.md` (SetShapeType pattern)

---

## ✅ Action Items

### For Today
1. [ ] Review gap analysis with team
2. [ ] Approve Phase A implementation plan
3. [ ] Start Phase A: Visual feedback (2 hours)
4. [ ] Test draft preview visibility

### For This Week
5. [ ] Complete Phase A + B (MVP functionality)
6. [ ] Complete Phase C + D (full editing)
7. [ ] Begin Phase E (polish)
8. [ ] Write end-to-end tests

### For Next Week
9. [ ] Complete Phase E + F
10. [ ] Final regression testing
11. [ ] Documentation update
12. [ ] Release bounds editing feature

---

**Summary**: Implementation is 60% complete. Foundation is excellent (architecture, commands, rendering all solid). Need 14 hours of focused work to complete user-facing workflows. Start with Phase A (visual feedback) today, aim for MVP by end of Day 1.

**Next Action**: Implement Phase A - integrate RenderPreview() in CImageView::OnDraw() (2 hours)

---

*Status: Gap Analysis Complete ✅*  
*Owner: Development Team*  
*Priority: P0*  
*Estimated Completion: 2 days*

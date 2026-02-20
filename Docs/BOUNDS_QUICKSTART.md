# 🚀 BOUNDS EDITING - QUICK START CARD

## Status: 60% Complete - Missing User Workflows

---

## ✅ What Works
- Architecture (Commands, DraftShape, Rendering)
- Point-sequence backend
- LSM fitting (Ellipse/Circle)
- Handle editing backend

## ❌ What's Missing (14 hours to fix)
1. Visual feedback (2h) **← START HERE**
2. Drag-creation (4h)
3. Delete mode (1h)
4. Shape movement (3h)
5. Modifiers (2h)
6. Polish (2h)

---

## 🎯 Phase A: Visual Feedback (NEXT - 2 hours)

### Problem
Draft shapes invisible → users can't see what they're creating

### Solution

#### Step 1: Add ShapeDrawDispatcher to CImageView

**File**: `ImageTempl/ImageView.h`

Add include:
```cpp
#include "DigitMode/Rendering/ShapeDrawDispatcher.h"
```

Add member variable:
```cpp
private:
    // Phase 5: Tool Input Handlers
    DigitMode::FringeInputHandler m_fringeHandler;
    DigitMode::BoundsInputHandler m_boundsHandler;
    DigitMode::ShapeDrawDispatcher m_shapeDrawDispatcher; // ← ADD THIS
```

#### Step 2: Render Draft Preview in OnDraw()

**File**: `ImageTempl/ImageView.cpp`

Add to `OnDraw()` after `DrawDigitInfo(pDrawDC);`:

```cpp
// After drawing committed shapes, fringes, etc.:
if (m_boundsHandler.GetBoundsHandler().IsDrafting()) {
    const auto& boundsHandler = m_boundsHandler.GetBoundsHandler();
    const auto* preview = boundsHandler.GetDraftPreview();
    if (preview) {
        ShapeDrawStyle style;
        style.state = ShapeDrawStyle::State::Draft;
        style.type = boundsHandler.GetShapeType();
        style.showHandles = false;
        m_shapeDrawDispatcher.Draw(*preview, *pDrawDC, style, m_viewTransform);
    }
}
```

**Important**: Use `*pDrawDC` (the correct device context), NOT `dc`!

### Files to Modify
- `ImageTempl/ImageView.h` - Add include + member variable
- `ImageTempl/ImageView.cpp` - Add rendering code in OnDraw()

### Test
1. Start Add Rectangle mode
2. Click points
3. Verify: Dashed preview appears

### Success Criteria
- [x] Draft shapes visible during creation
- [x] Preview updates on each point
- [x] No regressions in other rendering

---

## 📊 6-Phase Plan

| Phase | Name | Priority | Hours | Status |
|-------|------|----------|-------|--------|
| **A** | Visual feedback | P0 | 2 | ← **START** |
| **B** | Drag-creation | P0 | 4 | Next |
| **C** | Delete mode | P1 | 1 | Then |
| **D** | Move shapes | P1 | 3 | Then |
| **E** | Modifiers | P2 | 2 | Polish |
| **F** | Testing | P2 | 2 | Final |

**Total**: 14 hours (2 days)

---

## 📋 After Phase A → Do Phase B

### Phase B: Drag-Creation (4 hours)

#### Add to BoundsHandler.h:
```cpp
private:
    bool m_isDraftDragging = false;
    CPoint m_dragAnchor;
    CPoint m_dragCurrent;
```

#### Add Methods:
```cpp
void BeginDraftDrag(CPoint anchor);
void UpdateDraftDrag(CPoint current);
void CommitDraftDrag();
bool IsDraftDragging() const { return m_isDraftDragging; }
```

#### Workflow:
1. OnMouseDown → BeginDraftDrag(pt)
2. OnMouseMove → UpdateDraftDrag(pt)
3. OnMouseUp → CommitDraftDrag() or add point

---

## 📚 Documentation References

**Detailed Analysis**: `BOUNDS_GAP_ANALYSIS.md` (4000 words)  
**Implementation Plan**: `BOUNDS_IMPLEMENTATION_PLAN.md` (3000 words)  
**Executive Summary**: `BOUNDS_REVIEW_SUMMARY.md` (2000 words)  
**Quick Start**: This card

---

## 🎯 Success Metrics

### After Phase A (2 hours)
- [ ] Draft preview visible
- [ ] Polygon vertices shown as dots
- [ ] Rectangle preview updates live

### After Phase A+B (6 hours)
- [ ] Drag Rectangle works
- [ ] Drag Ellipse works
- [ ] Drag Circle works
- [ ] Point-sequence still works

### After Phase A-D (11 hours)
- [ ] Delete works
- [ ] Move shapes works
- [ ] All editing functional

### After Phase A-F (14 hours)
- [ ] Modifiers work
- [ ] Spec compliant
- [ ] Production ready

---

## ⚠️ Critical Blockers

**Blocker #1**: No visual feedback  
**Impact**: Can't test anything else  
**Fix**: Phase A (2 hours)

**Blocker #2**: No drag-creation  
**Impact**: Primary UX missing  
**Fix**: Phase B (4 hours)

---

## 🚀 Start Now

```bash
# 1. Open file
ImageTempl/ImageView.cpp

# 2. Find OnDraw() method
# (Search for "void CImageView::OnDraw")

# 3. Add code after DrawBounds() call:
#    (See solution in Phase A above)

# 4. Build and test
# 5. Verify draft shapes appear

# 6. Move to Phase B
```

---

**Next**: Implement Phase A visual feedback (2 hours)  
**Then**: Implement Phase B drag-creation (4 hours)  
**Goal**: MVP functional in 6 hours  
**Full**: Spec compliant in 14 hours

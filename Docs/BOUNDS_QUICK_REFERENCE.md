# Quick Reference: Bounds UI Mode Switching

## The Problem (Solved)

**Before**: No way to specify INTERNAL vs EXTERNAL vs APERTURE when adding shapes
- `SetEditMode(AddEllipse)` always defaulted to EXTERNAL

**After**: Added `SetShapeType()` method
- Call before `SetEditMode()`
- Draft uses the specified type

---

## The Solution: One Method, One Member Variable

### What Was Added to BoundsHandler

```cpp
// In BoundsHandler.h - NEW:
void SetShapeType(aperture::TypeLimits type) { m_shapeType = type; }
aperture::TypeLimits GetShapeType() const { return m_shapeType; }

// In BoundsHandler.h - NEW member:
private:
    aperture::TypeLimits m_shapeType = aperture::TypeLimits::EXTERNAL;

// In BoundsHandler.cpp - MODIFIED:
void SetEditMode(ShapeEditMode mode) {
    // ... cancel operations ...
    if (mode == AddRectangle || mode == AddEllipse || ...) {
        DraftShape draft;
        draft.type = m_shapeType;  // ← USE THIS INSTEAD OF HARDCODED EXTERNAL
        // ... rest of initialization ...
    }
}
```

---

## How to Use It: UI Command Handler Pattern

```cpp
// In CImageView::OnAddBoundInternalEllipse()
void CImageView::OnAddBoundInternalEllipse()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);  // Step 1
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);  // Step 2
    GetMainFrame()->SetStatusText("Click to define internal ellipse...");
    Invalidate(FALSE);
}
```

---

## When to Switch Modes: Decision Tree

```
User wants to:

┌─ ADD EXTERNAL SHAPE?
│  └─ SetShapeType(EXTERNAL) → SetEditMode(AddRectangle/Ellipse/Circle/Polygon)
│
├─ ADD INTERNAL SHAPE?
│  └─ SetShapeType(INTERNAL) → SetEditMode(AddRectangle/Ellipse/Circle/Polygon)
│
├─ ADD APERTURE SHAPE?
│  └─ SetShapeType(APERTURE) → SetEditMode(AddRectangle/Ellipse/Circle/Polygon)
│
├─ EDIT EXISTING SHAPES?
│  └─ SetEditMode(Select)
│
├─ DELETE SHAPES?
│  └─ SetEditMode(Delete)
│
└─ CANCEL CURRENT ACTION?
   └─ Press ESC (handled by OnKeyDown)
```

---

## Complete Call Sequence: User Clicks "Add Internal Ellipse"

```
1. USER ACTION
   Click menu: Edit → Bounds → Add Internal → Ellipse

2. MESSAGE ROUTING
   WM_COMMAND(ID_ADD_BOUND_INTERNAL_ELLIPSE)
   ↓
   CImageView::OnAddBoundInternalEllipse()

3. TOOL ACTIVATION
   ActivateBoundsTool()
   ↓
   GetInputRouter().SetActiveTool(&m_boundsHandler)

4. MODE & TYPE CONFIGURATION
   SetShapeType(aperture::TypeLimits::INTERNAL)
   ↓
   m_shapeType = INTERNAL
   
   SetEditMode(ShapeEditMode::AddEllipse)
   ↓
   Create DraftShape
   └─ draft.type = m_shapeType  ← NOW USES INTERNAL!
   └─ draft.kind = Ellipse
   └─ m_draft = draft

5. USER DRAWS
   Click point 1 (center)
   ↓
   OnMouseDown → BoundsInputHandler::OnMouseDown()
   └─ AddDraftPoint(worldPoint1)
   
   Click point 2 (edge)
   ↓
   OnMouseDown → BoundsInputHandler::OnMouseDown()
   └─ AddDraftPoint(worldPoint2)

6. USER COMMITS
   Press Enter or click again
   ↓
   CommitDraft()
   ├─ shape = m_draft->ToShape()
   ├─ AddShapeCommand created:
   │  ├─ type = INTERNAL
   │  ├─ shape = the ellipse
   │  └─ apertureCtrls = reference
   └─ Dispatcher.Execute(command)

7. RESULT
   ✓ New INTERNAL ellipse in CApertureCtrls.m_shapes.getInternal()
   ✓ Undo/redo available
   ✓ Can continue adding more shapes in same mode
   ✓ Can press ESC to cancel current draft
```

---

## The Three Shape Types Explained

| Type | Purpose | Visual | When to Use |
|------|---------|--------|------------|
| **EXTERNAL** | Bounds of the entire optical system | Rectangle, sometimes ellipse | Hard aperture edges, field limits |
| **INTERNAL** | Obstructions/obstacles inside the system | Circles, polygons | Spiders, secondary mirror shadows, vignetting elements |
| **APERTURE** | Defined optical aperture region | Circle, ellipse | Optical aperture for image formation |

---

## File Changes Made

| File | Change | Why |
|------|--------|-----|
| `DigitMode/BoundsHandler.h` | Added `SetShapeType()` method | Allow UI to specify shape category |
| `DigitMode/BoundsHandler.h` | Added `m_shapeType` member | Store current shape type |
| `DigitMode/BoundsHandler.cpp` | Modified `SetEditMode()` | Use `m_shapeType` instead of hardcoded EXTERNAL |

---

## UI Work Still Required (By User)

```
[ ] Add resource IDs (.rc file)
    ID_ADD_BOUND_EXTERNAL_RECT = 32801
    ID_ADD_BOUND_INTERNAL_ELLIPSE = 32811
    ... etc for all 12 shape combinations ...

[ ] Add handler declarations (ImageView.h)
    afx_msg void OnAddBoundInternalEllipse();
    afx_msg void OnUpdateAddBoundInternalEllipse(CCmdUI*);
    ... etc ...

[ ] Add message map entries (ImageView.cpp)
    ON_COMMAND(ID_ADD_BOUND_INTERNAL_ELLIPSE, OnAddBoundInternalEllipse)
    ON_UPDATE_COMMAND_UI(..., OnUpdateAddBoundInternalEllipse)

[ ] Implement handlers (ImageView.cpp)
    void CImageView::OnAddBoundInternalEllipse() {
        ActivateBoundsTool();
        m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
        m_boundsHandler.SetEditMode(ShapeEditMode::AddEllipse);
        Invalidate(FALSE);
    }

[ ] Create menu structure
    Edit → Bounds → Add Internal → Ellipse ... (and 11 more)

[ ] Add keyboard accelerators (optional)
    Alt+Shift+E = Internal Ellipse
    Ctrl+Shift+R = External Rectangle
    Ctrl+Alt+C = Aperture Circle
```

---

## Build Status

✅ **BoundsHandler changes compile successfully**
- All new methods added
- Implementation follows existing patterns
- Ready for UI integration

---

## Example Usage Patterns

### Pattern 1: Add External Rectangle
```cpp
void OnAddBoundExternalRect() {
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddRectangle);
    Invalidate(FALSE);
}
```

### Pattern 2: Add Internal Circle
```cpp
void OnAddBoundInternalCircle() {
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddCircle);
    Invalidate(FALSE);
}
```

### Pattern 3: Add Aperture Polygon
```cpp
void OnAddBoundAperturePolygon() {
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddPolygon);
    Invalidate(FALSE);
}
```

### Pattern 4: Switch to Edit/Select Mode
```cpp
void OnBoundModeSelect() {
    ActivateBoundsTool();
    m_boundsHandler.SetEditMode(ShapeEditMode::Select);
    // Note: type doesn't matter in Select mode
    Invalidate(FALSE);
}
```

---

## Key Takeaway

You only need THREE things to fully specify what shape will be created:

1. **Shape Geometry** - Rectangle, Ellipse, Circle, Polygon
   - Controlled by `SetEditMode(AddRectangle/Ellipse/Circle/Polygon)`

2. **Shape Type** - External, Internal, Aperture
   - Controlled by `SetShapeType(EXTERNAL/INTERNAL/APERTURE)`
   
3. **Active Tool** - Bounds handler
   - Controlled by `ActivateBoundsTool()`

Everything else follows from these three settings.

---

## Documentation References

For detailed information, see:
- `Docs/BOUNDS_UI_COMMAND_PATTERN.md` - Complete architecture explanation
- `Docs/BOUNDS_UI_IMPLEMENTATION_GUIDE.md` - Step-by-step implementation
- `Docs/PHASE5_INPUT_ARCHITECTURE_GUIDE.md` - Overall input routing system

---

*Status: Core architecture complete ✅ | UI implementation pending ⏳*

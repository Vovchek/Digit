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

// In BoundsHandler.h - NEW member (default to APERTURE for new UI):
private:
    aperture::TypeLimits m_shapeType = aperture::TypeLimits::APERTURE;

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

## How to Use It: UI Command Handler Pattern (With Type Toggle)

In the updated UI there is a single APERTURE/INTERNAL type toggle (`ID_BOUND_VISIBILITY`) plus four geometry commands (`ID_ADD_BOUND_*`). The handlers follow the same pattern: set type (from toggle) before `SetEditMode()`.

```cpp
// Type toggle (two-state toolbar button)
void CImageView::OnBoundVisisbility()
{
    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits current = handler.GetShapeType();
    aperture::TypeLimits next =
        (current == aperture::TypeLimits::INTERNAL)
            ? aperture::TypeLimits::APERTURE
            : aperture::TypeLimits::INTERNAL;

    handler.SetShapeType(next);
    if (handler.IsDrafting())
        handler.CancelDraft();
}

// Geometry command (uses current type from toggle)
void CImageView::OnAddBoundEllipse()
{
    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);                         // Step 1
    handler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse); // Step 2
    GetMainFrame()->SetStatusText("Click to define elliptical bound...");
    Invalidate(FALSE);
}
```

---

## When to Switch Modes: Decision Tree

```
User wants to:

┌─ ADD SHAPE (APERTURE / INTERNAL)?
│  ├─ Use ID_BOUND_VISIBILITY to choose type
│  │    - Unchecked  → APERTURE
│  │    - Checked    → INTERNAL
│  └─ Call geometry command → SetEditMode(AddRectangle/Ellipse/Circle/Polygon)
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
   - Ensure bounds type is INTERNAL (ID_BOUND_VISIBILITY pressed)
   - Click menu: Edit → Bounds → Add Shape → Ellipse

2. MESSAGE ROUTING
   WM_COMMAND(ID_ADD_BOUND_ELLIPSE)
   ↓
   CImageView::OnAddBoundInternalEllipse()

3. TOOL ACTIVATION
   ActivateBoundsTool()
   ↓
   GetInputRouter().SetActiveTool(&m_boundsHandler)

4. MODE & TYPE CONFIGURATION
   SetShapeType(aperture::TypeLimits::INTERNAL) // from toggle
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
    ID_BOUND_VISIBILITY   = 80000  // APERTURE / INTERNAL toggle
    ID_ADD_BOUND_CIRCLE   = 80001
    ID_ADD_BOUND_ELLIPSE  = 80002
    ID_ADD_BOUND_RECT     = 80003
    ID_ADD_BOUND_POLYGON  = 80004
    ID_BOUND_MODE_SELECT  = 80010
    ID_BOUND_MODE_DELETE  = 80011

[ ] Add handler declarations (ImageView.h)
    afx_msg void OnAddBoundCircle();
    afx_msg void OnAddBoundEllipse();
    afx_msg void OnAddBoundRect();
    afx_msg void OnAddBoundPolygon();
    afx_msg void OnBoundVisisbility();

[ ] Add message map entries (ImageView.cpp)
    ON_COMMAND(ID_ADD_BOUND_ELLIPSE, OnAddBoundEllipse)
    ON_COMMAND(ID_BOUND_VISIBILITY, OnBoundVisisbility)

[ ] Implement handlers (ImageView.cpp)
    - Use pattern: ActivateBoundsTool() → SetShapeType(current) → SetEditMode(AddXXX)

[ ] Create menu structure
    Edit → Bounds → Add Shape → (Circle/Rect/Ellipse/Polygon)

[ ] Add keyboard accelerators (optional)
    Ctrl+Shift+E = Add ellipse (current type)
    Ctrl+Shift+T = Toggle type APERTURE / INTERNAL
```

---

## Build Status

✅ **BoundsHandler changes compile successfully**
- All new methods added
- Implementation follows existing patterns
- Ready for UI integration

---

## Example Usage Patterns

### Pattern 1: Add Ellipse (current APERTURE/INTERNAL)
```cpp
void OnAddBoundEllipse() {
    ActivateBoundsTool();
    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits current = handler.GetShapeType();
    handler.SetShapeType(current);
    handler.SetEditMode(ShapeEditMode::AddEllipse);
    Invalidate(FALSE);
}
```

### Pattern 2: Toggle Type
```cpp
void OnBoundVisisbility() {
    ActivateBoundsTool();
    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits current = handler.GetShapeType();
    aperture::TypeLimits next = (current == aperture::TypeLimits::INTERNAL)
        ? aperture::TypeLimits::APERTURE
        : aperture::TypeLimits::INTERNAL;
    handler.SetShapeType(next);
}
```

### Pattern 3: Switch to Edit/Select Mode
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

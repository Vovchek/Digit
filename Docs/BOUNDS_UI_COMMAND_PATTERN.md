# Bounds Editing: UI Command Pattern & Mode Switching

## Problem Statement

Originally `SetEditMode()` in `BoundsHandler` hardcoded all draft shapes as EXTERNAL. We fixed that by introducing `SetShapeType()` / `GetShapeType()` and using `m_shapeType` when creating drafts.

With the updated UI spec we also changed how commands are exposed:

- We no longer have 12 separate commands for (EXTERNAL/INTERNAL/APERTURE) × (Rectangle/Ellipse/Circle/Polygon).
- The UI exposes 4 geometry commands + 1 two-state type toggle (APERTURE/INTERNAL).
- EXTERNAL shapes are still supported for legacy data but are not created via UI for now.

---

## Architecture Overview: UI → Mode Switch → Bounds Handler

```
┌─────────────────────────────────────────────────────────────────┐
│                      Resource File (.rc)                        │
│  ID_BOUND_VISIBILITY      = 80000  (APERTURE/INTERNAL toggle)  │
│  ID_ADD_BOUND_CIRCLE      = 80001                             │
│  ID_ADD_BOUND_ELLIPSE     = 80002                             │
│  ID_ADD_BOUND_RECT        = 80003                             │
│  ID_ADD_BOUND_POLYGON     = 80004                             │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                     CImageView Command Handlers                 │
│  OnBoundVisisbility()         ───→ Calls ActivateBoundsTool()   │
│                                  + SetShapeType(APERTURE/INT)   │
│  OnAddBoundCircle/Rect/...    ───→   + SetShapeType(current)    │
│                                  + SetEditMode(AddXXX)          │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                    BoundsInputHandler                           │
│  ActivateBoundsTool()  ───→ SetActiveTool(&m_boundsHandler)     │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                     BoundsHandler                               │
│  SetShapeType(aperture::TypeLimits::APERTURE/INTERNAL)          │
│  SetEditMode(ShapeEditMode::AddEllipse)                         │
│                                                                  │
│  Result:                                                         │
│  ├─ m_editMode = AddEllipse                                     │
│  ├─ m_draft.type = current m_shapeType                          │
│  └─ User can now draw points, creates APERTURE/INTERNAL ellipse │
└─────────────────────────────────────────────────────────────────┘
```

---

## Solution 1: Add SetShapeType() Method (Recommended)

### Step 1: Modify BoundsHandler.h

Add methods to explicitly set/get the shape type and default to APERTURE:

```cpp
// In BoundsHandler.h, add to public section:

/**
 * @brief Set the shape type for new shapes
 * @param type Shape type (EXTERNAL, INTERNAL, or APERTURE)
 * 
 * This controls what type of shape will be created when committing
 * a draft in Add modes. Must be called BEFORE AddDraftPoint().
 */
void SetShapeType(aperture::TypeLimits type) { m_shapeType = type; }

/**
 * @brief Get current shape type for new shapes
 * @return Type used for draft shapes
 */
aperture::TypeLimits GetShapeType() const { return m_shapeType; }

// Also add member variable (default to APERTURE for new UI):
private:
    aperture::TypeLimits m_shapeType = aperture::TypeLimits::APERTURE;
```

### Step 2: Modify BoundsHandler.cpp - SetEditMode()

```cpp
void BoundsHandler::SetEditMode(DigitMode::ShapeEditMode mode)
{
    // Cancel any active operations when switching modes
    if (m_isDragging) {
        CancelDrag();
    }
    
    if (IsDrafting()) {
        CancelDraft();
    }
    
    m_editMode = mode;
    
    // Initialize draft if switching to Add mode
    if (mode == DigitMode::ShapeEditMode::AddRectangle ||
        mode == ShapeEditMode::AddEllipse ||
        mode == ShapeEditMode::AddCircle ||
        mode == ShapeEditMode::AddPolygon)
    {
        // Create new draft with appropriate kind
        DraftShape draft;
        draft.type = m_shapeType;  // ✓ USE STORED SHAPE TYPE INSTEAD OF HARDCODED
        
        switch (mode) {
            case ShapeEditMode::AddRectangle:
                draft.kind = DraftShape::Kind::Rectangle;
                break;
            case ShapeEditMode::AddEllipse:
                draft.kind = DraftShape::Kind::Ellipse;
                break;
            case ShapeEditMode::AddCircle:
                draft.kind = DraftShape::Kind::Circle;
                break;
            case ShapeEditMode::AddPolygon:
                draft.kind = DraftShape::Kind::Polygon;
                break;
            default:
                break;
        }
        
        m_draft = draft;
    }
    else {
        // Clear draft if not in Add mode
        m_draft.reset();
        m_draftPreview.reset();
    }
}
```

---

## Solution 2: Combined SetEditModeAndType() Method (Alternative)

If you prefer a single method call, create a convenience overload:

```cpp
// In BoundsHandler.h:

/**
 * @brief Set edit mode and shape type in one call
 * @param mode New edit mode
 * @param type Shape type (EXTERNAL, INTERNAL, or APERTURE)
 */
void SetEditModeAndType(ShapeEditMode mode, aperture::TypeLimits type)
{
    SetShapeType(type);
    SetEditMode(mode);
}
```

This allows command handlers to use:

```cpp
m_boundsHandler.SetEditModeAndType(
    ShapeEditMode::AddEllipse,
    aperture::TypeLimits::INTERNAL
);
```

---

## Complete UI Command Handler Examples

### Example 1: Add Rectangle Using Current Type (APERTURE/INTERNAL)

```cpp
// Message map (excerpt)
ON_COMMAND(ID_ADD_BOUND_RECT, OnAddBoundRect)
ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_RECT, OnUpdateAddBound)

void CImageView::OnAddBoundRect()
{
    if (!GetImageCtrls()->HasImage())
        return;

    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);                 // Type (APERTURE/INTERNAL)
    handler.SetEditMode(ShapeEditMode::AddRectangle); // Geometry

    GetMainFrame()->SetStatusText("Click corners to create rectangular bound");
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBound(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

### Example 2: Type Toggle Command

```cpp
// Message map (excerpt)
ON_COMMAND(ID_BOUND_VISIBILITY, OnBoundVisisbility)
ON_UPDATE_COMMAND_UI(ID_BOUND_VISIBILITY, OnUpdateBoundVisibility)

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

void CImageView::OnUpdateBoundVisibility(CCmdUI* pCmdUI)
{
    bool hasImage = GetImageCtrls()->HasImage();
    pCmdUI->Enable(hasImage ? TRUE : FALSE);
    if (!hasImage)
        return;

    aperture::TypeLimits current = m_boundsHandler.GetBoundsHandler().GetShapeType();
    pCmdUI->SetCheck(current == aperture::TypeLimits::INTERNAL);
}
```

---

## Complete Mode Flow Diagram

### Scenario: User Adds Internal Ellipse Obstruction

```
1. USER ACTION:
   - Ensure bounds type is INTERNAL (toggle ID_BOUND_VISIBILITY pressed)
   - Click menu: "Bounds" → "Add Shape" → "Ellipse"
   
   ▼
   
2. MESSAGE ROUTING:
   WM_COMMAND with ID_ADD_BOUND_ELLIPSE
   ▼
   CImageView::OnAddBoundEllipse()
   
   ▼
   
3. TOOL ACTIVATION:
   ActivateBoundsTool()
   └─ InputRouter.SetActiveTool(&m_boundsHandler)
   
   ▼
   
4. MODE & TYPE CONFIGURATION:
   SetShapeType(INTERNAL)
   │
   └─ m_shapeType = INTERNAL
   
   SetEditMode(AddEllipse)
   │
   ├─ Cancel any active drag/draft
   ├─ Create DraftShape
   ├─ draft.type = m_shapeType  ✓ NOW USES INTERNAL
   ├─ draft.kind = Ellipse
   └─ m_draft = draft
   
   ▼
   
5. USER INTERACTION - POINT SEQUENCE:
   
   a) First click (center):
      OnMouseDown → BoundsInputHandler::OnMouseDown()
      ├─ Convert screen to world coords
      └─ m_boundsHandler.AddDraftPoint(worldPt)
         └─ m_draft->AddPoint()
      
      ▼ (Screen shows draft preview ellipse)
      
   b) Second click (edge point):
      OnMouseDown → BoundsInputHandler::OnMouseDown()
      ├─ Convert screen to world coords
      └─ m_boundsHandler.AddDraftPoint(worldPt)
         └─ m_draft->AddPoint()
      
      ▼ (Ellipse now fully defined)
      
   c) User presses Enter or clicks again:
      m_boundsHandler.CommitDraft()
      ├─ shape = m_draft->ToShape()  // Geometry from draft points
      ├─ AddShapeCommand created with:
      │  ├─ type = INTERNAL
      │  ├─ shape = new ellipse
      │  └─ apertureCtrls = reference to aperture controls
      └─ Dispatcher.Execute(command)
         └─ Undo/redo integrated
   
   ▼
   
6. VISUAL RESULT:
   New INTERNAL ellipse appears in shape list
   └─ Stored in CApertureCtrls::m_shapes.getInternal()
   
   ▼
   
7. USER CAN NOW:
   - Add more shapes (draft mode stays active)
   - Press ESC to cancel current draft
   - Switch to Edit mode (Select) to modify shapes
   - Click elsewhere to finalize
```

---

## When to Switch Modes: Complete Decision Tree

```
┌─────────────────────────────────────────────────────────────────┐
│                   USER CLICKS MENU ITEM                         │
└─────────────────────────────────────────────────────────────────┘
                            ▼
                ┌─────────────────────────┐
                │ ADD SHAPE or EDIT?      │
                └─────────────────────────┘
                /             |            \
               /              |             \
              ▼               ▼              ▼
       ┌─────────────┐  ┌──────────┐  ┌──────────────┐
       │ ADD SHAPE   │  │ SELECT   │  │ DELETE       │
       │             │  │ (Edit)   │  │              │
       └─────────────┘  └──────────┘  └──────────────┘
            │                │              │
            │                │              │
    ┌───────┴────────┐       │              │
    │                │       │              │
    ▼                ▼       ▼              ▼
┌─────────┐  ┌──────────┐ ┌──────┐  ┌────────────┐
│ WHICH   │  │ RECT?    │ │      │  │ OnDelete() │
│ SHAPE?  │  │ ELLIPSE? │ │Set   │  │            │
│         │  │ CIRCLE?  │ │mode  │  │SetEditMode │
├─────────┤  │ POLYGON? │ │to    │  │(Delete)    │
│EXTERNAL?│  └──────────┘ │Select│  └────────────┘
│INTERNAL?│               │      │
│APERTURE?│               └──────┘
└────┬────┘
     │
     ▼
┌───────────────────────────────────┐
│ DETERMINE:                        │
│ 1. ShapeType (External/Internal/) │
│ 2. ShapeEditMode (geometry type)  │
└───────────────────────────────────┘
     │
     ▼
┌───────────────────────────────────┐
│ CALL:                             │
│ ActivateBoundsTool();             │
│ SetShapeType(type);               │
│ SetEditMode(mode);                │
└───────────────────────────────────┘
     │
     ▼
┌───────────────────────────────────┐
│ USER INTERACTION:                 │
│ - Add: Click points, press Enter  │
│ - Edit: Drag handles             │
│ - Delete: Click shape            │
└───────────────────────────────────┘
```

---

## Resource File (.rc) Command IDs

Add these to your resource file:

```rc
// Bounds editing commands - Add External shapes
#define ID_ADD_BOUND_EXTERNAL_RECT      32801
#define ID_ADD_BOUND_EXTERNAL_ELLIPSE   32802
#define ID_ADD_BOUND_EXTERNAL_CIRCLE    32803
#define ID_ADD_BOUND_EXTERNAL_POLYGON   32804

// Bounds editing commands - Add Internal shapes
#define ID_ADD_BOUND_INTERNAL_RECT      32810
#define ID_ADD_BOUND_INTERNAL_ELLIPSE   32811
#define ID_ADD_BOUND_INTERNAL_CIRCLE    32812
#define ID_ADD_BOUND_INTERNAL_POLYGON   32813

// Bounds editing commands - Add Aperture shapes
#define ID_ADD_BOUND_APERTURE_RECT      32820
#define ID_ADD_BOUND_APERTURE_ELLIPSE   32821
#define ID_ADD_BOUND_APERTURE_CIRCLE    32822
#define ID_ADD_BOUND_APERTURE_POLYGON   32823

// Bounds editing mode commands
#define ID_BOUND_MODE_SELECT            32830
#define ID_BOUND_MODE_DELETE            32831
#define ID_BOUND_CANCEL_DRAFT           32832
```

---

## Menu Structure Example

```
Edit (menu)
├─ Bounds (submenu)
│  ├─ Add External (submenu)
│  │  ├─ Rectangle...        [ID_ADD_BOUND_EXTERNAL_RECT]
│  │  ├─ Ellipse...          [ID_ADD_BOUND_EXTERNAL_ELLIPSE]
│  │  ├─ Circle...           [ID_ADD_BOUND_EXTERNAL_CIRCLE]
│  │  └─ Polygon...          [ID_ADD_BOUND_EXTERNAL_POLYGON]
│  │
│  ├─ Add Internal (submenu)
│  │  ├─ Rectangle...        [ID_ADD_BOUND_INTERNAL_RECT]
│  │  ├─ Ellipse...          [ID_ADD_BOUND_INTERNAL_ELLIPSE]
│  │  ├─ Circle...           [ID_ADD_BOUND_INTERNAL_CIRCLE]
│  │  └─ Polygon...          [ID_ADD_BOUND_INTERNAL_POLYGON]
│  │
│  ├─ Add Aperture (submenu)
│  │  ├─ Rectangle...        [ID_ADD_BOUND_APERTURE_RECT]
│  │  ├─ Ellipse...          [ID_ADD_BOUND_APERTURE_ELLIPSE]
│  │  ├─ Circle...           [ID_ADD_BOUND_APERTURE_CIRCLE]
│  │  └─ Polygon...          [ID_ADD_BOUND_APERTURE_POLYGON]
│  │
│  ├─ Separator
│  ├─ Select Mode (radio)     [ID_BOUND_MODE_SELECT]
│  ├─ Delete Mode (radio)     [ID_BOUND_MODE_DELETE]
│  └─ Cancel Draft (grayed)   [ID_BOUND_CANCEL_DRAFT]
```

---

## Key Implementation Rules

### Rule 1: Always Call in Order
```cpp
void OnAddSomeShape() {
    // 1. FIRST: Activate tool (or ensure already active)
    ActivateBoundsTool();
    
    // 2. SECOND: Set shape type
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    
    // 3. THIRD: Set edit mode (creates draft with correct type)
    m_boundsHandler.SetEditMode(ShapeEditMode::AddEllipse);
    
    // 4. FOURTH: Update UI/status
    GetMainFrame()->SetStatusText("Adding internal ellipse...");
    
    // 5. FIFTH: Invalidate for visual feedback
    Invalidate(FALSE);
}
```

### Rule 2: SetEditMode() Clears Previous State
```cpp
// If user switches modes while drafting:
SetShapeType(TypeLimits::EXTERNAL);
SetEditMode(ShapeEditMode::AddRectangle);  // ← Cancels draft from previous Add mode!
```

### Rule 3: Commands Flow Through BoundsInputHandler
```
User click on canvas
    ↓
CImageView::OnLButtonDown() in BaseImageView (MFC message routing)
    ↓
InputRouter::OnMouseDown() (dispatcher)
    ↓
BoundsInputHandler::OnMouseDown() (active tool)
    ↓
BoundsHandler methods (coordinate conversion, draft management)
    ↓
Command creation (AddShapeCommand, ReplaceShapeCommand)
    ↓
CommandDispatcher::Execute() (undo/redo integrated)
```

### Rule 4: Shape Type Set Before Commit
```cpp
// WRONG - type not set when drafting:
SetEditMode(AddEllipse);
AddDraftPoint(...);
AddDraftPoint(...);
SetShapeType(INTERNAL);  // ❌ Too late! Draft already has type=EXTERNAL
CommitDraft();

// RIGHT - type set before mode:
SetShapeType(INTERNAL);   // ✓ Set first
SetEditMode(AddEllipse);  // ✓ Then set mode (initializes draft with correct type)
AddDraftPoint(...);
AddDraftPoint(...);
CommitDraft();            // ✓ Creates INTERNAL shape
```

---

## Coordinate Conversion Happens in BoundsInputHandler

When user clicks on canvas, here's what happens:

```cpp
// In BoundsInputHandler::OnMouseDown()
bool BoundsInputHandler::OnMouseDown(UINT flags, CPoint screenPt)
{
    // 1. Bounds handler not in Add mode? Delegate to selection/deletion
    if (m_boundsHandler.GetEditMode() == ShapeEditMode::Select) {
        // Hit test and prepare for drag
        auto hit = m_boundsHandler.HitTest(screenPt);
        if (hit.hit) {
            // Prepare drag with world coordinates
            m_boundsHandler.BeginDrag(hit.type, hit.shapeIndex, hit.controlPointIndex, screenPt);
            return true;  // CONSUMED
        }
    }
    
    // 2. Bounds handler in Add mode? Start drafting
    if (m_boundsHandler.GetEditMode() >= ShapeEditMode::AddRectangle &&
        m_boundsHandler.GetEditMode() <= ShapeEditMode::AddPolygon) {
        
        // Convert screen point to world coordinates
        CPoint2d worldPt = m_worldToScreen.ScreenToWorld(
            CPoint2d{ (double)screenPt.x, (double)screenPt.y }
        );
        
        // Add to draft
        m_boundsHandler.AddDraftPoint(worldPt);
        
        return true;  // CONSUMED
    }
    
    // 3. Not in bounds mode - delegate to navigation
    return false;  // Navigation will handle pan/zoom
}
```

---

## Testing the Mode Switch

```cpp
// In a test or debug scenario:

// Initialize bounds handler
m_boundsHandler.SetApertureCtrls(pApertureCtrls, pImage);
m_boundsHandler.SetViewTransform(&m_viewTransform);

// TEST CASE: Add Internal Ellipse
m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
ASSERT_EQ(aperture::TypeLimits::INTERNAL, m_boundsHandler.GetShapeType());

m_boundsHandler.SetEditMode(ShapeEditMode::AddEllipse);
ASSERT_EQ(ShapeEditMode::AddEllipse, m_boundsHandler.GetEditMode());

// Verify draft has correct type
ASSERT_TRUE(m_boundsHandler.IsDrafting());
const auto* draft = m_boundsHandler.GetDraft();
ASSERT_EQ(aperture::TypeLimits::INTERNAL, draft->type);

// Add points and commit
m_boundsHandler.AddDraftPoint(aperture::Point{100.0, 100.0});
m_boundsHandler.AddDraftPoint(aperture::Point{200.0, 150.0});
m_boundsHandler.CommitDraft();

// Verify shape was added as INTERNAL
const auto& shapes = pApertureCtrls->GetShapes();
ASSERT_FALSE(shapes.getInternal().empty());
ASSERT_TRUE(shapes.getExternal().empty());
```

---

## Summary: When to Call What

| Scenario | Call Sequence |
|----------|---|
| **User clicks "Add Internal Ellipse"** | `ActivateBoundsTool()` → `SetShapeType(INTERNAL)` → `SetEditMode(AddEllipse)` |
| **User clicks "Add External Rectangle"** | `ActivateBoundsTool()` → `SetShapeType(EXTERNAL)` → `SetEditMode(AddRectangle)` |
| **User clicks "Add Aperture Circle"** | `ActivateBoundsTool()` → `SetShapeType(APERTURE)` → `SetEditMode(AddCircle)` |
| **User switches to Select/Edit mode** | `ActivateBoundsTool()` → `SetEditMode(Select)` (type doesn't matter) |
| **User switches to Delete mode** | `ActivateBoundsTool()` → `SetEditMode(Delete)` (type doesn't matter) |
| **User closes bounds tool** | `ActivateFringeTool()` or `GetInputRouter().SetActiveTool(&m_navigationHandler)` |

---

## Next Steps

1. ✅ **Add SetShapeType() to BoundsHandler** - Simple getter/setter
2. ✅ **Modify SetEditMode() to use m_shapeType** - Instead of hardcoded EXTERNAL
3. ✅ **Create command handlers in CImageView** - One per shape type/geometry combo
4. ✅ **Add message map entries** - Connect resource IDs to handlers
5. ✅ **Add menu items** - Create Bounds submenu with organized options
6. ✅ **Update UI feedback** - Status bar shows current mode/type

This pattern ensures that shape type is specified BEFORE drafting begins, and the draft is initialized with the correct type.

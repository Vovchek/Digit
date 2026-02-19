# Bounds Editing: UI Command Pattern & Mode Switching

## Problem Statement

**Current Issue**: The `SetEditMode()` method in `BoundsHandler` defaults to creating EXTERNAL shapes:

```cpp
// Current implementation in BoundsHandler::SetEditMode()
draft.type = aperture::TypeLimits::EXTERNAL;  // ❌ HARDCODED - can't specify INTERNAL/APERTURE
```

When user clicks "Add Internal Ellipse" menu item, we have no way to tell BoundsHandler that it should create an INTERNAL shape instead of EXTERNAL.

**Solution**: Add shape type specification to the edit mode API.

---

## Architecture Overview: UI → Mode Switch → Bounds Handler

```
┌─────────────────────────────────────────────────────────────────┐
│                      Resource File (.rc)                        │
│  ID_ADD_BOUND_EXTERNAL_RECT  = 32801                           │
│  ID_ADD_BOUND_INTERNAL_ELLIPSE = 32802                         │
│  ID_ADD_BOUND_APERTURE_CIRCLE = 32803                          │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                     CImageView Command Handlers                 │
│  OnAddBoundExternalRect()      ───→ Calls ActivateBoundsTool()  │
│  OnAddBoundInternalEllipse()   ───→   + SetEditMode()           │
│  OnAddBoundApertureCircle()    ───→   + SetShapeType()          │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                    BoundsInputHandler                           │
│  ActivateBoundsTool()  ───→ SetActiveTool(&m_boundsHandler)     │
└─────────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                     BoundsHandler                               │
│  SetEditMode(ShapeEditMode::AddEllipse)                        │
│  SetShapeType(aperture::TypeLimits::INTERNAL)  ← NEW METHOD     │
│                                                                  │
│  Result:                                                         │
│  ├─ m_editMode = AddEllipse                                     │
│  ├─ m_draft.type = INTERNAL                                     │
│  └─ User can now draw points, creates INTERNAL ellipse          │
└─────────────────────────────────────────────────────────────────┘
```

---

## Solution 1: Add SetShapeType() Method (Recommended)

### Step 1: Modify BoundsHandler.h

Add a new method to explicitly set the shape type:

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

// Also add member variable:
private:
    aperture::TypeLimits m_shapeType = aperture::TypeLimits::EXTERNAL;
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

### Example 1: Add External Rectangle

```cpp
// In ImageTempl/ImageView.h
afx_msg void OnAddBoundExternalRect();
afx_msg void OnUpdateAddBoundExternalRect(CCmdUI* pCmdUI);

// In ImageTempl/ImageView.cpp - Message Map
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    // ...existing entries...
    ON_COMMAND(ID_ADD_BOUND_EXTERNAL_RECT, OnAddBoundExternalRect)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_EXTERNAL_RECT, OnUpdateAddBoundExternalRect)
    // ...existing entries...
END_MESSAGE_MAP()

// Implementation
void CImageView::OnAddBoundExternalRect()
{
    // Step 1: Activate bounds tool
    ActivateBoundsTool();
    
    // Step 2: Set edit mode AND shape type
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddRectangle);
    
    // Step 3: Update UI (e.g., status bar)
    GetMainFrame()->SetStatusText("Click corners to create external rectangle");
    
    // Step 4: Invalidate for visual feedback
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundExternalRect(CCmdUI* pCmdUI)
{
    // Enable if bounds tool can be activated
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

### Example 2: Add Internal Ellipse

```cpp
void CImageView::OnAddBoundInternalEllipse()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddEllipse);
    GetMainFrame()->SetStatusText("Click to define internal ellipse obstruction");
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundInternalEllipse(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

### Example 3: Add Aperture Circle

```cpp
void CImageView::OnAddBoundApertureCircle()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddCircle);
    GetMainFrame()->SetStatusText("Click center and edge to define aperture circle");
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundApertureCircle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

---

## Complete Mode Flow Diagram

### Scenario: User Adds Internal Ellipse Obstruction

```
1. USER ACTION:
   Click menu: "Bounds" → "Add Internal Ellipse"
   
   ▼
   
2. MESSAGE ROUTING:
   WM_COMMAND with ID_ADD_BOUND_INTERNAL_ELLIPSE
   ▼
   CImageView::OnAddBoundInternalEllipse()
   
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

# Implementing Bounds UI Commands: Step-by-Step Guide

## Overview

This guide walks through adding actual UI command handlers for bounds editing. We'll implement the complete flow from menu click to shape creation.

---

## Step 1: Add Resource IDs to .rc File

Edit your main resource file and add these command IDs (IDs shown here match `Resource.h`):

```rc
// Bounds editing - Type toggle and Add shapes
// Single two-state toggle switches APERTURE / INTERNAL type
#define ID_BOUND_VISIBILITY             80000 // switch APERTURE / INTERNAL bounds editing mode

// Geometry commands use the current type from ID_BOUND_VISIBILITY
#define ID_ADD_BOUND_CIRCLE             80001
#define ID_ADD_BOUND_ELLIPSE            80002
#define ID_ADD_BOUND_RECT               80003
#define ID_ADD_BOUND_POLYGON            80004

// Bounds editing - Mode commands
#define ID_BOUND_MODE_SELECT            80010
#define ID_BOUND_MODE_DELETE            80011
#define ID_BOUND_CANCEL_DRAFT           80012
```

---

## Step 2: Add Handler Declarations to ImageView.h

Add to `ImageTempl/ImageView.h` in the public section of `CImageView`:

```cpp
public:
    // ========================================================================
    // Bounds Editing Command Handlers (Phase 5)
    // ========================================================================

    // Add shapes (geometry only, type comes from visibility toggle)
    afx_msg void OnAddBoundCircle();
    afx_msg void OnUpdateAddBound(CCmdUI* pCmdUI);

    afx_msg void OnAddBoundEllipse();
    afx_msg void OnAddBoundRect();
    afx_msg void OnAddBoundPolygon();

    // Type toggle (APERTURE / INTERNAL)
    afx_msg void OnBoundVisisbility();
    afx_msg void OnUpdateBoundVisibility(CCmdUI* pCmdUI);

    // Mode switching
    afx_msg void OnBoundModeSelect();
    afx_msg void OnUpdateBoundModeSelect(CCmdUI* pCmdUI);

    afx_msg void OnBoundModeDelete();
    afx_msg void OnUpdateBoundModeDelete(CCmdUI* pCmdUI);
```

---

## Step 3: Add Message Map Entries to ImageView.cpp

In the `BEGIN_MESSAGE_MAP`/`END_MESSAGE_MAP` block of `CImageView`, add:

```cpp
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    // ...existing entries...

    // bounds editing commands
    ON_COMMAND(ID_ADD_BOUND_CIRCLE, OnAddBoundCircle)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_CIRCLE, OnUpdateAddBound)
    ON_COMMAND(ID_ADD_BOUND_ELLIPSE, OnAddBoundEllipse)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_ELLIPSE, OnUpdateAddBound)
    ON_COMMAND(ID_ADD_BOUND_RECT, OnAddBoundRect)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_RECT, OnUpdateAddBound)
    ON_COMMAND(ID_ADD_BOUND_POLYGON, OnAddBoundPolygon)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_POLYGON, OnUpdateAddBound)
    ON_COMMAND(ID_BOUND_VISIBILITY, OnBoundVisisbility)
    ON_UPDATE_COMMAND_UI(ID_BOUND_VISIBILITY, OnUpdateBoundVisibility)
    ON_COMMAND(ID_BOUND_MODE_SELECT, OnBoundModeSelect)
    ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_SELECT, OnUpdateBoundModeSelect)
    ON_COMMAND(ID_BOUND_MODE_DELETE, OnBoundModeDelete)
    ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_DELETE, OnUpdateBoundModeDelete)

    // ...existing entries...
END_MESSAGE_MAP()
```

---

## Step 4: Implement Command Handlers (Toolbar + Toggle)

With the new spec we have:

- 4 geometry commands: `ID_ADD_BOUND_CIRCLE`, `ID_ADD_BOUND_RECT`, `ID_ADD_BOUND_ELLIPSE`, `ID_ADD_BOUND_POLYGON`.
- 1 two-state type toggle: `ID_BOUND_VISIBILITY` (APERTURE / INTERNAL).
- 2 mode commands: `ID_BOUND_MODE_SELECT`, `ID_BOUND_MODE_DELETE`.

`BoundsHandler` now defaults to `APERTURE` for `m_shapeType`. `ID_BOUND_VISIBILITY` flips this between `APERTURE` and `INTERNAL`. The Add commands always use "current type + selected geometry".

### Add Shape Commands (geometry only)

```cpp
void CImageView::OnAddBoundCircle()
{
    if (!GetImageCtrls()->HasImage())
        return;

    ActivateBoundsTool();

    // Respect Key Pattern: set type before mode
    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);
    handler.SetEditMode(DigitMode::ShapeEditMode::AddCircle);

    GetMainFrame()->SetStatusText(_T("Click center and edge to define circular bound"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBound(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundEllipse()
{
    if (!GetImageCtrls()->HasImage())
        return;

    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);
    handler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);

    GetMainFrame()->SetStatusText(_T("Click to define elliptical bound"));
    Invalidate(FALSE);
}

void CImageView::OnAddBoundRect()
{
    if (!GetImageCtrls()->HasImage())
        return;

    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);
    handler.SetEditMode(DigitMode::ShapeEditMode::AddRectangle);

    GetMainFrame()->SetStatusText(_T("Click corners to create rectangular bound"));
    Invalidate(FALSE);
}

void CImageView::OnAddBoundPolygon()
{
    if (!GetImageCtrls()->HasImage())
        return;

    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    handler.SetShapeType(currentType);
    handler.SetEditMode(DigitMode::ShapeEditMode::AddPolygon);

    GetMainFrame()->SetStatusText(_T("Click vertices to create polygonal bound (press Enter when done)"));
    Invalidate(FALSE);
}
```

### Type Toggle (APERTURE / INTERNAL)

`ID_BOUND_VISIBILITY` is a two-state toolbar button. Unchecked = APERTURE, Checked = INTERNAL.

```cpp
void CImageView::OnBoundVisisbility()
{
    // Toggle between APERTURE and INTERNAL for subsequent Add commands
    ActivateBoundsTool();

    DigitMode::BoundsHandler& handler = m_boundsHandler.GetBoundsHandler();
    aperture::TypeLimits currentType = handler.GetShapeType();
    aperture::TypeLimits nextType =
        (currentType == aperture::TypeLimits::INTERNAL)
            ? aperture::TypeLimits::APERTURE
            : aperture::TypeLimits::INTERNAL;

    handler.SetShapeType(nextType);

    // If a draft was already started in previous type, cancel it so
    // the next shape uses the new type cleanly.
    if (handler.IsDrafting())
        handler.CancelDraft();

    LPCTSTR msg = (nextType == aperture::TypeLimits::INTERNAL)
        ? _T("Bounds type: INTERNAL (obstructions)")
        : _T("Bounds type: APERTURE (visible pupil)");
    GetMainFrame()->SetStatusText(msg);
    Invalidate(FALSE);
}

void CImageView::OnUpdateBoundVisibility(CCmdUI* pCmdUI)
{
    bool hasImage = GetImageCtrls()->HasImage();
    pCmdUI->Enable(hasImage ? TRUE : FALSE);
    if (!hasImage)
        return;

    aperture::TypeLimits currentType = m_boundsHandler.GetBoundsHandler().GetShapeType();
    bool isInternal = (currentType == aperture::TypeLimits::INTERNAL);
    pCmdUI->SetCheck(isInternal ? TRUE : FALSE); // pressed = INTERNAL
}
```

### Mode Switching (Select / Delete)

Mode switching remains the same as in the original guide:

```cpp
void CImageView::OnBoundModeSelect()
{
    ActivateBoundsTool();
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::Select);
    GetMainFrame()->SetStatusText(_T("Bounds: Select mode - drag to modify shapes"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateBoundModeSelect(CCmdUI* pCmdUI)
{
    bool isSelect = (m_boundsHandler.GetEditMode() == DigitMode::ShapeEditMode::Select);
    pCmdUI->SetRadio(isSelect ? TRUE : FALSE);
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnBoundModeDelete()
{
    ActivateBoundsTool();
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::Delete);
    GetMainFrame()->SetStatusText(_T("Bounds: Delete mode - click shapes to remove"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateBoundModeDelete(CCmdUI* pCmdUI)
{
    bool isDelete = (m_boundsHandler.GetEditMode() == DigitMode::ShapeEditMode::Delete);
    pCmdUI->SetRadio(isDelete ? TRUE : FALSE);
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

void CImageView::OnAddBoundInternalEllipse()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);
    GetMainFrame()->SetStatusText(_T("Click to define internal elliptical obstruction"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundInternalEllipse(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundInternalCircle()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddCircle);
    GetMainFrame()->SetStatusText(_T("Click center and edge to define internal circular obstruction"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundInternalCircle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundInternalPolygon()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddPolygon);
    GetMainFrame()->SetStatusText(_T("Click vertices to create internal polygonal obstruction (press Enter when done)"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundInternalPolygon(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

// Add Aperture shapes
void CImageView::OnAddBoundApertureRect()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddRectangle);
    GetMainFrame()->SetStatusText(_T("Click corners to define aperture rectangle"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundApertureRect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundApertureEllipse()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);
    GetMainFrame()->SetStatusText(_T("Click to define aperture ellipse"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundApertureEllipse(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundApertureCircle()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddCircle);
    GetMainFrame()->SetStatusText(_T("Click center and edge to define aperture circle"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundApertureCircle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundAperturePolygon()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddPolygon);
    GetMainFrame()->SetStatusText(_T("Click vertices to define aperture polygon (press Enter when done)"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundAperturePolygon(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

// Mode switching
void CImageView::OnBoundModeSelect()
{
    ActivateBoundsTool();
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::Select);
    GetMainFrame()->SetStatusText(_T("Bounds: Select mode - drag to modify shapes"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateBoundModeSelect(CCmdUI* pCmdUI)
{
    bool isSelect = (m_boundsHandler.GetEditMode() == DigitMode::ShapeEditMode::Select);
    pCmdUI->SetRadio(isSelect ? TRUE : FALSE);
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnBoundModeDelete()
{
    ActivateBoundsTool();
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::Delete);
    GetMainFrame()->SetStatusText(_T("Bounds: Delete mode - click shapes to remove"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateBoundModeDelete(CCmdUI* pCmdUI)
{
    bool isDelete = (m_boundsHandler.GetEditMode() == DigitMode::ShapeEditMode::Delete);
    pCmdUI->SetRadio(isDelete ? TRUE : FALSE);
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}
```

---

## Step 5: Create Menu Structure in Resource Editor

### Menu Items (Edit → Bounds)

Create this menu structure (or edit via RC file):

```
IDR_MAINFRAME (or your main menu resource ID)
├─ &Edit
│  ├─ &Bounds
│  │  ├─ Add &Shape
│  │  │  ├─ &Rectangle...\tCtrl+Shift+R    [ID_ADD_BOUND_EXTERNAL_RECT]
│  │  │  ├─ &Ellipse...\tCtrl+Shift+E      [ID_ADD_BOUND_EXTERNAL_ELLIPSE]
│  │  │  ├─ &Circle...\tCtrl+Shift+C       [ID_ADD_BOUND_EXTERNAL_CIRCLE]
│  │  │  └─ &Polygon...\tCtrl+Shift+P      [ID_ADD_BOUND_EXTERNAL_POLYGON]
│  │  ├─ ─────────────────────
│  │  ├─ &Select Type (radio)    [ID_BOUND_VISIBILITY]
│  │  ├─ &Select Mode (radio)    [ID_BOUND_MODE_SELECT]
│  │  ├─ &Delete Mode (radio)    [ID_BOUND_MODE_DELETE]
```

---

## Step 6: Add Keyboard Accelerators (Optional)

If you want keyboard shortcuts, add to your accelerator table (IDR_MAINFRAME):

```
// Add to accelerator table (geometry uses current type from ID_BOUND_VISIBILITY)
CTRL SHIFT C     ID_ADD_BOUND_CIRCLE
CTRL SHIFT R     ID_ADD_BOUND_RECT
CTRL SHIFT E     ID_ADD_BOUND_ELLIPSE
CTRL SHIFT P     ID_ADD_BOUND_POLYGON

// Toggle type Aperture/Internal
CTRL SHIFT T     ID_BOUND_VISIBILITY

// Quick access to modes
CTRL SHIFT S     ID_BOUND_MODE_SELECT
CTRL SHIFT_D     ID_BOUND_MODE_DELETE
```

---

## Step 7: Testing the Integration

### Manual Test Case 1: Add Internal Ellipse

```
1. Open image in CImageView
2. Ensure bounds type is INTERNAL
   - If toolbar button ID_BOUND_VISIBILITY is not pressed → click it once
   - Or use menu: Edit → Bounds → Bounds Type (checked = INTERNAL)
   ✓ BoundsHandler shape type is INTERNAL

3. Click: Edit → Bounds → Add Shape → Ellipse
   ✓ Status bar shows: "Click to define elliptical bound" (generic text)
   ✓ Cursor changes (if implemented)
   ✓ BoundsHandler is activated with:
     - EditMode = AddEllipse
     - ShapeType = INTERNAL

4. Click first point (center)
   ✓ Draft point added
   ✓ Preview shows dashed ellipse outline

5. Click second point (radius)
   ✓ Ellipse updated in preview
   ✓ Ellipse is now fully defined

6. Press Enter or click again
   ✓ Draft committed
   ✓ AddShapeCommand created with type=INTERNAL
   ✓ Shape added to CApertureCtrls internal shapes list
   ✓ Undo/redo available

7. Verify
   ASSERT_TRUE(apertureCtrls->GetShapes().getInternal().size() > 0);
```

### Manual Test Case 2: Switch Modes

```
1. Start with an ellipse being drafted in any type
2. Press Escape
   ✓ Draft canceled
   ✓ m_draft is cleared

3. Click: Edit → Bounds → Select Mode
   ✓ EditMode = Select
   ✓ Can now drag handles on existing shapes
   ✓ Status bar shows selection help
```

### Unit Test Case

```cpp
TEST_F(BoundsHandlerTest, SetShapeTypeInternalBeforeDrafting)
{
    // Set to INTERNAL before entering Add mode
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddEllipse);
    
    // Verify draft has INTERNAL type
    ASSERT_TRUE(m_boundsHandler.IsDrafting());
    const auto* draft = m_boundsHandler.GetDraft();
    ASSERT_EQ(aperture::TypeLimits::INTERNAL, draft->type);
    
    // Add points
    m_boundsHandler.AddDraftPoint(aperture::Point{100.0, 100.0});
    m_boundsHandler.AddDraftPoint(aperture::Point{200.0, 150.0});
    
    // Commit
    m_boundsHandler.CommitDraft();
    
    // Verify shape created as INTERNAL
    const auto& shapes = m_apertureCtrls.GetShapes();
    ASSERT_EQ(1, shapes.getInternal().size());
    ASSERT_EQ(0, shapes.getExternal().size());
    ASSERT_EQ(0, shapes.getApertures().size());
}

TEST_F(BoundsHandlerTest, SetShapeTypeApertureBeforeDrafting)
{
    m_boundsHandler.SetShapeType(aperture::TypeLimits::APERTURE);
    m_boundsHandler.SetEditMode(ShapeEditMode::AddCircle);
    
    ASSERT_TRUE(m_boundsHandler.IsDrafting());
    const auto* draft = m_boundsHandler.GetDraft();
    ASSERT_EQ(aperture::TypeLimits::APERTURE, draft->type);
    
    // Add points and commit
    m_boundsHandler.AddDraftPoint(aperture::Point{100.0, 100.0});
    m_boundsHandler.AddDraftPoint(aperture::Point{150.0, 150.0});
    m_boundsHandler.CommitDraft();
    
    // Verify shape created as APERTURE
    const auto& shapes = m_apertureCtrls.GetShapes();
    ASSERT_EQ(0, shapes.getInternal().size());
    ASSERT_EQ(0, shapes.getExternal().size());
    ASSERT_EQ(1, shapes.getApertures().size());
}
```

---

## Summary: What Each Step Does

| Step | Purpose | Example |
|------|---------|---------|
| **1. Resource IDs** | Define command constants | `ID_ADD_BOUND_INTERNAL_ELLIPSE = 32811` |
| **2. Handler Declarations** | Declare OnXxx methods | `afx_msg void OnAddBoundInternalEllipse();` |
| **3. Message Map** | Connect resource ID to handler | `ON_COMMAND(ID_ADD_BOUND_INTERNAL_ELLIPSE, OnAddBoundInternalEllipse)` |
| **4. Implementation** | Activate tool + set mode + type | `SetShapeType(INTERNAL); SetEditMode(AddEllipse);` |
| **5. Menu Structure** | Create user-visible menu | "Edit → Bounds → Add Internal → Ellipse" |
| **6. Accelerators** | Optional keyboard shortcuts | Alt+Shift+E for internal ellipse |
| **7. Testing** | Verify end-to-end behavior | Draft commits with correct type |

---

## Key Pattern: Always Set Type BEFORE Mode

```cpp
// ✓ CORRECT
m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);  // First
m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);  // Second

// ❌ WRONG - Type set too late
m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);  // Draft created with default EXTERNAL
m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);  // Too late!
```

Why? Because `SetEditMode()` creates the draft immediately with whatever type is currently set.

---

## Next Steps After Implementation

1. ✅ Add SetShapeType()/GetShapeType() methods - **DONE**
2. ✅ Modify SetEditMode() to use m_shapeType - **DONE**
3. ✅ Create command handlers - **DONE (in this guide)**
4. ⏳ Add menu items to .rc file
5. ⏳ Implement in CImageView.cpp
6. ⏳ Test end-to-end
7. ⏳ Add unit tests

All the architectural pieces are now in place!

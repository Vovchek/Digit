# Implementing Bounds UI Commands: Step-by-Step Guide

## Overview

This guide walks through adding actual UI command handlers for bounds editing. We'll implement the complete flow from menu click to shape creation.

---

## Step 1: Add Resource IDs to .rc File

Edit your main resource file and add these command IDs:

```rc
// Bounds editing - Add External shapes
#define ID_ADD_BOUND_EXTERNAL_RECT      32801
#define ID_ADD_BOUND_EXTERNAL_ELLIPSE   32802
#define ID_ADD_BOUND_EXTERNAL_CIRCLE    32803
#define ID_ADD_BOUND_EXTERNAL_POLYGON   32804

// Bounds editing - Add Internal shapes
#define ID_ADD_BOUND_INTERNAL_RECT      32810
#define ID_ADD_BOUND_INTERNAL_ELLIPSE   32811
#define ID_ADD_BOUND_INTERNAL_CIRCLE    32812
#define ID_ADD_BOUND_INTERNAL_POLYGON   32813

// Bounds editing - Add Aperture shapes
#define ID_ADD_BOUND_APERTURE_RECT      32820
#define ID_ADD_BOUND_APERTURE_ELLIPSE   32821
#define ID_ADD_BOUND_APERTURE_CIRCLE    32822
#define ID_ADD_BOUND_APERTURE_POLYGON   32823

// Bounds editing - Mode commands
#define ID_BOUND_MODE_SELECT            32830
#define ID_BOUND_MODE_DELETE            32831
#define ID_BOUND_CANCEL_DRAFT           32832
```

---

## Step 2: Add Handler Declarations to ImageView.h

Add to `ImageTempl/ImageView.h` in the public section of `CImageView`:

```cpp
public:
    // ========================================================================
    // Bounds Editing Command Handlers (Phase 5)
    // ========================================================================
    
    // Add External shapes
    afx_msg void OnAddBoundExternalRect();
    afx_msg void OnUpdateAddBoundExternalRect(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundExternalEllipse();
    afx_msg void OnUpdateAddBoundExternalEllipse(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundExternalCircle();
    afx_msg void OnUpdateAddBoundExternalCircle(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundExternalPolygon();
    afx_msg void OnUpdateAddBoundExternalPolygon(CCmdUI* pCmdUI);
    
    // Add Internal shapes
    afx_msg void OnAddBoundInternalRect();
    afx_msg void OnUpdateAddBoundInternalRect(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundInternalEllipse();
    afx_msg void OnUpdateAddBoundInternalEllipse(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundInternalCircle();
    afx_msg void OnUpdateAddBoundInternalCircle(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundInternalPolygon();
    afx_msg void OnUpdateAddBoundInternalPolygon(CCmdUI* pCmdUI);
    
    // Add Aperture shapes
    afx_msg void OnAddBoundApertureRect();
    afx_msg void OnUpdateAddBoundApertureRect(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundApertureEllipse();
    afx_msg void OnUpdateAddBoundApertureEllipse(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundApertureCircle();
    afx_msg void OnUpdateAddBoundApertureCircle(CCmdUI* pCmdUI);
    
    afx_msg void OnAddBoundAperturePolygon();
    afx_msg void OnUpdateAddBoundAperturePolygon(CCmdUI* pCmdUI);
    
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
    
    // Bounds editing - Add External shapes
    ON_COMMAND(ID_ADD_BOUND_EXTERNAL_RECT, OnAddBoundExternalRect)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_EXTERNAL_RECT, OnUpdateAddBoundExternalRect)
    
    ON_COMMAND(ID_ADD_BOUND_EXTERNAL_ELLIPSE, OnAddBoundExternalEllipse)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_EXTERNAL_ELLIPSE, OnUpdateAddBoundExternalEllipse)
    
    ON_COMMAND(ID_ADD_BOUND_EXTERNAL_CIRCLE, OnAddBoundExternalCircle)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_EXTERNAL_CIRCLE, OnUpdateAddBoundExternalCircle)
    
    ON_COMMAND(ID_ADD_BOUND_EXTERNAL_POLYGON, OnAddBoundExternalPolygon)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_EXTERNAL_POLYGON, OnUpdateAddBoundExternalPolygon)
    
    // Bounds editing - Add Internal shapes
    ON_COMMAND(ID_ADD_BOUND_INTERNAL_RECT, OnAddBoundInternalRect)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_INTERNAL_RECT, OnUpdateAddBoundInternalRect)
    
    ON_COMMAND(ID_ADD_BOUND_INTERNAL_ELLIPSE, OnAddBoundInternalEllipse)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_INTERNAL_ELLIPSE, OnUpdateAddBoundInternalEllipse)
    
    ON_COMMAND(ID_ADD_BOUND_INTERNAL_CIRCLE, OnAddBoundInternalCircle)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_INTERNAL_CIRCLE, OnUpdateAddBoundInternalCircle)
    
    ON_COMMAND(ID_ADD_BOUND_INTERNAL_POLYGON, OnAddBoundInternalPolygon)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_INTERNAL_POLYGON, OnUpdateAddBoundInternalPolygon)
    
    // Bounds editing - Add Aperture shapes
    ON_COMMAND(ID_ADD_BOUND_APERTURE_RECT, OnAddBoundApertureRect)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_APERTURE_RECT, OnUpdateAddBoundApertureRect)
    
    ON_COMMAND(ID_ADD_BOUND_APERTURE_ELLIPSE, OnAddBoundApertureEllipse)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_APERTURE_ELLIPSE, OnUpdateAddBoundApertureEllipse)
    
    ON_COMMAND(ID_ADD_BOUND_APERTURE_CIRCLE, OnAddBoundApertureCircle)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_APERTURE_CIRCLE, OnUpdateAddBoundApertureCircle)
    
    ON_COMMAND(ID_ADD_BOUND_APERTURE_POLYGON, OnAddBoundAperturePolygon)
    ON_UPDATE_COMMAND_UI(ID_ADD_BOUND_APERTURE_POLYGON, OnUpdateAddBoundAperturePolygon)
    
    // Mode switching
    ON_COMMAND(ID_BOUND_MODE_SELECT, OnBoundModeSelect)
    ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_SELECT, OnUpdateBoundModeSelect)
    
    ON_COMMAND(ID_BOUND_MODE_DELETE, OnBoundModeDelete)
    ON_UPDATE_COMMAND_UI(ID_BOUND_MODE_DELETE, OnUpdateBoundModeDelete)
    
    // ...existing entries...
END_MESSAGE_MAP()
```

---

## Step 4: Implement Command Handlers (DRY Approach)

Use a macro-based approach to reduce repetition:

### Macro Definition (at top of ImageView.cpp)

```cpp
// Helper macro to reduce boilerplate for Add Shape commands
#define IMPLEMENT_ADD_SHAPE_HANDLER(ClassName, MethodName, TypeArg, ModeArg, StatusMsg) \
void CImageView::MethodName() \
{ \
    ActivateBoundsTool(); \
    m_boundsHandler.SetShapeType(aperture::TypeLimits::TypeArg); \
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::ModeArg); \
    GetMainFrame()->SetStatusText(StatusMsg); \
    Invalidate(FALSE); \
} \
\
void CImageView::On##Update##MethodName##(CCmdUI* pCmdUI) \
{ \
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE); \
}
```

### Using the Macro (in ImageView.cpp)

```cpp
// ========================================================================
// Bounds Editing Command Handlers
// ========================================================================

// Add External shapes
void CImageView::OnAddBoundExternalRect()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddRectangle);
    GetMainFrame()->SetStatusText(_T("Click corners to create external rectangular bound"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundExternalRect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundExternalEllipse()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddEllipse);
    GetMainFrame()->SetStatusText(_T("Click to define external elliptical bound"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundExternalEllipse(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundExternalCircle()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddCircle);
    GetMainFrame()->SetStatusText(_T("Click center and edge to define external circular bound"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundExternalCircle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

void CImageView::OnAddBoundExternalPolygon()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::EXTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddPolygon);
    GetMainFrame()->SetStatusText(_T("Click vertices to create external polygonal bound (press Enter when done)"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundExternalPolygon(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

// Add Internal shapes
void CImageView::OnAddBoundInternalRect()
{
    ActivateBoundsTool();
    m_boundsHandler.SetShapeType(aperture::TypeLimits::INTERNAL);
    m_boundsHandler.SetEditMode(DigitMode::ShapeEditMode::AddRectangle);
    GetMainFrame()->SetStatusText(_T("Click corners to create internal rectangular obstruction"));
    Invalidate(FALSE);
}

void CImageView::OnUpdateAddBoundInternalRect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetImageCtrls()->HasImage() ? TRUE : FALSE);
}

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
│  │  ├─ Add &External
│  │  │  ├─ &Rectangle...\tCtrl+Shift+R    [ID_ADD_BOUND_EXTERNAL_RECT]
│  │  │  ├─ &Ellipse...\tCtrl+Shift+E      [ID_ADD_BOUND_EXTERNAL_ELLIPSE]
│  │  │  ├─ &Circle...\tCtrl+Shift+C       [ID_ADD_BOUND_EXTERNAL_CIRCLE]
│  │  │  └─ &Polygon...\tCtrl+Shift+P      [ID_ADD_BOUND_EXTERNAL_POLYGON]
│  │  │
│  │  ├─ Add &Internal
│  │  │  ├─ &Rectangle...\tAlt+Shift+R     [ID_ADD_BOUND_INTERNAL_RECT]
│  │  │  ├─ &Ellipse...\tAlt+Shift+E       [ID_ADD_BOUND_INTERNAL_ELLIPSE]
│  │  │  ├─ &Circle...\tAlt+Shift+C        [ID_ADD_BOUND_INTERNAL_CIRCLE]
│  │  │  └─ &Polygon...\tAlt+Shift+P       [ID_ADD_BOUND_INTERNAL_POLYGON]
│  │  │
│  │  ├─ Add &Aperture
│  │  │  ├─ &Rectangle...\tCtrl+Alt+R      [ID_ADD_BOUND_APERTURE_RECT]
│  │  │  ├─ &Ellipse...\tCtrl+Alt+E        [ID_ADD_BOUND_APERTURE_ELLIPSE]
│  │  │  ├─ &Circle...\tCtrl+Alt+C         [ID_ADD_BOUND_APERTURE_CIRCLE]
│  │  │  └─ &Polygon...\tCtrl+Alt+P        [ID_ADD_BOUND_APERTURE_POLYGON]
│  │  │
│  │  ├─ ─────────────────────
│  │  ├─ &Select Mode (radio)    [ID_BOUND_MODE_SELECT]
│  │  ├─ &Delete Mode (radio)    [ID_BOUND_MODE_DELETE]
```

---

## Step 6: Add Keyboard Accelerators (Optional)

If you want keyboard shortcuts, add to your accelerator table (IDR_MAINFRAME):

```
// Add to accelerator table
CTRL SHIFT R     ID_ADD_BOUND_EXTERNAL_RECT
CTRL SHIFT E     ID_ADD_BOUND_EXTERNAL_ELLIPSE
CTRL SHIFT C     ID_ADD_BOUND_EXTERNAL_CIRCLE
CTRL SHIFT P     ID_ADD_BOUND_EXTERNAL_POLYGON

ALT  SHIFT R     ID_ADD_BOUND_INTERNAL_RECT
ALT  SHIFT E     ID_ADD_BOUND_INTERNAL_ELLIPSE
ALT  SHIFT C     ID_ADD_BOUND_INTERNAL_CIRCLE
ALT  SHIFT P     ID_ADD_BOUND_INTERNAL_POLYGON

CTRL ALT  R      ID_ADD_BOUND_APERTURE_RECT
CTRL ALT  E      ID_ADD_BOUND_APERTURE_ELLIPSE
CTRL ALT  C      ID_ADD_BOUND_APERTURE_CIRCLE
CTRL ALT  P      ID_ADD_BOUND_APERTURE_POLYGON
```

---

## Step 7: Testing the Integration

### Manual Test Case 1: Add Internal Ellipse

```
1. Open image in CImageView
2. Click: Edit → Bounds → Add Internal → Ellipse
   ✓ Status bar shows: "Click to define internal elliptical obstruction"
   ✓ Cursor changes (if implemented)
   ✓ BoundsHandler is activated with:
     - EditMode = AddEllipse
     - ShapeType = INTERNAL

3. Click first point (center)
   ✓ Draft point added
   ✓ Preview shows dashed ellipse outline

4. Click second point (radius)
   ✓ Ellipse updated in preview
   ✓ Ellipse is now fully defined

5. Press Enter or click again
   ✓ Draft committed
   ✓ AddShapeCommand created with type=INTERNAL
   ✓ Shape added to CApertureCtrls internal shapes list
   ✓ Undo/redo available

6. Verify
   ASSERT_TRUE(apertureCtrls->GetShapes().getInternal().size() > 0);
```

### Manual Test Case 2: Switch Modes

```
1. Start with internal ellipse being drafted
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

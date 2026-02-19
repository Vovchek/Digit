# Phase 5: Input Architecture Guide
## CAD-Grade Input Handler Integration

**Author:** GitHub Copilot  
**Date:** 2024  
**Status:** Phase 5 Implementation Reference  
**Purpose:** Complete guide for connecting UI actions to input handlers

---

## Table of Contents

1. [Overall Architecture Flow](#overall-architecture-flow)
2. [Where Code Goes & What It Contains](#where-code-goes--what-it-contains)
3. [UI Command Handlers](#ui-command-handlers)
4. [Tool Activation Methods](#tool-activation-methods)
5. [Input Handlers](#input-handlers)
6. [InputRouter](#inputrouter)
7. [Complete Example](#complete-example)
8. [Data Flow Diagram](#data-flow-diagram)
9. [Key Principles](#key-principles)
10. [Summary Table](#summary-table)

---

## Overall Architecture Flow

The input architecture follows a clean separation of concerns:

```
UI Action (menu/toolbar)
        ↓
    Command Router (MFC)
        ↓
CImageView::ON_COMMAND Handler
        ↓
ActivateFringeTool() / ActivateBoundsTool()
        ↓
InputRouter.SetActiveTool()
        ↓
[Tool becomes active]
        ↓
User Input (mouse/keyboard)
        ↓
CBaseImageView::OnMouse*/OnKey*
        ↓
InputRouter routes to active tool
        ↓
Tool processes input via IInputHandler interface
```

---

## Where Code Goes & What It Contains

### 1. UI Command Handlers (CImageView)

**Location:** `ImageTempl/ImageView.h` (declaration) and `ImageTempl/ImageView.cpp` (implementation)

**Declaration Example:**
```cpp
class CImageView : public CBaseImageView {
    // ... existing code ...

public:
    // Tool activation (already exist)
    void ActivateFringeTool();
    void ActivateBoundsTool();

protected:
    // NEW: Command handlers for UI actions
    afx_msg void OnActivateDrawFringe();      // Menu: "Start Drawing Fringes"
    afx_msg void OnActivateEditBounds();      // Menu: "Edit Bounds"
    afx_msg void OnCancelEditMode();          // Menu: "Cancel" or ESC key
    
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
```

**Implementation Example:**
```cpp
// In ImageView.cpp, add to message map:
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    // ... existing handlers ...
    ON_COMMAND(ID_ACTIVATE_DRAW_FRINGE, OnActivateDrawFringe)
    ON_COMMAND(ID_ACTIVATE_EDIT_BOUNDS, OnActivateEditBounds)
    ON_COMMAND(ID_CANCEL_EDIT_MODE, OnCancelEditMode)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Implementations:
void CImageView::OnActivateDrawFringe()
{
    // Lightweight command handler - just delegates to tool activation
    ActivateFringeTool();
    // Optional: Show status message, update UI state
    GetMainFrame()->SetStatusText(_T("Draw Fringe Mode - Click to add points"));
}

void CImageView::OnActivateEditBounds()
{
    ActivateBoundsTool();
    GetMainFrame()->SetStatusText(_T("Edit Bounds Mode - Adjust apertures"));
}

void CImageView::OnCancelEditMode()
{
    // Cancel current edit mode
    // This broadcasts Cancel() to all handlers via InputRouter
    GetInputRouter().Cancel();
    
    // Could activate a neutral tool or navigation mode
    ActivateFringeTool();  // or some default tool
    
    GetMainFrame()->SetStatusText(_T(""));  // Clear status
}
```

**Key Points:**
- Command handlers are **lightweight** - they just call tool activation methods
- They can update UI state (status bar, button highlights, etc.)
- They should NOT contain input logic
- Command IDs should be defined in `resource.h`

---

### 2. Tool Activation Methods (CImageView)

**Location:** `ImageTempl/ImageView.cpp` (already exists, shown for completeness)

```cpp
void CImageView::ActivateFringeTool()
{
    // Set fringe handler as active tool in InputRouter
    // This is called from ON_COMMAND handlers or directly
    GetInputRouter().SetActiveTool(&m_fringeHandler);
    Invalidate(FALSE);
}

void CImageView::ActivateBoundsTool()
{
    // Set bounds handler as active tool in InputRouter
    GetInputRouter().SetActiveTool(&m_boundsHandler);
    Invalidate(FALSE);
}
```

**Key Points:**
- **Single responsibility:** Switch which tool is active
- **Lightweight:** Delegates to InputRouter
- **Triggers redraw:** Calls `Invalidate(FALSE)` to update display
- **Can be called from multiple places:**
  - FROM: Command handlers (UI menu/toolbar)
  - FROM: Keyboard shortcuts (ESC to cancel, etc.)
  - FROM: Other tools (tool switching within operation)

---

### 3. Input Handlers (DigitMode namespace)

**Location:** `DigitMode/FringeInputHandler.h/cpp` and `DigitMode/BoundsInputHandler.h/cpp`

**What They Contain:**
```cpp
class FringeInputHandler : public IInputHandler {
    // Implements IInputHandler interface:
    // - bool OnMouseDown(UINT flags, CPoint pt) override;
    // - bool OnMouseMove(UINT flags, CPoint pt) override;
    // - bool OnMouseUp(UINT flags, CPoint pt) override;
    // - bool OnMouseWheel(UINT flags, short delta, CPoint pt) override;
    // - bool OnKeyDown(UINT nChar) override;
    // - bool OnKeyUp(UINT nChar) override;
    // - void Cancel() override;
    
    // Internal state management:
    // - EditMode m_mode (Navigate, Draw, DotEdit)
    // - m_inputHandler (underlying domain logic)
    // - Drag tracking
    // - Selection management
};
```

**Key Points:**
- Tools **IMPLEMENT IInputHandler interface** - they provide input event handling
- Tools **MANAGE INTERNAL STATE** - modes, selections, drag tracking
- Tools **DELEGATE TO DOMAIN LOGIC** - BoundsHandler, InputHandler for actual operations
- Tools **RETURN bool** - true = consumed, false = fallback to navigation
- Tools **NEVER override MFC handlers** - only receive routed events from InputRouter
- Tools **DON'T call Invalidate()** - that's the view's responsibility

---

### 4. InputRouter (DigitMode namespace)

**Location:** `DigitMode/InputRouter.h/cpp`

**What It Does:**
```cpp
class InputRouter : public IInputHandler {
    // Central dispatcher
    void SetActiveTool(IInputHandler* pTool);
    void SetNavigationHandler(NavigationInputHandler* pNav);
    
    // Routing logic:
    // bool OnMouseDown(UINT flags, CPoint pt) override
    // {
    //     if (m_pActiveTool && m_pActiveTool->OnMouseDown(flags, pt))
    //         return true;  // Tool consumed event
    //     
    //     if (m_pNavHandler)
    //         return m_pNavHandler->OnMouseDown(flags, pt);  // Try navigation
    //     
    //     return false;  // Event not consumed
    // }
};
```

**Key Points:**
- **Central dispatcher** for all input events
- **Manages two handlers:**
  - **Active Tool** - primary handler (fringe, bounds, etc.)
  - **Navigation Handler** - fallback for pan/zoom/cancel
- **Routing rule:** Tool → Navigation → Consumed
- **Tool switching:** `SetActiveTool()` changes active handler on the fly
- **Universal Cancel:** `Cancel()` broadcasts to both tool and navigation

---

## UI Command Handlers

### Anatomy of a Command Handler

```cpp
// 1. Declare in header
class CImageView : public CBaseImageView {
protected:
    afx_msg void OnMyCommand();
    //}}AFX_MSG
};

// 2. Add to message map
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    ON_COMMAND(ID_MY_COMMAND, OnMyCommand)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// 3. Implement in .cpp
void CImageView::OnMyCommand()
{
    // Option A: Activate a tool
    ActivateFringeTool();
    
    // Option B: Send a command to document
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    if (pDoc)
        pDoc->SendCommand(...);
    
    // Option C: Manage view state
    GetMainFrame()->UpdateUI();
}
```

### Common UI Actions and Their Handlers

| UI Action | Handler | Calls | Purpose |
|-----------|---------|-------|---------|
| Menu: "Draw Fringe" | `OnActivateDrawFringe()` | `ActivateFringeTool()` | Activate fringe editing mode |
| Menu: "Edit Bounds" | `OnActivateEditBounds()` | `ActivateBoundsTool()` | Activate bounds editing mode |
| Menu: "Cancel" / ESC | `OnCancelEditMode()` | `GetInputRouter().Cancel()` | Cancel current operation |
| Toolbar: "Zoom In" | `OnZoomIn()` | `m_viewTransform.ZoomAt(...)` | Zoom at center |
| Toolbar: "Zoom Out" | `OnZoomOut()` | `m_viewTransform.ZoomAt(...)` | Zoom at center |
| Menu: "Undo" | `OnUndo()` | `m_cmdDispatcher.Undo()` | Undo last command |

---

## Tool Activation Methods

### Design Pattern

```cpp
void CImageView::ActivateToolName()
{
    // 1. Set tool as active in InputRouter
    GetInputRouter().SetActiveTool(&m_toolHandler);
    
    // 2. Optional: Update UI state
    GetMainFrame()->SetToolStatusText(_T("Tool Mode Active"));
    
    // 3. Trigger redraw
    Invalidate(FALSE);
}
```

### Why Separate Methods?

- **Single Responsibility** - each method activates one tool
- **Reusable** - called from multiple places (menu, keyboard, other tools)
- **Testable** - can unit test tool switching
- **Clear Intent** - code reads what it does: `ActivateBoundsTool()`

---

## Complete Example: Adding a New Mode

### Scenario
Add a **"Navigate Only" mode** (just pan/zoom, no editing).

### Step 1: Add Command IDs to resource.h

```cpp
#define ID_ACTIVATE_NAVIGATE_MODE  40101
#define ID_ACTIVATE_DRAW_FRINGE    40102
#define ID_ACTIVATE_EDIT_BOUNDS    40103
```

### Step 2: Add Menu Item

In your RC file or menu definition:
```
POPUP "&Edit", ID_EDIT_MENU
{
    MENUITEM "Navigate Mode\tN",     ID_ACTIVATE_NAVIGATE_MODE
    MENUITEM "Draw Fringe\tD",       ID_ACTIVATE_DRAW_FRINGE
    MENUITEM "Edit Bounds\tB",       ID_ACTIVATE_EDIT_BOUNDS
    MENUITEM SEPARATOR
    MENUITEM "Cancel\tESC",          ID_CANCEL_EDIT_MODE
}
```

### Step 3: Declare Handler in CImageView.h

```cpp
class CImageView : public CBaseImageView {
protected:
    afx_msg void OnActivateNavigateMode();
    //}}AFX_MSG
};
```

### Step 4: Add to Message Map

```cpp
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    // ... existing ...
    ON_COMMAND(ID_ACTIVATE_NAVIGATE_MODE, OnActivateNavigateMode)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
```

### Step 5: Implement Handler

```cpp
void CImageView::OnActivateNavigateMode()
{
    // Deactivate all tools - navigation becomes the only active handler
    GetInputRouter().SetActiveTool(nullptr);
    GetMainFrame()->SetStatusText(_T("Navigate Mode - Pan (Space+Drag), Zoom (Ctrl+Wheel)"));
    Invalidate(FALSE);
}
```

---

## Data Flow Diagram

```
┌─────────────────┐
│  UI Layer       │
├─────────────────┤
│ Menu/Toolbar    │
│   (Resource ID) │
└────────┬────────┘
         │ Sends WM_COMMAND
         ↓
┌─────────────────────────────────────┐
│ CImageView (Command Router)         │
├─────────────────────────────────────┤
│ ON_COMMAND(ID_xxx, OnActivateXxx)   │
│                                     │
│ void OnActivateDrawFringe() {       │
│   ActivateFringeTool();             │
│   ShowStatus(...);                  │
│ }                                   │
└────────┬────────────────────────────┘
         │ Calls tool activation
         ↓
┌──────────────────────────────────────────┐
│ Tool Activation (CImageView methods)     │
├──────────────────────────────────────────┤
│ void ActivateFringeTool() {              │
│   GetInputRouter().SetActiveTool(        │
│     &m_fringeHandler                     │
│   );                                     │
│   Invalidate(FALSE);                     │
│ }                                        │
└────────┬─────────────────────────────────┘
         │ Sets active tool
         ↓
┌──────────────────────────────────────────┐
│ InputRouter (Dispatcher)                 │
├──────────────────────────────────────────┤
│ IInputHandler* m_pActiveTool             │
│ NavigationInputHandler* m_pNavHandler    │
│                                          │
│ Routes all input to active tool          │
└────────┬─────────────────────────────────┘
         │ Routes input events
         ↓
┌──────────────────────────────────────────┐
│ Active Tool (IInputHandler)              │
├──────────────────────────────────────────┤
│ FringeInputHandler or BoundsInputHandler │
│                                          │
│ Processes gesture: mouse/keyboard        │
│ Dispatches commands                      │
└──────────────────────────────────────────┘
         │ If not consumed (return false)
         ↓
┌──────────────────────────────────────────┐
│ NavigationInputHandler (Fallback)        │
├──────────────────────────────────────────┤
│ Pan/Zoom/Cancel operations               │
│ Always available (except when deactivated)
└──────────────────────────────────────────┘
```

---

## Key Principles

### 1. Separation of Concerns

- **CImageView:** UI commands & tool switching
- **InputRouter:** Input event routing
- **Tool Handlers:** Input interpretation & domain operations

### 2. One Tool Active at a Time

```cpp
GetInputRouter().SetActiveTool(&m_fringeHandler);
   // m_boundsHandler is now inactive
   // Only m_fringeHandler receives input events
```

### 3. Navigation as Fallback

- If tool returns `false` (didn't consume event), navigation gets a chance
- Users can always pan/zoom even in edit modes
- Ensures app always remains responsive to basic navigation

### 4. State Feedback

- Update status bar after tool activation
- Update UI buttons/checkmarks to show active mode
- Users know which tool is active

### 5. Command Handlers are Thin

```cpp
// GOOD: Delegates to tool activation
void CImageView::OnActivateFringeTool()
{
    ActivateFringeTool();  // Clear, simple
}

// BAD: Duplicates tool logic
void CImageView::OnActivateFringeTool()
{
    GetInputRouter().SetActiveTool(&m_fringeHandler);  // Mixing concerns
    m_fringeHandler.Initialize(...);  // Initialization logic
    m_fringeHandler.SetMode(...);     // State management
}
```

### 6. Tools are Modal

- Tools can have internal modes (Navigate, Draw, DotEdit, etc.)
- Only ONE mode active per tool
- Tool decides how to handle each mode
- Tool returns `false` to delegate to navigation

### 7. Deterministic Input Flow

The order is **ALWAYS:**
1. User presses mouse/keyboard
2. CBaseImageView receives MFC event
3. CBaseImageView calls InputRouter
4. InputRouter checks active tool
5. If tool consumed (return true) → done
6. If tool didn't consume (return false) → try navigation
7. If navigation consumed → done
8. Otherwise → event ignored

---

## Summary Table

| Component | Location | Responsibility | Input/Output |
|-----------|----------|-----------------|--------------|
| **UI Command Handler** | CImageView.cpp | Respond to menu/toolbar clicks | IN: WM_COMMAND<br>OUT: Calls tool activation |
| **Tool Activation** | CImageView.cpp | Switch which tool is active | IN: Command from user<br>OUT: Calls InputRouter.SetActiveTool |
| **InputRouter** | DigitMode/InputRouter.* | Central dispatcher | IN: MFC events<br>OUT: Routes to tool/navigation |
| **Tool Handler** | DigitMode/*InputHandler.* | Process input events | IN: MFC events<br>OUT: Gestures → Commands |
| **Navigation** | DigitMode/NavigationInputHandler.* | Fallback pan/zoom | IN: MFC events<br>OUT: ViewTransform changes |
| **Tool State** | DigitMode/*InputHandler.* | Track edit mode | IN: Gestures<br>OUT: Mode-specific behavior |

---

## Implementation Checklist

When adding a new UI command:

- [ ] Define command ID in `resource.h` (e.g., `ID_ACTIVATE_MYTOOL`)
- [ ] Add menu/toolbar item pointing to that ID
- [ ] Declare `afx_msg void OnActivateMyTool()` in CImageView.h
- [ ] Add `ON_COMMAND(ID_ACTIVATE_MYTOOL, OnActivateMyTool)` to message map
- [ ] Implement handler: `void CImageView::OnActivateMyTool() { ActivateMyTool(); }`
- [ ] Create tool activation method: `void CImageView::ActivateMyTool() { ... }`
- [ ] Update status bar with mode information
- [ ] Test that InputRouter receives the tool correctly
- [ ] Verify tool input methods are called for user gestures

---

## References

- **IInputHandler** (DigitMode/IInputHandler.h) - Interface all tools implement
- **InputRouter** (DigitMode/InputRouter.h) - Central dispatcher
- **CImageView** (ImageTempl/ImageView.h/cpp) - UI command routing
- **CBaseImageView** (ImageTempl/BaseImageView.h/cpp) - MFC event forwarding
- **FringeInputHandler** - Example: fringe editing tool
- **BoundsInputHandler** - Example: bounds editing tool
- **NavigationInputHandler** - Pan/zoom/cancel handler

---

## Questions?

For questions about:
- **UI Integration:** See Section "UI Command Handlers"
- **Tool Architecture:** See Section "Input Handlers"
- **Event Routing:** See Section "Data Flow Diagram"
- **Examples:** See Section "Complete Example"

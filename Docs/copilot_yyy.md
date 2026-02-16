
---

# 🔒 Copilot Instruction — Refactor View / Input Architecture (MANDATORY)

## Goal

Refactor the current MFC view architecture so that:

• All raw mouse/keyboard handling is centralized
• Tool- and mode-specific logic is fully removed from Views
• Input routing is explicit, deterministic, and extensible
• `CBaseImageView` and `CImageView` have strictly separated responsibilities

This refactor is **architectural**, not cosmetic.
Do **not** preserve existing behavior if it violates the rules below.

---

## 1. Absolute Responsibility Split (NON-NEGOTIABLE)

### `CBaseImageView`

`CBaseImageView` is the **only class** that:

• Handles MFC input messages (`OnMouse*`, `OnKey*`)
• Owns and calls `InputRouter`
• Manages mouse capture and invalidation
• Owns the view transform (screen ↔ world)
• Owns the **NavigationInputHandler** (pan / zoom / cancel)

`CBaseImageView` MUST NOT:

❌ Reference bounds, fringes, shapes, commands, selections
❌ Contain any edit-mode or tool-specific logic
❌ Dispatch commands or mutate document data

---

### `CImageView`

`CImageView` is a **domain-specific derived view** that:

• Owns tool input handlers (Bounds, Fringe, etc.)
• Switches active tools (toolbar, menu, hotkeys)
• Implements `OnDraw()` for image, fringes, bounds, overlays

`CImageView` MUST NOT:

❌ Override any `OnMouse*` or `OnKey*` handlers
❌ Call `InputRouter` directly
❌ Contain gesture logic (pan, zoom, drag, cancel)
❌ Interpret mouse buttons or modifier keys

---

## 2. Input Flow (Canonical)

All user input MUST follow this flow:

```
MFC → CBaseImageView → InputRouter → ActiveTool OR NavigationHandler
```

No shortcuts.
No exceptions.

---

## 3. Introduce `IInputHandler`

Create a common interface for all tools and navigation:

```cpp
class IInputHandler {
public:
    virtual bool OnMouseDown(UINT flags, CPoint pt) = 0;
    virtual bool OnMouseMove(UINT flags, CPoint pt) = 0;
    virtual bool OnMouseUp(UINT flags, CPoint pt) = 0;
    virtual bool OnMouseWheel(UINT flags, short delta, CPoint pt) = 0;

    virtual bool OnKeyDown(UINT key) = 0;
    virtual bool OnKeyUp(UINT key) = 0;

    virtual void Cancel() = 0;
    virtual ~IInputHandler() = default;
};
```

Rules:
• Return `true` → event consumed
• Return `false` → fallback allowed

---

## 4. Implement `InputRouter`

Create a central router that delegates events.

```cpp
class InputRouter {
public:
    void SetActiveTool(IInputHandler* tool);
    void SetNavigationHandler(IInputHandler* nav);

    bool OnMouseDown(UINT, CPoint);
    bool OnMouseMove(UINT, CPoint);
    bool OnMouseUp(UINT, CPoint);
    bool OnMouseWheel(UINT, short, CPoint);
    bool OnKeyDown(UINT);
    bool OnKeyUp(UINT);

    void Cancel();

private:
    IInputHandler* m_activeTool = nullptr;
    IInputHandler* m_navigation = nullptr;
};
```

### Routing Rule (MANDATORY)

For **every event**:

1. Try active tool
2. If not consumed → try navigation handler

Example:

```cpp
if (m_activeTool && m_activeTool->OnMouseDown(...))
    return true;
return m_navigation && m_navigation->OnMouseDown(...);
```

---

## 5. NavigationInputHandler

Implement `NavigationInputHandler`:

Responsibilities:
• Pan: `Space + MouseDrag`
• Zoom: `Ctrl + MouseWheel`
• Cancel: `Esc`

Constraints:
• NO domain knowledge
• NO document mutation
• NO commands

---

## 6. Tool Input Handlers

Each editing tool has **exactly one** handler:

Examples:
• `BoundsInputHandler`
• `FringeInputHandler`

Each handler:
• Implements `IInputHandler`
• Internally manages its own modes (Edit / Draw / Navigate, etc.)
• Uses helper classes (`BoundsHandler`, `SelectionManager`)
• Commits changes **only via Commands**

Handlers MUST NOT:
❌ Touch `CImageView`
❌ Call MFC APIs
❌ Handle pan/zoom

---

## 7. Refactor `CBaseImageView`

`CBaseImageView` MUST:

• Own `InputRouter` and `NavigationInputHandler`
• Forward all MFC input events to router
• Manage capture + invalidate

Example:

```cpp
void CBaseImageView::OnLButtonDown(UINT f, CPoint p) {
    SetCapture();
    m_inputRouter.OnMouseDown(f, p);
    Invalidate(FALSE);
}
```

All input handlers MUST live here.

---

## 8. Refactor `CImageView`

`CImageView` MUST:

• Instantiate tool handlers
• Activate tools via `InputRouter::SetActiveTool()`
• Draw content in `OnDraw()`

Example:

```cpp
void CImageView::ActivateBoundsTool() {
    m_inputRouter.SetActiveTool(&m_boundsTool);
}
```

`CImageView` MUST NOT override any MFC input handlers.

---

## 9. Mandatory Cleanup

During refactor:

✔ Remove duplicate mouse/key handlers
✔ Remove mode checks from views
✔ Remove direct document mutation from input code
✔ Centralize all gestures in handlers

---

## 10. Validation Checklist (FAIL if violated)

• `CImageView` contains `OnMouse*` → ❌
• View checks edit modes → ❌
• Tools call `Invalidate()` → ❌
• Commands triggered from View → ❌
• InputRouter duplicated → ❌

---

## Final Intent

This refactor creates a **CAD-grade interaction model**:

• Views are passive
• Tools express intent
• Handlers interpret gestures
• Commands mutate state
• Navigation always works

**Do not compromise this architecture.**

---

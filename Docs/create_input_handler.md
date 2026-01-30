Below is a **COMPLETE, paste-ready Copilot instruction** focused **only on implementing `InputHandler` and integrating it into an MFC `CImageView`**.
It is consistent with:

* UX v1.0 (segment-primary)
* your Command architecture
* your rendering split
* classic MFC Doc/View
* *no* implementation leakage into Commands

This is the “guardrail document” that prevents Copilot from turning `InputHandler` into a god-object.

---

# 📌 COPILOT INSTRUCTION — InputHandler (MFringe)

You are implementing the **InputHandler** class and its integration into an **MFC CImageView**.

The InputHandler is the **interaction brain** of the editor.

---

## 1. Role of InputHandler (AUTHORITATIVE)

**InputHandler is responsible for:**

* Interpreting mouse & keyboard events
* Managing interaction state
* Deciding user intent
* Creating and dispatching Commands
* Maintaining transient interaction state

**InputHandler is NOT responsible for:**

❌ Drawing
❌ Hit-testing implementation
❌ Modifying geometry directly
❌ Undo/redo stacks
❌ Selection storage
❌ Cursor rendering

---

## 2. Ownership Rules (STRICT)

### InputHandler OWNS:

* Current mode:

  * Navigate
  * Draw
  * DotEdit
* Active segment index
* Active end (Head / Tail)
* Hovered object (cached HitResult)
* Drag state:

  * drag start
  * current cursor position
  * drag type (move dot, box select, rubber band)
* Armed multi-step commands (e.g. split awaiting second click)

### InputHandler DOES NOT OWN:

* Geometry → `CDigitInfo`
* Selection → `SelectionManager`
* Hit logic → `HitTester`
* Undo stack → `CommandDispatcher`

---

## 3. Constructor & Dependencies

### InputHandler must be constructed with references:

```cpp
class InputHandler {
public:
    InputHandler(
        CDigitInfo& doc,
        SelectionManager& selection,
        HitTester& hitTester,
        CommandDispatcher& dispatcher
    );
};
```

No globals.
No singletons.

---

## 4. Public API (MANDATORY)

Copilot must implement **at least** the following methods.

```cpp
class InputHandler {
public:
    // Mouse events
    void OnMouseMove(CPoint pt, UINT flags);
    void OnLButtonDown(CPoint pt, UINT flags);
    void OnLButtonUp(CPoint pt, UINT flags);
    void OnRButtonDown(CPoint pt, UINT flags);

    // Keyboard
    void OnKeyDown(UINT key);
    void OnKeyUp(UINT key);

    // Mode control
    void SetMode(EditorMode mode);
    EditorMode GetMode() const;

    // Query for View
    bool HasActiveSegment() const;
    size_t GetActiveSegment() const;
    ActiveEnd GetActiveEnd() const;
    CPoint GetCurrentCursorPos() const;
};
```

---

## 5. InputHandler Internal State (REQUIRED)

```cpp
struct DragState {
    bool active = false;
    DragType type;
    CPoint start;
    CPoint current;
    size_t segmentIndex;
    size_t dotIndex;
};
```

InputHandler must maintain:

* `EditorMode m_mode`
* `std::optional<size_t> m_activeSegment`
* `ActiveEnd m_activeEnd`
* `std::optional<HitResult> m_hover`
* `DragState m_drag`

---

## 6. Event Interpretation Rules

### Mouse Move

```text
OnMouseMove:
  → Update hover via HitTester
  → Update drag state if active
  → Request view repaint
```

NO geometry changes here.

---

### Left Button Down

Behavior depends on mode:

#### Navigate Mode

* Hit dot → begin drag dot
* Hit segment → select segment
* Empty space → begin box select
* Ctrl+hit → toggle selection
* Shift+hit → extend selection
* Hit selected → begin drag selected

#### Draw Mode

* Click empty while active end is None → start new segment with last Number+step
* Click while active end is set → extend segment from active end
* Click segment either end - sets active end → extend segment
* Ctrl+Click other segment end → connect segments, set active free end of other segment
* Ctrl+Click empty or Enter key → end segment, set active end None
* Update active segment & active end
* Rubber band from active end to cursor

#### Dot Edit Mode

* Click dot → begin move dot
* Click edge → insert dot (armed, on mouse up)
* Alt+Click dot → delete dot (command created)

---

### Left Button Up

* If dragging → create appropriate Command
* Dispatch command
* Clear drag state

---

### Right Button Down

* Otherwise → context menu (not implemented here)

---

### Key Down

#### Universal keys

* Esc → cancel current interaction, clear drag
* Del → delete selected objects (command created)

#### Draw Mode

* Backspace → remove last dot (command)
* Enter → finish current segment

#### Navigate Mode

* Tab → cycle selection (handled via SelectionManager)

---

## 7. Command Creation Rules

**InputHandler creates commands but does not execute logic itself.**

Pattern:

```cpp
auto cmd = std::make_unique<MoveDotCommand>(...);
dispatcher.Execute(std::move(cmd));
```

Rules:

* One user gesture → one command
* Drag creates exactly one command on mouse-up
* Multi-segment ops → one compound command (future)

---

## 8. Integration into CImageView (MANDATORY)

### CImageView owns InputHandler

```cpp
class CImageView : public CView {
private:
    std::unique_ptr<InputHandler> m_input;
};
```

### Wiring in OnInitialUpdate

```cpp
m_input = std::make_unique<InputHandler>(
    GetDocument()->DigitInfo,
    m_selectionManager,
    m_hitTester,
    m_commandDispatcher
);
```

---

### Forwarding MFC events (NO LOGIC IN VIEW)

```cpp
void CImageView::OnLButtonDown(UINT flags, CPoint pt) {
    m_input->OnLButtonDown(pt, flags);
    Invalidate();
}

void CImageView::OnMouseMove(UINT flags, CPoint pt) {
    m_input->OnMouseMove(pt, flags);
    Invalidate(FALSE);
}
```

The View:

* forwards events
* invalidates
* draws

Nothing else.

---

## 9. View ↔ InputHandler Contract

The View may **query but never mutate** InputHandler state:

```cpp
if (m_input->HasActiveSegment()) {
    DrawActiveEndMarker(...);
}

if (m_input->IsDragging()) {
    DrawRubberBand(...);
}
```

---

## 10. Forbidden Patterns (Copilot must NEVER generate)

❌ Geometry modification inside InputHandler
❌ Calling `CDigitInfo` mutators directly
❌ Selection storage in InputHandler
❌ Drawing inside InputHandler
❌ MFC message handling inside Commands
❌ Commands created inside View

---

## 11. Mental Model (for Copilot)

> **InputHandler decides what the user means.**
> **Commands decide what changes.**
> **The View only draws.**

---

## END OF INSTRUCTION

---

### What this guarantees

* Clean undo/redo
* Testable interaction logic
* No View bloat
* No Command pollution
* Easy future mode additions


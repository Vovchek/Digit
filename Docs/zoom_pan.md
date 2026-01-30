zoom/pan is exactly the kind of thing that **cuts across all modes**, and if you don’t isolate it properly it will contaminate InputHandler, View, *and* Commands.

Below is the **clean, orthodox way** to incorporate zoom/pan into your existing workflow **without breaking any invariants**.

---

## 1. First Principle (non-negotiable)

> **Zoom and pan are VIEW concerns, not editing concerns.**

They must:

* work in **all modes**
* **never** create Commands
* **never** touch the Document
* **temporarily override** other interactions while active

So they sit **orthogonally** to Draw / Navigate / DotEdit.

---

## 2. Introduce One Missing Actor: ViewTransform (or Camera)

You need a **single owner** of zoom & pan state.

### ViewTransform (authoritative owner)

```cpp
class ViewTransform {
public:
    // State
    double scale;          // zoom factor
    CPoint2d offset;       // pan offset in world coords

    // Operations
    void ZoomAt(const CPoint& screenPt, double factor);
    void PanBy(const CPoint& deltaScreen);

    // Transforms
    CPoint2d ScreenToWorld(const CPoint& pt) const;
    CPoint WorldToScreen(const CPoint2d& pt) const;
};
```

**Owns:**

* zoom level
* pan offset
* coordinate transforms

**Does NOT know about:**

* modes
* selection
* commands
* geometry semantics

---

## 3. Revised High-Level Architecture (extended)

```
MFC View (CImageView)
  |
  | mouse / wheel events
  v
InputHandler
  |
  | intent (edit vs pan vs zoom)
  v
CommandDispatcher → Command → Document

CImageView
  |
  v
ViewTransform
  |
  v
Renderer → CFringeSegment::DrawXXX
```

Key idea:

* **InputHandler decides intent**
* **ViewTransform applies view change**
* **Commands apply model change**

---

## 4. How Zoom/Pan Intercepts Input (Important)

Zoom and pan must be treated as **modal overrides**, not editor modes.

### Priority order for input interpretation

1. Zoom (Ctrl + Wheel)
2. Pan (Space + mouse drag)
3. Current editor mode (Draw / Navigate / DotEdit)

This priority must be **hard-coded**.

---

## 5. Zoom Handling (Ctrl + Mouse Wheel)

### Where it lives

* **Event forwarded to InputHandler**
* InputHandler delegates to **ViewTransform**
* View invalidates
* No Commands

### Flow

```
WM_MOUSEWHEEL
  ↓
CImageView::OnMouseWheel
  ↓
InputHandler::OnMouseWheel
  ↓
ViewTransform::ZoomAt(cursor, factor)
  ↓
Invalidate()
```

### InputHandler rule

```cpp
if (Ctrl is pressed) {
    viewTransform.ZoomAt(cursor, factor);
    return; // DO NOT forward to mode logic
}
```

### Why `ZoomAt(cursor)`?

Because modern CAD editors **zoom around cursor**, not center.

---

## 6. Pan Handling (Space + Mouse Drag)

### Interaction semantics

* Space pressed → enter temporary Pan state
* Left mouse drag → pan view
* Release Space or mouse → exit pan
* Original mode restored

### This is NOT a mode switch.

---

### InputHandler additions

```cpp
bool m_isPanning = false;
CPoint m_lastPanPoint;
```

---

### Mouse Down

```cpp
if (Space is pressed) {
    m_isPanning = true;
    m_lastPanPoint = pt;
    return;
}
```

---

### Mouse Move

```cpp
if (m_isPanning) {
    CPoint delta = pt - m_lastPanPoint;
    viewTransform.PanBy(delta);
    m_lastPanPoint = pt;
    return;
}
```

---

### Mouse Up / Key Up

```cpp
if (Space released || mouse up) {
    m_isPanning = false;
}
```

---

## 7. Critical Rule: Pan/Zoom Must Short-Circuit Editing

When pan or zoom is active:

* NO selection changes
* NO drag operations
* NO commands
* NO active segment changes

That’s why InputHandler must check pan/zoom **before** mode logic.

---

## 8. Coordinate Space Discipline (Very Important)

### Rule

> **All Commands and geometry operate in WORLD coordinates.**

Therefore:

* InputHandler must convert:

  * `screen → world` before creating Commands
* HitTester must work in **world coordinates**
* ViewTransform is the only place that knows about screen space

---

### Pattern

```cpp
CPoint2d worldPt = viewTransform.ScreenToWorld(screenPt);
```

This must happen **before**:

* hit testing
* command creation
* selection logic

---

## 9. View Integration (CImageView)

### OnDraw

```cpp
void CImageView::OnDraw(CDC* pDC)
{
    ApplyViewTransform(pDC); // scale + translate

    DrawDocument(pDC);
    DrawOverlays(pDC);
}
```

### ApplyViewTransform

* Set world→screen transform
* No geometry logic here

---

## 10. Cursor & Feedback During Pan/Zoom

### CursorManager rule

* Space pressed → Pan cursor
* Ctrl + wheel → Zoom cursor
* Otherwise → mode-dependent cursor

CursorManager should query:

* InputHandler::IsPanning()
* Key state

---

## 11. Undo/Redo Interaction

**Zoom and pan are NOT undoable.**

They are view state, not document state.

This is consistent with:

* CAD
* image editors
* scientific tools

---

## 12. Summary Responsibility Table

| Concern            | Owner             |
| ------------------ | ----------------- |
| Zoom level         | ViewTransform     |
| Pan offset         | ViewTransform     |
| Intercept zoom/pan | InputHandler      |
| Geometry edits     | Commands          |
| Undo/Redo          | CommandDispatcher |
| Drawing transform  | CImageView        |

---

## 13. One-Sentence Mental Model

> **Pan and zoom temporarily steal the mouse from the editor,
> but never leave a trace in the document.**

---

## 14. Optional (but recommended) Improvement

Add this to InputHandler API:

```cpp
bool IsPanning() const;
bool IsZooming() const;
```

So View and CursorManager can react cleanly.

---

* sketch **exact MFC handlers** (`OnMouseWheel`, `OnKeyDown`)
* show how to integrate this with your **HitTester**
* design smooth zooming (logarithmic scale)


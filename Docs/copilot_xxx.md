
---

# PART 1 — UPDATED UX / UI SPECIFICATION

**Bounds (Apertures & Obstructions) Editing and Drawing**

## 0. Terminology (locked)

* **Bound** – any shape participating in visibility (aperture / obstruction / external limit)
* **Shape** – geometric primitive (Ellipse, Rectangle, Polygon)
* **Edit mode** – tool state determining what interaction means
* **Handle** – interactive control point (move / resize / rotate / vertex)
* **Draft shape** – shape being created, not yet committed
* **Committed shape** – shape stored in `ShapeCollection` via command

---

## 1. Editing modes (explicit)

Bounds editing is **modal**, orthogonal to fringe editing.

### 1.1 Bounds Edit Modes

| Mode            | Purpose                                    |
| --------------- | ------------------------------------------ |
| `Select/Edit`   | Select existing shapes, move/resize/rotate |
| `Add Circle`    | Special case of ellipse (equal radii)      |
| `Add Ellipse`   | Create ellipse by point sequence           |
| `Add Rectangle` | Create rectangle by point sequence         |
| `Add Polygon`   | Create polygon by vertex clicks            |
| `Delete`        | Remove shape                               |

Each mode is mutually exclusive and reflected in cursor + status bar.

---

## 2. Shape creation by mouse (NEW – missing before)

> **Important:**
> For real apertures, the geometric center is often poorly defined or noisy.
> Therefore:
>
> * No creation mode relies on a single “center click”
> * All curved shapes are defined by **perimeter sampling**
> * Center, axes, and rotation are **derived**, not directly input

### 2.1 Rectangle creation (NEW)

**Input method**: point sequence (no drag-only)

#### Workflow

1. First click → corner A
2. Second click → corner B (defines width direction)
3. Third click → corner C (defines height & orientation)
4. Shape is committed on third click

#### Rules

* Rectangle is **oriented** (rotation inferred)
* If user presses `Esc` → cancel draft
* If user presses `Enter` / right click after 2nd point → axis-aligned rectangle fallback

📌 **Requirement for ApertureCore**

* Rectangle must support construction from **3 points**
* TODO in ApertureCore: implement this constructor

---

### 2.2 Ellipse creation (existing but formalized)

**Input method**: point sequence

#### Workflow

1. First 3 clicks → fit with equal axes (initial guess)
2. Next clicks → fit ellipse (refine axes & rotation)
3. Commit on 'Enter' / right-click

#### Modifiers

* `Esc`: cancel

---

### 2.3 Circle Creation (Explicit, Perimeter-Constrained)

Circle creation is **explicitly chosen by tool**, not inferred.

**Workflow**

1. User clicks **N ≥ 3 points on perimeter**
2. Points are collected identically to ellipse mode
3. LSM **circle fit** is performed (single radius constraint)
4. Shape committed on `Enter` / right-click

**Key Rule**

> Circle and ellipse are **never auto-distinguished**
> The selected tool determines the constraint model.

---

> ❌ **Do NOT infer circle vs ellipse from fit residuals**
> ❌ **Do NOT auto-switch shape type**
> Tool selection is authoritative.

---

### 2.4 Polygon creation

#### Workflow

1. Each click adds a vertex
2. Preview edge follows cursor
3. Right click or `Enter` closes polygon
4. Polygon is committed

#### Constraints

* Minimum 3 vertices
* Self-intersection not allowed (error feedback)

---

## 3. Visual appearance (MAJOR missing piece)

### 3.1 Color & style by **TypeLimits**

| Type       | Outline     | Fill             | Notes        |
| ---------- | ----------- | ---------------- | ------------ |
| `EXTERNAL` | Solid green | none             | Global limit |
| `APERTURE` | Solid cyan  | none             | Visible area |
| `INTERNAL` | Dashed red  | semi-transparent | Obstruction  |

Fill is **never interactive** (no hit-testing).

---

### 3.2 Appearance by editing state

| State               | Outline         | Handles                   |
| ------------------- | --------------- | ------------------------- |
| Idle (not selected) | Thin            | Hidden                    |
| Hovered             | Thicker         | Hidden                    |
| Selected            | Thick           | Visible                   |
| Dragging            | Thick + preview | Active handle highlighted |
| Draft               | Dashed          | Vertex dots only          |

---

## 4. Handles (reconfirmed & extended)

### 4.1 Handle visibility rules

* Handles shown **only for selected shape**
* Handles are always drawn **above** shapes
* Hit-testing priority: **handles > outline > nothing**

---

### 4.2 Handle sets per shape

#### Rectangle

* Move handle: center
* Rotate handle: above top edge
* Corner resize: 4
* Edge resize: 4

#### Ellipse / Circle

* Move handle: center
* Rotate handle: normal to major axis
* Axis resize: 4 (2 major, 2 minor)

> For ellipses and circles created by fitting:
>
> * Handles operate on **derived parameters** (center, axes, rotation)
> * Dragging a handle updates parameters, **not original sample points**
> * Original perimeter samples are discarded after commit

This keeps editing intuitive without re-introducing raw points.

#### Polygon

* Move handle: centroid
* Vertex handles: all vertices
* Edge-midpoint handles: optional (v2+)

---

### 4.3 Handle interaction rules

* Drag move → translate shape
* Drag resize → geometry update
* Drag rotate → rotate around center
* `Shift`:

  * Rectangle: keep aspect ratio
  * Ellipse: circle
  * Rotation: angle snapping (15°)
* `Alt`:

  * Resize from center

---

## 5. Drawing order in `OnDraw`

1. Bitmap
2. Fringes
3. Bounds (all shapes)
4. Draft shape (if any)
5. Selected shape handles
6. Tooltips / overlays

---

## 6. Selection & overlapping shapes

* Clicking **outline only** selects shape
* Interior is ignored (by design)
* If multiple outlines overlap:

  * Cycle selection on repeated clicks
* Handles always disambiguate selection

---

## 7. Mask & visibility feedback (UI)

Optional (v2):

* Hover pixel visibility preview (cursor color)
* Mask debug overlay (toggle)

---

# PART 2 — DETAILED IMPLEMENTATION PLAN (FOR COPILOT)

Below is **verbatim-style guidance** you can paste or adapt.

---

## A. Core responsibilities (DO NOT MIX)

* **ApertureCore**

  * Geometry
  * Visibility logic
  * No UI, no MFC

* **CApertureCtrls**

  * Owns ShapeCollection
  * Owns VisibilityMaskProvider
  * Entry point for visibility queries

* **BoundsHandler**

  * UI interaction state machine
  * Draft shapes
  * Handle enumeration & dragging
  * Emits Commands only

---

## B. Shape creation pipeline (NEW)

### 1. Add DraftShape subsystem (BoundsHandler)

```cpp
struct DraftShape {
    enum class Kind { Rectangle, Ellipse, Circle, Polygon };
    Kind kind;
    std::vector<aperture::Point> perimeterPoints; // ordered clicks
};
```

**Clarification**

* `perimeterPoints` is used for:

  * rectangle → corners
  * ellipse / circle → LSM fit
  * polygon → vertices

No separate center / axis storage.

---

BoundsHandler owns:

```cpp
std::optional<DraftShape> m_draft;
```

---

### 2. Mouse input rules (BoundsHandler)

* `OnLButtonDown`:

  * If Add-mode → append point to draft
  * If Select-mode → hit-test handles/shapes

* `OnMouseMove`:

  * Update preview geometry
  * Redraw

* `OnLButtonUp`:

  * Only relevant for drag handles

* `OnKeyDown`:
* `OnRButtonUp`:

  * `Esc` → cancel draft / cancel drag
  * `Enter` → finalize polygon / rectangle fallback

---

### 3. Draft → Shape conversion

On commit:

* Convert `DraftShape` → `aperture::Shape`
* Emit `AddShapeCommand`
* Clear draft

---

## C. Rectangle 3-point constructor (ApertureCore TODO)

Copilot task:

> Implement Rectangle constructor from three points:
>
> * p0, p1 define first axis
> * p2 defines orthogonal extent
> * Compute center, width, height, rotation

---

## D. Commands (STRICT)

* All edits go through Commands:

  * `AddShapeCommand`
  * `RemoveShapeCommand`
  * `ReplaceShapeCommand`

* BoundsHandler:

  * NEVER modifies ShapeCollection directly
  * Uses snapshot-before / snapshot-after

---

## E. Drawing implementation

### ❌ What is explicitly forbidden

```cpp
// ❌ NOT ALLOWED
class Shape {
    void Draw(Graphics&, DrawStyle);
};
```

**Reason:**
`ApertureCore` must remain:

* UI-free
* MFC-free
* rendering-agnostic

---

## ✅ Correct Design: External Shape Renderer

### 1️⃣ Introduce a rendering adapter layer (UI-side)

**NEW interface (UI / Digit / View layer):**

```cpp
class IShapeRenderer {
public:
    virtual ~IShapeRenderer() = default;

    virtual void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style
    ) = 0;
};
```

* Lives **outside ApertureCore**
* Knows about:

  * MFC (`CDC`)
  * pens, brushes, colors
  * selection / hover / editing state

---

### 2️⃣ Concrete renderer per shape type

```cpp
class RectangleRenderer : public IShapeRenderer;
class EllipseRenderer   : public IShapeRenderer;
class PolygonRenderer   : public IShapeRenderer;
```

Each renderer:

* pulls **pure geometry** via `Shape`’s existing query methods
* converts to screen space
* draws outlines, fills, handles (if requested)

---

### 3️⃣ Central dispatch (no RTTI abuse)

**In Digit / View layer:**

```cpp
class ShapeDrawDispatcher {
public:
    void Draw(
        const aperture::Shape& shape,
        CDC& dc,
        const ShapeDrawStyle& style
    );
};
```

Implementation:

* Uses `shape.GetKind()` or visitor-style dispatch
* Selects correct renderer
* Calls renderer.Draw(...)

---

### 4️⃣ ShapeDrawStyle (pure UI concern)

```cpp
struct ShapeDrawStyle {
    bool selected = false;
    bool hovered = false;
    bool editing = false;
    bool showHandles = false;
    aperture::TypeLimits type;
};
```

* Fully decoupled from geometry
* Computed by `BoundsHandler / SelectionManager`
* Passed **down**, never stored in shape

---

### 5️⃣ View usage (CImageView::OnDraw)

```cpp
for (const auto& shape : apertureCtrls.GetShapes()) {
    ShapeDrawStyle style = ComputeStyle(shape);
    m_shapeDrawDispatcher.Draw(*shape, dc, style);
}
```

---

## 🧠 Mental Model (important for Copilot)

> **Shapes describe geometry**
> **Renderers describe appearance**
> **Handlers describe interaction**

No class does more than one.

---

## 🚫 Explicit Copilot Guardrails (add verbatim)

Add this comment to spec:

> Shapes must **never**:
>
> * include drawing APIs
> * reference CDC / Graphics / pens / colors
> * know selection or hover state
>
> Drawing is exclusively handled by UI-layer renderers.

---

### ✅ Outcome

* ApertureCore stays pure
* MFC stays localized
* Drawing styles are flexible
* Handles, previews, and legacy visuals coexist cleanly
* Copilot stops trying to “helpfully” add Draw() to Shape


---

## F. Visibility mask integration (recap)

* `CApertureCtrls` owns:

  ```cpp
  VisibilityMaskProvider m_maskProvider;
  ```
* On any shape command execute/undo:

  ```cpp
  m_apertureCtrls.NotifyShapeModified();
  ```
* Mask rebuilt lazily

---

## G. What Copilot must NOT do

❌ Store handles inside shapes
❌ Let shapes know about screen or mouse
❌ Let CApertureCtrls depend on CImageCtrls
❌ Allow direct container mutation outside commands

---

## H. Validation checklist (give to Copilot)

* Rectangle can be created by 3 clicks
* Circle and Ellipse creation are diferent modes
* Handles appear only on selection
* Draft shapes never enter ShapeCollection
* Undo/Redo works for all edits
* Mask invalidates on any edit or image change

---

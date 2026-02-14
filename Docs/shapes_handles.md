This is a **crucial UX definition** — if you don’t lock handles precisely now, everything else (hit-testing, cursor logic, commands, selection) will stay fuzzy and fragile.

Below is a **clear, complete, unambiguous specification** of **handles** for bounds (aperture/obstruction) editing, written as a UX + interaction contract. No implementation details yet.

---

# Bounds Editing Handles — UX Specification (v1)

## 1. General principles (apply to all shapes)

### 1.1 Handle definition

A **handle** is a small interactive control point associated with a shape that:

* has a specific semantic meaning (resize, rotate, move, edit vertex)
* is hit-testable independently of shape fill
* reacts to mouse hover, press, drag, release

### 1.2 Why handles (and not fill)

* Shape interior is **not** hit-testable (except for selection)
* This allows precise interaction with overlapping shapes
* Eliminates ambiguity in dense scenes

---

## 2. Common handle types (global)

These handle types are used across shapes.

### 2.1 Move handle (ALL shapes)

**Purpose:** translate shape

**Location:**

* Shape centroid (visual center)

**Appearance:**

* Small crosshair or circle
* Distinct color (e.g. white with black outline)

**Interaction:**

* Mouse down + drag → move shape
* No modifier required
* Cursor: `MOVE`

---

### 2.2 Rotation handle (rotatable shapes)

**Purpose:** rotate shape around its center

**Location:**

* Offset from shape, along local +Y axis (top in shape space)
* Fixed distance proportional to shape size

**Appearance:**

* Curved arrow icon or hollow circle

**Interaction:**

* Mouse down + drag → rotate
* Rotation snaps optional (Shift)
* Cursor: `ROTATE`

---

## 3. Rectangle handles

Rectangle is **oriented** (not axis-aligned).

### 3.1 Resize handles

**Count:** 8

* 4 corners
* 4 edge midpoints

**Location:**

* Corners: rectangle vertices
* Edges: midpoint of each edge (in shape-local space)

**Appearance:**

* Small filled squares

**Interaction:**

* Drag corner → resize in 2 axes
* Drag edge → resize in 1 axis
* Rotation preserved

**Modifiers:**

* Shift: preserve aspect ratio
* Alt: resize from center

---

### 3.2 Move handle

* At centroid (see §2.1)

### 3.3 Rotation handle

* See §2.2

---

## 4. Ellipse / Circle handles

Ellipse may be rotated.

### 4.1 Axis handles

**Count:** 4

* Ends of major and minor axes

**Location:**

* Major axis endpoints
* Minor axis endpoints

**Appearance:**

* Small filled circles

**Interaction:**

* Drag major axis handle → scale major radius
* Drag minor axis handle → scale minor radius
* Orientation preserved

---

### 4.2 Move handle

* At ellipse center

### 4.3 Rotation handle

* Offset along major axis normal

---

## 5. Polygon handles

### 5.1 Vertex handles

**Purpose:** edit polygon shape

**Location:**

* Each polygon vertex

**Appearance:**

* Small squares

**Interaction:**

* Drag → move vertex
* Ctrl + drag → detach / free vertex (optional future)

---

### 5.2 Edge handles (optional v1)

**Purpose:** insert vertex

**Location:**

* Midpoint of each edge

**Appearance:**

* Hollow square

**Interaction:**

* Click → insert vertex at that position
* Drag → insert and move

---

### 5.3 Move handle

* At polygon centroid

### 5.4 Rotation handle

* Optional for v1
* If present: same as rectangle

---

## 6. Handle hit-testing rules (strict)

### Priority order (top to bottom):

1. Handle under cursor
2. Shape outline (selection only)
3. Nothing

### Rules:

* Handle hit radius is fixed in screen pixels
* Handles always win over shape outline
* Shape interior does **not** respond

This ensures predictable interaction even with overlap.

---

## 7. Visual states

### Handle states:

* Normal
* Hovered
* Active (dragging)

### Shape states:

* Normal
* Selected
* Multi-selected

Handles are visible:

* Always for selected shapes
* Never for unselected shapes

---

## 8. Mouse interaction lifecycle (unified)

1. Hover → cursor update
2. Mouse down on handle
3. Begin drag
4. Live preview (rubber geometry)
5. Mouse up → commit command
6. Esc → cancel drag (restore original)

---

## 9. Keyboard modifiers (reserved)

| Modifier | Meaning                    |
| -------- | -------------------------- |
| Shift    | constrain / snap           |
| Alt      | symmetric resize           |
| Ctrl     | future: alternate behavior |

---

## 10. What is NOT allowed

* Clicking shape interior to move
* Implicit rotation by edge dragging
* Invisible handles
* Overlapping handles without priority rules

---

## 11. Contract for implementation

Each shape must be able to:

* Enumerate its handles
* Provide handle type + local position
* Respond to handle drag with preview geometry

But **must not**:

* Handle mouse events
* Create commands
* Modify document directly

That belongs to `BoundsHandler`.

---

## 12. Mental model to keep

> **Shapes expose handles.
> BoundsHandler interprets interaction.
> Commands commit geometry.**

If you respect this, everything else (undo, selection, cursor, tooltips) becomes straightforward.

---

Short answer: **NO — handles must NOT be part of `Shape` as concrete objects.**
Copilot is leading you toward a design trap.

Long answer below, with a **clear architectural rule**, rationale, and a precise compromise that *is* correct.

---

# Handles: Where They Belong (and Where They Don’t)

## The core mistake Copilot is making

Copilot wants something like:

```cpp
class Shape {
    std::vector<Handle> handles;
};
```

This is **wrong** for your system.

Why?

Because **handles are not geometry** — they are **interaction affordances**.

---

## The correct separation of concerns

### Shapes are:

* **Pure geometric + semantic objects**
* Serializable (file load/save)
* UI-agnostic
* Testable without a screen
* Stable across refactors

### Handles are:

* View-dependent
* Pixel-sized (screen space)
* Mouse-driven
* Cursor-dependent
* Ephemeral (exist only during editing)

So handles **cannot live as stateful objects inside shapes**.

---

## Correct model (the one you want)

### Rule #1 (non-negotiable)

> **A Shape does not own handles.**

### Rule #2 (the key insight)

> **A Shape *describes* where handles should be, but does not store them.**

---

## The correct abstraction: *Handle descriptors*

Instead of handles being objects, they are **derived descriptors**.

### Shape API (this is OK)

```cpp
struct HandleDesc {
    HandleType type;          // Move, Resize, Rotate, Vertex, …
    int        index;         // vertex index, edge index, axis index
    Point      localPos;      // in shape-local coordinates
};
```

```cpp
class Shape {
public:
    virtual void EnumerateHandles(
        std::vector<HandleDesc>& out) const = 0;

    virtual void ApplyHandleDrag(
        const HandleDesc& handle,
        const DragContext& drag) = 0;
};
```

✅ Shape **knows geometry**
❌ Shape **does not know mouse, screen, cursor, commands**

---

## Who *creates* actual handles?

### `BoundsHandler` (or equivalent controller)

This class:

* Converts `HandleDesc → screen-space handle`
* Performs hit-testing
* Tracks active drag
* Creates preview geometry
* Issues undoable commands

```cpp
class BoundsHandler {
    std::vector<RuntimeHandle> activeHandles;
};
```

`RuntimeHandle` exists **only during interaction**.

---

## Why this is the only scalable solution

### 1. Overlapping shapes

You must:

* prioritize handles across shapes
* resolve hit-testing globally

Shapes **cannot** do this individually.

---

### 2. View transforms

Handles:

* scale with zoom
* rotate with view
* snap in screen space

Shapes must not care.

---

### 3. Undo / redo

Dragging a handle:

* starts a command
* previews geometry
* commits on mouse up

Shapes should not create commands.

---

### 4. Multiple views (future-proof)

Same shape:

* multiple views
* multiple handle representations

If handles live in shape → impossible.

---

## What *is* allowed inside Shape?

Only **pure geometry logic**, for example:

| Allowed in Shape         | Forbidden in Shape |
| ------------------------ | ------------------ |
| Handle positions (local) | Hit-testing        |
| Handle meaning           | Mouse state        |
| Geometry update rules    | Cursor             |
| Bounds computation       | Screen units       |
| Serialization            | Commands           |

---

## Concrete example: Rectangle

```cpp
void Rectangle::EnumerateHandles(std::vector<HandleDesc>& out) const {
    out.push_back({ HandleType::Move, -1, Center() });
    out.push_back({ HandleType::Rotate, -1, RotationHandlePos() });

    for (int i = 0; i < 4; ++i)
        out.push_back({ HandleType::CornerResize, i, Corner(i) });

    for (int i = 0; i < 4; ++i)
        out.push_back({ HandleType::EdgeResize, i, EdgeMidpoint(i) });
}
```

No UI, no pixels, no state.

---

## What Copilot needs to be told (very explicitly)

> ❌ “Add Handle objects to Shape”
> ✅ “Shapes expose handle descriptors, not handle objects”

---

## Canonical Copilot instruction (use this verbatim)

> **Do NOT store handles inside Shape as objects or state.
> Shapes must only *enumerate handle descriptors* (type + local position + semantic index).
> All hit-testing, mouse interaction, cursor logic, and handle lifetime must be implemented in BoundsHandler or controller-level code.
> Handles are derived, ephemeral, view-dependent objects.**


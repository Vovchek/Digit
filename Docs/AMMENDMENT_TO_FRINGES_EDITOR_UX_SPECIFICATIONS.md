Below is a **rewritten UX wording**, fully consistent with **Option A′ (segment-primary, fringe as logical grouping by Number)**, and aligned with your UX v1.0 intent.
No implementation hints, only **user-facing semantics and rules**.

---

# Digit UX v1.0 — **Segment-Primary Model**

## Core Concepts (Terminology — FIXED)

**Dot**
A control point with a 2D position.

**Edge**
A straight line between two consecutive dots in a segment.

**Segment** *(primary editable object)*
A continuous polyline defined by an ordered list of dots.
Segments are independent geometric objects.

**Number**
A numeric value representing constant fringe height.

**Fringe** *(logical grouping)*
A fringe is **the set of all segments sharing the same Number**.
A fringe has **no independent identity** beyond its Number.

> Selecting or editing a “fringe” always means operating on **all segments with the same Number**.

---

## Selection Model

### Selection Targets

The editor supports selecting:

* Dots
* Edges
* Segments
* Fringes (via segment groups)

Selection is **explicit and persistent**.
No implicit promotion or demotion occurs.

---

### Selection Hierarchy

```
Dot → Edge → Segment → Fringe (group of segments by Number)
```

Each level must be selected deliberately.

---

### Selection Rules

* **Dot selection** selects only that dot.
* **Edge selection** selects only that edge.
* **Segment selection** selects the entire segment.
* **Fringe selection** selects **all segments with the same Number**.

Selections may contain **mixed object types**.

---

### Additive Selection

Modifiers allow adding objects to the current selection without clearing it.

---

### Box (Rectangular) Selection

**Inclusion rules**:

* **Dot**: included if inside the box.
* **Edge**: included if fully inside or intersecting the box.
* **Segment**: included only if *all* its edges are included
  (unless Segment-select modifier is used).
* **Fringe**: included only via explicit Fringe-select command.

---

## Modes

### Navigate Mode (default)

Purpose:

* Selection control
* Multi-object operations
* Dragging and deletion

Capabilities:

* Select dots, edges, segments, fringes
* Box-select multiple objects
* Drag selected objects
* Delete selected objects

No geometry creation occurs in this mode.

---

### Draw Mode

Purpose:

* Create and extend segments

Behavior:

* Clicking in empty space starts a **new segment**

  * The new segment receives a Number = last Number + step
* Clicking an end dot of a segment continues that segment
* Connecting to another segment merges geometry

  * The **free end of the second segment becomes the active end**
* Drawing proceeds from the active end with a rubber-band preview
* Drawing can be ended explicitly (mouse or key)

Draw Mode never modifies existing dots except by extension or connection.

---

### Dot Edit Mode

Purpose:

* Local geometric editing

Capabilities:

* Move dots
* Insert dots on edges
* Delete dots or edges

Restrictions:

* No creation of new segments
* No Number changes
* No selection of entire fringes

Edits affect geometry only, not semantic grouping.

---

## Active End

When a segment is being edited in Draw Mode:

* One end is designated as the **active end**
* New dots are added only at the active end
* Connecting to another segment:

  * The connected segment’s **free end becomes the new active end**

Active end is always explicit and visually indicated.

---

## Fringe (Number) Operations

Fringe operations are **set-based commands** acting on selection.

Examples:

* Select Fringe → selects all segments with the same Number
* Delete Fringe → deletes all segments with the same Number
* Renumber Fringe → assigns a new Number to all selected segments
* Auto-number → assigns Numbers sequentially to selected segments

There is **no partial fringe selection**.

---

## Number Context

* Current Number is visible in the UI
* Increment/decrement commands change Number on selection
* Renumbering a segment immediately moves it to another fringe

---

## Tooltips

Tooltips reflect **logical meaning**, not storage structure.

### Dot Tooltip

```
#2.5 / 1(3) / 12
```

Meaning:

* Fringe Number: 2.5
* Segment index: 1 of 3 segments in this fringe
* Dot index: 12 within the segment

Trailing indices are omitted where irrelevant.

---

## Visual Feedback

* Selected dots, edges, and segments are highlighted
* Segments in the same fringe share a visual style
* Active segment end is clearly marked
* Rubber-band preview shown during drawing and connecting

---

## Undo / Redo

* One user intent = one undo step
* Fringe-level commands undo as a grouped operation
* Geometry edits undo locally and predictably

---

## Conceptual Guarantees (UX Invariants)

* Segments are the only geometric owners
* Fringes are logical, Number-based groupings
* Renumbering changes grouping, not geometry
* No hidden topology reconstruction occurs

---

### One-sentence mental model (for users)

> *You draw and edit **segments**;
> segments with the same **Number** together form a **fringe***.

---

If you want, next we can:

* tighten wording further into a **UX reference card**
* reconcile this wording with **tool icons & cursor states**
* derive a **formal invariant list** for refactoring safety
* map this wording back to your original command table

Just say where to go next.

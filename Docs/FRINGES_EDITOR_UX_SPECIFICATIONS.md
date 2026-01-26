
---

# Digit/MFringe UX Specification — **Frozen v1.0**

This version is considered **baseline-stable** for implementation and iteration.

---

## Amendment Applied (4.3 – Clarified)

### 4.3 Connecting Curves (Final)

* **Click with Connect modifier on an end of another curve**

  * Current curve connects to the target curve
  * **The free (non-connected) end of the second curve becomes the new active end**
  * Drawing continues seamlessly from that free end

This ensures:

* predictable continuation
* no “dead-end” after connection
* consistent mental model: *you always continue from the open end*

---

## 1. Canonical Terminology (Frozen)

| Term   | Meaning                                                     |
| ------ | ----------------------------------------------------------- |
| Dot    | Single control point                                        |
| Edge   | Straight line between two adjacent dots                     |
| Curve  | Continuous ordered polyline                                 |
| Fringe | Logical group of one or more curves sharing the same Number |
| Number | Constant height value assigned to a fringe                  |

The word **“segment” is not used** in UX.

---

## 2. Modes (Frozen Set)

Only modes that *fundamentally change mouse behavior* exist.

| Mode     | Purpose                                   |
| -------- | ----------------------------------------- |
| Navigate | Selection, move, delete, multi-object ops |
| Draw     | Create / continue / connect curves        |
| Dot Edit | Fine geometry editing (dots & edges)      |

All other operations are **commands**, not modes.

---

## 3. Selection Model (Frozen)

### Selection Levels

* Dot
* Edge
* Curve
* Fringe
* Other object (aperture, obstruction, zap…)

Selection is:

* explicit
* persistent
* hierarchical
* modifier-driven

---

## 4. Box (Rectangular) Selection — Frozen Rules

### Default (No Modifier)

| Entity        | Included if…                   |
| ------------- | ------------------------------ |
| Dot           | Inside box                     |
| Edge          | Intersects **or** fully inside |
| Curve         | **All edges included**         |
| Fringe        | Never                          |
| Other objects | Bounding-box based             |

---

### Curve-Select Modifier

* Curve included if **any part intersects box**

---

### Fringe-Select Modifier

* Entire fringe selected if **any curve intersects box**
* Multiple fringes allowed

---

## 5. Draw Mode (Frozen)

### Start / End Drawing

* **Left-click empty space**

  * Starts new curve
  * New fringe created
  * Number increases by `NumberStep`

* **Right-click** (mouse counterpart of Enter)

  * Ends current curve
  * Remains in Draw mode

---

### Continue Existing Curve

* **Left-click on either end of a curve**

  * Activates that curve
  * Clicked end becomes active
  * Drawing continues

---

### Connect Curves (Final)

* **Connect modifier + click curve end**

  * Curves are connected
  * **Free end of second curve becomes active**
  * Drawing continues from that end

---

### Delete While Drawing

| Action                  | Effect                |
| ----------------------- | --------------------- |
| Backspace               | Remove last added dot |
| Mouse + Delete modifier | Remove arbitrary dot  |

---

## 6. Dot Edit Mode (Frozen)

Mouse-first editing:

* Move dots
* Insert dot on edge
* Delete dots
* Drag edge (moves its dots)

No creation, no numbering.

---

## 7. Navigate Mode (Frozen)

* Full selection control
* Box select
* Drag selection
* Delete selection
* Selection promotion/demotion

All command-based edits are issued from here.

---

## 8. Fringe Edit Commands (Frozen)

One-shot commands operating on selection:

* Number + / –
* Auto-number (from selection)
* Simplify
* Subdivide
* Split curve
* Merge curves

Always undoable.

---

## 9. Tooltips (Frozen)

### Dot Tooltip

```
#2.5 / 1(1) / 12
```

* Fringe Number = 2.5
* Curve index = 1
* Total curves in fringe = 1
* Dot index = 12

---

### Other Entities

Only relevant identifiers shown:

* `Curve – 34 dots`
* `Fringe #2.5 (3 curves)`
* `Aperture`
* `Obstruction`

---

## 10. Cursor Semantics (Frozen)

Cursor always reflects **click outcome**, updated by mode + modifiers.

---

# Minimal Command / Key Table (UX v1.0)

This table is intentionally **small, memorable, and non-overloaded**.

---

## Global (All Modes)

| Key / Mouse     | Action                               |
| --------------- | ------------------------------------ |
| Esc             | Cancel / exit mode                   |
| Del             | Delete selection                     |
| Ctrl+Z / Ctrl+Y | Undo / Redo                          |
| + / -           | Increase / decrease Number           |
| Tab             | Cycle selection                      |
| Right-click     | Contextual end / cancel (mode-aware) |

---

## Selection & Navigation

| Input                  | Action                      |
| ---------------------- | --------------------------- |
| Left-click             | Select                      |
| Ctrl+Click             | Add / toggle selection      |
| Shift+Click            | Range select (dots / edges) |
| Drag empty space       | Box select                  |
| Ctrl+Drag              | Add box selection           |
| Curve-select modifier  | Promote to curve            |
| Fringe-select modifier | Promote to fringe           |

---

## Draw Mode

| Input                    | Action          |
| ------------------------ | --------------- |
| Left-click empty         | Start new curve |
| Left-click curve end     | Continue curve  |
| Connect modifier + click | Connect curves  |
| Right-click / Enter      | End curve       |
| Backspace                | Remove last dot |
| Delete modifier + click  | Remove dot      |

---

## Dot Edit Mode

| Input      | Action               |
| ---------- | -------------------- |
| Drag dot   | Move dot             |
| Click edge | Insert dot           |
| Drag edge  | Move edge            |
| Del        | Delete selected dots |

---

## Fringe Edit Commands (Navigate Mode)

| Key             | Action           |
| --------------- | ---------------- |
| F               | Select fringe    |
| Ctrl++ / Ctrl+- | Auto-number step |
| S               | Simplify         |
| Shift+S         | Subdivide        |
| M               | Merge curves     |
| X               | Split curve      |

(Exact letters adjustable, semantics frozen.)

---

## Final Status

✅ Terminology frozen
✅ Modes minimized and clear
✅ Mouse & keyboard balanced
✅ Draw workflow continuous and predictable
✅ Selection rules precise
✅ Ready for implementation & testing

---

Great, this is the right sequencing. Below is a **clean UX-level spec pass** that:

1. **freezes Ctrl / Shift / Alt semantics** (no ambiguity, CAD-consistent)
2. defines a **minimal, coherent icon / tool palette**

Still **spec only**, no implementation detail.

---

# Modifier Assignment — **Frozen Mapping (UX v1.0)**

## Design Principles

Modifiers must be:

* **Globally consistent** (same meaning everywhere)
* **Mnemonic** (Ctrl = add/control, Shift = range/constrain, Alt = alternate intent)
* **Non-overlapping** (each modifier has a primary role)

This follows CAD / vector editor norms.

---


## 1. Modifier Semantics (Global Meaning)

### Ctrl — *Add / Extend / Secondary*

> “Do more of the same”

Used for:

* Add to selection
* Toggle selection
* Add objects to an existing context
* Fine / precise control

---

### Shift — *Range / Constraint / Promote*

> “Extend or constrain what’s already selected”

Used for:

* Range selection
* Axis / direction constraints
* Promotion to higher-level entity

---

### Alt — *Alternate / Structural*

> “Do something different than the default”

Used for:

* Structural operations
* Special selection scope
* Destructive or topology-changing intent

---

## 2. Modifier Usage by Interaction

### 2.1 Selection

| Input       | Result                                |
| ----------- | ------------------------------------- |
| Click       | Select single                         |
| Ctrl+Click  | Add / toggle selection                |
| Shift+Click | Range select (dots / edges)           |
| Drag        | Box select                            |
| Ctrl+Drag   | Add box selection                     |
| Shift+Drag  | Constrain box (axis-aligned emphasis) |

---

### 2.2 Selection Promotion

| Input             | Result                           |
| ----------------- | -------------------------------- |
| Click curve       | Select curve                     |
| Shift+Click curve | Promote to curve (from dot/edge) |
| Alt+Click curve   | Promote to fringe                |
| Alt+Drag box      | Box-select fringes               |

Rationale:

* **Shift = bigger within same structure**
* **Alt = semantic jump (curve → fringe)**

---

### 2.3 Draw Mode

| Input                | Action               |
| -------------------- | -------------------- |
| Left-click empty     | Start new curve      |
| Left-click curve end | Continue curve       |
| Ctrl+Click curve end | **Connect curves**   |
| Right-click          | End curve            |
| Backspace            | Remove last dot      |
| Alt+Click dot        | Delete arbitrary dot |

Here:

* Ctrl = add/connect
* Alt = destructive alternate

---

### 2.4 Dot Edit Mode

| Input          | Action             |
| -------------- | ------------------ |
| Drag dot       | Move dot           |
| Shift+Drag dot | Constrain movement |
| Click edge     | Insert dot         |
| Alt+Click dot  | Delete dot         |
| Drag edge      | Move edge          |

---

### 2.5 Navigate Mode

| Input          | Action                          |
| -------------- | ------------------------------- |
| Drag selection | Move selection                  |
| Shift+Drag     | Constrain movement              |
| Alt+Drag       | Duplicate (reserved for future) |
| Del            | Delete selection                |

---

### 2.6 Number / Fringe Commands

| Input           | Action                      |
| --------------- | --------------------------- |
| + / -           | Change Number               |
| Ctrl++ / Ctrl+- | Auto-number with step       |
| Alt++ / Alt+-   | Reserved (future batch ops) |

---

## 3. Modifier Summary Table (One-Glance)

| Modifier | Meaning                | Typical Use               |
| -------- | ---------------------- | ------------------------- |
| Ctrl     | Add / Extend           | Add selection, connect    |
| Shift    | Range / Constrain      | Range select, axis lock   |
| Alt      | Structural / Alternate | Fringe select, delete dot |

This mapping is **frozen** unless a conflict is discovered in real use.

---

# Icon / Tool Palette Design — **UX v1.0**

## Design Principles

* Tools represent **modes**, not commands
* Commands are **buttons or menu actions**
* Icons must be **recognizable at 16–24 px**
* Scientific > decorative

---

## 4. Tool Palette Structure

### 4.1 Primary Tool Bar (Modes)

Persistent, mutually exclusive.

| Icon       | Tool     | Notes             |
| ---------- | -------- | ----------------- |
| 🡆 (Arrow) | Navigate | Default           |
| ✏️●        | Draw     | Pen with dot      |
| ●↔●        | Dot Edit | Vertex/edge focus |

Only one active at a time.

---

### 4.2 Secondary Tool Bar (Commands)

Momentary actions (do not latch).

#### Fringe / Curve Commands

| Icon    | Command       |
| ------- | ------------- |
| #       | Select Fringe |
| +# / -# | Change Number |
| ⇄       | Merge Curves  |
| ✂️      | Split Curve   |

---

#### Geometry Operations

| Icon | Command   |
| ---- | --------- |
| ≈    | Simplify  |
| ⋯    | Subdivide |

---

### 4.3 Visibility Rules

* Disabled if selection incompatible
* Tooltip always explains:

  * action
  * shortcut
  * scope (Dot / Curve / Fringe)

Example tooltip:

> **Simplify Curve**
> Applies to selected curves
> Shortcut: `S`

---

## 5. Icon Language (Visual Consistency)

### Shapes

* Dot → filled circle
* Edge → short line
* Curve → polyline
* Fringe → stacked polylines
* Number → `#`

### Styling

* Monochrome
* No text inside icons
* Color reflects state (hover / active)

---

## 6. Cursor vs Icon Relationship

* **Mode icon** defines cursor base
* **Modifier** alters cursor overlay

  * `+` for Ctrl
  * `↕` / `↔` for Shift
  * `!` or broken shape for Alt

This keeps feedback immediate.

---

## 7. Final Sanity Check (UX v1.0)

✔ Ctrl / Shift / Alt meanings are consistent
✔ No modifier does two unrelated things
✔ Modes are few and obvious
✔ Icons represent mental model, not code
✔ Keyboard and mouse workflows both complete

---


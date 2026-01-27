---

# Digit/MFringe UX Specification — **Frozen v1.0 (Segment-Primary)**

This version is considered **baseline-stable** for implementation and iteration.

**Model**: Segment-primary architecture. Fringes are logical groupings, not containers.

---

## Amendment Applied (4.3 – Clarified)

### 4.3 Connecting Segments (Final)

* **Click with Connect modifier on an end of another segment**

  * Current segment connects to the target segment
  * **The free (non-connected) end of the second segment becomes the new active end**
  * Drawing continues seamlessly from that free end

This ensures:

* predictable continuation
* no "dead-end" after connection
* consistent mental model: *you always continue from the open end*

---

## 1. Canonical Terminology (Frozen)

| Term    | Meaning                                                        |
| ------- | -------------------------------------------------------------- |
| Dot     | Single control point                                           |
| Edge    | Straight line between two adjacent dots                        |
| Segment | Continuous ordered polyline (primary geometric object)         |
| Number  | Constant height value assigned to a segment                    |
| Fringe  | Logical grouping of all segments sharing the same Number value |

**Key Principle**: Segments exist. Fringes are computed from segments by Number.

---

## 2. Modes (Frozen Set)

Only modes that *fundamentally change mouse behavior* exist.

| Mode     | Purpose                                      |
| -------- | -------------------------------------------- |
| Navigate | Selection, move, delete, multi-object ops    |
| Draw     | Create / continue / connect segments         |
| Dot Edit | Fine geometry editing (dots & edges)         |

All other operations are **commands**, not modes.

---

## 3. Selection Model (Frozen)

### Selection Levels

* Dot
* Edge
* Segment
* Fringe (set of segments with same Number)
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
| Segment       | **All edges included**         |
| Fringe        | Never                          |
| Other objects | Bounding-box based             |

---

### Segment-Select Modifier

* Segment included if **any part intersects box**

---

### Fringe-Select Modifier

* Entire fringe selected if **any segment intersects box**
* Multiple fringes allowed
* Selects **all segments with the same Number**

---

## 5. Draw Mode (Frozen)

### Start / End Drawing

* **Left-click empty space**

  * Starts new segment
  * Segment receives Number = last Number + step
  * New fringe membership (if Number is unique)

* **Right-click** (mouse counterpart of Enter)

  * Ends current segment
  * Remains in Draw mode

---

### Continue Existing Segment

* **Left-click on either end of a segment**

  * Activates that segment
  * Clicked end becomes active
  * Drawing continues

---

### Connect Segments (Final)

* **Connect modifier + click segment end**

  * Segments are connected
  * **Free end of second segment becomes active**
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

* Number + / – (renumber segments, changes fringe membership)
* Auto-number (from selection)
* Simplify
* Subdivide
* Split segment
* Merge segments

Always undoable.

**Note**: Renumbering a segment changes which fringe it belongs to.

---

## 9. Tooltips (Frozen)

### Dot Tooltip

```
#2.5 / 1(3) / 12
```

* Fringe Number = 2.5
* Segment index = 1 of 3 segments in this fringe
* Dot index = 12 within this segment

---

### Other Entities

Only relevant identifiers shown:

* `Segment – 34 dots`
* `Fringe #2.5 (3 segments)`
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

| Input                    | Action                      |
| ------------------------ | --------------------------- |
| Left-click               | Select                      |
| Ctrl+Click               | Add / toggle selection      |
| Shift+Click              | Range select (dots / edges) |
| Drag empty space         | Box select                  |
| Ctrl+Drag                | Add box selection           |
| Segment-select modifier  | Promote to segment          |
| Fringe-select modifier   | Promote to fringe           |

---

## Draw Mode

| Input                    | Action            |
| ------------------------ | ----------------- |
| Left-click empty         | Start new segment |
| Left-click segment end   | Continue segment  |
| Connect modifier + click | Connect segments  |
| Right-click / Enter      | End segment       |
| Backspace                | Remove last dot   |
| Delete modifier + click  | Remove dot        |

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

| Key             | Action            |
| --------------- | ----------------- |
| F               | Select fringe     |
| Ctrl++ / Ctrl+- | Auto-number step  |
| S               | Simplify          |
| Shift+S         | Subdivide         |
| M               | Merge segments    |
| X               | Split segment     |

(Exact letters adjustable, semantics frozen.)

---

## Final Status

✅ Terminology frozen (segment-primary)
✅ Modes minimized and clear
✅ Mouse & keyboard balanced
✅ Draw workflow continuous and predictable
✅ Selection rules precise
✅ Ready for implementation & testing

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

| Input               | Result                             |
| ------------------- | ---------------------------------- |
| Click segment       | Select segment                     |
| Shift+Click segment | Promote to segment (from dot/edge) |
| Alt+Click segment   | Promote to fringe                  |
| Alt+Drag box        | Box-select fringes                 |

Rationale:

* **Shift = bigger within same structure**
* **Alt = semantic jump (segment → fringe)**

---

### 2.3 Draw Mode

| Input                  | Action                 |
| ---------------------- | ---------------------- |
| Left-click empty       | Start new segment      |
| Left-click segment end | Continue segment       |
| Ctrl+Click segment end | **Connect segments**   |
| Right-click            | End segment            |
| Backspace              | Remove last dot        |
| Alt+Click dot          | Delete arbitrary dot   |

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
  * scope (Dot / Segment / Fringe)

Example tooltip:

> **Simplify Segment**
> Applies to selected segments
> Shortcut: `S`

---

## 5. Icon Language (Visual Consistency)

### Shapes

* Dot → filled circle
* Edge → short line
* Segment → polyline
* Fringe → stacked polylines (multiple segments)
* Number → `#`

### Styling

* Color reflects fringe number (applicable for dots, edges, segments, fringes) or denotes object type (apertures, obstructions)
* No text inside icons
* Color accent (brightness, glow) reflects state (hover / active)

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

Perfect — I’ll execute **steps 1–4 fully**, still staying at **UX/spec level**, and keeping everything consistent with frozen **UX v1.0**.

---

# UX v1.0 — Validation & Refinement Pass

## Step 1 — Conflict Audit (Modifiers, Modes, Gestures)

Goal: identify **collisions, ambiguities, or overloads** and resolve them explicitly.

### 1.1 Modifier Conflicts Review

| Context            | Potential Conflict                 | Resolution                                                                                                      |
| ------------------ | ---------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| Ctrl in Draw       | Add-to-selection vs Connect curves | **Resolved**: In Draw mode, Ctrl is contextually *Connect* when clicking curve end; selection is locked in Draw |
| Shift in Selection | Range select vs Promote            | **Resolved**: Shift+Click promotes only when click target is curve body; on dot/edge it remains range           |
| Alt in Selection   | Fringe-select vs Delete dot        | **Resolved**: Alt+Click deletes dot only in Draw/Dot Edit; in Navigate it promotes to Fringe                    |
| Right-click        | Context menu vs End drawing        | **Resolved**: In Draw mode, right-click = End drawing; elsewhere = context menu                                 |

### 1.2 Mode Boundary Conflicts

| Case                    | Risk                            | UX Rule                                                      |
| ----------------------- | ------------------------------- | ------------------------------------------------------------ |
| Selection while drawing | Accidental deselection          | Selection is locked during Draw                              |
| Delete in Draw          | Remove whole curve accidentally | Del deletes only last dot; full delete requires leaving Draw |
| Box select in Draw      | Unclear meaning                 | Box select disabled in Draw                                  |

**Conclusion:**
No unresolved conflicts. Mode-scoped meanings are acceptable and standard in CAD tools.

---

## Step 2 — Icon Sketch Sheet (Conceptual, Not Visual Assets)

Below are **semantic icon definitions**, sufficient for designers or Copilot later.

### 2.1 Mode Icons (Latchable)

| Tool     | Icon Concept     | Visual Meaning       |
| -------- | ---------------- | -------------------- |
| Navigate | Arrow cursor     | Selection & movement |
| Draw     | Polyline + dot   | Construct curve      |
| Dot Edit | Dot with handles | Edit geometry        |

---

### 2.2 Command Icons (Momentary)

#### Selection / Structure

| Command           | Icon Concept       |
| ----------------- | ------------------ |
| Select Fringe     | Stacked polylines  |
| Promote to Curve  | Polyline highlight |
| Promote to Fringe | Polyline → stack   |

---

#### Geometry Operations

| Command      | Icon Concept          |
| ------------ | --------------------- |
| Split Segment  | Scissors cutting line |
| Merge segments | Two lines joining     |
| Simplify     | Smooth wave           |
| Subdivide    | Dots added on line    |

---

#### Number Operations

| Command     | Icon Concept |
| ----------- | ------------ |
| Number +    | `# +`        |
| Number −    | `# −`        |
| Auto-number | `# → #`      |

---

### 2.3 Icon Rules (Frozen)

* No text inside icons
* Stroke-based, not filled shapes
* Same icon language across modes & commands
* Color indicates state, not meaning

---

## Step 3 — User Muscle-Memory Walkthrough (10-Minute Scenario)

This is a **realistic first-session flow** to validate UX learnability.

---

### Minute 0–1: Orientation

* Default tool = Navigate
* Cursor shows selection intent
* Hover tooltips explain entities clearly

User intuition: “This behaves like CAD / vector editor”

---

### Minute 1–4: First Fringe Digitization

1. Select **Draw tool**
2. Left-click empty space → first dot appears
3. Rubber-band visible
4. Continue clicking to add dots
5. Right-click → curve ends
6. New left-click → next fringe starts automatically (number increments)

✔ No keyboard required
✔ No modal confusion

---

### Minute 4–6: Refinement

1. Switch to **Dot Edit**
2. Drag dot → curve updates live
3. Click edge → insert dot
4. Alt+Click dot → remove mistake

✔ Direct manipulation, no commands needed

---

### Minute 6–8: Connection

1. Back to **Draw**
2. Click end of curve A
3. Ctrl+Click end of curve B → connect
4. Drawing continues from free end of B

✔ Mental model holds: “always continue from open end”

---

### Minute 8–10: Global Editing

1. Navigate mode
2. Box-select multiple curves
3. Alt+Click one → promote to Fringe
4. Press `+` → change number
5. Press `S` → simplify

✔ Power-user flow achievable quickly

---

### Walkthrough Verdict

* Discoverable without manual
* No dead ends
* No hidden state surprises
* Keyboard accelerators feel natural

---

## Step 4 — Context Menu Specification (Often Overlooked, Now Fixed)

Context menus are **selection-sensitive** and **mode-aware**.

---

### 4.1 Empty Space

* Navigate:

  * Start Draw
  * Select All
* Draw:

  * End Segment
  * Cancel Draw

---

### 4.2 Dot Context Menu

* Delete Dot
* Insert Dot Before / After
* Select Segment
* Select Fringe

---

### 4.3 Edge Context Menu

* Insert Dot
* Split Segment Here
* Select Segment
* Simplify Segment

---

### 4.4 Segment Context Menu

* Select Segment
* Select Fringe
* Split Segment
* Simplify
* Subdivide

---

### 4.5 Fringe Context Menu

* Select Fringe (selects all segments with same Number)
* Change Number…
* Auto-number
* Delete Fringe (deletes all segments with this Number)

---

### 4.6 Mixed Selection

* Delete Selection
* Change Number (if applicable)
* Clear Selection

Menu never shows invalid actions.

---

## Final Validation Summary

✔ Modifiers conflict-free
✔ Icons semantically consistent
✔ First-time workflow smooth
✔ Context menus complete & safe
✔ UX v1.0 is **internally consistent and CAD-grade**

---

Excellent — I’ll deliver **all four artifacts** I proposed, cleanly and concisely, as **UX v1.0 handoff material**.
These are meant to be *authoritative*, readable by humans **and** usable by Copilot / developers.

---

# 1. UX v1.0 — One-Page Cheat Sheet

## Modes

| Mode     | Purpose                                   |
| -------- | ----------------------------------------- |
| Navigate | Select, move, delete, multi-object ops    |
| Draw     | Create, continue, connect segments        |
| Dot Edit | Fine geometry editing                     |

---

## Objects

| Level   | Meaning                                 |
| ------- | --------------------------------------- |
| Dot     | Control point                           |
| Edge    | Line between dots                       |
| Segment | Continuous polyline (primary object)    |
| Fringe  | Logical group of segments with same Number |
| Other   | Apertures, obstructions, zap            |

---

## Core Rules

* Selection is persistent
* One mode active at a time
* Commands act on selection
* Esc always cancels
* **Segments exist, fringes are computed**

---

## Modifiers

| Modifier | Meaning                |
| -------- | ---------------------- |
| Ctrl     | Add / Connect          |
| Shift    | Range / Constrain      |
| Alt      | Structural / Alternate |

---

## Essential Keys

| Key        | Action           |
| ---------- | ---------------- |
| Esc        | Cancel           |
| Del        | Delete selection |
| + / -      | Change Number    |
| Ctrl+Z / Y | Undo / Redo      |
| Tab        | Cycle selection  |

---

## Draw Mode Quick Flow

1. Left-click empty → start segment (Number increments)
2. Left-click end → continue segment
3. Ctrl+Click other segment end → connect
4. Right-click → end segment

---

# 2. Event → Command → Undo Mapping

This ensures **undo predictability**.

| User Action      | Command       | Undo Scope          |
| ---------------- | ------------- | ------------------- |
| Add dot          | AddDot        | Single step         |
| Remove last dot  | RemoveDot     | Single step         |
| Insert dot       | InsertDot     | Single step         |
| Move dot(s)      | MoveGeometry  | One drag = one undo |
| Split segment    | SplitSegment  | Atomic              |
| Merge segments   | MergeSegments | Atomic              |
| Change Number    | Renumber      | Atomic              |
| Simplify         | Simplify      | Atomic              |
| Subdivide        | Subdivide     | Atomic              |
| Delete selection | DeleteObjects | Atomic              |

**Rule:**
One user intent = one undo step.

---

# 3. Selection State Diagram (Textual)

```
[None]
  |
  v
[Dot] ----Shift----> [Dot Range]
  |                     |
  | Promote             | Promote
  v                     v
[Edge] ------------> [Segment]
                          |
                       Alt Promote
                          v
                    [Fringe (all segments with same Number)]
```

### Notes

* Promotion never loses information
* Demotion is explicit (Alt or command)
* Mixed selection allowed only via Ctrl
* Fringe selection = selecting all segments with matching Number

---

# 4. Formal UX v1.0 Acceptance Checklist

This is what implementation must satisfy before sign-off.

## Selection

* [ ] Dot, Edge, Segment, Fringe selectable
* [ ] Box selection obeys inclusion rules
* [ ] Ctrl adds, Shift ranges, Alt promotes
* [ ] Selection persists across modes

## Draw Mode

* [ ] Start new segment on empty click
* [ ] Continue from segment ends
* [ ] Connect segments with Ctrl
* [ ] Free end becomes active after connect
* [ ] Right-click ends segment
* [ ] Backspace removes last dot

## Dot Edit

* [ ] Move dots with drag
* [ ] Insert dot on edge
* [ ] Delete dot with Alt+Click
* [ ] Edge drag moves its dots

## Navigate

* [ ] Multi-select drag
* [ ] Multi-delete
* [ ] Mixed object selection safe

## Commands

* [ ] Number +/- works on selection (changes fringe membership)
* [ ] Auto-number respects order
* [ ] Simplify / Subdivide work on segments
* [ ] Merge / Split atomic & undoable

## Feedback

* [ ] Cursor reflects action
* [ ] Tooltips follow spec
* [ ] Icons reflect modes/commands
* [ ] Invalid actions disabled

---

# Status

✅ UX v1.0 **fully specified** (segment-primary model)
✅ Ready for:

* Copilot instruction derivation
* UI mockups
* Implementation planning
* Test-case writing

---

## Copilot System Prompt — **Digit/MFringe UX v1.0 (Segment-Primary)**

You are working on **Digit**, a scientific CAD-like editor for interferogram fringes.

Follow **UX v1.0 strictly** (segment-primary model):

• **Terminology is fixed**: Dot (point), Edge (between dots), Segment (continuous polyline), Fringe (logical grouping of segments with same Number). 
• **Segments are primary objects**. Fringes are computed groupings, not containers.
• **Modes are limited**: Navigate, Draw, Dot Edit. Only these change mouse behavior.
• **Selection is persistent and hierarchical**: Dot → Edge → Segment → Fringe. No implicit promotion.
• **Modifiers are global**:
Ctrl = add / connect
Shift = range / constrain
Alt = structural / alternate
• **Draw mode**:
– Left-click empty → start new segment (Number += step)
– Left-click segment end → continue segment
– Ctrl+Click other segment end → connect; **free end becomes active**
– Right-click ends current segment
– Backspace removes last dot
• **Dot Edit mode**: move/insert/delete dots and edges only; no creation or numbering.
• **Navigate mode**: full selection, box select, multi-object drag/delete.
• **Box selection rules**:
– Edge if intersects or inside
– Segment only if *all* edges included (unless Segment-select modifier)
– Fringe only via Fringe-select modifier (selects all segments with same Number)
• **Commands (not modes)** act on selection: Number +/−, auto-number, simplify, subdivide, split, merge.
• **One user intent = one undo step.**
• **Renumbering a segment changes which fringe it belongs to.**
• **Never store fringes as containers. Compute them on-demand from segment Number values.**

If behavior is ambiguous, choose predictability over cleverness and follow CAD conventions.

---


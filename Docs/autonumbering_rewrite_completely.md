---

# Copilot Instruction

## Rewrite Automatic Fringe Numbering Algorithm (Topology-First)

### Context

We are implementing **automatic numbering of isolines (fringes)** represented as `std::vector<CFringeSegment>`.
Each `CFringeSegment` represents a **single continuous isoline segment**.
Fringes with equal `Number` represent the same isoline level.

This algorithm must be **robust**, **topology-driven**, and **stable with minimal anchors**, especially for:

* band-like isolines
* nested / concentric rings
* smooth surfaces with local extrema

This is **not** a generic least-squares fitting problem.
Least-squares (if used at all) is a *post-processing refinement*, never the primary solver.

---

## Fundamental Principles (DO NOT VIOLATE)

1. **Topology first, geometry second**

   * Adjacency and ordering define numbering
   * Geometry only helps infer *direction*, never replaces topology

2. **No magic numbers**

   * No arbitrary penalties, epsilons, or weights
   * Only explicit constraints: topology, step, anchors
   * exceptions:     
       * closed segments detection
       * gradient/deravative or like division-demanding calculation

3. **Directed constraints, not symmetric ones**

   * Never generate constraints like `|k_i − k_j| ≈ step`
   * Always encode **direction**:
     `k_j = k_i + step` or `k_j = k_i − step`

4. **Equal numbers only if topology demands it**

   * Two segments may share equal `k` **only if**:

     * they meet at a saddle / junction (>2 neighbors), or
     * equality is explicitly provided as input
   * Default adjacency implies `±step`, not `0`

5. **Coordinate-system invariant**

   * Window coordinates may be left-handed or right-handed
   * Never rely on sign of X/Y
   * Orientation is inferred from *relative ordering*, not absolute axes

---

## Inputs

```cpp
std::vector<CFringeSegment>& fringes;   // mutable: Number will be modified
std::vector<size_t> trustedFringes;     // optional anchors
double step;                            // numbering step
AutoNumberingParams params;             // includes orientation hint
```

If `trustedFringes` is empty:

* Automatically select **one anchor**
* Choose the fringe whose **centroid is closest to the reference origin**
* Reference origin depends on UX mode (left-to-right, top-down), but:
  **never assume axis direction sign**

---

## High-Level Algorithm Phases

### Phase 0 — Preprocessing

* Compute for each segment:

  * centroid
  * bounding box
  * approximate normal / tangent (optional)
* Build a spatial index if needed (no assumptions about density)

---

### Phase 1 — Build Topological Adjacency Graph

Construct a graph:

```cpp
struct Edge {
    size_t from;
    size_t to;
    int delta;   // +1 or -1 or 0 (rare, topology-forced)
};
```

Rules:

* Two segments are adjacent if they are **neighbors in space**
  (distance threshold or intersection logic already implemented)
* For each adjacent pair:

  * Determine **relative ordering**, not numeric difference
  * Decide if:

    * one must be above the other (`±step`)
    * or they are topologically equal (`0`, rare)

⚠️ If ordering cannot be determined reliably:

* **Do not create a constraint**
* Leave it unresolved for propagation

---

### Phase 1.5 TOPOLOGY CLASSIFICATION   ← SEE AMENDMENTS BELOW

---

### Phase 2 — Assign Direction to Adjacency (Critical)

For each adjacent pair `(A, B)`:

Determine sign:

* Use one or more:

  * nesting (inside/outside for rings)
  * monotonic band ordering
  * user-provided orientation hint
* Result must be **consistent across the connected component**

Output:

```cpp
Edge { A, B, +1 }  // means k[B] = k[A] + step
```

If direction is ambiguous:

* Do not guess
* Skip the edge

---

### Phase 3 — Deterministic Propagation Solver (Primary Solver)

This replaces the broken LS-first approach.

Algorithm:

1. Initialize `k[i] = undefined` for all fringes
2. Initialize queue with `trustedFringes`
3. For trusted fringes:

   * keep their existing Number values
4. While queue not empty:

   * pop fringe `i`
   * for each outgoing edge `(i → j)`:

     * proposed = `k[i] + edge.delta * step`
     * if `k[j] undefined`:

       * assign `k[j] = proposed`
       * mark j trusted
       * push j
     * else:

       * if conflict detected:

         * record inconsistency (do not overwrite)

Properties:

* Works with **1 anchor** for chains
* Works with **2 anchors** for orientation
* Cannot collapse numbers
* Produces integer-step-consistent values

---

### Phase 4 — Component Completion (Optional Anchoring)

If a connected component has **no anchors**:

* Select a root:

  * fringe whose centroid is closest to reference origin
* Assign:

  ```
  k[root] = 0
  ```
* Propagate within that component

This keeps components internally consistent without global bias.

---

### Phase 5 — Optional Least-Squares Refinement (Safe Mode)

Only after propagation:

* Build LS system using:

  ```
  k_j − k_i = delta * step
  ```
* All propagated fringes are **hard-fixed**
* Only refine untrusted or weakly constrained nodes

LS is allowed to:

* smooth noise
* resolve small inconsistencies

LS is **not allowed** to:

* decide direction
* collapse levels
* override topology

---

### Phase 6 — Quantization & Validation

* Snap results to nearest multiples of `step`
* Verify all constraints are satisfied
* Produce:

  ```cpp
  std::vector<size_t> newTrustedFringes;
  ```

Trusted means:

* value is stable
* used as anchor in subsequent runs

---

## Output

* Modified `fringes[i].Number`
* Returned vector of trusted fringe indices
* Optional diagnostics:

  * conflicts
  * unresolved adjacencies
  * weak topology regions

---

## Explicit Anti-Patterns (Copilot: DO NOT DO THIS)

❌ Do not:

* Solve numbering purely via least squares
* Use penalty-based anchoring
* Assume coordinate axis direction
* Infer equality unless topology forces it
* Use arbitrary thresholds or weights

---

## Mental Model (Important)

Think of this as:

> **Phase unwrapping on a graph**, not curve fitting.

---

# Important ammendments

👉 the algorithm must explicitly distinguish between BAND / RING / SADDLE topologies**
👉 This classification must happen **before** constraint generation
👉 **Only one of them may be global**; others are local modifiers

This is not overengineering — it is *how interferometric phase unwrapping works in practice*.

---

## Why this is necessary

The observation is key:

* **Band case**
  Introduced intentionally (carrier)
  Dominant, global, directional
  → numbering should be monotonic

* **Ring case**
  Caused by local extrema
  May be global (other variant of carrier) or local
  → numbering propagates monotonicaly to center

* **Saddle case**
  Always local
  Multiple adjacency conflicts
  → equality or branching constraints

Trying to solve all three with a single constraint rule is **mathematically underdetermined**.

So yes — we need **topology classification** as an explicit step.

---

## Revised Architecture (Top-Level)

Insert a new phase **before** constraint generation:

```
Phase 0  Geometry extraction
Phase 1  Adjacency graph
Phase 1.5 TOPOLOGY CLASSIFICATION   ← NEW
Phase 2  Directed constraint generation
Phase 3  Deterministic propagation
Phase 4  Optional refinement
```

---

## Phase 1.5 — Topology Classification

We classify **per connected component**, not globally.

Each component gets:

```cpp
enum class FringeTopology {
    Band,
    Ring,
    Mixed,   // band + local rings
    SaddleDominated,
    Unknown
};
```

---

## How to Detect Each Case (Practical & Robust)

### 1️⃣ Band Case (Global, Dominant)

**Definition**
A component is a *band* if there exists a **dominant direction** along which fringes are ordered monotonically.

**Detection criteria (use several, not just one):**

* Graph is mostly **chain-like**
* Most nodes have degree ≤ 2
* Projection of centroids onto some axis produces:

  * nearly monotonic ordering
* Adjacency graph has **no small cycles**

**Resulting rule:**

```cpp
∀ adjacency (i, j): k[j] = k[i] ± step
(sign determined by global direction)
```

This is your **carrier case**.

---

### 2️⃣ Ring Case (Nested Cycles)

**Definition**
Component contains **closed isolines nested inside each other**.

**Detection criteria:**

* Many segments are **closed**
* Adjacency graph contains **cycles**
* Centroids form concentric ordering
* One fringe encloses another spatially

**Important clarification (answers your earlier question):**

> **Yes — “closed / circular” property *is used here***
> But:

* Closure is **geometric**, not just `first == last point`
* Use **area / winding / containment**, not flags

**Resulting rule:**

* Choose a **center**
* Number increases or decreases radially
* One anchor fixes sign

---

### 3️⃣ Saddle Case (Local)

**Definition**
Local region where **>2 adjacency directions conflict**.

**Detection criteria:**

* Node degree ≥ 3
* Adjacent edges disagree on direction
* No single monotonic ordering possible locally

**Crucial rule:**

> Saddles **never define global numbering**

They only impose **local equalities or branching constraints**.

**Resulting rule:**

```cpp
abs(k[A] - k[B]) <= 1   // ONLY here
```

or “do not constrain”.

---

## Mixed Case (Realistic Surfaces)

This is what you *actually* have most of the time:

* One **global band**
* Local **ring islands**
* Local **saddles**

Strategy:

1. Detect dominant **Band direction**
2. Number globally using band rules
3. Inside ring regions:

   * Switch to radial rules
4. At saddles:

   * Allow equality or constraint skipping

This is why your earlier LS solver failed:
it tried to treat local anomalies as global truth.

---

## What This Means for Constraint Generation

Now constraints are **conditional**:

| Topology | Constraint                    |
| -------- | ----------------------------- |
| Band     | `k[j] = k[i] ± step`          |
| Ring     | `k[j] = k[i] ± step (radial)` |
| Saddle   | `k[j] = k[i]` or none         |
| Unknown  | Skip edge                     |

No magic. No guessing.

---

## Updated Copilot Instruction (Addendum)

You should explicitly tell Copilot:

> Before generating constraints, classify each connected component into Band, Ring, Saddle-dominated, or Mixed. Constraint rules depend on this classification. Do not generate numeric constraints without topology classification.

---

## Why This Guarantees Stability with Few Anchors

* **Band** → 1 anchor sufficient
* **Ring** → 1 anchor sufficient
* **Mixed** → 2 anchors stabilize orientation
* **Saddle** → cannot anchor globally (by definition)

This matches both:

* interferometry theory
* phase unwrapping practice

---

## Final reassurance

You’re not overcomplicating this.
You’ve hit the **exact boundary** between “toy solver” and “scientific-grade algorithm”.

If you want next, I can:

* Formalize **ring detection** (containment tests)
* Give **exact graph metrics** to distinguish band vs ring
* Write **pseudo-code** for Phase 1.5 only
* Help define **confidence metrics** per fringe

Just tell me which one to tackle next.

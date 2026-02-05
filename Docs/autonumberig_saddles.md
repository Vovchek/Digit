
---

# Auto-Numbering Fringes with saddles — **Formal Solver Specification (v1)**

This solver **must always complete** without human intervention.
Ambiguity is resolved by **ordered preference rules**, not by stopping.

The solver outputs:

1. Updated `Number` for all `CFringeSegment`
2. `trustedFringes` — indices considered reliable
3. `weakFringes` — indices solved by weakest rules (for later cleanup)

---

## 0. Core Principles (non-negotiable)

1. **Topology first, geometry second**
2. **No magic constants** (distance thresholds, angle cutoffs)
3. **Ambiguity is resolved deterministically, not ignored**
4. **Earlier-resolved regions dominate later ones**
5. **Saddles are resolved last and may trigger iteration**

---

## 1. Problem Model

### 1.1 Objects

* Each `CFringeSegment` = isoline sample of scalar field
* `Number` = scalar height
* `step` = fixed increment

### 1.2 Goal

Assign `Number` to all segments such that:

* Adjacent fringes differ by ±`step` or 0 (rare, saddle or forced by anchors ridge/dip case)
* Nested fringes are monotone
* Saddle topology preserves alternating parity
* Global inconsistencies are minimized

---

## 2. High-Level Solver Phases (MANDATORY ORDER)

### Phase 1 — Preclassification (no numbering yet)

#### 2.1 All crossing or connected (epsilon proximity) fringes merge into a single node

#### 2.2 Classify all fringes into **topological regions**:

1. **Band regions**
   – approximately parallel, non-closing fringes
2. **Ring regions**
   – nested curves, outmost curve is closed, inner curves might not be
3. **Saddle regions**
   – cyclic adjacency, no nesting, no monotone axis

> Each fringe belongs to **exactly one** primary region.

---

## 3. Phase 2 — Ring structure arrangement (strongest)

### Rule

If fringes form nested closed loops:

* Sort by containment depth
* Inner vs outer direction is left as adjustable
* Assign monotone numbers to nested rings (relative to outmost ring)
* Each nested family forms complex node represented by its outermost fringe

### Output

* All nested ring fringes packed into single nodes
* Nodes (Outmost fringes) numbered either **trusted** if it is defined with an anchor or weak otherwise
* Nested fringes either **trusted** if defined with anchors or relative to outmost ring with unresolved direction
* Export adjacency constraints

---

## 4. Phase 3a — Band Resolution (strongest)

### Rule

If fringes form a band:

* Define a dominant direction (PCA on centroids)
* Sort fringes along perpendicular direction
* Assign numbers monotonically by `step`
* If any fringe has an anchor, use it to fix the sequence and mark as **trusted**
* Adjacent fringes can only differ by ±`step` or 0. 
* Zero difference can only be defined by anchor enforsment - ridge or dip case.
* Zero-difference edge splits area into 2 with oposite gradient directions

### Output

* All band fringes numbered
* Mark as **trusted**
* Generate directional constraints for neighbors

---

## 5. Phase 3b — Rings embeding (strong)

### Rule

If node is a ring, simple or packed:

* It either has its band fringe counterpart (same k) or has first or last k
* Counterpart is found by the proximity to adjacent bands
* If 2 adjacant bands are found, and they are trusted, and have k difference of 1, assign ring number to one of the closest
* If 2 adjacant bands are found, and they are trusted, and have same k, assign ring number to their k+1 or k-1, gradient-dependent
* If only one adjacent band fringe is found, and it is trusted, default to its k, but mark it as weak, because it can be either k+1 or k-1 
* Assign a gradient to node based on band gradient in this area
* Resolve nested fringes according to outermost ring number and gradient

### Output

* All ring nodes numbered
* Mark as **trusted**
* Export adjacency constraints

---

## 5. Phase 4 — Saddle Detection (structure only)

A saddle region is defined as:

* Simple cycle (usually 4)
* No containment
* No dominant axis
* Each fringe adjacent to exactly two in region

---

## 6. Phase 5 — Saddle Constraint Model (CRITICAL)

### 6.1 Hard constraint (topological)

For saddle cycle `f0..fk`:

```
Δ(f[i] → f[i+1]) alternates sign
Sum of Δ around cycle = 0
```

This constraint **must always hold**.

---

## 7. Phase 6 — Saddle Resolution Strategy (AUTOMATIC)

Saddles are resolved **after** bands and rings.

Resolution proceeds in **ordered attempts**:

---

### Attempt A — External propagation (preferred)

If any saddle fringe neighbors a **trusted fringe**:

* Inherit direction to preserve smoothness
* Solve alternation accordingly
* Mark saddle as **trusted**

---

### Attempt B — Adjacent inference

If two **adjacent saddle fringes** already have assigned numbers:

* Enforce alternation
* Solve uniquely
* Mark saddle as **trusted**

---

### Attempt C — Global consistency choice (mandatory fallback)

If saddle still ambiguous:

1. Generate **both valid alternation solutions**
2. Score each solution by:

   * minimizing number jumps vs neighboring regions
   * preserving monotonic trends
3. Choose lower-penalty solution

> This guarantees completion.

### Mark result as **weak** (not trusted)

---

## 8. Phase 7 — Iterative Propagation

Because saddle resolution may unlock neighbors:

```
repeat
    resolve bands
    resolve rings
    resolve saddles
until no changes
```

Maximum iterations = number of regions
(guaranteed termination)

---

## 9. Final Validation Pass

After all numbers assigned:

* Verify adjacency differences ∈ {±step}
* Repair isolated violations by local adjustment
* Prefer trusted over weak during repair

---

## 10. Output Contract

The solver function must return:

```cpp
struct AutoNumberingResult {
    std::vector<size_t> trustedFringes;
    std::vector<size_t> weakFringes;
};
```

All fringes **must** have assigned numbers.

---

## 11. Copilot Instruction — IMPLEMENTATION DIRECTIVE

> Implement auto-numbering as a **multi-phase deterministic constraint solver**.
>
> Do NOT:
>
> * rely on arbitrary thresholds
> * stop on ambiguity
> * assume one anchor is sufficient in saddle cases
>
> Always:
>
> * resolve bands and rings before saddles
> * enforce alternating parity in saddles
> * pick between valid saddle solutions using global consistency
> * mark confidence level of each decision

---

## 12. Important Design Note (for you)

This design:

* Matches physical interferograms
* Produces stable, repeatable results
* Makes later manual cleanup predictable
* Avoids heuristic explosions
* Explains *why* something may be “weak”

This is exactly how professional phase-unwrapping and contour-labeling systems work.

---


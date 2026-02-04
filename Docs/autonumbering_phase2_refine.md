Perfect. Below is a **precise, prescriptive Copilot instruction** for **Phase 2 (Adjacency Construction)** that explicitly **forbids magic numbers**, **prioritizes ordering/topology**, and **guarantees correct behavior for parallel bands and nested rings**.

This is written in a style that Copilot can actually follow without “creative” geometry guessing.

---

# Copilot Instruction

## Phase 2 — Build Adjacency Graph (Topology-First, Order-Based)

### Purpose

Construct an adjacency graph between fringe segments (isolines) that reflects their **topological and ordering relationships**, not incidental geometric proximity.

This phase must **never rely on arbitrary distance, overlap, or angle thresholds** to decide adjacency.

---

## Fundamental Rules (MANDATORY)

1. **Adjacency is derived from ordering or containment, not proximity**
2. **Sorting and hierarchy come before pairwise testing**
3. **Hard geometric thresholds are forbidden**
4. **Geometry is used only to infer global structure**
5. **Fallback heuristics must be soft (weighted), never gating**

If these rules conflict with implementation convenience, the rules win.

---

## Inputs

```cpp
const std::vector<CFringeSegment>& fringes;
const std::vector<FringeNode>& nodes;   // contains centroid, closureScore
```

---

## Outputs

```cpp
std::vector<AdjacencyEdge> edges;
```

Each `AdjacencyEdge` must contain:

```cpp
struct AdjacencyEdge {
    size_t i, j;
    double weight;   // confidence only
    int sign;        // +1, -1, or 0 if undetermined
};
```

---

## Phase 2.1 — Determine Dominant Structural Type

### Step 2.1.1 — Estimate global tangent direction

* Collect representative tangents from all fringes
* Compute principal direction using PCA
* Let this be the dominant tangent vector **T**
* Define normal vector **N = perpendicular(T)**

---

### Step 2.1.2 — Estimate closure dominance

* Compute:

  ```
  closedRatio = average(closureScore_i)
  ```

Use this only for **classification**, not decisions.

---

### Step 2.1.3 — Classify structure (soft decision)

Determine which case applies:

| Condition              | Structure               |
| ---------------------- | ----------------------- |
| closedRatio is high    | Nested / ring-dominated |
| open curves + strong T | Parallel-band           |
| otherwise              | Mixed                   |

Do **not** branch on hard thresholds; allow ambiguity.

---

## Phase 2.2 — Parallel-Band Adjacency (Primary Path)

**This path must be used whenever isolines are predominantly open and share a dominant tangent direction.**

### Step 2.2.1 — Project centroids

For each fringe `i`:

```
s_i = dot(centroid_i, N)
```

---

### Step 2.2.2 — Sort by projection

Create ordered list:

```
order = sort fringes by s_i
```

---

### Step 2.2.3 — Build adjacency edges

For consecutive fringes in sorted order:

```
for k in 0..N-2:
    connect(order[k], order[k+1])
```

Each connection:

* is symmetric
* has maximal base weight (e.g. 1.0)
* sign is left unset (`0`) unless gradient is known

No other edges are created in this mode.

---

## Phase 2.3 — Nested-Ring Adjacency (Secondary Path)

**Used when closed curves dominate.**

### Step 2.3.1 — Containment testing

For each pair `(i, j)` where both have high closureScore:

* Test if centroid(i) is inside polygon(j)
* Build containment DAG (forest)

---

### Step 2.3.2 — Build adjacency from hierarchy

For each parent–child pair:

```
connect(parent, child)
```

Adjacency reflects nesting depth.

---

## Phase 2.4 — Mixed / Fallback Adjacency (Last Resort)

Only if neither parallel nor nested structure is dominant.

### Rules for fallback:

* Do not reject candidates based on thresholds
* Every fringe must have at least one neighbor
* Prefer nearest neighbor **along estimated normal**
* Distance may affect **weight only**

Example strategy:

* For each fringe, find nearest neighbor in +N and −N direction
* Connect both

---

## Phase 2.5 — Edge Post-Processing

* Remove duplicate edges
* Ensure graph connectivity if possible
* Do not over-connect (no full pairwise graphs)

---

## Explicitly Forbidden Patterns (Copilot MUST NOT DO)

❌ Pairwise distance gating (`if dist > X continue`)
❌ Axis-aligned overlap tests
❌ Angle thresholds to decide adjacency
❌ Early `continue` based on geometry
❌ Creating edges only when “close enough”

---

## Required Guarantees

After Phase 2:

1. Parallel monotone bands produce a simple chain graph
2. Nested rings produce parent–child adjacency
3. Each fringe has ≤2 strong neighbors in ordered regions
4. Graph is suitable for numbering propagation

---

## Design Intent (for Copilot reasoning)

> Phase 2 is about **discovering order**, not measuring distance.

If a human can trivially see that isolines are ordered, the code must do so *without tuning constants*.

---

## End of Phase 2 Instruction

---

If you want, next I can:

* translate this into **C++ scaffolding Copilot can fill**
* design a robust **structure classifier**
* retrofit your existing code with minimal churn
* or show how Phase 2 feeds Phase 3 cleanly

This Phase-2 rewrite is the key to making the whole algorithm reliable.


---

# Copilot Instruction

## Fully Automatic Fringe (Isoline) Numbering Algorithm

### Goal

Implement a **fully automatic numbering algorithm** that assigns consistent scalar “Number” values to a set of fringe segments (isolines), using topology- and adjacency-based constraints, given a small set of trusted reference fringes.

The algorithm must:

* Modify `CFringeSegment::Number`
* Respect trusted fringes as hard constraints
* Infer numbers for all others
* Return updated trusted fringes (those with high confidence)

---

## Function Signature (required)

```cpp
std::vector<size_t> AutoNumberFringes(
    std::vector<CFringeSegment>& fringes,
    const std::vector<size_t>& trustedFringeIndices,
    double step,
    double confidenceThreshold
);
```

### Semantics

* `fringes` – [i/o] geometry + Number field
* `trustedFringeIndices` – [i] indices of fringes whose Numbers are trusted
* `step` – [i] scalar increment between adjacent isolines
* `confidenceThreshold` – [i] minimum confidence for accepting inferred numbers
* **Return value** – indices of fringes considered *trusted after processing*

---

## Architectural Overview

Implement the algorithm in **six explicit phases**:

1. Preprocessing
2. Adjacency graph construction
3. Constraint generation
4. Numerical solve (continuous)
5. Quantization + validation
6. Confidence evaluation and trusted-set update

Each phase must be implemented as a **clearly separated block**.

---

## Phase 1 — Preprocessing

### 1.1 Build internal node representation

Create a lightweight internal struct:

```cpp
struct FringeNode {
    size_t index;               // index into fringes[]
    double knownValue;           // Number if trusted
    bool isTrusted;
    Vec2 centroid;
    bool isClosed;
};
```

Populate one node per `CFringeSegment`.

### 1.2 Normalize trusted indices

* Mark nodes in `trustedFringeIndices` as `isTrusted = true`
* Convert their `Number` to integer indices:

  ```
  k_i = Number / step
  ```

---

## Phase 2 — Build adjacency graph

### 2.1 Define adjacency criteria

Two fringes `i` and `j` are adjacent if:

* minimum distance < `Dmax`
* projected overlap length > `Lmin`
* average tangent angle < `θmax`

(Use conservative defaults; do NOT over-connect.)

### 2.2 Store adjacency edges

Define:

```cpp
struct Edge {
    size_t i, j;
    double weight;
    int sign;    // +1, -1, or 0 if unknown
};
```

Build a list of edges.

---

## Phase 3 — Generate constraints

### 3.1 Known-value constraints (hard)

For each trusted node `i`:

```
k_i = known
```

These are **fixed rows** in the solver.

---

### 3.2 Local adjacency constraints (soft)

For each adjacency edge `(i, j)`:

* If a global gradient is available:

  * determine sign via dot(normal, gradient)
* Else:

  * set `sign = 0` (solver decides)

Constraint:

```
k_j - k_i ≈ sign
```

Weighted by edge confidence.

---

### 3.3 Nested-curve constraints (soft)

If:

* both curves are closed
* one contains the other

Add constraint:

```
|k_j - k_i| = 1
```

Implement this as two competing soft constraints (+1 and −1), letting solver choose.

---

## Phase 4 — Solve relaxed system

### 4.1 Assemble linear system

Solve:

```
min Σ w_ij (k_j - k_i - s_ij)^2
```

Subject to:

```
k_i = known, for trusted i
```

Implementation notes:

* Use double-precision
* Build sparse normal equations
* Use any stable least-squares solver

---

### 4.2 Solve for continuous k*

Result:

```
k_i* ∈ ℝ
```

---

## Phase 5 — Quantization & validation

### 5.1 Quantize

For each fringe:

```
k_i = round(k_i*)
Number = k_i * step
```

---

### 5.2 Residual computation

For each adjacency edge:

```
residual_ij = |k_j - k_i - s_ij|
```

Store per-node average residual.

---

## Phase 6 — Confidence & trusted update

### 6.1 Compute confidence per fringe

For each fringe `i`:

```
confidence_i = 1 - avg(residuals)
```

Clamp to `[0,1]`.

---

### 6.2 Update trusted set

Rules:

* Original trusted fringes remain trusted
* Newly inferred fringes become trusted if:

  ```
  confidence_i >= confidenceThreshold
  ```

Return:

```
vector<size_t> newTrustedFringeIndices
```

---

## Expected Behavior Guarantees

* Fully deterministic
* No UI interaction
* Undoable via command wrapper
* Graceful failure in ambiguous regions
* Trusted fringes propagate confidence outward

---

## Explicit Non-Goals (important for Copilot)

* Do NOT attempt global monotonic ordering
* Do NOT force integer consistency if residuals are high
* Do NOT silently override trusted inputs
* Do NOT infer extrema aggressively


---

## Final mental model (for Copilot)

> This function is a **constraint-based graph labeling solver**.
> It modifies Numbers only through optimization and confidence validation.

---


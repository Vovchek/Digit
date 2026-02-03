# Known Limitations & Future Enhancements

## Current Version (v1.0)

### Tested & Working ✅
- **Parallel curves** (horizontal, vertical, inclined fringes)
- **Evenly-spaced patterns** (regular interferogram fringes)
- **Large gaps** (isolated fringe clusters)
- **Trusted constraint enforcement** (hard constraints)
- **Edge cases** (single fringe, negative numbers, custom steps)
- **Confidence evaluation** (trust propagation)

### Limitations ⚠️

#### 1. **Concentric Circles (Nested Curves)**
**Status:** Not supported in v1.0  
**Reason:** Phase 3.3 (nested-curve constraints) is TODO  
**Impact:** Concentric circles don't create adjacency edges because:
- All have same X-axis bounds
- Distance test insufficient without overlap check
- Tangent angle ambiguous for closed curves

**Workaround:** Mark all circles as trusted manually, or enhance with Phase 3.3

**Example of unsupported case:**
```cpp
CFringeSegment circle1 = CreateCircle(cx, cy, 10.0);
CFringeSegment circle2 = CreateCircle(cx, cy, 20.0);  // Won't connect to circle1
CFringeSegment circle3 = CreateCircle(cx, cy, 30.0);  // Won't connect to circle2
```

#### 2. **Sign Inference** (Advanced)
**Status:** Fixed to +1 between adjacent fringes  
**Reason:** Sign detection requires gradient information  
**Impact:** Algorithm assumes each fringe is ±1 step from neighbors

**Could be enhanced with:** Optical flow, gradient direction, user hints

#### 3. **Isolated Fringe Clusters**
**Status:** Handled but low confidence  
**Reason:** Isolated fringes have degree = 0 → confidence = 0.1  
**Impact:** Won't be marked as newly trusted unless explicitly marked

**Workaround:** Increase `confidenceThreshold` to 0.0 to accept all, or keep isolated clusters separate

---

## Phase 3.3: Nested Curve Detection (Future Enhancement)

### Concept
Detect when one closed curve is completely contained within another:

```cpp
bool IsNested(const CFringeSegment& inner, const CFringeSegment& outer) {
    // Check if all points of inner are inside bounding box of outer
    // Or better: use point-in-polygon test
}
```

### Implementation Strategy

1. **Detect closed curves** (already done in Phase 1)
2. **For each pair of closed curves:**
   - Compute bounding boxes
   - Check containment via centroid distance + radius
3. **Create constraints:**
   ```
   |k_outer - k_inner| = 1  (soft constraint, both directions)
   ```

### Example
```cpp
struct NestedCurveConstraint {
    size_t inner, outer;
    double weight;
};

std::vector<NestedCurveConstraint> FindNestedCurves(
    const std::vector<FringeNode>& nodes) {
    std::vector<NestedCurveConstraint> constraints;
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (nodes[i].isClosed && nodes[j].isClosed && i != j) {
                if (IsNested(nodes[i], nodes[j])) {
                    NestedCurveConstraint c;
                    c.inner = i;
                    c.outer = j;
                    c.weight = 0.8;  // Soft constraint
                    constraints.push_back(c);
                }
            }
        }
    }
    return constraints;
}
```

### Complexity
- O(n²) pairwise checks (acceptable for n < 1000)
- Point-in-polygon test: O(m) per pair (m = points per curve)
- Overall: O(n² × m) ≈ feasible for typical fringe sets

---

## Performance Optimization (Future)

### Sparse Matrix Solver
Current: Dense Gauss-Seidel iteration  
Problem: O(n²) memory for large n

**Solution:** Use sparse solver for n > 500
- Only ~O(m) non-zero entries (m ≈ edges)
- Can use conjugate gradient or MINRES
- Library: Eigen3 (already used in project?)

### GPU Acceleration
For real-time processing of large interferograms:
- CUDA kernels for adjacency computation
- Batched matrix operations
- Estimated speedup: 10-100x

---

## Known Test Gaps

| Scenario | Status | Issue |
|----------|--------|-------|
| Concentric circles | Skipped | Phase 3.3 TODO |
| Radial fringes | Not tested | May work, needs verification |
| Highly nonlinear spacing | Not tested | May fail convergence |
| Very large sets (n>1000) | Not tested | Memory + time limits unknown |

---

## Testing Recommendations for v1.1

1. **Add Phase 3.3 tests** after implementation
2. **Test with real interferogram data:**
   - Fringe spacing validation
   - Confidence score realism
3. **Benchmark with large sets (n=100-1000)**
4. **Validate edge case behavior**

---

## Backward Compatibility

✅ **No breaking changes** - Algorithm signature unchanged  
✅ **Handles all v1.0 cases** - Backward compatible

---

## References

- **Algorithm spec:** `Docs/autonumberig.md`
- **Implementation:** `DigitMode/Commands/AutoNumberingAlgorithm.h`
- **Tests:** `Tests/DigitModeTests/AutoNumberingAlgorithmTest.cpp`
- **UX integration:** Next phase

---

**Status:** v1.0 complete, v1.1 enhancements planned

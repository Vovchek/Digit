# CalcContourTest Suite - Incorrect Test Analysis

## Summary

Found **7+ tests** in `CalcContourTest.cpp` with **fundamentally wrong expectations** about EXTERNAL shape visibility logic.

All tests incorrectly assume multiple EXTERNAL shapes produce multiple separate contours (UNION), when the correct behavior is to produce a single contour for their INTERSECTION.

---

## Root Cause

### ? WRONG (what tests assume):
```
visible_area = EXTERNAL? ? EXTERNAL? ? ... ? EXTERNAL?
```
Multiple EXTERNAL shapes each define separate visible regions.

### ? CORRECT:
```
visible_area = EXTERNAL? ? EXTERNAL? ? ... ? EXTERNAL?
```
Multiple EXTERNAL apertures define ONE visible region = their intersection.

**For non-overlapping EXTERNAL shapes:**
```
visible_area = ? (EMPTY)
Expected contours: 0
```

---

## Tests Requiring Fixes (7 total)

1. **`TwoNonOverlappingEllipses_ProducesTwoContours`** - Non-overlapping ? expects 2, should be 0
2. **`EllipseInsideEllipse_EXTERNAL_ProducesOneContour`** - Wrong comments about visibility
3. **`EllipseAndRectangle_NonOverlapping`** - Non-overlapping ? expects 2, should be 0
4. **`EllipseRectanglePolygon_Mixed`** - 3 non-overlapping ? expects 3, should be 0
5. **`AllEXTERNAL_OnlyOutsidePointsVisible`** - Wrong name and expectations
6. **`ManySmallShapes_PerformanceTest`** - 25 non-overlapping ? expects 25, should be 0
7. **`TouchingButNotOverlapping_ProducesSeparateContours`** - Touching ? degenerate ? should be 0

See full document for detailed analysis and fixes for each test.

---

## Key Insight

**Optical Analogy:**

Multiple EXTERNAL apertures = Light passing through multiple filters in series:
- Must pass through **ALL** filters
- Final visible area = **INTERSECTION** of all filter openings
- Non-overlapping openings ? **no light passes through**

---

## Action Items

1. Fix 7 tests with wrong expectations
2. Fix misleading comments in 2+ tests
3. Add test for overlapping EXTERNAL shapes with intersection verification
4. Build and verify all tests pass


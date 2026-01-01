# Degenerate Polygon Bug - Quick Reference

## The Bug in One Sentence

Changing PI precision causes `NFi` to drop from 501?500, changing point sampling such that visibility filtering for overlapping EXTERNAL ellipses leaves only 2 visible points, creating an invalid 2-point "polygon" that crashes downstream bounds calculations.

## The Root Cause Chain

```
High-Precision PI
    ?
Slightly Different Perimeter
    ?
NFi = int(Perim / Step) truncates to different value (501 ? 500)
    ?
Different angular spacing ? different points sampled
    ?
Visibility test: point on Ellipse[0] is OUTSIDE Ellipse[1]
    ?
For EXTERNAL shapes: outside points = NOT visible
    ?
isPupil() returns false for most points
    ?
Only 2 points pass visibility filtering
    ?
ConnectSegments creates 2-point "polygon"
    ?
XYPolygon constructor accepts it (no validation)
    ?
Classification code marks it as EXTERNAL
    ?
GetBounds() produces invalid zero-area bounds
    ?
CRASH - downstream code expects valid bounds
```

## The Five Problems

1. **Integer Truncation**: `static_cast<int>(Perim / Step)` is sensitive to tiny changes
2. **Overly Strict Visibility**: EXTERNAL shapes mark boundary points as invisible
3. **No Minimum Point Check**: Code assumes contours always have "many" points
4. **No Degenerate Validation**: XYPolygon accepts 2-point input
5. **No Bounds Safety**: GetBounds() assumes ?3 points

## The Critical Values

- **NFi = 500 vs 501**: Boundary where bug appears
- **2 visible points**: Degenerate polygon threshold
- **EXTERNAL**: Shape type where visibility is problematic
- **Slightly overlapping**: Apertures with 0.1-0.5 unit offset

## The Test Files Created

1. **CalcContourDegenerateTest.cpp** (585 lines)
   - 33+ test cases covering all failure modes
   - Tests for 2-point polygons
   - Tests for zero-area bounds
   - Tests for visibility filtering aggressiveness
   - Tests for NFi=500 vs NFi=501 consistency

2. **CALCCONTOUR_DEGENERATE_POLYGON_BUG_ANALYSIS.md**
   - Detailed root cause analysis
   - Step-by-step explanation of failure chain
   - Visualization of the problem

3. **CALCCONTOUR_DEGENERATE_BUG_TEST_STRATEGY.md**
   - Fix recommendations (5 different approaches)
   - Test execution plan
   - Success criteria

## Key Test Cases

### Most Important Tests

1. `DetectDegeneratePolygon_TwoSlightlyDifferentEllipses`
   - Core bug scenario
   - Tests multiple NPntMax values
   
2. `PointCountConsistency_NFi500vs501`
   - Tests critical boundary
   - Verifies both produce valid polygons

3. `BugScenario_OnlyTwoVisiblePoints`
   - Exact reproduction of bug condition
   - Ensures no 2-point polygons created

### Validation Tests

4. `AllContoursHaveMinimumPoints`
   - Every polygon must have ?3 points

5. `AllContoursHaveValidBounds`
   - Every polygon must have non-zero area

6. `VisibilityFiltering_NotOverlyAggressive`
   - At least 5% of points should be visible

## How to Run Tests

```bash
cd Debug
.\Tests.exe --gtest_filter=CalcContourDegenerateTest.*
```

## Expected Test Results (Before Fixes)

**Many tests will FAIL**, documenting:
- 2-point polygons being created
- Zero-area bounds
- Only 2 visible points out of 500
- Inconsistent behavior between NFi=500 and NFi=501

## Recommended Fixes (Priority Order)

### Priority 1: Validate Minimum Points
```cpp
// After ConnectSegments in CalcContour
for (int i = NCont - 1; i >= 0; i--) {
    if (ArrCont[i].GetSize() < 3) {
        ArrCont.RemoveAt(i);  // Remove degenerate
    }
}
```

### Priority 2: Round Instead of Truncate
```cpp
// XYEllipse::GetContour()
int NFi = static_cast<int>(Perim / Step + 0.5);  // Round
if (NFi < 8) NFi = 8;  // Minimum points
```

### Priority 3: Relax Visibility at Boundaries
```cpp
// In isVisible() for EXTERNAL shapes
// Add small tolerance for boundary points
```

## Success Criteria

- ? All 33+ tests pass
- ? No polygon with < 3 points
- ? No zero-area bounds
- ? Point counts stable (±2) across NPntMax 498-502
- ? ?5% visible points for each shape
- ? DOS ZAP file loads without crash

## Impact of Bug

**Before Fix**:
- App crashes on certain ZAP files
- Only with high-precision PI (double precision)
- Unpredictable - depends on exact perimeter values
- Hard to debug - manifests as bounds calculation failure

**After Fix**:
- Robust handling of edge cases
- Consistent behavior regardless of PI precision
- Graceful degradation (remove degenerate polygons)
- Documented and tested behavior

## Files Modified (For Testing Only)

No production code modified - only tests created:
- `Tests/InterfSolver/Tools/CalcContourDegenerateTest.cpp`
- `Tests/InterfSolver/Tools/CALCCONTOUR_DEGENERATE_POLYGON_BUG_ANALYSIS.md`
- `Tests/InterfSolver/Tools/CALCCONTOUR_DEGENERATE_BUG_TEST_STRATEGY.md`
- `Tests/InterfSolver/Tools/CALCCONTOUR_DEGENERATE_BUG_QUICK_REFERENCE.md` (this file)

## Next Actions

1. Add `CalcContourDegenerateTest.cpp` to test project
2. Run tests to confirm failures
3. Analyze failure patterns
4. Implement fixes one at a time
5. Re-run tests after each fix
6. Verify all pass before merging

---

*Created: Analysis of degenerate polygon bug in CalcContour*  
*Purpose: Detect and document bug without modifying production code*  
*Status: Tests created, awaiting execution and fixes*

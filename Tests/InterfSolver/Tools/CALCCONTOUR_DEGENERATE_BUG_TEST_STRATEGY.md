# Degenerate Polygon Bug - Test Strategy & Fix Recommendations

## Executive Summary

The bug occurs when:
1. High-precision PI changes perimeter calculation slightly
2. `NFi = static_cast<int>(Perim / Step)` truncates to different value (500 vs 501)
3. Different point sampling causes different points to be tested for visibility
4. For two slightly overlapping EXTERNAL ellipses, visibility test is overly strict
5. Result: Only 2 points pass visibility ? degenerate 2-point polygon
6. Downstream code crashes on invalid bounds

## Root Causes Identified

### 1. **Integer Truncation Sensitivity**
```cpp
// XYEllipse::GetContour()
int NFi = static_cast<int>(Perim / Step);  // TRUNCATES
```
**Problem**: Small floating-point differences cause discrete jumps in point count.

### 2. **Overly Aggressive Visibility Filtering**
```cpp
// For EXTERNAL shapes
bool isVisible(const XYPoint &P) const {
  bool isIn = isInside(P);
  if (isIn && TypeLimits == INTERNAL)
    return false;   // INTERNAL = obstruction
  else if (!isIn && TypeLimits == EXTERNAL)
    return false;   // EXTERNAL = aperture opening
  return true;
}
```
**Problem**: For slightly overlapping EXTERNAL apertures, points on boundary of one aperture are often OUTSIDE the other aperture ? marked invisible.

### 3. **No Minimum Point Validation**
```cpp
// CalcContour - No validation!
ConnectSegments(ArrBLn, ArrCont, Eps);
```
**Problem**: Code assumes contours always have "enough" points. No check for degenerate cases.

### 4. **No Degenerate Geometry Protection**
```cpp
// XYPolygon constructor
XYPolygon :: XYPolygon(const XYBrokenLine &A, ...) {
  int NPnt = A.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  // NO CHECK for NPnt < 3!
}
```
**Problem**: Accepts 2-point "polygons" which cause invalid bounds.

## Test Strategy

### Phase 1: Detection Tests ? Created

**File**: `CalcContourDegenerateTest.cpp`

**Key Tests**:
1. `DetectDegeneratePolygon_TwoSlightlyDifferentEllipses` - Core bug scenario
2. `AllContoursHaveMinimumPoints` - Validates ?3 points per polygon
3. `AllContoursHaveValidBounds` - Checks for zero-area bounds
4. `PointCountConsistency_NFi500vs501` - Tests critical boundary
5. `VisibilityFiltering_NotOverlyAggressive` - Ensures substantial visible points
6. `BugScenario_OnlyTwoVisiblePoints` - Exact bug condition

### Phase 2: Validation Tests

These tests verify assumptions:
- Polygon constructors validate input
- ConnectSegments rejects degenerate inputs
- Bounds calculation handles edge cases
- Visibility logic preserves aperture semantics

### Phase 3: Regression Tests

Tests with real-world data patterns:
- Multiple overlapping apertures
- Various NPntMax values around 500
- Different ellipse size ratios
- Mixed shape types

## Recommended Fixes (For Future Implementation)

### Fix 1: Validate Minimum Points in CalcContour

```cpp
// After ConnectSegments
int NCont = ArrCont.GetSize();
for (int i = NCont - 1; i >= 0; i--) {
    if (ArrCont[i].GetSize() < 3) {
        // Log warning
        ArrCont.RemoveAt(i);  // Remove degenerate polygon
    }
}
```

### Fix 2: Use Rounding Instead of Truncation

```cpp
// XYEllipse::GetContour()
int NFi = static_cast<int>(Perim / Step + 0.5);  // ROUND instead of truncate
```

### Fix 3: Ensure Minimum Point Count

```cpp
// XYEllipse::GetContour()
int NFi = static_cast<int>(Perim / Step);
if (NFi < 8) NFi = 8;  // Minimum 8 points for any ellipse
```

### Fix 4: Relax Visibility for Nearly-Overlapping EXTERNAL Shapes

```cpp
// Option A: Add tolerance to visibility check
bool isVisible(const XYPoint &P) const {
  bool isIn = isInside(P);
  
  // For EXTERNAL, add small tolerance for boundary points
  if (TypeLimits == EXTERNAL) {
    double dist = /* distance to boundary */;
    if (dist < BOUNDARY_TOLERANCE) {
      return true;  // Treat boundary as visible
    }
  }
  
  // ... rest of logic
}
```

### Fix 5: Validate Polygon Constructor Input

```cpp
XYPolygon :: XYPolygon(const XYBrokenLine &A, ...) {
  int NPnt = A.GetSize();
  
  // VALIDATE INPUT
  if (NPnt < 3) {
    // Log warning or throw exception
    // For now, set to empty polygon
    return;
  }
  
  // ... rest of constructor
}
```

## Why Tests Come First

1. **Document Current Behavior**: Tests show WHAT fails, not just that it fails
2. **Prevent Regression**: After fixing, ensure bug doesn't return
3. **Guide Fixes**: Tests reveal WHERE to add validation
4. **Verify Assumptions**: Tests expose hidden assumptions in code

## Test Execution Plan

### Step 1: Add Tests to Build System

Add to `Tests/InterfSolver/Tools/CMakeLists.txt` or project file:
```
CalcContourDegenerateTest.cpp
```

### Step 2: Run Tests (Expected Failures)

```bash
.\Debug\Tests.exe --gtest_filter=CalcContourDegenerateTest.*
```

**Expected**: Many tests will FAIL, documenting the bug.

### Step 3: Analyze Failures

Each failure shows:
- Which scenario triggers bug
- What the symptoms are (point counts, bounds, etc.)
- Where in code path it occurs

### Step 4: Implement Fixes (Future Work)

After analysis, implement fixes one at a time, re-running tests after each fix.

### Step 5: Verify All Tests Pass

When all `CalcContourDegenerateTest.*` tests pass ? bug is fixed!

## Metrics to Track

Before fixes:
- Number of tests failing
- Scenarios that produce 2-point polygons
- Point count variations with different NPntMax

After fixes:
- All tests passing
- No 2-point polygons
- Stable point counts (±1-2) across NPntMax range

## Risk Assessment

**Without Tests**: 
- Bug may reappear in future
- Similar bugs may exist elsewhere
- Hard to verify fixes work

**With Tests**:
- Bug is documented and detectable
- Fixes can be verified
- Prevents regression
- Guides similar code improvements

## Next Steps

1. ? Tests created (`CalcContourDegenerateTest.cpp`)
2. ? Analysis documented (this file + `CALCCONTOUR_DEGENERATE_POLYGON_BUG_ANALYSIS.md`)
3. ?? Add tests to build system
4. ?? Run tests to confirm failures
5. ?? Analyze failure patterns
6. ?? Implement fixes (one at a time)
7. ?? Verify fixes with tests
8. ?? Document what was fixed

## Success Criteria

? All `CalcContourDegenerateTest.*` tests pass  
? No polygon with < 3 points  
? All polygons have valid (non-zero area) bounds  
? Point counts stable across NPntMax = 498-502  
? Visibility preserves substantial points (?5% of total)  
? Real-world DOS ZAP file loads without crash  

---

## Files Created

1. `CALCCONTOUR_DEGENERATE_POLYGON_BUG_ANALYSIS.md` - Detailed root cause analysis
2. `CalcContourDegenerateTest.cpp` - Comprehensive test suite (33+ tests)
3. `CALCCONTOUR_DEGENERATE_BUG_TEST_STRATEGY.md` - This file

These files provide:
- **Understanding**: Why the bug happens
- **Detection**: Tests that catch it
- **Guidance**: How to fix it
- **Verification**: How to confirm fix works

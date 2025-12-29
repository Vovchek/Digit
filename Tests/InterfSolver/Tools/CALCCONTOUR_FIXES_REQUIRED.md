# CalcContour Test Failures - Fixes Required

## Test Results Summary
- **Tests Run**: 32 CalcContourTest tests
- **Passed**: 20 (62.5%)
- **Failed**: 12 (37.5%)
- **Execution Time**: 3.1 seconds (no hangs/infinite loops)

## Root Causes Found

### 1. XYRect::GetContour() Const-Correctness Bug ?? CRITICAL

**File**: `InterfSolver/Tools/XYRect.cpp` line 147-153

**Problem**: The const method calls a non-const method, causing compilation/linking issues.

```cpp
bool XYRect::GetContour(XYBrokenLine& BLine, double Step) const  // <-- CONST
{
	if (Step <= 0.)
		return false;

	double Perim = Perimeter();
	int NFi = static_cast<int>(Perim / Step);
	bool isSuccess = GetContour(BLine, NFi);  // <-- Calls NON-CONST method!
	return isSuccess;
}
```

The non-const version at line 121:
```cpp
bool XYRect::GetContour(XYBrokenLine& BLine, int NFi)  // <-- NOT CONST
{
    // Implementation...
}
```

**Impact**: All 5 rectangle tests fail (produce 0 contours)

**Fix**: Make the method `GetContour(XYBrokenLine&, int NFi)` const:

```cpp
bool XYRect::GetContour(XYBrokenLine& BLine, int NFi) const  // Add const
{
	if (NFi < 2)
		return false;

	BLine.RemoveAll();
	// ... rest of implementation unchanged ...
	return true;
}
```

Also update header file `XYRect.h` line 40:
```cpp
bool GetContour(XYBrokenLine &BLine, int NFi) const;  // Add const
```

---

### 2. Segment Connection Tolerance Too Small ?? HIGH

**File**: `InterfSolver/Tools/CalcContour.cpp` line 145

**Problem**: 
```cpp
double Eps = 2.5 * Step;  // Can be extremely small
```

When `Step` is tiny (e.g., `1e-8`), `Eps` becomes `2.5e-8`, which is too strict for floating-point comparisons.

**Impact**: 
- Creates massive fragmentation (14 contours instead of 2)
- Many unclosed contours

**Fix**: Add minimum tolerance:

```cpp
double Eps = max(2.5 * Step, 1e-5);  // Ensure reasonable minimum
```

---

### 3. Contours Not Explicitly Closed ?? HIGH

**File**: `InterfSolver/Tools/CalcContour.cpp` line 188-192

**Problem**: Segments connect but don't form closed loops.

```cpp
if (CurCont.GetSize() > 0)
{
    Plg = XYPolygon(CurCont);  // No closing check!
    ArrCont.Add(Plg);
}
```

**Impact**: 7 contours fail `IsClosed()` test

**Fix**: Explicitly close contours before creating polygon:

```cpp
if (CurCont.GetSize() > 0)
{
    NCur = CurCont.GetSize();
    double closingDist = Distance(CurCont[0], CurCont[NCur-1]);
    
    // If endpoints are close, explicitly close by adding first point
    if (closingDist < Eps * 2.0)  // Slightly larger tolerance
    {
        CurCont.Add(CurCont[0]);
    }
    
    Plg = XYPolygon(CurCont);
    ArrCont.Add(Plg);
}
```

---

### 4. Classification Assumes Single External Contour ?? MEDIUM

**File**: `InterfSolver/Tools/CalcContour.cpp` lines 198-220

**Problem**: Marks all but one contour as INTERNAL, even for separate non-overlapping shapes.

```cpp
// Set contour types: one external, others internal
for (iElm = 0; iElm < NCont; iElm++)
{
    if (iElm == iExt)
        ArrCont[iElm].SetTypeLimits(EXTERNAL);
    else
        ArrCont[iElm].SetTypeLimits(INTERNAL);  // WRONG for separate shapes!
}
```

**Impact**: 
- `AllEXTERNAL_OnlyOutsidePointsVisible` marks 13 shapes as INTERNAL
- `ManySmallShapes_PerformanceTest` would mark 24 shapes as INTERNAL

**Fix**: Mark each contour based on whether it's inside any other:

```cpp
// Mark all contours that aren't inside others as EXTERNAL
for (iElm = 0; iElm < NCont; iElm++)
{
    P = ArrCont[iElm][0];
    bool isInsideAny = false;
    for (i = 0; i < NCont; i++)
    {
        if (i == iElm) continue;
        if (ArrCont[i].isInside(P))
        {
            isInsideAny = true;
            break;
        }
    }
    ArrCont[iElm].SetTypeLimits(isInsideAny ? INTERNAL : EXTERNAL);
}
```

---

## Implementation Plan

### Step 1: Fix XYRect Const-Correctness (CRITICAL)

**Files to modify**:
1. `InterfSolver/Tools/XYRect.h` - Add `const` to method declaration
2. `InterfSolver/Tools/XYRect.cpp` - Add `const` to method definition

**Expected impact**: Fixes 5 tests immediately
- ? `SingleRectangle_ProducesOneContour`
- ? `EllipseAndRectangle_NonOverlapping`  
- ? `EllipseRectanglePolygon_Mixed`
- ? `RotatedRectangle_ProducesCorrectContour`
- ? `Square_ProducesRectangularContour`

---

### Step 2: Increase Segment Connection Tolerance (HIGH)

**File**: `InterfSolver/Tools/CalcContour.cpp`

**Change line 145**:
```cpp
// Before:
double Eps = 2.5 * Step;

// After:
double Eps = max(2.5 * Step, 1e-5);
```

**Expected impact**: Fixes 3-4 tests
- ? `AllContours_AreClosed` (better segment connection)
- ? `AllEXTERNAL_OnlyOutsidePointsVisible` (less fragmentation)
- Partial fix for `TouchingButNotOverlapping_ProducesSeparateContours`

---

### Step 3: Explicitly Close Contours (HIGH)

**File**: `InterfSolver/Tools/CalcContour.cpp`

**Replace lines 188-192** with closing logic.

**Expected impact**: Fixes remaining closure issues
- ? `AllContours_AreClosed` (all 7 unclosed contours fixed)

---

### Step 4: Fix Classification Logic (MEDIUM)

**File**: `InterfSolver/Tools/CalcContour.cpp`

**Replace lines 207-220** with new classification.

**Expected impact**: Fixes 2 tests
- ? `ConcentricShapes_ClassifiesExternalAndInternal`
- ? `AllEXTERNAL_OnlyOutsidePointsVisible` (correct EXTERNAL/INTERNAL)

---

## Expected Results After All Fixes

**Failures: 12 ? 0**

All 32 CalcContourTest tests should pass.

---

## Additional Investigation Needed

### Possible Remaining Issues After Fixes

1. **TouchingButNotOverlapping_ProducesSeparateContours**
   - May need special handling for tangent shapes
   - Tolerance might merge them into one contour

2. **EllipseWithInternalHole_ProducesTwoContours** 
   - Currently produces 0 contours
   - May indicate `isPupil()` issue with INTERNAL shapes

3. **MixedEXTERNALandINTERNAL_ProducesCorrectContours**
   - Currently produces 0 contours  
   - Same `isPupil()` concern

If these still fail after the 4 main fixes, investigate `isPupil()` visibility logic for INTERNAL shapes.

---

## Files to Modify

1. ?? `InterfSolver/Tools/XYRect.h` - Add const qualifier
2. ?? `InterfSolver/Tools/XYRect.cpp` - Add const qualifier  
3. ?? `InterfSolver/Tools/CalcContour.cpp` - Three changes (tolerance, closing, classification)

---

## Rebuild and Re-test Commands

```powershell
# Rebuild
msbuild /p:Configuration=Debug

# Run CalcContour tests
.\Debug\Tests.exe --gtest_filter=CalcContourTest.*

# Run all tests
.\Debug\Tests.exe
```

---

## Notes

- No infinite loops found (my initial analysis was incorrect)
- All tests complete in reasonable time (3.1s total)
- Primary issue is const-correctness bug in XYRect
- Secondary issues are tolerance and closing logic
- Classification logic needs improvement for multiple external contours

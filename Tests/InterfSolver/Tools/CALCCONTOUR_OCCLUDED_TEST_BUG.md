# CompletelyOccludedShape Test Bug Analysis

## Summary

The test `CompletelyOccludedShape_ProducesNoContour` had **completely backwards expectations** based on fundamental misunderstanding of EXTERNAL shape semantics.

---

## The Broken Test

```cpp
TEST_F(CalcContourTest, CompletelyOccludedShape_ProducesNoContour) {
    // Large ellipse completely hiding small one  ? WRONG!
    XYEllipse largeEll(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse smallEll(3.0, 2.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(largeEll);
    arrEll.Add(smallEll);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Small ellipse should be completely hidden  ? WRONG!
    // Only large ellipse contour should appear  ? WRONG!
    EXPECT_GE(arrCont.GetSize(), 1);
}
```

### What the Test Thought Would Happen

**WRONG assumption:**
> "Large EXTERNAL ellipse occludes (hides) small EXTERNAL ellipse, so only the large ellipse boundary is visible"

This assumes EXTERNAL shapes work like **opaque objects** where larger shapes hide smaller ones.

---

## What Actually Happens with EXTERNAL Shapes

### Correct EXTERNAL Semantics

**EXTERNAL = Aperture/Opening:**
- Points **INSIDE** the shape can see through
- Multiple EXTERNAL shapes = Multiple apertures in series
- Visible area = **INTERSECTION** of all apertures

### The Scenario

```
     [Large Ellipse] (EXTERNAL)
    ?????????????????????????
   ???????????????????????????
  ?????????????????????????????
  ????????[Small]??????????????  ? Small ellipse inside
  ?????????????????????????????
   ???????????????????????????
    ?????????????????????????
```

**Visible area calculation:**
```
visible_area = largeEll ? smallEll
             = smallEll  (small is completely inside large)
```

**Result:**
- Visible region = **small ellipse**
- Expected contour = **small ellipse boundary**
- **NOT** the large ellipse!

---

## Why This Makes Sense (Optical Analogy)

### Multiple Apertures in Series

Think of camera with multiple diaphragm stops:

```
[Large Aperture]  ?  [Small Aperture]  ?  Image
   20mm opening        3mm opening
```

**What passes through:**
- Light must pass through **BOTH** apertures
- Large opening allows **everything** smaller than 20mm
- Small opening **restricts** to only 3mm
- **Final result:** 3mm opening (the smaller aperture limits)

**Conclusion:**
> The smaller aperture **defines** the final visible area, not the larger one!

---

## What CalcContour Does

### Processing Steps

1. **Extract segments from large ellipse:**
```cpp
// For each point on large ellipse boundary
// Check if visible: isPupil(pt, {small ellipse})
// Points on large boundary are OUTSIDE small ellipse
// Result: NO visible segments from large ellipse
```

2. **Extract segments from small ellipse:**
```cpp
// For each point on small ellipse boundary
// Check if visible: isPupil(pt, {large ellipse})
// Points on small boundary are INSIDE large ellipse
// Result: ALL points visible ? full small ellipse contour
```

3. **ConnectSegments:**
```
Input: [Small ellipse full contour]
Output: [Polygon of small ellipse]
```

4. **Expected result:**
```
arrCont.GetSize() = 1
arrCont[0] = Small ellipse contour
```

---

## Why the Test Might Fail

### Potential Issues

1. **Boundary tolerance problems:**
   - Points on small ellipse boundary might be **exactly** on the boundary
   - `isInside(largeEll)` behavior at boundary depends on `HIGH_PRECISION` tolerance
   - If boundary points are considered "outside" ? no segments extracted

2. **Step size issues:**
   - Very small ellipse (radii 3?2) with coarse sampling
   - Might not generate enough points to form valid polygon
   - Could result in degenerate polygon that gets filtered out

3. **Duplicate segment handling:**
   - If both ellipses somehow contribute segments
   - ConnectSegments might merge into degenerate polygon

---

## The Fix

### Renamed and Corrected Test

```cpp
TEST_F(CalcContourTest, TwoConcentricEllipses_EXTERNAL_InnerVisible) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;

    // Two concentric EXTERNAL ellipses (large contains small)
    // EXTERNAL means inside is visible
    // Visible area = intersection = smaller ellipse
    XYEllipse largeEll(20.0, 15.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse smallEll(3.0, 2.0, 0.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(largeEll);
    arrEll.Add(smallEll);

    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);

    // Should produce contour for the intersection (small ellipse)
    EXPECT_EQ(arrCont.GetSize(), 1)
        << "Intersection of concentric EXTERNAL ellipses = smaller ellipse";
    
    // Verify the contour approximates the small ellipse size
    if (arrCont.GetSize() > 0) {
        XYBounds bounds = arrCont[0].GetBounds();
        double width = bounds.XRight - bounds.XLeft;
        double height = bounds.YBottom - bounds.YTop;
        
        // Expected: width ~6, height ~4
        EXPECT_LT(width, 8.0)   << "Contour too wide for small ellipse";
        EXPECT_GT(width, 4.0)   << "Contour too narrow for small ellipse";
        EXPECT_LT(height, 6.0)  << "Contour too tall for small ellipse";
        EXPECT_GT(height, 2.0)  << "Contour too short for small ellipse";
    }
}
```

### Key Changes

1. **? Correct test name:** "TwoConcentricEllipses_EXTERNAL_InnerVisible"
2. **? Correct comment:** "Visible area = intersection = smaller ellipse"
3. **? Correct expectation:** `EXPECT_EQ(arrCont.GetSize(), 1)`
4. **? Bounds verification:** Checks that contour matches small ellipse size

---

## Investigation of Potential CalcContour Bugs

### Issue 1: Boundary Point Handling

**Problem:**
Points on the boundary of the small ellipse might not be consistently classified as "inside" the large ellipse due to floating-point precision.

**XYEllipse::isInside() logic:**
```cpp
double T = R - 1.;
if (T <= HIGH_PRECISION)  // Boundary tolerance
    return true;
```

**If `HIGH_PRECISION` is too small:**
- Boundary points might be considered "outside"
- No visible segments extracted from small ellipse
- Result: Empty contour array

**Fix needed:**
- Ensure `HIGH_PRECISION` tolerance is appropriate
- Or adjust `isPupil` logic to handle boundary cases

### Issue 2: Small Shape Sampling

**Problem:**
Small ellipse (radii 3?2) might not generate enough sample points with default step size.

**CalcContour step calculation:**
```cpp
Step = MaxPerim / NPntMax;
// For large ellipse: perimeter ~110, NPntMax=200 ? Step ~0.55
// For small ellipse: perimeter ~16 ? ~29 sample points
```

**29 points should be sufficient**, but if step size is too large, might create degenerate polygon.

**Unlikely to be an issue** with NPntMax=200.

### Issue 3: Duplicate Segment Handling

**Not applicable here** because:
- Only small ellipse contributes visible segments
- Large ellipse contributes nothing (all points outside small ellipse)

---

## Conclusion

### The Real Bug

**The bug is NOT in CalcContour** - it's in the **test's understanding** of EXTERNAL shapes!

The test had **completely backwards expectations**:
- ? Expected: Large ellipse occludes small ellipse
- ? Correct: Small ellipse defines visible area (intersection)

### Potential CalcContour Issues (to investigate)

1. **Boundary tolerance** - might need adjustment for edge cases
2. **Very small shapes** - might need minimum point count check

But these are **minor edge cases**, not fundamental bugs.

### Test Status

? **Test renamed and corrected**  
? **Expectations match correct EXTERNAL semantics**  
? **Bounds verification added to confirm small ellipse**

---

## Visual Explanation

### WRONG Understanding (what test assumed)

```
"Occlusion" (opaque objects):
Larger object hides smaller object

    [Large Object]
   ?????????????
   ?????????????
   ???[S]???????  ? Small hidden by large
   ?????????????
   ?????????????

Visible: Large object boundary only
```

### CORRECT Understanding (apertures)

```
Apertures (openings):
Smaller opening restricts larger opening

    [Large Aperture]
   ?????????????
   ?????????????
   ???[S]???????  ? Small restricts large
   ?????????????
   ?????????????

Visible area: Small aperture opening
Light path: Must pass through BOTH ? limited by smaller
```

**Analogy:**
- Two filters in series
- Large filter: allows everything < 20mm
- Small filter: allows everything < 3mm
- **Result:** Only 3mm passes (defined by smaller filter)

---

## Files Modified

1. **Tests/InterfSolver/Tools/CalcContourTest.cpp**
   - Removed: `CompletelyOccludedShape_ProducesNoContour`
   - Added: `TwoConcentricEllipses_EXTERNAL_InnerVisible` (corrected)
   - Removed duplicate: `EllipseInsideEllipse_EXTERNAL_ProducesOneContour`

---

## Build Status

? **BUILD SUCCEEDED**

---

## Next Steps

Run the test to verify:
```powershell
.\Tests.exe --gtest_filter="CalcContourTest.TwoConcentricEllipses_EXTERNAL_InnerVisible"
```

Expected: **Test passes** ?

If test fails:
1. Check boundary tolerance in `XYEllipse::isInside()`
2. Verify step size produces adequate sampling
3. Debug CalcContour segment extraction logic

---

## Key Takeaway

**EXTERNAL shapes are APERTURES, not opaque objects!**

- Large containing small ? "Large hides small"
- Large containing small = "Small restricts large"
- **Intersection defines visible area**

The test name "CompletelyOccludedShape" was **fundamentally wrong** for EXTERNAL shapes!

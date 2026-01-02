# CalcContourDegenerateTest Fixes

## Summary

Fixed **2 tests** in `CalcContourDegenerateTest.cpp` that had incorrect expectations based on misunderstanding of the correct XYShape visibility logic.

---

## Correct XYShape Visibility Logic

### For EXTERNAL Shapes (Apertures)
```cpp
bool isVisible(point) {
    if (isInside(point))
        return true;   // Inside aperture ? visible
    else
        return false;  // Outside aperture ? blocked
}
```

### For INTERNAL Shapes (Obstructions)
```cpp
bool isVisible(point) {
    if (isInside(point))
        return false;  // Inside obstruction ? blocked
    else
        return true;   // Outside obstruction ? visible
}
```

### Combined Visibility (isPupil)

**A point is visible if ALL shapes say it's visible:**

```cpp
bool isPupil(point, shapes) {
    for (shape : shapes) {
        if (!shape.isVisible(point))
            return false;  // Blocked by this shape
    }
    return true;  // Visible through all shapes
}
```

**Visible area formula:**
```
visible_area = intersection(ALL EXTERNAL) - union(ALL INTERNAL)
```

For multiple EXTERNAL shapes:
```
visible_area = EXTERNAL? ? EXTERNAL? ? ... ? EXTERNAL?
```

---

## Test Fixes

### Test 1: `ConnectSegments_ValidatesMinimumPoints` ? `ConnectSegments_FiltersOutDegenerateTwoPoints`

#### Problem
Test expected `ConnectSegments` to produce a polygon from a 2-point broken line, but the new implementation **filters out degenerate polygons**.

#### Old Test (WRONG)
```cpp
TEST_F(CalcContourDegenerateTest, ConnectSegments_ValidatesMinimumPoints) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    XYBrokenLine twoPointSegment;
    twoPointSegment.Add(XYPoint(0.0, 0.0));
    twoPointSegment.Add(XYPoint(1.0, 0.0));
    arrBLn.Add(twoPointSegment);
    
    ConnectSegments(arrBLn, arrCont, 0.1);
    
    // ? WRONG: Expected polygon to be created
    ASSERT_EQ(arrCont.GetSize(), 1);
    EXPECT_TRUE(arrCont[0].GetSize() >= 2);
}
```

#### New Test (CORRECT)
```cpp
TEST_F(CalcContourDegenerateTest, ConnectSegments_FiltersOutDegenerateTwoPoints) {
    CArrayXYBrokenLine arrBLn;
    CArrayXYPolygon arrCont;
    
    // Create single 2-point segment (degenerate)
    XYBrokenLine twoPointSegment;
    twoPointSegment.Add(XYPoint(0.0, 0.0));
    twoPointSegment.Add(XYPoint(1.0, 0.0));
    arrBLn.Add(twoPointSegment);
    
    ConnectSegments(arrBLn, arrCont, 0.1);
    
    // ? CORRECT: Degenerate polygon should be filtered out
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "2-point degenerate polygon should be filtered out";
}
```

#### Reason
`ConnectSegments` now calls `XYPolygon::isDegenerate()` and **skips** adding degenerate polygons:

```cpp
auto Plg = XYPolygon(CurCont);
if (!Plg.isDegenerate())  // NEW: Filter degenerate
    ArrCont.Add(Plg);
```

For a 2-point polygon:
- `GetSize() < 3` ? `isDegenerate() = true`
- Polygon is NOT added to output

---

### Test 2: `ExternalApertures_PreserveVisibleRegions` ? Split into Two Tests

#### Problem
Test expected two **non-overlapping** EXTERNAL apertures to produce **2 separate contours**, but correct logic requires points to be **inside ALL EXTERNAL shapes** simultaneously.

#### Old Test (WRONG)
```cpp
TEST_F(CalcContourDegenerateTest, ExternalApertures_PreserveVisibleRegions) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures that don't overlap
    XYEllipse ellipse1(5.0, 4.0, -10.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(5.0, 4.0, 10.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // ? WRONG: Expected 2 separate contours
    EXPECT_EQ(arrCont.GetSize(), 2);
    
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GT(arrCont[i].GetSize(), 50);
    }
}
```

#### Why It's Wrong

**Scenario:**
- `ellipse1` centered at `(-10, 0)` with radii `(5, 4)`
- `ellipse2` centered at `(10, 0)` with radii `(5, 4)`
- Distance between centers: **20 units**
- Ellipses **DO NOT OVERLAP**

**Visible area calculation:**
```
visible_area = ellipse1 ? ellipse2 = ? (EMPTY!)
```

**Why empty:**
- Point at `(-10, 0)` (center of ellipse1):
  - `ellipse1.isVisible()` = `true` (inside ellipse1)
  - `ellipse2.isVisible()` = `false` (outside ellipse2, distance = 20)
  - `isPupil()` = `false` (not visible through all shapes)

- Point at `(10, 0)` (center of ellipse2):
  - `ellipse1.isVisible()` = `false` (outside ellipse1, distance = 20)
  - `ellipse2.isVisible()` = `true` (inside ellipse2)
  - `isPupil()` = `false` (not visible through all shapes)

**No point exists that is inside BOTH ellipses!**

#### New Tests (CORRECT)

**Test 2a: Non-Overlapping Case**
```cpp
TEST_F(CalcContourDegenerateTest, ExternalApertures_NonOverlapping_EmptyVisibleArea) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures that DON'T overlap
    // Visible area = intersection of apertures = EMPTY
    XYEllipse ellipse1(5.0, 4.0, -10.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(5.0, 4.0, 10.0, 0.0, 0.0, EXTERNAL);
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // ? CORRECT: Non-overlapping ? no visible area ? no contours
    EXPECT_EQ(arrCont.GetSize(), 0)
        << "Non-overlapping EXTERNAL apertures should produce empty visible area";
}
```

**Test 2b: Overlapping Case**
```cpp
TEST_F(CalcContourDegenerateTest, ExternalApertures_Overlapping_ProducesContour) {
    CArrayXYEllipse arrEll;
    CArrayXYRect arrRect;
    CArrayXYPolygon arrPlg;
    CArrayXYPolygon arrCont;
    
    // Two EXTERNAL apertures that DO overlap
    // Visible area = intersection of apertures
    XYEllipse ellipse1(10.0, 8.0, 0.0, 0.0, 0.0, EXTERNAL);
    XYEllipse ellipse2(10.0, 8.0, 2.0, 1.0, 0.0, EXTERNAL);  // Overlapping
    
    arrEll.Add(ellipse1);
    arrEll.Add(ellipse2);
    
    CalcContour(arrEll, arrRect, arrPlg, arrCont, 200);
    
    // ? CORRECT: Overlapping ? visible intersection area ? contours
    EXPECT_GT(arrCont.GetSize(), 0)
        << "Overlapping EXTERNAL apertures should produce visible area";
    
    // All contours should be valid
    for (int i = 0; i < arrCont.GetSize(); i++) {
        EXPECT_GE(arrCont[i].GetSize(), MIN_VIABLE_POLYGON_POINTS)
            << "Contour " << i << " is degenerate";
        EXPECT_FALSE(IsDegenerate(arrCont[i]))
            << "Contour " << i << " is degenerate";
    }
}
```

---

## Visual Examples

### Non-Overlapping EXTERNAL Shapes

```
        [Ellipse 1]              [Ellipse 2]
            ???                      ???
          ???????                  ???????
         ?????????                ?????????
          ???????                  ???????
            ???                      ???
        at (-10,0)                at (10,0)

Visible area = Ellipse1 ? Ellipse2 = ? (EMPTY)
Expected contours: 0
```

### Overlapping EXTERNAL Shapes

```
        [Ellipse 1]
            ???
          ???????
         ????[????]? ? Overlap region
          ???????????
            ???????
              ???
            [Ellipse 2]

Visible area = Ellipse1 ? Ellipse2 = Overlap region
Expected contours: 1 (contour of overlap)
```

---

## Key Insight: Multiple EXTERNAL Shapes

**OLD (WRONG) thinking:**
> "Each EXTERNAL shape defines its own visible region, so we get multiple separate contours"

**NEW (CORRECT) thinking:**
> "Multiple EXTERNAL shapes define a **single visible region** = their **intersection**, so we get 0 or 1 contour"

### Real-World Analogy

**Multiple EXTERNAL apertures = Multiple overlapping masks:**
- Light must pass through **ALL** masks simultaneously
- Visible area = where **ALL** masks are transparent (intersection)
- Non-overlapping masks ? **no light passes through** ? black screen

**Example:**
- Mask 1: circular hole on left side
- Mask 2: circular hole on right side
- **Result:** No light! (holes don't overlap)

---

## Tests Unchanged (Already Correct)

The following tests already had correct expectations:

? `DetectDegeneratePolygon_TwoSlightlyDifferentEllipses` - Tests NFi variations  
? `AllContoursHaveMinimumPoints` - Validates >= 3 points  
? `AllContoursHaveValidBounds` - Validates non-zero area  
? `PerimeterCalculation_StableAcrossPrecision` - Tests stability  
? `PointCountConsistency_NFi500vs501` - Tests consistency  
? `VisibilityFiltering_NotOverlyAggressive` - Tests filtering  
? `VerySmallStep_ProducesValidPolygons` - Edge case  
? `VeryLargeStep_StillProducesValidPolygons` - Edge case  
? `TinyOverlappingEllipses_NoDegenerate` - Small shapes  
? `BugScenario_TwoExternalApertures_DifferentSizes` - Bug regression  
? `BugScenario_OnlyTwoVisiblePoints` - Bug regression  
? `PolygonConstructor_RejectsDegenerate` - Documents behavior  
? `RealWorld_MultipleApertures_NoDegenerate` - Integration test  

---

## Build & Test

### Build
```powershell
# Build solution
msbuild Digit.sln /p:Configuration=Debug
```

### Run Tests
```powershell
# Run all CalcContourDegenerateTest tests
.\Tests.exe --gtest_filter=CalcContourDegenerateTest.*
```

### Expected Results

All tests should now **PASS** ?

---

## Files Modified

1. **Tests/InterfSolver/Tools/CalcContourDegenerateTest.cpp**
   - Fixed `ConnectSegments_ValidatesMinimumPoints` ? `ConnectSegments_FiltersOutDegenerateTwoPoints`
   - Fixed `ExternalApertures_PreserveVisibleRegions` ? Split into two tests:
     - `ExternalApertures_NonOverlapping_EmptyVisibleArea`
     - `ExternalApertures_Overlapping_ProducesContour`

---

## Related Documentation

- `CALCCONTOUR_FIXES_APPLIED.md` - CalcContour test fixes
- `ISPUPIL_TEST_FIXES.md` - isPupil test logic fixes
- `XYSHAPE_TEST_SUITE_COMPLETE.md` - XYShape comprehensive test suite
- `XYSHAPE_PHASE4_COMPLETE.md` - XYPolygon refactoring completion

---

## Conclusion

The `CalcContourDegenerateTest` suite now correctly validates:

1. ? **Degenerate polygon filtering** - `ConnectSegments` properly filters out < 3 point polygons
2. ? **Multiple EXTERNAL aperture logic** - Correctly expects intersection, not union
3. ? **Empty visible area handling** - Non-overlapping apertures produce no contours
4. ? **Overlapping aperture handling** - Overlapping apertures produce valid contours

All tests are now **consistent with the correct XYShape visibility semantics**!

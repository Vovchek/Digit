# CalcContour Degenerate Polygon Bug - Root Cause Analysis

## Bug Description

**Symptom**: Application crashes when processing DOS ZAP file with two slightly different EXTERNAL apertures after increasing PI precision to full double precision.

**Trigger Condition**: 
- Perimeter calculation changes slightly with higher precision PI
- NFi (number of points) decreases from 501 to 500 for first contour
- Only 2 points on first ellipse considered visible
- Results in degenerate 2-point polygon
- Subsequent code fails due to empty/invalid bounds

## Root Cause Chain

### 1. **Floating-Point Precision Cascade**

```cpp
// In XYEllipse::Perimeter()
constexpr double dPI = 3.14159265358979323846;  // High precision
double Perim = dPI * (1.5 * (Ax + By) - sqrt(Ax * By));

// In XYEllipse::GetContour(XYBrokenLine &BLine, double Step)
int NFi = static_cast<int>(Perim / Step);  // TRUNCATION HERE
```

**Problem**: 
- With low-precision PI: `Perim / Step = 501.xxx` → `NFi = 501`
- With high-precision PI: `Perim / Step = 500.xxx` → `NFi = 500`
- Integer truncation causes off-by-one error in point count

### 2. **Visibility Filtering Fragility**

For two slightly different EXTERNAL ellipses:

```cpp
// In CalcContour - for first ellipse
extractVisibleSegments(CurCont, [&](const XYPoint& pt) {
  return isPupil(pt, ArrEll, iElm) && isPupil(pt, ArrRect) && isPupil(pt, ArrPlg);
});
```

**EXTERNAL shape semantics** (as per user specification):
- Points INSIDE shape → visible
- Points OUTSIDE shape → NOT visible

**The problem**:
When checking visibility of points on Ellipse[0]:
1. Point is ON boundary of Ellipse[0] (technically inside)
2. Check `isPupil(pt, ArrEll, 0)` → excludes Ellipse[0], checks Ellipse[1]
3. Point is OUTSIDE Ellipse[1] (different size/position)
4. For EXTERNAL shape, outside points are NOT visible → `isVisible()` returns `false`
5. Therefore `isPupil()` returns `false`
6. **MOST points filtered out as invisible!**

### 3. **The NFi=500 vs 501 Critical Difference**

With **NFi = 501** (low-precision PI):
- 501 points distributed around ellipse
- Some points (perhaps at cardinal angles) happen to be slightly MORE inside Ellipse[0]
- Due to floating-point rounding, these points might pass the `isVisible()` check for Ellipse[1]
- Result: Enough visible points to form valid contour

With **NFi = 500** (high-precision PI):
- 500 points distributed around ellipse
- Angular spacing changes: `dTh = 2π / 500` vs `dTh = 2π / 501`
- Different points sampled at different angles
- Fewer points happen to be in "safe zone"
- Result: **Only 2 points remain visible** → degenerate polygon

### 4. **Degenerate Polygon Propagation**

```cpp
// In ConnectSegments
if (NBLn == 1) {
    XYPolygon Plg = XYPolygon(ArrBLn[0]);  // Creates 2-point polygon
    ArrCont.Add(Plg);
    return; 
}
```

**XYPolygon constructor behavior**:
```cpp
XYPolygon :: XYPolygon(const XYBrokenLine &A, ...) : XYBrokenLine (A)
{
  int NPnt = A.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);  // Attempts to close
  // ...
}
```

A 2-point "polygon":
- Point[0] = (x1, y1)
- Point[1] = (x2, y2)  
- Constructor adds Point[0] again if not already closed
- Results in 2 or 3 points forming a LINE, not a polygon

### 5. **Classification Logic Failure**

```cpp
// Mark all contours that aren't inside others as EXTERNAL
for (iElm = 0; iElm < NCont; iElm++) {
    P = ArrCont[iElm][0];  // Takes first point of degenerate polygon
    isInsideAny = false;
    for (i = 0; i < NCont; i++) {
        if (i == iElm) continue;
        if (ArrCont[i].isInside(P)) {  // Checks if point inside other contour
            isInsideAny = true;
            break;
        }
    }
    ArrCont[iElm].SetTypeLimits(isInsideAny ? INTERNAL : EXTERNAL);
}
```

**Problem with degenerate polygon**:
- `ArrCont[i].isInside(P)` uses [winding number algorithm](https://en.wikipedia.org/wiki/Point_in_polygon#Winding_number_algorithm)
- For 2-point "polygon", winding number calculation fails
- May return incorrect results
- Marks degenerate polygon as EXTERNAL

### 6. **Downstream Crash**

Code that uses the contours expects:
- Valid polygons with ≥3 distinct points
- Ability to compute bounds: `(XMin, YMin, XMax, YMax)`
- 2-point polygon has **zero area bounds**: `XMin=XMax`, `YMin=YMax`

```cpp
// Somewhere in downstream code (GetBounds, etc.)
XYBounds GetBounds() {
  // ...
  double XMin = -E18;  // Initialized to wrong extreme!
  double XMax = E18;
  // With 2 points, may not update properly
  // Results in invalid bounds
}
```

## Why This Happens

The bug is a **perfect storm** of multiple issues:

1. **Floating-point truncation** in point count calculation
2. **Overly aggressive visibility filtering** for EXTERNAL shapes
3. **Lack of validation** for minimum viable polygon size
4. **No guard rails** against degenerate geometries
5. **Implicit assumption** that contours always have many points

## Visualization of the Problem

```
Low Precision PI (501 points):
Ellipse 0:  o-o-o-o-o-o-o...o (many points visible)
                ↓
           [Valid contour with 200+ points]

High Precision PI (500 points):  
Ellipse 0:  o-x-x-x-x-x-x...o (only 2 visible!)
            ↑               ↑
         visible         visible
                ↓
           [Degenerate 2-point "polygon"]
                ↓
           [CRASH - invalid bounds]
```

## The Real Root Cause

**The fundamental issue is NOT the PI precision**, but rather:

1. **Brittle visibility logic**: For slightly overlapping EXTERNAL apertures, the visibility test is too strict
2. **No minimum point threshold**: Code assumes contours will always have "enough" points
3. **Integer truncation sensitivity**: Small perimeter changes cause discrete point count jumps
4. **Lack of degenerate case handling**: No validation that polygon has ≥3 points

## Why Tests Should Catch This

The tests should verify:

1. **Degenerate input handling**: What happens with 0, 1, 2, or 3 points?
2. **Visibility logic consistency**: Do EXTERNAL shapes correctly identify visible regions?
3. **Point count stability**: Does small perimeter change cause dramatic point loss?
4. **Polygon validation**: Are all output polygons geometrically valid?
5. **Bounds calculation safety**: Do 2-point polygons have valid bounds?

## Recommendation

The bug reveals **fundamental assumptions** that need validation:

- Assumption: "Contours will always have many points" → **FALSE**
- Assumption: "Visibility logic will keep most points" → **FALSE for edge cases**
- Assumption: "Polygon constructor validates input" → **FALSE**
- Assumption: "Integer truncation is harmless" → **FALSE with boundary values**

**Tests must validate these assumptions explicitly.**

# Analysis: How Bounds Are Used to Limit Working Area in CreateRedCenters()

## Overview

`CreateRedCenters()` is the core function that detects interference fringe extremums (bright/dark centers) in the interferogram image. It uses **boundary contours** (EXTERNAL aperture and INTERNAL obstruction) to define the working area and limit pixel processing to only the valid measurement region.

---

## Data Flow: Bounds ? buf_line ? Pixel Processing

### Step 1: Boundary Definition (Before CreateRedCenters)

**Location**: `CDigitInfo::Auto()` ? `CreateBufLine()`

```cpp
void CDigitInfo::Auto() {
    CreateBufLine();        // ? Step 1: Define working area
    CreateRedCenters();     // ? Step 2: Find fringes within bounds
    // ...
}
```

### Step 2: Create buf_line Structure

**Purpose**: Pre-compute left/right X boundaries for each Y scan line

**Structure**: `buf_line[y][4]`
```cpp
buf_line[i][0] = left_x_aperture;    // EXTERNAL left boundary
buf_line[i][1] = right_x_aperture;   // EXTERNAL right boundary  
buf_line[i][2] = left_x_obstruction; // INTERNAL left boundary (-1 if none)
buf_line[i][3] = right_x_obstruction;// INTERNAL right boundary (-1 if none)
```

---

## buf_line Creation: Two Scenarios

### Scenario A: Simple Shapes (Circle, Ellipse, Rectangle)

**Function**: `CreateBufLineApertureSimple()` / `CreateBufLineObstructionSimple()`

**Process**:
1. Get bounding rectangle from `GetExtCorBound()`
2. For circles/ellipses: Use parametric equations to compute left/right X for each Y
3. For rectangles: Use constant left/right X values

**Example for Ellipse**:
```cpp
void CreateBufLineApertureSimple() {
    CRect BoundR;
    pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
    
    int ny = BoundR.Height() + 1;
    Init_buf_line(ny, 4);  // Allocate buf_line[ny][4]
    
    // For ELLIPSE: compute X from ellipse equation
    for (int i = 0; i < ny; i++) {
        y = BoundR.top + i;
        x = sqrt((1. - y?/b?) * a?);  // Ellipse equation
        
        buf_line[i][0] = center.x - x;  // Left boundary
        buf_line[i][1] = center.x + x;  // Right boundary
        buf_line[i][2] = -1;            // No obstruction
        buf_line[i][3] = -1;
    }
}
```

**Diagram**:
```
Image coordinates:
????????????????????????????
?                          ?
?    ????????????????      ? ? BoundR.top (begY)
?   ?                ?     ?
?  ?  <-- Aperture --> ?   ?
?  ?    EXTERNAL       ?   ?   For each Y:
?   ?                ?     ?   buf_line[i][0] = left_x
?    ????????????????      ?   buf_line[i][1] = right_x
?                          ?
????????????????????????????
```

---

### Scenario B: Complex Shapes (Polygon, Multiple Contours)

**Function**: `CreateBufLineApertureComplex()` / `CreateBufLineObstructionComplex()`

**Process**:
1. Get contour parts using `GetPartsOfContours(EXTERNAL/INTERNAL)`
2. For each Y line, scan pixels left-to-right and right-to-left
3. Use `isPupil()` to test if each pixel is inside the valid region
4. Record first and last visible pixels as boundaries

**Key Logic**:
```cpp
void CreateBufLineApertureComplex() {
    // Get EXTERNAL contour shapes
    CArrayXYEllipse ArrEll;
    CArrayXYRect ArrRect;
    CArrayXYPolygon ArrPlg;
    pB->GetPartsOfContours(EXTERNAL, ArrEll, ArrRect, ArrPlg);
    
    for (iy = t_y; iy < b_y; iy++) {
        l_b = r_b = -1;
        
        // Scan left-to-right to find first visible pixel
        for (ix = l_x; ix < r_x; ix++) {
            P.X = ix; P.Y = iy;
            if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
                l_b = ix;
                break;
            }
        }
        
        // Scan right-to-left to find last visible pixel
        for (ix = r_x - 1; ix > l_x - 1; ix--) {
            P.X = ix; P.Y = iy;
            if (isPupil(P, ArrEll, ArrRect, ArrPlg)) {
                r_b = ix;
                break;
            }
        }
        
        buf_line[iy - t_y][0] = l_b;
        buf_line[iy - t_y][1] = r_b;
    }
}
```

**What is `isPupil()`?**
- Tests if a point is **visible** (not occluded by shapes)
- For EXTERNAL shapes: Returns TRUE if point is **outside** all shapes
- For INTERNAL shapes: Returns TRUE if point is **inside** the shape
- Considers `TypeLimits` (EXTERNAL/INTERNAL) via `shape.isVisible(P)`

**Diagram for Complex Polygon**:
```
Scan each Y line:
         ??
        ?  ?
       ?    ?___
      ?          ?
     ?    hole    ?
    ?   ??????    ?
    ?   ? INTERNAL?
    ?   ??????   ?
     ?          ?
      ?________?

For Y=100:
  Scan ?  Find first isPupil=true ? l_b
  Scan ?  Find last isPupil=true  ? r_b
```

---

## Step 3: Using buf_line in CreateRedCenters()

### Boundary Setup

```cpp
void CreateRedCenters() {
    CRect BoundR;
    pB->GetExtCorBound(pB->ExtBoundType, xDIB, yDIB, BoundR, FALSE, TRUE);
    
    int begY = BoundR.top;     // Start Y
    int endY = BoundR.bottom;  // End Y
    int ny = BoundR.Height() + 1;
    
    Sections.SetSize(ny);  // One section per Y line
```

### Y-Loop: Process Only Bounded Region

```cpp
    for (int iy = begY; iy < endY; iy++) {
        int n = -1;
        
        // Read entire image row into line[] buffer
        for (auto iCol = 0; iCol < pI->m_pDIB->m_dwWidth; iCol++) {
            idx = ((yDIB - iy) * pI->m_pDIB->m_dwWidth + iCol);
            Pixel = pI->m_pDIB->m_lpSrcBits[idx];
            ++n;
            
            // *** CRITICAL: Apply bounds-based filtering ***
            if (nContours > 2) {
                P.X = iCol; P.Y = iy;
                if (isPupil(P, pB->ArrEll, pB->ArrRect, pB->ArrPlg))
                    line[n] = inv_line[n] = ((unsigned char)rgbPix.rgbRed);
                else
                    line[n] = inv_line[n] = 0;  // ? Zero out pixels outside bounds
            }
            else {
                line[n] = inv_line[n] = ((unsigned char)rgbPix.rgbRed);
            }
        }
```

**Key Insight**: 
- **If `nContours > 2`**: Use `isPupil()` to mask pixels outside working area
- **Else**: Process entire row (simple bounds, no masking needed)

---

### Section Formation with buf_line

```cpp
        int i = iy - begY;
        
        // Set section boundaries from buf_line
        Sections[i].L.P1.y = Sections[i].L.P2.y = iy;
        Sections[i].L.P1.x = buf_line[i][0];  // ? Left boundary
        Sections[i].L.P2.x = buf_line[i][1];  // ? Right boundary
        
        // Detect fringe centers within bounds
        if (pCtrls->FringeCenterAs == FC_MAX) {
            fon_del(line, 0, n);
            Sections[i].Form(i, line, n, ny, buf_line);  // ? buf_line passed in!
        }
```

**What `Sections[i].Form()` Does**:
1. Analyzes `line[]` buffer (already masked by bounds)
2. Uses `buf_line[i][0]` and `buf_line[i][1]` to know valid X range
3. Detects local maxima/minima as fringe centers
4. Populates `Sections[i].NumLines[]` with detected fringes
5. Each fringe stored as `CNumLine` with `redX` position

---

## Boundary Enforcement: Three Levels

### Level 1: buf_line Pre-computation (Geometric)
- **When**: Before pixel processing
- **How**: Compute exact left/right X boundaries per Y line
- **Result**: `buf_line[y][0..3]` array

### Level 2: Pixel Masking (isPupil)
- **When**: During pixel read in CreateRedCenters()
- **How**: Zero out pixels outside `isPupil()` region
- **Result**: `line[]` buffer contains only valid pixels

### Level 3: Section Bounds (Logical)
- **When**: During fringe detection in `Section.Form()`
- **How**: Use `buf_line[i][0]` and `buf_line[i][1]` as min/max X
- **Result**: Fringes only detected within `[L.P1.x, L.P2.x]` range

---

## Why Three Levels?

### Redundancy for Robustness
1. **Geometric** (buf_line): Fast lookup, no computation needed per pixel
2. **Pixel-level** (isPupil): Handles complex shapes that can't be pre-computed simply
3. **Logical** (Section bounds): Ensures fringe detection algorithms stay within limits

### Performance Optimization
- **Simple shapes**: Use buf_line only (fast parametric equations)
- **Complex shapes**: Use isPupil() masking (precise but slower)

---

## Example Workflow: Aperture + Obstruction

### Setup
```
EXTERNAL aperture: Circle (radius 100, center 200,200)
INTERNAL obstruction: Circle (radius 30, center 200,200)
```

### buf_line for Y=200 (center line)
```cpp
buf_line[100][0] = 200 - 100 = 100  // Aperture left
buf_line[100][1] = 200 + 100 = 300  // Aperture right
buf_line[100][2] = 200 - 30  = 170  // Obstruction left
buf_line[100][3] = 200 + 30  = 230  // Obstruction right
```

### Pixel Processing
```
For Y=200, X scanning:
  X=0..99:    Outside aperture ? line[x] = 0 (masked)
  X=100..169: Inside aperture ? line[x] = pixel_value
  X=170..230: Inside obstruction ? line[x] = 0 (masked)
  X=231..300: Inside aperture ? line[x] = pixel_value
  X=301..399: Outside aperture ? line[x] = 0 (masked)
```

### Section Bounds
```cpp
Sections[100].L.P1.x = 100  // Process from X=100
Sections[100].L.P2.x = 300  // Process to X=300
// (Obstruction handled by pixel masking in line[] buffer)
```

### Detected Fringes
```cpp
Sections[100].NumLines[0].redX = 120  // Fringe at X=120
Sections[100].NumLines[1].redX = 150  // Fringe at X=150
// (No fringes detected at X=170..230 due to obstruction)
Sections[100].NumLines[2].redX = 250  // Fringe at X=250
Sections[100].NumLines[3].redX = 280  // Fringe at X=280
```

---

## Code Verification Points

### 1. Check buf_line Validity
```cpp
// After CreateBufLine()
for (int i = 0; i < ny_buf_line; i++) {
    TRACE("Y=%d: aperture [%d,%d], obstruction [%d,%d]\n",
          begY + i, 
          buf_line[i][0], buf_line[i][1],
          buf_line[i][2], buf_line[i][3]);
}
```

### 2. Check isPupil Behavior
```cpp
// In CreateRedCenters() loop
if (nContours > 2) {
    P.X = iCol; P.Y = iy;
    bool visible = isPupil(P, pB->ArrEll, pB->ArrRect, pB->ArrPlg);
    TRACE("Pixel (%d,%d): %s\n", iCol, iy, visible ? "VISIBLE" : "MASKED");
}
```

### 3. Check Section Bounds
```cpp
// After section formation
for (int i = 0; i < Sections.GetSize(); i++) {
    TRACE("Section %d (Y=%d): X range [%.0f, %.0f], fringes=%d\n",
          i, 
          (int)Sections[i].L.P1.y,
          Sections[i].L.P1.x, 
          Sections[i].L.P2.x,
          Sections[i].NumLines.GetSize());
}
```

---

## Summary: Boundary Usage Lifecycle

| Stage | Function | Boundary Representation | Purpose |
|-------|----------|------------------------|---------|
| **1. Definition** | `CreateBufLine()` | Vector contours (ArrEll, ArrRect, ArrPlg) | Define working area geometry |
| **2. Pre-computation** | `CreateBufLineAperture/Obstruction()` | `buf_line[y][0..3]` array | Pre-compute X boundaries per Y |
| **3. Pixel Masking** | `CreateRedCenters()` pixel loop | `isPupil()` boolean test | Zero pixels outside bounds |
| **4. Section Setup** | `Sections[i].L.P1.x/P2.x` | Horizontal line segment | Define fringe detection range |
| **5. Fringe Detection** | `Sections[i].Form()` | Uses `buf_line` in algorithm | Detect extremums within bounds |
| **6. Storage** | `Sections[i].NumLines[].redX` | Fringe X positions | Store only valid fringes |

---

## Key Takeaways

? **buf_line** is a **scan-line boundary lookup table** that defines valid X ranges for each Y
? **isPupil()** provides **point-in-contour testing** for complex polygon shapes
? **Three-level enforcement** ensures no fringe detection outside working area
? **Performance optimized**: Simple shapes use fast buf_line, complex shapes use isPupil()
? **Robustness**: Multiple redundant checks prevent processing invalid regions

---

## Related Issues (CalcContour Test Failures)

The `isPupil()` function used here is the **same one** causing issues in CalcContour tests:
- Tests with INTERNAL shapes produce 0 contours
- Root cause: `isPupil()` visibility logic for INTERNAL TypeLimits

**Current behavior**:
```cpp
bool XYShape::isVisible(const XYPoint &P) const {
    bool isIn = isInside(P);
    if (isIn && TypeLimits == INTERNAL)
        return false;  // Inside INTERNAL ? NOT visible
    else if (!isIn && TypeLimits == EXTERNAL)
        return false;  // Outside EXTERNAL ? NOT visible
    return true;
}
```

**In CreateRedCenters context**:
- EXTERNAL aperture: `isPupil()` returns TRUE for pixels **outside** the shape (correct)
- INTERNAL obstruction: `isPupil()` returns FALSE for pixels **inside** the shape (correct)
- **Result**: Pixels inside obstruction are masked to 0 ?

**In CalcContour context** (the bug):
- INTERNAL hole contour points should be **visible** (they form the hole boundary)
- But `isPupil()` returns FALSE because points are inside INTERNAL shape
- **Result**: All contour points filtered out ? 0 contours ?

**Conclusion**: The `isPupil()` logic works correctly for **pixel masking** in CreateRedCenters, but fails for **contour generation** in CalcContour!

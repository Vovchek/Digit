# Documentation Correction: ZAP Sections Orientation

## Critical Error Fixed ✅

### Original Error ❌
**Incorrectly stated**: ZAP sections are **VERTICAL** scan lines  
**Incorrectly expanded**: ZAP = "Zonal Analysis Plane"

### Correct Information ✅
**ZAP sections are HORIZONTAL reference lines**  
**ZAP acronym**: Should NOT be expanded - just use "ZAP"

---

## What Was Fixed

### 1. Class-Level Documentation
**Before:**
```cpp
/**
 * **ZAP Section**: Vertical scan line across the interferogram...
 */
```

**After:**
```cpp
/**
 * **ZAP Section**: Horizontal reference line across the interferogram...
 */
```

### 2. Member Variable Documentation
**ZapLines member:**
- Fixed: "Vertical scan lines" → "Horizontal reference lines"
- Fixed: "Fringe identification by x-coordinate" → "Fringe identification by y-coordinate"

**idxDragZapLine member:**
- Fixed: "vertical scan lines" → "horizontal reference lines"

### 3. Method Documentation

#### CreateZAPSections()
**Before:**
```cpp
/**
 * 1. Divide aperture width into regular intervals
 * 2. Create vertical line at each interval
 */
```

**After:**
```cpp
/**
 * 1. Divide aperture height into regular intervals
 * 2. Create horizontal line at each interval
 */
```

#### SortZapLines()
**Before:**
```cpp
/**
 * @brief Sort ZapLines array by x-coordinate.
 * **Purpose**: Ensures left-to-right ordering for navigation
 */
```

**After:**
```cpp
/**
 * @brief Sort ZapLines array by y-coordinate.
 * **Purpose**: Ensures top-to-bottom ordering for navigation
 */
```

#### GetLockedZapSectionXYPos()
**Before:**
```cpp
/**
 * @param[out] P1 Top endpoint
 * @param[out] P2 Bottom endpoint
 */
```

**After:**
```cpp
/**
 * @param[out] P1 Left endpoint
 * @param[out] P2 Right endpoint
 * 
 * **Note**: ZAP sections are horizontal lines, so P1.y == P2.y
 */
```

#### SetLockedZapSectionYPos()
**Added clarification:**
```cpp
/**
 * @brief Update Y position of locked ZAP section.
 * 
 * @param iy New Y coordinate
 * 
 * **Usage**: Called during vertical drag of horizontal ZAP line
 */
```

---

## Technical Explanation

### Correct Understanding

**Scan Direction:**
- **Horizontal scan lines** (iy loop): Extract pixel intensities left-to-right at each y-position
- **Red centers detected**: Along each horizontal scan line
- **Fringes formed**: By connecting red centers vertically across scan lines

**ZAP Sections:**
- **Orientation**: HORIZONTAL lines (constant y-coordinate)
- **Purpose**: Reference lines at known y-positions
- **Navigation**: Top-to-bottom (sorted by y)
- **Dragging**: Vertical movement (changing y-coordinate)
- **Endpoints**: Left (x_min) to Right (x_max)

### Code Evidence

From `CreateRedCenters()`:
```cpp
for (int iy = begY; iy < endY; iy++) {
    // Horizontal scan line at y = iy
    for (int iCol = 0; iCol < width; iCol++) {
        // Scan left-to-right
        Pixel = pI->m_pDIB->m_lpSrcBits[idx];
        // ...
    }
    // Store scan line: Sections[i].L.P1.y = Sections[i].L.P2.y = iy
}
```

**Interpretation:**
- Outer loop (`iy`): Iterates vertically (top to bottom)
- Inner loop (`iCol`): Scans horizontally (left to right)
- Each `Sections[i]` represents ONE horizontal line
- ZAP sections are reference horizontal lines

---

## Impact on Other Documentation

### Files That May Need Review
- `Docs/DIGITINFO_DOCUMENTATION_SUMMARY.md` - Contains same errors
- Any other documentation mentioning ZAP sections
- Code comments in `DigitInfo.cpp` (check if consistent)

### Domain Terminology Update

**Correct Terminology:**
- **Scan line**: Horizontal line across interferogram
- **ZAP section**: Horizontal reference line (subset of scan lines)
- **Vertical connection**: Linking red centers across scan lines to form fringes
- **Horizontal extent**: Left to right span of a ZAP section
- **Vertical position**: Y-coordinate of a ZAP section

---

## Lessons Learned

### Documentation Best Practices
1. **Verify orientation** from code before documenting
2. **Don't expand acronyms** unless certain of meaning
3. **Check coordinate systems** (x/y, horizontal/vertical)
4. **Cross-reference code** to validate assumptions

### Code Analysis
Looking at the code first would have revealed:
- `iy` loop = vertical iteration (y-coordinates)
- `iCol` loop = horizontal iteration (x-coordinates)
- `Sections[i].L.P1.y == Sections[i].L.P2.y` = horizontal line
- `buf_line[i][0/1]` = left/right edges = horizontal span

---

## Corrected Files

| File | Status |
|------|--------|
| `DigitMode/DigitInfo.h` | ✅ Fixed |
| Build | ✅ Successful |

---

## Summary

**Error:** Documented ZAP sections as vertical instead of horizontal  
**Fix:** Corrected all occurrences in DigitInfo.h  
**Verification:** Build successful, documentation now accurate  
**Apology:** Sorry for the confusion caused by incorrect documentation!

---

**All ZAP section documentation now correctly describes them as HORIZONTAL reference lines across the interferogram.** ✅

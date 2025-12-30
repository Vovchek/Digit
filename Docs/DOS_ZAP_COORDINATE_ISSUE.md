# DOS ZAP Coordinate Transformation Issue

## Problem Summary

DOS ZAP files (`1KL112 saved old.zap`) fail to detect red centers properly. Only 9 centers are found near the bottom of the interferogram (Y=453-489), instead of thousands across the entire ellipse.

## Root Cause Analysis

### DOS ZAP Coordinate System
1. **Original DOS ZAP**: Y-coordinates stored with Y=0 at **bottom** (DOS screen coordinates)
2. **`ReadDosZAPData()`**: Inverts Y-coordinates using `InverseY(fakeHeight)` where `fakeHeight` is derived from bounds (468 pixels)
3. **After inversion**: Y=0 at **top** (Windows coordinates)

### Image Loading
1. **PCX file** (`1KL112.pcx`): Loaded with actual height **544 pixels**
2. **Coordinate mismatch**: ZAP file expects 468-pixel image, but actual image is 544 pixels tall

### Current Transformation Logic
```cpp
if (IntInfo.LoadedFileType == NUMBERING_INTERFEROGRAM_INFO::TYP_ZAP_DOS)
{
    int actualHeight = imageLoaded ? pI->ImageSize.cy : IntInfo.ImageSize[1];
    double dY = (actualHeight - IntInfo.ImageSize[1]);  // dY = 544 - 468 = 76
    
    IntInfo.DigitDat.ShiftY(dY);  // Shift vector data by +76
    IntInfo.EBnd.ShiftY(dY);
    IntInfo.ArrEll[i].ShiftY(dY);
}
```

This shifts the ellipse down by 76 pixels, but we don't know:
- **Where in the 544-pixel image** the 468-pixel region should be mapped
- Whether the PCX file contains the full interferogram or just a cropped region
- If the ZAP file was created with a **different image** than the current PCX

## Debug Evidence

### buf_line Setup (CORRECT)
```
CreateBufLineApertureSimple: ELLIPSE mode, naP=234, ny=469
First line buf_line[0]: [280,280]
Middle line buf_line[234]: [47,513]
Last line buf_line[468]: [280,280]
```
? `buf_line` is correctly filled for all 469 lines

### Ellipse Bounds (MATHEMATICALLY CORRECT)
```
After shift: Yc=267.611, Ax=233.891, By=233.891
BoundR: top=34, bottom=502 (height=468)
```
? Ellipse centered at Y=267 with radius ~234 gives bounds 34-502

### Red Center Detection (FAILS)
```
Processed 468 lines, 9 lines had centers
Centers found at lines: 419, 422, 451, 453, 455
Corresponding Y coordinates: 453, 456, 485, 487, 489
```
? Only 9 centers found near **bottom of ellipse** (Y=453-489)  
? Expected: Hundreds/thousands of centers across entire ellipse (Y=34-502)

## Hypothesis

The DOS ZAP file and PCX image are **misaligned**:
1. **Possibility A**: The actual fringe pattern in the PCX is at **different Y coordinates** than the shifted ellipse expects
2. **Possibility B**: The PCX file is a **different crop/resolution** than what the ZAP file was created with
3. **Possibility C**: The 76-pixel shift is **incorrect** - we should shift by a different amount or direction

## Proposed Solutions

### Option 1: Disable DOS ZAP Shift (Temporary)
Remove the coordinate shift logic entirely to see if the original (unshifted) coordinates work:
```cpp
// Comment out the shift for DOS ZAP
// if (IntInfo.LoadedFileType == NUMBERING_INTERFEROGRAM_INFO::TYP_ZAP_DOS) { ... }
```

### Option 2: Make Shift Amount Configurable
Let user adjust the shift amount interactively to find the correct alignment.

### Option 3: Use Actual Image Bounds
Instead of shifting, recalculate ellipse positions based on where the actual fringe pattern is detected in the image.

### Option 4: Investigate Win ZAP
Test if the **same PCX file** works correctly with a **Win ZAP** file to confirm the image itself is valid.

## Next Steps

1. **Test without shift**: Temporarily disable DOS ZAP coordinate shifting
2. **Compare with Win ZAP**: Load the same PCX with a Win ZAP file (if available)
3. **Visual inspection**: Manually check where fringes actually appear in the PCX file
4. **Review ZAP file creation**: Understand how the original DOS ZAP was created and with what image

## Files Affected

- `DigitMode/DigitInfo.cpp::LoadZAP()` - DOS ZAP coordinate transformation logic
- `InterfSolver/Tools/ReadWriteData.cpp::ReadDosZAPData()` - Y-axis inversion with fake image size

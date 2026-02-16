# Refactoring Plan Corrections - IBoundsProvider Removal

**Date**: Updated after architectural review

## Summary of Changes

The initial `DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md` contained a critical architectural flaw that has been corrected.

---

## The Flaw: IBoundsProvider Interface (REMOVED)

### What Was Wrong
- **Step 4** originally proposed creating `IBoundsProvider` interface
- This would have perpetuated dependency on deprecated `CBoundCtrls`
- **Why it's wrong**: Bounds are NOT algorithm inputs - they are inputs to the **mask generation** phase

### Correct Architecture
```
CBoundCtrls (deprecated)
    ↓
    ↓ Used ONCE to compute visibility mask
    ↓
CApertureCtrls::GetMaskProvider()
    ↓
    ↓ Algorithm queries mask for all spatial info
    ↓
Digitization Algorithm
    ├── IsVisiblePixel(x, y)?  [Queries mask]
    ├── GetScanlineRange(y)?    [Queries mask]
    └── Aperture center point   [Simple parameter for numbering]
```

**Key Insight**: The visibility mask ENCODES both aperture AND obstruction boundaries. Passing a bounds object would be redundant and wrong.

---

## Specific Changes Made to Plan

### 1. Removed Step 4 (IBoundsProvider)
- **Before**: Files 1-4 included creating `IBoundsProvider` interface
- **After**: Files 1-3 only (removed IBoundsProvider)
- **Result**: 19 files instead of 20

### 2. Simplified DigitizationInput (Step 1)
- **Before**: Included `bounds` and `apertureCenter` with `bool hasObstruction`
- **After**: Only includes:
  - `maskProvider` (visibility mask - the single source of spatial truth)
  - `apertureCenter` (simple `aperture::Point`, not bounds object)
  - `fringeCenterAs`, `contrastThreshold`, spacing parameters

**Code Before**:
```cpp
struct DigitizationInput {
    // ... other fields
    const aperture::visibility::VisibilityMaskProvider* maskProvider;
    int fringeCenterAs;
    double contrastThreshold;
    int minFringeSpacing, maxFringeSpacing;
    // ❌ NO bounds object needed
};
```

### 3. Updated FringeNumberer (Step 9)
- **Before**: `NumberFringes(polylines, apertureCenter, hasObstruction)`
- **After**: `NumberFringes(polylines, apertureCenter)`
- **Reason**: Obstruction info is implicit in the visibility mask (prior stages already respect it)

### 4. Updated StandardDigitizer (Step 12)
- **Before**: Queried bounds from mask: `auto apertureBounds = input.maskProvider->GetBounds()`
- **After**: Uses aperture center directly: `input.apertureCenter`
- **Reason**: Only the center point is needed for numbering reference

### 5. Completely Refactored Auto() (Step 14)
- **Before**: Called `GetBoundCtrls()` 
- **After**: NEVER calls `GetBoundCtrls()`
- **Dependencies now**:
  - `CImageCtrls*` (bitmap)
  - `CApertureCtrls*` (visibility mask)
  - `CControls*` (parameters)
  - ~~`CBoundCtrls*`~~ (ELIMINATED)

**Code After**:
```cpp
void CDigitInfo::Auto() {
    CImageCtrls* pI = GetImageCtrls();      // ✅ Needed
    CApertureCtrls* pA = GetApertureCtrls();   // ✅ Needed (mask)
    CControls* pCtrls = GetControls();      // ✅ Needed (params)
    // ✅ NO GetBoundCtrls() call!
    
    auto input = BuildDigitizationInput(pI, pA, pCtrls);
    auto output = strategy->Digitize(input);
    ConvertOutputToLegacyModel(output);
}
```

### 6. Updated File Structure
- **Files to Create**: 19 (removed IBoundsProvider)
- **Files to Modify**: 4 (removed reference to IBoundsProvider)
- **Key Benefit**: No need to maintain bounds interfaces

| Category | Before | After | Change |
|----------|--------|-------|--------|
| Algorithm Parameters | DigitizationParams.h | DigitizationParams.h | Simplified |
| Stages | 4 (unchanged) | 4 (unchanged) | No change |
| Strategy | 4 files (IDigitizationStrategy, StandardDigitizer, Factory, etc.) | 4 files | No change |
| Interfaces | 2 (IImageDataProvider, IBoundsProvider) | 1 (IImageDataProvider) | Removed IBoundsProvider |
| Tests | 8 tests | 9 tests | Added DigitizationInputBuildingTest |
| **TOTAL** | **20 files** | **19 files** | **-1 file** |

### 7. Updated Success Criteria
- **Added**: "✅ **NO CBoundCtrls dependency** in digitization algorithm"
- **Emphasis**: Complete decoupling from deprecated CBoundCtrls

---

## Why This Matters

### Architectural Purity
- The algorithm is now **purely functional**: bitmap + mask + parameters → results
- No dependency on domain objects (bounds)
- No possibility of cyclic dependencies

### CBoundCtrls Deprecation
- **Before correction**: CBoundCtrls would linger due to IBoundsProvider dependency
- **After correction**: CBoundCtrls can be fully deprecated once aperture editing is migrated
- **Timeline**: Algorithm can become independent in Phase 1-2

### Testability
- Algorithm stages can be tested with **ONLY**:
  - Synthetic bitmaps
  - Mock visibility masks
  - No need to mock complex bounds objects

### Future Evolution
- ML-based digitizer doesn't need bounds
- Adaptive algorithms don't need bounds
- Streaming/tiled algorithms don't need bounds
- All variants work with same input model

---

## How to Use the Corrected Plan

1. **Read Section 2.2** (Proposed Architecture) - now shows mask as single spatial reference
2. **Read Appendix A** (Critical Architectural Insight) - explains why bounds are not needed
3. **Follow Steps 1-20** - implementation is now architecturally correct
4. **Check Section 5** (Dependency Injection Plan) - now shows zero CBoundCtrls dependency

---

## Related Documentation

- **Main Plan**: `Docs/DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md`
- **Bounds Editing Architecture**: `Docs/BOUNDS_EDITING_ARCHITECTURE.md` (separate system, not affected)
- **Aperture System**: `Controls/CApertureCtrls.h` (source of visibility mask)

---

## Conclusion

The corrected plan achieves the original goals while being **architecturally pure**:

✅ Removes buf_line  
✅ Decouples from CDotInfo  
✅ Eliminates CBoundCtrls dependency  
✅ Enables strategy pattern  
✅ Improves testability  
✅ Maintains backward compatibility  

The algorithm now depends ONLY on:
- Bitmap data (pixels)
- Visibility mask (aperture + obstruction encoding)
- Simple parameters and aperture center point

This is the correct, minimal, and extensible design.

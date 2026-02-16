# Digitization Refactoring Plan - UPDATED

**Status**: ✅ UPDATED with scanLineIndex removal  
**Date**: Latest review  
**Change**: Removed redundant `scanLineIndex` field from `ExtremumPoint`

---

## What Was Updated

### Issue Identified
`ExtremumPoint::scanLineIndex` appeared to be an optimization, but:
- ✅ It's redundant with `position.y`
- ✅ No algorithmic benefit (both O(n) for grouping)
- ✅ Can be computed on-demand in O(1) time
- ✅ Violates single-source-of-truth principle

### Changes Made

**1. ExtremumPoint Struct (Step 1)**
- ❌ REMOVED: `int scanLineIndex;` field
- ✅ KEPT: Position, intensity, extremumType
- ✅ ADDED: Comment explaining computation: `scanLineIndex = (int)position.y - apertureTopY`

**2. FringeConnector Signature (Step 8)**
- ❌ REMOVED: `scanlineRanges` parameter
- ❌ REMOVED: `imageHeight` parameter (not needed for on-demand computation)
- ✅ ADDED: `GroupByScanLine()` helper method
- ✅ UPDATED: `ConnectExtrema()` to group internally

**3. StandardDigitizer (Step 12)**
- ✅ UPDATED: Call to `FringeConnector::ConnectExtrema()` with new signature

**4. Documentation (Appendix C)**
- ✅ NEW: Section explaining design rationale
- ✅ Code examples showing O(1) on-demand computation
- ✅ Comparison with pre-stored approach (same complexity)

---

## Files Updated

```
✅ Docs/DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md
   ├── Step 1: ExtremumPoint struct (removed scanLineIndex)
   ├── Step 8: FringeConnector (new signature, GroupByScanLine helper)
   ├── Step 12: StandardDigitizer (updated call)
   └── Appendix C: Design rationale (NEW)

✅ Docs/DIGITIZATION_SCANLINEINDEX_REMOVAL.md (NEW)
   ├── What changed (before/after)
   ├── Why it's better (SoT, simplicity)
   ├── Performance analysis (O(n) both ways)
   ├── Implementation notes
   └── Testing impact
```

---

## Key Points

### Architectural Principle Applied
**Don't store derived data if it's cheap to compute on-demand.**

### Implementation in FringeConnector
```cpp
std::map<int, std::vector<ExtremumPoint>> FringeConnector::GroupByScanLine(
    const std::vector<ExtremumPoint>& redCenters,
    int referenceY)
{
    std::map<int, std::vector<ExtremumPoint>> grouped;
    for (const auto& extremum : redCenters) {
        int scanLineIndex = static_cast<int>(extremum.position.y) - referenceY;
        grouped[scanLineIndex].push_back(extremum);
    }
    return grouped;
}
```

### Benefits
- ✅ Cleaner `ExtremumPoint` (3 fields, not 4)
- ✅ Single source of truth (position.y only)
- ✅ Same O(n) performance
- ✅ Easier to maintain and test
- ✅ No coupling to image height or aperture bounds

---

## What Stays the Same

- ✅ Stage 1: RedCenterDetector (outputs ExtremumPoint)
- ✅ Stage 2: FringeConnector (groups and connects)
- ✅ Stage 3: FringeNumberer (numbers polylines)
- ✅ Overall architecture: 3 pure stages + adapter
- ✅ All other data structures unchanged
- ✅ Strategy pattern remains
- ✅ Pure algorithm isolation maintained

---

## Test Strategy (No Changes)

Tests remain the same - grouping is now internal:

```cpp
TEST(FringeConnectorTest, ConnectsExtremaAcrossScanLines) {
    std::vector<ExtremumPoint> extrema = {
        {{10.0, 10.0}, 100, FC_MAX},
        {{11.0, 11.0}, 105, FC_MAX},
        {{12.0, 12.0}, 110, FC_MAX},
    };
    
    auto polylines = FringeConnector::ConnectExtrema(extrema, maskAccessor);
    
    // Test verifies connectivity, not internal scan line grouping
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].points.size(), 3);
}
```

---

## Summary

✅ **Plan is now clean and architecturally sound**
- No redundant data
- Single source of truth throughout
- O(n) complexity unchanged
- Simpler, more maintainable code

**Ready for implementation** with 19 files to create/modify, 20 implementation steps across 6 phases.

Timeline: **9-14 weeks** for complete refactoring and validation.

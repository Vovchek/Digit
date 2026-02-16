# Update Summary: Remove scanLineIndex Redundancy

**Date**: After architectural review with domain expert  
**Issue**: `ExtremumPoint::scanLineIndex` was redundant with `position.y`  
**Resolution**: Removed field, compute on-demand in O(1) time

---

## What Changed

### Before (Redundant)
```cpp
struct ExtremumPoint {
    CDPoint position;       // x, y in image coordinates
    int scanLineIndex;      // ← REDUNDANT (can be computed from position.y)
    double intensity;
    int extremumType;
};

// In FringeConnector
auto polylines = FringeConnector::ConnectExtrema(
    redCenters, 
    input.imageHeight,      // ← No longer needed
    maskAccessor
);
```

### After (Clean)
```cpp
struct ExtremumPoint {
    CDPoint position;       // x, y in image coordinates
    double intensity;
    int extremumType;
};

// In FringeConnector
auto polylines = FringeConnector::ConnectExtrema(
    redCenters, 
    maskAccessor
);

// Grouping happens inside FringeConnector
int scanLineIndex = static_cast<int>(extremum.position.y) - referenceY;  // O(1)
```

---

## Why This Is Better

### Single Source of Truth
- **Before**: Two representations of same information (position.y and scanLineIndex)
- **After**: Only position.y; scanLineIndex computed when needed

### No Performance Loss
Both approaches have identical complexity:
```
Grouping 1000 extrema by scan line:

Pre-stored:     for (auto& e : extrema) grouped[e.scanLineIndex].push_back(e);
                1000 direct array accesses = O(n)

On-demand:      for (auto& e : extrema) {
                    int idx = (int)e.position.y - refY;  // Single subtraction
                    grouped[idx].push_back(e);
                }
                1000 subtractions + 1000 array accesses = O(n)
```

The single subtraction is negligible compared to the vector push_back().

### Simpler Data Model
- ExtremumPoint has 3 fields instead of 4
- No synchronization issues if position.y changes
- Clearer intent: position is the ground truth

---

## Updated Files

### `DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md`

**Step 1** - Changed `ExtremumPoint` struct:
```cpp
struct ExtremumPoint {
    CDPoint position;       // x, y in image coordinates
    double intensity;
    int extremumType;
    // NOTE: No scanLineIndex - derive from position.y when needed
};
```

**Step 8** - Updated `FringeConnector`:
```cpp
class FringeConnector {
    // NEW signature - removed imageHeight and scanlineRanges
    static std::vector<FringePolyline> ConnectExtrema(
        const std::vector<ExtremumPoint>& redCenters,
        const VisibilityMaskAccessor& maskAccessor);
    
    // NEW helper method - groups extrema by scan line
    static std::map<int, std::vector<ExtremumPoint>> GroupByScanLine(
        const std::vector<ExtremumPoint>& redCenters,
        int referenceY);
};
```

**Step 12** - Updated `StandardDigitizer::Digitize`:
```cpp
auto polylines = FringeConnector::ConnectExtrema(
    redCenters, 
    maskAccessor  // imageHeight no longer needed
);
```

**Appendix C** - NEW section explaining the design decision

---

## Implementation Notes

When implementing FringeConnector, compute scanLineIndex as needed:

```cpp
std::map<int, std::vector<ExtremumPoint>> FringeConnector::GroupByScanLine(
    const std::vector<ExtremumPoint>& redCenters,
    int referenceY)
{
    std::map<int, std::vector<ExtremumPoint>> grouped;
    for (const auto& extremum : redCenters) {
        // Compute scanLineIndex on-demand (O(1) arithmetic)
        int scanLineIndex = static_cast<int>(extremum.position.y) - referenceY;
        grouped[scanLineIndex].push_back(extremum);
    }
    return grouped;
}
```

This could also be done with a vector of vectors if all scan lines are densely used:
```cpp
std::vector<std::vector<ExtremumPoint>> FringeConnector::GroupByScanLine(
    const std::vector<ExtremumPoint>& redCenters,
    int referenceY,
    int imageHeight)
{
    std::vector<std::vector<ExtremumPoint>> grouped(imageHeight);
    for (const auto& extremum : redCenters) {
        int scanLineIndex = static_cast<int>(extremum.position.y) - referenceY;
        if (scanLineIndex >= 0 && scanLineIndex < imageHeight) {
            grouped[scanLineIndex].push_back(extremum);
        }
    }
    return grouped;
}
```

---

## Testing Impact

**No test changes needed** - The grouping logic is internal to FringeConnector.

Tests for FringeConnector should:
- Verify polylines are correctly connected across scan lines
- Verify connection logic respects visibility mask
- No need to mock or check scanLineIndex values

Example:
```cpp
TEST(FringeConnectorTest, ConnectsExtremaAcrossScanLines) {
    // Create extrema at positions (x, y) across multiple scan lines
    std::vector<ExtremumPoint> extrema = {
        {{10.0, 10.0}, 100, FC_MAX},
        {{11.0, 11.0}, 105, FC_MAX},
        {{12.0, 12.0}, 110, FC_MAX},
    };
    
    auto polylines = FringeConnector::ConnectExtrema(extrema, maskAccessor);
    
    // Verify connected into single polyline
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].points.size(), 3);
}
```

---

## Documentation Updated

- ✅ `DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md` (main plan)
- ✅ `Appendix C` (design rationale)
- ✅ Code examples updated in Steps 1, 8, 12

---

## Architecture Summary

```
ExtremumPoint (Stage 1 output)
├── position (x, y)  ← Source of truth for scan line
├── intensity
└── extremumType

FringeConnector (Stage 2)
├── Input: vector<ExtremumPoint>
├── Internal: GroupByScanLine()
│   └── Computes scanLineIndex = (int)position.y - refY
├── Grouping: O(n)
└── Output: vector<FringePolyline>

Result: Single data source, no redundancy, no performance loss
```

---

## Conclusion

By removing the redundant `scanLineIndex` field, we achieve:
- ✅ Cleaner data model (3 fields instead of 4)
- ✅ Single source of truth (position.y only)
- ✅ Same O(n) performance for grouping
- ✅ Simpler maintenance and testing
- ✅ No algorithmic complexity increase

The architectural principle: **Don't store derived data, compute it on-demand if it's cheap to do so.**

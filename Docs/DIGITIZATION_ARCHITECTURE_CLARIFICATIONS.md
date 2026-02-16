# Digitization Plan Clarifications - Architecture Refinements

**Date**: After detailed architectural review with domain expert

## Summary of Critical Clarifications

The refactoring plan has been refined based on three critical architectural insights:

1. **NO ZAPSections** - ZAPLine concept is deprecated, not planned for future
2. **CFringeSegment is first-class** - Pure algorithm output is `std::vector<NumberedFringe>` (STL), then adapted to CFringeSegment via Stage 4 adapter
3. **Red dots for visualization** - ExtremumPoint data preserved in output for low-level algorithm visualization

---

## 1. Removal of ZAPSectionBuilder (OLD Step 10)

### What Changed
- **Before**: Step 10 created ZAPSectionBuilder to generate ZAP reference lines
- **After**: Step 10 is now FringeSegmentAdapter (pure → MFC bridge)

### Why
- ZAPLine concept is deprecated
- No future plans to use ZAPSections
- Removing this stage simplifies the pipeline

### Code Impact
- **Removed from DigitizationOutput**:
  ```cpp
  // REMOVED:
  std::vector<ZAPSectionInfo> zapSections;
  ```

- **StandardDigitizer implementation**: No Stage 4 ZAP building
- **CDigitInfo::ConvertOutputToLegacyModel()**: No ZAP section handling

---

## 2. Introduction of FringeSegmentAdapter (NEW Step 10)

### Architecture Problem
The pure algorithm must output STL-only data (no MFC), but CDigitInfo needs MFC CFringeSegment.

### Solution: Adapter Bridge
```
Pure Algorithm Output        Stage 4 Adapter             Final Storage
━━━━━━━━━━━━━━━━━━         ━━━━━━━━━━━━━━━             ━━━━━━━━━━━━━━
vector<NumberedFringe>  →  FringeSegmentAdapter  →  vector<CFringeSegment>
(STL-only, testable)       (thin bridge)             (MFC-compatible)
```

### Code Structure
```cpp
namespace DigitMode::digitization {

class FringeSegmentAdapter {
public:
    // Convert single fringe
    static CFringeSegment AdaptFringe(const NumberedFringe& numberedFringe);
    
    // Batch convert entire vector
    static std::vector<CFringeSegment> AdaptFringes(
        const std::vector<NumberedFringe>& fringes);
};

}
```

### Why This Is Correct
1. **Pure algorithm isolation** - StandardDigitizer has ZERO MFC dependencies
2. **Adapter simplicity** - Just converts data structure, no logic
3. **Clear separation** - MFC contamination enters only at boundary
4. **Testable** - Adapter can be tested in isolation

### Integration in CDigitInfo
```cpp
void CDigitInfo::ConvertOutputToLegacyModel(
    const digitization::DigitizationOutput& output)
{
    // Stage 4: Pure → MFC bridge
    auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
    
    // Populate final storage
    Fringes.clear();
    for (const auto& fringeSegment : mfcFringes) {
        Fringes.push_back(fringeSegment);
    }
}
```

---

## 3. Red Dots for Low-Level Visualization

### What Are Red Dots?
ExtremumPoint objects detected during Stage 1 (RedCenterDetector). These are intensity extrema found by the algorithm at the pixel level.

### Why Important
- **Low-level algorithm outcome** - Show what the algorithm detected before connection/numbering
- **Debugging aid** - Visualize if extrema detection is working correctly
- **User feedback** - Users can see which points were considered as candidates

### Where Stored
```cpp
// In DigitizationOutput:
std::vector<ExtremumPoint> redCenters;

// In CDigitInfo:
CArray<CDPoint> HidenDots;  // Already exists, repurposed
```

### Pipeline
```
Stage 1: RedCenterDetector
    ↓
    ↓ Detects extrema
    ↓
DigitizationOutput::redCenters
    ↓
    ↓ In CDigitInfo::ConvertOutputToLegacyModel()
    ↓
CDigitInfo::HidenDots  (for visualization)
```

### Code
```cpp
void CDigitInfo::ConvertOutputToLegacyModel(
    const digitization::DigitizationOutput& output)
{
    // Store red centers for visualization
    HidenDots.RemoveAll();
    for (const auto& redCenter : output.redCenters) {
        HidenDots.Add(redCenter.position);
    }
}
```

---

## Updated DigitizationOutput Structure

```cpp
struct DigitizationOutput {
    // Red dots - detected extrema for visualization (low-level outcome)
    std::vector<ExtremumPoint> redCenters;
    
    // Intermediate results (for debugging, optional)
    std::vector<FringePolyline> polylines;
    
    // Final pure output - numbered fringes (STL-only, no MFC)
    std::vector<NumberedFringe> fringes;
    
    // Algorithm metrics
    double averageFringeStep;
    int mainFringeNumber;
    
    // NOT INCLUDED:
    // - zapSections (ZAPLine concept deprecated)
    // - CFringeSegment (adapter handles conversion)
};
```

---

## Final Pipeline Architecture

```
CDigitInfo::Auto()
    ↓
    ↓ Build DigitizationInput (bitmap + mask + parameters)
    ↓
StandardDigitizer (pure algorithm, no MFC)
    ├─ Stage 1: RedCenterDetector
    │   ├─ Input: bitmap + mask
    │   └─ Output: vector<ExtremumPoint> (red dots)
    │
    ├─ Stage 2: FringeConnector
    │   ├─ Input: extrema + mask
    │   └─ Output: vector<FringePolyline>
    │
    └─ Stage 3: FringeNumberer
        ├─ Input: polylines + aperture center
        └─ Output: vector<NumberedFringe>
    ↓
    ↓ DigitizationOutput (pure STL)
    ↓
FringeSegmentAdapter (Stage 4, outside pure algorithm)
    ├─ Input: vector<NumberedFringe>
    └─ Output: vector<CFringeSegment> (MFC)
    ↓
CDigitInfo Storage
    ├─ Fringes (vector<CFringeSegment>)
    ├─ HidenDots (red centers for visualization)
    ├─ MainFringeNumber
    └─ Dots (legacy, if m_bUseFringeModel == false)
```

---

## File Structure Update

### New Files (19 total)
1. `DigitMode/DigitizationParams.h` - POD structures
2. `DigitMode/VisibilityMaskAccessor.h` - Mask queries
3. `DigitMode/RedCenterDetector.h` - Stage 1
4. `DigitMode/FringeConnector.h` - Stage 2
5. `DigitMode/FringeNumberer.h` - Stage 3
6. `DigitMode/FringeSegmentAdapter.h` - Stage 4 adapter (CHANGED from ZAPSectionBuilder)
7. `DigitMode/IDigitizationStrategy.h` - Strategy interface
8-10. StandardDigitizer, Factory, etc.
11-19. Tests (FringeSegmentAdapterTest replaces ZAPSectionBuilderTest)

### Key Change
- **Removed**: ZAPSectionBuilder (Step 10)
- **Added**: FringeSegmentAdapter (Step 10)
- **Count**: Still 19 files, same as before

---

## Test Updates

### Removed
- `ZAPSectionBuilderTest.cpp` - ZAPSections no longer needed

### Added
- `FringeSegmentAdapterTest.cpp` - Verify pure → MFC conversion

### Example Test
```cpp
TEST(FringeSegmentAdapterTest, ConvertsNumberedFringeToMFC) {
    // Create pure output
    digitization::NumberedFringe pureData;
    pureData.number = 3.5;
    pureData.points = { {10, 20}, {11, 21} };
    
    // Adapt
    CFringeSegment adapted = FringeSegmentAdapter::AdaptFringe(pureData);
    
    // Verify
    EXPECT_EQ(adapted.GetNumber(), 3.5);
    EXPECT_EQ(adapted.GetPointCount(), 2);
}
```

---

## Impact Summary

| Aspect | Impact | Reason |
|--------|--------|--------|
| Pure algorithm | CLEANER | No ZAPSection logic needed |
| MFC isolation | STRONGER | Clear Stage 4 adapter boundary |
| Testability | IMPROVED | Adapter is simple and focused |
| Red dots | PRESERVED | Available for visualization |
| ZAPSections | REMOVED | Concept deprecated |
| CFringeSegment | DECOUPLED | Not in algorithm output |
| Future variants | EASIER | No ZAP or MFC logic to replicate |

---

## Conclusion

The refined architecture is now:

✅ **Architecturally pure** - Pure algorithm (Stages 1-3) has zero framework dependencies
✅ **Clearly separated** - Stage 4 adapter is the explicit boundary
✅ **Fully testable** - Each stage testable in isolation, adapter testable separately
✅ **Future-proof** - ML, adaptive, or parallel variants need only replace Stages 1-3
✅ **User-friendly** - Red dots show algorithm's low-level detection
✅ **Maintainable** - Simple, clear data flow

The visualization of red dots provides transparency into the algorithm's behavior, helping users understand what the digitization is detecting at the pixel level.

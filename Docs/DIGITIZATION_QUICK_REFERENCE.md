# Digitization Refactoring - Quick Reference

## The Three Critical Insights

### 1️⃣ NO ZAPSections
- **What**: ZAPLine concept is **deprecated**, not planned for future
- **Change**: Removed ZAPSectionBuilder (OLD Step 10)
- **Impact**: Cleaner algorithm, simpler output structure
- **Code**: No `std::vector<ZAPSectionInfo>` in DigitizationOutput

### 2️⃣ FringeSegmentAdapter (Stage 4)
- **What**: Pure algorithm output (`std::vector<NumberedFringe>`) → MFC-compatible (`std::vector<CFringeSegment>`)
- **Why**: Keep pure algorithm free of MFC, use adapter bridge for conversion
- **Where**: Stage 4, applied in `CDigitInfo::ConvertOutputToLegacyModel()`
- **Code**:
  ```cpp
  auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
  ```

### 3️⃣ Red Dots for Visualization
- **What**: ExtremumPoint data preserved in output
- **Why**: Show low-level algorithm outcome (detected extrema before connection)
- **Where**: `DigitizationOutput::redCenters` → `CDigitInfo::HidenDots`
- **Code**:
  ```cpp
  for (const auto& redCenter : output.redCenters) {
      HidenDots.Add(redCenter.position);
  }
  ```

---

## Pipeline at a Glance

```
Input: bitmap + mask + aperture_center → [Pure Algorithm] → output: NumberedFringe
                                                    ↓
                                          [Adapter Bridge]
                                                    ↓
Output: CFringeSegment → CDigitInfo::Fringes
        Red dots → CDigitInfo::HidenDots
```

---

## Files to Create (19 total)

| # | File | Purpose |
|---|------|---------|
| 1 | `DigitizationParams.h` | POD input/output structures |
| 2 | `VisibilityMaskAccessor.h` | Mask query interface |
| 3 | `RedCenterDetector.h` | Stage 1: detect extrema |
| 4 | `FringeConnector.h` | Stage 2: connect polylines |
| 5 | `FringeNumberer.h` | Stage 3: number fringes |
| 6 | `FringeSegmentAdapter.h` | **Stage 4: pure → MFC** |
| 7 | `IDigitizationStrategy.h` | Strategy interface |
| 8 | `StandardDigitizer.h/cpp` | Strategy implementation |
| 9 | `DigitizationStrategyFactory.h` | Strategy factory |
| 10-19 | Test files (9) | Unit + integration tests |

---

## DigitizationOutput Structure

```cpp
struct DigitizationOutput {
    // Red dots (for visualization - low-level outcome)
    std::vector<ExtremumPoint> redCenters;
    
    // Intermediate (for debugging)
    std::vector<FringePolyline> polylines;
    
    // Final output (pure STL, no MFC)
    std::vector<NumberedFringe> fringes;
    
    // Metrics
    double averageFringeStep;
    int mainFringeNumber;
    
    // NOT included:
    // - zapSections (deprecated)
    // - CFringeSegment (adapter handles conversion)
};
```

---

## CDigitInfo Integration

```cpp
void CDigitInfo::Auto() {
    // 1. Build input
    auto input = BuildDigitizationInput(pI, pA, pCtrls);
    
    // 2. Run pure algorithm
    auto output = strategy->Digitize(input);
    
    // 3. Convert to legacy storage
    ConvertOutputToLegacyModel(output);
}

void CDigitInfo::ConvertOutputToLegacyModel(
    const DigitizationOutput& output) {
    
    // Stage 4: Adapter bridge
    auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
    
    // Store in Fringes (segment-primary model)
    Fringes.clear();
    for (const auto& fringeSegment : mfcFringes) {
        Fringes.push_back(fringeSegment);
    }
    
    // Store red dots for visualization
    HidenDots.RemoveAll();
    for (const auto& redCenter : output.redCenters) {
        HidenDots.Add(redCenter.position);
    }
    
    // NO ZAP section handling
    
    // Set state
    MainFringeNumber = output.mainFringeNumber;
}
```

---

## What Was Removed

| Item | Old Location | Reason |
|------|--------------|--------|
| **ZAPSectionBuilder** | Step 10 | ZAPLine deprecated |
| **zapSections** | DigitizationOutput | No future use |
| **IBoundsProvider** | Step 4 | Visibility mask sufficient |
| **hasObstruction parameter** | FringeNumberer | Implicit in mask |

---

## What Was Added

| Item | New Location | Reason |
|------|--------------|--------|
| **FringeSegmentAdapter** | Step 10 | Pure → MFC bridge |
| **redCenters in output** | DigitizationOutput | Visualization support |
| **Explicit Stage 4** | Pipeline | Clear MFC boundary |

---

## Testing Checklist

- [ ] Unit tests for RedCenterDetector (synthetic images)
- [ ] Unit tests for FringeConnector (synthetic extrema)
- [ ] Unit tests for FringeNumberer (synthetic polylines)
- [ ] Unit tests for FringeSegmentAdapter (pure → MFC conversion)
- [ ] Integration test for StandardDigitizer (end-to-end)
- [ ] Regression test for CDigitInfo::Auto() output
- [ ] Verify red dots appear in HidenDots
- [ ] Verify NO ZAP sections generated
- [ ] Verify CBoundCtrls NOT accessed in algorithm

---

## Architectural Invariants

✅ **Pure Algorithm**: Stages 1-3 have ZERO MFC dependencies  
✅ **Clear Bridge**: FringeSegmentAdapter is the only MFC touchpoint  
✅ **Red Dots**: ExtremumPoint always in output  
✅ **NO ZAPLines**: Removed entire concept  
✅ **STL Primary**: Algorithm works with std::vector, not CArray  
✅ **Testable**: Each stage mockable, no framework required  
✅ **Extensible**: Strategy pattern ready for variants  

---

## Future Variants (ML, Adaptive, etc.)

All future algorithms:
- Input: `DigitizationInput` (same contract)
- Output: `DigitizationOutput` (same contract)
- Bridge: `FringeSegmentAdapter` (unchanged!)
- No need to reimplement Stage 4

Example:
```cpp
class MLBasedDigitizer : public IDigitizationStrategy {
    DigitizationOutput Digitize(const DigitizationInput& input) override {
        // Completely different Stages 1-3
        // But output the same DigitizationOutput structure
        return output;
    }
};
```

---

## Success Criteria Checklist

- ✅ All 19 files created
- ✅ No buf_line anywhere
- ✅ No CDotInfo in algorithm
- ✅ Zero MFC in Stages 1-3
- ✅ Visibility mask used for all spatial queries
- ✅ NO CBoundCtrls dependency
- ✅ Red dots in output and visualization
- ✅ NO ZAP sections
- ✅ 100% unit test coverage
- ✅ Integration tests pass
- ✅ Regression tests pass
- ✅ Code review approved


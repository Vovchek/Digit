# IMMEDIATE ACTION ITEMS - Phase 2 Completion

**Generated:** February 2026  
**Phase Status:** 30% Complete (Need 70% more)  
**Critical Path:** FringeConnector → FringeNumberer → FringeSegmentAdapter  
**Timeline to Phase 2 Completion:** 2-3 weeks

---

## ✅ What's Already Done

1. **DigitizationParams.h** ✅ Complete
   - ExtremumPoint, FringePolyline, NumberedFringe, DigitizationInput/Output
   - Zero MFC dependencies
   - Status: In use in CreateRedCenters()

2. **RedCenterDetector.h/cpp** ✅ Complete
   - Replaces legacy CreateBufLine() + extrema detection
   - Uses visibility mask instead of buf_line array
   - Status: In use in CreateRedCenters(), producing red centers

3. **CreateRedCenters() Updated** ✅ Complete
   - Now uses RedCenterDetector::DetectExtrema()
   - Builds visibility mask from CApertureCtrls
   - Status: Working, maintains backward compatibility

---

## ❌ What Needs to Be Done (Critical Path)

### Step 1: Implement FringeConnector (Days 1-2)

**Why:** Connects individual extrema into continuous polylines. BLOCKS everything else.

**Files to Create:**
```
DigitMode/FringeConnector.h   (~80 lines)
DigitMode/FringeConnector.cpp (~150 lines)
```

**High-Level Algorithm:**
1. Group extrema by Y coordinate (scan line)
2. Sort each group left-to-right
3. Connect extrema from adjacent scan lines (nearest neighbor)
4. Create new polyline when connection breaks
5. Return vector of polylines

**Key Method Signature:**
```cpp
static std::vector<FringePolyline> ConnectExtrema(
    const std::vector<ExtremumPoint>& redCenters,
    const std::function<bool(int, int)>& isVisible
);
```

**Testing:**
- Unit test with synthetic extrema (vertical line, grid pattern)
- Unit test with visibility mask gaps
- Regression test with real interferogram

**Unblocks:** FringeNumberer, StandardDigitizer, Phase 3-4

---

### Step 2: Implement FringeNumberer (Days 2-3)

**Why:** Assigns sequential fringe numbers to polylines. BLOCKS StandardDigitizer.

**Files to Create:**
```
DigitMode/FringeNumberer.h   (~70 lines)
DigitMode/FringeNumberer.cpp (~120 lines)
```

**High-Level Algorithm:**
1. Calculate average fringe spacing from polyline distances
2. Distance metric: minimum distance from polyline to aperture center
3. Assign sequential numbers (1.0, 1.5, 2.0, ...)
4. Identify main fringe (closest to center)
5. Return NumberingResult struct

**Key Method Signature:**
```cpp
struct NumberingResult {
    std::vector<NumberedFringe> fringes;
    double averageFringeStep;
    int mainFringeIndex;
};

static NumberingResult NumberFringes(
    const std::vector<FringePolyline>& polylines,
    const aperture::Point& apertureCenter
);
```

**Testing:**
- Unit test with concentric polylines (known spacing)
- Unit test with main fringe identification
- Regression test with real interferogram

**Unblocks:** FringeSegmentAdapter, Auto() refactoring, Phase 3-4

---

### Step 3: Implement FringeSegmentAdapter (Day 3)

**Why:** Bridges pure algorithm (STL) to legacy MFC (CFringeSegment).

**Files to Create:**
```
DigitMode/FringeSegmentAdapter.h (~40 lines)
DigitMode/FringeSegmentAdapter.cpp (~30 lines)
```

**High-Level Algorithm:**
1. Convert NumberedFringe → CFringeSegment
2. Preserve all data (points, number, segment index)
3. Batch conversion support

**Key Method Signatures:**
```cpp
static CFringeSegment AdaptFringe(const NumberedFringe& numbered);
static std::vector<CFringeSegment> AdaptFringes(
    const std::vector<NumberedFringe>& fringes
);
```

**Testing:**
- Unit test: data preservation (point count, fringe number)
- Integration test: end-to-end pipeline

**Unblocks:** Auto() refactoring

---

### Step 4: Refactor Auto() (Day 4)

**Why:** Integrate pure pipeline into existing CDigitInfo.

**File to Modify:**
```
DigitMode/DigitInfo.cpp - Auto() method (~50 lines change)
```

**Current Implementation (Legacy):**
```cpp
void CDigitInfo::Auto() {
    Clear(FALSE);
    CreateRedCenters();      // ← Uses RedCenterDetector ✅
    SelectFringeStep();      // ← REMOVE (legacy)
    SelectMainSection();     // ← REMOVE (legacy)
    CreateNumLines();        // ← REMOVE (legacy)
    SelectMainFringe();      // ← REMOVE (legacy)
    CorrectNumbers();        // ← REMOVE (legacy)
    CreateZAPSections();     // ← REMOVE (legacy)
    SortDotsFY();
    SelectMainDot();
    SyncFringesToDots();
}
```

**New Implementation (Pure Pipeline):**
```cpp
void CDigitInfo::Auto() {
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    Clear(FALSE);
    
    // Build input for pure pipeline
    auto input = BuildDigitizationInput();
    
    // Stage 1: Detect extrema
    auto redCenters = digitization::RedCenterDetector::DetectExtrema(input);
    
    // Stage 2: Connect into polylines
    auto polylines = digitization::FringeConnector::ConnectExtrema(
        redCenters, input.isVisible
    );
    
    // Stage 3: Number fringes
    auto numberingResult = digitization::FringeNumberer::NumberFringes(
        polylines, GetApertureCenterAsPoint()
    );
    
    // Stage 4: Convert to MFC format
    auto mfcFringes = digitization::FringeSegmentAdapter::AdaptFringes(
        numberingResult.fringes
    );
    
    // Store results
    Fringes.clear();
    for (const auto& seg : mfcFringes) {
        Fringes.push_back(seg);
    }
    
    MainFringeNumber = numberingResult.mainFringeIndex;
    idxMainPoint = SelectedPoint(0, 0);
    
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
```

**Helper Methods to Add:**
```cpp
digitization::DigitizationInput BuildDigitizationInput() {
    CImageCtrls* pI = GetImageCtrls();
    CApertureCtrls* pA = GetApertureCtrls();
    CControls* pCtrls = GetControls();
    
    digitization::DigitizationInput input;
    input.bitmapData = pI->GetBitmapData();
    input.imageWidth = pI->GetWidth();
    input.imageHeight = pI->GetHeight();
    
    auto* maskProvider = &pA->GetMaskProvider();
    input.isVisible = [maskProvider](int x, int y) {
        return maskProvider->getMask().IsVisible(x, y);
    };
    
    input.fringeCenterAs = pCtrls->FringeCenterAs;
    input.contrastThreshold = pCtrls->Eps;
    input.apertureCenter = GetApertureCenterAsPoint();
    
    return input;
}

aperture::Point GetApertureCenterAsPoint() {
    // Compute center from aperture shapes
    // Return as aperture::Point(x, y)
}
```

---

### Step 5: Remove Legacy buf_line (Day 5)

**Why:** Clean up deprecated code that's no longer needed.

**Files to Modify:**
```
DigitMode/DigitInfo.h - Remove members
DigitMode/DigitInfo.cpp - Delete methods
```

**Remove from DigitInfo.h:**
```cpp
// DELETE these members:
int** buf_line;              // No longer used
int ny_buf_line;             // No longer used
```

**Delete from DigitInfo.cpp these methods:**
```cpp
void Init_buf_line(int ny, int n);
void Delete_buf_line();
void CreateBufLine();
void CreateBufLineAperture();
void CreateBufLineApertureSimple();
void CreateBufLineApertureComplex();
void CreateBufLineObstruction();
void CreateBufLineObstructionSimple();
void CreateBufLineObstructionComplex();
```

**Verification:**
- [ ] Search for "buf_line" in entire codebase - should find 0 results
- [ ] Build succeeds without warnings
- [ ] No references to deleted methods

---

## Testing Strategy

### Unit Tests (To Add During Implementation)

#### FringeConnector Tests
```
Tests/DigitModeTests/FringeConnectorTest.cpp
├─ ConnectsSimpleLinearExtrema
├─ CreatesNewPolylineOnGap  
├─ RespectVisibilityMask
├─ HandlesMultiplePolylines
└─ SortsExtremaCorrectly
```

#### FringeNumberer Tests
```
Tests/DigitModeTests/FringeNumbererTest.cpp
├─ NumbersConcentricPolylines
├─ IdentifiesMainFringe
├─ CalculatesFringeStepCorrectly
├─ HandlesEllipticalAperture
└─ HandlesSinglePolyline
```

#### Integration Tests
```
Tests/DigitModeTests/DigitizationPipelineTest.cpp
├─ EndToEndWithRealData
├─ CompareWithLegacyAuto
└─ RegressionTestInterferograms
```

### Quick Validation After Each Step
```bash
# Day 1 (after FringeConnector):
cmake --build build --config Debug
ctest -R FringeConnectorTest -V

# Day 3 (after FringeNumberer):
cmake --build build --config Debug
ctest -R "FringeConnectorTest|FringeNumbererTest" -V

# Day 4 (after Auto() refactoring):
cmake --build build --config Debug
# Load test file in application
# Verify fringe detection still works
# Verify output matches legacy version

# Day 5 (after buf_line removal):
cmake --build build --config Debug -Werror
# Should compile with zero warnings
```

---

## Code Organization

### New Namespace Organization
```
DigitMode
├── DigitMode::digitization (Pure Algorithm)
│   ├── DigitizationParams.h         ✅ DONE
│   ├── RedCenterDetector.h/cpp      ✅ DONE
│   ├── FringeConnector.h/cpp        ❌ TODO
│   ├── FringeNumberer.h/cpp         ❌ TODO
│   └── FringeSegmentAdapter.h/cpp   ❌ TODO
│
└── CDigitInfo (MFC Wrapper)
    ├── DigitInfo.h                   (Remove buf_line)
    └── DigitInfo.cpp                 (Update Auto(), remove legacy)

Tests/DigitModeTests
├── FringeConnectorTest.cpp          ❌ TODO
├── FringeNumbererTest.cpp           ❌ TODO
├── FringeSegmentAdapterTest.cpp     ❌ TODO
├── DigitizationPipelineTest.cpp     ❌ TODO
└── DigitizationRegressionTest.cpp   ❌ TODO
```

---

## Build & Validation Commands

### Initial Setup
```bash
cd C:\Users\vovch\source\repos\Vovchek\Digit
git checkout devel/apertures
git pull origin devel/apertures
```

### Build & Test Each Step
```bash
# After FringeConnector
cmake --build build --config Debug
ctest -R FringeConnectorTest -V

# After FringeNumberer  
cmake --build build --config Debug
ctest -R FringeNumbererTest -V

# After FringeSegmentAdapter
cmake --build build --config Debug
ctest -R FringeSegmentAdapterTest -V

# After Auto() refactoring
cmake --build build --config Debug
./bin/DigitApp  # Manual test in UI

# Final validation
cmake --build build --config Release
ctest --verbose
```

---

## Dependency Graph (For Reference)

```
Phase 2 Steps:

Step 5 (DigitizationParams)           ✅ DONE
    ↓
Step 5 (RedCenterDetector)            ✅ DONE
    ├─ Step 7 (CreateRedCenters)      ✅ DONE
    └─ Step 5 (FringeConnector)       ❌ TODO (CRITICAL)
        ├─ Step 5 (FringeNumberer)    ❌ TODO (CRITICAL)
        │   └─ Step 5 (FringeSegmentAdapter)  ❌ TODO (HIGH)
        │       └─ Step 14 (Auto() refactor)  ❌ TODO (HIGH)
        │           └─ Step 16 (Remove legacy) ❌ TODO (MEDIUM)
        │
        └─ Phase 4 (StandardDigitizer) ❌ BLOCKED until all above done
```

---

## Priority & Effort Summary

| Task | Priority | Est. Hours | Days | Blocker For |
|------|----------|-----------|------|-------------|
| **FringeConnector** | 🔴 CRITICAL | 12-16 | 2-3 | Numberrer, Phase 4 |
| **FringeNumberer** | 🔴 CRITICAL | 8-12 | 1-2 | Adapter, Phase 4 |
| **FringeSegmentAdapter** | 🟡 HIGH | 4-6 | 0.5-1 | Auto() refactor |
| **Auto() Refactoring** | 🟡 HIGH | 6-8 | 1 | buf_line removal |
| **Remove buf_line** | 🟡 HIGH | 4-6 | 1 | Testing, cleanup |
| **Unit Tests (all)** | 🟢 MEDIUM | 16-20 | 2-3 | Code review |
| **Integration Tests** | 🟢 MEDIUM | 8-12 | 1-2 | Code review |
| **Code Review & Fixes** | 🟢 MEDIUM | 4-8 | 1 | Release |
| **TOTAL** | | **62-88 hours** | **11-17 days** | |

---

## Recommended Schedule

### Week 1
- **Mon-Tue:** FringeConnector implementation + unit tests (Days 1-2)
- **Wed:** FringeNumberer implementation + unit tests (Day 3)
- **Thu:** FringeSegmentAdapter + Auto() refactoring (Day 4)
- **Fri:** buf_line removal + integration test setup (Day 5)

### Week 2  
- **Mon-Tue:** Integration & regression testing (Days 6-7)
- **Wed-Thu:** Code review & fixes (Days 8-9)
- **Fri:** Final validation & documentation (Day 10)

### Contingency
- If complexity increases: +3-5 days
- If regressions found: +2-3 days
- If performance issues: +2-3 days

---

## Success Metrics

Phase 2 is **COMPLETE** when:

✅ **Code Deliverables:**
- [ ] FringeConnector.h/cpp (all stages working)
- [ ] FringeNumberer.h/cpp (all stages working)
- [ ] FringeSegmentAdapter.h/cpp (conversion working)
- [ ] Auto() refactored (pure pipeline integrated)
- [ ] All buf_line code removed
- [ ] Zero warnings on build

✅ **Testing:**
- [ ] All unit tests pass (>90% coverage)
- [ ] All integration tests pass (real data)
- [ ] Regression tests: <1% output variance
- [ ] No performance regression (<5% slower)

✅ **Quality:**
- [ ] Code review approved
- [ ] Documentation updated
- [ ] CI/CD pipeline passes
- [ ] No breaking API changes

---

## Reference Documentation

- 📄 `DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md` - Overall architecture
- 📄 `PHASE2_PROGRESS_ANALYSIS.md` - Detailed current status (see file created)
- 📄 `PHASE2_COMPLETION_ROADMAP.md` - Detailed implementation guide (see file created)
- 📖 Copilot Instructions: `.github/copilot-instructions.md`

---

## Questions to Answer Before Starting

1. **FringeConnector Gap Tolerance:** How many pixels can be skipped in Y coordinates?
   - **Answer:** Typically 1-2 pixels (depends on FringeSpacing)

2. **Numbering Sequence:** Do fringes start at 0.5, 1.0, or custom?
   - **Answer:** Varies by system (usually 0.5 or 1.0, need to check legacy)

3. **Distance Metric:** Minimum, average, or weighted distance to center?
   - **Answer:** Typically minimum distance to any point in polyline

4. **Elliptical vs Circular:** How to handle non-circular apertures?
   - **Answer:** Use geometric center, not necessarilyCircular aperture

5. **Performance Targets:** Any image size limits to optimize for?
   - **Answer:** Typically 512x512 to 2048x2048

---

## Next Step: START HERE

**👉 IMMEDIATE ACTION:**

1. Create new file: `DigitMode/FringeConnector.h` (copy template from roadmap)
2. Implement GroupByScanLine() first (simplest helper)
3. Implement FindBestContinuation() (core logic)
4. Implement ConnectExtrema() wrapper
5. Add unit test: simple vertical line of extrema
6. Verify test passes
7. Proceed to next stage

**Estimated time for Step 1:** 4-6 hours

Once FringeConnector works → FringeNumberer unblocks
Once FringeNumberer works → Full pipeline unblocks
Once full pipeline works → Phase 3-4 can proceed

---

## Contact & Support

- **Code Repository:** `https://github.com/Vovchek/Digit` (branch: `devel/apertures`)
- **Issues/Blockers:** Refer to copilot-instructions.md for coding standards
- **Questions:** Check DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md appendices for architecture details

**Current Status:** 30% complete → 70% remaining → 2-3 weeks to finish

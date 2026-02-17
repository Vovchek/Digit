# Phase 2 Completion Roadmap

**Target:** Complete remaining 70% of Phase 2 work  
**Timeline:** 2-3 weeks  
**Effort:** ~11-17 developer days

---

## Quick Reference: What's Left to Do

### 🔴 CRITICAL PATH (Blocks Everything Else)

1. **FringeConnector.h/cpp** (2-3 days)
   - Stage 2: Connect extrema → polylines
   - Unblocks: FringeNumberer, StandardDigitizer
   - Test: Synthetic extrema patterns + real interferograms

2. **FringeNumberer.h/cpp** (1-2 days)
   - Stage 3: Number polylines
   - Unblocks: FringeSegmentAdapter, StandardDigitizer
   - Test: Concentric polyline patterns

3. **FringeSegmentAdapter.h/cpp** (0.5-1 day)
   - Stage 4: Convert to MFC format
   - Unblocks: Auto() final refactoring
   - Test: Data preservation (no information loss)

### 🟡 HIGH PRIORITY (Can Proceed in Parallel)

4. **Auto() Refactoring** (1-2 days)
   - Call pure pipeline instead of legacy methods
   - ConvertOutputToLegacyModel() adapter
   - Maintain backward compatibility

5. **Remove Legacy buf_line** (1 day)
   - Delete Init_buf_line(), Delete_buf_line(), CreateBufLine()
   - Delete CreateBufLineAperture*() variants
   - Delete CreateBufLineObstruction*() variants
   - Update CDigitInfo.h member variables

### 🟢 OPTIONAL/PARALLEL

6. **Unit Tests** (3-4 days)
   - RedCenterDetector tests (can add now)
   - FringeConnector tests (add during/after implementation)
   - FringeNumberer tests (add during/after implementation)
   - Integration tests with real data

---

## Detailed Implementation Roadmap

### Week 1: Implement Core Stages (Days 1-3)

#### Day 1: FringeConnector Implementation

**Deliverable:** `DigitMode/FringeConnector.h` + `DigitMode/FringeConnector.cpp`

**Step 1a: Create Header**
```cpp
// DigitMode/FringeConnector.h
#pragma once
#include <vector>
#include <functional>
#include <map>
#include "DigitMode/DigitizationParams.h"

namespace DigitMode::digitization {

class FringeConnector {
public:
    /**
     * @brief Connect extrema points into continuous fringe polylines
     * 
     * Implements Stage 2 of the digitization pipeline.
     * 
     * @param redCenters Vector of extrema points (from Stage 1)
     * @param isVisible Visibility checker function
     * @return Vector of connected polylines (ordered points)
     */
    static std::vector<FringePolyline> ConnectExtrema(
        const std::vector<ExtremumPoint>& redCenters,
        const std::function<bool(int, int)>& isVisible);

private:
    /**
     * @brief Group extrema by horizontal scan line (Y coordinate)
     * 
     * @param redCenters Input extrema
     * @return Map of scanLineIndex → sorted extrema on that line
     */
    static std::map<int, std::vector<ExtremumPoint>> GroupByScanLine(
        const std::vector<ExtremumPoint>& redCenters);
    
    /**
     * @brief Find the best continuation point on the next scan line
     * 
     * Implements continuity logic: prefers extrema closest to current point,
     * checking visibility on intervening pixels.
     * 
     * @param current The previous extremum
     * @param nextLineExtrema Candidate extrema on next scan line
     * @param isVisible Visibility checker for gap validation
     * @return Pointer to best continuation, or nullptr if no valid continuation
     */
    static const ExtremumPoint* FindBestContinuation(
        const ExtremumPoint& current,
        const std::vector<ExtremumPoint>& nextLineExtrema,
        const std::function<bool(int, int)>& isVisible);
};

} // namespace DigitMode::digitization
```

**Step 1b: Implement ConnectExtrema()**
```
Algorithm:
1. Group extrema by Y coordinate (scan line index)
2. For each group, sort extrema left-to-right
3. Start with first extremum on topmost scan line
4. Traverse downward, connecting each extremum to nearest on next line
5. Check visibility along connecting path
6. Create new polyline if connection broken
7. Return list of polylines
```

**Key Implementation Details:**
- **Gap Handling:** If Y jumps > 1 (missing scan line), check if connection is valid
- **Extremum Duplication:** Same extremum shouldn't appear in multiple polylines
- **Discontinuities:** When not connected, start new polyline with fresh segment index

---

#### Day 2: FringeNumberer Implementation

**Deliverable:** `DigitMode/FringeNumberer.h` + `DigitMode/FringeNumberer.cpp`

**Step 2a: Create Header**
```cpp
// DigitMode/FringeNumberer.h
#pragma once
#include <vector>
#include "DigitMode/DigitizationParams.h"

// Simple point type for aperture center
namespace aperture { struct Point { double x, y; }; }

namespace DigitMode::digitization {

class FringeNumberer {
public:
    /**
     * @brief Result of fringe numbering operation
     */
    struct NumberingResult {
        std::vector<NumberedFringe> fringes;  // Numbered and finalized
        double averageFringeStep;              // Calculated spacing
        int mainFringeIndex;                   // Index of fringe closest to center
    };
    
    /**
     * @brief Assign fringe numbers to connected polylines
     * 
     * Implements Stage 3 of the digitization pipeline.
     * Numbers are assigned based on distance from aperture center.
     * 
     * @param polylines Connected polylines (from Stage 2)
     * @param apertureCenter Reference point for numbering
     * @return Numbered fringes + statistics
     */
    static NumberingResult NumberFringes(
        const std::vector<FringePolyline>& polylines,
        const aperture::Point& apertureCenter);

private:
    /**
     * @brief Calculate average fringe spacing from polyline pattern
     * 
     * Analyzes polyline positions relative to aperture center.
     * Assumes roughly circular/concentric pattern.
     * 
     * @return Average distance between adjacent polylines
     */
    static double CalculateFringeStep(
        const std::vector<FringePolyline>& polylines,
        const aperture::Point& apertureCenter);
    
    /**
     * @brief Compute distance from polyline to aperture center
     * 
     * @return Minimum distance from any point in polyline to center
     */
    static double PolylineDistanceToCenter(
        const FringePolyline& polyline,
        const aperture::Point& center);
};

} // namespace DigitMode::digitization
```

**Step 2b: Implement NumberFringes()**
```
Algorithm:
1. Compute average fringe step from polyline spacing
2. Calculate distance from each polyline to aperture center
3. Sort polylines by distance (closest first)
4. Assign sequential numbers: 1.0, 1.5, 2.0, 2.5, ... (or 0.5, 1.0, 1.5, ...)
5. Find main fringe (closest to center)
6. Handle obstructed regions (if detected)
7. Return NumberedFringe objects with assigned numbers
```

**Key Implementation Details:**
- **Distance Metric:** Use minimum distance from polyline points to center
- **Numbering Sequence:** Typically starts at 0.5 or 1.0 depending on aperture design
- **Half-Fringes:** Some systems use 0.5 increment (1.0, 1.5, 2.0)
- **Main Fringe:** Index of polyline with minimum distance

---

#### Day 3: FringeSegmentAdapter + Integration

**Deliverable:** `DigitMode/FringeSegmentAdapter.h/cpp` + Update Auto()

**Step 3a: Create FringeSegmentAdapter**
```cpp
// DigitMode/FringeSegmentAdapter.h
#pragma once
#include <vector>
#include "DigitMode/DigitizationParams.h"

// Forward declare MFC type
class CFringeSegment;

namespace DigitMode::digitization {

class FringeSegmentAdapter {
public:
    /**
     * @brief Convert pure algorithm output to MFC-compatible format
     * 
     * Implements Stage 4 (adapter bridge).
     * Separates pure algorithm from MFC dependencies.
     * 
     * @param numbered Pure algorithm output (STL-only)
     * @return MFC CFringeSegment for CDigitInfo storage
     */
    static CFringeSegment AdaptFringe(const NumberedFringe& numbered);
    
    /**
     * @brief Batch conversion for complete pipeline output
     * 
     * @param fringes Vector of pure NumberedFringe objects
     * @return Vector of MFC CFringeSegment objects
     */
    static std::vector<CFringeSegment> AdaptFringes(
        const std::vector<NumberedFringe>& fringes);
};

} // namespace DigitMode::digitization
```

**Step 3b: Update Auto() to Use Pure Pipeline**
```cpp
// In DigitMode/DigitInfo.cpp

void CDigitInfo::Auto() {
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    Clear(FALSE);
    
    // Build input for pure pipeline
    auto input = BuildDigitizationInput();
    
    // Stage 1: Detect extrema
    auto redCenters = digitization::RedCenterDetector::DetectExtrema(input);
    HidenDots.clear();
    for (const auto& rc : redCenters) {
        HidenDots.push_back(CDPoint(rc.position.x, rc.position.y));
    }
    
    // Stage 2: Connect into polylines
    auto polylines = digitization::FringeConnector::ConnectExtrema(
        redCenters, input.isVisible
    );
    
    // Stage 3: Number fringes
    auto aperturePt = GetApertureCenterAsPoint();
    aperture::Point apCenter = {aperturePt.x, aperturePt.y};
    auto numberingResult = digitization::FringeNumberer::NumberFringes(
        polylines, apCenter
    );
    
    // Stage 4: Convert to MFC format
    auto mfcFringes = digitization::FringeSegmentAdapter::AdaptFringes(
        numberingResult.fringes
    );
    Fringes.clear();
    for (const auto& seg : mfcFringes) {
        Fringes.push_back(seg);
    }
    
    MainFringeNumber = numberingResult.mainFringeIndex;
    idxMainPoint = SelectedPoint(0, 0);
    
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
```

---

### Week 2: Cleanup & Testing (Days 4-10)

#### Day 4: Remove Legacy buf_line

**Files to Modify:**
- `DigitMode/DigitInfo.h` - Remove member variables
- `DigitMode/DigitInfo.cpp` - Delete methods

**Checklist:**
- [ ] Remove `int** buf_line` member
- [ ] Remove `int ny_buf_line` member
- [ ] Delete `Init_buf_line()`
- [ ] Delete `Delete_buf_line()`
- [ ] Delete `CreateBufLine()`
- [ ] Delete `CreateBufLineAperture()`, `CreateBufLineApertureSimple()`, `CreateBufLineApertureComplex()`
- [ ] Delete `CreateBufLineObstruction()`, `CreateBufLineObstructionSimple()`, `CreateBufLineObstructionComplex()`
- [ ] Verify no remaining buf_line references
- [ ] Verify build succeeds

---

#### Days 5-7: Unit Tests

**Test 1: FringeConnector Tests** (~100 lines)
```cpp
// Tests/DigitModeTests/FringeConnectorTest.cpp

TEST(FringeConnectorTest, ConnectsSimpleLinearExtrema) {
    // Create synthetic extrema in vertical line
    std::vector<ExtremumPoint> extrema = {
        {{5.0, 0.0}, 128, FC_MAX},
        {{5.5, 1.0}, 130, FC_MAX},
        {{6.0, 2.0}, 128, FC_MAX},
    };
    
    auto isVisible = [](int, int) { return true; };
    auto polylines = FringeConnector::ConnectExtrema(extrema, isVisible);
    
    EXPECT_EQ(polylines.size(), 1);
    EXPECT_EQ(polylines[0].points.size(), 3);
}

TEST(FringeConnectorTest, CreatesNewPolylineOnGap) {
    // Gap in Y coordinates (missing scan line)
    std::vector<ExtremumPoint> extrema = {
        {{5.0, 0.0}, 128, FC_MAX},
        {{5.5, 1.0}, 130, FC_MAX},
        // Missing Y=2
        {{6.0, 3.0}, 128, FC_MAX},
    };
    
    auto isVisible = [](int, int) { return true; };
    auto polylines = FringeConnector::ConnectExtrema(extrema, isVisible);
    
    // May be 1 or 2 polylines depending on gap handling strategy
    EXPECT_GE(polylines.size(), 1);
}

TEST(FringeConnectorTest, RespectVisibilityMask) {
    // Extrema separated by invisible region
    std::vector<ExtremumPoint> extrema = {
        {{5.0, 0.0}, 128, FC_MAX},
        {{5.0, 2.0}, 128, FC_MAX},
    };
    
    // Middle scan line is not visible
    auto isVisible = [](int x, int y) { 
        return y != 1;  // Gap at Y=1
    };
    
    auto polylines = FringeConnector::ConnectExtrema(extrema, isVisible);
    
    // Should create separate polylines or handle discontinuity
    EXPECT_GE(polylines.size(), 1);
}
```

**Test 2: FringeNumberer Tests** (~100 lines)
```cpp
// Tests/DigitModeTests/FringeNumbererTest.cpp

TEST(FringeNumbererTest, NumbersConcentricPolylines) {
    // Create concentric square polylines
    std::vector<FringePolyline> polylines;
    
    // Inner square
    FringePolyline inner;
    inner.points = {{5, 5}, {15, 5}, {15, 15}, {5, 15}, {5, 5}};
    inner.index = 0;
    polylines.push_back(inner);
    
    // Outer square
    FringePolyline outer;
    outer.points = {{0, 0}, {20, 0}, {20, 20}, {0, 20}, {0, 0}};
    outer.index = 1;
    polylines.push_back(outer);
    
    aperture::Point center = {10, 10};
    auto result = FringeNumberer::NumberFringes(polylines, center);
    
    EXPECT_EQ(result.fringes.size(), 2);
    EXPECT_LT(result.fringes[0].number, result.fringes[1].number);
    EXPECT_GT(result.averageFringeStep, 0);
}

TEST(FringeNumbererTest, IdentifiesMainFringe) {
    // Single polyline closest to center
    std::vector<FringePolyline> polylines;
    FringePolyline poly;
    poly.points = {{9, 9}, {11, 9}, {11, 11}, {9, 11}};
    poly.index = 0;
    polylines.push_back(poly);
    
    aperture::Point center = {10, 10};
    auto result = FringeNumberer::NumberFringes(polylines, center);
    
    EXPECT_EQ(result.mainFringeIndex, 0);
}
```

**Test 3: Integration Test** (~150 lines)
```cpp
// Tests/DigitModeTests/DigitizationPipelineTest.cpp

TEST(DigitizationPipelineTest, EndToEndWithRealData) {
    // Load test interferogram
    auto bitmap = LoadTestBitmap("test_interferogram.bmp");
    
    // Create simple circular aperture mask
    auto isVisible = [](int x, int y) {
        // Circular aperture centered at (256, 256) with radius 200
        int dx = x - 256, dy = y - 256;
        return (dx*dx + dy*dy) < 200*200;
    };
    
    // Build input
    digitization::DigitizationInput input;
    input.bitmapData = bitmap.data();
    input.imageWidth = 512;
    input.imageHeight = 512;
    input.isVisible = isVisible;
    input.fringeCenterAs = FC_MINMAX;
    input.apertureCenter = {256.0, 256.0};
    
    // Stage 1: Extrema detection
    auto extrema = RedCenterDetector::DetectExtrema(input);
    EXPECT_GT(extrema.size(), 0) << "Should detect at least some extrema";
    
    // Stage 2: Connect
    auto polylines = FringeConnector::ConnectExtrema(extrema, input.isVisible);
    EXPECT_GT(polylines.size(), 0) << "Should create at least one polyline";
    
    // Stage 3: Number
    auto result = FringeNumberer::NumberFringes(
        polylines, 
        {input.apertureCenter.x, input.apertureCenter.y}
    );
    EXPECT_GT(result.fringes.size(), 0) << "Should produce numbered fringes";
    EXPECT_GT(result.averageFringeStep, 0) << "Step should be positive";
    
    // Stage 4: Adapt
    auto mfcFringes = FringeSegmentAdapter::AdaptFringes(result.fringes);
    EXPECT_EQ(mfcFringes.size(), result.fringes.size())
        << "Adapter should preserve count";
}
```

---

#### Days 8-9: Integration & Regression Testing

**Regression Test: Compare Old vs New Auto()**
```cpp
// Tests/DigitModeTests/DigitizationRegressionTest.cpp

TEST(RegressionTest, AutoProducesConsistentResults) {
    // Load test interferogram
    CString testFile = _T("C:\\TestData\\test_interferogram.zap");
    
    // Old implementation (legacy)
    CDigitInfo legacyDigit;
    legacyDigit.Load(testFile);
    legacyDigit.Auto();  // Uses legacy CreateNumLines etc.
    
    // New implementation
    CDigitInfo newDigit;
    newDigit.Load(testFile);
    newDigit.Auto();  // Uses pure pipeline
    
    // Compare results
    EXPECT_EQ(legacyDigit.Fringes.size(), newDigit.Fringes.size());
    EXPECT_NEAR(legacyDigit.MainFringeNumber, newDigit.MainFringeNumber, 0.1);
    
    // Verify key statistics match
    for (size_t i = 0; i < legacyDigit.Fringes.size(); ++i) {
        EXPECT_EQ(
            legacyDigit.Fringes[i].GetNumber(),
            newDigit.Fringes[i].GetNumber()
        );
    }
}
```

---

#### Day 10: Code Review & Final Cleanup

**Checklist:**
- [ ] All tests pass locally
- [ ] No compiler warnings
- [ ] Code review feedback addressed
- [ ] Documentation updated
- [ ] Legacy code deprecation notices added
- [ ] Build on CI/CD passes
- [ ] Performance profiling shows no regression

---

## Risk Mitigation

### Risk 1: FringeConnector Gap Handling
**Problem:** How to handle missing scan lines (Y jumps)?  
**Mitigation:** 
- Implement configurable gap tolerance (default: 1 pixel)
- Check visibility across gaps
- Create new polyline if gap too large

### Risk 2: Circular vs Elliptical Apertures
**Problem:** Distance calculation varies by shape  
**Mitigation:**
- Use minimum distance metric (conservative)
- Test with circular, elliptical, and polygonal apertures
- Add test cases for each type

### Risk 3: Numbering Precision
**Problem:** Floating-point rounding in sequential numbers  
**Mitigation:**
- Use consistent rounding (0.5 tolerance)
- Unit tests with known polyline spacing
- Regression tests vs. legacy output

### Risk 4: MFC Dependency in Adapter
**Problem:** Need to ensure pure algorithm stays pure  
**Mitigation:**
- FringeSegmentAdapter only depends on NumberedFringe (pure)
- No visibility queries in adapter
- Adapter imports CFringeSegment only (for conversion)

---

## Success Criteria

### Phase 2 Complete When:

- [ ] FringeConnector.h/cpp implemented and tested
- [ ] FringeNumberer.h/cpp implemented and tested
- [ ] FringeSegmentAdapter.h/cpp implemented and tested
- [ ] Auto() refactored to use pure pipeline
- [ ] All legacy buf_line code removed
- [ ] Build succeeds without warnings
- [ ] All unit tests pass (>80% coverage)
- [ ] Integration tests pass (real interferograms)
- [ ] Regression tests show <1% output variance
- [ ] Code review approved
- [ ] Documentation updated (this file + code comments)

### Current Progress:
- ✅ 30% Complete (DigitizationParams + RedCenterDetector)
- ❌ 70% Remaining (FringeConnector, Numberer, Adapter, Cleanup)

### Estimated Remaining Effort:
- **11-17 developer days**
- **~1000 LOC**
- **2-3 weeks** with parallel testing

---

## File Checklist

### New Files to Create:
- [ ] `DigitMode/FringeConnector.h`
- [ ] `DigitMode/FringeConnector.cpp`
- [ ] `DigitMode/FringeNumberer.h`
- [ ] `DigitMode/FringeNumberer.cpp`
- [ ] `DigitMode/FringeSegmentAdapter.h`
- [ ] `DigitMode/FringeSegmentAdapter.cpp`
- [ ] `Tests/DigitModeTests/FringeConnectorTest.cpp`
- [ ] `Tests/DigitModeTests/FringeNumbererTest.cpp`
- [ ] `Tests/DigitModeTests/FringeSegmentAdapterTest.cpp`
- [ ] `Tests/DigitModeTests/DigitizationPipelineTest.cpp`
- [ ] `Tests/DigitModeTests/DigitizationRegressionTest.cpp`

### Files to Modify:
- [ ] `DigitMode/DigitInfo.h` - Remove buf_line members
- [ ] `DigitMode/DigitInfo.cpp` - Update Auto(), remove legacy methods
- [ ] `DigitMode/DigitizationParams.h` - Add FC_* enum if needed

### Files to Clean Up:
- [ ] Remove `AnalyzeScanline()` stub from RedCenterDetector.cpp
- [ ] Move FC_* defines to proper location
- [ ] Add deprecation notices to legacy methods

---

## Related Documentation

- See: `DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md` (overall plan)
- See: `PHASE2_PROGRESS_ANALYSIS.md` (current status - this assessment)
- See: `Docs/Phase1_CompletionReport.md` (ApertureCore geometry)
- See: `Docs/Phase2_CompletionReport.md` (Draft shape creation - different project)
- See: `Docs/Phase3_TestingReport.md` (Rendering layer)
- See: `Docs/Phase4_CompletionReport.md` (UI integration)

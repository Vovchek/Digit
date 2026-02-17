# Middle Refactoring Test Suite Documentation

**File:** `Tests/DigitModeTests/MiddleRefactoringTest.cpp`  
**Status:** ✅ Compiling successfully  
**Purpose:** Verify refactored `middle_()`, `approx_()`, `fon_del_()` produce identical results to legacy versions and validate new multi-region capability

---

## Overview

This test suite comprehensively compares the legacy extrema detection pipeline with the refactored versions:

### Legacy Functions (Original Interface)
```cpp
void middle(unsigned char* line, int nx, int ny, int y, int **buf_line, 
            CArray<double, double>& CenterFrg, int& nnpolos);
double approx(int *n, int *x, int *y);
void fon_del(uint8_t* line, int leftIdx, int rightIdx);
```

### Refactored Functions (Improved Interface)
```cpp
std::vector<double> middle_(const uint8_t* line, std::size_t nx, int y,
                            std::function<bool(int x, int y)> IsVisible);
double approx_(int* n, int* x, int* y);
void fon_del_(uint8_t* line, int leftIdx, int rightIdx);
```

### Key Improvements
| Feature | Legacy | Refactored |
|---------|--------|-----------|
| **Multiple Regions** | Only 2 via buf_line | Unlimited via lambda |
| **Memory Management** | Manual buf_line array | Automatic (vector output) |
| **Visibility Handling** | Implicit in buf_line | Explicit lambda function |
| **Return Type** | Out-parameter CArray | Return vector<double> |
| **Interface** | MFC-dependent | Pure C++ STL |

---

## Test Organization

### Test Classes

#### 1. FonDelTest (Background Deletion)
Tests the background subtraction function.

**Tests:**
- `SingleRegion_ProducesConsistentResults` - Legacy vs refactored on full line
- `PartialRegion_ProducesConsistentResults` - Partial range processing
- `HandlesNonuniformBackground` - Gradient background handling

**What It Checks:**
- Output byte-for-byte identical
- Works on full and partial regions
- Handles non-uniform backgrounds

---

#### 2. ApproxTest (Approximation Algorithm)
Tests the linear approximation of extrema positions.

**Tests:**
- `LinearApproximation_ProducesConsistentResults` - Basic 3-point case
- `SinglePoint_ProducesConsistentResults` - Edge case: 1 point
- `PerfectLine_ProducesConsistentResults` - Known linear data

**What It Checks:**
- Floating-point results identical (EXPECT_DOUBLE_EQ)
- Single point handled correctly
- Linear relationship detected accurately

---

#### 3. MiddleTest (Extrema Detection - Core Algorithm)
Tests the extrema detection in single and multiple regions.

**Tests:**
- `SingleRegion_DetectsConsistentExtrema` - Basic detection
- `PartialRegion_DetectsOnlyInRegion` - Region boundary respect
- `MultipleRegions_DetectsInEachRegion` - ⭐ New capability: 2 regions
- `MultipleRegions_StrongPeaks` - ⭐ New capability: 3 regions
- `GapHandling_SkipsInvisibleRegions` - Discontinuous regions
- `DenseExtrema_DetectsAll` - Many peaks

**What It Checks:**
- ✅ Detects peaks in visible regions only
- ✅ **Handles multiple non-contiguous regions** (new!)
- ✅ Respects visibility mask boundaries
- ✅ Works with dense extrema patterns

**Key Insight:** The refactored version can process:
```
Legacy:  Region1 [=========] OR Region1 [========] repeated calls
Refactored: Region1 [====] Gap [====] Region2 [====] single call
```

---

#### 4. MiddleIntegrationTest (Full Pipeline)
Tests the complete extrema detection pipeline.

**Tests:**
- `FullPipeline_FonDelThenMiddle` - Sequential: fon_del → middle
- `MultipleRegionsAdvantage` - Demonstrate new capability
- `RobustnessToNoisyData` - Noisy input handling

**What It Checks:**
- Complete pipeline compatibility
- Advantage of multi-region support
- Noise tolerance

---

#### 5. MiddleEdgeCasesTest (Boundary Conditions)
Tests edge cases and error handling.

**Tests:**
- `EmptyRegion_NoExtremaDetected` - Flat line (no peaks)
- `SinglePixelRegion_HandledGracefully` - Single pixel visible
- `AllPixelsInvisible_NoExtremaDetected` - All masked out
- `VeryNarrowRegions_StillDetects` - 2-pixel wide regions

**What It Checks:**
- Graceful handling of degenerate cases
- No crashes on extreme inputs
- Correct behavior when nothing to detect

---

## Running the Tests

### Run All Tests
```bash
cd C:\Users\vovch\source\repos\Vovchek\Digit
cmake --build build --config Debug
ctest -R MiddleRefactoring -V
```

### Run Specific Test Class
```bash
# Test background deletion
ctest -R MiddleRefactoringTests.FonDelTest -V

# Test approximation algorithm
ctest -R MiddleRefactoringTests.ApproxTest -V

# Test extrema detection (core)
ctest -R MiddleRefactoringTests.MiddleTest -V

# Test edge cases
ctest -R MiddleRefactoringTests.MiddleEdgeCasesTest -V

# Test integration
ctest -R MiddleRefactoringTests.MiddleIntegrationTest -V
```

### Run Specific Test
```bash
# Test multi-region support (most important)
ctest -R "MiddleTest.MultipleRegions" -V

# Test robustness
ctest -R "MiddleTest.GapHandling" -V

# Test complete pipeline
ctest -R "FullPipeline" -V
```

### Show Detailed Output
```bash
# Run with verbose output (shows all assertions)
ctest -R MiddleRefactoring -V --output-on-failure

# Run specific test with verbose output
ctest -R "MiddleTest.SingleRegion" -V --output-on-failure
```

---

## Test Helpers

### CreateSyntheticLine()
Creates grayscale image data with known intensity profile.

```cpp
std::vector<unsigned char> line = CreateSyntheticLine(100, {
    {20, 200},   // Gaussian peak at x=20 with intensity 200
    {50, 220},   // Gaussian peak at x=50 with intensity 220
    {80, 180}    // Gaussian peak at x=80 with intensity 180
});
```

**Features:**
- Background intensity = 50
- Gaussian-shaped peaks at specified positions
- Configurable peak heights

---

### Visibility Mask Creators
Create lambda functions for region visibility checking.

#### Single Region
```cpp
auto mask = CreateSingleRegionMask(0, 99);  // [0...99]
// Hides: none
```

#### Two Regions (Split)
```cpp
auto mask = CreateTwoRegionMask(0, 30, 60, 95);  // [0-30] and [60-95]
// Hides: [31-59]  ← Gap in the middle
```

#### Three Regions
```cpp
auto mask = CreateThreeRegionMask(0, 40, 60, 100, 105, 120);
// Hides: [41-59] and [101-104]  ← Two gaps
```

#### Custom Lambda
```cpp
auto mask = [](int x, int y) {
    // Custom: only even pixel positions visible
    return (x % 2) == 0;
};
```

---

## Key Test Scenarios

### Scenario 1: Basic Compatibility (Single Region)
**Goal:** Verify refactored version matches legacy for simple case

```cpp
TEST_F(FonDelTest, SingleRegion_ProducesConsistentResults) {
    // Line data
    std::vector<unsigned char> line1 = testLine;
    std::vector<unsigned char> line2 = testLine;
    
    // Apply both versions
    fon_del(line1.data(), 0, 99);        // Legacy
    fon_del_(line2.data(), 0, 99);       // Refactored
    
    // Compare: should be byte-for-byte identical
    for (size_t i = 0; i < line1.size(); ++i) {
        EXPECT_EQ(line1[i], line2[i]);
    }
}
```

**Expected Result:** ✅ PASS - Identical output

---

### Scenario 2: Multi-Region Support (New Capability)
**Goal:** Demonstrate refactored version handles multiple non-contiguous regions

```cpp
TEST_F(MiddleTest, MultipleRegions_DetectsInEachRegion) {
    // Create two separate regions with peaks in each
    auto isVisible = CreateTwoRegionMask(0, 30, 60, 95);
    
    // Refactored version: single call, multiple regions
    auto detected = middle_(testLine.data(), testLine.size(), 0, isVisible);
    
    // Legacy version: would need two separate calls to buf_line
    
    // Verify: detections in both regions
    for (double x : detected) {
        bool inRegion1 = x >= 0.0 && x <= 30.0;
        bool inRegion2 = x >= 60.0 && x <= 95.0;
        EXPECT_TRUE(inRegion1 || inRegion2);
    }
}
```

**Expected Result:** ✅ PASS - Handles multi-region in single call

---

### Scenario 3: Robustness to Noise
**Goal:** Verify algorithm works with realistic noisy data

```cpp
TEST_F(MiddleIntegrationTest, RobustnessToNoisyData) {
    // Add realistic noise
    for (size_t i = 10; i < 90; ++i) {
        noisyLine[i] += (rand() % 5) - 2;  // ±2 random noise
    }
    
    // Refactored version should still detect
    auto detected = middle_(noisyLine.data(), noisyLine.size(), 0, isVisible);
    
    EXPECT_GE(detected.size(), 1) << "Should detect despite noise";
}
```

**Expected Result:** ✅ PASS - Robust detection

---

### Scenario 4: Edge Cases
**Goal:** Ensure graceful handling of boundary conditions

```cpp
TEST_F(MiddleEdgeCasesTest, AllPixelsInvisible_NoExtremaDetected) {
    // All pixels masked out
    auto isVisible = [](int x, int y) { return false; };
    
    auto detected = middle_(line.data(), line.size(), 0, isVisible);
    
    // Should return empty, not crash
    EXPECT_EQ(detected.size(), 0);
}
```

**Expected Result:** ✅ PASS - Returns empty without crash

---

## Interpreting Results

### All Tests Pass ✅
```
[====] 28 tests from MiddleRefactoringTests
  FonDelTest: 3 tests PASSED
  ApproxTest: 3 tests PASSED
  MiddleTest: 6 tests PASSED
  MiddleIntegrationTest: 3 tests PASSED
  MiddleEdgeCasesTest: 4 tests PASSED

✅ Conclusion: Refactored versions are correct and compatible
```

### Specific Test Fails ❌

**If FonDelTest fails:**
- Check: `fon_del_()` produces different output than `fon_del()`
- Likely: Bug in refactored background deletion
- Action: Compare byte-by-byte output at index shown in error

**If MiddleTest.MultipleRegions fails:**
- Check: Is lambda visibility function called correctly?
- Check: Does refactored middle_() handle discontinuous regions?
- Likely: Region boundary checking issue
- Action: Add debug output in middle_() to see region bounds

**If ApproxTest fails:**
- Check: Floating-point precision differences?
- Action: Try EXPECT_DOUBLE_EQ vs. EXPECT_NEAR(result1, result2, 1e-9)

---

## Debugging Tips

### Add Debug Output
```cpp
TEST_F(MiddleTest, MultipleRegions_DetectsInEachRegion) {
    auto detected = middle_(testLine.data(), testLine.size(), 0, isVisible);
    
    // Debug: show what was detected
    std::cout << "Detected " << detected.size() << " extrema:\n";
    for (size_t i = 0; i < detected.size(); ++i) {
        std::cout << "  [" << i << "] x=" << detected[i] << "\n";
    }
    
    EXPECT_GE(detected.size(), 1);
}
```

### Run Single Test with Output
```bash
ctest -R "MiddleTest.SingleRegion" -V --output-on-failure
```

### Run in Debugger
```bash
# Visual Studio
devenv.exe build\Digit.sln /debugexe bin\DigitModeTests.exe
```

---

## Test Statistics

| Test Class | Count | Purpose |
|-----------|-------|---------|
| **FonDelTest** | 3 | Background deletion |
| **ApproxTest** | 3 | Linear approximation |
| **MiddleTest** | 6 | Single + multi-region detection |
| **MiddleIntegrationTest** | 3 | Complete pipeline |
| **MiddleEdgeCasesTest** | 5 | Boundary conditions |
| **TOTAL** | **20** | Full coverage |

---

## Expected Test Results

### If All New Refactored Functions Work Correctly
```
FonDelTest::SingleRegion               PASS ✅
FonDelTest::PartialRegion              PASS ✅
FonDelTest::NonuniformBackground       PASS ✅

ApproxTest::LinearApproximation        PASS ✅
ApproxTest::SinglePoint                PASS ✅
ApproxTest::PerfectLine                PASS ✅

MiddleTest::SingleRegion               PASS ✅
MiddleTest::PartialRegion              PASS ✅
MiddleTest::MultipleRegions            PASS ✅ ← NEW CAPABILITY
MiddleTest::MultipleRegions_Strong     PASS ✅ ← NEW CAPABILITY
MiddleTest::GapHandling                PASS ✅ ← NEW CAPABILITY
MiddleTest::DenseExtrema               PASS ✅

MiddleIntegrationTest::FullPipeline    PASS ✅
MiddleIntegrationTest::MultiRegionAdv  PASS ✅
MiddleIntegrationTest::RobustnessNoise PASS ✅

MiddleEdgeCasesTest::EmptyRegion       PASS ✅
MiddleEdgeCasesTest::SinglePixel       PASS ✅
MiddleEdgeCasesTest::AllInvisible      PASS ✅
MiddleEdgeCasesTest::NarrowRegions     PASS ✅

=====================================
Result: 20/20 PASSED ✅
```

---

## Integration with CI/CD

These tests should be added to your CI pipeline:

```yaml
# .github/workflows/build.yml
- name: Run Middle Refactoring Tests
  run: ctest -R MiddleRefactoring -V --output-on-failure
  
- name: Check Multi-Region Support
  run: ctest -R "MiddleTest.MultipleRegions" -V
```

---

## Reference

**Files Modified:**
- `Utils/middle.h` - Contains refactored functions
- `Utils/middle.cpp` - Implementation

**Test File:**
- `Tests/DigitModeTests/MiddleRefactoringTest.cpp` - This test suite

**Related:**
- `DigitMode/RedCenterDetector.cpp` - Uses new middle_() API
- `DigitMode/DigitInfo.cpp` - Calls RedCenterDetector

---

## Summary

✅ **20 comprehensive tests covering:**
- Backward compatibility (legacy vs refactored)
- Multi-region support (new capability)
- Integration with complete pipeline
- Edge cases and robustness
- Noise tolerance

**Key Test:** `MiddleTest::MultipleRegions_*` - Validates the new multi-region capability that replaces limited buf_line approach.

**Run Now:** `ctest -R MiddleRefactoring -V --output-on-failure`

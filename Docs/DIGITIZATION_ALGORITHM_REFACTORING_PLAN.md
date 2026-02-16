# Deep Refactoring of CDigitInfo::Auto() Algorithm

## Executive Summary

The `CDigitInfo::Auto()` method is the core automatic interferogram fringe digitization algorithm. It currently has several architectural issues:

1. **Tight coupling** to legacy data structures (CDotInfo, flat Dots array)
2. **Outdated masking system** (buf_line) that needs to be replaced with aperture visibility mask
3. **Direct dependency on CBoundCtrls** (deprecated, to be phased out)
4. **MFC dependencies** for image and bounds access
5. **Monolithic algorithm** unsuitable for strategy pattern or future algorithm variants
6. **Poor testability** due to heavy mutable state and side effects

This document outlines a comprehensive refactoring strategy to modernize the algorithm while maintaining backward compatibility.

---

## Key Architectural Insights

**Critical Discovery (see Appendix A)**:
- The visibility mask is **sufficient** for all spatial queries
- Bounds objects are **NOT needed** by the algorithm
- Bounds are **inputs to mask generation**, not algorithm inputs
- Algorithm needs **ONLY**: bitmap + visibility mask + aperture center (simple point)
- This enables **complete decoupling from CBoundCtrls**, allowing safe deprecation

---

## 1. Current State Analysis

### 1.1 Current Data Flow in Auto()

```
Auto()
├── CreateBufLine()
│   ├── CreateBufLineAperture()      [Creates buf_line mask]
│   └── CreateBufLineOntruction()   [Adds obstruction mask]
├── CreateRedCenters()               [Detects extrema, populates HidenDots & Sections]
├── SelectFringeStep()               [Calculates average fringe spacing]
├── SelectMainSection()              [Identifies primary ZAP line]
├── CreateNumLines()                 [Connects extrema → fringes]
├── SelectMainFringe()               [If obstruction present]
├── CorrectNumbers()                 [Adjusts fringe numbering]
├── CreateZAPSections()              [Creates reference lines]
├── SortDotsFY()                     [Sorts dots by fringe & Y]
├── SelectMainDot()                  [Sets initial selection]
├── Delete_buf_line()                [Cleanup]
└── SyncFringesToDots()              [Legacy conversion]
```

### 1.2 Current Architecture Issues

| Issue | Location | Impact |
|-------|----------|--------|
| **buf_line dependency** | CreateBufLineAperture(), CreateBufLineOntruction() | Manual 2D array management, no visibility masking |
| **CDotInfo usage** | Dots array, CreateRedCenters(), CreateNumLines() | Deprecated data model, carries iZapSec field |
| **Direct MFC access** | CBoundCtrls*, CImageCtrls* pointers | Hard to test, couples algorithm to UI framework |
| **Global state** | Many CArray members (Dots, HidenDots, Sections, ZapLines) | Side effects, difficult to parallelize or unit test |
| **Single algorithm** | No extensibility point for alternatives | Difficult to add ML-based or adaptive algorithms |
| **Mutable iteration state** | idxMainDot, idxMainSection, CurrentNumber | Mixed algorithm state + UI state |

### 1.3 Dependencies Currently Pulled In

```
CDigitInfo
├── CDotInfo (deprecated, will be phased out)
├── CBoundCtrls (boundary shapes: ellipses, rectangles, polygons)
│   └── CArrayXYEllipse, CArrayXYRect, CArrayXYPolygon
├── CImageCtrls (image bitmap access)
│   └── SECDib (legacy image wrapper)
├── CControls (global parameters: FringeCenterAs, MaxPow, Eps, etc.)
├── CSectionInfo (horizontal scan sections)
├── CZapLineInfo (reference lines)
├── CFringeSegment (NEW: segment-primary model)
├── MFC types (CArray, CDC, CPoint, CRect, etc.)
└── buf_line[][] (C-style 2D array, int**)
```

---

## 2. Refactoring Goals & Strategy

### 2.1 Core Objectives

**By Priority:**

1. **Remove buf_line dependency** → Use aperture visibility mask from CApertureCtrls
2. **Decouple from CDotInfo** → Work directly with points and fringe numbers
3. **Decouple from MFC** → Accept data through interfaces/parameters
4. **Enable strategy pattern** → Extract core algorithm steps into pluggable stages
5. **Improve testability** → Pure functions consuming inputs, producing outputs
6. **Maintain backward compatibility** → Existing callers (Auto() invocation) must remain unchanged

### 2.2 Proposed Architecture

**New Digitization Pipeline:**

```
DigitizationAlgorithm (strategy interface)
├── Input Parameters (POD struct)
│   ├── bitmap (grayscale pixel data)
│   ├── visibility mask (from aperture)
│   ├── aperture center (simple point for reference)
│   └── parameters (extremum detection thresholds, etc.)
├── Stage 1: RedCenterDetector
│   └── Input: bitmap, visibility mask, parameters
│   └── Output: vector<ExtremumPoint> (red centers)
├── Stage 2: FringeConnector
│   └── Input: extrema, visibility mask
│   └── Output: vector<FringePolyline>
├── Stage 3: FringeNumberer
│   └── Input: polylines, aperture center (point only)
│   └── Output: vector<NumberedFringe>
└── Stage 4: CFringeSegment adapter
    └── Input: numbered fringes
    └── Output: vector<CFringeSegments> (CDigitInfo owned fringes)
```

**Critical Insight:** The visibility mask is SUFFICIENT for all spatial queries. Bounds are NOT needed.
- CBoundCtrls is used ONCE to compute the visibility mask in CApertureCtrls
- After mask is computed, algorithm only needs: bitmap + mask + aperture center (simple point)
- This allows complete decoupling from deprecated CBoundCtrls

---

## 3. Detailed Refactoring Steps

### Phase 1: Extract & Isolate (Steps 1-4)
Create new classes and prepare the ground for algorithm extraction.

### Phase 2: Replace Masking System (Steps 5-7)
Replace buf_line with aperture visibility checks.

### Phase 3: Decouple Input/Output (Steps 8-10)
Remove MFC dependencies from algorithm core, create adapter bridge.

### Phase 4: Implement Strategy Pattern (Steps 11-13)
Create pluggable algorithm stages with strategy interface.

### Phase 5: Refactor Auto() (Steps 14-17)
Rewrite Auto() to use new pipeline and adapter.

### Phase 6: Validation & Cleanup (Steps 18-20)
Test, document, and remove legacy code.

## 4. Atomic Implementation Steps

### Phase 1: Extract & Isolate

#### Step 1: Create Input Parameter Structures
**File**: `DigitMode/DigitizationParams.h` (NEW)

Define POD (Plain Old Data) structures to encapsulate algorithm inputs:

```cpp
namespace DigitMode::digitization {

// Extremum point detected in scan line
struct ExtremumPoint {
    CDPoint position;       // x, y in image coordinates
    double intensity;       // pixel intensity value
    int extremumType;       // FC_MAX or FC_MIN
    // NOTE: No scanLineIndex - derive from position.y when needed
    // scanLineIndex = static_cast<int>(position.y) - apertureTopY
};

// Detected fringe polyline (before numbering)
struct FringePolyline {
    std::vector<CDPoint> points;  // ORDERED sequence
    int index;                    // segment index (for discontinuous fringes)
};

// Numbered and finalized fringe
struct NumberedFringe {
    std::vector<CDPoint> points;
    double number;          // fringe number (1.0, 1.5, 2.0, ...)
    int segmentIndex;       // to distinguish discontinuous parts
};

// Algorithm input parameters
struct DigitizationInput {
    // Image data
    const unsigned char* bitmapData;
    int imageWidth, imageHeight;
    
    // Visibility checker: pure function with INTEGER coordinates
    // Matches VisibilityMask API directly: bool IsVisible(int x, int y)
    // No coordinate conversions needed - pixel indexing is native
    std::function<bool(int x, int y)> isVisible;
    
    // Algorithm parameters
    int fringeCenterAs;                   // FC_MAX, FC_MIN, or FC_MINMAX
    double contrastThreshold;             // for extremum detection
    int minFringeSpacing;                 // pixels
    int maxFringeSpacing;                 // pixels
    
    // Aperture center (needed ONLY for fringe numbering, as a simple point)
    // This is where floating-point may be used (geometric calculations)
    // NOT used for visibility checking (that's integer-based)
    aperture::Point apertureCenter;
};

// Algorithm output results
struct DigitizationOutput {
    std::vector<ExtremumPoint> redCenters;      // Red dots for visualization (low-level algorithm outcome)
    std::vector<FringePolyline> polylines;      // Intermediate: unnumbered polylines
    std::vector<NumberedFringe> fringes;        // Final pure output: numbered fringes (STL-only, no MFC)
    double averageFringeStep;                   // Calculated step size
    int mainFringeNumber;                       // Primary fringe index
    
    // NOTE: NO zapSections - ZAPLine concept is deprecated and not planned for future
    // NOTE: NO CFringeSegment - pure algorithm must not depend on MFC
    //       FringeSegmentAdapter (Stage 4) converts pure output to MFC format
};

} // namespace DigitMode::digitization
```

**Rationale**: POD structures decouple the algorithm from MFC and UI framework. They're easy to test, serialize, and pass around.

**Dependencies**: None (just std and geometry types)

**Tests**: Unit test structures in isolation

---

#### Step 2: Removed - VisibilityMaskAccessor No Longer Needed
**Rationale**: The pure function approach in DigitizationInput::isVisible replaces the need for a wrapper class.

Algorithm receives visibility checks as a simple callable (std::function<bool(int, int)>), which can be:
- A lambda capturing the mask provider
- A function pointer
- Any other callable type
- A template parameter (for zero-cost abstraction)

This is cleaner and more testable than a wrapper class.

---

#### Step 3 (formerly Step 3): Extract Image Data Provider Interface
**File**: `DigitMode/IImageDataProvider.h` (NEW)

Define minimal interface for image access (independent of CImageCtrls):

```cpp
namespace DigitMode {

class IImageDataProvider {
public:
    virtual ~IImageDataProvider() = default;
    
    /**
     * @brief Get grayscale bitmap data
     * @return Pointer to first pixel (row-major, bottom-to-top in DIB)
     */
    virtual const unsigned char* GetBitmapData() const = 0;
    
    /**
     * @brief Get image dimensions
     * @return Width of the image
     */
    virtual int GetImageWidth() const = 0;
    virtual int GetImageHeight() const = 0;
    
    /**
     * @brief Get pixel at (x, y) - handles DIB coordinate system
     * @param x, y Image pixel coordinates (origin top-left)
     * @return Grayscale intensity [0, 255]
     */
    virtual unsigned char GetPixel(int x, int y) const = 0;
};

} // namespace DigitMode
```

**Rationale**: Allows algorithm to work with any image source, not just CImageCtrls/SECDib. Testable with mock images.

**Dependencies**: None (pure interface)

**Tests**: Unit tests with mock implementations

---

### Phase 2: Replace Masking System

#### Step 5: Implement RedCenterDetector
**File**: `DigitMode/RedCenterDetector.h` (NEW)

Extract extremum detection into a standalone function (no mutable state, no buf_line):

```cpp
namespace DigitMode::digitization {

class RedCenterDetector {
public:
    /**
     * @brief Detect intensity extrema (red centers) in grayscale bitmap
     * 
     * Replaces CreateBufLine() + CreateRedCenters().
     * Uses visibility checker function instead of buf_line.
     * 
     * @param input Algorithm input parameters (includes isVisible function)
     * @return Vector of detected extrema
     */
    static std::vector<ExtremumPoint> DetectExtrema(
        const DigitizationInput& input);
    
private:
    /**
     * @brief Analyze a single horizontal scan line for extrema
     * 
     * Queries isVisible function to determine valid pixels on this scan line.
     * No buf_line array or wrapper class needed.
     * 
     * @param scanlineY Y coordinate of scan line
     * @param scanlineData Grayscale pixel intensity for the line
     * @param isVisible Visibility checker function: bool(int x, int y)
     * @param fringeCenterAs Extremum detection mode
     * @return Extrema found on this line
     */
    static std::vector<ExtremumPoint> AnalyzeScanline(
        int scanlineY,
        const std::vector<unsigned char>& scanlineData,
        const std::function<bool(int, int)>& isVisible,
        int fringeCenterAs);
};

} // namespace DigitMode::digitization
```

**Key Changes**:
- No buf_line creation or management
- No VisibilityMaskAccessor wrapper needed
- Receives visibility checker as pure function: `std::function<bool(int x, int y)>`
- Pure function (inputs → outputs, no state mutations)
- Easily testable with lambda visibility checkers
- Zero coupling to VisibilityMaskProvider

**Dependencies**: 
- DigitizationParams.h (includes std::function)

**Tests**: 
- Unit test with synthetic grayscale images and lambda visibility checkers
- Verify extrema detected at expected positions
- Test with different FringeCenterAs modes (FC_MAX, FC_MIN, FC_MINMAX)

**Testing Example**:
```cpp
TEST(RedCenterDetectorTest, DetectsExtremaWithLambdaVisibility) {
    // Create test input
    DigitizationInput input;
    input.bitmapData = testBitmap.data();
    input.imageWidth = 256;
    input.imageHeight = 256;
    
    // Use lambda for visibility - no wrapper class needed!
    input.isVisible = [](int x, int y) { 
        return x >= 10 && x < 246 && y >= 10 && y < 246;  // Simple rectangle
    };
    
    input.fringeCenterAs = FC_MAX;
    
    auto redCenters = RedCenterDetector::DetectExtrema(input);
    
    EXPECT_GT(redCenters.size(), 0);
}
```

---

#### Step 6: Remove CreateBufLine() and Buffer Management
**File**: `DigitMode/DigitInfo.cpp`

Delete the following methods entirely:
- `void Init_buf_line(int ny, int n)`
- `void Delete_buf_line()`
- `void CreateBufLine()`
- `void CreateBufLineAperture()`, `CreateBufLineApertureSimple()`, `CreateBufLineApertureComplex()`
- `void CreateBufLineOntruction()`, `CreateBufLineObstructionSimple()`, `CreateBufLineObstructionComplex()`

Delete member variables:
- `int** buf_line`
- `int ny_buf_line`

**Rationale**: Replaced by VisibilityMaskAccessor which queries aperture mask on-demand. No need for pre-computed buf_line.

**Search & Update All References** to buf_line in:
- CreateRedCenters()
- CSectionInfo::Form() method calls
- Any other callers

---

#### Step 7: Update CreateRedCenters() to Use Visibility Mask
**File**: `DigitMode/DigitInfo.cpp`

Rewrite CreateRedCenters() to use RedCenterDetector:

```cpp
void CDigitInfo::CreateRedCenters()
{
    HidenDots.RemoveAll();
    Sections.RemoveAll();
    
    CImageCtrls* pI = GetImageCtrls();
    CApertureCtrls* pA = GetApertureCtrls();  // NEW
    CControls* pCtrls = GetControls();
    
    // Build input parameters
    DigitizationInput input;
    input.bitmapData = pI->GetBitmapData();
    input.imageWidth = pI->ImageSize.cx;
    input.imageHeight = pI->ImageSize.cy;
    input.maskProvider = &pA->GetMaskProvider();  // NEW: Use aperture mask
    input.fringeCenterAs = pCtrls->FringeCenterAs;
    // ... other parameters
    
    // Detect extrema using new algorithm
    auto redCenters = RedCenterDetector::DetectExtrema(input);
    
    // Convert output to legacy HidenDots and Sections arrays
    for (const auto& extremum : redCenters) {
        HidenDots.Add(extremum.position);
        // Populate Sections[extremum.scanLineIndex] with extremum
    }
}
```

**Rationale**: 
- Gradually transition from buf_line to visibility mask
- Keep CDigitInfo::Auto() interface unchanged
- RedCenterDetector is pure, testable, and mask-aware

---

### Phase 3: Decouple Input/Output

#### Step 8: Create FringeConnector (Pure Algorithm)
**File**: `DigitMode/FringeConnector.h` (NEW)

Extract fringe connection logic (CreateNumLines behavior) into a pure function:

```cpp
namespace DigitMode::digitization {

class FringeConnector {
public:
    /**
     * @brief Connect extrema points into continuous fringe polylines
     * 
     * Replaces CreateNumLines().
     * Does NOT use CDotInfo or Dots array.
     * 
     * @param redCenters Detected extrema (ordered by position.y from top to bottom)
     * @param isVisible Visibility checker function: bool(int x, int y)
     * @return Vector of connected polylines (not yet numbered)
     */
    static std::vector<FringePolyline> ConnectExtrema(
        const std::vector<ExtremumPoint>& redCenters,
        const std::function<bool(int, int)>& isVisible);
    
private:
    /**
     * @brief Group extrema by scan line (horizontal pixel rows)
     * 
     * @param redCenters Input extrema
     * @param referenceY Reference Y coordinate (aperture top)
     * @return Map of scanLineIndex → vector<ExtremumPoint>
     */
    static std::map<int, std::vector<ExtremumPoint>> GroupByScanLine(
        const std::vector<ExtremumPoint>& redCenters,
        int referenceY);
    
    /**
     * @brief Determine which extremum on the next scan line connects to current point
     * 
     * Uses spatial proximity and visibility checker to find the best continuation.
     */
    static ExtremumPoint* FindNextConnectedPoint(
        const ExtremumPoint& current,
        const std::vector<ExtremumPoint>& candidatesOnNextLine,
        const std::function<bool(int, int)>& isVisible);
};

} // namespace DigitMode::digitization
```

**Implementation Notes**:
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

**Key Changes**:
- Pure function (no mutable state)
- Receives visibility checker as `std::function<bool(int, int)>` parameter
- Does NOT use VisibilityMaskAccessor wrapper
- Works with basic data structures (vector of ExtremumPoint)
- Testable with synthetic extrema and lambda visibility checkers

**Dependencies**: DigitizationParams.h

**Tests**: Unit test with synthetic extrema layouts and lambda visibility checkers

---

#### Step 9: Create FringeNumberer (Pure Algorithm)
**File**: `DigitMode/FringeNumberer.h` (NEW)

Extract numbering logic (SelectFringeStep, CreateNumLines numbering part, CorrectNumbers) into a pure function:

```cpp
namespace DigitMode::digitization {

class FringeNumberer {
public:
    /**
     * @brief Assign fringe numbers to polylines
     * 
     * Replaces SelectFringeStep() + CreateNumLines() numbering + CorrectNumbers().
     * 
     * NOTE: Obstruction information is NOT needed here.
     * The visibility mask already encodes obstruction boundaries.
     * Numbering based on distance from aperture center is sufficient.
     * 
     * @param polylines Connected polylines (unnumbered)
     * @param apertureCenter Center of the aperture (reference point for numbering, simple point only)
     * @return Numbered fringes with calculated step size and main fringe index
     */
    struct NumberingResult {
        std::vector<NumberedFringe> fringes;
        double averageFringeStep;
        int mainFringeIndex;
    };
    
    static NumberingResult NumberFringes(
        const std::vector<FringePolyline>& polylines,
        const aperture::Point& apertureCenter);
    
private:
    /**
     * @brief Calculate average fringe spacing (SelectFringeStep logic)
     * 
     * Based on distance from aperture center.
     * No bounds object needed - just use geometric analysis of polyline positions.
     */
    static double CalculateFringeStep(
        const std::vector<FringePolyline>& polylines,
        const aperture::Point& apertureCenter);
    
    /**
     * @brief Assign sequential numbers to polylines based on distance from center
     */
    static std::vector<NumberedFringe> AssignNumbers(
        const std::vector<FringePolyline>& polylines,
        double fringeStep,
        const aperture::Point& apertureCenter);
};

} // namespace DigitMode::digitization
```

**Key Changes**:
- Pure function (no side effects)
- No CDotInfo or mutable state
- NO hasObstruction parameter (obstruction is handled by mask in prior stages)
- Encapsulates all numbering logic
- Easily tested with synthetic polylines
- Aperture center is a simple point, not a bounds object

**Dependencies**: DigitizationParams.h

**Tests**: Unit test numbering with concentric polylines, verify correct step calculation and numbering sequence

---

#### Step 10: Create CFringeSegment Adapter
**File**: `DigitMode/FringeSegmentAdapter.h` (NEW)

Convert pure algorithm output (NumberedFringe) to MFC-compatible CFringeSegment:

```cpp
namespace DigitMode::digitization {

/**
 * @brief Adapter: Convert pure algorithm output to MFC-compatible CFringeSegment
 * 
 * The core digitization algorithm works with pure STL structures (NumberedFringe).
 * This adapter bridges the gap between pure algorithm and MFC-contaminated CFringeSegment.
 * 
 * RATIONALE:
 * - CFringeSegment is the first-class data model for CDigitInfo (segment-primary model)
 * - It contains MFC types (inherits from CObject, uses MFC patterns)
 * - Pure algorithm must not depend on MFC
 * - Adapter performs the final conversion: pure → MFC-compatible
 */
class FringeSegmentAdapter {
public:
    /**
     * @brief Convert pure NumberedFringe to MFC CFringeSegment
     * 
     * @param numberedFringe Pure algorithm output (STL-only)
     * @return MFC-compatible CFringeSegment for CDigitInfo storage
     */
    static CFringeSegment AdaptFringe(const NumberedFringe& numbered);
    
    /**
     * @brief Convert entire vector of pure fringes to MFC fringes
     * 
     * Batch conversion for StandardDigitizer output.
     * 
     * @param fringes Vector of pure NumberedFringe objects
     * @return Vector of MFC CFringeSegment objects ready for CDigitInfo::Fringes
     */
    static std::vector<CFringeSegment> AdaptFringes(
        const std::vector<NumberedFringe>& fringes);
};

} // namespace DigitMode::digitization
```

**Implementation**:

```cpp
CFringeSegment FringeSegmentAdapter::AdaptFringe(const NumberedFringe& numbered) {
    CFringeSegment adapted(numbered.number, numbered.segmentIndex);
    for (const auto& point : numbered.points) {
        adapted.AddPoint(point);
    }
    return adapted;
}

std::vector<CFringeSegment> FringeSegmentAdapter::AdaptFringes(
    const std::vector<NumberedFringe>& fringes) {
    std::vector<CFringeSegment> result;
    for (const auto& fringeData : fringes) {
        result.push_back(AdaptFringe(fringeData));
    }
    return result;
}
```

**Architecture**:
```
Pure Algorithm               Adapter                    CDigitInfo
━━━━━━━━━━━━━━            ━━━━━━━━━━━━━              ━━━━━━━━━━━━
std::vector<              FringeSegmentAdapter        std::vector<
  NumberedFringe  ━━━╋→  (Stage 4, outside       ━━→   CFringeSegment
  (STL-only)             pure pipeline)               (MFC-compatible)
```

**Key Insight**:
- Pure algorithm is completely MFC-free
- Only the adapter touches CFringeSegment (MFC-contaminated)
- Adapter is thin and testable in isolation

**Dependencies**: DigitizationParams.h, CFringeSegment.h

**Tests**: Unit test adapting NumberedFringe → CFringeSegment, verify point counts and fringe numbers preserved

---

## Phase 4: Implement Strategy Pattern

#### Step 11: Create DigitizationStrategy Interface
**File**: `DigitMode/IDigitizationStrategy.h` (NEW)

Define an abstract strategy interface for algorithm variants:

```cpp
namespace DigitMode::digitization {

/**
 * @brief Abstract strategy for interferogram digitization
 * 
 * Allows plugging in different algorithms:
 * - StandardDigitizer (current approach)
 * - MLBasedDigitizer (future: neural network)
 * - AdaptiveDigitizer (future: parameter-adaptive)
 */
class IDigitizationStrategy {
public:
    virtual ~IDigitizationStrategy() = default;
    
    /**
     * @brief Execute digitization pipeline
     * @param input Algorithm parameters and image/mask data
     * @return Complete digitization results
     */
    virtual DigitizationOutput Digitize(const DigitizationInput& input) = 0;
    
    /**
     * @brief Return human-readable name for this strategy
     */
    virtual std::string GetName() const = 0;
};

} // namespace DigitMode::digitization
```

**Rationale**: 
- Encapsulates entire algorithm choice point
- Allows multiple implementations (standard, adaptive, ML-based, etc.)
- Future-proof for algorithm evolution

**Dependencies**: DigitizationParams.h

---

#### Step 12: Implement StandardDigitizer Strategy
**File**: `DigitMode/StandardDigitizer.h/cpp` (NEW)

Concrete implementation using the extracted stages:

```cpp
namespace DigitMode::digitization {

class StandardDigitizer : public IDigitizationStrategy {
public:
    std::string GetName() const override { return "Standard"; }
    
    DigitizationOutput Digitize(const DigitizationInput& input) override;
    
private:
    /**
     * @brief Execute the standard pipeline:
     * Stage 1: RedCenterDetector::DetectExtrema
     * Stage 2: FringeConnector::ConnectExtrema
     * Stage 3: FringeNumberer::NumberFringes
     * Stage 4: ZAPSectionBuilder::BuildZAPSections
     */
};

} // namespace DigitMode::digitization
```

**Implementation**:

```cpp
DigitizationOutput StandardDigitizer::Digitize(const DigitizationInput& input) {
    DigitizationOutput output;
    
    // Stage 1: Detect red centers
    // isVisible function is passed through input
    auto redCenters = RedCenterDetector::DetectExtrema(input);
    output.redCenters = redCenters;
    
    // Stage 2: Connect into polylines
    // Pass visibility checker directly to connector
    auto polylines = FringeConnector::ConnectExtrema(
        redCenters, 
        input.isVisible  // Pure function, not a wrapper object
    );
    output.polylines = polylines;
    
    // Stage 3: Number fringes
    auto numberingResult = FringeNumberer::NumberFringes(
        polylines,
        input.apertureCenter
    );
    output.fringes = numberingResult.fringes;
    output.averageFringeStep = numberingResult.averageFringeStep;
    output.mainFringeNumber = numberingResult.mainFringeIndex;
    
    return output;
}
```

**Key Benefits**:
- Pure STL pipeline (no MFC contamination)
- No VisibilityMaskAccessor wrapper needed
- Visibility checker is a simple callable: can be lambda, function, functor, etc.
- Easy to mock in tests with lambdas
- Zero coupling to VisibilityMaskProvider class
- Template version available for compile-time optimization (see Appendix D)

**Dependencies**: All stage classes (RedCenterDetector, FringeConnector, etc.)

**Tests**: Integration test verifying complete pipeline with synthetic interferogram

---

#### Step 13: Create DigitizationStrategyFactory
**File**: `DigitMode/DigitizationStrategyFactory.h` (NEW)

Factory for creating strategy instances:

```cpp
namespace DigitMode::digitization {

class DigitizationStrategyFactory {
public:
    enum StrategyType {
        STANDARD,      // Current standard algorithm
        ADAPTIVE,      // Placeholder for future
        ML_BASED,      // Placeholder for future
    };
    
    /**
     * @brief Create a strategy instance
     * @param type Strategy type to instantiate
     * @return Unique pointer to strategy (caller owns lifetime)
     */
    static std::unique_ptr<IDigitizationStrategy> CreateStrategy(
        StrategyType type = STANDARD);
    
    /**
     * @brief Get all available strategy types
     * @return Vector of type identifiers
     */
    static std::vector<StrategyType> GetAvailableStrategies();
};

} // namespace DigitMode::digitization
```

**Rationale**: Centralizes algorithm selection, makes it easy to add/remove strategies later

**Dependencies**: IDigitizationStrategy.h, StandardDigitizer.h

---

#### Step 14: Update Auto() to Use Strategy
**File**: `DigitMode/DigitInfo.cpp`

Refactor Auto() to delegate to strategy, WITHOUT accessing CBoundCtrls:

```cpp
void CDigitInfo::Auto()
{
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    Clear(FALSE);
    
    // Get ONLY the dependencies needed: image and aperture mask
    CImageCtrls* pI = GetImageCtrls();
    CApertureCtrls* pA = GetApertureCtrls();      // Aperture mask is the ONLY spatial dependency
    CControls* pCtrls = GetControls();
    
    // NOTE: NO GetBoundCtrls() call!
    // Bounds are already encoded in the visibility mask.
    
    // Build input parameters
    digitization::DigitizationInput input = BuildDigitizationInput(pI, pA, pCtrls);
    
    // Create strategy (currently always Standard, but can be parameterized)
    auto strategy = digitization::DigitizationStrategyFactory::CreateStrategy(
        digitization::DigitizationStrategyFactory::STANDARD
    );
    
    // Execute digitization (pure algorithm, no side effects)
    auto output = strategy->Digitize(input);
    
    // Convert output back to legacy CDigitInfo data structures
    ConvertOutputToLegacyModel(output);
    
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

private:
    digitization::DigitizationInput BuildDigitizationInput(
        CImageCtrls* pI,
        CApertureCtrls* pA,
        CControls* pCtrls)
    {
        digitization::DigitizationInput input;
        
        // Image data
        input.bitmapData = pI->GetBitmapData();
        input.imageWidth = pI->ImageSize.cx;
        input.imageHeight = pI->ImageSize.cy;
        
        // Visibility checker: pure function with INTEGER coordinates
        // Lambda directly wraps VisibilityMask API (int, int → bool)
        // No coordinate conversion needed - pixel indexing is native
        input.isVisible = [pA](int x, int y) {
            // VisibilityMask::IsVisible(int, int) is called directly
            return pA->GetMaskProvider()->IsVisible(x, y);
        };
        
        // Aperture center: floating-point geometric reference
        // Used ONLY for fringe numbering distance calculations
        // NEVER passed to visibility checker (integer pixels)
        input.apertureCenter = ComputeApertureCenterFromShapes(pA);
        
        // Algorithm parameters
        input.fringeCenterAs = pCtrls->FringeCenterAs;
        input.contrastThreshold = pCtrls->Eps;
        // ... other parameters
        
        return input;
    }
    
    aperture::Point ComputeApertureCenterFromShapes(CApertureCtrls* pA)
    {
        // Option 1: Query aperture shapes directly from CApertureCtrls
        // Option 2: Compute from bounding box of external shapes
        // This is just a simple point computation, not bounds object
        
        auto externalShapes = pA->GetShapes().getExternal();
        if (externalShapes.empty()) {
            return aperture::Point(0, 0);
        }
        
        // Compute center of external aperture
        // (bounding box center, or shape-specific center)
        // Result: simple aperture::Point
        aperture::Point center = pA->ComputeApertureBoundingBoxCenter();
        return center;
    }
    
    void ConvertOutputToLegacyModel(const digitization::DigitizationOutput& output)
    {
        // Stage 4 (outside pure algorithm): Convert pure output to MFC-compatible storage
        
        // Convert pure NumberedFringe → MFC CFringeSegment
        // This is where MFC contamination enters, so it's separated from pure algorithm
        auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
        
        // Populate Fringes (segment-primary model)
        Fringes.clear();
        for (const auto& fringeSegment : mfcFringes) {
            Fringes.push_back(fringeSegment);
        }
        
        // Keep legacy Dots for backward compatibility (if still needed)
        if (!m_bUseFringeModel) {
            SyncFringesToDots();
        }
        
        // Store red centers for visualization (low-level algorithm outcome)
        HidenDots.RemoveAll();
        for (const auto& redCenter : output.redCenters) {
            HidenDots.Add(redCenter.position);
        }
        
        // NO ZAP sections - that concept is deprecated
        // ZapLines are no longer generated automatically
        // User-defined ZapLines (if any) are preserved by Clear(FALSE)
        
        MainFringeNumber = output.mainFringeNumber;
        idxMainPoint = SelectedPoint(0, 0);  // Select first point of first fringe
    }
```

**Key Benefits**:
- Auto() becomes thin adapter between MFC world and pure algorithm
- **ZERO CBoundCtrls dependency** - algorithm is completely decoupled
- Easy to test strategy in isolation
- Easy to swap algorithms without touching Auto()
- Clear separation of concerns

**Dependencies**: digitization::IDigitizationStrategy, DigitizationStrategyFactory

---

### Phase 5: Refactor Auto() Complete Implementation

#### Step 15: Update Auto() Main Implementation
Already covered in Step 14 - Auto() becomes a thin wrapper.

---

#### Step 16: Remove Legacy Method Implementations
**File**: `DigitMode/DigitInfo.cpp`

Delete methods that are now redundant:
- `CreateNumLines()` (replaced by FringeConnector)
- `SelectFringeStep()` (replaced by FringeNumberer)
- `CreateZAPSections()` (replaced by ZAPSectionBuilder)
- `CorrectNumbers()` (logic moved to FringeNumberer)
- `SelectMainFringe()` (logic moved to FringeNumberer)
- `SelectMainSection()` (replaced by visibility queries)

**Keep** methods used elsewhere:
- `SyncFringesToDots()` (still needed for legacy Dots → Fringes conversion)
- `SyncDotsToFringes()` (may still be needed)

**Update** method signatures:
- `CreateRedCenters()` → Now wraps RedCenterDetector, no longer builds buf_line

---

#### Step 17: Transition to New Segment-Primary Model
**File**: `DigitMode/DigitInfo.cpp`

Once strategies are working with pure output structures, update the final conversion:

```cpp
void CDigitInfo::ConvertOutputToLegacyModel(const digitization::DigitizationOutput& output)
{
    // Stage 4 (outside pure algorithm): Convert pure output to MFC-compatible storage
    
    // Convert pure NumberedFringe → MFC CFringeSegment
    // This is where MFC contamination enters, so it's separated from pure algorithm
    auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
    
    // Populate Fringes (segment-primary model)
    Fringes.clear();
    for (const auto& fringeSegment : mfcFringes) {
        Fringes.push_back(fringeSegment);
    }
    
    // Keep legacy Dots for backward compatibility (if still needed)
    if (!m_bUseFringeModel) {
        SyncFringesToDots();
    }
    
    // Store red centers for visualization (low-level algorithm outcome)
    HidenDots.RemoveAll();
    for (const auto& redCenter : output.redCenters) {
        HidenDots.Add(redCenter.position);
    }
    
    // NO ZAP sections - that concept is deprecated
    // ZapLines are no longer generated automatically
    // User-defined ZapLines (if any) are preserved by Clear(FALSE)
    
    MainFringeNumber = output.mainFringeNumber;
    idxMainPoint = SelectedPoint(0, 0);  // Select first point of first fringe
}
```

---

## Phase 6: Validation & Cleanup

#### Step 18: Unit Tests for Pure Algorithms
**Files**: 
- `Tests/DigitModeTests/RedCenterDetectorTest.cpp` (NEW)
- `Tests/DigitModeTests/FringeConnectorTest.cpp` (NEW)
- `Tests/DigitModeTests/FringeNumbererTest.cpp` (NEW)
- `Tests/DigitModeTests/FringeSegmentAdapterTest.cpp` (NEW - formerly ZAPSectionBuilderTest)
- `Tests/DigitModeTests/StandardDigitizerTest.cpp` (NEW)

Each test file verifies:
- Pure functions with synthetic inputs
- Expected outputs
- Edge cases (small images, no obstruction, dense/sparse fringes, etc.)

**Example Test Structure (Pure Algorithm)**:

```cpp
TEST(RedCenterDetectorTest, DetectsMaximaInSimpleScanline) {
    // Create synthetic grayscale image with known extrema
    std::vector<unsigned char> testBitmap = { /* synthetic data */ };
    
    // Create mock visibility mask
    MockVisibilityMaskProvider mockMask;
    EXPECT_CALL(mockMask, IsVisible(_)).WillRepeatedly(Return(true));
    
    // Run detection
    digitization::DigitizationInput input;
    input.bitmapData = testBitmap.data();
    input.fringeCenterAs = FC_MAX;
    input.maskProvider = &mockMask;
    // ... set other fields
    
    auto redCenters = RedCenterDetector::DetectExtrema(input);
    
    // Verify extrema at expected positions
    EXPECT_EQ(redCenters.size(), expectedCount);
    EXPECT_THAT(redCenters[0].position, PositionNear({expectedX, expectedY}));
}
```

**Example Test for Adapter**:

```cpp
TEST(FringeSegmentAdapterTest, ConvertsNumberedFringeToMFC) {
    // Create pure algorithm output
    digitization::NumberedFringe pureData;
    pureData.number = 3.5;
    pureData.segmentIndex = 0;
    pureData.points = { {10, 20}, {11, 21}, {12, 22} };
    
    // Adapt to MFC
    CFringeSegment adapted = FringeSegmentAdapter::AdaptFringe(pureData);
    
    // Verify MFC version matches pure version
    EXPECT_EQ(adapted.GetNumber(), 3.5);
    EXPECT_EQ(adapted.GetIndex(), 0);
    EXPECT_EQ(adapted.GetPointCount(), 3);
    EXPECT_EQ(adapted.GetPoint(0).x, 10);
    EXPECT_EQ(adapted.GetPoint(0).y, 20);
}
```

**Dependencies**: Google Test, Google Mock

---

#### Step 19: Integration Test with Real Data
**File**: `Tests/DigitModeTests/DigitizationIntegrationTest.cpp` (NEW)

Test the complete pipeline with real or realistic interferogram images:

```cpp
TEST(StandardDigitizerTest, ProcessesRealInterferogramSuccessfully) {
    // Load real test interferogram image
    auto bitmap = LoadTestImage("test_interferogram.bmp");
    
    // Set up aperture (circular aperture, central obstruction)
    CApertureCtrls aperture(imageProvider);
    aperture.AddExternalShape(CreateCircle(512, 512, 500));
    aperture.AddInternalShape(CreateCircle(512, 512, 100));
    aperture.GetMaskProvider().Invalidate();
    
    // Build input
    digitization::DigitizationInput input;
    input.bitmapData = bitmap.data();
    input.imageWidth = bitmap.width();
    input.imageHeight = bitmap.height();
    input.maskProvider = &aperture.GetMaskProvider();
    input.fringeCenterAs = FC_MINMAX;
    
    // Digitize
    StandardDigitizer digitizer;
    auto output = digitizer.Digitize(input);
    
    // Verify reasonable results
    EXPECT_GT(output.fringes.size(), 0);
    EXPECT_GT(output.averageFringeStep, 0);
    EXPECT_GT(output.zapSections.size(), 0);
    
    // Verify no CDotInfo or buf_line was used
    // (covered by fact that this uses pure algorithm classes)
}
```

---

#### Step 20: Deprecation & Cleanup
**File**: `DigitMode/DigitInfo.h` & `.cpp`

Add deprecation notices to legacy methods:

```cpp
/**
 * @deprecated Use DigitizationStrategy instead
 * This method is kept for backward compatibility only
 */
OBSOLETE_API void CreateNumLines();

/**
 * @deprecated CDotInfo is deprecated. Use CFringeSegment instead.
 * @see CFringeSegment
 */
class CDotInfo { /* ... */ };
```

Add comments marking sections to be removed in future major version:

```cpp
// TODO (v3.0): Remove these methods when fully migrated to pure digitization strategies
// - CreateBufLine()
// - CreateBufLineAperture()
// - CreateBufLineOntruction()
// - CreateRedCenters() (old implementation)
// - CreateNumLines()
// - SelectFringeStep()
// - SelectMainFringe()
// - CorrectNumbers()
// - CreateZAPSections()

// TODO (v3.0): Remove CDotInfo entirely when transitioning to segment-primary model is complete
```

---

## 10. Timeline Estimate

| Phase | Steps | Effort | Duration |
|-------|-------|--------|----------|
| **Phase 1: Extract & Isolate** | 1-3 | 2-3 weeks | Low risk, foundational |
| **Phase 2: Replace Masking** | 4-6 | 1-2 weeks | Medium complexity |
| **Phase 3: Decouple I/O** | 7-9 | 2-3 weeks | Testing-heavy |
| **Phase 4: Strategy Pattern** | 10-13 | 1-2 weeks | Design-heavy, straightforward implementation |
| **Phase 5: Refactor Auto()** | 14-16 | 1 week | Straightforward |
| **Phase 6: Validation** | 17-20 | 2-3 weeks | Test-heavy |
| **TOTAL** | **20 steps** | **9-14 weeks** | Parallelizable |

**Note**: File count is 19 (removed IBoundsProvider). Step count remains 20 due to reorganization.

## 11. Success Criteria

✅ **Completed when:**

1. ✅ All 19 files created and unit tested
2. ✅ `Auto()` uses strategy pattern (pure algorithm)
3. ✅ No buf_line allocations anywhere
4. ✅ No CDotInfo usage in core algorithm
5. ✅ Zero MFC includes in digitization algorithm files
6. ✅ Visibility mask used for all spatial queries (NO bounds objects)
7. ✅ **NO CBoundCtrls dependency** in digitization algorithm
8. ✅ 100% unit test coverage for pure algorithm stages
9. ✅ Integration tests pass with real interferograms
10. ✅ Regression tests show identical output vs. old implementation
11. ✅ Code review approved
12. ✅ Documentation updated

**Architectural Achievement**: Complete decoupling from deprecated CBoundCtrls, making it safe to deprecate.

---

## Appendix A: Critical Architectural Insight - Why Bounds Are NOT Needed

**Problem**: Early drafts of this plan included `IBoundsProvider` interface to supply aperture/obstruction bounds to the algorithm.

**Why This Is Wrong**:
1. **Bounds are inputs to mask generation, not algorithm inputs**
   - CBoundCtrls → CApertureCtrls (computes mask)
   - Visibility mask → algorithm (queries mask)

2. **The visibility mask encodes ALL spatial information**
   - Aperture boundaries → visible pixels
   - Obstruction boundaries → invisible pixels
   - Mask is the definitive spatial reference

3. **Passing bounds perpetuates CBoundCtrls dependency**
   - Would require `IBoundsProvider` interface
   - Would delay deprecation of CBoundCtrls
   - Adds unnecessary complexity

4. **Algorithm needs ONLY**:
   - Bitmap data (pixels)
   - Visibility mask (spatial queries)
   - Aperture center (simple point, for numbering reference)

**Correct Architecture**:
```
CBoundCtrls (deprecated)
    ↓ (used ONCE to.compute the visibility mask)
    ↓
CApertureCtrls::GetMaskProvider()
    ↓
    ↓ (algorithm queries mask, never accesses bounds)
    ↓
DigitizationAlgorithm
    ├── Query mask: "IsVisible(x, y)?"
    ├── Query mask: "GetScanlineRange(y)?"
    └── Use aperture center point for numbering
```

**Result**: Zero CBoundCtrls dependency in algorithm, making it safe to deprecate CBoundCtrls in future versions.

---

## Appendix B: Adapter Pattern - Pure Algorithm to MFC Bridge

**Problem**: The core digitization algorithm must be pure (no MFC dependencies) to be testable and extensible. However, the final output must integrate with CDigitInfo, which uses MFC-dependent CFringeSegment.

**Solution**: FringeSegmentAdapter acts as a bridge:

```
Pure Algorithm               Adapter                    CDigitInfo
━━━━━━━━━━━━━━            ━━━━━━━━━━━━━              ━━━━━━━━━━━━
std::vector<              FringeSegmentAdapter        std::vector<
  NumberedFringe  ━━━╋→  (Stage 4, outside       ━━→   CFringeSegment
  (STL-only)             pure pipeline)               (MFC-compatible)
```

**Architecture**:

```cpp
// Stage 1-3: Pure algorithm (StandardDigitizer)
DigitizationOutput output = strategy->Digitize(input);
// output.fringes = vector<NumberedFringe> (pure STL)

// Stage 4: Adapter bridge (in CDigitInfo::ConvertOutputToLegacyModel)
auto mfcFringes = FringeSegmentAdapter::AdaptFringes(output.fringes);
// mfcFringes = vector<CFringeSegment> (MFC-compatible)

// Final storage
Fringes.clear();
for (auto& fringeSegment : mfcFringes) {
    Fringes.push_back(fringeSegment);
}
// Fringes is now populated with MFC CFringeSegment objects
```

**Key Benefits**:

1. **Pure Algorithm Purity**
   - StandardDigitizer has ZERO MFC dependencies
   - All three stages (RedCenterDetector, FringeConnector, FringeNumberer) are testable in isolation
   - Can be replaced with ML, adaptive, or parallel variants without touching MFC

2. **Adapter Simplicity**
   - FringeSegmentAdapter is thin and focused (just converts STL → MFC)
   - Easy to test (verify each NumberedFringe maps correctly)
   - Easily modified if CFringeSegment structure changes

3. **Separation of Concerns**
   - MFC contamination enters ONLY at the boundary
   - CDigitInfo handles the bridge, not the algorithm
   - Future algorithms don't need to know about MFC

4. **Red Dots for Visualization**
   - ExtremumPoint objects (red centers) preserved in output
   - Stored in CDigitInfo::HidenDots for visualization
   - Shows low-level algorithm outcome (what extrema were detected)

**Why NOT CFringeSegment in Pure Output?**
- CFringeSegment inherits from CObject (MFC)
- Uses MFC patterns (serialization, diagnostics, etc.)
- Pure algorithm should never depend on framework classes
- Adapter pattern is the correct architectural solution


---

## Appendix C: Design Note - No Redundant Data in ExtremumPoint

**Question**: Why not store `scanLineIndex` in ExtremumPoint for fast grouping in Stage 2?

**Answer**: Because it's redundant and doesn't improve algorithmic complexity.

### The Redundancy
`ExtremumPoint::position.y` already encodes which horizontal scan line the extremum came from:
```cpp
// These encode the SAME information:
int scanLineIndex = static_cast<int>(extremum.position.y) - apertureTopY;  // O(1)
int y = extremum.position.y;                                               // already have it
```

### Why Not Store It?
1. **Single source of truth principle** - position.y is definitive
2. **No algorithmic benefit** - Both approaches are O(n) for grouping:
   - Pre-computed: `groupedByLine[extremum.scanLineIndex].push_back(...)` = O(1) per extremum
   - On-demand: `groupedByLine[static_cast<int>(extremum.position.y) - ref].push_back(...)` = O(1) per extremum
3. **Simpler data model** - ExtremumPoint has only essential fields
4. **Easier maintenance** - One less field to keep synchronized

### Implementation
FringeConnector groups extrema on-demand:
```cpp
std::map<int, std::vector<ExtremumPoint>> grouped;
for (const auto& extremum : redCenters) {
    int scanLineIndex = static_cast<int>(extremum.position.y) - referenceY;
    grouped[scanLineIndex].push_back(extremum);  // O(1) grouping, no data duplication
}
```

This is cleaner, more maintainable, and costs nothing in performance.

---

## Appendix D: Template-Based Approach - Universal Flexibility

**Alternative to std::function**: For maximum flexibility and zero-cost abstraction, use templates.

### Coordinate Systems

**Important Distinction:**
- **Visibility checking** (bitmap pixel queries): **INTEGER coordinates** - matches VisibilityMask API
- **Aperture center** (geometric reference point): **FLOATING-POINT** (aperture::Point)
- **Downstream processing** (if any): May use floating-point Point(double, double)

This is correct because:
1. Pixels are discrete integer coordinates (you can't have pixel 123.45)
2. VisibilityMask::IsVisible(int x, int y) expects integers
3. Aperture center is geometric (may be at sub-pixel coordinates)
4. The two never mix in the pure algorithm

### Runtime Flexibility: std::function<bool(int, int)>

```cpp
struct DigitizationInput {
    std::function<bool(int, int)> isVisible;  // Runtime polymorphism
    // Easy to test with lambdas, but small runtime cost
};

// Call site
input.isVisible = [pA](int x, int y) { 
    return pA->GetMaskProvider()->IsVisible(x, y);
};

auto extrema = RedCenterDetector::DetectExtrema(input);
```

**Pros:**
- Simple to understand and use
- Easy to test with different lambdas
- Works with any callable

**Cons:**
- Small runtime cost (virtual function call, type erasure)
- Visibility checker passed as member of struct

### Compile-Time Flexibility: Template Approach
```cpp
namespace DigitMode::digitization {

template<typename VisibilityChecker>
class RedCenterDetectorT {
public:
    static std::vector<ExtremumPoint> DetectExtrema(
        const unsigned char* bitmapData,
        int imageWidth, int imageHeight,
        const VisibilityChecker& isVisible,
        int fringeCenterAs);
};

// Type alias for convenience
using RedCenterDetector = RedCenterDetectorT<std::function<bool(int, int)>>;

} // namespace DigitMode::digitization
```

**Pros:**
- Zero-cost abstraction (inlined at compile time)
- Works with any callable (lambda, function, functor, class method)
- Maximum performance
- No virtual function overhead

**Cons:**
- More complex template syntax
- Requires header-only implementation (or explicit instantiations)
- Longer compilation times

### Recommended Approach

**For now**: Use `std::function<bool(int, int)>` in DigitizationInput
- Simple to implement and understand
- Runtime cost is negligible for digitization algorithm
- Easy to test and maintain
- Can evolve to templates if profiling shows need

**Future**: Consider template version if performance profiling identifies visibility checks as bottleneck
- Add as alternative implementation
- Keep std::function version for backward compatibility
- Profile before optimizing

### Example: Switching to Template (Future)

```cpp
// Header with template
template<typename VisibilityChecker>
class RedCenterDetectorT {
    // Implementation
};

// Explicit instantiation for std::function
template class RedCenterDetectorT<std::function<bool(int, int)>>;

// Type alias maintains same interface
using RedCenterDetector = RedCenterDetectorT<std::function<bool(int, int)>>;

// Test with lambda (zero overhead with template)
auto isVisibleLambda = [](int x, int y) { return true; };
auto extrema = RedCenterDetectorT<decltype(isVisibleLambda)>::DetectExtrema(
    bitmap, width, height, isVisibleLambda, FC_MAX
);
```

**Conclusion**: Start with std::function, switch to templates only if performance analysis requires it.

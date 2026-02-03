# Sprint 1 Quick Start: Fringe-Native CreateNumLines()

## Goal
Implement `CreateNumLinesFringeModel()` to build Fringes directly from red centers, replacing the current dot-based construction.

## Current vs. New Approach

### Current (Dot-Based)
```
CreateRedCenters() → HidenDots, Sections
                   ↓
CreateNumLines()   → Builds Dots array
                   → Lost: segment boundaries, continuity
                   ↓
SyncDotsToFringes() → Reconstructs Fringes (approximation)
```

### New (Fringe-Native)
```
CreateRedCenters() → HidenDots, Sections
                   ↓
CreateNumLinesFringeModel() → Builds Fringes directly
                            → Preserves: segment boundaries, continuity
                            ↓
SyncFringesToDots() → Generates Dots (backward compat)
```

---

## Implementation Steps

### Step 1: Create Helper Structures

**File:** `DigitMode/DigitInfo.h` (private section)

```cpp
private:
    /**
     * @brief Active fringe tracking during construction
     */
    struct ActiveFringe {
        int iFringe;      ///< Index into Fringes array
        CDPoint lastPoint; ///< Last point added to this fringe
        int lastSection;   ///< Last section index where point was added
        
        ActiveFringe() : iFringe(-1), lastSection(-1) {}
        ActiveFringe(int i, const CDPoint& pt, int sec) 
            : iFringe(i), lastPoint(pt), lastSection(sec) {}
    };
```

---

### Step 2: Implement Fringe-Native CreateNumLines()

**File:** `DigitMode/DigitInfo.cpp`

**Add new method:**

```cpp
/**
 * @brief Build Fringes directly from detected red centers (fringe-native)
 * 
 * **Algorithm**:
 * 1. For each section (top to bottom):
 *    a. Get red centers in this section
 *    b. For each center, try to continue an existing fringe
 *    c. If no match, start a new fringe
 *    d. Terminate fringes that have no continuation
 * 
 * **Output**: Populates Fringes vector with continuous polylines
 * 
 * @see CreateNumLinesDotModel() for legacy implementation
 */
void CDigitInfo::CreateNumLinesFringeModel()
{
    // Track active fringes by number
    // Key = fringe number, Value = ActiveFringe info
    std::map<double, ActiveFringe> activeFringes;
    
    double continuityThreshold = SecSegm * CorrectionSecSegm;
    if (continuityThreshold < 0) continuityThreshold = 5.0; // Default
    
    // Process sections top to bottom
    for (int iSec = 0; iSec < Sections.GetSize(); iSec++) {
        CSectionInfo& section = Sections[iSec];
        double sectionY = section.L.P1.y;
        
        // Get red centers in this section
        std::vector<RedCenterInfo> centersInSection;
        for (int iLine = 0; iLine < section.NumLines.GetSize(); iLine++) {
            RedCenterInfo rc;
            rc.x = section.NumLines[iLine].redX;
            rc.y = sectionY;
            rc.number = section.NumLines[iLine].Number;
            rc.iSection = iSec;
            rc.iLine = iLine;
            centersInSection.push_back(rc);
        }
        
        // Sort centers by x-coordinate (left to right)
        std::sort(centersInSection.begin(), centersInSection.end(),
            [](const RedCenterInfo& a, const RedCenterInfo& b) {
                return a.x < b.x;
            });
        
        // Track which fringes were continued in this section
        std::set<double> continuedNumbers;
        
        // Try to continue existing fringes
        for (auto& rc : centersInSection) {
            CDPoint pt(rc.x, rc.y);
            bool continued = false;
            
            // Find nearest active fringe
            double minDist = std::numeric_limits<double>::max();
            double bestNumber = -1;
            
            for (auto& [number, active] : activeFringes) {
                double dist = Distance(pt, active.lastPoint);
                if (dist < minDist && dist <= continuityThreshold) {
                    // Check if numbers are compatible
                    double numberDiff = std::abs(number - rc.number);
                    if (numberDiff < numStep * 0.5) {  // Within half step
                        minDist = dist;
                        bestNumber = number;
                    }
                }
            }
            
            if (bestNumber >= 0) {
                // Continue existing fringe
                auto& active = activeFringes[bestNumber];
                AddPointToFringe(active.iFringe, pt);
                active.lastPoint = pt;
                active.lastSection = iSec;
                continuedNumbers.insert(bestNumber);
                continued = true;
            }
            
            if (!continued) {
                // Start new fringe
                int iFringe = CreateFringe(rc.number, -1);
                AddPointToFringe(iFringe, pt);
                activeFringes[rc.number] = ActiveFringe(iFringe, pt, iSec);
                continuedNumbers.insert(rc.number);
            }
        }
        
        // Terminate fringes that weren't continued
        // (Remove them from active list)
        std::vector<double> toRemove;
        for (auto& [number, active] : activeFringes) {
            if (continuedNumbers.find(number) == continuedNumbers.end()) {
                // Fringe not continued in this section
                if (iSec - active.lastSection > 2) {
                    // Allow 2-section gap for noisy data
                    toRemove.push_back(number);
                }
            }
        }
        
        for (double num : toRemove) {
            activeFringes.erase(num);
        }
    }
    
    TRACE("CreateNumLinesFringeModel: Created %zu fringes\n", Fringes.size());
}

/**
 * @brief Helper to get red center info from a section
 */
struct RedCenterInfo {
    double x;
    double y;
    double number;
    int iSection;
    int iLine;
};

/**
 * @brief Calculate Euclidean distance between two points
 */
double CDigitInfo::Distance(const CDPoint& a, const CDPoint& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
```

**Add helper struct to header:**

```cpp
// In DigitInfo.h, private section
private:
    struct ActiveFringe {
        int iFringe;
        CDPoint lastPoint;
        int lastSection;
        
        ActiveFringe() : iFringe(-1), lastSection(-1) {}
        ActiveFringe(int i, const CDPoint& pt, int sec) 
            : iFringe(i), lastPoint(pt), lastSection(sec) {}
    };
    
    struct RedCenterInfo {
        double x, y;
        double number;
        int iSection, iLine;
    };
    
    static double Distance(const CDPoint& a, const CDPoint& b);
```

---

### Step 3: Update Auto() to Use New Method

**File:** `DigitMode/DigitInfo.cpp`

```cpp
void CDigitInfo::Auto()
{
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    Clear(FALSE);
    
    // Common steps
    CreateBufLine();
    CreateRedCenters();
    SelectFringeStep();
    SelectMainSection();
    
    if (m_bUseFringeModel) {
        // NEW: Build Fringes directly
        CreateNumLinesFringeModel();
        
        // Optional: Generate Dots for backward compatibility
        if (NeedDotsForLegacyFeatures()) {
            SyncFringesToDots();
            SortDotsFY();
        }
    } else {
        // LEGACY: Build Dots (existing code)
        CreateNumLines();  // Existing dot-based implementation
        SyncDotsToFringes();  // Generate Fringes from Dots
    }
    
    if (!isInsideScreen) {
        SelectMainFringe();
        CorrectNumbers();
    }
    
    CreateZAPSections();
    SelectMainDot();
    Delete_buf_line();
    
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    m_bUseFringeModel = true;  // Ensure fringe model is active
}

/**
 * @brief Check if legacy features require Dots array
 */
bool CDigitInfo::NeedDotsForLegacyFeatures() const {
    // Check if ZAP section navigation is active
    // Check if any legacy UI is using Dots directly
    // For now, always generate for safety
    return true;
}
```

---

### Step 4: Test Implementation

**File:** `Tests/DigitModeTests/FringeConstructionTest.cpp` (NEW)

```cpp
#include "pch.h"
#include "DigitMode/DigitInfo.h"
#include "Controls/BoundCtrls.h"
#include "Controls/ImageCtrls.h"
#include <gtest/gtest.h>

class FringeConstructionTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    
    void SetUp() override {
        // Set up aperture
        CBoundCtrls* pB = GetBoundCtrls();
        pB->ExtBoundType = BOUND_RECT;
        // ... configure bounds ...
        
        // Create mock interferogram
        CreateMockInterferogram();
    }
    
    void CreateMockInterferogram() {
        // Create simple test pattern: 3 horizontal fringes
        digitInfo.Init_buf_line(100, 4);
        for (int i = 0; i < 100; i++) {
            digitInfo.buf_line[i][0] = 0;
            digitInfo.buf_line[i][1] = 300;
            digitInfo.buf_line[i][2] = -1;
            digitInfo.buf_line[i][3] = -1;
        }
        
        // Simulate red centers for 3 fringes
        digitInfo.Sections.SetSize(100);
        for (int y = 0; y < 100; y++) {
            digitInfo.Sections[y].L.P1.y = y;
            digitInfo.Sections[y].L.P2.y = y;
            
            // Three fringes at x = 100, 200, 300
            if (y % 10 < 8) {  // Allow some gaps
                AddRedCenter(y, 100, 1.0);
                AddRedCenter(y, 200, 2.0);
                AddRedCenter(y, 300, 3.0);
            }
        }
    }
    
    void AddRedCenter(int y, double x, double number) {
        CNumLine nl;
        nl.redX = x;
        nl.Number = number;
        digitInfo.Sections[y].NumLines.Add(nl);
    }
};

TEST_F(FringeConstructionTest, CreatesFringesFromRedCenters) {
    digitInfo.m_bUseFringeModel = true;
    digitInfo.SelectFringeStep();  // Calculate numStep
    digitInfo.CreateNumLinesFringeModel();
    
    // Should create 3 fringes
    EXPECT_EQ(3, digitInfo.Fringes.size());
    
    // Each fringe should have ~90 points (10 sections have gaps)
    for (size_t i = 0; i < digitInfo.Fringes.size(); i++) {
        EXPECT_GT(digitInfo.Fringes[i].GetPointCount(), 80);
        EXPECT_LT(digitInfo.Fringes[i].GetPointCount(), 100);
    }
}

TEST_F(FringeConstructionTest, AssignsCorrectNumbers) {
    digitInfo.m_bUseFringeModel = true;
    digitInfo.numStep = 1.0;
    digitInfo.CreateNumLinesFringeModel();
    
    // Check fringe numbers
    std::vector<double> numbers;
    for (auto& fringe : digitInfo.Fringes) {
        numbers.push_back(fringe.GetNumber());
    }
    std::sort(numbers.begin(), numbers.end());
    
    EXPECT_NEAR(1.0, numbers[0], 0.1);
    EXPECT_NEAR(2.0, numbers[1], 0.1);
    EXPECT_NEAR(3.0, numbers[2], 0.1);
}

TEST_F(FringeConstructionTest, MatchesDotModelResults) {
    // Run with dot model
    CDigitInfo dotModel;
    // ... set up same data ...
    dotModel.m_bUseFringeModel = false;
    dotModel.CreateNumLines();  // Existing implementation
    
    // Run with fringe model
    digitInfo.m_bUseFringeModel = true;
    digitInfo.CreateNumLinesFringeModel();
    
    // Convert fringe model to dots for comparison
    digitInfo.SyncFringesToDots();
    
    // Should have similar number of dots
    EXPECT_NEAR(dotModel.Dots.size(), digitInfo.Dots.size(), 
                dotModel.Dots.size() * 0.1);  // Within 10%
}
```

---

### Step 5: Validate & Benchmark

**Validation Checklist:**
- [ ] CreateNumLinesFringeModel() compiles
- [ ] Tests pass
- [ ] Fringes have reasonable point counts
- [ ] Fringe numbers match expected values
- [ ] Visual inspection: fringes are continuous

**Benchmark:**
```cpp
// Measure performance
auto start = std::chrono::high_resolution_clock::now();
digitInfo.CreateNumLinesFringeModel();
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
TRACE("CreateNumLinesFringeModel took %lld ms\n", duration.count());
```

---

## Expected Results

### Success Criteria
- ✅ Fringes.size() matches expected fringe count
- ✅ Each fringe is continuous (no large gaps)
- ✅ Fringe numbers are correct
- ✅ Performance ≤ 2x current CreateNumLines()

### Common Issues

**Issue 1: Too Many Fringes Created**
- **Cause:** Continuity threshold too strict
- **Fix:** Increase `continuityThreshold` or allow more section gaps

**Issue 2: Fringes Stop/Start Unexpectedly**
- **Cause:** Red center detection has gaps
- **Fix:** Increase gap tolerance (allow 2-3 section gaps)

**Issue 3: Wrong Fringe Numbers**
- **Cause:** Number assignment from red centers incorrect
- **Fix:** Verify SelectFringeStep() and NumberingLine() logic

---

## Next Steps After Sprint 1

1. **Sprint 2:** Implement ZAP I/O (ReconstructFringesFromZapDots)
2. **Sprint 3:** Integration testing with real interferograms
3. **Sprint 4:** UI updates for fringe navigation
4. **Sprint 5:** Deprecate dot model

---

## Getting Help

**Questions:**
- How do I test with real interferogram data?
- What if fringe numbers don't match?
- How do I debug fringe continuity issues?

**Resources:**
- `Docs/FRINGE_MODEL_TRANSITION_PLAN.md` - Full transition plan
- `Docs/NAVIGATE_MODE_IMPLEMENTATION.md` - Segment-primary model UX
- Existing `CreateNumLines()` - Reference implementation

**Contact:** Post questions in GitHub Issues with tag `fringe-model`

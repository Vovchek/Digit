# Transition Plan: Dot-Based to Segment-Primary Fringe Model

## Executive Summary

**Goal**: Complete migration from flat Dots array to segment-primary Fringes model while maintaining backward compatibility with existing ZAP files and automatic digitization workflow.

**Timeline**: Phased approach over 3-4 iterations
**Risk**: Medium-High (affects core data model and I/O)
**Testing**: Extensive validation required at each phase

---

## Current State Analysis

### Problem 1: ZAP File I/O Uses Dots Model

**Current Implementation:**
```cpp
// In LoadZAP / SaveZAP
for (each fringe number) {
    for (each ZAP section) {
        // Find dot at (section_x, fringe_number)
        Dots.push_back(CDotInfo(x, y, number, iZapSec));
    }
}
```

**Issues:**
- ✅ FRN format already exists (fringe-based)
- ❌ ZAP format is dot-oriented (legacy)
- ❌ No segment information in ZAP files
- ❌ Conversion from ZAP to Fringes is lossy

**Impact:** Medium - ZAP is legacy format, FRN is modern

---

### Problem 2: Auto() Digitization Builds Dots

**Current Workflow:**
```cpp
void Auto() {
    CreateBufLine();           // OK - geometry-based
    CreateRedCenters();        // OK - creates HidenDots (intermediate)
    SelectFringeStep();        // OK - analyzes Sections
    CreateNumLines();          // ❌ Builds Dots array
    CreateZAPSections();       // ❌ Associates Dots with sections
    SortDotsFY();             // ❌ Sorts Dots
    SyncFringesToDots();      // ❌ Backward conversion
}
```

**Issues:**
- ❌ CreateNumLines() builds flat Dots array
- ❌ Fringe continuity lost (each dot independent)
- ❌ Segment boundaries not tracked
- ❌ Conversion to Fringes is reconstruction, not native

**Impact:** High - core digitization logic

---

## Transition Strategy

### Phase 0: Preparation (Current State) ✅

**Status:** COMPLETE

- [x] CFringeSegment class implemented
- [x] Fringes vector added to CDigitInfo
- [x] m_bUseFringeModel flag added
- [x] Basic conversion utilities (SyncFringesToDots, SyncDotsToFringes)
- [x] Segment-primary API (CreateFringe, AddPointToFringe, etc.)

**Remaining Gaps:**
- [ ] Auto() still builds Dots primarily
- [ ] ZAP I/O not fringe-aware
- [ ] No native fringe construction in CreateNumLines()

---

### Phase 1: Fringe-Native Auto() Digitization

**Goal:** Make Auto() build Fringes directly, Dots become secondary

#### 1.1 Refactor CreateNumLines() to Build Fringes

**Current Logic (Simplified):**
```cpp
void CreateNumLines() {
    // For each section
    for (int iSec = 0; iSec < Sections.size(); iSec++) {
        // For each red center in section
        for (int iLine = 0; iLine < Sections[iSec].NumLines.size(); iLine++) {
            // Find nearest red center in next section
            double x = Sections[iSec].NumLines[iLine].redX;
            double y = Sections[iSec].L.P1.y;
            
            // Create dot
            Dots.push_back(CDotInfo(x, y, number, iZapSec));
        }
    }
}
```

**New Logic (Fringe-Primary):**
```cpp
void CreateNumLines() {
    // Track active fringes (fringe index -> last point info)
    std::map<double, ActiveFringe> activeFringes;
    
    // For each section (top to bottom)
    for (int iSec = 0; iSec < Sections.size(); iSec++) {
        std::vector<RedCenter> centersInSection = GetRedCenters(iSec);
        
        // For each red center in this section
        for (auto& center : centersInSection) {
            CDPoint pt(center.x, center.y);
            
            // Try to continue existing fringe
            bool continued = false;
            for (auto& [number, active] : activeFringes) {
                if (IsNearEnough(pt, active.lastPoint, tolerance)) {
                    // Continue existing fringe
                    AddPointToFringe(active.iFringe, pt);
                    active.lastPoint = pt;
                    continued = true;
                    break;
                }
            }
            
            if (!continued) {
                // Start new fringe
                double number = AssignFringeNumber(center, iSec);
                int iFringe = CreateFringe(number, -1);
                AddPointToFringe(iFringe, pt);
                activeFringes[number] = {iFringe, pt};
            }
        }
        
        // Terminate fringes that have no continuation
        RemoveInactiveFringes(activeFringes, centersInSection);
    }
    
    // LEGACY SUPPORT: Build Dots from Fringes
    if (m_bUseFringeModel) {
        SyncFringesToDots();  // Generate Dots for backward compatibility
    }
}
```

**Key Changes:**
- ✅ Fringes built directly (primary)
- ✅ Segment boundaries preserved
- ✅ Fringe continuity tracked
- ✅ Dots generated as byproduct (optional)

**Implementation Steps:**
1. Create ActiveFringe helper struct
2. Implement fringe continuation logic
3. Implement fringe termination detection
4. Test against existing Auto() results

**Validation:**
```cpp
// Test that new CreateNumLines produces same results
TEST(FringeModelTest, CreateNumLines_MatchesDotModel) {
    // Run old Dot-based CreateNumLines
    digitInfo.m_bUseFringeModel = false;
    digitInfo.Auto();
    auto oldDots = digitInfo.Dots;
    
    // Run new Fringe-based CreateNumLines
    digitInfo.Clear();
    digitInfo.m_bUseFringeModel = true;
    digitInfo.Auto();
    auto newFringes = digitInfo.Fringes;
    auto generatedDots = ConvertFringesToDots(newFringes);
    
    // Compare
    EXPECT_EQ(oldDots.size(), generatedDots.size());
    EXPECT_FRINGES_MATCH(oldDots, generatedDots, tolerance);
}
```

---

#### 1.2 Update CreateZAPSections() for Fringes

**Current:**
```cpp
void CreateZAPSections() {
    // Associate Dots with ZAP sections
    for (auto& dot : Dots) {
        int iZapSec = FindNearestZapSection(dot.P.x);
        dot.iZapSec = iZapSec;
    }
}
```

**New (Fringe-Aware):**
```cpp
void CreateZAPSections() {
    // Create ZAP sections at regular intervals
    int numSections = CalculateOptimalSectionCount();
    double intervalY = apertureHeight / numSections;
    
    for (int i = 0; i < numSections; i++) {
        double y = apertureTop + i * intervalY;
        CZapLineInfo zapLine;
        zapLine.SetY(y);
        ZapLines.Add(zapLine);
    }
    
    // OPTIONAL: Associate fringe points with sections
    // (Only needed if ZAP section navigation is required)
    if (NeedZapSectionNavigation()) {
        for (size_t iFr = 0; iFr < Fringes.size(); iFr++) {
            for (int iPt = 0; iPt < Fringes[iFr].GetPointCount(); iPt++) {
                CDPoint pt = Fringes[iFr].GetPoint(iPt);
                int iZapSec = FindNearestZapSection(pt.y);
                // Store association if needed
            }
        }
    }
}
```

**Key Changes:**
- ✅ ZAP sections are geometry references (horizontal lines)
- ✅ Fringes don't need ZAP section indices (optional association)
- ✅ Navigation can use spatial queries instead of indices

---

#### 1.3 Eliminate SortDotsFY() Dependency

**Current:**
```cpp
void Auto() {
    // ...
    SortDotsFY();  // Sort Dots by fringe, then Y
}
```

**New:**
```cpp
void Auto() {
    // ...
    // Fringes are already ordered by construction
    // Optional: Sort Fringes by number
    std::sort(Fringes.begin(), Fringes.end(), 
        [](const CFringeSegment& a, const CFringeSegment& b) {
            return a.GetNumber() < b.GetNumber();
        });
    
    // LEGACY: Generate sorted Dots if needed
    if (!m_bUseFringeModel) {
        SyncFringesToDots();
        SortDotsFY();
    }
}
```

---

### Phase 2: Fringe-Aware File I/O

#### 2.1 Enhance FRN Format (Already Fringe-Based)

**Current FRN Format:**
```
FRINGE_COUNT: N
FRINGE 0
  NUMBER: 1.0
  SEGMENT: 0
  POINT_COUNT: M
  POINT: x1 y1
  POINT: x2 y2
  ...
FRINGE 1
  ...
```

**Status:** ✅ Already supports segment-primary model

**Action Required:** NONE (use as primary save format)

---

#### 2.2 Implement ZAP-to-Fringe Conversion (Load)

**Challenge:** ZAP format stores dots at ZAP section intersections, not polylines

**Strategy:** Reconstruct fringes from dot grid

**Implementation:**
```cpp
BOOL CDigitInfo::LoadZAP(LPCTSTR fname) {
    // Load ZAP data into temporary dot structure
    std::vector<ZapDot> zapDots;
    ReadZAPFile(fname, zapDots);
    
    if (m_bUseFringeModel) {
        // Convert ZAP dots to Fringes
        Fringes = ReconstructFringesFromZapDots(zapDots);
        
        // Optional: Generate Dots for legacy compatibility
        SyncFringesToDots();
    } else {
        // Legacy: Build Dots directly
        ConvertZapDotsToDotsArray(zapDots);
    }
    
    return TRUE;
}

std::vector<CFringeSegment> ReconstructFringesFromZapDots(
    const std::vector<ZapDot>& zapDots) 
{
    // Group dots by fringe number
    std::map<double, std::vector<CDPoint>> fringePoints;
    for (auto& zd : zapDots) {
        fringePoints[zd.number].push_back(zd.point);
    }
    
    // Create fringes
    std::vector<CFringeSegment> fringes;
    for (auto& [number, points] : fringePoints) {
        // Sort points by Y coordinate (ZAP sections are horizontal)
        std::sort(points.begin(), points.end(), 
            [](const CDPoint& a, const CDPoint& b) {
                return a.y < b.y;
            });
        
        // Check for fringe splits (gaps > threshold)
        std::vector<std::vector<CDPoint>> segments = 
            SplitIntoSegments(points, gapThreshold);
        
        // Create fringe segment for each continuous run
        for (auto& segPoints : segments) {
            CFringeSegment fringe;
            fringe.SetNumber(number);
            for (auto& pt : segPoints) {
                fringe.AddPoint(pt);
            }
            fringes.push_back(fringe);
        }
    }
    
    return fringes;
}

std::vector<std::vector<CDPoint>> SplitIntoSegments(
    const std::vector<CDPoint>& points, double maxGap)
{
    std::vector<std::vector<CDPoint>> segments;
    std::vector<CDPoint> currentSegment;
    
    for (size_t i = 0; i < points.size(); i++) {
        if (currentSegment.empty()) {
            currentSegment.push_back(points[i]);
        } else {
            double gap = Distance(points[i], currentSegment.back());
            if (gap <= maxGap) {
                currentSegment.push_back(points[i]);
            } else {
                // Gap too large - start new segment
                segments.push_back(currentSegment);
                currentSegment.clear();
                currentSegment.push_back(points[i]);
            }
        }
    }
    
    if (!currentSegment.empty()) {
        segments.push_back(currentSegment);
    }
    
    return segments;
}
```

**Key Features:**
- ✅ Reconstructs fringes from ZAP grid
- ✅ Detects fringe splits (multiple segments per number)
- ✅ Preserves fringe numbers
- ❌ Loses exact segment boundaries (reconstruction is approximate)

**Limitation:** ZAP format doesn't store segment boundaries, so reconstruction may differ from original Auto() detection

---

#### 2.3 Implement Fringe-to-ZAP Conversion (Save)

**Strategy:** Sample fringes at ZAP section positions

**Implementation:**
```cpp
BOOL CDigitInfo::SaveZAP(LPCTSTR fname, int extIdx) {
    if (m_bUseFringeModel) {
        // Generate Dots by sampling Fringes at ZAP sections
        GenerateDotsFromFringesAtZapSections();
    }
    
    // Collect dot data (now works for both models)
    NUMBERING_INTERFEROGRAM_INFO IntInfo;
    if (!CollectNumberingInterferogramInfo(IntInfo))
        return FALSE;
    
    // Write ZAP file
    if (extIdx == 2)
        WriteWinZAPData(fname, IntInfo);
    else if (extIdx == 3)
        WriteDosZAPData(fname, IntInfo);
    
    return true;
}

void CDigitInfo::GenerateDotsFromFringesAtZapSections() {
    Dots.clear();
    
    // For each ZAP section
    for (int iZapSec = 0; iZapSec < ZapLines.GetSize(); iZapSec++) {
        double zapY = ZapLines[iZapSec].GetY();
        
        // For each fringe
        for (size_t iFr = 0; iFr < Fringes.size(); iFr++) {
            // Find intersection with ZAP section
            CDPoint intersection;
            if (FindFringeIntersectionWithY(Fringes[iFr], zapY, intersection)) {
                // Create dot at intersection
                CDotInfo dot;
                dot.P = intersection;
                dot.Number = Fringes[iFr].GetNumber();
                dot.iZapSec = iZapSec;
                Dots.push_back(dot);
            }
        }
    }
}

bool CDigitInfo::FindFringeIntersectionWithY(
    const CFringeSegment& fringe, double y, CDPoint& outPt)
{
    int pointCount = fringe.GetPointCount();
    for (int i = 0; i < pointCount - 1; i++) {
        CDPoint p1 = fringe.GetPoint(i);
        CDPoint p2 = fringe.GetPoint(i + 1);
        
        // Check if Y is between p1.y and p2.y
        if ((p1.y <= y && y <= p2.y) || (p2.y <= y && y <= p1.y)) {
            // Linear interpolation
            double t = (y - p1.y) / (p2.y - p1.y);
            outPt.x = p1.x + t * (p2.x - p1.x);
            outPt.y = y;
            return true;
        }
    }
    return false;  // Fringe doesn't intersect this Y
}
```

**Key Features:**
- ✅ Generates ZAP-compatible dot grid from fringes
- ✅ Preserves fringe numbers
- ✅ Backward compatible with ZAP readers
- ❌ Loses segment information (ZAP format limitation)

**Trade-off:** ZAP export from Fringes is lossy (segment boundaries lost)

---

### Phase 3: Migration Path & Compatibility

#### 3.1 File Format Decision Tree

```
Load File:
  Is extension .FRN?
    YES → LoadFRN() → Fringes (native)
    NO  → Is extension .ZAP?
      YES → LoadZAP() → 
        m_bUseFringeModel?
          YES → ReconstructFringesFromZapDots() → Fringes
          NO  → ConvertZapDotsToDotsArray() → Dots
          
Save File:
  User preference or format detection:
    FRN (recommended) → SaveFRN() → Direct fringe export
    ZAP (legacy)      → SaveZAP() → Sample fringes at ZAP sections
```

**Recommendation:**
- ✅ **Default to FRN** for new saves
- ✅ **Support ZAP load** for backward compatibility
- ✅ **Support ZAP save** for legacy tool interop
- ⚠️ **Warn user** when saving FRN→ZAP (lossy conversion)

---

#### 3.2 Auto() Dual-Mode Support (Transition Period)

```cpp
void CDigitInfo::Auto() {
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    Clear(FALSE);
    
    // Common steps (geometry-based)
    CreateBufLine();           // ✅ Model-independent
    CreateRedCenters();        // ✅ Model-independent (HidenDots)
    SelectFringeStep();        // ✅ Model-independent (Sections)
    SelectMainSection();       // ✅ Model-independent
    
    if (m_bUseFringeModel) {
        // NEW: Fringe-native workflow
        CreateNumLinesFringeModel();  // Build Fringes directly
        SelectMainFringe();           // Find main fringe
        CorrectNumbers();             // Fix numbering
        CreateZAPSections();          // Create reference lines
        
        // Generate Dots for legacy UI/navigation
        SyncFringesToDots();
        SortDotsFY();  // Keep legacy navigation working
    } else {
        // LEGACY: Dot-based workflow
        CreateNumLinesDotModel();     // Build Dots (old code)
        SelectMainFringe();
        CorrectNumbers();
        CreateZAPSections();
        SortDotsFY();
        
        // Generate Fringes for new features
        SyncDotsToFringes();
    }
    
    SelectMainDot();
    Delete_buf_line();
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
```

**Benefits:**
- ✅ Both models supported during transition
- ✅ Flag-based switching for testing
- ✅ Gradual migration (can revert if issues)
- ✅ Legacy features keep working

---

#### 3.3 Testing Strategy

**Unit Tests:**
```cpp
// Test fringe reconstruction from ZAP
TEST(FringeIOTest, LoadZAP_ReconstructsFringes) {
    CDigitInfo digitInfo;
    digitInfo.m_bUseFringeModel = true;
    ASSERT_TRUE(digitInfo.LoadZAP("test_data/sample.zap"));
    
    EXPECT_GT(digitInfo.Fringes.size(), 0);
    EXPECT_EQ(digitInfo.Fringes[0].GetPointCount(), expectedCount);
}

// Test fringe export to ZAP
TEST(FringeIOTest, SaveZAP_PreservesIntersections) {
    CDigitInfo digitInfo;
    digitInfo.m_bUseFringeModel = true;
    // ... create test fringes ...
    
    ASSERT_TRUE(digitInfo.SaveZAP("output.zap", 2));
    
    // Reload and verify
    CDigitInfo reloaded;
    reloaded.m_bUseFringeModel = false;  // Load as dots
    reloaded.LoadZAP("output.zap");
    
    // Check that dots match ZAP section intersections
    EXPECT_DOTS_AT_ZAP_SECTIONS(reloaded.Dots, digitInfo.ZapLines);
}

// Test Auto() equivalence
TEST(FringeModelTest, Auto_FringeModel_MatchesDotModel) {
    CImageDoc doc1, doc2;
    LoadTestInterferogram(&doc1);
    LoadTestInterferogram(&doc2);
    
    // Run with dot model
    doc1.Digit.m_bUseFringeModel = false;
    doc1.Digit.Auto();
    
    // Run with fringe model
    doc2.Digit.m_bUseFringeModel = true;
    doc2.Digit.Auto();
    
    // Compare results
    EXPECT_FRINGE_GEOMETRY_MATCHES(doc1.Digit, doc2.Digit, tolerance);
    EXPECT_FRINGE_NUMBERS_MATCH(doc1.Digit, doc2.Digit);
}
```

**Integration Tests:**
```cpp
// Test full round-trip
TEST(IntegrationTest, RoundTrip_FRN) {
    CDigitInfo original;
    // ... create fringes ...
    original.SaveFRN("test.frn");
    
    CDigitInfo loaded;
    loaded.LoadFRN("test.frn");
    
    EXPECT_FRINGES_EQUAL(original.Fringes, loaded.Fringes);
}

// Test ZAP backward compatibility
TEST(IntegrationTest, RoundTrip_ZAP_BackwardCompatible) {
    CDigitInfo original;
    original.m_bUseFringeModel = true;
    // ... create fringes ...
    original.SaveZAP("test.zap", 2);
    
    // Load with legacy tool (dot model)
    CDigitInfo legacy;
    legacy.m_bUseFringeModel = false;
    legacy.LoadZAP("test.zap");
    
    // Verify dots are reasonable
    EXPECT_GT(legacy.Dots.size(), 0);
    EXPECT_DOTS_HAVE_VALID_NUMBERS(legacy.Dots);
}
```

---

### Phase 4: UI/Navigation Updates

#### 4.1 Keyboard Navigation (Already Partially Done)

**Current (Dot Model):**
```cpp
void OnKeyDown(UINT nChar, ...) {
    if (idxMainDot != -1) {
        // Navigate in Dots array using ZAP sections
        if (nChar == VK_LEFT) {
            GetNextDotInSection(Dots[idxMainDot].iZapSec, -1, idx, dP);
        }
    }
}
```

**New (Fringe Model):**
```cpp
void OnKeyDown(UINT nChar, ...) {
    if (m_bUseFringeModel) {
        if (idxMainPoint.IsValid()) {
            if (nChar == VK_LEFT) {
                // Move along fringe
                idxMainPoint.iPoint--;
            } else if (nChar == VK_RIGHT) {
                idxMainPoint.iPoint++;
            } else if (nChar == VK_UP) {
                // Jump to nearest point in previous fringe
                JumpToAdjacentFringe(-1);
            }
        }
    } else {
        // Legacy navigation
        // ...
    }
}
```

**Status:** ✅ Already implemented (see DigitInfo.cpp OnKeyDown)

---

#### 4.2 ZAP Section Navigation (Optional)

**Question:** Do we still need ZAP section navigation in fringe model?

**Options:**

**Option A: Remove ZAP Section Navigation**
- ✅ Simpler model (fringes are self-contained)
- ✅ No ZAP section indices needed
- ❌ Breaks existing UI patterns

**Option B: Virtual ZAP Sections**
- ✅ Preserve UI patterns
- ✅ No storage in fringe objects
- ✅ Calculate intersections on-demand

```cpp
// Get fringe point nearest to ZAP section Y
bool GetFringePointAtZapSection(int iFringe, int iZapSec, CDPoint& outPt) {
    double zapY = ZapLines[iZapSec].GetY();
    return FindFringeIntersectionWithY(Fringes[iFringe], zapY, outPt);
}

// Navigate: Left/Right = move along ZAP section, Up/Down = change section
void NavigateAlongZapSection(int direction) {
    int currentZapSec = FindNearestZapSection(idxMainPoint);
    int nextZapSec = currentZapSec + direction;
    
    if (nextZapSec >= 0 && nextZapSec < ZapLines.GetSize()) {
        // Find intersection of current fringe with new section
        CDPoint newPt;
        if (GetFringePointAtZapSection(idxMainPoint.iFringe, nextZapSec, newPt)) {
            // Find nearest actual point to intersection
            idxMainPoint.iPoint = FindNearestPointIndex(
                Fringes[idxMainPoint.iFringe], newPt);
        }
    }
}
```

**Recommendation:** Option B (virtual ZAP sections) - preserves UI, no storage overhead

---

### Phase 5: Deprecation & Cleanup

#### 5.1 Mark Dot Model as Deprecated

```cpp
class CDigitInfo {
public:
    /**
     * @deprecated Use Fringes (segment-primary model) instead
     * This field will be removed in version 3.0
     */
    [[deprecated("Use Fringes instead")]]
    std::vector<CDotInfo> Dots;
    
    /**
     * @deprecated Use idxMainPoint instead
     */
    [[deprecated("Use idxMainPoint for fringe model")]]
    int idxMainDot;
};
```

---

#### 5.2 Migration Checklist

**Before Removing Dot Model:**
- [ ] All Auto() workflows use CreateNumLinesFringeModel()
- [ ] All file I/O tested with fringe model
- [ ] All UI navigation works with idxMainPoint
- [ ] All commands work with Fringes
- [ ] Performance benchmarks show no regression
- [ ] User documentation updated
- [ ] Migration guide published

**Safe to Remove:**
- [ ] `Dots` vector
- [ ] `idxMainDot`, `idxDragDot`
- [ ] `SortDotsFY()`
- [ ] `CreateNumLinesDotModel()` (old implementation)
- [ ] Dot-based ZAP conversion code

---

## Implementation Timeline

### Sprint 1 (Week 1-2): Core Fringe Construction
- [ ] Implement `CreateNumLinesFringeModel()`
- [ ] Test fringe continuity detection
- [ ] Validate against current Auto() results
- [ ] Commit: "feat: implement fringe-native Auto() digitization"

### Sprint 2 (Week 3-4): File I/O
- [ ] Implement `ReconstructFringesFromZapDots()`
- [ ] Implement `GenerateDotsFromFringesAtZapSections()`
- [ ] Test ZAP round-trip (save → load → compare)
- [ ] Commit: "feat: add fringe-aware ZAP I/O"

### Sprint 3 (Week 5-6): Integration & Testing
- [ ] Update Auto() to use dual-mode
- [ ] Add comprehensive unit tests
- [ ] Add integration tests
- [ ] Performance testing
- [ ] Commit: "test: comprehensive fringe model validation"

### Sprint 4 (Week 7-8): UI & Documentation
- [ ] Update keyboard navigation
- [ ] Implement virtual ZAP sections
- [ ] Update user documentation
- [ ] Create migration guide
- [ ] Commit: "docs: fringe model migration guide"

### Sprint 5 (Week 9-10): Deprecation & Cleanup
- [ ] Mark Dot model as deprecated
- [ ] Add deprecation warnings
- [ ] Create removal plan
- [ ] Final testing
- [ ] Commit: "refactor: deprecate dot model"

---

## Risk Mitigation

### Risk 1: Auto() Produces Different Results

**Mitigation:**
- Create regression test suite (100+ test interferograms)
- Run both models side-by-side
- Quantify differences (should be < 1 pixel)
- Tune fringe continuation thresholds

**Acceptance Criteria:**
- 95% of fringes match within 0.5 pixels
- 100% of fringe numbers match
- No fringe splits/merges where not expected

---

### Risk 2: ZAP Load/Save Lossy

**Mitigation:**
- Document ZAP format limitations
- Add "Lossless save (FRN recommended)" warning
- Implement quality metrics (completeness score)
- Provide conversion tool (ZAP → FRN)

**Acceptance Criteria:**
- ZAP round-trip preserves ≥90% of fringe geometry
- All fringe numbers preserved
- Users notified of data loss

---

### Risk 3: Performance Regression

**Mitigation:**
- Benchmark Auto() with both models
- Profile fringe construction
- Optimize hot paths (intersection detection, continuity checks)
- Cache spatial queries

**Acceptance Criteria:**
- Auto() ≤ 10% slower with fringe model
- Memory usage ≤ 20% increase (fringes store more metadata)
- UI remains responsive (<100ms for navigation)

---

## Success Criteria

### Functional Requirements
- [x] ✅ Fringes can be created from Auto() digitization
- [ ] ✅ Fringes can be saved to FRN format
- [ ] ✅ Fringes can be loaded from FRN format
- [ ] ✅ Fringes can be loaded from ZAP format (reconstructed)
- [ ] ✅ Fringes can be saved to ZAP format (sampled)
- [ ] ✅ Keyboard navigation works with fringes
- [ ] ✅ Manual editing works with fringes
- [ ] ✅ Selection (Navigate mode) works with fringes

### Quality Requirements
- [ ] ✅ Unit test coverage ≥ 80%
- [ ] ✅ Integration tests for all file formats
- [ ] ✅ Regression tests pass
- [ ] ✅ Performance within acceptable limits
- [ ] ✅ Documentation complete

### Migration Requirements
- [ ] ✅ Backward compatibility with ZAP files
- [ ] ✅ Dual-mode support during transition
- [ ] ✅ Clear migration path for users
- [ ] ✅ Deprecation warnings in place

---

## Next Steps

### Immediate (This Week)
1. **Review this plan** with team/stakeholders
2. **Set m_bUseFringeModel = true** as default for testing
3. **Start Sprint 1**: Implement CreateNumLinesFringeModel()

### Short-term (Next Month)
4. Complete Sprints 1-2 (core + file I/O)
5. Run comprehensive tests
6. Get user feedback on fringe model

### Long-term (3 Months)
7. Complete all sprints
8. Deprecate dot model
9. Plan removal for version 3.0

---

## Conclusion

**The transition is feasible** with careful phased approach:
- ✅ **Phase 1** makes Auto() build Fringes natively
- ✅ **Phase 2** ensures file I/O works (FRN primary, ZAP compatible)
- ✅ **Phase 3** provides dual-mode support for safety
- ✅ **Phase 4** updates UI to work with fringes
- ✅ **Phase 5** deprecates old model

**Recommendation:** Proceed with Sprint 1 implementation and validate results before committing to full migration.

**Key Principle:** Maintain backward compatibility throughout transition; only remove Dot model when Fringe model is proven stable in production.

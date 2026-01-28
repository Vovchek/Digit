# CDigitInfo Extraction - Executive Summary

## What is CDigitInfo?

`CDigitInfo` is a **data + algorithm class** that manages fringe numbering in interferogram images. It:
- Detects fringe centers in scan lines
- Numbers fringes (assigns sequential numbers to interference fringes)
- Manages user-created sections and reference points
- Provides queries for accessing fringe data

**Core Value:** The fringe numbering algorithm is sophisticated and reusable.

---

## Extraction Difficulty: MEDIUM ??

**Good News:**
- Core algorithm is self-contained
- Composed types (CSectionInfo, CDotInfo, etc.) are clean
- 95% of methods are algorithm/query based

**Bad News:**
- 5 **critical dependencies** on global singleton objects
- UI rendering mixed into data class
- Image/control access via global functions

**Effort Estimate:** 3-4 days for complete extraction with refactoring

---

## The 5 Critical Dependencies ("Seams")

### 1. **Global Control Access** (PRIORITY: P0 - CRITICAL)
**Problem:** Methods call `GetBoundCtrls()`, `GetImageCtrls()`, `GetControls()` global functions

**Affected:** 8 methods
- CreateBufLineAperture*() 
- CreateBufLineObstruction*()
- CreateRedCenters()
- CreateZAPSections()
- CreateZAPSectionsOnLoadZAPFile()

**Solution:** Dependency Injection - Pass control objects via `SetDependencies()`

**Code Change Impact:** ~50 lines of changes
```cpp
// BEFORE
CBoundCtrls* pB = GetBoundCtrls();

// AFTER  
CBoundCtrls* pB = m_pBounds;  // Injected in constructor
```

---

### 2. **UI Rendering** (PRIORITY: P1 - HIGH)
**Problem:** `Draw()` method mixes UI (CDC) with data

**Affected:** 4 Draw methods
- CDigitInfo::Draw()
- CSectionInfo::Draw()
- CDotInfo::Draw()
- CZapLineInfo::Draw()

**Solution:** Create separate `CDigitInfoRenderer` class

**Code Change Impact:** Extract ~200 lines to new class

```cpp
// BEFORE: pDigitInfo->Draw(pDC, DotSide);
// AFTER:  renderer.Draw(pDC, *pDigitInfo, DotSide);
```

---

### 3. **Keyboard Input** (PRIORITY: P2 - MEDIUM)
**Problem:** `OnKeyDown()` handles keyboard events (MFC-specific)

**Affected:** 1 method
- OnKeyDown()

**Solution:** Move to view class or use callback interface

**Code Change Impact:** ~40 lines moved, easy refactor

---

### 4. **Cursor State** (PRIORITY: P2 - MEDIUM)
**Problem:** `Auto()` calls `::SetCursor()` - Windows API

**Affected:** 2 methods
- Auto()
- CreateRedCenters()

**Solution:** Use progress handler interface

**Code Change Impact:** ~10 lines changed

```cpp
// BEFORE: ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
// AFTER:  if (m_pProgress) m_pProgress->OnProcessingStarted();
```

---

### 5. **File I/O** (PRIORITY: P3 - LOW)
**Problem:** Load/Save methods are stubs, need serializer interface

**Affected:** 6 methods + 2 static helpers
- Load(), Save()
- LoadZAP(), SaveZAP()
- LoadFRN(), SaveFRN()
- ResolveImagePath() [static]
- CreateFakeGrayImage() [static]

**Solution:** Define `IDigitInfoSerializer` interface

**Code Change Impact:** ~30 lines in CDigitInfo (stub methods become delegates)

---

## What Can Be Extracted Immediately (No Changes)

? **Safe to extract as-is:**

### 1. Composed Classes
- `CSectionInfo` - Line numbering information
- `CDotInfo` - Reference fringe points
- `CZapLineInfo` - Fringe section markers
- `CNumLine` - Numbered fringe definition

### 2. Support Libraries
- `Utils\middle.h` - Pure image processing functions
  - `fon_del()`, `invert_line()`, `SortDouble()`, etc.
- `InterfSolver\Tools\isPupil.h/cpp` - Geometry testing
- `MGTools\Include\Utils\BaseDataType.h` - Data structures

### 3. Data Access Methods
- `GetFringeDots()`
- `GetFirstDotInFringe()`, `GetNextDotInFringe()`
- `GetFirstDotInSection()`, `GetNextDotInSection()`
- `GetDotNumbers()`, `GetNearestZapSection()`
- All query methods (IsSections, IsDots, etc.)

---

## Extraction Roadmap

### Stage 1: Core Extract (1 day)
```
1. Copy DigitInfo.h/cpp with composed classes
2. Copy support files (middle.h, isPupil.h/cpp)
3. Copy BaseDataType.h (MGTools foundation)
4. Add dependency injection interface
5. Compile & verify no breaking changes to algorithm
```

### Stage 2: Decouple Control Access (1 day)
```
1. Add m_pBounds, m_pImage, m_pControls member variables
2. Add SetDependencies() method
3. Replace all GetBoundCtrls() calls with m_pBounds
4. Replace all GetImageCtrls() calls with m_pImage
5. Replace all GetControls() calls with m_pControls
6. Test algorithm still works with injected deps
```

### Stage 3: Extract Rendering (1 day)
```
1. Create CDigitInfoRenderer class
2. Move Draw() implementations to renderer
3. Remove Draw() from CDigitInfo/CSectionInfo/CDotInfo
4. Update calling code to use renderer
5. Test rendering with mocked/real CDC objects
```

### Stage 4: Interface Refactoring (0.5 days)
```
1. Create IDigitInfoProgress interface
2. Create IDigitInfoSerializer interface
3. Implement in consuming project
4. Test integration
```

---

## Dependency Diagram

```
CDigitInfo
?? Core Algorithm ? EXTRACT AS-IS
?  ?? CreateBufLine*()
?  ?? CreateRedCenters() [DEPENDS ON CONTROLS]
?  ?? CreateNumLines()
?  ?? SelectMainFringe()
?  ?? CorrectNumbers()
?
?? Data Storage ? EXTRACT AS-IS
?  ?? CSectionInfo array
?  ?? CDotInfo array
?  ?? CZapLineInfo array
?  ?? buf_line (2D buffer)
?
?? Query Methods ? EXTRACT AS-IS
?  ?? GetFringeDots()
?  ?? IsSections()
?  ?? IsLockedDot()
?
?? UI Methods ? REFACTOR NEEDED
?  ?? Draw() ? MOVE TO CDigitInfoRenderer
?  ?? OnKeyDown() ? MOVE TO VIEW
?
?? Global Dependencies ? INJECT NEEDED
   ?? GetBoundCtrls() ? m_pBounds
   ?? GetImageCtrls() ? m_pImage
   ?? GetControls() ? m_pControls
```

---

## Files to Copy/Create in New Project

**From Original Project:**
```
? DigitMode/DigitInfo.h (.cpp with modifications)
? DigitMode/SectionInfo.h (.cpp with modifications)
? DigitMode/DotInfo.h (.cpp with modifications)
? DigitMode/ZapLineInfo.h (.cpp with modifications)
? DigitMode/CreateNumLines.cxx (included file)
? DigitMode/SelectNumber.cxx (included file)
? Utils/middle.h (.cpp)
? InterfSolver/Tools/isPupil.h (.cpp)
? AppDef.h (constants only)
? MGTools/Include/Utils/BaseDataType.h
```

**Create New in Target Project:**
```
? DigitMode/IDigitInfoProgress.h (interface stub)
? DigitMode/IDigitInfoSerializer.h (interface stub)
? DigitMode/DigitInfoRenderer.h (.cpp) - NEW renderer class
? Controls/BoundCtrls.h (stub or minimal, for type definition)
? Controls/ImageCtrls.h (stub or minimal, for type definition)
? Controls/controls.h (stub or minimal, for type definition)
```

**Stub Files (Minimal Implementations):**
```cpp
// Controls/ImageCtrls.h - STUB ONLY
class CImageCtrls
{
public:
    CSize ImageSize;
    SECDib* m_pDIB;
    // ... only the members you need ...
};

// Controls/BoundCtrls.h - STUB ONLY  
class CBoundCtrls
{
public:
    int ExtBoundType;
    int InsBoundType;
    CArrayXYEllipse ArrEll;
    // ... only the members you need ...
};

// Controls/controls.h - STUB ONLY
class CControls
{
public:
    int FringeCenterAs;
    UINT ViewState;
    // ... only the members you need ...
};
```

---

## Lines of Code Impacted

| Operation | Lines | Type | Risk |
|-----------|-------|------|------|
| Copy as-is | ~800 | Copy | Low |
| Dependency Injection | ~50 | Modify | Low |
| Extract Renderer | ~200 | Extract | Medium |
| Interface Stubs | ~100 | Create | Low |
| Stub Controls | ~150 | Create | Medium |
| **TOTAL** | ~1,300 | Mixed | **Medium** |

---

## Testing Strategy

1. **Unit Tests:** Leverage existing `Tests/DigitModeTests/DigitInfoTest.cpp`
   - Tests fringe numbering algorithm
   - Can run without UI or global controls
   
2. **Integration Tests:** Create in new project
   - Inject mock control objects
   - Verify dependencies are used correctly
   
3. **Rendering Tests:** Visual verification
   - Use CDigitInfoRenderer with test CDC object
   - Verify no crashes in Draw operations

4. **Regression Tests:** Compare results
   - Load sample interferogram data
   - Compare fringe numbering output before/after extraction

---

## Risk Assessment

### HIGH RISK ??
- **Missing dependency injection parameter:** Methods will crash if dependencies not set
  - *Mitigation:* Add NULL checks, assertion helpers

- **Breaking changes to composed classes:** If CSectionInfo changes, ripple effects
  - *Mitigation:* Keep composed classes unchanged during extraction

### MEDIUM RISK ??
- **Rendering bugs:** CDC calls in renderer may not work as expected
  - *Mitigation:* Create comprehensive test images

- **Image processing differences:** Different buffer initialization behavior
  - *Mitigation:* Use test cases with known inputs/outputs

### LOW RISK ?
- **Algorithm changes:** Core numbering logic is unchanged
- **Data structure changes:** Composed types copied as-is
- **Query methods:** No dependencies, safe extraction

---

## Success Criteria

1. ? All 8 dependency-injected methods work with injected controls
2. ? CDigitInfoRenderer produces identical visual output
3. ? Existing unit tests pass (DigitInfoTest.cpp)
4. ? No calls to global GetBoundCtrls/GetImageCtrls/GetControls in extracted code
5. ? No #include of "Utils/mutils.h" (global functions) in extracted code
6. ? Compiles cleanly in new project with zero external dependencies beyond MFC
7. ? Can instantiate CDigitInfo independently without application startup

---

## Questions for Implementation

Before starting, clarify with your team:

1. **MFC Dependency:** Is MFC mandatory in target project, or can we abstract it?
   - *Answer determines:* Whether we keep CDC rendering or create abstract interface

2. **File Format:** What format do Load/Save methods expect?
   - *Answer determines:* IDigitInfoSerializer implementation complexity

3. **Performance Requirements:** Any real-time constraints?
   - *Answer determines:* Whether we optimize buffer allocation

4. **Backward Compatibility:** Must old DigitInfo still work?
   - *Answer determines:* Whether we modify or create new extracted version

5. **Image Processing Library:** Is MGTools required or optional?
   - *Answer determines:* Whether we add abstraction for SECDib/image handling

---

## Next Steps

**Immediate (This Week):**
1. ? Review this analysis with your team
2. ? Clarify the 5 questions above
3. ? Set up new project structure
4. ? Begin Stage 1 (Core Extract)

**Short-term (Next Week):**
1. Complete Stage 2 (Dependency Injection)
2. Complete Stage 3 (Renderer Extraction)
3. Run existing unit tests
4. Create integration tests

**Medium-term (Next 2 Weeks):**
1. Complete Stage 4 (Interface Refactoring)
2. Full integration testing
3. Documentation and examples
4. Knowledge transfer

---

## References in This Analysis

?? **DIGITINFO_DEPENDENCY_ANALYSIS.md** - Detailed dependency breakdown
- Direct includes analysis
- Indirect dependencies through composed types
- External library dependencies
- Architecture diagram
- Effort estimate breakdown

?? **DIGITINFO_DEPENDENCY_GRAPH.txt** - Visual dependency map
- Tree structure of all dependencies
- Composition hierarchy
- Tight coupling points identified
- Class hierarchy with method organization

?? **DIGITINFO_REFACTORING_GUIDE.md** - Code examples
- Concrete refactoring patterns
- Before/after code samples for each seam
- Integration patterns for consuming project
- Checklist of methods affected

---

**Status:** Ready to extract. Recommend starting with Stage 1 (Core Extract) this week.

**Contact:** For clarifications on any dependency or refactoring approach, refer to the detailed documents above.

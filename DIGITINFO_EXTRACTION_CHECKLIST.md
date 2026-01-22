# CDigitInfo Extraction Checklist

## Pre-Extraction Phase

### Understanding & Planning
- [ ] Read DIGITINFO_EXTRACTION_SUMMARY.md (executive overview)
- [ ] Read DIGITINFO_DEPENDENCY_ANALYSIS.md (detailed analysis)
- [ ] Review DIGITINFO_DEPENDENCY_GRAPH.txt (visual structure)
- [ ] Review DIGITINFO_REFACTORING_GUIDE.md (code examples)
- [ ] Clarify answers to 5 key questions (see summary document)
- [ ] Schedule 3-4 days for full extraction + testing
- [ ] Get team approval for extraction approach

### Environment Setup
- [ ] Create new Visual Studio project/solution
- [ ] Set up folder structure: DigitMode/, Controls/, Utils/, etc.
- [ ] Configure includes paths for MFC dependencies
- [ ] Create mock/stub directory for control classes
- [ ] Set up Git branch for extraction work
- [ ] Ensure backup of original source code

---

## Stage 1: Core Extract (?1 day)

### Step 1.1: Copy Core Files
- [ ] Copy `DigitMode/DigitInfo.h`
- [ ] Copy `DigitMode/DigitInfo.cpp`
- [ ] Copy `DigitMode/SectionInfo.h`
- [ ] Copy `DigitMode/SectionInfo.cpp`
- [ ] Copy `DigitMode/DotInfo.h`
- [ ] Copy `DigitMode/DotInfo.cpp`
- [ ] Copy `DigitMode/ZapLineInfo.h`
- [ ] Copy `DigitMode/ZapLineInfo.cpp`
- [ ] Copy `DigitMode/CreateNumLines.cxx` (included file)
- [ ] Copy `DigitMode/SelectNumber.cxx` (included file)

### Step 1.2: Copy Support Files
- [ ] Copy `Utils/middle.h`
- [ ] Copy `Utils/middle.cpp`
- [ ] Copy `InterfSolver/Tools/isPupil.h`
- [ ] Copy `InterfSolver/Tools/isPupil.cpp`
- [ ] Copy `MGTools/Include/Utils/BaseDataType.h`
- [ ] Copy `MGTools/Include/Utils/Utils.h` (or minimal stub)
- [ ] Copy `AppDef.h` (constants only)

### Step 1.3: Create Stub/Minimal Control Headers
- [ ] Create `Controls/controls.h` (minimal stub with needed members)
- [ ] Create `Controls/BoundCtrls.h` (minimal stub with needed members)
- [ ] Create `Controls/ImageCtrls.h` (minimal stub with needed members)
- [ ] Reference: List of needed members in each file (from analysis)

**Check for needed members:**
```cpp
// controls.h needs: CArray, FringeCenterAs, ViewState
// BoundCtrls.h needs: ExtBoundType, InsBoundType, ArrEll, ArrRect, ArrPlg, ArrContour
// ImageCtrls.h needs: m_pDIB, ImageSize, m_dwWidth, m_dwHeight
```

### Step 1.4: Fix Includes (Critical!)
- [ ] Open `DigitInfo.cpp`
- [ ] Replace: `#include "Utils\mutils.h"` ? Remove this line (we'll inject dependencies)
- [ ] Verify includes for middle.h, BaseDataType.h all resolve
- [ ] Fix any Windows.h / stdafx.h path issues
- [ ] Check for missing MGTools includes

### Step 1.5: Initial Compilation
- [ ] Attempt to build project
- [ ] Fix any compilation errors related to:
  - [ ] Missing includes
  - [ ] Undefined types (CArray, BOOL, COLORREF, etc.)
  - [ ] Missing method definitions in stubs
- [ ] Document any unexpected dependencies found
- [ ] **DO NOT proceed if build fails** - Fix compiler errors first

### Step 1.6: Verify Algorithm Still Works
- [ ] Copy `Tests/DigitModeTests/DigitInfoTest.cpp` mock implementations
- [ ] Create minimal test case (initialize CDigitInfo, call Init())
- [ ] Verify no crashes on instantiation
- [ ] Verify data structures are properly initialized

---

## Stage 2: Decouple Control Dependencies (?1 day)

### Step 2.1: Add Dependency Injection Infrastructure
- [ ] Create `DigitInfo_Dependencies.h` (or add to DigitInfo.h):

```cpp
// Pseudo-code structure
class CDigitInfo {
private:
    CBoundCtrls* m_pBounds;
    CImageCtrls* m_pImage;
    CControls* m_pControls;
    
public:
    void SetDependencies(CBoundCtrls* pB, CImageCtrls* pI, CControls* pC);
};
```

- [ ] Edit `DigitInfo.h` - add three member variables
- [ ] Edit `DigitInfo.h` - add SetDependencies() method declaration
- [ ] Edit `DigitInfo.cpp` - implement SetDependencies()

### Step 2.2: Replace Global Function Calls in CreateBufLineApertureSimple()
- [ ] Open `DigitInfo.cpp`, find method ~line 181
- [ ] Locate: `CBoundCtrls* pB = GetBoundCtrls();`
- [ ] Replace with: `CBoundCtrls* pB = m_pBounds;`
- [ ] Locate: `CImageCtrls* pI = GetImageCtrls();`
- [ ] Replace with: `CImageCtrls* pI = m_pImage;`
- [ ] Add NULL checks if needed:
  ```cpp
  if (!m_pBounds || !m_pImage) {
      TRACE("Dependencies not injected\n");
      return;
  }
  ```
- [ ] Compile and verify no errors

### Step 2.3: Replace Global Function Calls in CreateBufLineApertureComplex()
- [ ] Locate method ~line 250
- [ ] Replace `GetBoundCtrls()` with `m_pBounds`
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.4: Replace in CreateBufLineObstructionSimple()
- [ ] Locate method ~line 292
- [ ] Replace `GetBoundCtrls()` with `m_pBounds`
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.5: Replace in CreateBufLineObstructionComplex()
- [ ] Locate method ~line 383
- [ ] Replace `GetBoundCtrls()` with `m_pBounds`
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.6: Replace in CreateRedCenters()
- [ ] Locate method ~line 442
- [ ] Replace `GetBoundCtrls()` with `m_pBounds`
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Replace `GetControls()` with `m_pControls`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.7: Replace in CreateZAPSections()
- [ ] Locate method ~line 762
- [ ] Replace `GetBoundCtrls()` with `m_pBounds`
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.8: Replace in CreateZAPSectionsOnLoadZAPFile()
- [ ] Locate method ~line 813
- [ ] Replace `GetImageCtrls()` with `m_pImage`
- [ ] Add NULL checks
- [ ] Compile

### Step 2.9: Update Auto() Method
- [ ] Locate method ~line 442
- [ ] Remove (or comment out): `::SetCursor(::LoadCursor(NULL, IDC_WAIT));`
- [ ] Remove (or comment out): `::SetCursor(::LoadCursor(NULL, IDC_ARROW));`
- [ ] Add comment: `// TODO: Use progress callback instead`
- [ ] Compile

**Section 2 Complete Check:**
- [ ] All 8 methods updated (0 GetBoundCtrls calls remaining)
- [ ] All 0 GetImageCtrls calls remaining
- [ ] All 0 GetControls calls remaining
- [ ] Compile with no errors
- [ ] No references to global functions in modified methods

---

## Stage 3: Extract UI Rendering (?1 day)

### Step 3.1: Create Renderer Class Header
- [ ] Create new file: `DigitMode/DigitInfoRenderer.h`
- [ ] Add class declaration:
  ```cpp
  class CDigitInfoRenderer
  {
  public:
      CDigitInfoRenderer();
      ~CDigitInfoRenderer();
      
      void Draw(CDC* pDC, const CDigitInfo& digitInfo, int DotSide);
      void SetControls(CControls* pCtrl);
      
  private:
      void DrawZapLines(CDC* pDC, const CDigitInfo& digitInfo);
      void DrawSections(CDC* pDC, const CDigitInfo& digitInfo);
      void DrawDots(CDC* pDC, const CDigitInfo& digitInfo, int DotSide);
      void DrawFringeDots(CDC* pDC, const CDigitInfo& digitInfo);
      void DrawExtremums(CDC* pDC, const CDigitInfo& digitInfo);
      
      CControls* m_pControls;
  };
  ```
- [ ] Add includes: DigitInfo.h, controls.h, Appdef.h, windowsx.h

### Step 3.2: Extract CDigitInfo::Draw() Implementation
- [ ] Copy code from `DigitInfo.cpp` Draw() method
- [ ] Paste into new renderer
- [ ] Replace `this->` references with `digitInfo.`
- [ ] Replace `Sections` with `digitInfo.Sections`
- [ ] Replace `ZapLines` with `digitInfo.ZapLines`
- [ ] Replace `Dots` with `digitInfo.Dots`
- [ ] Replace `idxDragZapLine` with `digitInfo.idxDragZapLine`
- [ ] Replace `idxDragDot` with `digitInfo.idxDragDot`
- [ ] Replace `idxMainDot` with `digitInfo.idxMainDot`
- [ ] Replace `HidenDots` with `digitInfo.HidenDots`
- [ ] Replace `MainFringeNumber` with `digitInfo.MainFringeNumber`

### Step 3.3: Create Renderer Implementation File
- [ ] Create `DigitMode/DigitInfoRenderer.cpp`
- [ ] Implement constructor/destructor (empty stubs)
- [ ] Implement SetControls()
- [ ] Implement Draw() - main dispatcher
- [ ] Implement DrawZapLines() - from extracted code
- [ ] Implement DrawSections() - from extracted code  
- [ ] Implement DrawDots() - from extracted code
- [ ] Implement DrawFringeDots() - from extracted code
- [ ] Implement DrawExtremums() - from extracted code

### Step 3.4: Remove Draw() from CDigitInfo
- [ ] Open `DigitMode/DigitInfo.h`
- [ ] Find: `void Draw(CDC* pDC, int DotSide);`
- [ ] Delete this line (or comment it out)
- [ ] Compile - expect linker error initially

### Step 3.5: Verify Rendering Methods Still Exist (for now)
- [ ] CSectionInfo::Draw() - Should still exist (used by renderer)
- [ ] CDotInfo::Draw() - Should still exist (used by renderer)
- [ ] CZapLineInfo::Draw() - Should still exist (used by renderer)
- [ ] If needed, make these non-UI methods available (const data access)
- [ ] Compile

### Step 3.6: Add Renderer to Your View Class
- [ ] In your view header, add:
  ```cpp
  private:
      CDigitInfoRenderer m_renderer;
  ```
- [ ] In view OnDraw():
  ```cpp
  // Before:
  // pDigitInfo->Draw(pDC, DotSide);
  
  // After:
  m_renderer.SetControls(/* get controls */);
  m_renderer.Draw(pDC, *pDigitInfo, DotSide);
  ```
- [ ] Compile

**Stage 3 Complete Check:**
- [ ] CDigitInfoRenderer compiles cleanly
- [ ] DigitInfoRenderer.cpp/h files created
- [ ] No Draw() method in CDigitInfo
- [ ] View code updated to use renderer
- [ ] Compile with no unresolved externals

---

## Stage 4: Interface Refactoring (?0.5 day)

### Step 4.1: Create Progress Handler Interface (Optional - P2)
- [ ] Create: `DigitMode/IDigitInfoProgress.h`
- [ ] Add interface:
  ```cpp
  class IDigitInfoProgress {
  public:
      virtual ~IDigitInfoProgress() {}
      virtual void OnProcessingStarted() {}
      virtual void OnProcessingCompleted() {}
      virtual void OnProgress(int percentComplete) {}
  };
  ```
- [ ] Add to CDigitInfo: `void SetProgressHandler(IDigitInfoProgress* p);`
- [ ] Update Auto(): Remove SetCursor calls, use handler instead
- [ ] Compile

### Step 4.2: Create Serializer Interface (Optional - P3)
- [ ] Create: `DigitMode/IDigitInfoSerializer.h`
- [ ] Add interface with Load/Save methods
- [ ] Add to CDigitInfo: `void SetSerializer(IDigitInfoSerializer* p);`
- [ ] Update Load/Save methods: Delegate to serializer
- [ ] Compile

### Step 4.3: Handle OnKeyDown() (Optional - P2)
- [ ] Either:
  - [ ] A) Move logic to your view class
  - [ ] B) Create callback interface
  - [ ] C) Keep but don't use (mark as deprecated)
- [ ] Compile

**Stage 4 Complete Check:**
- [ ] Optional interfaces created
- [ ] CDigitInfo updated with setters
- [ ] Methods delegate to handlers/serializers
- [ ] No more SetCursor() calls in CDigitInfo
- [ ] Compile with no errors

---

## Post-Extraction Verification

### Compilation Check
- [ ] Full solution compiles cleanly ?
- [ ] No unresolved external symbols
- [ ] No C4996 deprecation warnings for extracted code
- [ ] No C4101 unused variable warnings

### Dependency Check
- [ ] Search for "GetBoundCtrls" in extracted code ? **0 matches**
- [ ] Search for "GetImageCtrls" in extracted code ? **0 matches**
- [ ] Search for "GetControls" in extracted code ? **0 matches** (except in stubs)
- [ ] Search for "#include.*mutils.h" ? **0 matches**

### Functionality Check
- [ ] Can instantiate: `CDigitInfo digitInfo;` ?
- [ ] Can call: `digitInfo.Init();` ?
- [ ] Can call: `digitInfo.SetDependencies(...);` ?
- [ ] Can call query methods without crash ?
- [ ] Existing unit tests still pass ?

### Integration Check
- [ ] View can call: `m_renderer.Draw(pDC, *pDigitInfo, DotSide);` ?
- [ ] View compiles with renderer ?
- [ ] No runtime crashes when dependencies injected ?

---

## Testing & Validation

### Unit Tests
- [ ] Run existing `DigitInfoTest.cpp` with mock implementations
- [ ] All tests pass: CreateNumLines algorithm works
- [ ] All tests pass: Number propagation correct
- [ ] All tests pass: Query methods return correct results

### Integration Tests
Create these in your new project:
- [ ] Test with mock CImageCtrls (fake DIB data)
- [ ] Test with mock CBoundCtrls (fake bounds)
- [ ] Test with mock CControls (settings)
- [ ] Test Auto() executes without crash
- [ ] Test all CreateBufLine*() methods work

### Regression Tests
- [ ] Load sample interferogram data
- [ ] Run fringe numbering algorithm
- [ ] Compare output with original implementation
- [ ] Verify fringe numbers are identical
- [ ] Verify red center positions are identical

### Rendering Tests
- [ ] Create test CDC context (memory DC)
- [ ] Call renderer.Draw(CDC*, digitInfo, DotSide)
- [ ] Verify no crash
- [ ] Verify objects were drawn (no assertions)
- [ ] Load sample image, visualize output

---

## Documentation

- [ ] Create README.md explaining:
  - [ ] What CDigitInfo does
  - [ ] How to inject dependencies
  - [ ] How to use the renderer
  - [ ] Example code for integration
- [ ] Add code comments for:
  - [ ] SetDependencies() - explain why injection needed
  - [ ] Each refactored method - note about dependency requirements
  - [ ] Renderer class - explain separation of concerns
- [ ] Document any differences from original:
  - [ ] Constructor behavior
  - [ ] Method signatures changed
  - [ ] New required interfaces
- [ ] Create migration guide for teams using original

---

## Cleanup & Finalization

- [ ] Remove all TODO comments (or resolve them)
- [ ] Remove debug TRACE statements added during development
- [ ] Code review: Check for commented-out code
- [ ] Code review: Check for inconsistent style vs original
- [ ] Ensure consistent variable naming (pB, pI, pC, m_p*)
- [ ] Add file headers with copyright/license info
- [ ] Verify no personal paths in code (C:\Ilya\... etc.)
- [ ] Create extraction validation script (checks dependencies)

---

## Sign-Off Checklist

**Quality Gate 1: Code Quality**
- [ ] Compiles with no warnings (excluding 3rd party code)
- [ ] All dependencies properly injected
- [ ] No global function calls remaining
- [ ] Code style consistent with original

**Quality Gate 2: Functionality**
- [ ] Algorithm produces correct results
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Rendering produces expected output

**Quality Gate 3: Integration**
- [ ] Can be compiled in new project independently
- [ ] Works with different control implementations
- [ ] Can be integrated into consuming application
- [ ] Documentation complete

**Quality Gate 4: Documentation**
- [ ] README explains usage
- [ ] Code comments explain refactoring
- [ ] Migration guide for teams
- [ ] Examples provided

---

## Estimated Timeline

| Stage | Effort | Days | Key Deliverable |
|-------|--------|------|-----------------|
| Setup & Planning | 4 hrs | 0.5 | Initial analysis, decisions |
| Stage 1: Core Extract | 6 hrs | 0.75 | Compiling codebase |
| Stage 2: Dependency Injection | 8 hrs | 1 | Working algorithm without globals |
| Stage 3: Renderer Extraction | 6 hrs | 0.75 | Rendering separated |
| Stage 4: Interfaces | 4 hrs | 0.5 | Optional handlers working |
| Testing & Validation | 8 hrs | 1 | All tests passing |
| Documentation & Cleanup | 4 hrs | 0.5 | Ready for delivery |
| **TOTAL** | **40 hrs** | **5 days** | **Extracted CDigitInfo** |

---

## Troubleshooting

### "Cannot resolve GetBoundCtrls"
? You still have `#include "Utils/mutils.h"` somewhere
? Search for "mutils.h" and remove it

### "CControls not defined"
? Did you create `Controls/controls.h`?
? Add `#include "Controls/controls.h"` to DigitInfo.cpp

### "Method GetBoundCtrls not found at call site"
? You forgot to remove the global function call
? Replace with `m_pBounds`

### "m_pBounds is NULL, crash!"
? You didn't call SetDependencies()
? Add NULL check: `if (!m_pBounds) return;`

### "CDC methods not found"
? Missing `#include <afxwin.h>` or similar MFC header
? Check your StdAfx.h includes MFC

### "CreateNumLines doesn't work"
? Check that SelectNumber.cxx is included
? Verify macros in middle.h are defined
? Check array sizing in CNumLine

---

## Next Steps After Completion

1. **Create Example/Demo Project**
   - Show how to use extracted CDigitInfo
   - Demonstrate dependency injection
   - Provide sample data files

2. **Create Wrapper Classes (Optional)**
   - If target project doesn't use MFC, create wrapper
   - Abstract away CDC from rendering
   - Use abstract image interface instead of SECDib

3. **Performance Optimization (Optional)**
   - Profile CreateRedCenters() algorithm
   - Optimize buf_line allocation
   - Reduce memory allocations

4. **Version Control**
   - Merge to main branch
   - Tag as "CDigitInfo-v1.0-extracted"
   - Create release notes

5. **Team Communication**
   - Brief team on changes
   - Provide usage documentation
   - Answer integration questions

---

**Good luck with the extraction! ??**

Use this checklist as your progress tracker. Check off items as you complete them.
When stuck, refer back to the detailed guides:
- DIGITINFO_EXTRACTION_SUMMARY.md
- DIGITINFO_DEPENDENCY_ANALYSIS.md  
- DIGITINFO_REFACTORING_GUIDE.md

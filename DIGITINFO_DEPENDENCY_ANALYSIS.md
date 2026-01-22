# CDigitInfo Dependency Analysis & Extraction Plan

## Executive Summary
`CDigitInfo` is a complex class that manages fringe numbering data in interferogram images. It has **significant dependencies** on:
- MFC framework (CArray, CDC, etc.)
- Global functions that access singleton controls (GetBoundCtrls, GetImageCtrls, GetControls)
- UI-related operations (CDC drawing, cursor manipulation)
- Image processing libraries (MGTools)
- File I/O operations
- Custom utilities (mutils, middle.h)

**Extraction difficulty: MEDIUM-HIGH** - Requires careful API refactoring to decouple from global control dependencies.

---

## 1. DIRECT DEPENDENCIES (Class Includes)

### In DigitInfo.h
```
? MGTools\StdAfx.h           - MFC precompiled headers
? Appdef.h                   - App constants & defines
? MGTools\Include\Utils\BaseDataType.h  - Data structures (CArray, CDPoint, CDLine, etc.)
? DigitMode\SectionInfo.h    - CSectionInfo class (composited array)
? DigitMode\DotInfo.h        - CDotInfo class (composited array)
? DigitMode\ZapLineInfo.h    - CZapLineInfo class (composited array)
? InterfSolver\Tools\ReadWriteData.h   - File I/O structures (NUMBERING_INTERFEROGRAM_INFO)
```

### In DigitInfo.cpp
```
? DigitInfo.h                - Self
? Utils\mutils.h             - Global accessor functions + utility functions
? Utils\middle.h             - Image processing (fon_del, invert_line, SortDouble)
? MGTools\Include\Utils\Utils.h  - Common utilities
? <math.h>                   - Math functions
? <filesystem>               - Path handling
? <string>                   - Std string
? <windowsx.h>               - Windows API
? DigitMode\CreateNumLines.cxx  - Included implementation file
? DigitMode\SelectNumber.cxx    - Included implementation file
```

---

## 2. COMPOSITION RELATIONSHIPS (Owned Objects)

These must be extracted with CDigitInfo:

| Member | Type | Purpose | Location |
|--------|------|---------|----------|
| `Sections` | `CArray<CSectionInfo>` | Lines containing fringe information | ? Own it |
| `ZapLines` | `CArray<CZapLineInfo>` | Manual fringe section markers | ? Own it |
| `Dots` | `CArray<CDotInfo>` | Manual reference fringe points | ? Own it |
| `HidenDots` | `CArray<CDPoint>` | Detected fringe centers | ? Own it |
| `buf_line` | `int**` | 2D buffer for line processing | ? Own it |

---

## 3. CRITICAL DEPENDENCIES - GLOBAL SINGLETON ACCESS

These are **SEAMS** where refactoring is needed:

### A. Control Access Functions (defined in Utils/mutils.h)
```cpp
// These functions access global singleton state
CControls* GetControls();              // Fringe detection & rendering settings
CImageCtrls* GetImageCtrls();          // Image data & properties
CBoundCtrls* GetBoundCtrls();          // Aperture/obstruction boundaries

// Used in these methods:
- CreateBufLineAperture()              // Lines 148-164
- CreateBufLineOnstruction()           // Lines 166-179
- CreateBufLineApertureSimple()        // Lines 181-248
- CreateBufLineApertureComplex()       // Lines 250-290
- CreateBufLineObstructionSimple()     // Lines 292-381
- CreateBufLineObstructionComplex()    // Lines 383-440
- CreateRedCenters()                   // Lines 442-552
- CreateZAPSections()                  // Lines 762-811
- CreateZAPSectionsOnLoadZAPFile()     // Lines 813-868
```

**Refactoring Strategy:**
- **Dependency Injection**: Pass `CImageCtrls*`, `CBoundCtrls*`, `CControls*` as parameters
- **Alternative**: Create an abstraction interface for image/bounds data access

---

### B. UI Dependencies (CDC - Device Context Drawing)

Methods with UI dependencies:
```cpp
void Draw(CDC* pDC, int DotSide);     // Lines 583-683
void OnKeyDown(...)                    // Lines 108-140
void CreateRedCenters() [SetCursor]    // Lines 113 & 449
void Auto() [SetCursor]                // Lines 442 & 449
```

**Issues:**
- `::SetCursor()` - Windows API for cursor manipulation
- `CDC*` - MFC device context (inherent to MFC)
- `pDC->SetPixelV()`, `pDC->MoveTo()`, `pDC->LineTo()` - MFC drawing

**Refactoring Strategy:**
- Move Draw() to a separate renderer class (separate concern)
- OnKeyDown() could use callback pattern or event system
- SetCursor calls can be wrapped or removed for non-interactive mode

---

### C. File I/O Dependencies

```cpp
BOOL Load(LPCTSTR fname);              // File read
BOOL Save(LPCTSTR fname, int extIdx);  // File write
BOOL LoadZAP(LPCTSTR fname);
BOOL LoadFRN(LPCTSTR fname);
BOOL SaveZAP(LPCTSTR fname, int extIdx);
BOOL SaveFRN(LPCTSTR fname);

// Comment in header: "TODO: remove file dependency"
```

**Location:** Lines 1025-1043 in .cpp (stub implementations)
**Refactoring Strategy:** These appear to be stubs. Full implementation may be in separate .cxx files.

---

### D. Image Path Resolution (New Dependency)

```cpp
static std::string ResolveImagePath(const std::string& dataFilePath, 
                                    const std::string& imageFileName);
static BOOL CreateFakeGrayImage(CImageCtrls* pImageCtrls, int width, int height);
```

**Issues:**
- Resolves relative image paths
- Creates fake images if missing
- Depends on `CImageCtrls*` pointer

---

## 4. INDIRECT DEPENDENCIES (Through Composed Classes)

### CSectionInfo ? DigitMode\SectionInfo.h
```cpp
class CNumLine {
    double Number;
    CDLine segmL;     // BaseDataType
    BOOL Included;
    double redX;
};

class CSectionInfo {
    CDLine L;
    CArray<CNumLine> NumLines;
    double aveStep;
    COLORREF Color;
    BOOL MainLine;
    BOOL VisibleRedDots;
    
    // Methods with UI dependency:
    void Draw(CDC* pDC, double MainFringeNumber);  // TODO: remove UI dependency
    void Form(int i, unsigned char* line, int n, int ny, int **buf_line);
};
```

**UI Dependency:** CSectionInfo::Draw() uses CDC* ? Must refactor

---

### CDotInfo ? DigitMode\DotInfo.h
```cpp
class CDotInfo {
    int iZapSec;
    double Number;
    CDPoint P;
    
    // UI dependency:
    void Draw(CDC* pDC, int dotSide, BOOL mainDot=FALSE);  // TODO: remove UI dependency
};
```

**UI Dependency:** CDotInfo::Draw() uses CDC* ? Must refactor

---

### CZapLineInfo ? DigitMode\ZapLineInfo.h
```cpp
class CZapLineInfo {
    int iSec;
    double Number;
    CDLine L;
    BOOL Removed;
    
    // UI dependency:
    void Draw(CDC* pDC);  // TODO: remove UI dependency
};
```

**UI Dependency:** CZapLineInfo::Draw() uses CDC* ? Must refactor

---

## 5. EXTERNAL LIBRARY DEPENDENCIES

### BaseDataType.h (MGTools\Include\Utils\BaseDataType.h)
**Provides:**
- `CDPoint` (x, y double coordinates)
- `CDLine` (P1, P2 line segments)
- `CArray<T>` (MFC dynamic array)
- `CRect` (rectangle)
- `CPoint` (integer coordinates)
- `XYPoint`, `XYEllipse`, `XYPolygon`, `XYRect` (geometry)

**Status:** ? **Safe to extract** - Foundation library, no backlinks to DigitInfo

---

### Utils Functions (Utils\middle.h)
```cpp
void middle(unsigned char* line, int nx, int ny, int y, int **buf_line, 
            CArray<double, double>& CenterFrg, int& nnpolos);
double approx(int *n,int *x,int *y);
void SortDouble(CArray<double, double>& CenterFrg);
void fon_del(unsigned char* line, int x , int x1);        // Background deletion
void invert_line(unsigned char* line, int x , int x1);
void delet_u(unsigned char* line, int end1, int end2, double aa, double bb);
```

**Status:** ? **Safe to extract** - Pure processing functions, self-contained

---

### isPupil Functions (InterfSolver\Tools\isPupil.h/.cpp)
```cpp
bool isPupil(XYPoint P, CArrayXYEllipse& ArrEll, 
             CArrayXYRect& ArrRect, CArrayXYPolygon& ArrPlg);
bool isPupil(XYPoint P, CArrayXYPolygon& ArrContour);
```

**Used in:** CreateBufLineApertureComplex(), CreateBufLineObstructionComplex()
**Status:** ? **Safe to extract** - Geometry helper, self-contained

---

## 6. ARCHITECTURE DIAGRAM

```
???????????????????????????????????????????????????????????????
?                    CDigitInfo                               ?
?  (Fringe numbering data & algorithms)                       ?
?                                                             ?
?  ????????????????????????????????????????????????????      ?
?  ?  Composed Objects (EXTRACT AS-IS)                ?      ?
?  ?  • CArray<CSectionInfo> Sections                 ?      ?
?  ?  • CArray<CZapLineInfo> ZapLines                 ?      ?
?  ?  • CArray<CDotInfo> Dots                         ?      ?
?  ?  • CArray<CDPoint> HidenDots                     ?      ?
?  ?  • int** buf_line                                ?      ?
?  ????????????????????????????????????????????????????      ?
?                                                             ?
?  ????????????????????????????????????????????????????      ?
?  ?  Algorithm Methods (EXTRACT WITH REFACTORING)    ?      ?
?  ?  • CreateBufLine*()  ???????????                 ?      ?
?  ?  • CreateRedCenters()  ?????    ?                ?      ?
?  ?  • CreateNumLines()        ?    ?                ?      ?
?  ?  • SelectMainSection()     ?    ?                ?      ?
?  ?  • CorrectNumbers()        ?    ?                ?      ?
?  ?  • CreateZAPSections()     ?    ?                ?      ?
?  ???????????????????????????????????????????????????      ?
?                             ?    ?                         ?
?  ???????????????????????????????????????????????????      ?
?  ?  UI Methods (SEPARATE TO NEW CLASS)             ?      ?
?  ?  • Draw(CDC*)  ???? REFACTOR OUT                ?      ?
?  ?  • CreateRedCenters() [SetCursor part]          ?      ?
?  ?  • OnKeyDown() [event handling]                 ?      ?
?  ?????????????????????????????????????????????????      ?
?                                                             ?
?  ????????????????????????????????????????????????????      ?
?  ?  File I/O Methods (MARK AS TODO)                 ?      ?
?  ?  • Load/Save/LoadZAP/SaveZAP/etc.                ?      ?
?  ?  • ResolveImagePath() [static]                   ?      ?
?  ?  • CreateFakeGrayImage() [static]                ?      ?
?  ????????????????????????????????????????????????????      ?
???????????????????????????????????????????????????????????????
        ?                    ?                    ?
        ?                    ?                    ?
????????????????  ????????????????????  ??????????????????????
? TIGHT SEAMS  ?  ?  SEMI-TIED       ?  ?  GOOD TO GO        ?
?(Inject deps) ?  ?  (Interface)     ?  ?(Extract freely)    ?
????????????????  ????????????????????  ??????????????????????
   ? ? ?            ? ?                    ? ? ? ?
   ? ? ?            ? ?                    ? ? ? ?
   • • •            • •                    • • • •
GetBoundCtrls   CSectionInfo         BaseDataType
GetImageCtrls   CDotInfo             mutils functions
GetControls     CZapLineInfo         middle functions
SetCursor       Draw(CDC*)           isPupil functions
CDC*            OnKeyDown()          AppDef.h
```

---

## 7. EXTRACTION STRATEGY

### PHASE 1: Extract Core Data (No Refactoring Needed)
? **CDigitInfo** class definition with:
  - All member variables
  - CSectionInfo, CDotInfo, CZapLineInfo, CDPoint arrays
  - buf_line management (Init_buf_line, Delete_buf_line)

? **Composed classes:**
  - CSectionInfo
  - CDotInfo
  - CZapLineInfo
  - CNumLine

? **Dependencies:**
  - BaseDataType.h (copy from MGTools)
  - AppDef.h (constants)
  - mutils.h functions (pure algorithms)
  - middle.h functions

---

### PHASE 2: Refactor Control Dependencies
**Approach 1 - Dependency Injection (RECOMMENDED):**
```cpp
class CDigitInfo {
    // Instead of:
    // CBoundCtrls* pB = GetBoundCtrls();
    
    // Accept injected dependencies:
private:
    CBoundCtrls* m_pBounds;
    CImageCtrls* m_pImage;
    CControls* m_pControls;
    
public:
    void SetDependencies(CBoundCtrls* bounds, CImageCtrls* image, CControls* ctrl);
};
```

**Affected Methods:** (21 total)
- CreateBufLineAperture*() [2 methods]
- CreateBufLineObstruction*() [2 methods]
- CreateRedCenters()
- CreateZAPSections()
- CreateZAPSectionsOnLoadZAPFile()
- Auto()

---

### PHASE 3: Extract UI Rendering
**Create separate `CDigitInfoRenderer` class:**
```cpp
class CDigitInfoRenderer {
    void Draw(CDC* pDC, const CDigitInfo& digitInfo, int DotSide);
    void DrawSections(CDC* pDC, ...);
    void DrawZapLines(CDC* pDC, ...);
    void DrawDots(CDC* pDC, ...);
};

// CDigitInfo no longer owns Draw() and related UI methods
```

**Methods to move:**
- CDigitInfo::Draw()
- CSectionInfo::Draw()
- CDotInfo::Draw()
- CZapLineInfo::Draw()

---

### PHASE 4: Handle File I/O
**Options:**
1. **Keep in place** - these are already stubs (minimal implementation)
2. **Create abstraction** - IDigitInfoSerializer interface
3. **Defer** - implement in consuming project

---

## 8. BREAKING POINTS (Seams) SUMMARY

| Seam | Type | Files Affected | Refactoring Effort | Priority |
|------|------|---------------|--------------------|----------|
| GetBoundCtrls/GetImageCtrls/GetControls | Dependency Injection | 7 methods in DigitInfo.cpp | **HIGH** | **P0** |
| CDC::Draw() | Extract to Renderer | 4 Draw() methods | **MEDIUM** | **P1** |
| ::SetCursor() | Callback/Wrapper | Auto(), CreateRedCenters() | **LOW** | **P2** |
| OnKeyDown() | Event System | CDigitInfo | **LOW** | **P2** |
| File I/O | Serializer Interface | Load/Save/Etc | **MEDIUM** | **P3** |
| Image Path Resolution | Pass as parameter | ResolveImagePath(), CreateFakeGrayImage() | **LOW** | **P3** |

---

## 9. ESTIMATED EFFORT

| Phase | Effort | Days | Complexity |
|-------|--------|------|-----------|
| Phase 1: Extract Core Data | 2-3 hours | < 1 | Low |
| Phase 2: Refactor Control Dependencies | 8-12 hours | 1-2 | **High** |
| Phase 3: Extract UI Rendering | 6-8 hours | 1 | Medium |
| Phase 4: File I/O Abstraction | 4-6 hours | < 1 | Medium |
| **TOTAL** | **20-30 hours** | **3-4 days** | **Medium** |

---

## 10. RECOMMENDED EXTRACTION ORDER

1. **First:** Extract composed types (CSectionInfo, CDotInfo, CZapLineInfo, BaseDataType)
2. **Second:** Extract CDigitInfo with dependency injection pattern
3. **Third:** Extract utility functions (mutils, middle)
4. **Fourth:** Create CDigitInfoRenderer in consuming project
5. **Fifth:** Implement serialization interface in consuming project

---

## CONCLUSION

**CDigitInfo is extractable** with moderate refactoring effort, primarily around:
- **Dependency Injection** for control access (GetBoundCtrls, GetImageCtrls, GetControls)
- **UI Separation** via renderer extraction
- **File I/O Abstraction** for serialization

The core algorithm is **sound and self-contained** once dependencies are injected. Estimated 3-4 days for complete extraction with full refactoring.

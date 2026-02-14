# CBaseImageDoc::boundCtrls Dependency Graph

**Document Purpose**: This document maps all dependencies and usage patterns of the `CBaseImageDoc::boundCtrls` member variable throughout the Digit project to facilitate refactoring.

**Last Updated**: 2024
**Member Type**: `CBoundCtrls boundCtrls`
**Declared In**: `ImageTempl\BaseImageDoc.h` (line 33)
**Total References Found**: 49+ occurrences across the codebase

---

## Table of Contents
1. [Overview](#overview)
2. [Data Members Access Patterns](#data-members-access-patterns)
3. [Method Call Patterns](#method-call-patterns)
4. [File-by-File Usage Breakdown](#file-by-file-usage-breakdown)
5. [Dependency Graph](#dependency-graph)
6. [Refactoring Considerations](#refactoring-considerations)

---

## Overview

### Class Definition Location
- **File**: `ImageTempl\BaseImageDoc.h`
- **Line**: 33
- **Declaration**: `CBoundCtrls boundCtrls;`
- **Access**: Public member of `CBaseImageDoc`

### CBoundCtrls Interface
CBoundCtrls now implements `IBoundsData` interface (defined in `DigitMode\IBoundsData.h`).

---

## Data Members Access Patterns

### 1. **ExtBoundType** (External Bound Type)
**Type**: `int`
**Purpose**: Stores the shape type of the external bound (BOUND_RECT, BOUND_ELLIPSE, BOUND_POLYGON, BOUND_NONE)

**Read Access**:
- `ImageTempl\ImageView.cpp:52` - Read to determine external bound type
- `Tests\DigitModeTests\BoundsHandlerTest.cpp:376, 550` - Test assertions
- Via `GetExtBoundType()` interface method

**Write Access**:
- `Tests\DigitModeTests\BoundsHandlerTest.cpp:59` - Direct assignment in tests
- Internal modifications in `CBoundCtrls::SetBound()` and `CBoundCtrls::AddBound()`

---

### 2. **InsBoundType** (Internal Bound Type)
**Type**: `int`
**Purpose**: Stores the shape type of the internal bound (obstruction)

**Read Access**:
- `ImageTempl\ImageView.cpp:82` - Read to determine internal bound type
- Via `GetInsBoundType()` interface method

**Write Access**:
- Internal modifications in `CBoundCtrls::SetBound()` and `CBoundCtrls::AddBound()`

---

### 3. **CurBound** (Current Bound Being Edited)
**Type**: `CRect`
**Purpose**: Temporary storage for the bound being edited (before commit)

**Read Access**:
- Used internally during bound editing operations
- Read by drawing code to show preview

**Write Access**:
- Modified during interactive editing
- Cleared after `AddBound()` or `SetBound()`

---

### 4. **CustomDot** (Current Custom Dot Position)
**Type**: `CPoint`
**Purpose**: Stores the position of the currently active custom dot during polygon/custom bound editing

**Read/Write Access**:
- `ImageTempl\BaseImageView.cpp:402, 430, 431, 432` - Mouse tracking and marker drawing
  - Line 402: Check if dot can be removed
  - Line 430: Draw marker at current position
  - Line 431: Update position
  - Line 432: Redraw marker at new position

---

### 5. **CustomDots** (Array of Custom Dots)
**Type**: `CArray<CPoint, CPoint>`
**Purpose**: Collection of all custom dots for polygon/custom bound definition

**Read Access**:
- `ImageTempl\BaseImageView.cpp:404` - Check if this is first dot (`GetSize()==1`)

**Write Access**:
- `ImageTempl\ImageDoc.cpp:148, 166` - `RemoveAll()` during cleanup operations

---

### 6. **ArrRect** (Array of Rectangle Bounds)
**Type**: `CArrayXYRect`
**Purpose**: Stores committed rectangle bounds

**Access Patterns**:
- `Tests\DigitModeTests\BoundsHandlerTest.cpp:45, 71, 72, 73, 377` - Test access
  - Line 45, 71: Validation (`ASSERT_VALID`)
  - Line 72: Clear array (`RemoveAll()`)
  - Line 73: Add rectangle (`Add()`)
  - Line 377: Check size (`GetSize()`)

---

### 7. **ArrEll** (Array of Ellipse Bounds)
**Type**: `CArrayXYEllipse`
**Purpose**: Stores committed ellipse bounds

**Access Patterns**:
- Used via `GetPartsOfContours()` method
- Internal access during bound calculations

---

### 8. **ArrPlg** (Array of Polygon Bounds)
**Type**: `CArrayXYPolygon`
**Purpose**: Stores committed polygon bounds

**Access Patterns**:
- Used via `GetPartsOfContours()` method
- Internal access during contour calculations

---

### 9. **ArrContour** (Resulting Contour)
**Type**: `CArrayXYPolygon`
**Purpose**: Combined contour after merging all bounds for isPupil test

**Access Patterns**:
- Calculated internally
- Used by visibility testing algorithms

---

### 10. **CurPlg** (Current Polygon Points)
**Type**: `CArray<CPoint, CPoint>`
**Purpose**: Polygon points for the bound currently being edited

**Access Patterns**:
- Modified during polygon editing
- Cleared after commit

---

### 11. **LastAddedBoundType** (History of Added Bounds)
**Type**: `CArray<int, int>`
**Purpose**: Tracks the type of each added bound for undo operations

**Access Patterns**:
- Used by `RemoveLastBound()` to identify which bound to remove

---

### 12. **NPntNax** (Max Polygon Points)
**Type**: `int`
**Purpose**: Maximum number of points for polygon contour approximation (magic number)

**Access Patterns**:
- Set during initialization
- Used in polygon approximation algorithms

---

## Method Call Patterns

### Initialization & Cleanup
| Method | Callers | Purpose |
|--------|---------|---------|
| `Init()` | `Tests\DigitModeTests\BoundsHandlerTest.cpp:41` | Initialize/reset all bounds data |
| `RemoveAllBound()` | Various cleanup contexts | Clear all bounds (calls `RemoveExtBound()` + `RemoveInsBound()`) |
| `RemoveExtBound()` | Explicit external bound removal | Clear external bounds |
| `RemoveInsBound()` | Explicit internal bound removal | Clear internal bounds |
| `RemoveLastBound()` | Undo operations | Remove most recently added bound |

---

### Editing Operations
| Method | Callers | Purpose |
|--------|---------|---------|
| `AddCustomDot(CPoint P)` | `ImageTempl\BaseImageView.cpp:403` | Add a custom dot to current polygon |
| `RemoveCustomDot(CPoint P)` | `ImageTempl\BaseImageView.cpp:402` | Remove a custom dot from current polygon |
| `CustomDotInFocus(CPoint P)` | Mouse hover/hit-testing | Check if point is near a custom dot |
| `SetCurBound(int _Type)` | `ImageTempl\BaseImageView.cpp:405, 407` | Stage current bound (preview, no commit) |
| `RemoveCurBound()` | Cancel operations | Discard current editing state |
| `SetBound(int _Type, int idxExtIns)` | Internal commit operations | Commit bound to arrays and recalc contour |
| `AddBound(int _Type, int idxExtIns)` | User commit actions | Calls `SetBound()` then clears `CurBound` and `CurPlg` |

---

### Query Operations (IBoundsData Interface)
| Method | Callers | Purpose |
|--------|---------|---------|
| `GetExtBoundType()` | `DigitMode\BoundsHandler.cpp:89` | Get external bound shape type |
| `GetInsBoundType()` | Various visibility checkers | Get internal bound shape type |
| `GetExtRealBound(...)` | `ImageTempl\ImageView.cpp:54`<br>`DigitMode\BoundsHandler.cpp:94`<br>`Tests\...\BoundsHandlerTest.cpp:347, 361` | Get external bound cropped to image dimensions |
| `GetInsRealBound(...)` | `ImageTempl\ImageView.cpp:85, 110` | Get internal bound cropped to image dimensions |
| `GetExtCorBound(...)` | Coordinate-corrected queries | Get external bound with optional coordinate correction |
| `GetInsCorBound(...)` | Coordinate-corrected queries | Get internal bound with optional coordinate correction |

---

### Specialized Queries
| Method | Callers | Purpose |
|--------|---------|---------|
| `IsExtBound()` | Conditional rendering/processing | Check if any external bound exists |
| `IsInsBound()` | Conditional rendering/processing | Check if any internal bound exists |
| `IsCurArea()` | Validation before commit | Check if current editing state is valid |
| `GetPartsOfContours(...)` | `DigitMode\DigitInfo.cpp`, contour processing | Get arrays of specific bound type parts |

---

### File I/O
| Method | Callers | Purpose |
|--------|---------|---------|
| `FormBoundsOnLoadFile()` | Document loading | Reconstruct bounds after file deserialization |

---

## File-by-File Usage Breakdown

### 1. **ImageTempl\ImageView.cpp**
**Total References**: ~15
**Role**: Primary UI interaction and rendering

**Usage Patterns**:
```cpp
Line 52:  int BoundType = pDoc->boundCtrls.ExtBoundType;
Line 54:  res = pDoc->boundCtrls.GetExtRealBound(BoundType, xDIB, yDIB, Bound, PlgPoints);
Line 82:  BoundType = pDoc->boundCtrls.InsBoundType;
Line 85:  res = pDoc->boundCtrls.GetInsRealBound(BoundType, xDIB, yDIB, idx, Bound, PlgPoints);
Line 110: res = pDoc->boundCtrls.GetInsRealBound(BoundType, xDIB, yDIB, idx, Bound, PlgPoints);
```

**Operations**:
- Query external/internal bound types
- Retrieve bounds for rendering
- Iterate through multiple internal bounds

---

### 2. **ImageTempl\BaseImageView.cpp**
**Total References**: ~7
**Role**: Base class mouse tracking and custom dot editing

**Usage Patterns**:
```cpp
Line 402: if(pDoc->boundCtrls.RemoveCustomDot(pDoc->boundCtrls.CustomDot))
Line 403: pDoc->boundCtrls.AddCustomDot(pDoc->boundCtrls.CustomDot);
Line 404: if(pDoc->boundCtrls.CustomDots.GetSize()==1)
Line 405:     pDoc->boundCtrls.SetCurBound(pCtrls->CurTypeBound);
Line 407:     pDoc->boundCtrls.SetCurBound(pCtrls->CurTypeBound);
Line 430: DrawMarker(dc, pDoc->boundCtrls.CustomDot);
Line 431: pDoc->boundCtrls.CustomDot = point;
Line 432: DrawMarker(dc, pDoc->boundCtrls.CustomDot);
```

**Operations**:
- Custom dot addition/removal
- Mouse tracking for polygon editing
- Preview rendering via `SetCurBound()`

---

### 3. **ImageTempl\ImageDoc.cpp**
**Total References**: 2
**Role**: Document cleanup

**Usage Patterns**:
```cpp
Line 148: boundCtrls.CustomDots.RemoveAll();
Line 166: boundCtrls.CustomDots.RemoveAll();
```

**Operations**:
- Clear custom dots during document reset/cleanup

---

### 4. **DigitMode\BoundsHandler.cpp**
**Total References**: ~5
**Role**: New architecture - uses IBoundsData interface

**Usage Patterns**:
```cpp
Line 89:  int extBoundType = m_pBounds->GetExtBoundType();
Line 94:  if (m_pBounds->GetExtRealBound(extBoundType, xDIB, yDIB, bound, plgPoints))
```

**Operations**:
- Hit-testing bound handles via interface
- Coordinate transformation
- **Uses interface methods, not direct member access**

---

### 5. **DigitMode\DigitInfo.cpp**
**Total References**: Multiple
**Role**: Digit analysis and fringe processing

**Key Methods Using boundCtrls**:
- Visibility checking (isPupil tests)
- Contour retrieval for analysis
- Bound-aware coordinate calculations

**Access Patterns**:
- Primarily through query methods
- Uses `GetPartsOfContours()` for analysis

---

### 6. **Tests\DigitModeTests\BoundsHandlerTest.cpp**
**Total References**: ~15
**Role**: Unit testing

**Usage Patterns**:
```cpp
Line 41:  boundCtrls.Init();
Line 45:  ASSERT_VALID(&boundCtrls.ArrRect);
Line 59:  boundCtrls.ExtBoundType = BOUND_RECT;
Line 72:  boundCtrls.ArrRect.RemoveAll();
Line 73:  boundCtrls.ArrRect.Add(rect);
Line 347: BOOL result = boundCtrls.GetExtRealBound(BOUND_RECT, 800, 600, bound, plgPoints);
Line 376: EXPECT_EQ(BOUND_RECT, boundCtrls.ExtBoundType);
Line 377: EXPECT_EQ(1, boundCtrls.ArrRect.GetSize());
```

**Operations**:
- Direct member access for test setup
- Validation of internal state
- Method behavior verification

---

### 7. **InterfSolver\Tools\CalcContour.cpp**
**Total References**: ~3
**Role**: Contour calculation utilities

**Usage Patterns**:
- Access to `ArrContour` for visibility calculations
- Polygon processing using `NPntNax`

---

### 8. **Controls\controls.cpp/h**
**Total References**: Multiple (via includes and helper functions)
**Role**: Control utilities and helper functions

---

## Dependency Graph

```
CBaseImageDoc
    └── boundCtrls (CBoundCtrls)
        │
        ├── DIRECT CONSUMERS (Legacy Direct Access)
        │   ├── ImageTempl\ImageView.cpp
        │   │   └── Rendering, bound retrieval
        │   ├── ImageTempl\BaseImageView.cpp
        │   │   └── Mouse tracking, custom dot editing
        │   ├── ImageTempl\ImageDoc.cpp
        │   │   └── Document cleanup
        │   ├── DigitMode\DigitInfo.cpp
        │   │   └── Analysis, fringe processing
        │   └── Tests\DigitModeTests\BoundsHandlerTest.cpp
        │       └── Unit testing
        │
        ├── INTERFACE CONSUMERS (Modern IBoundsData)
        │   └── DigitMode\BoundsHandler.cpp
        │       └── Uses GetExtBoundType(), GetExtRealBound() via IBoundsData*
        │
        ├── UTILITY CONSUMERS
        │   ├── InterfSolver\Tools\CalcContour.cpp
        │   │   └── Contour calculations
        │   └── Controls\controls.cpp
        │       └── Helper functions
        │
        └── DATA FLOW
            │
            ├── INPUT (Editing)
            │   └── BaseImageView → CustomDot/CustomDots → SetCurBound → AddBound → Arrays
            │
            ├── STORAGE
            │   ├── ArrRect (committed rectangles)
            │   ├── ArrEll (committed ellipses)
            │   ├── ArrPlg (committed polygons)
            │   └── ArrContour (merged result)
            │
            └── OUTPUT (Querying)
                ├── GetExtRealBound() → ImageView rendering
                ├── GetInsRealBound() → Obstruction handling
                └── GetPartsOfContours() → Analysis algorithms
```

---

## Refactoring Considerations

### 1. **Interface Migration Status**
- ✅ **IBoundsData interface defined** (`DigitMode\IBoundsData.h`)
- ✅ **CBoundCtrls implements IBoundsData**
- ✅ **BoundsHandler.cpp uses interface** (no direct `boundCtrls` access)
- ⚠️ **Legacy code still uses direct access** (ImageView.cpp, DigitInfo.cpp)

### 2. **Direct Access Hotspots** (Needs Refactoring)
| File | Lines | Access Type | Severity |
|------|-------|-------------|----------|
| `ImageTempl\ImageView.cpp` | ~15 refs | Direct member/method access | HIGH |
| `ImageTempl\BaseImageView.cpp` | ~7 refs | Direct member access (CustomDot) | HIGH |
| `ImageTempl\ImageDoc.cpp` | 2 refs | Direct array access (CustomDots) | MEDIUM |
| `DigitMode\DigitInfo.cpp` | Multiple | Direct method calls | MEDIUM |
| `InterfSolver\Tools\CalcContour.cpp` | ~3 refs | Direct array access | LOW |

### 3. **Recommended Refactoring Steps**

#### Phase 1: Extend IBoundsData Interface
Add missing methods to `IBoundsData`:
```cpp
// Missing interface methods:
virtual void AddCustomDot(CPoint P) = 0;
virtual bool RemoveCustomDot(CPoint P) = 0;
virtual bool CustomDotInFocus(CPoint P) = 0;
virtual void SetCurBound(int Type) = 0;
virtual void RemoveCurBound() = 0;
virtual bool AddBound(int Type, int idxExtIns) = 0;
virtual void RemoveLastBound() = 0;
virtual void RemoveAllBound() = 0;
virtual BOOL IsCurArea() const = 0;
// ... etc.
```

#### Phase 2: Migrate ImageView.cpp
Replace:
```cpp
int BoundType = pDoc->boundCtrls.ExtBoundType;
res = pDoc->boundCtrls.GetExtRealBound(...);
```

With:
```cpp
IBoundsData* pBounds = &pDoc->boundCtrls;  // or GetBoundsInterface()
int BoundType = pBounds->GetExtBoundType();
res = pBounds->GetExtRealBound(...);
```

#### Phase 3: Migrate BaseImageView.cpp
Abstract custom dot operations:
```cpp
// Old:
pDoc->boundCtrls.AddCustomDot(pDoc->boundCtrls.CustomDot);

// New:
IBoundsData* pBounds = pDoc->GetBoundsInterface();
pBounds->AddCustomDot(pBounds->GetCurrentCustomDot());
```

#### Phase 4: Data Member Encapsulation
Currently **public** members should become **protected/private**:
- `CurBound`, `CustomDot`, `CustomDots`, `CurPlg`
- `ArrEll`, `ArrRect`, `ArrPlg`, `ArrContour`
- `ExtBoundType`, `InsBoundType`, `LastAddedBoundType`

Provide getters/setters through interface.

### 4. **Breaking Change Analysis**

**High Risk** (many references):
- `CustomDot` - 7 direct accesses in BaseImageView.cpp
- `CustomDots` - 3 accesses (2 in ImageDoc.cpp)
- `ExtBoundType` / `InsBoundType` - Multiple reads in ImageView.cpp

**Medium Risk**:
- `ArrRect`, `ArrEll`, `ArrPlg` - Test code accesses directly
- Method calls like `GetInsRealBound(...)` with legacy overload

**Low Risk**:
- `NPntNax` - Internal use only
- `ArrContour` - Mostly internal

### 5. **Testing Strategy**
1. ✅ Existing unit tests in `BoundsHandlerTest.cpp`
2. ⚠️ Need integration tests for interface migration
3. ⚠️ Need regression tests for ImageView rendering
4. ⚠️ Need tests for custom dot editing workflow

### 6. **ApertureCore Integration**
**Current Blocker**: Macro conflict between `AppDef.h` and `ApertureCore\TypeLimits.h`

**Conflicting Macros**:
```cpp
// AppDef.h:78-79
#define S_INTERNAL 0
#define S_EXTERNAL 1

// TypeLimits.h:34, 44
enum class TypeLimits : uint8_t {
    EXTERNAL = 0,  // ← Conflicts with macro!
    INTERNAL = 1,  // ← Conflicts with macro!
```

**Solution**:
Add macro protection in `TypeLimits.h`:
```cpp
#pragma once
#include <cstdint>

// Protect against legacy MFC macro pollution
#ifdef EXTERNAL
#undef EXTERNAL
#endif
#ifdef INTERNAL
#undef INTERNAL
#endif

namespace aperture {
enum class TypeLimits : uint8_t {
    EXTERNAL = 0,
    INTERNAL = 1,
    // ...
```

**After Fix**:
Uncomment and enable:
```cpp
// BoundCtrls.h:9
#include "ApertureCore\include\ApertureCore\Visibility\ShapeCollection.h"

// BoundCtrls.h:29
aperture::ShapeCollection Shapes;  // Enable this member
```

### 7. **Dependency Injection Opportunity**
Instead of:
```cpp
CBaseImageDoc doc;
doc.boundCtrls.GetExtRealBound(...);  // Tight coupling
```

Consider:
```cpp
class CBaseImageDoc {
    IBoundsData* GetBoundsInterface() { return &boundCtrls; }
};

// Usage:
IBoundsData* pBounds = doc.GetBoundsInterface();
pBounds->GetExtRealBound(...);  // Loose coupling via interface
```

---

## Summary Statistics

| Metric | Count |
|--------|-------|
| **Total References** | 49+ |
| **Files Accessing boundCtrls** | 8+ |
| **Public Data Members** | 12 |
| **Public Methods** | 20+ |
| **Interface Methods Implemented** | 4 |
| **Test Files** | 1 (BoundsHandlerTest.cpp) |
| **Direct Access (needs migration)** | ~35 occurrences |
| **Interface Access (modern)** | ~5 occurrences |

---

## Next Steps for Refactoring

1. ✅ Fix `TypeLimits.h` macro conflict (add `#undef` guards)
2. ⬜ Extend `IBoundsData` interface with all missing methods
3. ⬜ Migrate `ImageView.cpp` to use interface
4. ⬜ Migrate `BaseImageView.cpp` custom dot operations to interface
5. ⬜ Encapsulate public data members (make protected/private)
6. ⬜ Add `CBaseImageDoc::GetBoundsInterface()` accessor
7. ⬜ Enable `aperture::ShapeCollection Shapes` member
8. ⬜ Update tests to use interface where appropriate
9. ⬜ Create integration tests for migration
10. ⬜ Document new interface usage patterns

---

**Document Version**: 1.0  
**Generated**: 2024  
**Maintainer**: Development Team

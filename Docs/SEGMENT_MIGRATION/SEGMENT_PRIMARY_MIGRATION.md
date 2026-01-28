# Segment-Primary Model Migration — Summary

**Date**: 2026-01-26  
**Status**: Specification documents updated  
**Model**: Segment-primary architecture (fringes as logical groupings)

---

## 1. Core Concept Changes

### Old Model (Fringe-as-Container)
```
Fringe #1.0 {
    Curve A (polyline)
    Curve B (polyline)
}
```

### New Model (Segment-Primary)
```
Segments:
    Segment A → Number = 1.0
    Segment B → Number = 1.0
    Segment C → Number = 2.0

Query "Fringe #1.0":
    → Returns {Segment A, Segment B}
```

**Key Insight**: Segments exist. Fringes are computed.

---

## 2. Terminology Changes

| Old Term | New Term | Meaning |
|----------|----------|---------|
| **Curve** | **Segment** | Continuous ordered polyline (primary object) |
| **Fringe** (container) | **Fringe** (logical group) | All segments with same Number value |
| `CFringe` | `CFringeSegment` | Class name for segment objects |
| `iCurve` | `iSegment` | Index into segment array |
| "Curve index" | "Segment index" | Tooltip terminology |

---

## 3. Class Renaming

### Primary Data Class
```cpp
// OLD
class CFringe {
    double m_Number;
    CArray<CDPoint> m_Points;
};

// NEW
class CFringeSegment {  // Or just CSegment
    double m_Number;              // Segment's number (determines fringe membership)
    std::vector<CDPoint> m_Points; // Ordered polyline
};
```

### Selection Enums
```cpp
// OLD
enum class SelectionLevel { None, Dot, Edge, Curve, Fringe };

// NEW
enum class SelectionLevel { None, Dot, Edge, Segment, Fringe };
```

---

## 4. Data Model Changes

### Old: Dual Storage
```cpp
class CDigitInfo {
    CArray<CFringe> Fringes;  // Container of curves by fringe
};
```

### New: Flat Storage + Queries
```cpp
class CDigitInfo {
    std::vector<CFringeSegment> segments;  // Primary storage
    
    // Fringes are computed on-demand
    std::vector<CFringeSegment*> GetSegmentsWithNumber(double num);
    std::set<double> GetAllNumbers();
    int GetSegmentCountForNumber(double num);
    int GetSegmentIndexInFringe(int iSegment);  // 1-based index within fringe
};
```

---

## 5. Selection Model Changes

### Old: Hierarchical within container
```
Fringe → Curve → Edge → Dot
```

### New: Flat with logical grouping
```
Segment → Edge → Dot
Fringe = query all segments with same Number
```

**SelectionManager Changes**:
```cpp
// OLD
void SelectCurve(int iFringe, int iCurve);
void SelectFringe(int iFringe);

// NEW
void SelectSegment(int iSegment);
void SelectFringe(double number);  // Selects ALL segments with this Number
```

---

## 6. Tooltip Changes

### Old Format
```
#2.5 / 1(3) / 12
  ↑     ↑ ↑    ↑
  │     │ │    └─ Dot index
  │     │ └────── Total curves in fringe
  │     └──────── Curve index
  └────────────── Fringe number
```

### New Format (Same display, different semantics)
```
#2.5 / 1(3) / 12
  ↑     ↑ ↑    ↑
  │     │ │    └─ Dot index within segment
  │     │ └────── Total segments in fringe (with Number=2.5)
  │     └──────── Segment index within fringe (computed, not stored)
  └────────────── Number value
```

---

## 7. Command Changes

### Renumbering Behavior

**Old**: Renumber within fringe container
```cpp
Fringe #1.0.SetNumber(2.0);
// All curves in container move to new number
```

**New**: Renumber changes fringe membership
```cpp
segment.SetNumber(2.0);
// Segment instantly belongs to Fringe #2.0 (logical group)
// No container move required
```

### Fringe Operations

**Old**: Operate on fringe container
```cpp
void DeleteFringe(int iFringe);  // Delete container
```

**New**: Operate on all segments with same Number
```cpp
void DeleteFringe(double number);  // Delete all segments where Number == number
```

---

## 8. File Renaming Required

### Source Files
- `DigitMode/CFringe.h` → `DigitMode/CFringeSegment.h`
- `DigitMode/CFringe.cpp` → `DigitMode/CFringeSegment.cpp`

### Test Files
- `Tests/DigitMode/CFringeTest.cpp` → `Tests/DigitMode/CFringeSegmentTest.cpp`
- `Tests/DigitMode/CFringeFileIOTest.cpp` → `Tests/DigitMode/CFringeSegmentFileIOTest.cpp`

### Project Files
- Update `Digit.vcxproj` with new file names
- Update `Digit.vcxproj.filters` with new filter structure

---

## 9. Documents Updated

### ✅ FRINGES_EDITOR_UX_SPECIFICATIONS.md
- Replaced "Curve" → "Segment" throughout
- Clarified "Fringe" as logical grouping
- Updated tooltips, commands, selection rules
- Added "Segment-Primary Model" subtitle

### ✅ IMPLEMENT_UX.md
- Updated architecture to use `CFringeSegment`
- Changed `iCurve` → `iSegment` in all examples
- Added fringe query methods (`GetSegmentsWithNumber()`)
- Updated commands (RenumberCommand, DeleteSelectionCommand)
- Clarified: "Segments exist, fringes are computed"

### ✅ ROADMAP_UX.md
- Phase 1: Added CFringe → CFringeSegment rename task
- Phase 2: Updated Draw mode to "segment creation"
- Updated all code examples with segment terminology
- Updated test names (DrawModeTest, SegmentTest)
- File organization reflects new naming

---

## 10. Migration Checklist

### Immediate Actions (Before Implementation)
- [ ] Rename `CFringe.h` → `CFringeSegment.h`
- [ ] Rename `CFringe.cpp` → `CFringeSegment.cpp`
- [ ] Update `class CFringe` → `class CFringeSegment`
- [ ] Rename test files (`CFringeTest` → `CFringeSegmentTest`)
- [ ] Update project files (`.vcxproj`, `.vcxproj.filters`)

### Code Changes (During Refactoring)
- [ ] Replace `CArray` → `std::vector` in CFringeSegment
- [ ] Add query methods to CDigitInfo
  - `GetSegmentsWithNumber(double)`
  - `GetAllNumbers()`
  - `GetSegmentCountForNumber(double)`
  - `GetSegmentIndexInFringe(int iSegment)`
- [ ] Update SelectionManager
  - `SelectSegment(int iSegment)`
  - `SelectFringe(double number)` (queries all matching segments)
- [ ] Update all `iCurve` → `iSegment` in codebase
- [ ] Update all "Curve" → "Segment" in comments/UI strings

### Testing (After Refactoring)
- [ ] All existing CFringe tests pass with new CFringeSegment
- [ ] Fringe query methods return correct results
- [ ] SelectFringe selects all segments with matching Number
- [ ] Renumbering changes fringe membership correctly
- [ ] Tooltips display correct segment index within fringe

---

## 11. Key Architectural Principles

### ✅ DO
- **Store segments** in a flat array/vector
- **Compute fringes** on-demand from segment Number values
- **Query by Number** to get all segments in a fringe
- **Renumber segments** to change fringe membership
- **Use std::vector** for collections (not CArray)
- **Use std::string** for text (not CString)

### ❌ DON'T
- ❌ Store fringes as containers of segments
- ❌ Maintain separate fringe objects
- ❌ Reconstruct topology when renumbering
- ❌ Use dual storage (segments + fringes)
- ❌ Mix MFC auxiliary types (CArray, CString) with STL

---

## 12. Implementation Roadmap

Follow these phases in order:

1. **Phase 1** (Weeks 1-2): Rename classes, create query methods
2. **Phase 2** (Weeks 3-4): Implement Draw mode with segment terminology
3. **Phase 3** (Weeks 5-6): Selection & Navigate mode
4. **Phase 4** (Week 7): Dot Edit mode
5. **Phase 5** (Weeks 8-9): Commands & Undo/Redo
6. **Phase 6** (Week 10): Polish & Testing

**Total**: 10 weeks (2.5 months) for full UX v1.0 implementation.

---

## 13. Reference Material

### Authoritative Documents (In Order)
1. **AMMENDMENT_TO_FRINGES_EDITOR_UX_SPECIFICATIONS.md** - Original segment-primary concept
2. **FRINGES_EDITOR_UX_SPECIFICATIONS.md** - User-facing UX spec (segment-primary)
3. **IMPLEMENT_UX.md** - Architecture design (segment-primary)
4. **ROADMAP_UX.md** - Implementation plan (segment-primary)

### One-Sentence Mental Model
> *You draw and edit **segments**; segments with the same **Number** together form a **fringe**.*

---

## 14. Next Steps

1. **Review this summary** with team
2. **Approve renaming** strategy (CFringe → CFringeSegment)
3. **Plan refactoring sprint** (1-2 days for rename + query methods)
4. **Begin Phase 1** of roadmap (Foundation)
5. **Test incrementally** at each phase

---

**End of Migration Summary**

**Questions? Refer to IMPLEMENT_UX.md for architecture details.**

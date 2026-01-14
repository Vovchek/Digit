# Phase 6: ApertureCore Integration Plan

**Date:** 2024  
**Phase:** 6 - Integration with Digit  
**Status:** PLANNING

---

## Executive Summary

Develop integration strategy to migrate Digit application from legacy InterfSolver geometry code to modern ApertureCore library. This will be a **gradual migration** using adapter layers to minimize risk and maintain functionality throughout the process.

---

## Current State Assessment

### InterfSolver Usage in Digit

**Primary Components:**
1. **XYShape Family** - Geometry primitives (XYEllipse, XYRect, XYPolygon)
2. **isPupil()** - Visibility checking function
3. **CalcContour()** - Contour calculation with occlusion
4. **CArray Types** - Collection management (CArrayXYEllipse, CArrayXYRect, CArrayXYPolygon)

**Usage Locations:**
- `InterfSolver/Tools/` - Implementation (DLL)
- Various Digit modules - Consumers of InterfSolver

### InterfSolver Architecture

```
InterfSolver (DLL)
├── XYShape (base class)
│   ├── XYEllipse
│   ├── XYRect
│   └── XYPolygon
├── isPupil() - visibility checking
├── CalcContour() - contour with occlusion
└── CArray<T> - MFC-based collections
```

### ApertureCore Architecture

```
ApertureCore (Static Library)
├── Shape (base class)
│   ├── Ellipse
│   ├── Rectangle
│   └── Polygon
├── VisibilityChecker - modern visibility engine
├── ShapeCollection - modern collection
└── std::vector<T> - STL collections
```

---

## Migration Challenges

### 1. API Differences

| Legacy (InterfSolver) | Modern (ApertureCore) | Challenge |
|----------------------|----------------------|-----------|
| `CArrayXYEllipse` | `std::vector<std::unique_ptr<Shape>>` | Collection type |
| `isPupil(point, arrays...)` | `checker.isVisible(point)` | API pattern |
| `XYEllipse` | `Ellipse` | Class names |
| `CalcContour()` | No direct equivalent | Complex algorithm |
| MFC `CArray` | STL `std::vector` | Container type |
| Raw pointers | `std::unique_ptr` | Ownership model |

### 2. Coordinate System Differences

**InterfSolver:**
- Uses `int TypeSystCoor` (MEASURING=0, NORMALISED=1)
- Manual coordinate transformation

**ApertureCore:**
- Uses `CoordinateSystem` enum (SCREEN, MATH)
- `NormalizationState` enum (MEASURING, NORMALIZED)
- Built-in transformation support

### 3. Collection Management

**InterfSolver:**
```cpp
CArrayXYEllipse ellipses;
ellipses.Add(XYEllipse(...));
isPupil(point, ellipses, rects, polygons);
```

**ApertureCore:**
```cpp
ShapeCollection shapes;
shapes.addExternal(std::make_unique<Ellipse>(...));
VisibilityChecker checker(shapes);
checker.isVisible(point);
```

---

## Integration Strategy

### Phase 6.1: Adapter Layer (Low Risk)

Create thin adapter layer to wrap ApertureCore for use in existing InterfSolver-based code.

**Goal:** Enable gradual migration without breaking existing code

**Components:**
1. **ApertureAdapter** - Wraps ApertureCore types
2. **Legacy type conversions** - XYEllipse ↔ Ellipse
3. **Collection adapters** - CArrayXY* ↔ ShapeCollection
4. **Function wrappers** - isPupil → VisibilityChecker

### Phase 6.2: Selective Migration (Medium Risk)

Migrate specific modules one at a time, starting with least critical.

**Goal:** Prove integration works, build confidence

**Candidates:**
1. Test/utility code first
2. New features use ApertureCore directly
3. Non-critical workflows

### Phase 6.3: Core Migration (Higher Risk)

Migrate critical paths to ApertureCore.

**Goal:** Replace InterfSolver in production code

**Targets:**
1. CalcContour() replacement
2. Main visibility checking
3. Core geometry operations

---

## Phase 6.1 Detailed Plan: Adapter Layer

### Step 1: Create ApertureAdapter Library

**New Project:** `ApertureAdapter` (static library)

**Purpose:** Bridge between InterfSolver and ApertureCore

**Location:** `ApertureAdapter/` (sibling to ApertureCore)

### Step 2: Type Conversion Functions

**File:** `ApertureAdapter/TypeConversion.h`

```cpp
namespace aperture::adapter {

// XYPoint ↔ Point
inline Point toAperturePoint(const XYPoint& xy) {
    return {xy.X, xy.Y};
}

inline XYPoint fromAperturePoint(const Point& p) {
    XYPoint xy;
    xy.X = p.x;
    xy.Y = p.y;
    return xy;
}

// XYEllipse ↔ Ellipse
std::unique_ptr<Ellipse> toApertureEllipse(const XYEllipse& xy);
XYEllipse fromApertureEllipse(const Ellipse& ellipse);

// XYRect ↔ Rectangle
std::unique_ptr<Rectangle> toApertureRectangle(const XYRect& xy);
XYRect fromApertureRectangle(const Rectangle& rect);

// XYPolygon ↔ Polygon
std::unique_ptr<Polygon> toAperturePolygon(const XYPolygon& xy);
XYPolygon fromAperturePolygon(const Polygon& poly);

} // namespace aperture::adapter
```

### Step 3: Collection Adapter

**File:** `ApertureAdapter/CollectionAdapter.h`

```cpp
namespace aperture::adapter {

class CollectionAdapter {
public:
    // Build ShapeCollection from legacy arrays
    static ShapeCollection fromLegacyArrays(
        const CArrayXYEllipse& ellipses,
        const CArrayXYRect& rects,
        const CArrayXYPolygon& polygons);
    
    // Extract legacy arrays from ShapeCollection
    static void toLegacyArrays(
        const ShapeCollection& shapes,
        CArrayXYEllipse& ellipses,
        CArrayXYRect& rects,
        CArrayXYPolygon& polygons);
};

} // namespace aperture::adapter
```

### Step 4: isPupil Wrapper

**File:** `ApertureAdapter/VisibilityAdapter.h`

```cpp
namespace aperture::adapter {

// Drop-in replacement for isPupil using ApertureCore
inline bool isPupil_Adapter(const XYPoint& P,
                            const CArrayXYEllipse& ellipses,
                            const CArrayXYRect& rects,
                            const CArrayXYPolygon& polygons) {
    // Convert to ApertureCore
    auto shapes = CollectionAdapter::fromLegacyArrays(ellipses, rects, polygons);
    VisibilityChecker checker(shapes);
    Point pt = toAperturePoint(P);
    
    // Use ApertureCore visibility checking
    return checker.isVisible(pt);
}

} // namespace aperture::adapter
```

### Step 5: CalcContour Adapter (Future)

**File:** `ApertureAdapter/ContourAdapter.h`

```cpp
namespace aperture::adapter {

// Adapter for CalcContour using ApertureCore
void CalcContour_Adapter(
    const CArrayXYEllipse& ellipses,
    const CArrayXYRect& rects,
    const CArrayXYPolygon& polygons,
    CArrayXYPolygon& contours,
    int NPntMax = N_CONT);

} // namespace aperture::adapter
```

---

## Implementation Timeline

### Week 1: Setup & Type Conversion
- [x] Phase 5 complete (documentation)
- [ ] Create ApertureAdapter project
- [ ] Implement type conversion functions
- [ ] Write unit tests for conversions

### Week 2: Collection & Visibility Adapters
- [ ] Implement CollectionAdapter
- [ ] Implement VisibilityAdapter (isPupil replacement)
- [ ] Write integration tests
- [ ] Performance comparison tests

### Week 3: Selective Migration
- [ ] Identify low-risk migration candidates
- [ ] Migrate test code to use adapters
- [ ] Migrate utility functions
- [ ] Validate results match legacy

### Week 4: CalcContour Integration
- [ ] Analyze CalcContour algorithm
- [ ] Design ApertureCore-based implementation
- [ ] Implement ContourAdapter
- [ ] Comprehensive testing

### Week 5+: Production Migration
- [ ] Migrate critical paths
- [ ] Performance validation
- [ ] Remove InterfSolver dependency
- [ ] Final testing and release

---

## Risk Mitigation

### Testing Strategy

1. **Unit Tests** - Test each adapter function individually
2. **Integration Tests** - Test adapter with real Digit data
3. **Comparison Tests** - Verify ApertureCore matches InterfSolver results
4. **Performance Tests** - Ensure no regression

### Rollback Plan

1. **Feature Flags** - Toggle between legacy and adapter at runtime
2. **Parallel Execution** - Run both, compare results
3. **Gradual Rollout** - Per-module migration
4. **Quick Revert** - Keep InterfSolver in place initially

---

## Success Criteria

### Phase 6.1 (Adapter) Success
- [x] ApertureCore builds and links successfully
- [ ] All type conversions work correctly
- [ ] isPupil_Adapter produces identical results to isPupil
- [ ] Performance within 10% of legacy
- [ ] Zero crashes or memory leaks

### Phase 6.2 (Selective Migration) Success
- [ ] At least 3 modules migrated successfully
- [ ] All existing tests pass
- [ ] No regression in functionality
- [ ] Code is cleaner/more maintainable

### Phase 6.3 (Core Migration) Success
- [ ] CalcContour replacement working
- [ ] All visibility checking uses ApertureCore
- [ ] InterfSolver dependency removed
- [ ] Performance meets or exceeds legacy
- [ ] Production-ready release

---

## Next Steps

**Immediate Actions:**

1. **Create ApertureAdapter project structure**
   ```
   ApertureAdapter/
   ├── include/apertureAdapter/
   │   ├── TypeConversion.h
   │   ├── CollectionAdapter.h
   │   └── VisibilityAdapter.h
   ├── src/
   │   ├── TypeConversion.cpp
   │   ├── CollectionAdapter.cpp
   │   └── VisibilityAdapter.cpp
   ├── tests/
   │   └── AdapterTests.cpp
   └── CMakeLists.txt
   ```

2. **Add to Digit build system**
   - Link ApertureCore
   - Link ApertureAdapter
   - Keep InterfSolver for now

3. **Begin type conversion implementation**
   - Start with XYPoint ↔ Point (simplest)
   - Move to XYEllipse ↔ Ellipse
   - Then XYRect and XYPolygon

---

## Open Questions

1. **CalcContour Algorithm** - Does ApertureCore need exact equivalent or can we improve?
2. **Performance Requirements** - What are acceptable performance bounds?
3. **Testing Data** - Do we have comprehensive test datasets?
4. **Migration Schedule** - What's the target date for InterfSolver removal?

---

**Created:** 2024  
**Phase:** 6 - Integration  
**Status:** READY TO BEGIN

**Next:** Create ApertureAdapter project and implement type conversions


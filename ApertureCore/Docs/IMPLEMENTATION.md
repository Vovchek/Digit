# ApertureCore Implementation Roadmap

## Project Status

? **Phase 0: Planning & Design** - COMPLETE
- Design document created
- CMake build system configured
- Core headers designed (Point, Bounds, TypeLimits)
- Architecture defined

## Implementation Phases

### Phase 1: Core Geometry (Week 1) ??

#### 1.1 Point Class
- [ ] `src/geometry/Point.cpp` - Stream operators
- [ ] Tests: `tests/geometry/PointTest.cpp`
  - Construction
  - Arithmetic operations
  - Distance calculations
  - Geometric operations (dot, cross, rotate)

#### 1.2 Bounds Class  
- [ ] `src/geometry/Bounds.cpp` - Stream operators, utilities
- [ ] Tests: `tests/geometry/BoundsTest.cpp`
  - Construction variants
  - Property calculations
  - Containment queries
  - Intersection/union operations

#### 1.3 Shape Base Class
- [ ] `include/aperturecore/geometry/Shape.h` - Already designed
- [ ] `src/geometry/Shape.cpp` - Base implementation
- [ ] Default `isVisible()` implementation

#### 1.4 Ellipse Class
- [ ] `include/aperturecore/geometry/Ellipse.h`
- [ ] `src/geometry/Ellipse.cpp`
  - Constructor with rotation
  - `isInside()` using transformation matrix
  - `getBounds()` for rotated ellipse
  - `getContour()` parametric generation
  - Perimeter approximation (Ramanujan)
- [ ] Tests: `tests/geometry/EllipseTest.cpp`
  - All from old XYEllipseTest
  - Rotation correctness
  - Bounds accuracy

#### 1.5 Rectangle Class
- [ ] `include/aperturecore/geometry/Rectangle.h`
- [ ] `src/geometry/Rectangle.cpp`
  - Constructor with rotation
  - `isInside()` with rotation
  - Corner generation
- [ ] Tests: `tests/geometry/RectangleTest.cpp`

#### 1.6 Polygon Class
- [ ] `include/aperturecore/geometry/Polygon.h`
- [ ] `src/geometry/Polygon.cpp`
  - Point-in-polygon test (ray casting)
  - Perimeter calculation
  - Area calculation (shoelace formula)
  - Degeneracy detection
- [ ] Tests: `tests/geometry/PolygonTest.cpp`

**Deliverable:** All geometry tests passing ?

---

### Phase 2: Visibility System (Week 2) ??

#### 2.1 TypeLimits
- [ ] `src/visibility/TypeLimits.cpp` - String conversion
- [ ] Tests: `tests/visibility/TypeLimitsTest.cpp`

#### 2.2 ShapeCollection
- [ ] `include/aperturecore/visibility/ShapeCollection.h`
- [ ] `src/visibility/ShapeCollection.cpp`
  - Separate storage by type
  - Efficient access patterns
- [ ] Tests: `tests/visibility/ShapeCollectionTest.cpp`

#### 2.3 VisibilityChecker - **CORE ALGORITHM**
- [ ] `include/aperturecore/visibility/VisibilityChecker.h`
- [ ] `src/visibility/VisibilityChecker.cpp`
  
**Implementation:**
```cpp
bool VisibilityChecker::isVisible(const Point& point) const {
    stats_.totalChecks++;
    
    // 1. Initial state
    bool visible = shapes_.hasAnyExternal();
    
    // 2. Check EXTERNAL (required intersection)
    for (const auto& shape : shapes_.getExternal()) {
        stats_.externalChecks++;
        if (!shape->isInside(point)) {
            return false;  // Outside any EXTERNAL ? blocked
        }
    }
    
    // 3. Check APERTURE (allow openings)
    for (const auto& shape : shapes_.getApertures()) {
        stats_.apertureChecks++;
        if (shape->isInside(point)) {
            visible = true;  // Inside APERTURE ? force visible
            break;  // Early exit optimization
        }
    }
    
    // 4. Check INTERNAL (final veto)
    for (const auto& shape : shapes_.getInternal()) {
        stats_.internalChecks++;
        if (shape->isInside(point)) {
            return false;  // Inside INTERNAL ? blocked
        }
    }
    
    return visible;
}
```

- [ ] Tests: `tests/visibility/VisibilityCheckerTest.cpp`
  - All old isPupil tests
  - New APERTURE scenarios
  - Edge cases (empty, single shape, etc.)

**Critical Tests:**
```cpp
// 1. No EXTERNAL, with APERTURE
TEST(VisibilityChecker, NoExternalApertureCreatesOpening)

// 2. EXTERNAL with APERTURE extending
TEST(VisibilityChecker, ApertureExtendsExternalBounds)

// 3. APERTURE blocked by INTERNAL
TEST(VisibilityChecker, InternalVetosAperture)

// 4. Multiple APERTUREs (union)
TEST(VisibilityChecker, MultipleAperturesUnion)

// 5. Complex: EXTERNAL + APERTURE + INTERNAL
TEST(VisibilityChecker, ComplexVisibilityScenario)
```

#### 2.4 VisibilityMask
- [ ] `include/aperturecore/visibility/VisibilityMask.h`
- [ ] `src/visibility/VisibilityMask.cpp`
  - Grid construction
  - Batch building from shapes
  - Efficient storage (std::vector<bool>)
  - World ? Grid coordinate conversion
- [ ] Tests: `tests/visibility/VisibilityMaskTest.cpp`

**Deliverable:** Full visibility system working ?

---

### Phase 3: Contour Utilities (Week 3) ?

#### 3.1 Evaluate Need
- [ ] Review if contour generation is still needed
- [ ] Decision: Optional or remove?

If needed:

#### 3.2 ContourExtractor
- [ ] `include/aperturecore/contour/ContourExtractor.h`
- [ ] `src/contour/ContourExtractor.cpp`
  - Marching squares from VisibilityMask
- [ ] Tests: `tests/contour/ContourExtractorTest.cpp`

#### 3.3 SegmentConnector  
- [ ] `include/aperturecore/contour/SegmentConnector.h`
- [ ] `src/contour/SegmentConnector.cpp`
  - Duplicate segment detection (NEW FIX!)
  - Segment connection logic
- [ ] Tests: `tests/contour/SegmentConnectorTest.cpp`

**Deliverable:** Optional contour tools ?

---

### Phase 4: Legacy Compatibility (Week 3-4) ??

#### 4.1 Legacy Adapter
- [ ] `include/aperturecore/legacy/LegacyAdapter.h`
- [ ] `src/legacy/LegacyAdapter.cpp`
  - XYPoint ? Point conversion
  - XYBounds ? Bounds conversion
  - XYEllipse ? Ellipse adapter
  - XYRect ? Rectangle adapter
  - XYPolygon ? Polygon adapter

#### 4.2 Legacy isPupil
- [ ] Wrapper function matching old signature
- [ ] Handle CArray types

```cpp
// Legacy API
extern "C" {
    bool isPupil(const XYPoint& pt, 
                 const CArrayXYEllipse& ellipses,
                 const CArrayXYRect& rects,
                 const CArrayXYPolygon& polygons);
}
```

#### 4.3 Legacy Tests
- [ ] `tests/legacy/BackwardsCompatibilityTest.cpp`
  - All old test cases should pass
  - Identical results to old implementation

**Deliverable:** Drop-in replacement ready ?

---

### Phase 5: Testing & Validation (Week 4) ?

#### 5.1 Integration Tests
- [ ] `tests/integration/ClassicScenariosTest.cpp`
  - All classic aperture + obstruction scenarios
  - Match old behavior exactly

- [ ] `tests/integration/NewApertureScenariosTest.cpp`
  - New APERTURE type scenarios
  - Multi-aperture combinations
  - Edge cases

#### 5.2 Performance Tests
- [ ] `tests/performance/PerformanceTest.cpp`
  - Benchmark single point checks
  - Benchmark batch operations
  - Benchmark mask generation
  - Compare with old implementation

#### 5.3 Stress Tests
- [ ] Large number of shapes (100+ shapes)
- [ ] Large masks (4K resolution)
- [ ] Complex polygons (1000+ vertices)

**Deliverable:** Confidence in correctness & performance ?

---

### Phase 6: Documentation & Examples (Week 5) ??

#### 6.1 API Documentation
- [ ] Doxygen configuration
- [ ] Generate API docs
- [ ] Code examples in headers

#### 6.2 User Guide
- [ ] Migration guide from old code
- [ ] APERTURE type usage examples
- [ ] Common patterns & best practices

#### 6.3 Examples
- [ ] `examples/basic_visibility.cpp`
- [ ] `examples/aperture_system.cpp`
- [ ] `examples/mask_generation.cpp`
- [ ] `examples/legacy_migration.cpp`

**Deliverable:** Complete documentation ?

---

### Phase 7: Production Deployment (Week 6) ??

#### 7.1 Build Configuration
- [ ] Release build optimization
- [ ] DLL export configuration
- [ ] Symbol visibility

#### 7.2 Integration with Digit
- [ ] Build as `InterfSolver.dll` replacement
- [ ] Test in actual Digit application
- [ ] Verify all UI features work

#### 7.3 Validation
- [ ] Load real interferogram data
- [ ] Compare results with old DLL
- [ ] Performance profiling

#### 7.4 Deployment
- [ ] Create installer/package
- [ ] Rollback plan
- [ ] User acceptance testing

**Deliverable:** Production-ready library ?

---

## Quick Start Implementation Order

### Day 1: Foundation
1. Point class + tests
2. Bounds class + tests

### Day 2: Shapes
3. Shape base class
4. Ellipse class + tests

### Day 3: More Shapes
5. Rectangle class + tests
6. Polygon class + tests

### Day 4: Visibility Core
7. TypeLimits + tests
8. ShapeCollection + tests

### Day 5: Visibility Algorithm
9. VisibilityChecker implementation
10. VisibilityChecker comprehensive tests

### Week 2: Remaining Systems
11. VisibilityMask
12. Legacy adapter
13. Integration tests

### Week 3+: Polish & Deploy
14. Performance optimization
15. Documentation
16. Deployment

---

## Testing Strategy

### Unit Tests (Per Component)
- Test each class in isolation
- Mock dependencies where needed
- 100% code coverage goal

### Integration Tests (Cross-Component)
- Test realistic scenarios
- Combine multiple shapes
- Verify complex interactions

### Regression Tests (Legacy Compatibility)
- Every old test case
- Identical numerical results
- Same edge case handling

### Performance Tests (Benchmarks)
- Single operations
- Batch operations
- Memory usage
- Comparison with baseline

---

## Success Criteria

### Functional
- ? All geometry operations correct
- ? Visibility algorithm correct
- ? Backwards compatible
- ? New APERTURE type working

### Quality
- ? 90%+ code coverage
- ? No memory leaks
- ? No compiler warnings
- ? Clean static analysis

### Performance
- ? Faster or equal to old code
- ? Lower memory usage
- ? Scales to large problems

### Usability
- ? Clear API
- ? Good documentation
- ? Easy migration path

---

## Risk Mitigation

### Risk 1: Algorithm Differences
**Mitigation:** Extensive comparison testing with old implementation

### Risk 2: Performance Regression
**Mitigation:** Early benchmarking, profiling, optimization

### Risk 3: Integration Issues
**Mitigation:** Legacy adapter, gradual rollout

### Risk 4: Schedule Slip
**Mitigation:** Prioritize core features, defer nice-to-haves

---

## Next Immediate Steps

1. **Create project structure:**
   ```bash
   mkdir -p ApertureCore/{src,include,tests}/{geometry,visibility,contour,legacy}
   ```

2. **Implement Point class:**
   - Create `src/geometry/Point.cpp`
   - Create `tests/geometry/PointTest.cpp`
   - Verify tests pass

3. **Implement Bounds class:**
   - Create `src/geometry/Bounds.cpp`
   - Create `tests/geometry/BoundsTest.cpp`
   - Verify tests pass

4. **Set up CI/CD:**
   - Configure automated builds
   - Run tests on commit

5. **Begin Shape implementation:**
   - Start with Ellipse (most complex)
   - Verify against old XYEllipse tests

---

## Questions to Resolve

1. **Contour generation:** Still needed? If yes, implement marching squares or keep old algorithm?
   
2. **Performance target:** Match old code or aim for improvement?

3. **C API:** Need C interface for broader compatibility?

4. **Serialization:** Binary format? JSON? Both?

5. **Thread safety:** Make VisibilityChecker thread-safe?

---

## Resources

- **Design:** `ApertureCore/DESIGN.md`
- **README:** `ApertureCore/README.md`
- **This roadmap:** `ApertureCore/IMPLEMENTATION.md`

---

**Ready to start implementation!** ??

Begin with Phase 1.1 (Point class) when ready.

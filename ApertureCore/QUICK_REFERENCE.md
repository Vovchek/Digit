# ApertureCore Quick Reference

## Project Overview

**Purpose:** Modern C++ library for geometric aperture visibility calculations  
**Replaces:** InterfSolver.dll  
**Key Feature:** New `APERTURE` TypeLimits for flexible visibility control  
**Language:** C++17 (no MFC)  
**Build:** CMake  
**Tests:** Google Test  

---

## TypeLimits (3 Types)

| Type | Inside | Outside | Use Case |
|------|--------|---------|----------|
| **EXTERNAL** | ? Visible | ? Blocked | Main aperture/lens |
| **INTERNAL** | ? Blocked | ? Visible | Obstruction/hole |
| **APERTURE** | ? Visible | ?? Unchanged | Local opening |

### Visibility Algorithm

```cpp
visible = hasAnyExternal()
for EXTERNAL: if !inside ? BLOCK
for APERTURE: if inside ? ALLOW
for INTERNAL: if inside ? BLOCK
return visible
```

---

## Core Classes

### Point
```cpp
Point p{x, y};
double d = p.distanceTo(other);
Point rotated = p.rotated(angleRad);
```

### Bounds
```cpp
Bounds b{left, top, right, bottom};
bool contains = b.contains(point);
b.merge(otherBounds);
```

### Shape (abstract)
```cpp
class Ellipse : public Shape {
    bool isInside(Point) override;
    Bounds getBounds() override;
    std::vector<Point> getContour(step) override;
};
```

### ShapeCollection
```cpp
ShapeCollection shapes;
shapes.addExternal(std::make_unique<Ellipse>(...));
shapes.addAperture(std::make_unique<Rectangle>(...));
shapes.addInternal(std::make_unique<Polygon>(...));
```

### VisibilityChecker
```cpp
VisibilityChecker checker(shapes);
bool visible = checker.isVisible(point);
```

### VisibilityMask
```cpp
VisibilityMask mask(bounds, resolution);
mask.buildFrom(shapes);
bool visible = mask.isVisible(x, y);
```

---

## Directory Structure

```
ApertureCore/
??? include/aperturecore/    # Public headers
?   ??? geometry/            # Point, Bounds, Shapes
?   ??? visibility/          # TypeLimits, Checker, Mask
?   ??? legacy/              # Backwards compatibility
??? src/                     # Implementation
??? tests/                   # Google Test suites
??? examples/                # Usage examples
```

---

## Build Commands

```bash
# Configure
cmake -B build -DAPERTURE_BUILD_TESTS=ON

# Build
cmake --build build

# Test
cd build && ctest --verbose

# Install
cmake --install build --prefix /path/to/install
```

---

## Usage Examples

### Basic Visibility Check

```cpp
#include <aperturecore/geometry/Ellipse.h>
#include <aperturecore/visibility/ShapeCollection.h>
#include <aperturecore/visibility/VisibilityChecker.h>

using namespace aperture;

ShapeCollection shapes;

auto aperture = std::make_unique<Ellipse>(10, 8, 0, 0, 0);
aperture->setTypeLimits(TypeLimits::EXTERNAL);
shapes.addExternal(std::move(aperture));

VisibilityChecker checker(shapes);
bool visible = checker.isVisible(Point{5, 0});
```

### APERTURE Type (New!)

```cpp
// No main aperture ? everything invisible
ShapeCollection shapes;

// Add viewing window
auto window = std::make_unique<Ellipse>(5, 5, 10, 10, 0);
window->setTypeLimits(TypeLimits::APERTURE);
shapes.addAperture(std::move(window));

VisibilityChecker checker(shapes);
checker.isVisible(Point{10, 10});  // true (in window)
checker.isVisible(Point{50, 50});  // false (outside)
```

### Visibility Mask

```cpp
Bounds imageBounds{0, 0, 1920, 1080};
VisibilityMask mask(imageBounds, 1.0);  // 1 pixel resolution
mask.buildFrom(shapes);

// Query individual pixels
for (int y = 0; y < 1080; ++y) {
    for (int x = 0; x < 1920; ++x) {
        if (mask.isVisible(x, y)) {
            // Pixel is visible
        }
    }
}
```

---

## Migration from Old Code

### XYPoint ? Point
```cpp
// Old
XYPoint p;
p.X = 10;
p.Y = 5;

// New
Point p{10, 5};
```

### XYEllipse ? Ellipse
```cpp
// Old
XYEllipse e(10, 8, 0, 0, 0, EXTERNAL);

// New
auto e = std::make_unique<Ellipse>(10, 8, 0, 0, 0);
e->setTypeLimits(TypeLimits::EXTERNAL);
```

### isPupil ? VisibilityChecker
```cpp
// Old
bool v = isPupil(point, arrEllipses, arrRects, arrPolygons);

// New
ShapeCollection shapes;
// ... add shapes ...
VisibilityChecker checker(shapes);
bool v = checker.isVisible(point);
```

### buf_line ? VisibilityMask
```cpp
// Old
Init_buf_line(height, width);
// ... complex scan-line logic ...

// New
Bounds bounds{0, 0, width, height};
VisibilityMask mask(bounds, 1.0);
mask.buildFrom(shapes);
bool v = mask.isVisible(x, y);
```

---

## Testing

### Unit Test Example

```cpp
#include <gtest/gtest.h>
#include <aperturecore/geometry/Point.h>

TEST(PointTest, DistanceCalculation) {
    Point p1{0, 0};
    Point p2{3, 4};
    EXPECT_DOUBLE_EQ(p1.distanceTo(p2), 5.0);
}
```

### Visibility Test Example

```cpp
TEST(VisibilityChecker, ApertureCreatesOpening) {
    ShapeCollection shapes;
    
    auto aperture = std::make_unique<Ellipse>(10, 10, 0, 0, 0);
    aperture->setTypeLimits(TypeLimits::APERTURE);
    shapes.addAperture(std::move(aperture));
    
    VisibilityChecker checker(shapes);
    
    EXPECT_TRUE(checker.isVisible(Point{0, 0}));    // inside
    EXPECT_FALSE(checker.isVisible(Point{20, 0}));  // outside
}
```

---

## Implementation Phases

| Phase | Duration | Status | Deliverable |
|-------|----------|--------|-------------|
| 0. Design | 1 day | ? Done | Architecture docs |
| 1. Geometry | 1 week | ?? Next | Point, Bounds, Shapes |
| 2. Visibility | 1 week | ? Planned | Checker, Mask |
| 3. Contour | 1 week | ? Optional | Extractor, Connector |
| 4. Legacy | 1 week | ? Planned | Adapter, compatibility |
| 5. Testing | 1 week | ? Planned | Full test suite |
| 6. Docs | 1 week | ? Planned | API docs, examples |
| 7. Deploy | 1 week | ? Planned | Production ready |

---

## Key Files

| File | Purpose |
|------|---------|
| `DESIGN.md` | Architecture & design decisions |
| `README.md` | User documentation |
| `IMPLEMENTATION.md` | Development roadmap |
| `PROJECT_SUMMARY.md` | Project overview |
| `CMakeLists.txt` | Build configuration |

---

## Common Patterns

### Create Shape with Type

```cpp
auto shape = std::make_unique<Ellipse>(a, b, cx, cy, rot);
shape->setTypeLimits(TypeLimits::EXTERNAL);
shapes.addExternal(std::move(shape));
```

### Batch Visibility Check

```cpp
std::vector<Point> points = {...};
std::vector<bool> visible = checker.checkPoints(points);
```

### Get Bounds of All Shapes

```cpp
Bounds combined = shapes.getCombinedBounds();
```

---

## Performance Tips

1. **Reuse VisibilityChecker** - Don't recreate for every check
2. **Batch operations** - Use `checkPoints()` for many points
3. **VisibilityMask** - For repeated queries at same resolution
4. **Shape ordering** - Put INTERNAL first (early rejection)
5. **Bounds checking** - Quick culling before expensive tests

---

## FAQ

**Q: Why 3 TypeLimits instead of 2?**  
A: `APERTURE` allows creating "openings" without defining global visibility bounds. Classic `EXTERNAL` + `INTERNAL` can't do this.

**Q: Is APERTURE backwards compatible?**  
A: Yes! Old code uses only EXTERNAL/INTERNAL (0/1). New code can use APERTURE (2).

**Q: Performance impact?**  
A: Target is equal or better. Algorithm optimized for early exit.

**Q: Thread-safe?**  
A: `VisibilityChecker` is const-correct and thread-safe for reading. Statistics are mutable but can be made atomic.

**Q: Memory usage?**  
A: STL containers optimize automatically. VisibilityMask uses `std::vector<bool>` (bit-packed).

---

## Support

- **Documentation:** See `README.md`, `DESIGN.md`
- **Examples:** See `examples/` directory
- **Tests:** See `tests/` for usage patterns
- **Issues:** Check existing tests for edge cases

---

## License

Same as parent Digit project.

---

**Quick Start:** Begin with `IMPLEMENTATION.md` Phase 1! ??

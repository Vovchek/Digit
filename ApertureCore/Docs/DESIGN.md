# ApertureCore Library - Modern Refactoring Plan

## Overview

Create a new standalone C++ library (`ApertureCore`) to replace `InterfSolver.dll` with:
- Modern C++ (STL instead of MFC)
- New `APERTURE` TypeLimits
- SOLID principles
- Full test coverage
- Backwards compatibility

---

## New TypeLimits Semantics

### Three Types of Limits

```cpp
enum class TypeLimits {
    EXTERNAL,   // Aperture - only INSIDE is visible (blocks outside)
    INTERNAL,   // Obstruction - INSIDE is blocked (allows outside)
    APERTURE    // Opening - INSIDE is visible, doesn't affect outside
};
```

### Visibility Algorithm (New)

```cpp
bool isVisible(Point pt, Shapes shapes) {
    // 1. Initial state
    bool visible = shapes.hasAnyExternal();  // true if any EXTERNAL exists
    
    // 2. Apply EXTERNAL constraints (apertures)
    for (auto& shape : shapes.getExternal()) {
        if (!shape.isInside(pt)) {
            return false;  // Outside ALL EXTERNAL ? blocked
        }
    }
    
    // 3. Apply APERTURE openings
    bool inAnyAperture = false;
    for (auto& shape : shapes.getApertures()) {
        if (shape.isInside(pt)) {
            inAnyAperture = true;
            break;
        }
    }
    if (inAnyAperture) {
        visible = true;  // Inside APERTURE ? force visible
    }
    
    // 4. Apply INTERNAL obstructions (final veto)
    for (auto& shape : shapes.getInternal()) {
        if (shape.isInside(pt)) {
            return false;  // Inside INTERNAL ? blocked
        }
    }
    
    return visible;
}
```

### Use Cases

**Case 1: Classic aperture with hole**
```cpp
EXTERNAL large_circle;    // Main aperture
INTERNAL small_circle;    // Central obstruction (hole)
// Visible: donut shape
```

**Case 2: Opening within blocked area**
```cpp
// No EXTERNAL ? everything initially invisible
APERTURE window;          // Creates visible opening
// Visible: only inside window
```

**Case 3: Multiple apertures with obstructions**
```cpp
EXTERNAL outer;           // Main boundary
APERTURE opening1;        // Local opening
APERTURE opening2;        // Another opening
INTERNAL block;           // Obstruction in one opening
// Visible: outer ? (opening1 ? opening2) - block
```

---

## Project Structure

```
ApertureCore/
??? CMakeLists.txt
??? include/
?   ??? aperturecore/
?       ??? geometry/
?       ?   ??? Point.h
?       ?   ??? Bounds.h
?       ?   ??? Shape.h
?       ?   ??? Ellipse.h
?       ?   ??? Rectangle.h
?       ?   ??? Polygon.h
?       ?   ??? BrokenLine.h
?       ??? visibility/
?       ?   ??? TypeLimits.h
?       ?   ??? VisibilityChecker.h
?       ?   ??? ShapeCollection.h
?       ?   ??? VisibilityMask.h
?       ??? contour/
?       ?   ??? ContourExtractor.h
?       ?   ??? SegmentConnector.h
?       ??? io/
?           ??? ShapeSerializer.h
?           ??? BoundsIO.h
??? src/
?   ??? geometry/
?   ??? visibility/
?   ??? contour/
?   ??? io/
??? tests/
    ??? geometry/
    ??? visibility/
    ??? contour/
    ??? integration/
```

---

## Core Classes Design

### 1. Point (replaces XYPoint)

```cpp
// include/aperturecore/geometry/Point.h
#pragma once
#include <cmath>
#include <iosfwd>

namespace aperture {

struct Point {
    double x{0.0};
    double y{0.0};
    
    constexpr Point() = default;
    constexpr Point(double x_, double y_) : x(x_), y(y_) {}
    
    // Distance
    double distanceTo(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx*dx + dy*dy);
    }
    
    // Operators
    Point& operator+=(const Point& other);
    Point& operator-=(const Point& other);
    Point operator+(const Point& other) const;
    Point operator-(const Point& other) const;
    Point operator*(double scalar) const;
    
    bool operator==(const Point& other) const;
    bool isNear(const Point& other, double tolerance = 1e-6) const;
};

std::ostream& operator<<(std::ostream& os, const Point& p);

} // namespace aperture
```

### 2. Bounds (replaces XYBounds)

```cpp
// include/aperturecore/geometry/Bounds.h
#pragma once
#include "Point.h"
#include <algorithm>

namespace aperture {

class Bounds {
public:
    double left{0.0};
    double top{0.0};
    double right{0.0};
    double bottom{0.0};
    
    constexpr Bounds() = default;
    constexpr Bounds(double l, double t, double r, double b)
        : left(l), top(t), right(r), bottom(b) {}
    
    // Properties
    bool isEmpty() const {
        return left == 0.0 && top == 0.0 && right == 0.0 && bottom == 0.0;
    }
    
    double width() const { return right - left; }
    double height() const { return bottom - top; }
    Point center() const { return {(left + right)/2, (top + bottom)/2}; }
    
    // Modifications
    void clear();
    void shift(double dx, double dy);
    void expand(const Point& point);
    void merge(const Bounds& other);
    
    // Queries
    bool contains(const Point& point) const;
    bool intersects(const Bounds& other) const;
};

} // namespace aperture
```

### 3. TypeLimits & Shape Base

```cpp
// include/aperturecore/visibility/TypeLimits.h
#pragma once

namespace aperture {

enum class TypeLimits {
    EXTERNAL = 0,  // Aperture - inside visible, blocks outside
    INTERNAL = 1,  // Obstruction - blocks inside, allows outside
    APERTURE = 2   // Opening - inside visible, doesn't affect outside
};

const char* toString(TypeLimits type);
TypeLimits fromString(const char* str);

} // namespace aperture
```

```cpp
// include/aperturecore/geometry/Shape.h
#pragma once
#include "Point.h"
#include "Bounds.h"
#include "../visibility/TypeLimits.h"
#include <vector>
#include <memory>

namespace aperture {

class Shape {
public:
    virtual ~Shape() = default;
    
    // Core interface
    virtual bool isInside(const Point& point) const = 0;
    virtual Bounds getBounds() const = 0;
    virtual std::vector<Point> getContour(double stepSize) const = 0;
    virtual double perimeter() const = 0;
    
    // Type limits
    TypeLimits getTypeLimits() const { return typeLimits_; }
    void setTypeLimits(TypeLimits type) { typeLimits_ = type; }
    
    // Visibility (uses TypeLimits semantics)
    bool isVisible(const Point& point) const;
    
    // Clone
    virtual std::unique_ptr<Shape> clone() const = 0;
    
protected:
    TypeLimits typeLimits_{TypeLimits::EXTERNAL};
};

} // namespace aperture
```

### 4. Concrete Shapes

```cpp
// include/aperturecore/geometry/Ellipse.h
#pragma once
#include "Shape.h"

namespace aperture {

class Ellipse : public Shape {
public:
    Ellipse(double semiMajor, double semiMinor, 
            double centerX, double centerY, 
            double rotationDeg = 0.0);
    
    // Shape interface
    bool isInside(const Point& point) const override;
    Bounds getBounds() const override;
    std::vector<Point> getContour(double stepSize) const override;
    double perimeter() const override;
    std::unique_ptr<Shape> clone() const override;
    
    // Properties
    Point center() const { return center_; }
    double semiMajor() const { return semiMajor_; }
    double semiMinor() const { return semiMinor_; }
    double rotation() const { return rotation_; }
    
private:
    double semiMajor_;
    double semiMinor_;
    Point center_;
    double rotation_;  // radians
    
    // Cached transformation matrix
    void updateTransform();
    double cosR_, sinR_;
};

} // namespace aperture
```

### 5. ShapeCollection

```cpp
// include/aperturecore/visibility/ShapeCollection.h
#pragma once
#include "../geometry/Shape.h"
#include <vector>
#include <memory>

namespace aperture {

class ShapeCollection {
public:
    // Add shapes
    void addShape(std::unique_ptr<Shape> shape);
    void addExternal(std::unique_ptr<Shape> shape);
    void addInternal(std::unique_ptr<Shape> shape);
    void addAperture(std::unique_ptr<Shape> shape);
    
    // Query shapes by type
    const std::vector<std::unique_ptr<Shape>>& getExternal() const;
    const std::vector<std::unique_ptr<Shape>>& getInternal() const;
    const std::vector<std::unique_ptr<Shape>>& getApertures() const;
    
    // Queries
    bool hasAnyExternal() const;
    size_t totalCount() const;
    Bounds getCombinedBounds() const;
    
    // Clear
    void clear();
    
private:
    std::vector<std::unique_ptr<Shape>> external_;
    std::vector<std::unique_ptr<Shape>> internal_;
    std::vector<std::unique_ptr<Shape>> apertures_;
};

} // namespace aperture
```

### 6. VisibilityChecker

```cpp
// include/aperturecore/visibility/VisibilityChecker.h
#pragma once
#include "ShapeCollection.h"
#include "../geometry/Point.h"

namespace aperture {

class VisibilityChecker {
public:
    explicit VisibilityChecker(const ShapeCollection& shapes);
    
    // Check single point
    bool isVisible(const Point& point) const;
    
    // Batch check
    std::vector<bool> checkPoints(const std::vector<Point>& points) const;
    
    // Statistics
    struct Stats {
        size_t totalChecks{0};
        size_t externalChecks{0};
        size_t apertureChecks{0};
        size_t internalChecks{0};
    };
    
    Stats getStats() const { return stats_; }
    void resetStats();
    
private:
    const ShapeCollection& shapes_;
    mutable Stats stats_;
};

} // namespace aperture
```

### 7. VisibilityMask (replaces buf_line)

```cpp
// include/aperturecore/visibility/VisibilityMask.h
#pragma once
#include "ShapeCollection.h"
#include "../geometry/Bounds.h"
#include <vector>
#include <cstdint>

namespace aperture {

class VisibilityMask {
public:
    VisibilityMask(const Bounds& bounds, double resolution);
    
    // Build mask from shapes
    void buildFrom(const ShapeCollection& shapes);
    
    // Query
    bool isVisible(int x, int y) const;
    bool isVisible(const Point& point) const;
    
    // Access
    int width() const { return width_; }
    int height() const { return height_; }
    const Bounds& bounds() const { return bounds_; }
    double resolution() const { return resolution_; }
    
    // Export
    std::vector<uint8_t> toByteArray() const;
    void fromByteArray(const std::vector<uint8_t>& data);
    
    // Statistics
    size_t countVisible() const;
    double fillRatio() const;  // visible / total
    
private:
    Bounds bounds_;
    double resolution_;
    int width_;
    int height_;
    std::vector<bool> mask_;
    
    Point gridToWorld(int x, int y) const;
    std::pair<int, int> worldToGrid(const Point& p) const;
};

} // namespace aperture
```

---

## Backwards Compatibility Layer

### Legacy API Wrapper

```cpp
// include/aperturecore/legacy/LegacyAdapter.h
#pragma once

// Forward declarations for old types
struct XYPoint;
struct XYBounds;
class XYEllipse;
class XYRect;
class XYPolygon;

namespace aperture {
namespace legacy {

// Convert old ? new
Point fromXYPoint(const XYPoint& old);
Bounds fromXYBounds(const XYBounds& old);
std::unique_ptr<Shape> fromXYEllipse(const XYEllipse& old);
std::unique_ptr<Shape> fromXYRect(const XYRect& old);
std::unique_ptr<Shape> fromXYPolygon(const XYPolygon& old);

// Convert new ? old
XYPoint toXYPoint(const Point& p);
XYBounds toXYBounds(const Bounds& b);

// Legacy isPupil implementation
bool isPupil(const Point& pt, const ShapeCollection& shapes);

} // namespace legacy
} // namespace aperture
```

---

## Build System (CMake)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(ApertureCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(APERTURE_BUILD_TESTS "Build tests" ON)
option(APERTURE_BUILD_LEGACY "Build legacy compatibility layer" ON)

# Library
add_library(aperturecore
    # Geometry
    src/geometry/Point.cpp
    src/geometry/Bounds.cpp
    src/geometry/Shape.cpp
    src/geometry/Ellipse.cpp
    src/geometry/Rectangle.cpp
    src/geometry/Polygon.cpp
    src/geometry/BrokenLine.cpp
    
    # Visibility
    src/visibility/TypeLimits.cpp
    src/visibility/ShapeCollection.cpp
    src/visibility/VisibilityChecker.cpp
    src/visibility/VisibilityMask.cpp
    
    # Contour (if needed)
    src/contour/ContourExtractor.cpp
    src/contour/SegmentConnector.cpp
    
    # IO
    src/io/ShapeSerializer.cpp
)

target_include_directories(aperturecore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Legacy adapter
if(APERTURE_BUILD_LEGACY)
    add_library(aperturecore_legacy
        src/legacy/LegacyAdapter.cpp
    )
    target_link_libraries(aperturecore_legacy PUBLIC aperturecore)
endif()

# Tests
if(APERTURE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
install(TARGETS aperturecore
    EXPORT ApertureCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/aperturecore
    DESTINATION include
)
```

---

## Testing Strategy

### Test Structure

```
tests/
??? geometry/
?   ??? PointTest.cpp
?   ??? BoundsTest.cpp
?   ??? EllipseTest.cpp
?   ??? RectangleTest.cpp
?   ??? PolygonTest.cpp
??? visibility/
?   ??? TypeLimitsTest.cpp
?   ??? VisibilityCheckerTest.cpp
?   ??? VisibilityMaskTest.cpp
?   ??? ApertureLogicTest.cpp
??? integration/
?   ??? ClassicScenariosTest.cpp
?   ??? NewApertureScenariosTest.cpp
?   ??? PerformanceTest.cpp
??? legacy/
    ??? BackwardsCompatibilityTest.cpp
```

### Key Test Cases

```cpp
// tests/visibility/ApertureLogicTest.cpp

TEST(ApertureLogic, NoExternalInitiallyInvisible) {
    // No EXTERNAL ? everything invisible
    ShapeCollection shapes;
    shapes.addAperture(makeCircle(10, 0, 0));
    
    VisibilityChecker checker(shapes);
    
    // Inside aperture ? visible
    EXPECT_TRUE(checker.isVisible({0, 0}));
    
    // Outside aperture ? invisible
    EXPECT_FALSE(checker.isVisible({20, 0}));
}

TEST(ApertureLogic, ExternalWithAperture) {
    // EXTERNAL defines base visible area
    // APERTURE creates additional opening
    ShapeCollection shapes;
    shapes.addExternal(makeCircle(20, 0, 0));    // Main aperture
    shapes.addAperture(makeCircle(5, 15, 0));    // Opening outside main
    
    VisibilityChecker checker(shapes);
    
    // Inside EXTERNAL
    EXPECT_TRUE(checker.isVisible({0, 0}));
    
    // Inside APERTURE (outside EXTERNAL)
    EXPECT_TRUE(checker.isVisible({15, 0}));
    
    // Outside both
    EXPECT_FALSE(checker.isVisible({30, 0}));
}

TEST(ApertureLogic, ApertureBlockedByInternal) {
    // INTERNAL has final veto
    ShapeCollection shapes;
    shapes.addAperture(makeCircle(10, 0, 0));
    shapes.addInternal(makeCircle(5, 0, 0));
    
    VisibilityChecker checker(shapes);
    
    // In APERTURE but outside INTERNAL
    EXPECT_TRUE(checker.isVisible({7, 0}));
    
    // In both ? blocked by INTERNAL
    EXPECT_FALSE(checker.isVisible({0, 0}));
}
```

---

## Migration Path

### Phase 1: Core Library (Week 1-2)
1. ? Create project structure
2. ? Implement geometry classes (Point, Bounds, Shape, Ellipse, Rectangle, Polygon)
3. ? Implement TypeLimits enum
4. ? Basic unit tests

### Phase 2: Visibility System (Week 2-3)
1. ? Implement ShapeCollection
2. ? Implement VisibilityChecker with new algorithm
3. ? Implement VisibilityMask
4. ? Comprehensive visibility tests

### Phase 3: Legacy Compatibility (Week 3)
1. ? Create LegacyAdapter
2. ? Test against existing XYShape code
3. ? Ensure isPupil backwards compatibility

### Phase 4: Integration (Week 4)
1. ? Build as DLL/shared library
2. ? Create C API wrapper for easy interop
3. ? Integration tests with existing app
4. ? Performance benchmarks

### Phase 5: Deployment (Week 5)
1. ? Side-by-side testing
2. ? Replace InterfSolver.dll
3. ? Verify all functionality
4. ? Document new APERTURE features

---

## Contour Generation (Evaluation)

### Question: Is contour generation still needed?

**Analysis:**
- **Current use:** CalcContour generates visible boundary polygons
- **With VisibilityMask:** Already have pixel-level visibility
- **Potential uses:**
  - Vector graphics export
  - Shape-based collision detection
  - UI boundary rendering

**Decision:** Keep contour generation as **optional** utility:

```cpp
// include/aperturecore/contour/ContourExtractor.h
namespace aperture {

class ContourExtractor {
public:
    explicit ContourExtractor(const VisibilityMask& mask);
    
    // Extract contours from mask
    std::vector<Polygon> extractContours();
    
    // Extract with simplification
    std::vector<Polygon> extractSimplified(double tolerance);
    
private:
    const VisibilityMask& mask_;
    
    // Marching squares algorithm
    std::vector<Point> marchingSquares();
    void traceContour(int x, int y, std::vector<Point>& contour);
};

} // namespace aperture
```

---

## SOLID Principles Application

### Single Responsibility
- `Point`: Geometric point operations only
- `Shape`: Shape geometry only
- `VisibilityChecker`: Visibility logic only
- `VisibilityMask`: Raster visibility storage only

### Open/Closed
- `Shape` base class: Open for extension (new shapes)
- `VisibilityChecker`: Closed for modification (algorithm stable)

### Liskov Substitution
- All `Shape` derivatives fully interchangeable
- `Ellipse`, `Rectangle`, `Polygon` behave consistently

### Interface Segregation
- Separate interfaces for geometry vs visibility
- Optional contour extraction (not forced on all users)

### Dependency Inversion
- `VisibilityChecker` depends on `ShapeCollection` abstraction
- Shapes don't know about visibility checking

---

## Next Steps

1. **Create project skeleton**
2. **Implement core geometry classes**
3. **Add unit tests**
4. **Implement visibility system**
5. **Add integration tests**
6. **Create legacy adapter**
7. **Performance testing**
8. **Documentation**

Would you like me to start implementing any specific component?

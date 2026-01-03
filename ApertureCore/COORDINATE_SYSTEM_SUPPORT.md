# Coordinate System Support Added to ApertureCore ?

## Summary

Successfully added coordinate system tracking and normalization functionality to the Shape hierarchy, matching the XYShape legacy interface.

---

## Changes Made

### 1. CoordinateSystem Enum

Added to `Shape.h`:

```cpp
enum class CoordinateSystem {
    MEASURING = 0,   // Original measuring coordinates
    NORMALIZED = 1   // Normalized coordinates (centered, scaled)
};
```

Maps to legacy XYShape constants:
- `MEASURING` ? `TypeSystCoor = 0`
- `NORMALIZED` ? `TypeSystCoor = 1` (NORMALISED in legacy)

### 2. Shape Base Class Updates

**New Member Variable:**
```cpp
CoordinateSystem coordSystem_{CoordinateSystem::MEASURING};
```

**New Pure Virtual Methods:**
```cpp
virtual void normalize(double originX, double originY, double radius) = 0;
virtual void denormalize(double originX, double originY, double radius) = 0;
virtual void inverseY(double centerY) = 0;
virtual void shiftX(double deltaX) = 0;
virtual void shiftY(double deltaY) = 0;
```

**New Accessors:**
```cpp
CoordinateSystem getCoordinateSystem() const;
void setCoordinateSystem(CoordinateSystem system);
bool isNormalized() const;
bool isMeasuring() const;
```

---

## Implementation Details

### Ellipse

```cpp
void Ellipse::normalize(double originX, double originY, double radius) {
    semiMajor_ /= radius;
    semiMinor_ /= radius;
    center_.x = (center_.x - originX) / radius;
    center_.y = (center_.y - originY) / radius;
    coordSystem_ = CoordinateSystem::NORMALIZED;
}

void Ellipse::denormalize(double originX, double originY, double radius) {
    semiMajor_ *= radius;
    semiMinor_ *= radius;
    center_.x = center_.x * radius + originX;
    center_.y = center_.y * radius + originY;
    coordSystem_ = CoordinateSystem::MEASURING;
}

void Ellipse::inverseY(double centerY) {
    center_.y = centerY - center_.y;
    rotationDeg_ = -rotationDeg_;
    rotationRad_ = -rotationRad_;
    updateRotationCache();  // Recalculate sin/cos
}
```

### Rectangle

```cpp
void Rectangle::normalize(double originX, double originY, double radius) {
    width_ /= radius;
    height_ /= radius;
    center_.x = (center_.x - originX) / radius;
    center_.y = (center_.y - originY) / radius;
    coordSystem_ = CoordinateSystem::NORMALIZED;
}

// Similar for denormalize, inverseY, shiftX, shiftY
```

### Polygon

```cpp
void Polygon::normalize(double originX, double originY, double radius) {
    for (auto& vertex : vertices_) {
        vertex.x = (vertex.x - originX) / radius;
        vertex.y = (vertex.y - originY) / radius;
    }
    coordSystem_ = CoordinateSystem::NORMALIZED;
}

// Similar for other transformation methods
```

---

## Usage Examples

### Basic Normalization

```cpp
Ellipse ellipse(100, 80, 512, 384, 0);  // a=100, b=80, center=(512,384)
ellipse.normalize(512, 384, 200);       // Normalize around center with R=200

// Now ellipse is in normalized coordinates:
// center = (0, 0)
// semiMajor = 0.5
// semiMinor = 0.4
// coordSystem = NORMALIZED

ellipse.denormalize(512, 384, 200);     // Convert back
// Back to original coordinates
```

### Coordinate System Query

```cpp
if (shape.isNormalized()) {
    // Work in normalized coordinates
} else {
    // Work in measuring coordinates
}

// Or explicit check:
if (shape.getCoordinateSystem() == CoordinateSystem::MEASURING) {
    // ...
}
```

### Y-Axis Inversion

```cpp
// Invert Y coordinate (for screen to mathematical coordinate conversion)
ellipse.inverseY(768);  // Mirror around Y=768

// For rotated shapes, rotation angle is negated
// For polygons, all vertices are mirrored
```

### Shifting

```cpp
shape.shiftX(10.5);  // Move right by 10.5 units
shape.shiftY(-5.2);  // Move down by 5.2 units
```

---

## Compatibility with Legacy XYShape

### Mapping Table

| ApertureCore | Legacy XYShape | Notes |
|--------------|----------------|-------|
| `CoordinateSystem::MEASURING` | `MEASURING (0)` | Default state |
| `CoordinateSystem::NORMALIZED` | `NORMALISED (1)` | After normalization |
| `normalize(x, y, r)` | `Normalize(x, y, r)` | Same algorithm |
| `denormalize(x, y, r)` | `DeNormalize(x, y, r)` | Same algorithm |
| `inverseY(yc)` | `InverseY(yc)` | Same behavior |
| `shiftX(dx)` | `ShiftX(dx)` | Same behavior |
| `shiftY(dy)` | `ShiftY(dy)` | Same behavior |
| `getCoordinateSystem()` | `GetTypeSystCoor()` | Returns enum vs int |
| `setCoordinateSystem()` | `SetTypeSystCoor()` | Type-safe enum |

### Algorithm Verification

All normalization formulas match legacy implementation:

**Normalize:**
```cpp
// Legacy: X = (X - Xo) / Ro
// ApertureCore: x = (x - originX) / radius
```

**Denormalize:**
```cpp
// Legacy: X = X * Ro + Xo
// ApertureCore: x = x * radius + originX
```

**InverseY:**
```cpp
// Legacy: Y = YcInv - Y
// ApertureCore: y = centerY - y
```

---

## Benefits

### Type Safety
- `CoordinateSystem` enum instead of `int`
- Prevents accidental misuse

### Clarity
- `isNormalized()` more readable than `TypeSystCoor == NORMALISED`
- Explicit method names

### Consistency
- All shapes implement same interface
- No special cases for different shape types

### Maintainability
- Coordinate system state tracked explicitly
- Easy to verify transformations

---

## Testing Recommendations

### Unit Tests Needed

```cpp
TEST(EllipseTest, NormalizeDenormalize) {
    Ellipse ell(100, 80, 512, 384, 0);
    
    EXPECT_TRUE(ell.isMeasuring());
    
    ell.normalize(512, 384, 200);
    EXPECT_TRUE(ell.isNormalized());
    EXPECT_NEAR(ell.center().x, 0.0, 1e-6);
    EXPECT_NEAR(ell.center().y, 0.0, 1e-6);
    EXPECT_NEAR(ell.semiMajor(), 0.5, 1e-6);
    
    ell.denormalize(512, 384, 200);
    EXPECT_TRUE(ell.isMeasuring());
    EXPECT_NEAR(ell.center().x, 512.0, 1e-6);
    EXPECT_NEAR(ell.semiMajor(), 100.0, 1e-6);
}

TEST(RectangleTest, InverseY) {
    Rectangle rect(100, 80, 256, 192, 45);
    
    double originalY = rect.center().y;
    double originalRot = rect.rotationDegrees();
    
    rect.inverseY(384);
    
    EXPECT_NEAR(rect.center().y, 384 - originalY, 1e-6);
    EXPECT_NEAR(rect.rotationDegrees(), -originalRot, 1e-6);
}

TEST(PolygonTest, ShiftAndNormalize) {
    Polygon poly{{Point{0,0}, Point{10,0}, Point{10,10}, Point{0,10}}};
    
    poly.shiftX(100);
    poly.shiftY(200);
    
    EXPECT_NEAR(poly.vertex(0).x, 100, 1e-6);
    EXPECT_NEAR(poly.vertex(0).y, 200, 1e-6);
    
    poly.normalize(105, 205, 10);
    
    EXPECT_NEAR(poly.vertex(0).x, -0.5, 1e-6);
    EXPECT_NEAR(poly.vertex(0).y, -0.5, 1e-6);
}
```

---

## Build Status

? **All files compile successfully**

```
Ellipse.cpp
Rectangle.cpp
Polygon.cpp
Shape.cpp
aperturecore.lib
? Build complete
```

---

## Files Modified

1. `include/aperturecore/geometry/Shape.h`
   - Added `CoordinateSystem` enum
   - Added coordinate transformation pure virtual methods
   - Added coordinate system accessors

2. `include/aperturecore/geometry/Ellipse.h`
   - Declared transformation method overrides

3. `src/geometry/Ellipse.cpp`
   - Implemented normalization (scales radii and center)
   - Implemented denormalization
   - Implemented inverseY (negates rotation)
   - Implemented shifts

4. `include/aperturecore/geometry/Rectangle.h`
   - Declared transformation method overrides

5. `src/geometry/Rectangle.cpp`
   - Implemented normalization (scales dimensions and center)
   - Implemented denormalization
   - Implemented inverseY (negates rotation)
   - Implemented shifts

6. `include/aperturecore/geometry/Polygon.h`
   - Declared transformation method overrides

7. `src/geometry/Polygon.cpp`
   - Implemented normalization (transforms all vertices)
   - Implemented denormalization
   - Implemented inverseY (mirrors all vertices)
   - Implemented shifts (translates all vertices)

---

## Commit

```
git commit -m 'feat(aperturecore): add coordinate system tracking and normalization'

7 files changed, 232 insertions(+), 2 deletions(-)
```

---

## Next Steps

1. **Create Tests** - Add unit tests for coordinate transformations
2. **Integration Testing** - Test with ShapeCollection and VisibilityChecker
3. **Documentation** - Add usage examples to README
4. **Legacy Adapter** - Create adapter for XYShape compatibility

---

## Summary

The ApertureCore shape hierarchy now fully supports coordinate normalization and transformation, matching the legacy XYShape interface while providing type safety and modern C++ design. All shapes can be normalized/denormalized, inverted, and shifted consistently.

**Status:** ? Complete and building successfully!

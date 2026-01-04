# Shape Constructor Updates - Test Migration Guide

## ? **What Changed**

All shape constructors now accept optional `TypeLimits`, `SpatialSystem`, and `NormalizationState` parameters with sensible defaults:

### New Constructor Signatures

```cpp
// Ellipse
Ellipse(double semiMajorAxis, double semiMinorAxis,
        double centerX, double centerY,
        double rotationDegrees = 0.0,
        TypeLimits typeLimits = TypeLimits::EXTERNAL,
        CoordinateSystem spatialSystem = CoordinateSystem::screen(),
        NormalizationState normState = NormalizationState::MEASURING);

// Rectangle  
Rectangle(double width, double height,
          double centerX, double centerY,
          double rotationDegrees = 0.0,
          TypeLimits typeLimits = TypeLimits::EXTERNAL,
          CoordinateSystem spatialSystem = CoordinateSystem::screen(),
          NormalizationState normState = NormalizationState::MEASURING);

// Polygon
Polygon(const std::vector<Point>& vertices,
        TypeLimits typeLimits = TypeLimits::EXTERNAL,
        CoordinateSystem spatialSystem = CoordinateSystem::screen(),
        NormalizationState normState = NormalizationState::MEASURING);
```

## ?? **How to Update Tests**

### OLD WAY (with separate addShape calls):

```cpp
// ? OLD - No longer works
shapes.addShape(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0), 
                TypeLimits::EXTERNAL);
```

### NEW WAY (TypeLimits in constructor):

```cpp
// ? NEW - Set TypeLimits in constructor
shapes.addShape(std::make_unique<Ellipse>(
    50.0, 50.0, 100.0, 100.0,  // dimensions and position
    0.0,                         // rotation (default)
    TypeLimits::EXTERNAL         // visibility type
));
```

### Alternative (use convenience methods):

```cpp
// ? ALSO WORKS - Use addExternal/addInternal/addAperture
shapes.addExternal(std::make_unique<Ellipse>(50.0, 50.0, 100.0, 100.0));
shapes.addInternal(std::make_unique<Ellipse>(20.0, 20.0, 100.0, 100.0));
shapes.addAperture(std::make_unique<Rectangle>(10.0, 50.0, 150.0, 100.0));
```

## ?? **Migration Examples**

### Example 1: Simple Ellipse

```cpp
// OLD
shapes.addShape(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0), 
                TypeLimits::EXTERNAL);

// NEW Option 1: Constructor parameter
shapes.addShape(std::make_unique<Ellipse>(
    100.0, 100.0, 0.0, 0.0, 0.0, TypeLimits::EXTERNAL
));

// NEW Option 2: Convenience method
shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 0.0, 0.0));
```

### Example 2: Rectangle with Rotation

```cpp
// OLD
shapes.addShape(std::make_unique<Rectangle>(20.0, 100.0, cx, cy, 45.0), 
                TypeLimits::APERTURE);

// NEW Option 1: Constructor parameter
shapes.addShape(std::make_unique<Rectangle>(
    20.0, 100.0, cx, cy, 45.0, TypeLimits::APERTURE
));

// NEW Option 2: Convenience method
shapes.addAperture(std::make_unique<Rectangle>(20.0, 100.0, cx, cy, 45.0));
```

### Example 3: Polygon

```cpp
// OLD
std::vector<Point> vertices = {{0,0}, {10,0}, {10,10}, {0,10}};
shapes.addShape(std::make_unique<Polygon>(vertices), TypeLimits::INTERNAL);

// NEW Option 1: Constructor parameter
shapes.addShape(std::make_unique<Polygon>(vertices, TypeLimits::INTERNAL));

// NEW Option 2: Convenience method
shapes.addInternal(std::make_unique<Polygon>(vertices));
```

## ?? **Recommended Approach**

For **test files**, use **Option 2 (convenience methods)** - it's clearest:

```cpp
// ? RECOMMENDED for tests - very clear intent
shapes.addExternal(std::make_unique<Ellipse>(100.0, 100.0, 100.0, 100.0));
shapes.addInternal(std::make_unique<Ellipse>(30.0, 30.0, 100.0, 100.0));
shapes.addAperture(std::make_unique<Rectangle>(5.0, 150.0, 100.0, 100.0));
```

For **production code**, use **Option 1 (constructor parameter)** when:
- Loading from files
- Deserializing
- Creating from configuration
- TypeLimits is part of the data

## ?? **Quick Sed Replace Commands**

For batch updating test files:

```bash
# Replace EXTERNAL
sed -i 's/addShape(std::make_unique<Ellipse>(\([^)]*\)), *TypeLimits::EXTERNAL)/addExternal(std::make_unique<Ellipse>(\1))/g' *.cpp

# Replace INTERNAL
sed -i 's/addShape(std::make_unique<Ellipse>(\([^)]*\)), *TypeLimits::INTERNAL)/addInternal(std::make_unique<Ellipse>(\1))/g' *.cpp

# Replace APERTURE
sed -i 's/addShape(std::make_unique<Ellipse>(\([^)]*\)), *TypeLimits::APERTURE)/addAperture(std::make_unique<Ellipse>(\1))/g' *.cpp
```

## ? **Benefits**

1. **Less boilerplate** - No need to call `setTypeLimits()` after construction
2. **Immutable TypeLimits** - Set once at construction
3. **Clearer code** - TypeLimits is part of the shape definition
4. **Better defaults** - Sensible defaults (EXTERNAL, SCREEN, MEASURING)
5. **Easier testing** - Use `addExternal/addInternal/addAperture` for clarity

## ?? **TODO for Test Files**

The following test files need updating:
- ? `VisibilityCheckerTest.cpp` - Use convenience methods
- ? `VisibilityPerformanceTest.cpp` - Use convenience methods

Apply the patterns above to update all `addShape()` calls with TypeLimits parameters.

# Visibility Checker Signature - CONFIRMED

**Date**: After verification with VisibilityMask implementation  
**Decision**: `std::function<bool(int, int)>` - INTEGER COORDINATES  
**Rationale**: Matches VisibilityMask API exactly, no conversion overhead

---

## Confirmation

### VisibilityMask API
```cpp
// From ApertureCore/visibility/VisibilityMask.h
class VisibilityMask {
    bool IsVisible(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return false;
        }
        int index = y * width + x;
        return data[index] != 0;
    }
};
```

**Key facts:**
- ✅ Accepts `int x, int y` parameters
- ✅ Internal indexing: `y * width + x` (integer arithmetic)
- ✅ Returns `bool` (visible/invisible)
- ✅ Bounds checking included

### Algorithm Visibility Checker
```cpp
std::function<bool(int, int)> isVisible;
```

**Perfect match:**
- ✅ Same signature: `bool(int, int)`
- ✅ No coordinate conversion needed
- ✅ Direct pixel indexing (efficiency)
- ✅ Clear semantics: checking discrete pixels

---

## Coordinate Systems in Digitization

### Layer 1: Visibility Checking (INTEGER)
```cpp
// In RedCenterDetector - scanning pixels
for (int y = 0; y < imageHeight; ++y) {
    for (int x = 0; x < imageWidth; ++x) {
        if (input.isVisible(x, y)) {  // Integer coordinates
            // Check this pixel for extremum
            double intensity = GetPixelIntensity(x, y);
            // ...
        }
    }
}
```

**Why integer:**
- Pixels are discrete (can't have pixel 123.45)
- VisibilityMask uses integer indexing
- No conversion overhead
- Semantically clear

### Layer 2: Geometric Calculations (FLOATING-POINT)
```cpp
// In FringeNumberer - numbering fringes
aperture::Point centerFP = input.apertureCenter;  // Point(double, double)

for (const auto& polyline : polylines) {
    double distFromCenter = ComputeDistance(polyline, centerFP);  // FP math
    int fringeNumber = CalculateFringeNumber(distFromCenter);
    // ...
}
```

**Why floating-point:**
- Aperture center may be at sub-pixel coordinates
- Distance calculations use geometry
- Floating-point arithmetic natural for this layer
- Never used for pixel visibility checking

### Layer 3: Downstream Processing (VARIES)
```cpp
// In CDigitInfo::ConvertOutputToLegacyModel()
// If Point(double, double) appears here, that's OK - different concern
// Visibility checking is already done with integer coordinates
```

---

## Why This Is Correct

| Concern | Layer | Type | Reason |
|---------|-------|------|--------|
| **Pixel visibility** | Stage 1-2 | **Integer** | Native to VisibilityMask, pixel coordinates are discrete |
| **Aperture reference** | Stage 3 | **Floating-point** | Geometric center may be sub-pixel |
| **Distance calculations** | Stage 3 | **Floating-point** | Math on geometric coordinates |
| **Downstream storage** | Stage 4+ | Varies | Outside pure algorithm scope |

---

## Implementation Implications

### No Coordinate Conversion Needed
```cpp
// Direct integer pixel checking
if (input.isVisible(pixelX, pixelY)) {  // int, int → bool
    // Safe, efficient, clear
}

// NOT:
// double fx = pixelX + 0.5;
// double fy = pixelY + 0.5;
// if (input.isVisible((int)fx, (int)fy)) { ... }  // Unnecessary!
```

### Signature Lock-In
```cpp
namespace DigitMode::digitization {

// This is the correct and final signature
using VisibilityChecker = std::function<bool(int, int)>;

struct DigitizationInput {
    VisibilityChecker isVisible;
    // aperture::Point apertureCenter;  // Different concern
};

}
```

---

## Testing Implications

### Simple Integer-Based Tests
```cpp
TEST(RedCenterDetectorTest, PixelVisibilityChecking) {
    // Lambda with integer coordinates - natural and clear
    auto isVisible = [](int x, int y) {
        return x >= 10 && x < 246 && y >= 10 && y < 246;
    };
    
    auto extrema = RedCenterDetector::DetectExtrema(
        bitmap, width, height, isVisible, FC_MAX
    );
    
    EXPECT_GT(extrema.size(), 0);
}
```

### VisibilityMask Integration Test
```cpp
TEST(RedCenterDetectorTest, RealVisibilityMask) {
    aperture::visibility::VisibilityMask mask(256, 256);
    // ... fill mask ...
    
    // Direct API wrapping - no conversion
    auto isVisible = [&mask](int x, int y) {
        return mask.IsVisible(x, y);
    };
    
    auto extrema = RedCenterDetector::DetectExtrema(
        bitmap, 256, 256, isVisible, FC_MAX
    );
}
```

---

## Future Considerations

### Template Specialization (If Needed)
```cpp
// For zero-cost specialization, use template
template<typename VisibilityChecker>
class RedCenterDetectorT {
    static std::vector<ExtremumPoint> DetectExtrema(
        const unsigned char* bitmap, int w, int h,
        const VisibilityChecker& isVisible, int mode);
};

// With integer lambda - inlined and optimized
auto isVisibleLambda = [](int x, int y) { return x < 100; };
auto result = RedCenterDetectorT<decltype(isVisibleLambda)>::DetectExtrema(
    bitmap, w, h, isVisibleLambda, FC_MAX
);
```

### Floating-Point Variant (If Needed Later)
```cpp
// Only if sub-pixel visibility checking is needed
// (Not expected for this algorithm)
struct DigitizationInputFP {
    std::function<bool(double, double)> isVisibleFP;
};

// But standard version stays with integers
struct DigitizationInput {
    std::function<bool(int, int)> isVisible;  // Pixels are integers
};
```

---

## Decision Summary

✅ **Visibility checker signature: `std::function<bool(int, int)>`**
- Matches VisibilityMask API directly
- No coordinate conversion overhead
- Clear semantic intent (checking discrete pixels)
- Efficient and maintainable

✅ **Aperture center: `aperture::Point` (floating-point)**
- Separate concern (geometric reference, not pixel checking)
- Used only for fringe numbering calculations
- Never passed to visibility checker

✅ **Coordinate systems are CORRECT and OPTIMAL**
- Integer for pixel operations (natural, efficient)
- Floating-point for geometry (natural, efficient)
- No mixing of concerns

---

## Implementation Checklist

- ✅ Use `std::function<bool(int, int)>` in DigitizationInput
- ✅ RedCenterDetector scans with integer coordinates
- ✅ FringeConnector receives same function signature
- ✅ FringeNumberer uses aperture::Point (floating-point) separately
- ✅ No coordinate conversion needed anywhere
- ✅ Tests use integer-based lambdas
- ✅ VisibilityMask integration is direct (no wrapping needed)

---

**READY FOR IMPLEMENTATION** ✅

# Update: Function-Based Visibility Checking

**Date**: After architectural refinement  
**Change**: Replaced `VisibilityMaskAccessor` wrapper with pure function approach  
**Future**: Template-based approach available for zero-cost optimization

---

## What Changed

### Before (Wrapper Class - Tight Coupling)
```cpp
// Step 2: Created VisibilityMaskAccessor wrapper
class VisibilityMaskAccessor {
    const aperture::visibility::VisibilityMaskProvider& m_maskProvider;
public:
    bool IsVisiblePixel(int x, int y) const;
};

struct DigitizationInput {
    const aperture::visibility::VisibilityMaskProvider* maskProvider;
};

// In RedCenterDetector
VisibilityMaskAccessor accessor(*input.maskProvider);
if (accessor.IsVisiblePixel(x, y)) { ... }
```

**Problems:**
- ❌ Still depends on VisibilityMaskProvider class
- ❌ Wrapper class adds indirection
- ❌ Testing requires mock VisibilityMaskProvider
- ❌ Couples algorithm to specific provider interface

### After (Pure Function - Clean Decoupling)
```cpp
struct DigitizationInput {
    std::function<bool(int, int)> isVisible;  // Pure function signature
};

// In RedCenterDetector
if (input.isVisible(x, y)) { ... }

// In CDigitInfo::Auto()
input.isVisible = [pA](int x, int y) {
    return pA->GetMaskProvider()->IsVisible(x, y);
};
```

**Benefits:**
- ✅ Zero coupling to VisibilityMaskProvider
- ✅ Algorithm receives pure function, not provider
- ✅ Easy to test with lambda: `[](int x, int y) { return true; }`
- ✅ Works with any callable (lambda, function, functor, method)

---

## Architecture Simplification

### Removed
- Step 2: VisibilityMaskAccessor class (NO LONGER NEEDED)
- `GetScanlineVisibilityBounds()` method (can compute directly)
- `HasVisiblePixels()` method (client can implement if needed)

### What Stays
- Pure function signature: `std::function<bool(int, int)> isVisible`
- All other algorithm stages unchanged

---

## Updated Interfaces

### RedCenterDetector
```cpp
class RedCenterDetector {
    static std::vector<ExtremumPoint> DetectExtrema(
        const DigitizationInput& input);  // input.isVisible is used directly
};
```

### FringeConnector
```cpp
class FringeConnector {
    static std::vector<FringePolyline> ConnectExtrema(
        const std::vector<ExtremumPoint>& redCenters,
        const std::function<bool(int, int)>& isVisible);
};
```

### StandardDigitizer
```cpp
auto polylines = FringeConnector::ConnectExtrema(
    redCenters,
    input.isVisible  // Pass function directly
);
```

---

## Testing Benefits

### Before (Wrapper)
```cpp
MockVisibilityMaskProvider mockMask;
VisibilityMaskAccessor accessor(mockMask);
// Complex setup needed
```

### After (Function)
```cpp
auto isVisible = [](int x, int y) { 
    return x >= 10 && x < 246;  // Simple lambda
};
auto result = FringeConnector::ConnectExtrema(extrema, isVisible);
```

**Much cleaner and more intuitive!**

---

## Coordinate System

### Integer Coordinates
The plan assumes **integer pixel coordinates** in the visibility checker:
```cpp
std::function<bool(int x, int y)> isVisible;
```

This is correct for:
- Pixel-by-pixel scanning in RedCenterDetector
- Direct bitmap array indexing
- Integer coordinates from aperture masks

If RedCenterDetector does floating-point sub-pixel analysis, we'd use:
```cpp
std::function<bool(double x, double y)> isVisible;
```

---

## Future: Template Optimization

### Current Approach (Runtime)
```cpp
// std::function has small runtime cost (type erasure, virtual call)
input.isVisible = [pA](int x, int y) { 
    return pA->GetMaskProvider()->IsVisible(x, y);
};
```

### Future Alternative (Compile-Time Zero-Cost)
```cpp
// Template version - inlined, no runtime overhead
template<typename VisibilityChecker>
class RedCenterDetectorT {
    static std::vector<ExtremumPoint> DetectExtrema(
        const unsigned char* bitmap, int w, int h,
        const VisibilityChecker& isVisible, int mode);
};

// Still works same way
auto isVisibleLambda = [pA](int x, int y) { ... };
auto extrema = RedCenterDetectorT<decltype(isVisibleLambda)>::DetectExtrema(
    bitmap, w, h, isVisibleLambda, FC_MAX
);
```

**When to switch**: Only if profiling shows visibility checks are bottleneck (unlikely)

---

## Implementation Checklist

- ✅ Update DigitizationInput structure (use std::function)
- ✅ Remove VisibilityMaskAccessor class (don't create)
- ✅ Update RedCenterDetector to use input.isVisible
- ✅ Update FringeConnector signature
- ✅ Update StandardDigitizer implementation
- ✅ Update Auto() to pass lambda
- ✅ Add template version to Appendix D (for future reference)

---

## Summary

| Aspect | Before | After | Benefit |
|--------|--------|-------|---------|
| **Coupling** | VisibilityMaskProvider | Pure function | Complete decoupling |
| **Wrapper** | VisibilityMaskAccessor class | None needed | Simpler code |
| **Testing** | Mock provider | Lambda | Easier tests |
| **Flexibility** | Fixed to provider interface | Any callable | Universal solution |
| **Runtime cost** | Wrapper method calls | Direct std::function | Same (~one virtual call) |
| **Future optimization** | Limited | Template version | Zero-cost abstraction ready |

---

## Files Updated

✅ `Docs/DIGITIZATION_ALGORITHM_REFACTORING_PLAN.md`
- Step 1: DigitizationInput (use std::function)
- Step 2: Removed (VisibilityMaskAccessor not needed)
- Step 5: RedCenterDetector (updated)
- Step 8: FringeConnector (updated)
- Step 12: StandardDigitizer (updated)
- Step 14: Auto() (updated with lambda)
- **Appendix D**: Template approach (NEW)

---

## Next Steps

1. Confirm RedCenterDetector uses **integer coordinates** (not floating-point)
2. Implement `std::function<bool(int, int)>` approach
3. If performance profiling identifies visibility checks as bottleneck → switch to template version
4. Keep Appendix D as reference for future template optimization

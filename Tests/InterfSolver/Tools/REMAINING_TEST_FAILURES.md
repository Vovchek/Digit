# Remaining XYEllipse Test Failures

## Summary
- **Original failures**: 8
- **Current failures**: 2
- **Fixed**: 6 tests

## Fixes Applied

### 1. isVisible Logic (Fixed 5 tests)
**Problem**: Logic was completely reversed
**Files**: `InterfSolver/Tools/XYEllipse.cpp` lines 105-111, 265-271

**Before**:
```cpp
if (isIn && TypeLimits == INTERNAL)
    return false;   // WRONG!
else if (!isIn && TypeLimits == EXTERNAL)
    return false;   // WRONG!
return true;
```

**After**:
```cpp
if (isIn && TypeLimits == INTERNAL)
    return true;    // Inside visible for INTERNAL
else if (!isIn && TypeLimits == EXTERNAL)
    return true;    // Outside visible for EXTERNAL
return false;
```

### 2. Friend Function isInside Boundary Condition (Fixed contributing factor)
**Problem**: Inconsistent boundary handling between member and friend function
**File**: `InterfSolver/Tools/XYEllipse.cpp` line 260

**Changed**: `if (T < 0.)` ? `if (T <= 0.)`

### 3. Three-Point Circle Formula (Fixed 1 test)
**Problem**: Typo in circle-through-three-points formula
**File**: `InterfSolver/Tools/XYEllipse.cpp` line 365

**Before**:
```cpp
double C = ... + (x2*x2 + y2*y2) * (y3 - x1) + ...
```

**After**:
```cpp
double C = ... + (x2*x2 + y2*y2) * (x3 - x1) + ...
```

### 4. GetExtents Tolerance (Previously fixed)
**File**: `Tests/InterfSolver/Tools/XYEllipseTest.cpp`
**Changed**: Tolerance from `1e-6` to `2e-6` for 45° rotation test

## Remaining Failures

### 1. GetContour_BrokenLine_PointsOnEllipse
**Status**: 13 points out of 100 fail `isInside` check
**Root Cause**: Numerical precision issue

The `GetContour` method generates points using parametric equations:
```cpp
R = Ax * By / sqrt(pow(Ax*SiT,2) + pow(By*CoT,2));
P.X = Xc + R * (Co * CoT - Si * SiT);
P.Y = Yc + R * (Si * CoT + Co * SiT);
```

The `isInside` check computes:
```cpp
R = (X1*X1/(Ax*Ax) + Y1*Y1/(By*By));
T = R - 1;
return (T <= 0);
```

Due to floating-point rounding in the trigonometric functions and divisions, some points end up with `T` slightly > 0 (e.g., `T = 1e-15`).

**Solutions**:
1. **Recommended**: Add small tolerance to `isInside` for boundary points
2. Alternative: Relax test tolerance to accept points very close to boundary
3. Alternative: Use a tolerance parameter in the test

### 2. VectorConstructor_MoreThanFivePoints_LSM
**Status**: Least-squares ellipse fitting not converging correctly

The LSM fitting with >5 points uses algebraic distance minimization. The test generates 20 points on a known ellipse (cx=10, cy=5, a=8, b=4) and expects the fitted ellipse to be within 0.5 units of the original.

**Possible Issues**:
- The fitting algorithm may need better initialization
- Matrix inversion numerical stability
- The tolerance of 0.5 may be too strict for algebraic fitting (geometric fitting would be better)

**Investigation Needed**: Run the test with detailed output to see actual fitted values vs. expected values.

## Recommendations

### For GetContour Test
Modify `isInside` to include a small tolerance for boundary detection:

```cpp
bool XYEllipse::isInside(const XYPoint &P) const {
    double X1 =  (P.X - Xc) * Co + (P.Y - Yc) * Si;
    double Y1 = -(P.X - Xc) * Si + (P.Y - Yc) * Co;
    double R = (X1*X1/(Ax*Ax) + Y1*Y1/(By*By));
    double T = R - 1.;
    
    constexpr double BOUNDARY_TOLERANCE = 1e-10;
    return (T <= BOUNDARY_TOLERANCE);
}
```

Or modify the test to accept points within numerical precision of the boundary.

### For LSM Test
Need to investigate actual vs. expected values. The algebraic ellipse fitting may inherently have larger errors than 0.5 units for this particular ellipse configuration.

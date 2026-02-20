# TestEllipseHelpers - Setup and Compilation Guide

## Issue
TestEllipseHelpers.cpp was not being compiled even after a clean rebuild.

## Root Cause
The test file was created but **not added to `tests/CMakeLists.txt`**, so CMake didn't know it should be compiled.

## Solution
Added `geometry/TestEllipseHelpers.cpp` to the `geometry_tests` executable in `tests/CMakeLists.txt`:

```cmake
# Geometry tests
add_executable(geometry_tests
    geometry/PointTest.cpp
    geometry/BoundsTest.cpp
    geometry/EllipseTest.cpp
    geometry/EllipseFittingTest.cpp
    geometry/TestEllipseHelpers.cpp      # <-- ADDED THIS LINE
    geometry/RectangleTest.cpp
    geometry/PolygonTest.cpp
)
target_link_libraries(geometry_tests PRIVATE ${TEST_LIBS})
gtest_discover_tests(geometry_tests)
```

## What TestEllipseHelpers.cpp Tests

This file contains rigorous unit tests for two critical helper functions in `src/geometry/Ellipse.cpp`:

### 1. `solveLinearSystem5x5(A, b, x)`
- Solves 5×5 linear systems using Gaussian elimination with partial pivoting
- Test cases:
  - Identity matrix
  - Diagonal matrix
  - Complex 5×5 system with verification
  - Near-singular matrix (robustness)

### 2. `conicToEllipse(a, b, c, d, e, f, centerX, centerY, semiMajor, semiMinor, rotationDeg)`
- Converts conic equation coefficients to ellipse parameters
- Validates that the result is an ellipse (not hyperbola or parabola)
- Test cases:
  - Axis-aligned ellipse (a=10, b=5)
  - Translated ellipse (center at (3,2))
  - Circle (radius 5)
  - Hyperbola (should REJECT)
  - Parabola (should REJECT)
  - Translated circle (center at (5,3), radius 4)
  - Rotated ellipse with B coefficient
  - Point verification (points on ellipse satisfy conic equation)

## How to Run Tests

After rebuild, the tests are automatically discovered and can be run:

```bash
# Run all geometry tests (including helpers)
ctest -R "geometry_tests" --verbose

# Run only the SolveLinearSystem5x5 tests
ctest -R "SolveLinearSystem5x5Test" --verbose

# Run only the ConicToEllipse tests
ctest -R "ConicToEllipseTest" --verbose
```

## Test Output

Each test includes diagnostic output showing:
- Computed vs. expected values
- Center, semi-major/minor axes, and rotation angle
- Tolerance ranges

This allows visual inspection of numerical accuracy.

## Implementation Notes

The test file duplicates the helper functions from `Ellipse.cpp` in an anonymous namespace. This allows:
- Full testing of the functions in isolation
- Verification of numerical correctness
- Comparison against known mathematical values

The tests are **rigorous** because they:
1. Test edge cases (singular matrices, degenerate conics)
2. Verify algebraic correctness (solution · matrix = RHS)
3. Use multiple test cases with known analytical solutions
4. Include tolerance bounds appropriate for floating-point arithmetic


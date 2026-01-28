# XYShape Refactoring Plan

## Overview

Refactor `XYEllipse`, `XYRect`, and `XYPolygon` to inherit from abstract base class `XYShape` to eliminate code duplication and establish a common interface.

## Current Problems

### Code Duplication

All three classes (`XYEllipse`, `XYRect`, `XYPolygon`) have duplicate implementations of:

1. **Type Management**:
   ```cpp
   int TypeLimits;
   int TypeSystCoor;
   void SetTypeLimits(int TypLim);
   int GetTypeLimits();
   void SetTypeSystCoor(int _TypeSystCoor);
   int GetTypeSystCoor();
   ```

2. **Visibility Logic** (with BUGS in friend functions):
   ```cpp
   bool isVisible(const XYPoint &P) const;
   bool isVisible(double X, double Y);
   ```

3. **Common Interface**:
   - `isInside()`
   - `GetContour()`
   - `Normalize()`
   - `Perimeter()`

### Inconsistent Implementations

- `isVisible()` has **DIFFERENT implementations** in member vs friend functions
- Friend function `isVisible(XYEllipse)` has **INVERTED logic** (BUG)
- No common interface for polymorphic usage

## Solution: XYShape Base Class

### Class Hierarchy

```
XYShape (abstract)
??? XYEllipse
??? XYRect
??? XYPolygon : public XYBrokenLine
```

### XYShape Responsibilities

#### Protected Members (inherited by all)
```cpp
int TypeLimits;      // EXTERNAL or INTERNAL
int TypeSystCoor;    // MEASURING or NORMALISED
```

#### Concrete Methods (implemented in base)
```cpp
void SetTypeLimits(int TypLim);
int GetTypeLimits() const;
void SetTypeSystCoor(int _TypeSystCoor);
int GetTypeSystCoor() const;
bool isVisible(const XYPoint &P) const;      // Template method
bool isVisible(double X, double Y);           // Template method
```

#### Pure Virtual Methods (must implement in derived)
```cpp
virtual double Perimeter() const = 0;
virtual bool isInside(const XYPoint &P) const = 0;
virtual bool isInside(double X, double Y) = 0;
virtual bool GetContour(XYBrokenLine &BLine, int NFi) const = 0;
virtual bool GetContour(XYBrokenLine &BLine, double Step) const = 0;
virtual bool GetContour(XYPolygon &Plg, int NFi) const = 0;
virtual bool GetContour(XYPolygon &Plg, double Step) const = 0;
virtual void Normalize(double Xo, double Yo, double Ro) = 0;
```

#### Optional Virtual Methods (default implementation)
```cpp
virtual XYBounds GetBounds() const;
virtual void GetBounds(XYBounds &Bnd) const;
```

## Refactoring Steps

### Phase 1: Create Base Class ?

1. ? Create `XYShape.h` with abstract interface
2. ? Create `XYShape.cpp` with common implementations
3. ? Document the CORRECT `isVisible()` logic

### Phase 2: Update XYEllipse

#### Changes to XYEllipse.h

```cpp
#ifndef _XYELLIPSE_H_
#define _XYELLIPSE_H_

#include "XYShape.h"  // NEW: Include base class
#include "XYPoint.h"
#include "XYBounds.h"
// Forward declarations
class XYPolygon;
class XYBrokenLine;

class XYEllipse : public XYShape  // NEW: Inherit from XYShape
{
public:
    // Shape-specific parameters (not in base class)
    double Ax, By;      // Semi-axes
    double Xc, Yc;      // Center
    double Fi;          // Rotation angle
    double Si, Co;      // Cached sin/cos
    
    // REMOVE: TypeLimits, TypeSystCoor (now in base class)

public:
    // Constructors
    XYEllipse(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
              double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYEllipse(const XYEllipse &A);
    XYEllipse(const XYBounds &Bnd, int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYEllipse(const std::vector<XYPoint> &points, int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    virtual ~XYEllipse();  // NEW: virtual destructor
    
    XYEllipse& operator= (const XYEllipse &A);
    void Set(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
             double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    
    // REMOVE: SetTypeLimits, GetTypeLimits, SetTypeSystCoor, GetTypeSystCoor
    // (inherited from XYShape)
    
    // Pure virtual implementations (override from XYShape)
    virtual double Perimeter() const override;
    virtual bool isInside(const XYPoint &P) const override;
    virtual bool isInside(double X, double Y) override;
    virtual bool GetContour(XYBrokenLine &BLine, int NFi) const override;
    virtual bool GetContour(XYBrokenLine &BLine, double Step) const override;
    virtual bool GetContour(XYPolygon &Plg, int NFi) const override;
    virtual bool GetContour(XYPolygon &Plg, double Step) const override;
    virtual void Normalize(double Xo, double Yo, double Ro) override;
    
    // REMOVE or MARK as DEPRECATED: isVisible() methods
    // (inherited correct implementation from XYShape)
    // Keep for compatibility if needed, but mark as using base implementation
    
    // Optional virtual overrides
    virtual XYBounds GetBounds() const override;
    virtual void GetBounds(XYBounds &Bnd) const override;
    
    // Shape-specific methods (not virtual)
    void GetExtents(double &Xmin, double &Ymin, double &Xmax, double &Ymax) const;
    void InverseY(double YcInv);
    void ShiftX(double dX);
    void ShiftY(double dY);
    void DeNormalize(double Xo, double Yo, double Ro);
    
    // DEPRECATED: Friend functions with incorrect isVisible logic
    // These should be removed after updating all call sites
    friend bool isInside(const XYEllipse &Ell, const XYPoint &P);
    friend bool isVisible(const XYEllipse &Ell, const XYPoint &P);  // BUG: Inverted logic!
    friend void GetContour(const XYEllipse &Ell, XYPolygon &Plg, int NFi);
};
#endif
```

#### Changes to XYEllipse.cpp

```cpp
#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYBrokenLine.h"

// Constructor - call base class constructor
XYEllipse::XYEllipse(double _Ax, double _By, double _Xc, double _Yc, double _Fi, 
                     int _TypeLimits, int _TypeSystCoor)
    : XYShape(_TypeLimits, _TypeSystCoor),  // NEW: Initialize base class
      Ax(_Ax), By(_By), Xc(_Xc), Yc(_Yc), Fi(_Fi)
{
    Si = sin(GRD_RD * Fi);
    Co = cos(GRD_RD * Fi);
    // REMOVE: TypeLimits = _TypeLimits; (now in base)
    // REMOVE: TypeSystCoor = _TypeSystCoor; (now in base)
}

// Copy constructor
XYEllipse::XYEllipse(const XYEllipse &A)
    : XYShape(A.TypeLimits, A.TypeSystCoor),  // NEW: Copy base members
      Ax(A.Ax), By(A.By), Xc(A.Xc), Yc(A.Yc), Fi(A.Fi), Si(A.Si), Co(A.Co)
{
}

// Assignment operator
XYEllipse& XYEllipse::operator=(const XYEllipse &A)
{
    if (this != &A) {
        // Copy base class members
        XYShape::operator=(A);  // NEW: Call base assignment if implemented
        // Or manually:
        TypeLimits = A.TypeLimits;
        TypeSystCoor = A.TypeSystCoor;
        
        // Copy derived class members
        Ax = A.Ax;
        By = A.By;
        Xc = A.Xc;
        Yc = A.Yc;
        Fi = A.Fi;
        Si = A.Si;
        Co = A.Co;
    }
    return *this;
}

// Virtual method implementations - add 'override' keyword
double XYEllipse::Perimeter() const  // Already correct
{
    constexpr double dPI = 3.14159265358979323846;
    double Perim = dPI * (1.5 * (Ax + By) - sqrt(Ax * By));
    return Perim;
}

bool XYEllipse::isInside(const XYPoint &P) const  // Already correct
{
    double X1 =  (P.X - Xc) * Co + (P.Y - Yc) * Si;
    double Y1 = -(P.X - Xc) * Si + (P.Y - Yc) * Co;
    double R = (X1 * X1 / (Ax * Ax) + Y1 * Y1 / (By * By));
    double T = R - 1.;
    
    if (T <= HIGH_PRECISION)
        return true;
    else if (T > 0.)
        return false;
    
    return true;
}

bool XYEllipse::isInside(double X, double Y)  // Already correct
{
    XYPoint P(X, Y);
    return isInside(P);
}

// REMOVE: isVisible() implementations (use base class)
// The base class provides CORRECT implementation using template method pattern

bool XYEllipse::GetContour(XYBrokenLine &BLine, int NFi) const  // Already correct
{
    // ... existing implementation ...
}

// ... rest of implementations unchanged ...

void XYEllipse::Normalize(double Xo, double Yo, double Ro)  // Already correct
{
    Ax /= Ro;
    By /= Ro;
    Xc = (Xc - Xo) / Ro;
    Yc = (Yc - Yo) / Ro;
    TypeSystCoor = NORMALISED;  // Base class member
}

// DEPRECATED friend functions - mark for removal
// These have INCORRECT isVisible logic!

bool isInside(const XYEllipse &Ell, const XYPoint &P)
{
    // This is correct, but prefer member function
    return Ell.isInside(P);
}

bool isVisible(const XYEllipse &Ell, const XYPoint &P)
{
    // ? BUG: This has INVERTED logic!
    // Should use: return Ell.isVisible(P);
    // But that would give DIFFERENT behavior
    
    // Current INCORRECT implementation:
    bool isIn = isInside(Ell, P);
    if (isIn && Ell.TypeLimits == INTERNAL)
        return true;   // WRONG: should return false
    else if (!isIn && Ell.TypeLimits == EXTERNAL)
        return true;   // WRONG: should return false
    return false;      // WRONG: should return true
}
```

### Phase 3: Update XYRect

Similar changes to `XYRect.h` and `XYRect.cpp`:
- Inherit from `XYShape`
- Remove duplicate `TypeLimits`, `TypeSystCoor` members
- Remove `SetTypeLimits`, `GetTypeLimits`, etc. (use inherited)
- Remove `isVisible()` (use base class implementation)
- Mark methods as `override`
- Update constructors to call base class
- Fix friend functions or mark as deprecated

### Phase 4: Update XYPolygon

**Special case**: `XYPolygon` currently inherits from `XYBrokenLine`. We need to decide:

#### Option A: Multiple Inheritance
```cpp
class XYPolygon : public XYBrokenLine, public XYShape
```
**Pros**: Maintains existing inheritance from `XYBrokenLine`
**Cons**: Diamond problem potential, more complex

#### Option B: Composition
```cpp
class XYPolygon : public XYShape
{
protected:
    XYBrokenLine points;  // Composition instead of inheritance
```
**Pros**: Cleaner design, no diamond problem
**Cons**: Breaks existing code that depends on `XYPolygon` being a `XYBrokenLine`

#### Option C: Keep Current + Interface
```cpp
class XYPolygon : public XYBrokenLine  // Keep existing
{
protected:
    int TypeLimits;      // Keep (can't remove without breaking)
    int TypeSystCoor;
    
    // Implement XYShape interface without formal inheritance
```
**Pros**: No breaking changes
**Cons**: Doesn't eliminate duplication

**Recommended**: **Option A** with proper interface segregation

### Phase 5: Update Call Sites

1. **Search for friend function usage**:
   ```cpp
   // Find all calls to:
   isInside(ellipse, point)
   isVisible(ellipse, point)  // BUG: Inverted logic!
   isInside(rect, point)
   isVisible(rect, point)     // BUG: Inverted logic!
   ```

2. **Replace with member calls**:
   ```cpp
   // Change from:
   if (isVisible(ellipse, point))
   
   // To:
   if (ellipse.isVisible(point))
   ```

3. **Critical**: Check `isPupil()` implementations
   - Ensure they use MEMBER functions, not friend functions
   - Friend functions have INVERTED logic!

### Phase 6: Enable Polymorphism

After refactoring, we can use polymorphic containers:

```cpp
// NEW capability: Polymorphic shape handling
std::vector<std::unique_ptr<XYShape>> shapes;
shapes.push_back(std::make_unique<XYEllipse>(...));
shapes.push_back(std::make_unique<XYRect>(...));
shapes.push_back(std::make_unique<XYPolygon>(...));

// Polymorphic usage
for (const auto& shape : shapes) {
    if (shape->isVisible(point)) {
        // ...
    }
}
```

This would allow refactoring `isPupil()` to:
```cpp
bool isPupil(const XYPoint& P, const std::vector<XYShape*>& shapes)
{
    for (const auto* shape : shapes) {
        if (!shape->isVisible(P)) {
            return false;  // Point is blocked
        }
    }
    return true;  // Point is visible through all shapes
}
```

## Benefits

### 1. Eliminates Code Duplication
- ? Common type management in one place
- ? Single `isVisible()` implementation (correct logic)
- ? Consistent interface across all shapes

### 2. Fixes Bugs
- ? Removes friend functions with inverted `isVisible()` logic
- ? Ensures all shapes use same (correct) visibility logic
- ? Eliminates inconsistencies between member and friend functions

### 3. Improves Maintainability
- ? Changes to common logic only need to be made once
- ? Clear separation of concerns (base vs derived)
- ? Easier to add new shape types

### 4. Enables Polymorphism
- ? Can treat all shapes uniformly
- ? Simplifies isPupil() and CalcContour()
- ? Better testability (mock shapes)

### 5. Better Documentation
- ? Clear contract (pure virtuals)
- ? Template method pattern for visibility
- ? Self-documenting code

## Testing Strategy

### Unit Tests

1. **Test each derived class individually**:
   - Verify all virtual methods work correctly
   - Test visibility logic with all TypeLimits combinations
   - Ensure backward compatibility

2. **Test polymorphic usage**:
   - Create shapes via base pointers
   - Call virtual methods polymorphically
   - Verify correct dispatch

3. **Test base class directly**:
   - Cannot instantiate (abstract)
   - But can test via derived classes

### Integration Tests

1. **isPupil() tests**:
   - Ensure refactored code passes existing tests
   - Add tests for polymorphic usage

2. **CalcContour() tests**:
   - Verify no regressions
   - Test with mixed shape types

### Regression Tests

1. **Run full test suite**:
   - All existing tests must pass
   - No behavioral changes

2. **Visual verification**:
   - Load saved files
   - Verify contours render correctly
   - Check pupil calculation results

## Migration Path

### Step 1: Add Base Class (Non-Breaking)
- ? Create `XYShape.h` and `XYShape.cpp`
- ? Add to build system
- ? No changes to existing code yet

### Step 2: Update One Class at a Time

**Order**: XYRect ? XYEllipse ? XYPolygon
(XYRect is simplest, XYPolygon is most complex)

For each class:
1. Update header to inherit from `XYShape`
2. Remove duplicate members
3. Update constructors
4. Mark virtual methods
5. Compile and fix errors
6. Run tests
7. Fix any issues
8. Commit

### Step 3: Update Call Sites
1. Find all friend function calls
2. Replace with member function calls
3. Test each change
4. Commit incrementally

### Step 4: Remove Deprecated Code
1. Remove friend functions
2. Remove any compatibility shims
3. Final cleanup
4. Commit

### Step 5: Refactor for Polymorphism
1. Update isPupil() to use polymorphism
2. Update CalcContour() to use polymorphism
3. Consider std::vector<XYShape*> vs CArrays
4. Commit

## Risks and Mitigation

### Risk 1: Breaking Changes
**Mitigation**: Incremental changes, keep friend functions temporarily, extensive testing

### Risk 2: XYPolygon Multiple Inheritance
**Mitigation**: Careful design, clear documentation, thorough testing

### Risk 3: Performance Impact
**Mitigation**: Virtual function overhead is negligible for this use case, profile if concerned

### Risk 4: Serialization
**Mitigation**: Ensure saved files can still be loaded, test with real data

## Files to Modify

### New Files
- ? `InterfSolver/Tools/XYShape.h`
- ? `InterfSolver/Tools/XYShape.cpp`

### Modified Files
- `InterfSolver/Tools/XYEllipse.h`
- `InterfSolver/Tools/XYEllipse.cpp`
- `InterfSolver/Tools/XYRect.h`
- `InterfSolver/Tools/XYRect.cpp`
- `InterfSolver/Tools/XYPolygon.h`
- `InterfSolver/Tools/XYPolygon.cpp`
- `InterfSolver/Tools/isPupil.cpp` (update friend function calls)
- `InterfSolver/Tools/CalcContour.cpp` (potential polymorphic refactor)
- Any other files using friend functions

### Test Files
- Create `Tests/InterfSolver/Tools/XYShapeTest.cpp`
- Update `Tests/InterfSolver/Tools/XYEllipseTest.cpp`
- Update `Tests/InterfSolver/Tools/isPupilTest.cpp`
- Update `Tests/InterfSolver/Tools/CalcContourTest.cpp`

## Success Criteria

? All existing tests pass
? No changes to calculated results (bit-for-bit compatibility)
? Code coverage maintained or improved
? Build completes without warnings
? Documentation updated
? Can load and process existing saved files
? No memory leaks
? Performance not degraded

## Timeline Estimate

- **Phase 1**: Create base class - ? DONE (1 hour)
- **Phase 2**: Update XYEllipse - 2-3 hours
- **Phase 3**: Update XYRect - 2 hours
- **Phase 4**: Update XYPolygon - 3-4 hours
- **Phase 5**: Update call sites - 2-3 hours
- **Phase 6**: Enable polymorphism - 2-3 hours
- **Testing**: 4-6 hours
- **Documentation**: 2 hours

**Total**: ~20-25 hours

## Next Steps

1. ? Review this plan
2. ? Create XYShape.h and XYShape.cpp
3. ?? Start with XYRect refactoring (simplest case)
4. ?? Proceed to XYEllipse
5. ?? Finally XYPolygon (most complex)
6. ?? Update call sites
7. ?? Enable polymorphic usage

## References

- **Design Patterns**: Template Method, Strategy
- **SOLID Principles**: Interface Segregation, Liskov Substitution
- **C++ Best Practices**: Virtual destructors, override keyword

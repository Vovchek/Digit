# ?? Quick Start: Implement isPupil() Optimization

## Step 1: Add Bounding Box Caching (15 minutes)

### Edit `InterfSolver/Tools/XYPolygon.h`

```cpp
class XYPolygon : public XYBrokenLine
{
protected:
    int TypeLimits;
    int TypeSystCoor;
    
    // ? ADD THESE:
    mutable XYBounds m_CachedBounds;
    mutable bool m_BoundsCached;
    
public:
    XYPolygon();
    // ... existing methods ...
    
    // ? ADD THESE:
private:
    void CacheBounds() const;
    void InvalidateBoundsCache();
};
```

### Edit `InterfSolver/Tools/XYPolygon.cpp`

```cpp
// ? ADD to constructor
XYPolygon::XYPolygon()
{
    TypeLimits = EXTERNAL;
    TypeSystCoor = MEASURING;
    m_BoundsCached = false;  // ? ADD THIS
}

// ? ADD to copy constructor
XYPolygon::XYPolygon(const XYPolygon &A) : XYBrokenLine (A)
{
    TypeLimits = A.TypeLimits;
    TypeSystCoor = A.TypeSystCoor;
    m_BoundsCached = false;  // ? ADD THIS (don't copy cache)
}

// ? ADD to ALL constructors that modify ArrPnt
// (Add m_BoundsCached = false; to each)

// ? ADD these new methods at end of file:

void XYPolygon::CacheBounds() const
{
    int NPnt = GetSize();
    if (NPnt == 0) {
        m_BoundsCached = true;
        return;
    }
    
    m_CachedBounds.XLeft = m_CachedBounds.XRight = ArrPnt[0].X;
    m_CachedBounds.YTop = m_CachedBounds.YBottom = ArrPnt[0].Y;
    
    for (int i = 1; i < NPnt; i++) {
        if (ArrPnt[i].X < m_CachedBounds.XLeft)   m_CachedBounds.XLeft = ArrPnt[i].X;
        if (ArrPnt[i].X > m_CachedBounds.XRight)  m_CachedBounds.XRight = ArrPnt[i].X;
        if (ArrPnt[i].Y < m_CachedBounds.YTop)    m_CachedBounds.YTop = ArrPnt[i].Y;
        if (ArrPnt[i].Y > m_CachedBounds.YBottom) m_CachedBounds.YBottom = ArrPnt[i].Y;
    }
    
    m_BoundsCached = true;
}

void XYPolygon::InvalidateBoundsCache()
{
    m_BoundsCached = false;
}

// ? MODIFY isInside() - ADD BOUNDING BOX CHECK AT START:

bool XYPolygon::isInside(const XYPoint &P) const
{
    // ? ADD THIS BLOCK:
    // Bounding box pre-check
    if (!m_BoundsCached) {
        CacheBounds();
    }
    
    if (P.X < m_CachedBounds.XLeft || P.X > m_CachedBounds.XRight ||
        P.Y < m_CachedBounds.YTop || P.Y > m_CachedBounds.YBottom) {
        return false;  // Outside bounding box ? definitely outside polygon
    }
    
    // ? EXISTING CODE BELOW (winding number algorithm)
    int i;
    double Phi = 0.;
    int NPnt = GetSize();
    for (i = 0; i < NPnt - 1; i++)
        Phi += Angle(ArrPnt[i+1]-P, ArrPnt[i]-P);
    if (fabs(Phi) > 6.28)
        return true;
    else if (fabs(Phi) < 0.0001)
        return false;
    return true;
}
```

**Test It**:
```bash
# Build and run
cmake --build . --config Debug
./Tests --gtest_filter=IsPupilTest.*

# Should see ~10x speedup in benchmark!
```

---

## Step 2: Replace with Ray Casting (30 minutes)

### Edit `InterfSolver/Tools/XYPolygon.cpp`

```cpp
// ? REPLACE entire isInside() method:

bool XYPolygon::isInside(const XYPoint &P) const
{
    // Bounding box pre-check (from Step 1)
    if (!m_BoundsCached) {
        CacheBounds();
    }
    
    if (P.X < m_CachedBounds.XLeft || P.X > m_CachedBounds.XRight ||
        P.Y < m_CachedBounds.YTop || P.Y > m_CachedBounds.YBottom) {
        return false;
    }
    
    // ? REPLACE winding number with ray casting:
    int NPnt = GetSize();
    int crossings = 0;
    
    for (int i = 0, j = NPnt - 1; i < NPnt; j = i++) {
        const XYPoint& vi = ArrPnt[i];
        const XYPoint& vj = ArrPnt[j];
        
        // Does edge [vj, vi] cross horizontal ray from P to +infinity?
        if (((vi.Y > P.Y) != (vj.Y > P.Y)) &&
            (P.X < (vj.X - vi.X) * (P.Y - vi.Y) / (vj.Y - vi.Y) + vi.X)) {
            crossings++;
        }
    }
    
    // Odd number of crossings = inside
    return (crossings & 1) == 1;
}
```

**Test It**:
```bash
# Re-run tests
./Tests --gtest_filter=IsPupilTest.*

# Should see ~50-100x total speedup!
```

---

## Step 3: Update Global isInside() Function

```cpp
// ? UPDATE global isInside() to match:

bool isInside(const XYPolygon &Plg, const XYPoint &P)
{
    // Just delegate to member function (it has the optimizations)
    return Plg.isInside(P);
}
```

---

## Verification Checklist

- [ ] All tests pass (`IsPupilTest.*`)
- [ ] Performance benchmark shows **50-100x improvement**
- [ ] `CreateRedCenters()` runs in **<1 second** instead of 25s
- [ ] No memory leaks (run with valgrind/ASAN)

---

## Expected Results

### Before Optimization
```
Benchmark_SinglePolygon_100kPoints: 25000 ms
Benchmark_MultipleShapes_10kPoints: 2500 ms
CreateRedCenters() for 1000x1000 image: ~25 seconds
```

### After Step 1 (Bounding Box)
```
Benchmark_SinglePolygon_100kPoints: 2500 ms   (10x faster)
Benchmark_MultipleShapes_10kPoints: 250 ms    (10x faster)
CreateRedCenters() for 1000x1000 image: ~2.5 seconds
```

### After Step 2 (Ray Casting)
```
Benchmark_SinglePolygon_100kPoints: 250 ms    (100x faster!)
Benchmark_MultipleShapes_10kPoints: 25 ms     (100x faster!)
CreateRedCenters() for 1000x1000 image: ~0.25 seconds ??
```

---

## Troubleshooting

### If tests fail after Step 2:
- Check ray casting logic (edge cases at polygon boundaries)
- Verify `crossings & 1` (odd/even test)
- Test with simple polygon first (square)

### If still slow:
- Profile to verify polygon test is still the bottleneck
- Consider Step 3 (spatial grid) from main guide
- Check if `CacheBounds()` is being called too often

---

## Next Steps (Optional)

If 100x isn't enough, see `ISPUPIL_OPTIMIZATION_GUIDE.md` for:
- Global bounding box caching
- Spatial grid acceleration
- OpenMP parallelization

---

## Quick Reference

**Files to Edit**:
1. `InterfSolver/Tools/XYPolygon.h` - Add member variables
2. `InterfSolver/Tools/XYPolygon.cpp` - Implement optimizations

**Time Required**:
- Step 1: 15 minutes
- Step 2: 30 minutes
- **Total**: ~45 minutes for **100x speedup!** ??

---

**GO!** ??

# ?? isPupil() Performance Optimization Analysis

## ?? Current Performance Problem

### **Bottleneck Identified**

In `CreateRedCenters()`, for each pixel (potentially 1000x1000 = 1M pixels), we call:
```cpp
isPupil(P, pB->ArrContour)  // Called 1,000,000 times!
```

For each call:
1. Loops through all polygons in `ArrContour` (typically 1-10)
2. For each polygon, calls `XYPolygon::isInside(P)`
3. `isInside()` uses **winding number algorithm** with `atan2()` calls

**Cost per pixel**:
- For 1 polygon with 500 points: **500 `Angle()` calls** ? **500 `atan2()` calls**
- `atan2()` is **~100x slower** than basic arithmetic

**Total cost**:
- 1M pixels ? 500 atan2() calls = **500 million atan2() calls!**
- At ~50ns per atan2(), this takes **~25 seconds** just for trig!

---

## ?? Optimization Strategies

### **Strategy 1: Bounding Box Pre-Check** ?????
**Impact**: ?? **HUGE** (90-95% reduction in polygon tests)

#### Problem
Currently, every pixel tests against the full polygon, even if it's nowhere near it.

#### Solution
Add bounding box check **before** expensive `isInside()` call:

```cpp
bool XYPolygon::isInside(const XYPoint &P) const
{
    // ? OPTIMIZATION 1: Bounding box pre-check
    if (!m_BoundsCached) {
        CacheBounds();  // Compute once, cache forever
    }
    
    // Quick reject: point outside bounding box ? definitely outside polygon
    if (P.X < m_Bounds.XLeft || P.X > m_Bounds.XRight ||
        P.Y < m_Bounds.YTop || P.Y > m_Bounds.YBottom) {
        return false;  // ? FAST EXIT (4 comparisons instead of 500 atan2 calls!)
    }
    
    // Only now do expensive winding number test
    int NPnt = GetSize();
    double Phi = 0.;
    for (int i = 0; i < NPnt - 1; i++)
        Phi += Angle(ArrPnt[i+1]-P, ArrPnt[i]-P);
    
    return (fabs(Phi) > 6.28);
}
```

**Implementation**:
```cpp
// In XYPolygon.h
class XYPolygon : public XYBrokenLine {
private:
    mutable XYBounds m_Bounds;          // Cached bounding box
    mutable bool m_BoundsCached = false; // Cache validity flag
    
    void CacheBounds() const {
        int NPnt = GetSize();
        if (NPnt == 0) return;
        
        m_Bounds.XLeft = m_Bounds.XRight = ArrPnt[0].X;
        m_Bounds.YTop = m_Bounds.YBottom = ArrPnt[0].Y;
        
        for (int i = 1; i < NPnt; i++) {
            if (ArrPnt[i].X < m_Bounds.XLeft)   m_Bounds.XLeft = ArrPnt[i].X;
            if (ArrPnt[i].X > m_Bounds.XRight)  m_Bounds.XRight = ArrPnt[i].X;
            if (ArrPnt[i].Y < m_Bounds.YTop)    m_Bounds.YTop = ArrPnt[i].Y;
            if (ArrPnt[i].Y > m_Bounds.YBottom) m_Bounds.YBottom = ArrPnt[i].Y;
        }
        
        m_BoundsCached = true;
    }
    
    void InvalidateCache() const {
        m_BoundsCached = false;
    }
};
```

**Expected Speedup**: **10-50x** (most pixels are outside bounding box!)

---

### **Strategy 2: Replace Winding Number with Ray Casting** ????
**Impact**: ?? **VERY HIGH** (10-20x faster for inside tests)

#### Problem
Current winding number algorithm:
```cpp
for (int i = 0; i < NPnt - 1; i++)
    Phi += Angle(ArrPnt[i+1]-P, ArrPnt[i]-P);  // ? 500 atan2() calls!
```

#### Solution
Use **ray casting algorithm** (no trig needed!):

```cpp
bool XYPolygon::isInside(const XYPoint &P) const
{
    // Bounding box check first
    if (!m_BoundsCached) CacheBounds();
    if (P.X < m_Bounds.XLeft || P.X > m_Bounds.XRight ||
        P.Y < m_Bounds.YTop || P.Y > m_Bounds.YBottom) {
        return false;
    }
    
    // Ray casting algorithm (cast ray from P to infinity along +X axis)
    int NPnt = GetSize();
    int crossings = 0;
    
    for (int i = 0, j = NPnt - 1; i < NPnt; j = i++) {
        const XYPoint& vi = ArrPnt[i];
        const XYPoint& vj = ArrPnt[j];
        
        // Check if edge crosses horizontal ray from P
        if (((vi.Y > P.Y) != (vj.Y > P.Y)) &&
            (P.X < (vj.X - vi.X) * (P.Y - vi.Y) / (vj.Y - vi.Y) + vi.X)) {
            crossings++;
        }
    }
    
    return (crossings % 2) == 1;  // Odd crossings = inside
}
```

**Why It's Faster**:
- ? OLD: 500 `atan2()` calls ? **~25,000ns**
- ? NEW: 500 comparisons + simple arithmetic ? **~500ns**

**Expected Speedup**: **50x** for points inside bounding box!

---

### **Strategy 3: Cache `ArrContour` Bounding Box** ???
**Impact**: ?? **MEDIUM** (2-5x for multi-contour scenes)

#### Problem
When multiple contours exist, we test all of them even if point is far away.

#### Solution
```cpp
// In CBoundCtrls
class CBoundCtrls {
private:
    mutable XYBounds m_ContourBounds;   // Bounding box of ALL contours
    mutable bool m_ContourBoundsCached = false;
    
public:
    const XYBounds& GetContourBounds() const {
        if (!m_ContourBoundsCached) {
            CacheContourBounds();
        }
        return m_ContourBounds;
    }
    
    void CacheContourBounds() const {
        if (ArrContour.GetSize() == 0) return;
        
        m_ContourBounds = ArrContour[0].GetBounds();
        for (int i = 1; i < ArrContour.GetSize(); i++) {
            XYBounds b = ArrContour[i].GetBounds();
            if (b.XLeft < m_ContourBounds.XLeft) m_ContourBounds.XLeft = b.XLeft;
            if (b.XRight > m_ContourBounds.XRight) m_ContourBounds.XRight = b.XRight;
            if (b.YTop < m_ContourBounds.YTop) m_ContourBounds.YTop = b.YTop;
            if (b.YBottom > m_ContourBounds.YBottom) m_ContourBounds.YBottom = b.YBottom;
        }
        m_ContourBoundsCached = true;
    }
};

// In CreateRedCenters()
XYBounds globalBounds = pB->GetContourBounds();

for (int iy = begY; iy < endY; iy++) {
    for (auto iCol = 0; iCol < xDIB; iCol++) {
        // ...
        if (useComplexMasking) {
            P.X = iCol; P.Y = iy;
            
            // Global bounding box check
            if (P.X < globalBounds.XLeft || P.X > globalBounds.XRight ||
                P.Y < globalBounds.YTop || P.Y > globalBounds.YBottom) {
                line[n] = inv_line[n] = 0;  // Outside all contours
            }
            else if (isPupil(P, pB->ArrContour)) {
                line[n] = inv_line[n] = rgbPix.rgbRed;
            }
            else {
                line[n] = inv_line[n] = 0;
            }
        }
    }
}
```

---

### **Strategy 4: Spatial Indexing (Grid Acceleration)** ?????
**Impact**: ?? **MASSIVE** (100-1000x for complex polygons)

#### Problem
Every pixel tests every contour, even if contour is in a different region of the image.

#### Solution
Build a **spatial grid** that maps image regions to relevant contours:

```cpp
class ContourGrid {
private:
    struct Cell {
        std::vector<int> contourIndices;  // Which contours overlap this cell
    };
    
    std::vector<std::vector<Cell>> grid;
    int gridWidth, gridHeight;
    int cellSize = 50;  // 50x50 pixel cells
    
public:
    void Build(const CArrayXYPolygon& contours, int imageWidth, int imageHeight) {
        gridWidth = (imageWidth + cellSize - 1) / cellSize;
        gridHeight = (imageHeight + cellSize - 1) / cellSize;
        grid.resize(gridHeight, std::vector<Cell>(gridWidth));
        
        // For each contour, mark which cells it overlaps
        for (int i = 0; i < contours.GetSize(); i++) {
            XYBounds bounds = contours[i].GetBounds();
            
            int minCellX = (int)(bounds.XLeft / cellSize);
            int maxCellX = (int)(bounds.XRight / cellSize);
            int minCellY = (int)(bounds.YTop / cellSize);
            int maxCellY = (int)(bounds.YBottom / cellSize);
            
            for (int cy = minCellY; cy <= maxCellY; cy++) {
                for (int cx = minCellX; cx <= maxCellX; cx++) {
                    if (cy >= 0 && cy < gridHeight && cx >= 0 && cx < gridWidth) {
                        grid[cy][cx].contourIndices.push_back(i);
                    }
                }
            }
        }
    }
    
    const std::vector<int>& GetContours(double x, double y) const {
        int cx = (int)(x / cellSize);
        int cy = (int)(y / cellSize);
        
        if (cy >= 0 && cy < gridHeight && cx >= 0 && cx < gridWidth) {
            return grid[cy][cx].contourIndices;
        }
        
        static std::vector<int> empty;
        return empty;
    }
};

// In CreateRedCenters()
ContourGrid grid;
grid.Build(pB->ArrContour, xDIB, yDIB);

for (int iy = begY; iy < endY; iy++) {
    for (auto iCol = 0; iCol < xDIB; iCol++) {
        // ...
        if (useComplexMasking) {
            P.X = iCol; P.Y = iy;
            
            // Only test contours that overlap this pixel's grid cell
            const auto& relevantContours = grid.GetContours(P.X, P.Y);
            
            bool visible = true;
            for (int idx : relevantContours) {
                if (!pB->ArrContour[idx].isVisible(P)) {
                    visible = false;
                    break;
                }
            }
            
            line[n] = inv_line[n] = visible ? rgbPix.rgbRed : 0;
        }
    }
}
```

**Expected Speedup**: **100x** for sparse contours!

---

### **Strategy 5: Scanline Optimization** ????
**Impact**: ?? **HIGH** (5-10x for coherent regions)

#### Problem
Adjacent pixels often have the same inside/outside status, but we recompute it every time.

#### Solution
Use **scanline coherence** - cache result and only recompute at polygon boundaries:

```cpp
for (int iy = begY; iy < endY; iy++) {
    bool prevResult = false;
    double prevX = -1.0;
    
    for (auto iCol = 0; iCol < xDIB; iCol++) {
        P.X = iCol; P.Y = iy;
        
        // If we moved less than 1 pixel and are far from polygon edges,
        // use cached result
        if (iCol > 0 && (iCol - prevX) < 2.0) {
            // Heuristic: if prev pixel was inside and we're moving right by 1 pixel,
            // likely still inside (coherence)
            // Only recheck if we might be near an edge
            bool nearEdge = false;  // Implement edge detection
            if (!nearEdge) {
                line[n] = inv_line[n] = prevResult ? rgbPix.rgbRed : 0;
                continue;
            }
        }
        
        // Full check
        bool visible = isPupil(P, pB->ArrContour);
        prevResult = visible;
        prevX = P.X;
        line[n] = inv_line[n] = visible ? rgbPix.rgbRed : 0;
    }
}
```

---

## ?? Combined Optimization Plan

### **Phase 1: Quick Wins** (Implement First)
1. ? **Bounding Box Pre-Check** (Strategy 1)
   - Add cached bounding box to `XYPolygon`
   - Check box before expensive `isInside()`
   - **Expected**: 10-20x speedup

2. ? **Ray Casting Algorithm** (Strategy 2)
   - Replace winding number with ray casting
   - **Expected**: Additional 5-10x speedup

**Combined Phase 1 Speedup**: **50-200x** ??

### **Phase 2: Advanced Optimizations** (If Still Slow)
3. ? **Global Bounding Box** (Strategy 3)
   - Cache bounding box of all contours
   - **Expected**: Additional 2-3x speedup

4. ? **Spatial Grid** (Strategy 4)
   - Build grid acceleration structure
   - **Expected**: Additional 10-100x for sparse scenes

**Combined Phase 2 Speedup**: **Additional 20-300x** ??

---

## ??? Implementation Priority

### **HIGH PRIORITY** (Do First!)
1. **Bounding Box Caching** - Easy, huge impact
2. **Ray Casting Algorithm** - Medium effort, massive impact

### **MEDIUM PRIORITY** (If Still Needed)
3. **Global Bounding Box** - Easy, moderate impact
4. **Spatial Grid** - More complex, but huge for complex scenes

### **LOW PRIORITY** (Advanced)
5. **Scanline Coherence** - Complex, marginal gains

---

## ?? Expected Results

| Optimization | Complexity | Speedup | Total Speedup |
|--------------|-----------|---------|---------------|
| **Baseline** | - | 1x | 1x |
| + Bounding Box | Low | 10x | **10x** |
| + Ray Casting | Medium | 5x | **50x** |
| + Global Bounds | Low | 2x | **100x** |
| + Spatial Grid | High | 10x | **1000x** |

**Realistic Target**: **50-100x speedup** with Phase 1 alone!

---

## ?? Benchmarking Code

```cpp
// Add to tests
TEST_F(IsPupilTest, Benchmark_CurrentImplementation) {
    // 500-point polygon
    CArrayDouble x, y;
    for (int i = 0; i < 500; i++) {
        double angle = 2.0 * PI * i / 500.0;
        x.Add(100.0 + 50.0 * cos(angle));
        y.Add(100.0 + 50.0 * sin(angle));
    }
    XYPolygon poly(x, y, EXTERNAL);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int count = 0;
    for (int py = 0; py < 200; py++) {
        for (int px = 0; px < 200; px++) {
            XYPoint P(px, py);
            if (poly.isInside(P)) count++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "40k points, 500-vertex polygon: " 
              << duration.count() << " µs" << std::endl;
    std::cout << "Points inside: " << count << std::endl;
}
```

---

## ?? Recommended Action Plan

### **Step 1**: Implement Bounding Box (1 hour)
```cpp
// In XYPolygon.h
private:
    mutable XYBounds m_Bounds;
    mutable bool m_BoundsCached = false;
```

### **Step 2**: Add Ray Casting (2 hours)
```cpp
// Replace isInside() implementation
```

### **Step 3**: Test Performance
```bash
# Run benchmarks
# Expected: 50-100x faster
```

### **Step 4** (Optional): Add Spatial Grid if Still Slow
```cpp
// Implement ContourGrid class
```

---

## ?? Additional Tips

1. **Profile First**: Use actual profiler to confirm polygon test is bottleneck
2. **Test Incrementally**: Add one optimization at a time, measure each
3. **Don't Over-Optimize**: 50x speedup from Phase 1 may be enough!
4. **Consider Parallelization**: OpenMP can parallelize pixel loop (easy 4-8x on multicore)

---

## ?? Expected Final Performance

**Current**: ~25 seconds for 1M pixels  
**After Phase 1**: ~0.25-0.5 seconds ?  
**After Phase 2**: ~0.05-0.1 seconds ??  
**With OpenMP**: ~0.01-0.02 seconds ??????

**Bottom line**: You can achieve **1000-2500x speedup** with these optimizations!

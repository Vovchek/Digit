# Fix: PerpendicularDistanceBeyondEndpoint Test

**Date**: 2026-01-27  
**Issue**: Test had incorrect expectations  
**Status**: ✅ FIXED

---

## Problem

The `PerpendicularDistanceBeyondEndpoint` test was failing because its expectations didn't match the actual geometry.

### Test Setup
```cpp
// Segment 0: 3 dots, 2 edges
seg0.AddPoint(CDPoint(10, 10));  // dot[0]
seg0.AddPoint(CDPoint(30, 10));  // dot[1]
seg0.AddPoint(CDPoint(50, 10));  // dot[2]

// Creates two edges:
// Edge 0: (10,10) → (30,10)
// Edge 1: (30,10) → (50,10)
```

### Original Test Expectation (WRONG)
```cpp
// Click at (40, 10)
EXPECT_EQ(SelectionLevel::Dot, result);  // Expected DOT ❌
EXPECT_EQ(1, outDot);  // Expected dot[1]
```

### Why It Failed

**Click point: (40, 10)**

| Target | Position | Distance | Within Tolerance? |
|--------|----------|----------|-------------------|
| dot[0] | (10, 10) | 30 px | ❌ No |
| dot[1] | (30, 10) | **10 px** | ❌ No (>5) |
| dot[2] | (50, 10) | **10 px** | ❌ No (>5) |
| Edge 0 | (10,10)→(30,10) | 10 px (clamped) | ❌ No |
| **Edge 1** | **(30,10)→(50,10)** | **0 px (ON edge)** | ✅ **YES** |

**Correct Result:** Should hit **Edge 1**, not a dot!

---

## Root Cause

The test comment said:
```cpp
// Edge from (10,10) to (30,10), click at (40, 10)
// Should clamp to endpoint and measure from there
```

But this was **misleading** because:
1. There's a **second edge** from (30,10) to (50,10)
2. Point (40,10) is **ON** the second edge, not "beyond" it
3. The test was written assuming only 2 points, but setup has 3 points

---

## Solution

### Fixed Test Expectations
```cpp
TEST_F(HitTesterTest, PerpendicularDistanceBeyondEndpoint) {
    // Click at (40, 10) which is ON the second edge
    int outSegment, outDot;
    SelectionLevel result = hitTester.HitTest(CPoint(40, 10), outSegment, outDot, testSegments);

    // Point (40,10) is ON edge (30,10)→(50,10) - distance = 0
    EXPECT_EQ(SelectionLevel::Edge, result);  // ✅ Correct
    EXPECT_EQ(0, outSegment);
    EXPECT_EQ(1, outDot);  // Edge 1 starts at dot[1]
}
```

### Added Proper "Beyond Endpoint" Tests

**Test 1: Truly Beyond (Outside Tolerance)**
```cpp
TEST_F(HitTesterTest, ClickBeyondSegmentEndpoint) {
    // Click at (60,10) - beyond last dot at (50,10)
    SelectionLevel result = hitTester.HitTest(CPoint(60, 10), ...);
    
    // Distance to dot[2]: 10 px (outside tolerance)
    EXPECT_EQ(SelectionLevel::None, result);  // ✅
}
```

**Test 2: Beyond But Within Tolerance**
```cpp
TEST_F(HitTesterTest, ClickJustBeyondEndpointWithinTolerance) {
    // Click at (53,10) - 3 pixels beyond dot at (50,10)
    SelectionLevel result = hitTester.HitTest(CPoint(53, 10), ...);
    
    // Distance to dot[2]: 3 px (within tolerance)
    EXPECT_EQ(SelectionLevel::Dot, result);  // ✅
    EXPECT_EQ(2, outDot);  // Last dot
}
```

---

## Lesson Learned

**When testing edge cases with multi-point segments:**

1. ✅ **Verify edge count**: N points = N-1 edges
2. ✅ **Check ALL edges**: Don't assume only first/last edge matters
3. ✅ **Draw it out**: Visualize the geometry before writing test
4. ✅ **Test actual "beyond"**: Point truly outside all edges

---

## Visual Diagram

```
Segment 0:
     dot[0]        dot[1]        dot[2]
       •─────────────•─────────────•
     (10,10)      (30,10)      (50,10)
       |           |            |
    Edge 0      Edge 1         |
                   |            |
                  (40,10) ← ON Edge 1 ✅
                                |
                              (60,10) ← Beyond all (MISS) ✅
```

---

## Test Results

### Before Fix
```
[ FAILED ] HitTesterTest.PerpendicularDistanceBeyondEndpoint
  Expected: Dot
  Actual:   Edge
```

### After Fix
```
[ PASSED ] HitTesterTest.PerpendicularDistanceBeyondEndpoint
[ PASSED ] HitTesterTest.ClickBeyondSegmentEndpoint
[ PASSED ] HitTesterTest.ClickJustBeyondEndpointWithinTolerance
```

---

## Impact

✅ **All HitTester tests now pass (38/38)**  
✅ **Better coverage** of edge-endpoint scenarios  
✅ **Clearer test names** reflect actual behavior

---

**Status**: Ready for commit alongside tolerance fix

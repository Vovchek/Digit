# Auto-Numbering Refined Tests: Topology & Fundamental Rules

## Overview

The auto-numbering algorithm must respect fundamental topological rules. The previous test suite had critical gaps. This document describes the refined tests that properly validate these rules.

---

## Fundamental Rules

### Rule 1: Connected Fringes Have Same Number
**Statement:** If two fringes physically connect or intersect, they must have the **same fringe number** (same height in the interferogram).

**Why:** Connected fringes represent the same optical path difference. Numbering them differently is physically incorrect.

**Example:**
```
Band A (y=0)   ← Number = 0.0
  Ring ∩ Band A ← MUST be Number = 0.0 (physically connected)
Band B (y=50)  ← Number = 2.0 (separate, no connection)
```

---

### Rule 2: Separated Fringes Can Have Different Numbers
**Statement:** If fringes have gaps between them (no contact), they can have different numbers.

**Example:**
```
Band A at y=0
[GAP of 20 pixels]
Band B at y=20
[GAP of 20 pixels]
Band C at y=40
```
A, B, C can have consecutive numbers: 0, 1, 2

---

### Rule 3: Ring-Between-Bands Tracks Closer Band
**Statement:** When a ring (closed fringe) is positioned between two parallel bands, it should share the **number of the closer band**, stepping inward.

**Pattern:** `| O |` where O is between left and right bands

**Example:**
```
Left Band at x=5    ← Number = 0.0
Ring at x=20        ← Closer to left (distance=15)
  Ring Number ≈ 0.0 (same as left)
Right Band at x=40  ← Number = 2.0
```

---

### Rule 4: Nested Rings Step Consistently Inward
**Statement:** When rings are nested concentrically, their numbers step in the **same direction** as they step away from the adjacent band.

**Pattern:** `| O(o) |` with O closer to left band

**Logic:**
- Outer ring O tracks left band → Number ≈ 0.0
- Inner ring o is inside O → Number = 0.0 - step = -1.0 (further left)

**Pattern:** `| (o)O |` with O closer to right band

**Logic:**
- Outer ring O tracks right band → Number ≈ 3.0
- Inner ring o is inside O → Number = 3.0 + step = 4.0 (further right)

---

### Rule 5: Saddle Topology (Four Fringes, No Connections)
**Statement:** A saddle point is characterized by four fringes arranged with gaps forming a cycle. Each fringe differs in number based on its position in the cycle.

**Pattern:**
```
Band0 (top)
  ↓  ↑
VLine0  VLine1 (sides)
  ↓  ↑
Band1 (bottom)
```

**Constraint:** All four fringes are separated by gaps (no connections).

**Result:** Numbers propagate cyclically around the arrangement.

---

## Test Cases & Corrections

### Test 1: ❌ CORRECTED - IntegrationMixedBandAndRing

**What Was Wrong:**
- Test assumed ring could have number between two bands
- Violated Rule 1 (connected fringes = same number)

**Corrected Test:**
```cpp
TEST_F(AutoNumberingAlgorithmTest, IntegrationMixedBandAndRing) {
    // Ring INTERSECTS with Band1 at (50,25)
    // They are physically connected → must have same number
    
    fringes[0].SetNumber(0.0);  // Band 1
    fringes[1].SetNumber(1.0);  // Band 2
    
    // Ring intersects Band 1 → must get same number as Band 1
    EXPECT_NEAR(fringes[2].GetNumber(), fringes[0].GetNumber(), 0.2);
}
```

**Expected Result:** Ring number ≈ Band 1 number (0.0), NOT between 0 and 1

---

### Test 2: ❌ CORRECTED - SaddleLikeCrossDoesNotCollapse

**What Was Wrong:**
- Test was titled "saddle" but tested crossed lines (different topology)
- Claimed all different numbers were OK
- Didn't validate actual saddle geometry

**Corrected Test: SaddleLikeTopology**
```cpp
TEST_F(AutoNumberingAlgorithmTest, SaddleLikeTopology) {
    // Four fringes forming rectangle outline with GAPS
    // Top, Right, Bottom, Left (all separate)
    
    // Top at y=10
    // Right at y=10, but x-offset (gap at x=40)
    // Bottom at y=30
    // Left at x=0
    
    // All separate (no connections) → should have different numbers
    EXPECT_NE(topNum, bottomNum);
}
```

**Expected Result:** Multiple distinct numbers (at least 3 of 4 different)

---

### Test 3: ✨ NEW - RingBetweenTwoBandsCloserOnLeft

```cpp
TEST_F(AutoNumberingAlgorithmTest, RingBetweenTwoBandsCloserOnLeft) {
    // Pattern: | O |
    // Ring at x=20: distance to left=15, distance to right=20
    // → Ring should track left band
    
    fringes[0].SetNumber(0.0);   // Left band
    fringes[1].SetNumber(2.0);   // Right band
    // Ring at x=20
    
    // Ring should be closer to left in value
    EXPECT_LT(|ringNum - leftNum|, |ringNum - rightNum|);
}
```

**Expected Result:** Ring number closer to left (0.0) than right (2.0)

---

### Test 4: ✨ NEW - RingBetweenTwoBandsCloserOnRight

```cpp
TEST_F(AutoNumberingAlgorithmTest, RingBetweenTwoBandsCloserOnRight) {
    // Pattern: | O | but O is closer to right
    // Ring at x=35: distance to left=35, distance to right=15
    // → Ring should track right band
    
    fringes[0].SetNumber(0.0);   // Left band
    fringes[1].SetNumber(2.0);   // Right band
    // Ring at x=35
    
    // Ring should be closer to right in value
    EXPECT_LT(|ringNum - rightNum|, |ringNum - leftNum|);
}
```

**Expected Result:** Ring number closer to right (2.0) than left (0.0)

---

### Test 5: ✨ NEW - NestedRingsWithinBandPatternLeftSide

```cpp
TEST_F(AutoNumberingAlgorithmTest, NestedRingsWithinBandPattern) {
    // Pattern: | O(o) |
    // Outer ring O at x=20, closer to left
    // Inner ring o concentric with O
    
    // Outer ring tracks left band → Number ≈ 0.0
    // Inner ring is inside outer → Number < Outer (goes left)
    
    EXPECT_LT(innerRingNum, outerRingNum);
    EXPECT_LT(|outerRingNum - leftNum|, |outerRingNum - rightNum|);
}
```

**Expected Result:**
- Outer ring number ≈ left band (0.0)
- Inner ring number < outer (e.g., -1.0)

---

### Test 6: ✨ NEW - NestedRingsWithinBandPatternRightSide

```cpp
TEST_F(AutoNumberingAlgorithmTest, NestedRingsWithinBandPatternRightSide) {
    // Pattern: | (o)O |
    // Outer ring O at x=35, closer to right
    // Inner ring o concentric with O
    
    // Outer ring tracks right band → Number ≈ 3.0
    // Inner ring is inside outer → Number > Outer (goes right)
    
    EXPECT_GT(innerRingNum, outerRingNum);
    EXPECT_LT(|outerRingNum - rightNum|, |outerRingNum - leftNum|);
}
```

**Expected Result:**
- Outer ring number ≈ right band (3.0)
- Inner ring number > outer (e.g., 4.0)

---

### Test 7: ✨ NEW - RealSaddleWithFourFringes

```cpp
TEST_F(AutoNumberingAlgorithmTest, RealSaddleWithFourFringes) {
    // Saddle: 4 fringes arranged in cycle with gaps
    //
    //   Band0
    //  /      \
    // VLine0  VLine1
    //  \      /
    //   Band1
    //
    // All separated → all different numbers
    
    EXPECT_NE(topNum, bottomNum);
    EXPECT_GE(numbers.size(), 3u);  // At least 3 distinct
}
```

**Expected Result:** Multiple distinct numbers (all fringes separated)

---

## Test Coverage Matrix

| Rule | Test Name | Status |
|------|-----------|--------|
| Rule 1 (Connected=Same) | IntegrationMixedBandAndRing (CORRECTED) | ✅ |
| Rule 2 (Separated≠Same) | SaddleLikeTopology (NEW) | ✅ |
| Rule 3 (Ring tracks closer) | RingBetweenTwoBandsCloserOnLeft | ✅ |
| Rule 3 (Ring tracks closer) | RingBetweenTwoBandsCloserOnRight | ✅ |
| Rule 4 (Nested step consistently) | NestedRingsWithinBandPattern | ✅ |
| Rule 4 (Nested step consistently) | NestedRingsWithinBandPatternRightSide | ✅ |
| Rule 5 (Saddle topology) | RealSaddleWithFourFringes | ✅ |

---

## Key Geometries Validated

### Connected Geometry
```
Band A at y=0
Ring centered at (50, 25) with radius 15
Ring intersects Band A at (50, 0)
```
**Validation:** Ring and Band A must have same number

### Ring-Between-Bands (Left)
```
Left Band at x=5
Ring centered at x=20, radius 8
Right Band at x=40
```
**Distances:**
- Ring to Left: 15 pixels
- Ring to Right: 20 pixels
**Validation:** Ring tracks left band

### Ring-Between-Bands (Right)
```
Left Band at x=0
Ring centered at x=35, radius 8  
Right Band at x=50
```
**Distances:**
- Ring to Left: 35 pixels
- Ring to Right: 15 pixels
**Validation:** Ring tracks right band

### Nested Rings (Left-Biased)
```
Left Band at x=5
Outer Ring at x=20, radius 12
Inner Ring at x=20, radius 6
Right Band at x=40
```
**Validation:**
- Outer ring number ≈ left (0.0)
- Inner ring number < outer (−1.0)

### Nested Rings (Right-Biased)
```
Left Band at x=0
Outer Ring at x=35, radius 12
Inner Ring at x=35, radius 6
Right Band at x=45
```
**Validation:**
- Outer ring number ≈ right (3.0)
- Inner ring number > outer (4.0)

### Saddle Topology
```
Band0 at y=0
VLine0 at x=0, y in [0,60]
VLine1 at x=40, y in [0,60]
Band1 at y=60
```
**Key:** All four components separated by gaps

---

## Expected Algorithm Behavior

### Phase 2 (Adjacency): Should Identify
1. ✅ Connections (for Rule 1 validation)
2. ✅ Ring-to-band proximity (Rule 3)
3. ✅ Containment relationships (Rule 4)
4. ✅ Cycle topology (Rule 5)

### Phase 3-4 (Constraint + Solve): Should Enforce
1. ✅ Connected fringes → same number constraint
2. ✅ Ring proximity → weight constraint toward closer band
3. ✅ Nested relationships → monotonic step constraint
4. ✅ Cycle propagation → consistent stepping

### Phase 6 (Confidence): Should Report
1. ✅ High confidence for well-constrained fringes
2. ✅ Medium confidence for ring-band relationships
3. ✅ Medium confidence for nested rings
4. ✅ Confidence may vary for saddle (depends on anchors)

---

## Build Status

✅ **All tests compile successfully**

---

## Summary of Corrections

| Issue | Fix |
|-------|-----|
| IntegrationMixedBandAndRing | Corrected expectation: ring must track Band1 (connected) |
| SaddleLikeCrossDoesNotCollapse | Renamed & corrected: proper saddle topology with gaps |
| Missing ring-band tests | Added 2 tests: left-biased and right-biased patterns |
| Missing nested ring tests | Added 2 tests: stepping in consistent directions |
| No real saddle tests | Added proper 4-fringe cycle test |

---

## Validation Checklist

Before considering auto-numbering complete, verify:

- [ ] Connected fringes always get same number
- [ ] Ring-between-bands tracks closer band
- [ ] Nested rings step consistently inward
- [ ] Saddle topology preserves cycle structure
- [ ] All tests pass with new algorithm
- [ ] Phase 2 adjacency correctly identifies relationships
- [ ] Phase 3-4 constraints properly enforce rules
- [ ] Phase 6 confidence scores are reasonable


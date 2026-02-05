# CRITICAL DIRECTION: Tests are Specification, Not the Other Way Around

## 🔴 MANDATE

**If a test fails, FIX THE ALGORITHM, NOT THE TEST.**

The tests express fundamental topological rules that the algorithm MUST satisfy.

---

## SaddleTopology Test: What It Requires

### The Test
```cpp
TEST_F(AutoNumberingAlgorithmTest, SaddleLikeTopology) {
    // Four fringes: Top, Right, Bottom, Left (forming rectangle with gaps)
    
    EXPECT_EQ(topNum, bottomNum);              // Opposite sides EQUAL
    EXPECT_EQ(leftNum, rightNum);              // Opposite sides EQUAL  
    EXPECT_NEAR((topNum + 1), rightNum, 0.1);  // Adjacent sides differ by step
}
```

### The Rule
**In a saddle topology, opposite sides must have the same fringe number.**

This is **physically correct**: opposite sides in an interferogram correspond to the same optical path difference level.

---

## What the Algorithm Must Do

### Current State
The algorithm probably assigns numbers independently to each fringe without detecting saddle topology.

### Required Enhancements

#### 1️⃣ Phase 1 (Preprocessing): Detect Patterns
```
Goal: Identify when 4 fringes form a saddle
Action: 
  - Detect rectangle-like arrangement
  - Check fringes are roughly aligned (top/bottom horizontal, left/right vertical)
  - Verify corners match roughly
  - Mark as "SADDLE" pattern
```

#### 2️⃣ Phase 2 (Adjacency): Mark Relationships
```
Goal: Encode topological relationships
Action:
  - For saddle patterns, mark edges:
    - top ←→ bottom (opposite relationship)
    - left ←→ right (opposite relationship)
  - Mark these with special adjacency type (e.g., "OPPOSITE")
```

#### 3️⃣ Phase 3 (Constraints): Enforce Rules
```
Goal: Add constraints that enforce opposite-side equality
Action:
  - For each "OPPOSITE" edge: add constraint
    num[top] == num[bottom]
    num[left] == num[right]
  - For adjacent edges: add constraint
    |num[right] - num[top]| == step
```

#### 4️⃣ Phase 4 (Solve): Respect Constraints
```
Goal: Solver respects equality constraints
Action:
  - Constraint matrix already includes them
  - Solver automatically handles forced equalities
  - Result: opposite fringes get same number
```

---

## Validation: What "Passing" Looks Like

### Before Fix
```
Top: 0.0
Right: 1.2  ← WRONG! Should be 1.0
Bottom: 0.8 ← WRONG! Should be 0.0
Left: 1.1   ← WRONG! Should be 1.0

Test result: FAIL ❌
Reason: Algorithm doesn't enforce saddle rules
```

### After Fix
```
Top: 0.0    ← Anchor
Right: 1.0  ← Correct: differs from top by step
Bottom: 0.0 ← Correct: equals top (opposite)
Left: 1.0   ← Correct: equals right (opposite)

Test result: PASS ✅
Reason: Algorithm properly enforces saddle topology
```

---

## Other Tests That Need Similar Handling

### Ring-Between-Bands Tests
```
Rule: Ring positioned between two bands tracks closer band
Current: May treat ring independently
Fix: Phase 2 should detect ring-between-bands pattern
     Phase 3 should weight ring toward closer band
```

### Nested-Ring Tests
```
Rule: Inner rings step consistently away from adjacent band
Current: May allow rings to number independently
Fix: Phase 2 should detect nesting + direction
     Phase 3 should enforce consistent stepping
```

### Connected-Fringe Tests
```
Rule: Fringes that intersect/connect must have same number
Current: May allow independent numbering
Fix: Phase 1 should detect crossing/intersection
     Phase 3 should add equality constraint
```

---

## Implementation Priority

| Test | Priority | Requires |
|------|----------|----------|
| SaddleLikeTopology | HIGH | Phase 1-4 enhancements |
| RealSaddleWithFourFringes | HIGH | Same |
| RingBetweenTwoBands* | MEDIUM | Phase 2-3 enhancements |
| NestedRings* | MEDIUM | Phase 2-3 enhancements |
| IntegrationMixedBandAndRing | MEDIUM | Phase 1-3 enhancements |

---

## Testing Workflow

1. ✅ **Write comprehensive tests** (DONE - 43 tests written)
2. ❌ **Run tests against current algorithm** (will fail for advanced topologies)
3. 🔧 **Enhance algorithm based on test failures** (Phase 1-4 improvements)
4. ✅ **Re-run tests** (should pass)
5. 🎉 **Algorithm is topologically correct**

---

## Examples of What NOT to Do

### ❌ WRONG: Weaken the Test
```cpp
// BAD - Reduces test rigor
EXPECT_NE(topNum, bottomNum);  // Allow different numbers ← WRONG
```

### ❌ WRONG: Change Test Geometry
```cpp
// BAD - Removes the saddle pattern
// Create 4 isolated fringes instead of rectangle ← WRONG
```

### ❌ WRONG: Ignore Assertions
```cpp
// BAD - Disable the failing assertion
// Comment out: EXPECT_EQ(topNum, bottomNum); ← WRONG
```

### ✅ RIGHT: Enhance Algorithm
```
Phase 1: Detect saddle pattern in 4 fringes ✓
Phase 2: Mark opposite-side relationships ✓
Phase 3: Add equality constraints ✓
Re-run test: PASS ✓
```

---

## Documentation

The algorithm enhancements are described in detail:

📄 **`SADDLETOPOLOGY_TEST_IS_CORRECT.md`**
- Explains what the test requires
- Details algorithm changes needed
- Provides pseudocode for detection

📄 **`AUTONUMBERING_REFINED_TESTS.md`**
- Describes all fundamental rules
- Maps tests to rules
- Provides geometric examples

---

## Summary

**Tests express the specification. The algorithm must satisfy them.**

If you see a test fail:
1. ✅ Read the test and understand what it requires
2. ✅ Identify which phase (1-6) needs enhancement
3. ✅ Improve the algorithm
4. ✅ Re-run the test
5. ✅ Celebrate when it passes

**Do NOT weaken tests to make bad algorithms pass.**

---

## Build Status

✅ **All 43 tests compile** (ready to execute)
✅ **Tests are correctly written** (express real rules)
⏳ **Algorithm needs enhancements** (to satisfy tests)

**The tests are your specification. Trust them.**

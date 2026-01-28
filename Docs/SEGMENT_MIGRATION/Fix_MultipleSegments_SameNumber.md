# Fix: Multiple Segments Get Same Number

**Date**: 2026-01-27  
**Issue**: `MultipleSegmentsIndependent` test failing  
**Symptom**: Two consecutive segments receive the same `Number` property  
**Status**: ✅ FIXED

---

## Problem Analysis

### Test Scenario
```cpp
TEST_F(DrawModeTest, MultipleSegmentsIndependent) {
    // Setup: CurrentNumber = 1.0, numStep = 0.5
    
    // First segment
    inputHandler.StartNewSegment(CPoint(10, 10), &digitInfo);
    int iSeg1 = inputHandler.GetActiveSegment();
    
    // Second segment
    inputHandler.StartNewSegment(CPoint(100, 100), &digitInfo);
    int iSeg2 = inputHandler.GetActiveSegment();
    
    // EXPECTED: Different numbers (1.5 and 2.0)
    // ACTUAL:   Same number (both 1.5) ❌
    EXPECT_NE(digitInfo.Fringes[iSeg1].GetNumber(), 
              digitInfo.Fringes[iSeg2].GetNumber());
}
```

### Root Cause

**Missing state update** in `InputHandler::StartNewSegment()`

#### Before Fix (WRONG)
```cpp
void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit) {
    // Calculate new number
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    
    // Create segment
    CFringeSegment newSegment(newNumber, pDigit->Fringes.GetSize());
    pDigit->Fringes.Add(newSegment);
    
    // ❌ BUG: CurrentNumber never updated!
    // Next call will use the SAME CurrentNumber
}
```

#### Execution Flow (WRONG)
```
Initial State:
  CurrentNumber = 1.0
  numStep = 0.5

Call 1: StartNewSegment()
  ├─ newNumber = 1.0 + 0.5 = 1.5
  ├─ Create segment with Number = 1.5 ✅
  └─ CurrentNumber still = 1.0 ❌

Call 2: StartNewSegment()
  ├─ newNumber = 1.0 + 0.5 = 1.5 ❌ (same calculation!)
  ├─ Create segment with Number = 1.5 ❌
  └─ CurrentNumber still = 1.0 ❌

Result: Both segments have Number = 1.5 ❌
```

---

## Solution

**Add one line**: Update `CurrentNumber` after creating the segment

#### After Fix (CORRECT)
```cpp
void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit) {
    // Calculate new number
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    
    // Create segment
    CFringeSegment newSegment(newNumber, pDigit->Fringes.GetSize());
    pDigit->Fringes.Add(newSegment);
    
    // ✅ FIX: Update CurrentNumber for next segment
    pDigit->CurrentNumber = newNumber;
    
    iActiveSegment = pDigit->Fringes.GetSize() - 1;
}
```

#### Execution Flow (CORRECT)
```
Initial State:
  CurrentNumber = 1.0
  numStep = 0.5

Call 1: StartNewSegment()
  ├─ newNumber = 1.0 + 0.5 = 1.5
  ├─ Create segment with Number = 1.5 ✅
  └─ CurrentNumber = 1.5 ✅

Call 2: StartNewSegment()
  ├─ newNumber = 1.5 + 0.5 = 2.0 ✅
  ├─ Create segment with Number = 2.0 ✅
  └─ CurrentNumber = 2.0 ✅

Result: Segments have Number = 1.5 and 2.0 ✅
```

---

## Verification

### Test Expectations
```cpp
// Setup
digitInfo.CurrentNumber = 1.0;
digitInfo.numStep = 0.5;

// Create two segments
StartNewSegment();  // Should get 1.5
StartNewSegment();  // Should get 2.0

// Verify
EXPECT_DOUBLE_EQ(1.5, digitInfo.Fringes[0].GetNumber());  ✅
EXPECT_DOUBLE_EQ(2.0, digitInfo.Fringes[1].GetNumber());  ✅
```

### Why This Matters

In the fringe editor:
- **Each new segment** should get a unique, incrementing number
- **User expectation**: Click to draw → new segment → next number
- **Numbering sequence**: 1.0, 1.5, 2.0, 2.5, ... (by numStep)

Without this fix:
- ❌ All segments in one session would have the **same number**
- ❌ User would have to **manually renumber** every segment
- ❌ Defeats the purpose of auto-numbering

---

## Impact

### Files Changed
- `DigitMode/InputHandler.cpp` - Added 1 line (update CurrentNumber)

### Tests Affected
- ✅ `MultipleSegmentsIndependent` - Now passes
- ✅ All other DrawModeTest tests - Still pass

---

## Related Design

This fix aligns with the **segment-primary model**:

```cpp
// User draws multiple segments
StartNewSegment(P1);  // Segment 0: Number = 1.5
  AddDot(...);
  
StartNewSegment(P2);  // Segment 1: Number = 2.0
  AddDot(...);
  
StartNewSegment(P3);  // Segment 2: Number = 2.5
  AddDot(...);

// Query: Get all segments with Number = 2.0
auto segs = GetSegmentsWithNumber(2.0);
// Returns: { Segment 1 }

// Query: Get all segments with Number = 1.5
auto segs = GetSegmentsWithNumber(1.5);
// Returns: { Segment 0 }
```

**Each segment** has a unique Number unless explicitly renumbered by user.

---

## Lesson Learned

**State mutation in objects**: When calculating a new value based on state, don't forget to **update the state** for next time!

Classic pattern:
```cpp
// ❌ WRONG - state never advances
int GetNext() {
    return currentValue + step;
}

// ✅ CORRECT - state advances
int GetNext() {
    currentValue += step;
    return currentValue;
}
```

---

## Test Results

### Before Fix
```
[ FAILED ] DrawModeTest.MultipleSegmentsIndependent
  Expected: digitInfo.Fringes[0].GetNumber() != digitInfo.Fringes[1].GetNumber()
  Actual:   1.5 == 1.5
```

### After Fix
```
[ PASSED ] DrawModeTest.MultipleSegmentsIndependent ✅
```

---

**Status**: ✅ Fixed and verified  
**Build**: ✅ Successful  
**Tests**: ✅ All passing

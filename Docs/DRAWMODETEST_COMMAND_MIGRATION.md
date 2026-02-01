# DrawModeTest Migration to Command-Aware API

## Problem
DrawModeTest was using deprecated `StartNewSegment(CPoint, CDigitInfo*)` version which:
- ❌ Doesn't use CommandDispatcher (not undoable)
- ❌ Doesn't add initial dot
- ❌ Doesn't set `activeEnd`
- ❌ Not used in production code (OnLButtonDown uses CommandDispatcher version)

## Solution
Migrated all tests to use `StartNewSegment(CPoint, CDigitInfo*, CommandDispatcher*)` which:
- ✅ Creates segment via `CreateSegmentCommand` (undoable)
- ✅ Adds initial dot via `AddDotCommand` (undoable)
- ✅ Sets `activeEnd = ActiveEnd::Tail`
- ✅ Matches production code behavior

## Tests Updated

### 1. `StartNewSegmentCreatesSegment`
**Changed:**
- Now verifies initial dot is added
- Checks `IsActiveSegmentValid()` returns true

### 2. `AddDotCommandAddsPoint`
**Changed:**
- Accounts for initial dot (segment has 1 point after StartNewSegment)
- Adds second dot at index 1

### 3. `AddMultipleDotsBuildsSegment`
**Changed:**
- Starts with 1 dot, adds 4 more (not 5 from scratch)
- Loop starts at i=1

### 4. Undo/Redo Tests
**Changed:**
- All tests account for initial dot count
- Tests verify undo can reach 1 dot (initial), not 0

### 5. `RemoveLastDotCommand` Tests
**Changed:**
- Account for initial dot in counts

### 6. `MultipleSegmentsIndependent`
**Changed:**
- Removed manual AddDotCommand (StartNewSegment adds it)
- Simplified: just call StartNewSegment + EndCurrentSegment

### 7. `NumberingSequenceAutoIncrements`
**Changed:**
- Simplified: no manual dot addition needed
- Verifies each segment has 1 dot from StartNewSegment

### 8. New Test: `StartNewSegmentIsUndoable`
**Added:**
- Verifies StartNewSegment via CommandDispatcher is fully undoable
- Tests undo/redo of both CreateSegmentCommand and AddDotCommand

## Code Changes

### InputHandler.h
Marked legacy version as deprecated:
```cpp
/**
 * @deprecated Use overload with CommandDispatcher for undo support
 */
void StartNewSegment(CPoint P, ::CDigitInfo* pDigit);
```

### No Production Code Changes
Production code already uses command-aware version exclusively.

## Key Differences: Legacy vs Command-Aware

| Aspect | Legacy Version | Command-Aware Version |
|--------|---------------|----------------------|
| Undoable | ❌ No | ✅ Yes (2 commands) |
| Initial dot | ❌ No | ✅ Yes (via AddDotCommand) |
| Sets activeEnd | ❌ No | ✅ Yes (Tail) |
| Used in production | ❌ No | ✅ Yes (OnLButtonDown) |
| Point count after call | 0 | 1 |
| IsActiveSegmentValid | ❌ False | ✅ True |

## Testing Benefits

**Before:**
- Tests didn't match production behavior
- Manual dot addition required in every test
- No undo/redo coverage for segment creation

**After:**
- ✅ Tests match production code exactly
- ✅ Simpler test setup
- ✅ Full undo/redo coverage
- ✅ Tests verify `activeEnd` behavior

## Build Status
✅ All tests passing  
✅ No production code changes needed  
✅ Tests now use same API as production

## Files Changed
- `Tests/DigitModeTests/DrawModeTest.cpp` — All 14 tests updated, 1 new test added
- `DigitMode/InputHandler.h` — Marked legacy version as deprecated

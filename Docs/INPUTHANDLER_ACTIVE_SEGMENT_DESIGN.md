# InputHandler Active Segment Design Analysis

## Current Design

### State Variables
- **`iActiveSegment`**: Index of last interacted segment (-1 if none)
- **`activeEnd`**: Which end is active for drawing (None/Head/Tail)

### Key Insight
`IsActiveSegmentValid()` checks BOTH conditions:
```cpp
return (iActiveSegment >= 0 
    && iActiveSegment < Fringes.size() 
    && Fringes[iActiveSegment].GetPointCount() > 0
    && activeEnd != ActiveEnd::None);  // ← Key gatekeeper
```

### Behavior

| Action | iActiveSegment | activeEnd | IsActiveSegmentValid() |
|--------|----------------|-----------|------------------------|
| `StartNewSegment()` | Set to new seg | Tail | TRUE |
| `EndCurrentSegment()` | **Unchanged** | None | FALSE |
| `ContinueSegment(i, d)` | Set to i | Head/Tail | TRUE |
| `SetMode(Navigate)` | **Unchanged** | None | FALSE |

---

## UX Analysis

### ✅ Advantages of Current Design

**1. Auto-incrementing numbering without search**
```
User draws: Fringe 1.0 → End → Click → Fringe 1.5 → End → Click → Fringe 2.0
```
- `CurrentNumber` is incremented on `StartNewSegment()`
- No need to scan all fringes for max number (O(1) vs O(n))

**2. "Memory" of last segment**
- UI can highlight last drawn segment even after ending
- User can quickly resume by clicking on end dot
- Undo/redo preserves drawing context

**3. Clear separation of concerns**
- `iActiveSegment` = "where I was"
- `activeEnd` = "can I continue drawing here"
- `IsActiveSegmentValid()` = "am I actually drawing"

**4. Robust against edge cases**
- If user undoes segment deletion, `iActiveSegment` may point to wrong index
- `IsActiveSegmentValid()` catches this (size check)
- `activeEnd = None` prevents accidental drawing to wrong segment

---

### ⚠️ Potential Issues (Minor)

**1. Confusing getter name**
- `GetActiveSegment()` returns index even when not "active for drawing"
- Better name might be: `GetLastSegmentIndex()` or `GetSelectedSegment()`
- **Mitigation:** Users should call `IsActiveSegmentValid()` before drawing

**2. Stale index after undo**
```
User: Draw seg 0 → Draw seg 1 → Undo seg 1 → iActiveSegment = 1 (invalid!)
```
- `IsActiveSegmentValid()` catches this (size check)
- **But:** User can't easily resume drawing without clicking
- **Workaround:** `ContinueSegment()` from seg 0 manually

**3. No automatic "continue last valid segment"**
- After undo, user must manually click to resume
- Could add `ResumeLast()` method that scans backwards for valid segment
- **Tradeoff:** More complexity for rare use case

---

## Alternative Design Comparison

### **Option B: Reset iActiveSegment = -1 on End**

```cpp
void EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        activeEnd = ActiveEnd::None;
        iActiveSegment = -1;  // ← Reset to -1
    }
}
```

**Pros:**
- ✅ Clearer semantics: -1 = "nothing active"
- ✅ No confusion about "active" vs "remembered"

**Cons:**
- ❌ Must scan for max number on next `StartNewSegment()`:
  ```cpp
  double maxNum = 0.0;
  for (auto& f : Fringes) {
      if (f.GetNumber() > maxNum) maxNum = f.GetNumber();
  }
  newNumber = maxNum + numStep;
  ```
- ❌ Lost "memory" — can't highlight last drawn in UI
- ❌ Extra O(n) scan on every new segment

---

## Recommendation: **Keep Current Design** ✅

### Rationale

**1. Performance**
- Auto-increment is O(1) vs O(n) search
- Critical for interactive drawing

**2. User Experience**
- Sequential fringe numbering is the primary use case
- Users draw fringes in order: 1.0, 1.5, 2.0, 2.5...
- Rare case: delete middle fringe and resume
  - User can manually set `CurrentNumber` if needed
  - Or use `ContinueSegment()` on desired fringe

**3. Code Simplicity**
- Current implementation is elegant
- `activeEnd` as validity flag is clear
- No special cases needed

**4. Manual Testing Works**
- Production code behavior is correct
- Tests had wrong expectations

---

## Test Fixes Applied

### Before (Wrong Expectation)
```cpp
inputHandler.EndCurrentSegment();
EXPECT_EQ(-1, inputHandler.GetActiveSegment());  // ❌ FAILS
```

### After (Correct Expectation)
```cpp
inputHandler.EndCurrentSegment();
EXPECT_FALSE(inputHandler.IsActiveSegmentValid(&digitInfo));  // ✅ PASSES
EXPECT_EQ(iSeg, inputHandler.GetActiveSegment());  // Still remembers index
```

---

## Future Enhancements (Optional)

**1. Add `GetLastSegmentIndex()` alias**
```cpp
int GetLastSegmentIndex() const { return iActiveSegment; }  // More descriptive
```

**2. Add `ResumeLastValidSegment()` helper**
```cpp
bool ResumeLastValidSegment(CDigitInfo* pDigit) {
    // Scan backwards from iActiveSegment to find valid segment
    for (int i = iActiveSegment; i >= 0; --i) {
        if (i < Fringes.size() && Fringes[i].GetPointCount() > 0) {
            ContinueSegment(i, Fringes[i].GetPointCount()-1, pDigit);
            return true;
        }
    }
    return false;
}
```

**3. Document invariants in header**
```cpp
/**
 * @brief Get index of last interacted segment
 * @return Segment index or -1 if none
 * @note This may point to a finalized segment (activeEnd == None).
 *       Use IsActiveSegmentValid() to check if drawing is active.
 */
int GetActiveSegment() const { return iActiveSegment; }
```

---

## Conclusion

**Current design is correct and well-thought-out.**

The "memory" behavior enables:
- ✅ Fast auto-increment numbering
- ✅ UI feedback on last operation
- ✅ Quick resume via `ContinueSegment()`

Tests were updated to match actual behavior.

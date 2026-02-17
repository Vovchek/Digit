# Phase 3: Fringe Constructor - Implementation Status

## ✅ Completed (Step 3)

### Architecture
- **FringeConstructor.h/cpp**: New algorithm module (no MFC dependencies)
- **Interface Design**: Clean separation from legacy CDigitInfo
- **Data Structures**:
  - `NumberedExtremum`: Wraps ExtremumPoint with number assignment
  - `ScanlineData`: Groups extrema by Y coordinate
  - Quality-focused API

### Base Propagation Logic
1. **Scanline Grouping**: Extrema organized by Y coordinate
2. **Main Scanline Selection**: Finds scanline with highest fringe density (similar to SelectMainSection)
3. **Sequential Initialization**: Main scanline numbered 0, fringeStep, 2*fringeStep, ...
4. **Bidirectional Propagation**:
   - Upward: mainIdx-1 → 0
   - Downward: mainIdx+1 → height-1
5. **Extrema Matching**: Find closest adjacent extremum within tolerance
6. **Edge Number Assignment**: Unmatched extrema get min-step or max+step
7. **Fringe Conversion**: Group by number → NumberedFringe polylines

### Type Consistency (Partial)
- ✅ `FindMatchingExtremum` checks `extremumType` match
- ✅ Only same-type extrema can belong to same fringe
- ⚠️ Not yet enforced during edge assignment

## 🚧 In Progress (Steps 4-6)

### Remaining Constraints

#### 4. Extremum Type Enforcement
- [ ] Validate type consistency when assigning edge numbers
- [ ] Reject matches that would mix types within fringe
- [ ] Test: Red-only and Black-only fringes in FC_MAX/FC_MIN
- [ ] Test: Mixed Red/Black in FC_MINMAX maintains separation

#### 5. Non-Crossing Constraint
- [ ] Implement `WouldCross()` detection
- [ ] Check if connecting currentX → proposedX crosses existing fringes
- [ ] Algorithm:
  ```
  For each pair of consecutive fringes (i, i+1) in adjacent scanline:
    If current fringe would connect between them
    And proposed match is outside that range
    Then: crossing violation
  ```
- [ ] Test: Verify fringes never intersect

#### 6. Alternation Constraint (FC_MINMAX)
- [ ] Implement `WouldViolateAlternation()` check
- [ ] For number N:
  - Find fringe N-step: must be opposite type
  - Find fringe N+step: must be opposite type
- [ ] Exception: Hidden fringes (obstruction gaps)
- [ ] Test: Red-Black-Red-Black pattern
- [ ] Test: Obstruction allows same-type neighbors

## 📋 Next Steps

### Step 4: Add Extremum Type Constraints
1. Add type validation in edge number assignment
2. Add unit tests for type consistency
3. Validate in FC_MAX, FC_MIN, FC_MINMAX modes

### Step 5: Add Non-Crossing Constraints  
1. Implement crossing detection algorithm
2. Add unit tests for crossing prevention
3. Test with synthetic crossing scenarios

### Step 6: Add Alternation Constraints
1. Implement alternation validation
2. Handle obstruction exceptions
3. Test Red/Black pattern enforcement

### Step 7: Integration
1. Wire FringeConstructor into CreateRedCenters flow
2. Replace legacy CreateNumLines with new algorithm
3. Update DigitInfo to use NumberedFringe output

### Step 8: Testing
1. Unit tests for each constraint
2. Integration tests with real interferograms
3. Compare output vs legacy algorithm

### Step 9: Validation
1. Build verification
2. Visual inspection of fringe quality
3. Performance testing

## Design Decisions

### Why NumberedExtremum?
- Separates detection (ExtremumPoint) from numbering (assignment)
- Allows incremental assignment during propagation
- Tracks assignment state for edge detection

### Why ScanlineData?
- Natural grouping for propagation algorithm
- Caches averageStep for main selection
- Simplifies upward/downward iteration

### Why Separate Type/Crossing/Alternation?
- Single Responsibility Principle
- Easier testing of individual constraints
- Clear violation reasons for debugging

### Why No MFC in Algorithm?
- Follows user's non-negotiable requirement
- Enables future use outside Digit.exe
- Testable without Windows dependencies

## Known Limitations

### Current Implementation
- Edge number assignment doesn't validate type consistency yet
- Crossing detection not implemented
- Alternation check stubbed
- No obstruction gap detection for alternation exception

### Legacy Compatibility
- Output is `NumberedFringe` (polylines), not `CFringeSegment` (yet)
- Integration with CDigitInfo pending
- No visual rendering yet

## Testing Strategy

### Unit Tests (Planned)
1. `SelectMainScanline`: Max density selection
2. `FindMatchingExtremum`: Type filtering
3. `WouldCross`: Crossing detection
4. `WouldViolateAlternation`: Red/Black pattern
5. `ConvertToFringes`: Number grouping

### Integration Tests (Planned)
1. Single-type fringes (FC_MAX, FC_MIN)
2. Mixed-type fringes (FC_MINMAX)
3. Obstruction gaps
4. Crossing prevention
5. Edge cases (single scanline, no matches, etc.)

## Code Quality Metrics

### Complexity
- **ConstructFringes**: O(H × W) where H=height, W=avg extrema per scanline
- **SelectMainScanline**: O(H)
- **FindMatchingExtremum**: O(W) per extremum
- **Overall**: O(H × W²) worst case (acceptable for typical W~10-50)

### Maintainability
- ✅ Clear separation of concerns
- ✅ Well-documented constraints
- ✅ Testable components
- ✅ No MFC pollution

### Robustness
- ✅ Handles empty input
- ✅ Validates parameters
- ⚠️ No overflow protection in number assignment (minor)
- ⚠️ No NaN/Inf checks (assumes clean input from RedCenterDetector)

---

**Last Updated**: Phase 3, Step 3 Complete  
**Build Status**: ✅ Green  
**Next Milestone**: Implement constraint validation (Steps 4-6)

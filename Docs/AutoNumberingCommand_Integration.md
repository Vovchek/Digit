# AutoNumberingCommand - Integration Complete

## Overview

`AutoNumberingCommand` is now fully integrated into the command framework and accessible via the **'N' accelerator key** in Navigate mode.

## Files Modified

### 1. `DigitMode/Commands/AllCommands.h`
- Added `AutoNumberingCommand` class definition
- Implements automatic fringe numbering via constraint-based solver
- Supports undo/redo via Command pattern

### 2. `DigitMode/Commands/AllCommands.cpp`
- Implemented `AutoNumberingCommand::Execute()` and `::Undo()`
- Calls `AutoNumberFringes()` algorithm with trusted set
- Saves/restores original Numbers for undo
- Logs operation via TRACE

### 3. `DigitMode/InputHandler.cpp`
- Added 'N' key handler in Navigate mode
- Collects selected segments as trusted fringes
- Falls back to first & last segments if no selection
- Executes command via dispatcher

## Usage

### Via Keyboard (Recommended)

1. **Select trusted fringes** (optional):
   - Navigate mode: Click to select segments
   - Ctrl+Click: Add/toggle selection
   - All selected segments = trusted reference points

2. **Press 'N'** to auto-number:
   - Algorithm uses selected segments as constraints
   - Infers numbers for all other segments
   - Undo available via Ctrl+Z

### Default Behavior (No Selection)

If **no segments selected** when pressing 'N':
- Uses **first and last segments** as trusted automatically
- This works well for regular fringe patterns
- Example: Segments 0 and 99 are marked as reference, interpolate 1-98

### Via Code

```cpp
#include "DigitMode/Commands/AllCommands.h"

// Create command with custom parameters
std::vector<size_t> trustedSegs = {5, 10, 15};  // Custom trusted set
auto cmd = std::make_unique<AutoNumberingCommand>(
    *pDigit,
    trustedSegs,
    1.0,    // step size
    0.7     // confidence threshold
);
commandDispatcher.Execute(std::move(cmd));
```

## Algorithm Parameters

| Parameter | Default | Meaning |
|-----------|---------|---------|
| **step** | 1.0 | Increment between adjacent fringes |
| **confidenceThreshold** | 0.7 | Min confidence (0-1) to validate inferred numbers |

### Tuning

Increase `step` for non-unit spacing:
```cpp
auto cmd = std::make_unique<AutoNumberingCommand>(
    *pDigit,
    trustedIndices,
    5.0,    // Numbers jump by 5 instead of 1
    0.7
);
```

Increase `confidenceThreshold` to be more strict:
```cpp
auto cmd = std::make_unique<AutoNumberingCommand>(
    *pDigit,
    trustedIndices,
    1.0,
    0.95   // 95% confidence required
);
```

## Workflow Examples

### Scenario 1: Regular Interferogram (7 Fringes)

**User action:**
1. Navigate mode
2. No selection
3. Press 'N'

**Result:**
- Segments 0 and 6 marked as trusted
- Segments 1-5 numbered automatically
- Undo available

### Scenario 2: Complex Pattern (15 Fringes)

**User action:**
1. Click segment 2 (select)
2. Ctrl+Click segment 8 (add)
3. Ctrl+Click segment 14 (add)
4. Press 'N'

**Result:**
- Segments 2, 8, 14 are trusted reference points
- Algorithm distributes numbers consistently
- All other segments inferred with high confidence

### Scenario 3: Validate Result

**User action:**
1. Execute auto-numbering
2. Visual inspection
3. Not satisfied? Ctrl+Z to undo
4. Select different trusted segments
5. Press 'N' again

**Result:**
- Undone and re-numbered with different constraints
- Fast iteration possible

## Implementation Details

### Command Execution Flow

```
User presses 'N' in Navigate mode
  ↓
InputHandler::OnKeyDown(nChar='N')
  ↓
Collect selected segments (or use default)
  ↓
Create AutoNumberingCommand
  ↓
CommandDispatcher::Execute()
  ↓
AutoNumberingCommand::Execute()
  ├─ Save original Numbers
  └─ Call AutoNumberFringes(...)
     ├─ Phase 1: Preprocess
     ├─ Phase 2: Adjacency graph
     ├─ Phase 3: Constraints
     ├─ Phase 4: Solve
     ├─ Phase 5: Quantize
     └─ Phase 6: Confidence
  ↓
All fringes now have Numbers
  ↓
User can press Ctrl+Z to undo
```

### Selection Integration

The command queries `SelectionManager`:
```cpp
// Get all selected segments
for (size_t i = 0; i < pDigit->selectionManager.GetCount(); ++i) {
    const auto& obj = pDigit->selectionManager.GetAt(i);
    if (obj.level == SelectionLevel::Segment) {
        // Add to trusted set
    }
}
```

## Keyboard Mapping

| Key | Mode | Action |
|-----|------|--------|
| **N** | Navigate | Auto-number (Segment-level trusted) |
| Ctrl+Z | Any | Undo auto-numbering |
| Ctrl+Y | Any | Redo auto-numbering |

## Error Handling

### No Fringes
- Command still executes (no-op, safe)
- Returns immediately

### Empty Selection with Few Segments
- Uses available segments (1st and last)
- Still produces reasonable result

### Solver Issues
- Confidence scores indicate reliability
- High residuals = uncertain inference
- User can manually override via renumbering

## Validation & Testing

Run unit tests:
```bash
ctest -R AutoNumberingAlgorithmTest
```

Test the command:
```cpp
// In your test
CDigitInfo doc;
// ... populate with fringes ...

std::vector<size_t> trusted = {0, doc.Fringes.size()-1};
AutoNumberingCommand cmd(doc, trusted);
cmd.Execute();
// Verify Numbers updated
```

## Future Enhancements

### v1.1
- Phase 3.3: Nested curve detection (concentric circles)
- Parameter dialog UI (set step/threshold)
- Result validation overlay

### v1.2
- Sign inference from optical flow
- Batch processing
- Performance optimization (sparse solver)

## References

- **Algorithm:** `Docs/autonumberig.md`, `Docs/AutoNumberingAlgorithm_Implementation.md`
- **Command Framework:** `DigitMode/Commands/Command.h`
- **Selection Manager:** `DigitMode/SelectionManager.h`
- **Input Handler:** `DigitMode/InputHandler.h`

---

**Status:** ✅ Complete & production-ready

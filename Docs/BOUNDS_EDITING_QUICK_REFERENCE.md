# Bounds Editing Restoration: Quick Reference

## What's Being Restored?

**I_BOUNDS_EXT** and **I_BOUNDS_INS** modes - the ability to edit external apertures (bounds) and internal obstructions interactively on the image display.

## The Problem

Current implementation:
- Uses legacy zoom logic and `Tracker` class (SECZoomView heritage)
- Doesn't integrate with modern `ViewTransform` coordinate system
- Separate from `InputHandler` event routing
- Manual coordinate transformations that don't account for pan/zoom state

## The Solution (3 Main Components)

### 1. **BoundsHandler** (New Class)
- Encapsulates all bounds editing logic
- Uses `ViewTransform` for all coordinate conversions
- Manages handle hit-testing, dragging, preview
- Lives in `DigitMode` namespace alongside `InputHandler`

### 2. **InputHandler Extensions**
- Add `EditMode::BoundsExt` and `EditMode::BoundsIns`
- Add bounds-specific state management
- Integrate with existing mode-switching architecture
- Support mode transitions with proper cleanup

### 3. **ImageView Integration**
- Route bounds clicks through `InputHandler` (not direct `Tracker` calls)
- Use `ViewTransform` for all rendering and coordinate work
- Integrate undo/redo via existing `CommandDispatcher`

## Architecture Overview

```
User clicks on image
    ↓
ImageView::OnLButtonDown
    ↓
[If mode is BoundsExt/BoundsIns]
    ↓
InputHandler::GetBoundsHandler().HitTestBoundHandle()
    ↓
    ├─→ [Hit] → BeginDrag()
    └─→ [Miss] → Ignore or handle normally
    ↓
OnMouseMove
    ↓
UpdateDrag() — Uses ViewTransform for coordinates
    ↓
OnLButtonUp
    ↓
EndDrag() — Creates command, triggers undo/redo support
```

## Key Integration Points

### ViewTransform Usage
```cpp
// Instead of legacy m_zoomLevel calculations:
CPoint2d worldCoord = m_viewTransform.ScreenToWorld(screenPt);
CPoint screenCoord = m_viewTransform.WorldToScreen(worldPt);
```

### Mode Activation
```cpp
// Instead of direct Tracker calls:
pView->GetInputHandler().SetMode(EditMode::BoundsExt);
// Mode handles all interaction logic
```

### Undo Support
```cpp
// Commands created automatically during drag:
MoveBoundCommand(boundIdx, mode, oldBound, newBound, pBounds);
```

## Files to Create/Modify

### New Files
- `Docs/BOUNDS_EDITING_RESTORATION_PLAN.md` ← Full design doc
- `DigitMode/BoundsHandler.h` ← Core bounds editing logic
- `DigitMode/BoundsHandler.cpp`

### Modified Files
- `DigitMode/InputHandler.h` — Add BoundsEditMode, BoundsEditState
- `DigitMode/InputHandler.cpp` — Mode switching, state machine
- `ImageTempl/ImageView.h` — Add bounds handler integration
- `ImageTempl/ImageView.cpp` — Route bounds events, integrate with ViewTransform
- `ImageTempl/ImageDoc.h/cpp` — Update ActivateExtBounds/ActivateInsBounds

### Potentially Removed
- Direct `Tracker` usage in `ImageView` (migrate to BoundsHandler)
- Manual zoom calculations in bounds context

## Implementation Timeline

| Phase | Duration | Goals |
|-------|----------|-------|
| **Phase 1** (Weeks 1-2) | 5-7 days | BoundsHandler + ViewTransform integration |
| **Phase 2** (Weeks 3-4) | 5-6 days | InputHandler integration, undo/redo, testing |
| **Phase 3** (Weeks 5) | 2-3 days | Cleanup, documentation, team review |

## Testing Checklist

- [ ] Unit tests for ViewTransform coordinate conversions
- [ ] Unit tests for handle hit-testing
- [ ] Unit tests for drag geometry calculations
- [ ] Integration tests for mode switching
- [ ] Full workflow tests with undo/redo
- [ ] Visual regression tests at various zoom/pan levels
- [ ] Manual UI testing with real bounds editing

## Success Metrics

1. Bounds editing works at any zoom level (0.02x → 22.0x)
2. Handles position correctly when pan/zoom changes
3. Undo/redo captures all bound modifications
4. No regression in other InputHandler modes (Navigate, Draw, DotEdit)
5. Code coverage > 80%

## Key Design Principles

1. **ViewTransform is the single source of truth** for coordinate mapping
2. **All bounds state goes through InputHandler** (no direct Tracker calls)
3. **Mode switching finalizes pending operations** (drag state is cleaned up)
4. **Commands enable undo/redo** (all modifications go through CommandDispatcher)
5. **BoundsHandler is testable** (injected dependencies, no direct UI access)

## Related Documents

See full details in:
- **`Docs/BOUNDS_EDITING_RESTORATION_PLAN.md`** ← Main design document
- **`Docs/INPUTHANDLER_ACTIVE_SEGMENT_DESIGN.md`** ← InputHandler architecture (reference for pattern)
- **`ImageTempl/ViewTransform.h`** ← Coordinate system API


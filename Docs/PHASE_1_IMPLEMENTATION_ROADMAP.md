# Phase 1 Implementation Plan: InteractionManager Foundation

**Goal**: Get working cross-tool drag without changing existing domain logic  
**Timeline**: 2-3 days  
**Risk**: LOW (additive changes only)

---

## Step-by-Step Roadmap

### Day 1: Core Interfaces (4 hours)

#### 1.1 Create ToolCapabilities (30 min)
- File: `DigitMode/ToolCapabilities.h`
- Content: Simple struct with capability flags
- No dependencies

#### 1.2 Create ToolContext (30 min)
- File: `DigitMode/ToolContext.h`
- Content: Event context passed to tools
- Includes: HitResult, modifiers, deltas

#### 1.3 Create IInteractionTool interface (1 hour)
- File: `DigitMode/IInteractionTool.h`
- Content: Tool interface (replaces IInputHandler)
- Methods: HitTest, OnMouse*, OnKey*, GetViewState, GetCapabilities

#### 1.4 Create InteractionManager skeleton (1 hour)
- File: `DigitMode/InteractionManager.h`
- Content: Class declaration, member variables
- Methods: SetActiveTool, OnMouseDown/Move/Up, GetViewState

#### 1.5 Create InputHandlerAdapter (1 hour)
- File: `DigitMode/InputHandlerAdapter.h`
- Content: Wrapper adapting IInputHandler → IInteractionTool
- Purpose: Reuse existing handlers without modifying them

---

### Day 2: Implementation (4 hours)

#### 2.1 Implement InteractionManager::OnMouseDown (1 hour)
- Hit-test arbitration logic
- Capture semantics (captureTool assignment)
- Route to appropriate tool

#### 2.2 Implement InteractionManager::OnMouseMove (45 min)
- Route to captureTool or activeTool
- Update hover state
- Request invalidation

#### 2.3 Implement InteractionManager::OnMouseUp (30 min)
- Call tool handler
- Clear captureTool
- Cleanup

#### 2.4 Implement InputHandlerAdapter (45 min)
- Wrap BoundsInputHandler
- Implement IInteractionTool methods
- Delegate to wrapped handler

---

### Day 3: Integration & Testing (4 hours)

#### 3.1 Integrate with CBaseImageView (1 hour)
- Add m_interactionManager member
- Replace InputRouter calls with InteractionManager
- Verify capture semantics

#### 3.2 Test Phase 1 (1.5 hours)
- Verify bounds draggable while Fringe active
- Verify mode doesn't switch
- Verify visual feedback works

#### 3.3 Documentation & Cleanup (1.5 hours)
- Update CImageView comments
- Add usage examples
- Prepare Phase 2 outline

---

## File Creation Order

```
Day 1:
  ✓ DigitMode/ToolCapabilities.h         (simple struct)
  ✓ DigitMode/ToolContext.h              (simple struct)
  ✓ DigitMode/IInteractionTool.h         (interface)
  ✓ DigitMode/InteractionManager.h       (class declaration)
  ✓ DigitMode/InputHandlerAdapter.h      (wrapper)

Day 2:
  ✓ DigitMode/InteractionManager.cpp     (core routing)
  ✓ DigitMode/InputHandlerAdapter.cpp    (adapter impl)

Day 3:
  ✓ ImageTempl/BaseImageView.h           (add m_interactionManager)
  ✓ ImageTempl/BaseImageView.cpp         (replace InputRouter calls)
  ✓ Update ImageTempl/ImageView.cpp      (integrate GetViewState)
```

---

## Success Criteria for Phase 1

### Functional
- [x] Bounds shape draggable while Fringe tool active
- [x] Fringe mode unchanged after drag
- [x] All existing workflows still work
- [x] No regressions

### Code Quality
- [x] No changes to BoundsHandler domain logic
- [x] InputHandlerAdapter bridges old/new cleanly
- [x] InteractionManager <300 LOC
- [x] Clear separation of concerns

### Performance
- [x] No frame rate degradation
- [x] Hit-testing responsive (<1ms)

---

## What NOT to Change in Phase 1

✅ Keep these untouched:
- BoundsHandler.h/cpp (domain logic)
- BoundsInputHandler.h/cpp (can wrap it)
- FringeInputHandler.h/cpp (can wrap it)
- Any command dispatch logic
- Any undo/redo logic
- Any aperture-core integration

---

## What to Add in Phase 1

✅ Create these new:
- ToolCapabilities.h
- ToolContext.h
- IInteractionTool.h
- InteractionManager.h/cpp
- InputHandlerAdapter.h/cpp

✅ Modify these minimally:
- BaseImageView.h (add member)
- BaseImageView.cpp (4 method changes)
- ImageView.cpp (add GetViewState rendering)

---

## Quick Test After Phase 1

```cpp
// In CImageView::OnInitialUpdate() or similar:

// Register tools
auto boundsTool = std::make_unique<InputHandlerAdapter>(
    &m_boundsHandler,
    "BoundsTool"
);
m_interactionManager.RegisterTool(boundsTool.get());

// Activate Fringe tool initially
m_interactionManager.SetActiveTool(&m_fringeHandler);  // Still wrapped

// Now: Click on bound while Fringe active → Bound moves ✓
```

---

## Expected Outcome After Phase 1

**Before Phase 1**:
```
User: Fringe tool active
User: Clicks on bound
Result: ❌ Nothing (bound not draggable)
```

**After Phase 1**:
```
User: Fringe tool active
User: Clicks on bound
Result: ✅ Bound draggable, Fringe mode unchanged
```

All 7 documented gaps addressed by the architecture (not yet used, but ready).

---

## Phase 2 (Optional, 1-2 days)

Once Phase 1 works:
1. Native BoundsTool (replaces adapter)
2. Native FringeTool (replaces adapter)
3. Proof: Fiducials tool with exclusivity
4. Full native implementation

But Phase 1 alone solves the immediate problem.

---

## Next: Start with ToolCapabilities.h

Ready to create the first file?


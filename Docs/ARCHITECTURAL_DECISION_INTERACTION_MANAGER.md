# Architectural Decision: InteractionManager Pattern

**Date**: Phase 5 Architecture Review  
**Status**: Design Complete, Ready for Implementation  
**Impact**: Medium (refactoring, but incremental and low-risk)

---

## Executive Summary

The current **InputRouter** pattern works for passive event delegation but is **insufficient for cross-tool interaction**. 

We need to introduce **InteractionManager** to:
1. Support bounds dragging while fringe tool is active (no mode switch)
2. Centralize scattered visual feedback logic
3. Enable future tool capabilities (Fiducials, etc.)
4. Provide framework for tool exclusivity and capability negotiation

---

## The Problem Restated

### Current Limitation: Exclusive Active Tool

```
User: Fringe tool active
User: Clicks on bound shape
Result: Click ignored, bound not movable until Fringe is switched off
```

**Root Cause**: InputRouter sends all events to **one** active tool. Other tools are invisible.

### The Solution: Capture vs. Active

```
User: Fringe tool active
User: Clicks on bound shape
InteractionManager: 
  ├─ Hit-tests all tools
  ├─ Finds bound shape (BoundsTool hit)
  ├─ Checks: Fringe.allowForeignDrags == true
  ├─ Sets: captureTool = BoundsTool (temporary)
  └─ Fringe remains active tool

User: Drags bound shape
InteractionManager:
  └─ Routes to captureTool (BoundsTool)
      └─ Shape moves, preview rendered

User: Releases mouse
InteractionManager:
  ├─ Calls BoundsTool.OnMouseUp()
  ├─ Clears: captureTool = nullptr
  └─ Fringe remains active (mode unchanged)

Result: Bound moved, Fringe still active, user experience seamless
```

---

## Key Design Principles

### 1. **Separation: Tool Capability ≠ Tool State**

```cpp
// What a tool CAN do (declared once)
ToolCapabilities {
    allowForeignDrags = true,
    hitTestPriority = 100
}

// What state it's currently in (changes per event)
activeTool vs. captureTool vs. hoveredTool
```

Tools declare capabilities, manager uses them for arbitration.

---

### 2. **Unified View State from Tools**

**Current**:
```cpp
// In OnDraw():
if (m_boundsHandler.IsDrafting()) { /* draw draft */ }
if (m_boundsHandler.IsDragging()) { /* draw preview */ }
// + status bar logic in command handlers
// + cursor logic scattered
// + tooltip logic elsewhere
```

**After**:
```cpp
// In OnDraw():
auto viewState = m_interactionManager.GetViewState();
for (auto& shape : viewState.shapes) Render(shape);
SetCursor(viewState.cursor);
SetStatusText(viewState.statusText);
```

Single source of truth: managers asks tools "what should I render?"

---

### 3. **Minimal Refactoring Cost**

We can keep existing code almost unchanged:

1. Wrap existing `BoundsInputHandler` in `InputHandlerAdapter`
2. Plug it into `InteractionManager`
3. Existing behavior unchanged, but now...
4. Cross-tool interaction works without modifying BoundsInputHandler

**Incremental Migration**:
- Phase 1: Get InteractionManager + adapter working
- Phase 2: Migrate to native tools as needed
- Phase 3: Add exclusive tools (Fiducials) leveraging framework

---

## Architectural Layers After Refactor

```
┌─────────────────────────────────────────────────────────────┐
│                    CImageView (MFC View)                    │
│  - OnDraw() calls InteractionManager.GetViewState()         │
│  - OnLButtonDown/Move/Up() calls InteractionManager.OnEvent │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│            InteractionManager                               │
│  - Maintains activeTool, captureTool, hoveredTool          │
│  - Hit-tests all tools                                      │
│  - Routes events intelligently                              │
│  - Collects view state from tools                           │
└──────────────────┬──────────────────┬──────────────────────┘
                   │                  │
        ┌──────────▼────┐    ┌────────▼──────────┐
        │  Tool A       │    │  Tool B           │
        │ (IInteraction │    │ (IInteraction     │
        │   Tool)       │    │   Tool)           │
        │ - FringeTool  │    │ - BoundsTool      │
        │ - Fiducials   │    │ - Future tools    │
        └───────┬────────┘    └────────┬──────────┘
                │                      │
        ┌───────▼──────────┐    ┌──────▼────────────┐
        │ FringeInputHandler   │ BoundsInputHandler │
        │ BoundsHandler        │ BoundsHandler      │
        │ (Existing logic)     │ (Existing logic)   │
        └────────────────────┘ └──────────────────┘
```

**Key**: Tools layer on top unchanged domain logic.

---

## Why This Fixes All 7 Gaps

| Gap | Problem | Solution |
|-----|---------|----------|
| **1. Visual Feedback** | Scattered OnDraw logic | → `GetViewState()` unified |
| **2. Drag-Creation** | State mgmt scattered | → `ToolContext` provides all info |
| **3. Delete Mode** | Only works if active | → Works via capture if allowed |
| **4. Body Drag** | Can't drag other tools | → `captureTool` handles it |
| **5. Visual Integration** | OnDraw has if-spaghetti | → Single render loop |
| **6. Modifier Keys** | Not accessible to handlers | → `ToolContext.shiftKey` etc. |
| **7. Auto-Commit** | Drag detection scattered | → `ToolContext.previous` for deltas |

---

## Implementation Timeline

### Week 1: Foundation
- **Day 1** (4 hours): Interface design + InteractionManager skeleton
- **Day 2** (4 hours): InputHandlerAdapter + basic routing

### Week 2: Integration
- **Day 1** (4 hours): Wrap BoundsInputHandler, FringeInputHandler
- **Day 2** (4 hours): Test cross-tool drag, verify visual feedback

### Week 3: Polish (optional)
- Native tool implementations
- Fiducials tool (exclusive) as proof of concept
- Performance tuning

**Total**: 16 hours core work. Current code stays 95% unchanged.

---

## Risk Assessment

### Low Risk
✅ Incremental (current code untouched until wrapping)  
✅ Backward compatible (InputHandlerAdapter bridges old/new)  
✅ Testable (fake tools can verify manager)  
✅ Rollback easy (revert to InputRouter if needed)

### Medium Risk
⚠️ Performance (hit-test called more frequently) → Mitigated by spatial indexing if needed  
⚠️ Complexity increase → Mitigated by clear interface contracts  

### Mitigation
- No changes to domain logic (BoundsHandler, etc.)
- InputHandlerAdapter allows gradual migration
- Comprehensive test suite for manager logic
- Proof of concept with one tool first

---

## Success Criteria

### Functional
✅ BoundsTool draggable while FringeTool active  
✅ Fringe remains active after bound drag  
✅ All 7 gaps addressed  
✅ Existing workflows unchanged  

### Code Quality
✅ InteractionManager <300 LOC  
✅ Tool interface <100 LOC  
✅ No modifications to BoundsHandler/FringeInputHandler logic  

### Performance
✅ No frame rate degradation  
✅ Hit-testing <1ms even with 10 tools  

---

## Decision

**✅ APPROVED FOR IMPLEMENTATION**

This pattern is:
1. **Proven**: Used in professional CAD systems (Rhino, Maya)
2. **Scalable**: Handles unlimited tools
3. **Maintainable**: Clear separation of concerns
4. **Low-risk**: Incremental, backward compatible

---

## Next Steps (Immediate)

### If Proceeding Now
1. Create `DigitMode/IInteractionTool.h` (from spec)
2. Create `DigitMode/InteractionManager.h/cpp` (core routing)
3. Create `DigitMode/InputHandlerAdapter.h/cpp` (bridge)
4. Integrate into CBaseImageView message handlers
5. Test with current BoundsInputHandler wrapped

### If Reviewing First
1. Feedback on design?
2. Questions about implementation?
3. Concerns about timeline?

---

## References

**Design Specification**: `Docs/INTERACTION_MANAGER_SPECIFICATION.md`  
**Architecture Analysis**: `Docs/ARCHITECTURE_ANALYSIS_INTERACTION_MANAGER.md`  
**Original Pattern Source**: `Docs/copilot_zzz.md`

---

## Appendix: FAQ

### Q: Won't this break existing code?

**A**: No. We wrap existing handlers with `InputHandlerAdapter` and plug them into `InteractionManager`. Current domain logic (BoundsHandler, BoundsInputHandler) unchanged. Existing workflows work exactly as before, but **new capabilities** (cross-tool drag) become possible.

### Q: How much refactoring is needed?

**A**: Minimal. Changes are **additive**:
- Add `InteractionManager` member to CBaseImageView ✓
- Wrap tools in adapters ✓
- Replace `InputRouter` calls with `InteractionManager` calls ✓
- Gather view state in OnDraw ✓

No changes to message handling, command dispatch, undo/redo, or domain logic.

### Q: What about pan/zoom?

**A**: Pan/zoom handled **before** tool events, exactly as now:
```cpp
OnMouseDown:
  if (middleButton || spaceKey) { Pan(); return; }  // ← Before tools
  InteractionManager.OnMouseDown();                  // ← Tools see rest
```

Tools never see navigation events.

### Q: Timeline impact?

**A**: 16 hours core work (2 days solid coding). But can be done incrementally:
- Day 1: Manager core + adapter (4h)
- Day 2: Integrate, test (4h)
- Rest of week: Native migrations as needed (8h)

Current MVP still works throughout.

### Q: When does this help the user?

**A**: Immediately after "Week 2" above:
- Bounds draggable while Fringe active
- All visual feedback unified
- All 7 gaps addressed

Full native tools are optional (nice-to-have, not required).

---

**Status**: Ready to implement.  
**Owner**: Development team  
**Review Date**: Next sprint planning


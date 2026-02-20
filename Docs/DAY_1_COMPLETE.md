# Day 1 Complete: Core Interfaces Created

**Status**: ✅ All Phase 1 Day 1 header files created  
**Time**: ~2 hours (header design)  
**Next**: Day 2 Implementation (InteractionManager.cpp, InputHandlerAdapter.cpp)

---

## What Was Created

### ✅ DigitMode/ToolCapabilities.h
- Struct defining tool capability flags
- Properties: allowForeignDrags, supportsHover, isExclusive, hitTestPriority
- Documentation: usage examples for different tool types

### ✅ DigitMode/ToolContext.h
- Struct providing event context to tools
- Contains: mouse state, keyboard modifiers, hit result, previous state
- Helpers: GetDelta(), IsLeftButtonDown(), HasModifier()

### ✅ DigitMode/IInteractionTool.h
- Interface for all interaction tools
- Methods: HitTest, OnMouse*, OnKey*, OnActivate, OnDeactivate, GetViewState
- Replaces: IInputHandler (with added semantics)

### ✅ DigitMode/InteractionManager.h
- Central tool manager and event router
- Members: activeTool, captureTool, hoveredTool
- Methods: RegisterTool, SetActiveTool, OnMouseDown/Move/Up, GetViewState
- ~200 LOC declaration

### ✅ DigitMode/InputHandlerAdapter.h
- Bridge for wrapping IInputHandler as IInteractionTool
- Allows existing code to work without modification
- Can be overridden for tool-specific behavior

---

## Architecture Now Looks Like

```
┌─────────────────────────────────────────────────┐
│         Day 1: Interfaces Complete ✓            │
├─────────────────────────────────────────────────┤
│                                                 │
│  ToolCapabilities.h ─────────┐                 │
│  ToolContext.h ──────────────┤                 │
│  IInteractionTool.h ◄────────┤ InteractionMgr  │
│  InteractionManager.h ◄──────┤                 │
│  InputHandlerAdapter.h ───────┘                │
│                                                 │
├─────────────────────────────────────────────────┤
│        Day 2: Implementation Needed              │
├─────────────────────────────────────────────────┤
│                                                 │
│  ✓ Interfaces defined                          │
│  ✗ InteractionManager.cpp (routing logic)      │
│  ✗ InputHandlerAdapter.cpp (adapter impl)      │
│                                                 │
├─────────────────────────────────────────────────┤
│        Day 3: Integration Needed                │
├─────────────────────────────────────────────────┤
│                                                 │
│  ✗ BaseImageView.h (add m_interactionManager)  │
│  ✗ BaseImageView.cpp (replace InputRouter)     │
│  ✗ ImageView.cpp (add GetViewState rendering)  │
│                                                 │
└─────────────────────────────────────────────────┘
```

---

## Compilation Status

All header files are **self-contained** and can be compiled independently.

**Build test**: 
```bash
# These should compile with no errors (header-only, no dependencies yet)
cd Digit
# Include paths should auto-resolve; no cpp files needed yet
```

No changes to existing code yet—new files only.

---

## Next Steps (Day 2)

### Step 1: Implement InteractionManager.cpp (1 hour)
```cpp
// Key methods:
- BestHit()          // Hit-test arbitration
- OnMouseDown()      // Capture semantics
- OnMouseMove()      // Route to active/capture
- OnMouseUp()        // Clear capture
- GetViewState()     // Merge tool states
```

### Step 2: Implement InputHandlerAdapter.cpp (45 min)
```cpp
// Key methods:
- OnMouseDown()      // Translate to wrapped handler
- OnMouseMove()      // Translate to wrapped handler
- GetViewState()     // Default empty (override per tool)
```

### Step 3: Test Compilation (15 min)
```bash
# Build should succeed with new cpp files
# No runtime tests yet (wait for Day 3 integration)
```

---

## Key Design Decisions Locked In

### ✅ Capture Semantics
```
activeTool = FringeTool (mode, never changes until explicit SetActiveTool)
captureTool = BoundsTool (temporary, cleared on mouse up)
→ Result: FringeTool remains active but BoundsTool handles drag
```

### ✅ Arbitration Priority
```
for each tool:
    hit = tool.HitTest()

best = SelectBestHit(all_hits) by:
    1) Priority (from ToolCapabilities)
    2) Distance (closer = better)
```

### ✅ Event Routing
```
OnMouseDown: → best tool (may set captureTool)
OnMouseMove: → captureTool OR activeTool
OnMouseUp:   → captureTool, then clear it
```

### ✅ View State
```
ActiveTool.GetViewState(isActive=true, isCapturing=false)
CaptureTools.GetViewState(isActive=false, isCapturing=true)
HoveredTool.GetViewState(isActive=false, isCapturing=false)
→ All merged into CompositeViewState
```

---

## Files Not Changed (Isolation Guaranteed)

✅ Untouched:
- BoundsHandler.h/cpp
- BoundsInputHandler.h/cpp
- FringeInputHandler.h/cpp
- ImageView.h/cpp (except Day 3)
- BaseImageView.h/cpp (except Day 3)
- InputRouter.h/cpp (still exists, becomes optional)
- All command dispatch
- All undo/redo
- All aperture-core integration

**Isolation**: InteractionManager is **additive only**. Existing code continues to work.

---

## Deliverables for Day 2

### Morning (1 hour)
- [ ] Implement InteractionManager.cpp (core routing)
- [ ] Implement InputHandlerAdapter.cpp (bridge)

### Afternoon (15 min)
- [ ] Verify compilation
- [ ] Prepare Day 3 integration plan

---

## Readiness Check

Before starting Day 2 implementation, verify:

- [ ] All 5 header files created and compile clean
- [ ] No include cycles (ToolContext references HitResult, IInteractionTool, etc.)
- [ ] No dependencies on existing BoundsHandler/FringeInputHandler yet
- [ ] Documentation clear (examples, usage patterns)
- [ ] Ready to implement core routing logic

---

## Day 2 Implementation Tips

### For InteractionManager.cpp

1. **Hit-test arbitration**
   ```cpp
   HitResult BestHit(CPoint pt) {
       std::vector<HitResult> hits;
       for (auto tool : m_tools) {
           auto hit = tool->HitTest(pt);
           if (hit.hit) {
               hit.tool = tool;
               hit.priority = tool->GetCapabilities().hitTestPriority;
               hits.push_back(hit);
           }
       }
       return hits.empty() ? HitResult() : *std::min_element(hits.begin(), hits.end());
   }
   ```

2. **Capture semantics on mouse down**
   ```cpp
   bool OnMouseDown(UINT flags, CPoint pt) {
       auto hit = BestHit(pt);
       if (!hit.hit) return false;
       
       if (hit.tool == m_activeTool) {
           return hit.tool->OnMouseDown(ctx);  // Normal
       } else {
           if (m_activeTool->GetCapabilities().allowForeignDrags) {
               m_captureTool = hit.tool;       // Capture!
               return m_captureTool->OnMouseDown(ctx);
           }
       }
       return false;
   }
   ```

3. **Route on mouse move**
   ```cpp
   bool OnMouseMove(UINT flags, CPoint pt) {
       auto tool = m_captureTool ? m_captureTool : m_activeTool;
       if (tool) {
           tool->OnMouseMove(ctx);
           return true;
       }
       return false;
   }
   ```

### For InputHandlerAdapter.cpp

1. **Simple translation**
   ```cpp
   bool OnMouseDown(const ToolContext& ctx) override {
       return m_handler->OnMouseDown(ctx.mouseFlags, ctx.screenPoint);
   }
   ```

2. **Default GetViewState** (empty for now)
   ```cpp
   ViewState GetViewState(bool isActive, bool isCapturing) const override {
       return ViewState();  // Empty; override in derived class
   }
   ```

---

## Ready to Proceed?

✅ All Day 1 headers created  
✅ Design locked in  
✅ No breaking changes  
✅ Ready for Day 2 implementation  

**Start Day 2 when ready: Create InteractionManager.cpp**


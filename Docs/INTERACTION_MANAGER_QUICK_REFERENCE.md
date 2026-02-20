# Quick Reference: InteractionManager vs. Current Architecture

## The One-Slide Difference

### Current (InputRouter)
```
One active tool
↓
All events → active tool
↓
No cross-tool interaction
```

### Needed (InteractionManager)
```
One active tool + optional captureTool
↓
All events → best-hit tool (may be different from active)
↓
Cross-tool interaction without mode switch ✓
```

---

## The One Problem It Solves

**User Story**: "I want to move a bounds shape while the Fringe tool is active"

### Current: ❌ Impossible
```
User: Fringe tool active
User: Clicks on bound
Result: Nothing happens
Why: FringeInputHandler is active, BoundsInputHandler not called
Fix: Must manually switch to Bounds tool first
```

### After InteractionManager: ✅ Seamless
```
User: Fringe tool active  
User: Clicks on bound
InteractionManager:
  - Hit-tests: Fringe hit, Bounds hit
  - Picks: Bounds hit (highest priority)
  - Sets: captureTool = BoundsTool (temporary!)
  - Fringe stays active (mode unchanged)
Result: Bound draggable, Fringe remains active
```

---

## The Seven Implementation Gaps It Addresses

| # | Gap | Current | After InteractionManager |
|---|-----|---------|--------------------------|
| 1 | Visual feedback scattered in OnDraw | ❌ | ✅ Unified GetViewState() |
| 2 | Drag-creation not coordinated | ❌ | ✅ ToolContext provides state |
| 3 | Delete mode only if tool active | ❌ | ✅ Works via capture |
| 4 | Can't move shapes of other tools | ❌ | ✅ captureTool handles it |
| 5 | OnDraw has if-spaghetti | ❌ | ✅ Single render loop |
| 6 | Modifier keys not accessible | ❌ | ✅ In ToolContext |
| 7 | Auto-commit drag detection scattered | ❌ | ✅ Previous position in context |

---

## Core Concepts (5 minutes to understand)

### ToolCapabilities
```cpp
struct ToolCapabilities {
    bool allowForeignDrags = true;  // Can other tools capture input?
    int hitTestPriority = 100;       // Priority for arbitration
    bool isExclusive = false;        // Must this tool be active?
};

// Fringe: allowForeignDrags=true  → bounds can steal input
// Bounds: allowForeignDrags=true  → fringe can steal input
// Fiducials (future): isExclusive=true → must be active to use
```

### ToolContext
```cpp
struct ToolContext {
    UINT mouseFlags;        // What buttons are pressed
    CPoint screenPoint;     // Where mouse is
    bool shiftKey;          // Modifier state
    bool altKey;
    HitResult hit;          // What was hit
    CPoint previous;        // Last position (for deltas)
};

// Tools get everything they need, no global state queries
```

### captureTool
```cpp
// CAPTURE = Temporary input redirection
// NOT mode switch

m_activeTool = FringeTool        // Current mode
m_captureTool = BoundsTool       // Temporary drag receiver

OnMouseMove → routes to m_captureTool
OnMouseUp → clears m_captureTool, m_activeTool unchanged
```

---

## Implementation: Two Phases

### Phase 1: Get It Working (2-3 days)
1. Create `InteractionManager` class
2. Create `IInteractionTool` interface  
3. Create `InputHandlerAdapter` (wraps existing handlers)
4. Replace InputRouter calls with InteractionManager calls
5. Test: bounds draggable while fringe active

✅ Zero changes to existing domain logic  
✅ Current workflows unchanged  
✅ New capabilities work  

### Phase 2: Optimize (1-2 days, optional)
1. Native BoundsTool (replaces adapter)
2. Native FringeTool (replaces adapter)
3. Fiducials tool as proof of exclusive tool pattern

---

## Files to Create (from Spec)

```cpp
// Core
DigitMode/IInteractionTool.h          // Tool interface
DigitMode/InteractionManager.h/cpp    // Central routing
DigitMode/ToolCapabilities.h          // Capability struct
DigitMode/ToolContext.h               // Event context struct

// Bridge (for Phase 1)
DigitMode/InputHandlerAdapter.h/cpp   // Wrap existing handlers

// (Optional Phase 2)
DigitMode/BoundsTool.h/cpp            // Native tool
DigitMode/FringeTool.h/cpp            // Native tool
```

---

## Integration Points (Minimal Changes)

### BaseImageView.h
```cpp
// Add member:
DigitMode::InteractionManager m_interactionManager;
```

### BaseImageView.cpp (message handlers)
```cpp
// Replace:
// m_inputRouter.OnMouseDown(nFlags, point);
// With:
m_interactionManager.OnMouseDown(nFlags, point);

// Same for Move, Up, Wheel, Key, Cancel
```

### ImageView.cpp (OnDraw)
```cpp
// Add after existing rendering:
auto viewState = m_interactionManager.GetViewState();
for (auto& shape : viewState.shapes) {
    m_shapeDrawDispatcher.Draw(...);
}
SetCursor(viewState.cursor);
GetMainFrame()->SetStatusText(viewState.statusText);
```

---

## Risk & Confidence

### Risk Level: ✅ **LOW**

- ✅ Incremental (wrap existing code, don't change it)
- ✅ Backward compatible (InputHandlerAdapter bridges old/new)
- ✅ Testable (fake tools verify manager logic)
- ✅ Rollback easy (revert to InputRouter if needed)

### Confidence: ✅ **VERY HIGH**

- ✅ Pattern proven in CAD systems (Rhino, Maya, AutoCAD)
- ✅ Design validated against 7 identified gaps
- ✅ Zero changes to domain logic (BoundsHandler, etc.)
- ✅ Performance not a concern (hit-testing is fast)

---

## Before/After Code Example

### Current
```cpp
void CImageView::OnDraw(CDC* pDC) {
    DrawBounds(pDrawDC);
    DrawDigitInfo(pDrawDC);
    
    // Scattered preview logic
    if (m_boundsHandler.GetBoundsHandler().IsDrafting()) {
        const auto* preview = boundsHandler.GetDraftPreview();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Draft;
            m_shapeDrawDispatcher.Draw(*preview, *pDrawDC, style, m_viewTransform);
        }
    }
    
    if (m_boundsHandler.GetBoundsHandler().IsDragging()) {
        const auto* previewShape = boundsHandler.GetPreviewShape();
        if (previewShape) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Selected;
            m_shapeDrawDispatcher.Draw(*previewShape, *pDrawDC, style, m_viewTransform);
        }
    }
}
```

### After
```cpp
void CImageView::OnDraw(CDC* pDC) {
    DrawBounds(pDrawDC);
    DrawDigitInfo(pDrawDC);
    
    // Unified rendering
    auto viewState = m_interactionManager.GetViewState();
    for (const auto& shape : viewState.shapes) {
        m_shapeDrawDispatcher.Draw(
            *shape.shape, 
            *pDrawDC, 
            shape.style, 
            m_viewTransform
        );
    }
    
    SetCursor(viewState.cursor);
    GetMainFrame()->SetStatusText(viewState.statusText);
}
```

✅ Cleaner, extensible, no scattered logic

---

## Decision Checklist

- [ ] Understand why InputRouter is insufficient (cross-tool interaction)
- [ ] Agree with ToolCapabilities/ToolContext design
- [ ] Accept Phase 1 minimal implementation (2-3 days)
- [ ] Comfortable with InputHandlerAdapter bridge approach
- [ ] Understand risk is LOW
- [ ] Ready to proceed?

---

## Next Action

**To Proceed**:
1. Review `INTERACTION_MANAGER_SPECIFICATION.md` for detailed API
2. Review `ARCHITECTURE_ANALYSIS_INTERACTION_MANAGER.md` for rationale
3. Decide: Start Phase 1 implementation?

**Questions**:
- Design feedback?
- Timeline concerns?
- Prefer to keep InputRouter (accept cross-tool limitation)?

---


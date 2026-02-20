# Architecture Analysis: Current vs. Needed (InteractionManager)

**Status**: Review of current InputRouter-based architecture against InteractionManager principles from `copilot_zzz.md`

---

## Current Architecture (InputRouter Pattern)

```
CBaseImageView::OnLButtonDown()
  ↓
InputRouter::OnMouseDown(flags, pt)
  ├─ activeTool.OnMouseDown(flags, pt) → bool
  └─ if !consumed: navigation.OnMouseDown(flags, pt) → bool
  
View calls: Invalidate(FALSE)
```

### What We Have Today

| Component | Implemented | Status |
|-----------|-------------|--------|
| **InputRouter** | ✅ | Passive delegation only |
| **IInputHandler** | ✅ | Implemented by BoundsInputHandler, FringeInputHandler |
| **BoundsInputHandler** | ✅ | Wraps BoundsHandler, dispatches to domain logic |
| **BoundsHandler** | ✅ | Domain logic (state machine, drag handling, command dispatch) |
| **View Invalidation** | ✅ | Automatic via BaseImageView after all events |
| **Capture Semantics** | ❌ | Not implemented (MFC SetCapture used, but tool state not managed) |
| **Cross-tool Drag** | ❌ | Not supported (each tool is exclusive) |
| **Global Hover** | ❌ | Scatter-coded in BoundsHandler |
| **Visual Feedback** | ⚠️ | Hardcoded in OnDraw, not coordinated with interaction state |

---

## The Gap: What InputRouter Doesn't Handle

### 1. **Tool Capture Semantics**
**Problem**: No mechanism for temporary tool capture without mode switch.

**Example**: 
```
User: Fringe tool active
User: Clicks on bound shape
Current: Nothing happens (event not consumed, or consumed but ignored)
Needed: BoundsTool becomes temporary captureTool
        User drags bound
        On release: BoundsTool releases capture, Fringe remains active
```

**Current Code**:
```cpp
// BoundsInputHandler::HandleSelectModeMouseDown
if (hit.hit && hit.isBody()) {
    m_boundsHandler.BeginDrag(hit.type, hit.shapeIndex, -1, pt);
    return true;  // Consumed
}
```

**Problem**: This only works if BoundsInputHandler is the active tool. If FringeInputHandler is active, this code never runs.

---

### 2. **Tool Exclusivity vs. Cross-Tool Interaction**

**Current Model**: One active tool, all events go there.

**Needed Model**:
```
struct ToolCapabilities {
    bool allowForeignDrags = true;  // Can other tools temporarily take control?
    bool supportsHover = true;       // Should hover show this tool's UI?
    bool exclusiveSelection = false; // Does selecting require mode switch?
};

// Fringe: supportsHover=true, allowForeignDrags=true → can show bound hover hints
// Bounds: supportsHover=true, allowForeignDrags=true → can show fringe hover hints
// Fiducials (future): exclusiveSelection=true, allowForeignDrags=false → must activate tool
```

**Current**: No capability negotiation. Each handler assumes it's active.

---

### 3. **Hit-Testing Arbitration**

**Problem**: Only active tool hit-tests. Foreign tools are invisible.

**Current Flow**:
```cpp
// Only FringeInputHandler runs if Fringe is active
// BoundsInputHandler never sees the bounds, so can't highlight them
```

**Needed Flow**:
```
InteractionManager::OnMouseMove(pt)
  for each tool in toolset:
      hit = tool.HitTest(pt)
  
  best_hit = SelectBestHit(all_hits)  // Priority order
  
  if best_hit.tool != activeTool:
      // Foreign tool hover
      if activeTool.allowForeignDrags:
          Render foreign tool highlight
  else:
      // Active tool hover
      Render active tool UI
```

---

### 4. **Capture vs. Active Tool State**

**Current**: `m_bCaptured` flag in BaseImageView tracks if view has focus.

**Needed**: Separate capture state from active tool state.

```cpp
class InteractionManager {
public:
    IInteractionTool* m_activeTool;      // Current mode (Fringe, Bounds, etc.)
    IInteractionTool* m_captureTool;     // Who has capture (may differ from active)
    
    // On mouse down: may set captureTool ≠ activeTool
    // On mouse move: route to captureTool if set, else activeTool
    // On mouse up: release captureTool, but activeTool stays the same
};
```

**Why This Matters**:
- Fringe tool active
- User drags on bound (captureTool = BoundsTool)
- User releases mouse (captureTool = nullptr, activeTool still Fringe)
- Status bar doesn't change, mode doesn't switch, but bound was moved

---

### 5. **Visual Feedback Coordination**

**Current**: Scattered logic
- Status bar messages in command handlers (ImageView.cpp)
- Cursor management: not visible
- Tooltips: initialized but not tied to interaction state
- Preview rendering: hardcoded in OnDraw

**Needed**: Unified view state from interaction context

```cpp
struct InteractionViewState {
    CString statusText;
    HCURSOR cursor;
    bool showPreview;
    bool showHandles;
    // ... derived from: activeTool + captureTool + hoverTool
};

InteractionManager::GetViewState() → InteractionViewState
CImageView::OnDraw() → Render based on viewState
```

---

## How to Fix Each Documented Gap

Using InteractionManager pattern from copilot_zzz.md:

### Gap 1: Drag-Based Bounding Box ✅ Becomes Natural

**Current Problem**: OnMouseDown starts drag, but view state management is scattered.

**With InteractionManager**:
```cpp
InteractionManager::OnMouseDown(pt)
  hit = captureTool.HitTest(pt)  // Determine if dragging body or handle
  captureTool.OnMouseMove()      // Start drag operation
  
InteractionManager::OnMouseMove(pt)
  if captureTool:
      captureTool.OnMouseMove()  // Update preview live
      RequestViewUpdate()        // Unified invalidation request
  
InteractionManager::OnMouseUp(pt)
  captureTool.OnMouseUp()        // End drag, dispatch command
  captureTool = nullptr
```

---

### Gap 2: Visual Feedback During Creation ✅ Becomes Automatic

**Current Problem**: Preview rendering logic split between BoundsHandler and OnDraw.

**With InteractionManager**:
```cpp
struct ToolViewState {
    const Shape* previewShape;
    const DraftShape* draftShape;
    bool showHandles;
    ShapeEditMode editMode;
};

InteractionManager::GetViewState()
  → Calls activeTool.GetViewState()
  → View renders unified state
```

No more "if IsDrafting() then… if IsDragging() then…" scattered logic.

---

### Gap 3: Delete Mode ✅ Becomes Coordinated

**Current**: Delete mode works if BoundsTool is active.

**With InteractionManager**:
```cpp
InteractionManager::SetActiveTool(BoundsTool)
  activeTool.SetEditMode(Delete)
  
InteractionManager::OnMouseDown(pt)
  hit = activeTool.HitTest(pt)
  activeTool.OnMouseDown(pt, hit)  // BoundsTool handles it, dispatches RemoveShapeCommand
```

No special case needed—just coordinated tool + mode + event flow.

---

### Gap 4: Shape Movement & Body Drag ✅ Fixed by Capture Semantics

**Current Problem**: 
- If BoundsTool is active: body drag works
- If FringeTool is active: body drag never seen

**With InteractionManager**:
```cpp
InteractionManager::OnMouseDown(pt)
  // Hit-test ALL tools
  fringe_hit = fringeTool.HitTest(pt)
  bounds_hit = boundsTool.HitTest(pt)
  
  best = SelectBestHit({fringe_hit, bounds_hit})  // e.g., bounds_hit wins
  
  if best.tool != activeTool and activeTool.allowForeignDrags:
      captureTool = boundsTool  // Temporary capture!
  else:
      captureTool = activeTool
  
  captureTool.OnMouseDown(pt, best)
```

**Result**: User can move bounds even when Fringe tool is active.

---

### Gap 5: Visual Integration ✅ Centralized

**Current**: `OnDraw()` has scattered logic:
```cpp
if (m_boundsHandler.GetBoundsHandler().IsDrafting()) { ... }
if (m_boundsHandler.GetBoundsHandler().IsDragging()) { ... }
```

**With InteractionManager**:
```cpp
CImageView::OnDraw(CDC* pDC)
  InteractionViewState state = m_interactionManager.GetViewState();
  
  if (state.showPreview && state.previewShape) {
      m_shapeDrawDispatcher.Draw(*state.previewShape, ...);
  }
  
  if (state.showHandles && state.previewShape) {
      m_shapeDrawDispatcher.DrawHandles(*state.previewShape, ...);
  }
  
  SetCursor(state.cursor);
  GetMainFrame()->SetStatusText(state.statusText);
```

Single cohesive rendering pipeline.

---

### Gap 6: Modifier Keys ✅ Coordinated Through Context

**Current**: No modifier key handling visible.

**With InteractionManager**:
```cpp
struct ToolContext {
    UINT mouseFlags;      // MK_LBUTTON, MK_RBUTTON, etc.
    UINT modifiers;       // GetKeyState() for Shift, Alt, Ctrl
    const HitResult& hit; // What was hit
};

captureTool.OnMouseMove(toolContext)
  // Inside BoundsTool: can access context.modifiers
  if (context.modifiers & MK_SHIFT) {
      ApplyConstraint(SquareAspect);  // Or CircleConstraint
  }
```

---

### Gap 7: Auto-Commit ✅ Tool Responsibility

**Current**: Drag detection scattered.

**With InteractionManager**:
```cpp
InteractionManager::OnMouseUp(pt)
  captureTool.OnMouseUp(pt, context)
  
// Inside BoundsTool::OnMouseUp:
if (isDraftDragging) {
    distance = Distance(dragStart, pt)
    if (distance < DRAG_THRESHOLD) {
        // Click, not drag: keep draft active for point-sequence
        EndDraftDrag()
    } else {
        // Drag detected: auto-commit
        CommitDraft()
    }
}
```

---

## Summary: The Missing Piece

| Capability | InputRouter | InteractionManager |
|-----------|-------------|-------------------|
| **Passive event delegation** | ✅ | ✅ (+ active) |
| **Tool activation** | ✅ | ✅ |
| **Cross-tool drag capture** | ❌ | ✅ |
| **Global hit-testing** | ❌ | ✅ |
| **Tool capability negotiation** | ❌ | ✅ |
| **Unified view state** | ❌ | ✅ |
| **Coordinated visual feedback** | ❌ | ✅ |
| **Scalable to N tools** | ⚠️ Limited | ✅ Yes |

---

## Recommended Next Step

**Do NOT refactor existing code yet.** Instead:

1. **Design phase** (1-2 hours):
   - Define `IInteractionTool` interface (refactor from IInputHandler)
   - Define `ToolCapabilities` struct
   - Define `ToolContext` struct
   - Design `InteractionManager` class skeleton

2. **Proof of concept** (2-3 hours):
   - Implement InteractionManager
   - Adapt ONE handler (BoundsInputHandler) to implement IInteractionTool
   - Test cross-tool drag scenario: Fringe active, move bound

3. **Incremental migration** (N hours):
   - Migrate FringeInputHandler to IInteractionTool
   - Migrate visual feedback coordination
   - Add capability negotiation for future tools (Fiducials, etc.)

**Benefit**: Current code stays working. New architecture proven incrementally.

---

## Files to Create/Modify

### New Files
- `DigitMode/IInteractionTool.h` — Replace IInputHandler for tools
- `DigitMode/InteractionManager.h/cpp` — Central routing + state
- `DigitMode/ToolCapabilities.h` — Capability struct
- `DigitMode/ToolContext.h` — Event context struct

### Refactor (Phase 2)
- `DigitMode/BoundsHandler.h` — Adapter to IInteractionTool
- `DigitMode/FringeInputHandler.h` — Adapter to IInteractionTool
- `ImageTempl/ImageView.cpp` — Replace InputRouter with InteractionManager
- `ImageTempl/ImageView.h` — Add m_interactionManager member

---

## Architecture After Refactor

```
CBaseImageView::OnLButtonDown(UINT flags, CPoint pt)
  ├─ m_inputRouter.Cancel()         // Still handles ESC, navigation
  └─ m_interactionManager.OnMouseDown(flags, pt)
      ├─ HitTest all tools
      ├─ Select best hit + determine capture
      ├─ Forward to captureTool (or activeTool)
      └─ Request view update (returns UpdateRequest)
      
CBaseImageView::OnMouseMove(UINT flags, CPoint pt)
  └─ m_interactionManager.OnMouseMove(flags, pt)
      ├─ Route to captureTool or activeTool
      ├─ Update hover state
      └─ Request view update
      
CImageView::OnDraw(CDC* pDC)
  ├─ InteractionViewState state = m_interactionManager.GetViewState()
  ├─ Render committed shapes (unchanged)
  ├─ Render preview, handles, hover (from state)
  └─ Update cursor, status bar (from state)
```

This is **incremental, low-risk, high-value**. Current code keeps working while new architecture proves itself.


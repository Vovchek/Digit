# Day 3 Integration Plan: Wire InteractionManager into View

**Status**: ⏳ READY (Day 2 complete, Day 3 ready to start)  
**Timeline**: ~2 hours  
**Complexity**: Medium (straightforward integration, minimal logic)  
**Risk**: LOW (targeted, isolated changes)

---

## Day 3 Overview

After Day 2 (manager fully implemented), Day 3 will:
1. Create tool adapters wrapping existing handlers
2. Integrate manager into CBaseImageView
3. Integrate manager into CImageView rendering
4. Test end-to-end: cross-tool drag

---

## Step 1: Create BoundsToolAdapter (30 min)

**File**: `DigitMode/BoundsToolAdapter.h` (new)

```cpp
#pragma once
#include "InputHandlerAdapter.h"
#include <memory>

namespace DigitMode {

class BoundsInputHandler;
class BoundsHandler;

/**
 * @brief Specialized adapter for BoundsInputHandler
 * 
 * Wraps BoundsInputHandler (IInputHandler) as IInteractionTool.
 * Provides hit-testing and view state generation for bounds editing.
 */
class BoundsToolAdapter : public InputHandlerAdapter {
public:
    BoundsToolAdapter(BoundsInputHandler* boundsHandler, BoundsHandler* handler);
    
    // Override with bounds-specific implementations
    HitResult HitTest(CPoint screenPt, int tolerance = 5) override;
    IInteractionTool::ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
private:
    BoundsHandler* m_boundsHandler;  // For domain logic
};

}  // namespace DigitMode
```

**File**: `DigitMode/BoundsToolAdapter.cpp` (new)

```cpp
#include "stdafx.h"
#include "BoundsToolAdapter.h"
#include "BoundsInputHandler.h"
#include "BoundsHandler.h"

namespace DigitMode {

BoundsToolAdapter::BoundsToolAdapter(
    BoundsInputHandler* boundsHandler,
    BoundsHandler* handler)
    : InputHandlerAdapter(boundsHandler, "BoundsTool")
    , m_boundsHandler(handler)
{
}

HitResult BoundsToolAdapter::HitTest(CPoint screenPt, int tolerance)
{
    if (!m_boundsHandler) {
        return HitResult();
    }
    
    // Delegate to BoundsHandler's HitTest
    auto hit = m_boundsHandler->HitTest(screenPt, tolerance);
    return hit;
}

IInteractionTool::ViewState BoundsToolAdapter::GetViewState(bool isActive, bool isCapturing) const
{
    ViewState state;
    
    if (!m_boundsHandler) {
        return state;
    }
    
    // Draw draft preview if drafting
    if (m_boundsHandler->IsDrafting()) {
        const auto* preview = m_boundsHandler->GetDraftPreview();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Draft;
            style.type = m_boundsHandler->GetShapeType();
            style.showHandles = false;
            state.shapes.push_back({preview, style});
        }
    }
    
    // Draw preview with handles if dragging
    if (m_boundsHandler->IsDragging() && isCapturing) {
        const auto* preview = m_boundsHandler->GetPreviewShape();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Selected;
            style.type = m_boundsHandler->GetHoveredShapeType();
            style.showHandles = true;
            state.shapes.push_back({preview, style});
        }
    }
    
    return state;
}

}  // namespace DigitMode
```

---

## Step 2: Create FringeToolAdapter (30 min)

Similar to BoundsToolAdapter but for FringeInputHandler.

---

## Step 3: Integrate with CBaseImageView (45 min)

**File**: `ImageTempl/BaseImageView.h`

Add member:
```cpp
private:
    DigitMode::InteractionManager m_interactionManager;
```

**File**: `ImageTempl/BaseImageView.cpp`

Replace InputRouter calls in message handlers:

```cpp
// OLD:
// m_inputRouter.OnMouseDown(nFlags, point);

// NEW:
m_interactionManager.OnMouseDown(nFlags, point);
```

Same for: OnMouseMove, OnMouseUp, OnKeyDown, Cancel

---

## Step 4: Integrate with CImageView (30 min)

**File**: `ImageTempl/ImageView.cpp`

In `OnDraw()`, after existing rendering:

```cpp
// Get unified view state from InteractionManager
auto viewState = m_interactionManager.GetViewState();

// Render shapes from all tools
for (const auto& layer : viewState.layers) {
    for (const auto& shape : layer.toolState.shapes) {
        m_shapeDrawDispatcher.Draw(
            *shape.shape,
            *pDrawDC,
            shape.style,
            m_viewTransform
        );
    }
}

// Update UI
if (viewState.cursor) {
    ::SetCursor(viewState.cursor);
}
if (!viewState.statusText.IsEmpty()) {
    GetMainFrame()->SetStatusText(viewState.statusText);
}
```

---

## Step 5: Initialize in OnInitialUpdate (30 min)

**File**: `ImageTempl/ImageView.cpp` in `OnInitialUpdate()`

```cpp
// Create tool adapters
auto boundsAdapter = std::make_unique<BoundsToolAdapter>(
    &m_boundsHandler,
    &m_boundsHandler.GetBoundsHandler()
);

auto fringeAdapter = std::make_unique<FringeToolAdapter>(
    &m_fringeHandler,
    &m_fringeHandler.GetInputHandler()
);

// Register tools
m_interactionManager.RegisterTool(boundsAdapter.get());
m_interactionManager.RegisterTool(fringeAdapter.get());

// Set initial active tool (Fringe)
m_interactionManager.SetActiveTool(fringeAdapter.get());

// Keep references (or use make_shared if storing)
m_boundsToolAdapter = std::move(boundsAdapter);
m_fringeToolAdapter = std::move(fringeAdapter);
```

---

## Expected Test Scenario

After Day 3 complete:

```
1. Start app, image loaded
2. Fringe tool is active (default)
3. Create a bounds shape (click points or drag)
4. Switch to Fringe tool
5. Hover over bounds shape → See it highlighted ✓
6. Click and drag bounds shape → Shape moves ✓
7. Release → Fringe tool still active ✓
```

**All at once - no mode switching!**

---

## Success Criteria for Day 3

### Functional
- [x] *(Day 2)* InteractionManager fully implemented
- [ ] *(Day 3)* BoundsToolAdapter created and working
- [ ] *(Day 3)* FringeToolAdapter created and working
- [ ] *(Day 3)* Integrated with CBaseImageView
- [ ] *(Day 3)* Integrated with CImageView
- [ ] *(Day 3)* Bounds draggable while Fringe active

### Code Quality
- [ ] No changes to BoundsHandler/FringeInputHandler
- [ ] Clean adapter bridges
- [ ] Minimal view integration
- [ ] All compiling cleanly

### Testing
- [ ] Cross-tool drag verified
- [ ] All 7 gaps addressed (by architecture)
- [ ] No regressions
- [ ] Performance acceptable

---

## Files to Create/Modify (Day 3)

### New Files
- `DigitMode/BoundsToolAdapter.h/cpp` (2 files)
- `DigitMode/FringeToolAdapter.h/cpp` (2 files)

### Modified Files
- `ImageTempl/BaseImageView.h` (add member)
- `ImageTempl/BaseImageView.cpp` (4 message handlers)
- `ImageTempl/ImageView.h` (add members for adapters)
- `ImageTempl/ImageView.cpp` (OnInitialUpdate + OnDraw)

---

## Important Note: NavigationInputHandler

**NavigationInputHandler** should NOT become a tool.

Why:
- Pan/zoom are **global navigation**, not mode-switching tools
- They should be handled **before** tools (not competing with them)
- According to copilot_zzz.md: "Pan/Zoom — DO NOT duplicate!"

**Solution**: Keep NavigationInputHandler separate
- InputRouter still manages it (or it's called before InteractionManager)
- Tools never see pan/zoom events
- This prevents duplication and conflicts

---

## Timeline Summary

```
Day 1: ✅ Headers designed (2 hours)
Day 2: ✅ Implementation (2 hours)
Day 3: ⏳ Integration (~2 hours)
   ├─ BoundsToolAdapter: 30 min
   ├─ FringeToolAdapter: 30 min
   ├─ CBaseImageView: 45 min
   ├─ CImageView: 30 min
   └─ Testing: 15 min
```

**Total**: ~6 hours for complete Phase 1

---

## Rollback Plan

If Day 3 has issues:
1. Revert adapter files
2. Revert BaseImageView changes
3. Revert ImageView changes
4. Go back to InputRouter (still exists)

**Estimated time to rollback**: 30 minutes

---

## Next: Start Day 3

Ready to create BoundsToolAdapter and FringeToolAdapter?


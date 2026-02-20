# Day 2 Implementation: InteractionManager.cpp & InputHandlerAdapter.cpp

**Timeline**: 2 hours  
**Complexity**: Medium (straightforward routing logic)  
**Risk**: LOW (localized, testable logic)

---

## File 1: InteractionManager.cpp (1 hour)

Location: `DigitMode/InteractionManager.cpp`

Core methods to implement:

### 1. Constructor/Destructor
```cpp
InteractionManager::InteractionManager()
    : m_activeTool(nullptr), m_captureTool(nullptr), m_hoveredTool(nullptr),
      m_requestInvalidate(false)
{
}

InteractionManager::~InteractionManager()
{
}
```

### 2. RegisterTool (initialization)
```cpp
void InteractionManager::RegisterTool(IInteractionTool* tool)
{
    if (!tool) return;
    m_tools.push_back(tool);
}
```

### 3. SetActiveTool (mode switching)
```cpp
void InteractionManager::SetActiveTool(IInteractionTool* tool)
{
    if (m_activeTool == tool) return;  // No-op if same
    
    if (m_activeTool) {
        m_activeTool->OnDeactivate();
    }
    
    m_activeTool = tool;
    m_captureTool = nullptr;  // Clear capture on mode switch
    
    if (m_activeTool) {
        m_activeTool->OnActivate();
    }
}
```

### 4. BestHit (arbitration core)
```cpp
HitResult InteractionManager::BestHit(CPoint screenPt, int tolerance) const
{
    std::vector<HitResult> hits;
    
    // Ask all tools for hit results
    for (auto tool : m_tools) {
        auto hit = tool->HitTest(screenPt, tolerance);
        if (hit.hit) {
            hit.tool = tool;
            hit.priority = tool->GetCapabilities().hitTestPriority;
            hits.push_back(hit);
        }
    }
    
    // No hits
    if (hits.empty()) {
        return HitResult();
    }
    
    // Sort by priority (higher first) then distance (lower first)
    std::sort(hits.begin(), hits.end());
    return hits[0];
}
```

### 5. MakeContext (helper)
```cpp
ToolContext InteractionManager::MakeContext(UINT flags, CPoint pt, const HitResult& hit)
{
    ToolContext ctx;
    ctx.mouseFlags = flags;
    ctx.screenPoint = pt;
    ctx.hit = hit;
    
    // Capture modifier state
    UpdateModifiers(ctx);
    
    // Previous state
    if (m_lastMousePoint != CPoint(0, 0)) {
        ctx.previous.screenPoint = m_lastMousePoint;
        ctx.previous.valid = true;
    }
    m_lastMousePoint = pt;
    
    return ctx;
}
```

### 6. UpdateModifiers (helper)
```cpp
void InteractionManager::UpdateModifiers(ToolContext& ctx)
{
    ctx.shiftKey = (GetKeyState(VK_SHIFT) & 0x80) != 0;
    ctx.altKey = (GetKeyState(VK_MENU) & 0x80) != 0;
    ctx.ctrlKey = (GetKeyState(VK_CONTROL) & 0x80) != 0;
}
```

### 7. OnMouseDown (key method - capture semantics)
```cpp
bool InteractionManager::OnMouseDown(UINT flags, CPoint pt)
{
    if (!m_activeTool) return false;
    
    // Find what was hit
    auto hit = BestHit(pt);
    auto ctx = MakeContext(flags, pt, hit);
    
    if (!hit.hit) {
        // Nothing hit - let navigation handle it
        return false;
    }
    
    // Check if hit belongs to active tool
    if (hit.tool == m_activeTool) {
        // Normal case: active tool handles it
        return m_activeTool->OnMouseDown(ctx);
    }
    
    // Foreign tool hit - check if we can capture
    if (m_activeTool->GetCapabilities().allowForeignDrags) {
        // Capture semantics: temporary tool capture without mode switch
        m_captureTool = hit.tool;
        return m_captureTool->OnMouseDown(ctx);
    }
    
    // Not allowed to capture
    return false;
}
```

### 8. OnMouseMove (routing)
```cpp
bool InteractionManager::OnMouseMove(UINT flags, CPoint pt)
{
    // Route to capture tool if active, else active tool
    auto tool = m_captureTool ? m_captureTool : m_activeTool;
    if (!tool) return false;
    
    auto hit = BestHit(pt);
    auto ctx = MakeContext(flags, pt, hit);
    
    // Update hover
    if (hit.tool != m_hoveredTool) {
        m_hoveredTool = hit.tool;
        m_requestInvalidate = true;
    }
    
    // Route event
    tool->OnMouseMove(ctx);
    
    // Request invalidation for preview updates
    m_requestInvalidate = true;
    return true;
}
```

### 9. OnMouseUp (clear capture)
```cpp
bool InteractionManager::OnMouseUp(UINT flags, CPoint pt)
{
    // Route to tool
    auto tool = m_captureTool ? m_captureTool : m_activeTool;
    if (!tool) return false;
    
    auto ctx = MakeContext(flags, pt, m_lastHit);
    tool->OnMouseUp(ctx);
    
    // Clear capture
    m_captureTool = nullptr;
    
    m_requestInvalidate = true;
    return true;
}
```

### 10. OnMouseWheel (active tool only)
```cpp
bool InteractionManager::OnMouseWheel(UINT flags, short zDelta, CPoint pt)
{
    if (!m_activeTool) return false;
    
    auto ctx = MakeContext(flags, pt);
    return m_activeTool->OnMouseWheel(ctx, zDelta);
}
```

### 11. OnKeyDown (active tool only)
```cpp
bool InteractionManager::OnKeyDown(UINT nChar)
{
    if (!m_activeTool) return false;
    
    ToolContext ctx;
    UpdateModifiers(ctx);
    ctx.screenPoint = m_lastMousePoint;
    
    return m_activeTool->OnKeyDown(ctx, nChar);
}
```

### 12. Cancel (cleanup)
```cpp
void InteractionManager::Cancel()
{
    if (m_activeTool) {
        m_activeTool->Cancel();
    }
    if (m_captureTool) {
        m_captureTool->Cancel();
    }
    m_captureTool = nullptr;
    m_requestInvalidate = true;
}
```

### 13. GetViewState (unified rendering state)
```cpp
InteractionManager::CompositeViewState InteractionManager::GetViewState() const
{
    CompositeViewState state;
    
    // Gather from active tool first
    if (m_activeTool) {
        auto toolState = m_activeTool->GetViewState(true, m_captureTool == nullptr);
        state.layers.push_back({toolState, m_activeTool, true, m_captureTool == nullptr});
    }
    
    // Then from capture tool if different
    if (m_captureTool && m_captureTool != m_activeTool) {
        auto toolState = m_captureTool->GetViewState(false, true);
        state.layers.push_back({toolState, m_captureTool, false, true});
    }
    
    // Then from hovered tool if different from both
    if (m_hoveredTool && m_hoveredTool != m_activeTool && m_hoveredTool != m_captureTool) {
        auto toolState = m_hoveredTool->GetViewState(false, false);
        state.layers.push_back({toolState, m_hoveredTool, false, false});
    }
    
    // Resolve cursor (first non-null)
    for (const auto& layer : state.layers) {
        if (layer.toolState.cursor) {
            state.cursor = layer.toolState.cursor;
            break;
        }
    }
    
    // Resolve status text (prefer active, then capture)
    if (m_activeTool) {
        auto toolState = m_activeTool->GetViewState(true, m_captureTool == nullptr);
        if (!toolState.statusText.IsEmpty()) {
            state.statusText = toolState.statusText;
        }
    }
    if (state.statusText.IsEmpty() && m_captureTool) {
        auto toolState = m_captureTool->GetViewState(false, true);
        if (!toolState.statusText.IsEmpty()) {
            state.statusText = toolState.statusText;
        }
    }
    
    // Resolve tooltip (from hovered, if different from active)
    if (m_hoveredTool && m_hoveredTool != m_activeTool) {
        auto toolState = m_hoveredTool->GetViewState(false, false);
        if (!toolState.tooltip.IsEmpty()) {
            state.tooltip = toolState.tooltip;
        }
    }
    
    // Request invalidation if any tool needs it
    for (const auto& layer : state.layers) {
        if (layer.toolState.requestInvalidate) {
            state.requestInvalidate = true;
            break;
        }
    }
    
    return state;
}
```

### 14. Stub queries (can be enhanced later)
```cpp
bool InteractionManager::IsDragging() const
{
    // Placeholder: can be enhanced to query tools
    return m_captureTool != nullptr;
}

bool InteractionManager::IsDrafting() const
{
    // Placeholder: can be enhanced to query tools
    return false;
}
```

---

## File 2: InputHandlerAdapter.cpp (45 min)

Location: `DigitMode/InputHandlerAdapter.cpp`

```cpp
#include "InputHandlerAdapter.h"
#include "IInputHandler.h"

namespace DigitMode {

InputHandlerAdapter::InputHandlerAdapter(IInputHandler* handler, const char* name)
    : m_handler(handler), m_name(name)
{
}

ToolCapabilities InputHandlerAdapter::GetCapabilities() const
{
    // Default capabilities for wrapped handler
    // Can be overridden in derived classes for specific handlers
    return ToolCapabilities{
        .supportsHover = true,
        .allowForeignDrags = true,
        .requiresExplicitActivation = false,
        .isExclusive = false,
        .hitTestPriority = 100,
        .name = m_name
    };
}

HitResult InputHandlerAdapter::HitTest(CPoint screenPt, int tolerance)
{
    // Default: empty hit (can be overridden in derived classes)
    // TODO: If wrapped handler has HitTest capability, implement here
    return HitResult();
}

bool InputHandlerAdapter::OnMouseDown(const ToolContext& ctx)
{
    if (!m_handler) return false;
    return m_handler->OnMouseDown(ctx.mouseFlags, ctx.screenPoint);
}

void InputHandlerAdapter::OnMouseMove(const ToolContext& ctx)
{
    if (!m_handler) return;
    m_handler->OnMouseMove(ctx.mouseFlags, ctx.screenPoint);
}

void InputHandlerAdapter::OnMouseUp(const ToolContext& ctx)
{
    if (!m_handler) return;
    m_handler->OnMouseUp(ctx.mouseFlags, ctx.screenPoint);
}

bool InputHandlerAdapter::OnMouseWheel(const ToolContext& ctx, short zDelta)
{
    if (!m_handler) return false;
    return m_handler->OnMouseWheel(ctx.mouseFlags, zDelta, ctx.screenPoint);
}

bool InputHandlerAdapter::OnKeyDown(const ToolContext& ctx, UINT nChar)
{
    if (!m_handler) return false;
    return m_handler->OnKeyDown(nChar);
}

void InputHandlerAdapter::Cancel()
{
    if (!m_handler) return;
    m_handler->Cancel();
}

void InputHandlerAdapter::OnActivate()
{
    // Default: empty (can be overridden)
}

void InputHandlerAdapter::OnDeactivate()
{
    // Default: cancel wrapped handler
    if (m_handler) {
        m_handler->Cancel();
    }
}

IInteractionTool::ViewState InputHandlerAdapter::GetViewState(bool isActive, bool isCapturing) const
{
    // Default: empty state
    // Derived classes should override with actual tool-specific state
    return ViewState();
}

}  // namespace DigitMode
```

---

## Compilation Verification

After creating both .cpp files, build should succeed:

```bash
cd Digit
msbuild Digit.sln /p:Configuration=Debug /p:Platform=x64
# Should show: ✓ Build successful
```

---

## Testing (Simple)

Add to test file or temporary code:

```cpp
// Test: Can register tools
DigitMode::InteractionManager mgr;
ASSERT(mgr.GetActiveTool() == nullptr);

// Test: Can set active tool
auto adapter = std::make_unique<InputHandlerAdapter>(&someHandler, "Test");
mgr.RegisterTool(adapter.get());
mgr.SetActiveTool(adapter.get());
ASSERT(mgr.GetActiveTool() == adapter.get());
```

---

## Deliverables for Day 2

- [ ] InteractionManager.cpp (complete, ~250 LOC)
- [ ] InputHandlerAdapter.cpp (complete, ~60 LOC)
- [ ] Build verification (successful)
- [ ] Code review (self-check)

---

## What's Ready for Day 3

After Day 2:
- ✅ Core routing implemented
- ✅ Capture semantics working
- ✅ Hit-test arbitration functional
- ✅ View state composition ready
- ✅ All APIs callable

Ready to integrate with CBaseImageView/CImageView on Day 3.

---


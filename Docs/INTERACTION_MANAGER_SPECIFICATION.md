# InteractionManager Design Specification

**Purpose**: Formalize the tool management and event routing architecture needed to support cross-tool interaction and unified visual feedback.

**Scope**: Replace scattered tool coordination with a single, authoritative interaction manager.

---

## 1. Core Interfaces

### 1.1 ToolCapabilities (Struct)

```cpp
namespace DigitMode {

/**
 * @brief Capability declaration for tools
 * 
 * Allows tools to declare their interaction model without hardcoding
 * tool-specific logic in the manager.
 */
struct ToolCapabilities {
    /// Can this tool accept hover highlights when not active?
    bool supportsHover = true;
    
    /// Can foreign tools temporarily capture input (body drag)?
    bool allowForeignDrags = true;
    
    /// Must this tool be explicitly activated to use?
    /// (vs. being available for background operations)
    bool requiresExplicitActivation = false;
    
    /// When this tool is active, does it exclude other tools from responding?
    /// (Fiducials=true, Fringe=false, Bounds=false)
    bool isExclusive = false;
    
    /// Priority when multiple tools claim the same hit point
    /// Higher = takes precedence. Default = 100
    int hitTestPriority = 100;
};

}
```

---

### 1.2 HitResult (Struct - Enhanced)

```cpp
namespace DigitMode {

/**
 * @brief Result of hit-testing a tool
 * 
 * Unified result structure used by manager to arbitrate between tools.
 */
struct HitResult {
    bool hit = false;
    
    /// Which tool reported this hit
    /// (Set by manager, not by tool)
    IInteractionTool* tool = nullptr;
    
    /// Tool-specific: what was hit (shape index, handle, etc.)
    void* toolContext = nullptr;
    
    /// Distance from query point (for tie-breaking)
    double distance = std::numeric_limits<double>::max();
    
    /// Priority for arbitration (from tool capabilities)
    int priority = 0;
    
    bool operator<(const HitResult& other) const {
        // Higher priority first
        if (priority != other.priority) return priority > other.priority;
        // Closer distance second
        if (distance != other.distance) return distance < other.distance;
        return false;
    }
};

}
```

---

### 1.3 ToolContext (Struct)

```cpp
namespace DigitMode {

/**
 * @brief Event context passed to tools
 * 
 * Provides tools with all information they need without 
 * requiring them to query global state.
 */
struct ToolContext {
    /// Mouse state
    UINT mouseFlags = 0;      // MK_LBUTTON, MK_RBUTTON, etc.
    CPoint screenPoint;        // Current mouse position (screen coords)
    
    /// Keyboard state (read at event time)
    bool shiftKey = false;
    bool altKey = false;
    bool ctrlKey = false;
    
    /// Hit result from manager arbitration
    HitResult hit;
    
    /// Previous state (for computing deltas)
    struct Previous {
        CPoint screenPoint;
        bool valid = false;
    } previous;
    
    /// Request view update?
    bool requestInvalidate = false;
};

}
```

---

### 1.4 IInteractionTool (Interface)

```cpp
namespace DigitMode {

/**
 * @brief Tool interface for interaction system
 * 
 * Replaces IInputHandler. Adds semantics for:
 * - Capability declaration
 * - Context-aware event handling
 * - View state generation
 * - Unified hover/preview rendering
 */
class IInteractionTool {
public:
    virtual ~IInteractionTool() = default;
    
    // ========================================================================
    // Capability & State
    // ========================================================================
    
    /// Declare what this tool can do
    virtual ToolCapabilities GetCapabilities() const = 0;
    
    /// Get human-readable name for debugging/status
    virtual const char* GetName() const = 0;
    
    // ========================================================================
    // Hit-Testing (used for arbitration)
    // ========================================================================
    
    /// Hit-test at screen point
    /// Returns hit.hit=false if nothing hit (not an error)
    virtual HitResult HitTest(CPoint screenPt, int tolerance = 5) = 0;
    
    // ========================================================================
    // Event Handling
    // ========================================================================
    
    /// Mouse button down
    /// Returns true if event was consumed (prevents fallback)
    virtual bool OnMouseDown(const ToolContext& ctx) = 0;
    
    /// Mouse move (with or without button held)
    /// Called for both active tool and capture tool
    virtual void OnMouseMove(const ToolContext& ctx) = 0;
    
    /// Mouse button up
    virtual void OnMouseUp(const ToolContext& ctx) = 0;
    
    /// Mouse wheel
    virtual bool OnMouseWheel(const ToolContext& ctx, short zDelta) = 0;
    
    /// Keyboard down
    virtual bool OnKeyDown(const ToolContext& ctx, UINT nChar) = 0;
    
    /// Cancel current operation (ESC key)
    virtual void Cancel() = 0;
    
    // ========================================================================
    // Activation (Tool Switching)
    // ========================================================================
    
    /// Called when tool becomes active
    /// Safe place to update UI, initialize state
    virtual void OnActivate() = 0;
    
    /// Called when tool becomes inactive
    /// Safe place to cleanup, cancel draft operations
    virtual void OnDeactivate() = 0;
    
    // ========================================================================
    // Visual Feedback
    // ========================================================================
    
    /// Generate view state for rendering
    /// Called during OnDraw to gather all visual elements
    struct ViewState {
        /// Shapes to render
        struct Shape {
            const aperture::Shape* shape = nullptr;
            ShapeDrawStyle style;
        };
        std::vector<Shape> shapes;
        
        /// Cursor to display
        HCURSOR cursor = nullptr;
        
        /// Status bar text
        CString statusText;
        
        /// Tooltip text
        CString tooltip;
        
        /// Request another invalidation pass?
        bool requestInvalidate = false;
    };
    
    /// Gather all visual elements for rendering
    /// @param isActive True if this is the active tool
    /// @param isCapturing True if this tool currently has capture
    virtual ViewState GetViewState(bool isActive, bool isCapturing) const = 0;
};

}
```

---

## 2. InteractionManager

```cpp
namespace DigitMode {

/**
 * @brief Central tool manager and event router
 * 
 * Responsibilities:
 * - Maintain active and capture tool state
 * - Arbitrate hit-testing between tools
 * - Route events to appropriate tool
 * - Coordinate visual feedback
 * - Handle tool activation/deactivation
 */
class InteractionManager {
public:
    InteractionManager();
    ~InteractionManager();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /// Register a tool (does not activate)
    void RegisterTool(IInteractionTool* tool);
    
    /// Set the currently active tool
    void SetActiveTool(IInteractionTool* tool);
    
    /// Get current active tool
    IInteractionTool* GetActiveTool() const { return m_activeTool; }
    
    /// Get tool that currently has capture (may differ from active)
    IInteractionTool* GetCaptureTool() const { return m_captureTool; }
    
    // ========================================================================
    // Event Routing (called by CBaseImageView)
    // ========================================================================
    
    /// Mouse button down
    /// Returns true if event consumed (prevents fallback to navigation)
    bool OnMouseDown(UINT flags, CPoint pt);
    
    /// Mouse move
    bool OnMouseMove(UINT flags, CPoint pt);
    
    /// Mouse button up
    bool OnMouseUp(UINT flags, CPoint pt);
    
    /// Mouse wheel
    bool OnMouseWheel(UINT flags, short zDelta, CPoint pt);
    
    /// Keyboard down
    bool OnKeyDown(UINT nChar);
    
    /// Cancel (ESC key)
    void Cancel();
    
    // ========================================================================
    // View State (called by CImageView::OnDraw)
    // ========================================================================
    
    /// Gather all visual elements for rendering
    struct CompositeViewState {
        struct Layer {
            IInteractionTool::ViewState toolState;
            IInteractionTool* tool = nullptr;
        };
        std::vector<Layer> layers;  // Active tool first, then capture, then others
        
        HCURSOR cursor;
        CString statusText;
        CString tooltip;
    };
    
    /// Get unified view state from all active tools
    CompositeViewState GetViewState() const;
    
    // ========================================================================
    // State Queries
    // ========================================================================
    
    /// Is any tool currently dragging?
    bool IsDragging() const;
    
    /// Is any tool currently creating a draft?
    bool IsDrafting() const;
    
    // ========================================================================
    // Internal Helpers
    // ========================================================================
    
private:
    /// Find best hit across all tools
    HitResult BestHit(const std::vector<HitResult>& hits) const;
    
    /// Build tool context from event data
    ToolContext MakeContext(UINT flags, CPoint pt) const;
    
    /// Update modifier key state
    void UpdateModifiers(ToolContext& ctx);
    
    /// Request view invalidation (typically called by event handlers)
    void RequestInvalidate();
    
    // ========================================================================
    // State
    // ========================================================================
    
    std::vector<IInteractionTool*> m_tools;
    
    IInteractionTool* m_activeTool = nullptr;    // Current mode (Fringe, Bounds, etc.)
    IInteractionTool* m_captureTool = nullptr;   // Who has temporary capture
    IInteractionTool* m_hoveredTool = nullptr;   // Tool whose object is under mouse (for hover highlight)
    
    HitResult m_lastHit;                         // Last hit for computing deltas
    CPoint m_lastMousePoint;                     // Last mouse position
};

}
```

---

## 3. Migration Path: Adapter from IInputHandler

```cpp
namespace DigitMode {

/**
 * @brief Temporary adapter to reuse existing input handlers as tools
 * 
 * Allows incremental migration:
 * 1. Implement IInteractionTool natively
 * 2. Or wrap existing IInputHandler in this adapter
 */
class InputHandlerAdapter : public IInteractionTool {
public:
    InputHandlerAdapter(IInputHandler* handler, const char* name);
    
    // ========================================================================
    // IInteractionTool implementation
    // ========================================================================
    
    ToolCapabilities GetCapabilities() const override;
    const char* GetName() const override { return m_name; }
    
    HitResult HitTest(CPoint screenPt, int tolerance) override;
    bool OnMouseDown(const ToolContext& ctx) override;
    void OnMouseMove(const ToolContext& ctx) override;
    void OnMouseUp(const ToolContext& ctx) override;
    bool OnMouseWheel(const ToolContext& ctx, short zDelta) override;
    bool OnKeyDown(const ToolContext& ctx, UINT nChar) override;
    void Cancel() override;
    
    void OnActivate() override;
    void OnDeactivate() override;
    
    ViewState GetViewState(bool isActive, bool isCapturing) const override;
    
private:
    IInputHandler* m_handler;
    const char* m_name;
};

}
```

---

## 4. Integration with CBaseImageView

### Before (Current)
```cpp
void CBaseImageView::OnLButtonDown(UINT nFlags, CPoint point) {
    SetCapture();
    m_bCaptured = TRUE;
    m_inputRouter.OnMouseDown(nFlags, point);  // ← Passive delegation only
    Invalidate(FALSE);
    CScrollView::OnLButtonDown(nFlags, point);
}
```

### After (With InteractionManager)
```cpp
void CBaseImageView::OnLButtonDown(UINT nFlags, CPoint point) {
    SetCapture();
    m_bCaptured = TRUE;
    m_interactionManager.OnMouseDown(nFlags, point);  // ← Active arbitration
    // No need for explicit Invalidate—manager requests it
    CScrollView::OnLButtonDown(nFlags, point);
}
```

### In CImageView::OnDraw()

```cpp
void CImageView::OnDraw(CDC* pDC) {
    // ... existing image/document rendering ...
    
    // Get unified view state from interaction manager
    auto viewState = m_interactionManager.GetViewState();
    
    // Render all tool elements in order
    for (const auto& layer : viewState.layers) {
        for (const auto& shape : layer.toolState.shapes) {
            m_shapeDrawDispatcher.Draw(
                *shape.shape, 
                *pDC, 
                shape.style, 
                m_viewTransform
            );
        }
    }
    
    // Update UI elements
    SetCursor(viewState.cursor);
    GetMainFrame()->SetStatusText(viewState.statusText);
    
    // Handle invalidation request
    if (viewState.layers[0].toolState.requestInvalidate) {
        PostMessage(WM_PAINT);  // or Invalidate(FALSE)
    }
}
```

---

## 5. Tool Adaptation: BoundsHandler → BoundsTool

### Current Structure
```
BoundsInputHandler (IInputHandler)
  └─ BoundsHandler (domain logic)
      ├─ SetEditMode(mode)
      ├─ BeginDrag() / UpdateDrag() / EndDrag()
      └─ GetDraft() / GetPreviewShape()
```

### Refactored Structure
```
BoundsTool (IInteractionTool)
  └─ BoundsHandler (unchanged domain logic)
      ├─ SetEditMode(mode)
      ├─ BeginDrag() / UpdateDrag() / EndDrag()
      └─ GetDraft() / GetPreviewShape()
```

### Implementation Example

```cpp
class BoundsTool : public IInteractionTool {
public:
    BoundsTool();
    
    // IInteractionTool
    ToolCapabilities GetCapabilities() const override {
        return {
            .supportsHover = true,
            .allowForeignDrags = true,
            .requiresExplicitActivation = false,
            .isExclusive = false,
            .hitTestPriority = 100
        };
    }
    
    HitResult HitTest(CPoint screenPt, int tolerance) override {
        return m_handler.HitTest(screenPt, tolerance);
    }
    
    bool OnMouseDown(const ToolContext& ctx) override {
        // Delegate to existing BoundsInputHandler logic
        return m_inputHandler.OnMouseDown(ctx.mouseFlags, ctx.screenPoint);
    }
    
    // ... other overrides ...
    
    ViewState GetViewState(bool isActive, bool isCapturing) const override {
        ViewState state;
        
        // Add preview shape if drafting
        if (m_handler.IsDrafting()) {
            state.shapes.push_back({
                .shape = m_handler.GetDraftPreview(),
                .style = ShapeDrawStyle{.state = ShapeDrawStyle::State::Draft}
            });
        }
        
        // Add preview shape if dragging
        if (m_handler.IsDragging() && isCapturing) {
            state.shapes.push_back({
                .shape = m_handler.GetPreviewShape(),
                .style = ShapeDrawStyle{.state = ShapeDrawStyle::State::Selected, .showHandles = true}
            });
        }
        
        state.statusText = m_lastStatusText;
        state.cursor = m_lastCursor;
        
        return state;
    }
    
private:
    BoundsInputHandler m_inputHandler;
    BoundsHandler m_handler;
    CString m_lastStatusText;
    HCURSOR m_lastCursor;
};
```

---

## 6. Key Differences from Current Architecture

| Feature | InputRouter | InteractionManager |
|---------|-------------|-------------------|
| **Event routing** | `tool.OnEvent()` | `BestHit → tool.OnEvent()` |
| **Capture handling** | Via MFC SetCapture | Via m_captureTool |
| **Cross-tool drag** | Not possible | Built-in via allowForeignDrags |
| **Hit-testing** | Per-tool | Global arbitration |
| **Hover state** | Per-tool | Global hover with foreign tool highlight |
| **View state** | Scattered (OnDraw + handlers) | Unified GetViewState() |
| **Tool switching cost** | Low | Very low (just set m_activeTool) |
| **Scalability** | ~2-3 tools max | N tools + future-proof |

---

## 7. Implementation Phases

### Phase 1: Interface Definition (2 hours)
- Define IInteractionTool
- Define ToolCapabilities, ToolContext, HitResult
- Define InteractionManager skeleton
- Create InputHandlerAdapter

### Phase 2: InteractionManager Core (3 hours)
- Implement event routing (OnMouseDown, OnMouseMove, OnMouseUp)
- Implement hit-test arbitration
- Implement capture semantics
- Test with dummy tools

### Phase 3: Adapt Existing Tools (4 hours)
- Wrap BoundsInputHandler in InputHandlerAdapter (quick)
- Wrap FringeInputHandler in InputHandlerAdapter (quick)
- Test that current behavior unchanged
- Verify cross-tool drag scenario

### Phase 4: Native Tool Implementation (future)
- Implement BoundsTool natively (replaces adapter)
- Implement FringeTool natively
- Add Fiducials tool as first exclusive tool
- Demonstrate capability negotiation

---

## Next Action

**If you want to proceed**: Use this spec to implement InteractionManager incrementally.

**If you want to review first**: Feedback on the design before code?


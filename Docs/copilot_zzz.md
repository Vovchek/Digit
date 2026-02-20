Yes — you’ve just hit the **next inevitable problem** after splitting logic into multiple domain handlers:

> *“Now I have BoundsHandler, Fringe(Input)Handler… soon FiducialsHandler… who’s in charge when input comes?”*

If you let the **View talk to handlers directly**, you will:

* duplicate pan/zoom everywhere ❌
* lose single-tool semantics ❌
* get cross-tool hit-testing chaos ❌
* be unable to implement “move bound while Fringe tool active” ❌

So now you need the thing that was *implicitly assumed* before but must become explicit:

---

# 🔴 Introduce: InteractionManager (Tool Controller)

This is the **owner of tools**, not the tools themselves.

Handlers (`BoundsHandler`, `FringeHandler`, future `FiducialsHandler`)
become **tools** in an interaction system.

They no longer:

* listen to MFC messages
* decide if they are active
* compete for hit-testing

They only:

> implement capabilities

---

# Tool responsibilities (after refactor)

Each handler becomes:

```cpp
class IInteractionTool {
public:
    virtual HitResult HitTest(WorldPoint pt) = 0;

    virtual void OnLButtonDown(const ToolContext&, const HitResult&) = 0;
    virtual void OnMouseMove(const ToolContext&) = 0;
    virtual void OnLButtonUp(const ToolContext&) = 0;

    virtual InteractionViewState GetViewState() const = 0;

    virtual ToolCapabilities GetCapabilities() const = 0;
};
```

---

# ToolCapabilities is the key to inter-tool interaction

```cpp
struct ToolCapabilities {
    bool supportsHover = true;
    bool supportsDrag = true;
    bool exclusiveSelection = false;
    bool allowForeignDrags = true;   // <--- THIS is your case
};
```

Examples:

| Tool          | exclusiveSelection | allowForeignDrags |
| ------------- | ------------------ | ----------------- |
| FringeTool    | true               | true              |
| BoundsTool    | false              | true              |
| FiducialsTool | true               | false             |

Meaning:

> Fringe is active
> but user clicks bound
> BoundsTool may take drag if allowed

---

# 🔵 InteractionManager becomes traffic police

### It owns:

```cpp
IInteractionTool* m_activeTool;
std::vector<IInteractionTool*> m_tools;
IInteractionTool* m_captureTool = nullptr;
```

---

# Input flow (this is what you’re missing now)

## On mouse down:

```
1) Ask ALL tools for HitTest()
2) Pick best hit (global priority)
3) If hit belongs to:
     a) active tool → active handles it
     b) other tool:
         if active.allowForeignDrags:
             captureTool = foreignTool
         else:
             ignore
4) Forward OnLButtonDown to captureTool
```

So:

> Fringe tool active
> click on bound
> → BoundsTool becomes captureTool
> → drag begins
> → active tool remains Fringe
> → mode does not switch

---

## On mouse move:

```
if captureTool != nullptr:
    captureTool->OnMouseMove()
else:
    activeTool->OnMouseMove()
```

---

## On mouse up:

```
captureTool->OnLButtonUp()
captureTool = nullptr
```

Temporary capture — not mode switch.

Exactly what you want.

---

# 🟢 Pan / Zoom — DO NOT duplicate!

Pan/Zoom is **global navigation**, not a tool.

Handled before tools:

```cpp
if (mods.middleButton || mods.space) {
    m_viewportController.Pan(...)
    return;
}

if (wheel) {
    m_viewportController.Zoom(...)
    return;
}
```

Tools **never see** these events.

So:

* no duplication
* no conflicts
* tools always receive world-stable input

---

# 🟡 Hover logic (cross-tool)

Same arbitration applies:

```
for each tool:
    hit = tool->HitTest()

store best hit
```

If best hit belongs to foreign tool:

* highlight it
* but don’t activate it

This enables:

> hover bound while Fringe tool active

without switching tool.

---

# Rendering now becomes composited

InteractionManager gathers:

```
DocumentViewState
+ ActiveTool.ViewState()
+ CaptureTool.ViewState()
+ HoverTool.ViewState()
```

and View renders merged interaction overlay.

---

# Mental model to lock in

```
CImageView
   ↓
InteractionManager
   ├── activeTool
   ├── captureTool (temporary)
   └── tool set
          ├── BoundsTool
          ├── FringeTool
          └── FiducialsTool
```

Tools:

* don’t know each other
* don’t know if they are active
* don’t know about pan/zoom
* only respond when manager routes input

---

# This gives you:

✅ One active tool
✅ Cross-tool drag without switching
✅ Optional tool exclusivity
✅ No duplicated pan/zoom
✅ Global hover
✅ Future-proof for fiducials etc.

---

If you want, next step is a **Copilot-ready instruction**:

> “Refactor BoundsHandler and InputHandler into IInteractionTool tools and introduce InteractionManager routing with capture semantics.”

# Bounds Editing Architecture Diagrams

## Current Architecture (Legacy)

```
┌────────────────────────────────────────────────────────────────┐
│                         ImageView                              │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ Message Handlers (OnLButtonDown, OnMouseMove, OnDraw)    │  │
│  └──────────────────────────────────────────────────────────┘  │
│                             ↓                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ BeginTracker / DragTracker / DropTracker                 │  │
│  │  (Direct Tracker manipulation)                           │  │
│  └──────────────────────────────────────────────────────────┘  │
│                             ↓                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │          CMTraker (Utils/Tracker.h)                      │  │
│  │  • Manages 4 corner handles                              │  │
│  │  • Uses legacy m_zoomLevel from BaseImageView            │  │
│  │  • Manual coordinate transformations                     │  │
│  │  • No ViewTransform awareness                            │  │
│  └──────────────────────────────────────────────────────────┘  │
│                             ↓                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │     CBoundCtrls / CImageCtrls                            │  │
│  │  (Bounds data model from CBaseImageDoc)                  │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘

⚠️  Issues:
    • No coordination with InputHandler (separate from main event flow)
    • Legacy zoom logic doesn't account for offset/pan
    • Tracker class tightly coupled to rectangle representation
    • Manual coordinate transforms are error-prone
```

## Target Architecture (New)

```
┌──────────────────────────────────────────────────────────────────────────┐
│                         ImageView                                        │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ Message Handlers (OnLButtonDown, OnMouseMove, OnDraw)              │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                             ↓                                            │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ Route through InputHandler (unified event flow)                    │  │
│  │  • Check current EditMode (Navigate/Draw/DotEdit/BoundsExt/...)    │  │
│  │  • Delegate to appropriate mode handler                            │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                             ↓ (if BoundsExt/BoundsIns)                   │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ InputHandler + BoundsHandler Interaction                           │  │
│  │  ┌──────────────────────────────────────────────────────────┐      │  │
│  │  │ BoundsHandler (DigitMode/BoundsHandler.h)                │      │  │
│  │  │  • HitTest for handles using ViewTransform               │      │  │
│  │  │  • BeginDrag / UpdateDrag / EndDrag state machine        │      │  │
│  │  │  • Preview bound calculation                             │      │  │
│  │  │  • Handle drawing / feedback rendering                   │      │  │
│  │  │  • ALL coordinate transforms via ViewTransform           │      │  │
│  │  └──────────────────────────────────────────────────────────┘      │  │
│  │                                                                    │  │
│  │  Integrated into InputHandler as nested component:                 │  │
│  │    InputHandler {                                                  │  │
│  │      EditMode currentMode;                                         │  │
│  │      BoundsHandler m_boundsHandler;  // ← NEW                      │  │
│  │    }                                                               │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                             ↓                                            │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ ViewTransform (unified coordinate system)                          │  │
│  │  • ScreenToWorld() — input conversion                              │  │
│  │  • WorldToScreen() — render feedback                               │  │
│  │  • Handles zoom, pan, offset all transparently                     │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                             ↓                                            │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ CommandDispatcher (undo/redo)                                      │  │
│  │  • MoveBoundCommand                                                │  │
│  │  • AddBoundCommand                                                 │  │
│  │  • RemoveBoundCommand                                              │  │
│  └────────────────────────────────────────────────────────────────────┘  │
│                             ↓                                            │
│  ┌────────────────────────────────────────────────────────────────────┐  │
│  │ CBoundCtrls / CImageCtrls (data model)                             │  │
│  └────────────────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────────────┘

✅ Benefits:
   • Unified event flow through InputHandler
   • ViewTransform handles all coordinate complexity
   • Testable BoundsHandler with injected dependencies
   • Undo/redo support via CommandDispatcher
   • Mode isolation prevents state leakage
```

## Data Flow: Bounds Drag Operation

```
User clicks on bound handle
    ↓
ImageView::OnLButtonDown(CPoint screenPt)
    ↓
InputHandler::GetMode() == EditMode::BoundsExt?
    ├─→ YES: Continue
    └─→ NO: Route to other handler
    ↓
BoundsHandler::HitTestBoundHandle(screenPt, &boundIdx, &handleIdx)
    ├─ Uses ViewTransform::WorldToScreen() to get handle positions
    ├─ Checks if screenPt is near any handle
    ├─ Returns: boundIdx (which bound), handleIdx (which corner: 0-3)
    ↓
BoundsHandler::BeginDrag(boundIdx, handleIdx, screenPt)
    ├─ Store: m_boundIndex = boundIdx
    ├─ Store: m_handleIndex = handleIdx
    ├─ Store: m_dragStart = screenPt
    ├─ Snapshot: m_boundSnapshot = GetCurrentBound(boundIdx)
    ├─ Set: m_isDragging = true
    ↓

User moves mouse
    ↓
ImageView::OnMouseMove(CPoint screenPt)
    ↓
[If BoundsExt/BoundsIns mode and isDragging]
    ↓
BoundsHandler::UpdateDrag(screenPt)
    ├─ Calculate screen delta: Δscreen = screenPt - m_dragStart
    ├─ Convert to world delta:
    │   worldStart = ViewTransform::ScreenToWorld(m_dragStart)
    │   worldCurr = ViewTransform::ScreenToWorld(screenPt)
    │   Δworld = (worldCurr.x - worldStart.x, worldCurr.y - worldStart.y)
    ├─ Compute new bound:
    │   newBound = ComputeNewBoundFromDrag(m_boundSnapshot, m_handleIndex, Δworld)
    ├─ Update preview: m_previewBound = newBound
    ├─ Store: m_dragCurrent = screenPt
    ↓
ImageView::Invalidate() → OnDraw()
    ↓
OnDraw() calls:
    ├─ BoundsHandler::DrawPreviewBound(pDC)
    │   ├─ Converts previewBound (world) to screen via ViewTransform
    │   ├─ Draws bound outline at preview position
    │   └─ Uses ViewTransform::WorldToScreen() for all corners
    ├─ BoundsHandler::DrawHandles(pDC)
    │   ├─ For each corner of preview bound:
    │   │   ├─ screenPt = ViewTransform::WorldToScreen(worldPt)
    │   │   ├─ Draw small square/circle at screenPt
    │   └─ Highlight active handle
    ↓

User releases mouse
    ↓
ImageView::OnLButtonUp(CPoint screenPt)
    ↓
[If BoundsExt/BoundsIns mode and isDragging]
    ↓
BoundsHandler::UpdateDrag(screenPt)  // Final update
    ↓
BoundsHandler::EndDrag(bCommit=true)
    ├─ If bCommit AND newBound != oldBound:
    │   ├─ Create MoveBoundCommand:
    │   │   MoveBoundCommand(boundIdx, mode, m_boundSnapshot, m_previewBound, pBounds)
    │   ├─ Dispatch to CommandDispatcher:
    │   │   m_pCmdDisp->Dispatch(std::move(cmd))
    │   └─ Command executes:
    │       ├─ CBoundCtrls::SetBound(m_previewBound) [pseudo]
    │       └─ Mark document as modified
    ├─ Else if !bCommit:
    │   └─ Revert silently (no command)
    ├─ Clean up state:
    │   ├─ m_isDragging = false
    │   ├─ m_handleIndex = -1
    │   ├─ m_boundIndex = -1
    ├─ Release capture
    ↓
View redrawn with new bound position
    ↓
Undo buffer updated (can undo via Ctrl+Z)
```

## State Machine: Mode Transitions

```
                    ┌──────────────┐
                    │   Navigate   │ ← Default mode
                    │ (Selection)  │
                    └──────────────┘
                     ↑            ↓
            [User presses]  [User clicks]
            [Ctrl+Shift+B]  [Bound handle]
                     ↓            ↑
         ┌──────────────────────────┐
         │   BoundsExt Mode         │
         │  • isDragging state      │
         │  • handleIndex, boundIdx │
         │  • previewBound          │
         └──────────────────────────┘
                     ↑            ↓
            [Mode finalize]  [User drags]
            [Clean state]    [on update]
                     ↓            ↑
                    Draw
                  (Drawing
                  fringes)

Transitions via InputHandler::SetMode(EditMode newMode):
  • Finalizes previous mode (e.g., ends active segment if in Draw mode)
  • Initializes new mode
  • UpdateCursor
  • Clear temporary state (selections, previews, etc.)

State cleanup on mode switch:
  ├─ BoundsExt → Navigate:
  │   ├─ If isDragging: EndDrag(false)  [revert preview]
  │   ├─ Clear m_boundsEdit state
  │   └─ Restore Navigate cursors
  ├─ BoundsExt → Draw:
  │   ├─ Same cleanup as above
  │   └─ Lock selection, initialize drawing
  └─ (etc.)
```

## Coordinate Transformation Pipeline

```
Input Event (screen coordinates)
    ↓
    ┌─────────────────────────────────────┐
    │ Mouse Event in ImageView::OnXxx()   │
    │ screenPt = CPoint(x, y)             │
    │ [client window coordinates]         │
    └─────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────┐
    │ Pass to InputHandler/BoundsHandler  │
    ├─────────────────────────────────────┤
    │ BoundsHandler::HitTest(screenPt)    │
    │  • Call ViewTransform::WorldToScreen│
    │    for each handle position         │
    │  • Compare screenPt to handle pos   │
    │  • Determine if hit or miss         │
    └─────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────┐
    │ On drag:                            │
    │ BoundsHandler::UpdateDrag(screenPt) │
    ├─────────────────────────────────────┤
    │ 1. screenStart = m_dragStart        │
    │    screenCurr = screenPt            │
    │    [both in client coordinates]     │
    │                                     │
    │ 2. Convert to world coordinates:    │
    │    worldStart = ViewTransform       │
    │      ::ScreenToWorld(screenStart)   │
    │    worldCurr = ViewTransform        │
    │      ::ScreenToWorld(screenCurr)    │
    │    [both in image/world coords]     │
    │                                     │
    │ 3. Calculate world delta:           │
    │    Δx = worldCurr.x - worldStart.x  │
    │    Δy = worldCurr.y - worldStart.y  │
    │    [displacement in world space]    │
    │                                     │
    │ 4. Apply delta to original bound:   │
    │    oldBound = m_boundSnapshot       │
    │    newBound =                       │
    │      ApplyDeltaToBound(oldBound,    │
    │                        handleIdx,   │
    │                        Δx, Δy)      │
    │                                     │
    │ 5. Store preview:                   │
    │    m_previewBound = newBound        │
    │    [in world coordinates]           │
    └─────────────────────────────────────┘
    ↓
    ┌─────────────────────────────────────┐
    │ On rendering:                       │
    │ BoundsHandler::DrawPreviewBound()   │
    ├─────────────────────────────────────┤
    │ 1. Get preview in world coords:     │
    │    worldBound = m_previewBound      │
    │                                     │
    │ 2. For each corner:                 │
    │    screenCorner = ViewTransform     │
    │      ::WorldToScreen(worldCorner)   │
    │    [convert back to client coords]  │
    │                                     │
    │ 3. Draw using screen coordinates:   │
    │    pDC->MoveTo(screenCorner)        │
    │    pDC->LineTo(nextScreenCorner)    │
    │    [graphics rendered directly]     │
    └─────────────────────────────────────┘
    ↓
User sees preview in correct screen position
regardless of zoom level, pan offset, or scaling
```

## Key Classes & Relationships

```
┌─────────────────────────────────────────────────────────────┐
│                 CImageView (UI Layer)                       │
│  • OnLButtonDown / OnMouseMove / OnLButtonUp                │
│  • OnDraw / Invalidate                                      │
│  • References:                                              │
│    - m_inputHandler (InputHandler)                          │
│    - m_viewTransform (ViewTransform)                        │
│    - GetDocument()→boundCtrls (CBoundCtrls)                 │
└─────────────────────────────────────────────────────────────┘
              ↓                          ↑
         delegates to                routes events through
              ↓                          ↑
┌─────────────────────────────────────────────────────────────┐
│            InputHandler (Event Router)                      │
│  • SetMode(EditMode)                                        │
│  • GetMode() → EditMode                                     │
│  • OnMouseDown / OnMouseMove / OnMouseUp                    │
│  • m_boundsHandler (BoundsHandler)  ← NEW                   │
│  • m_drag (DragState) — shared across modes                 │
└─────────────────────────────────────────────────────────────┘
              ↓                          ↑
       delegates to                returns results to
              ↓                          ↑
┌─────────────────────────────────────────────────────────────┐
│          BoundsHandler (Bounds Logic)                       │
│  • HitTestBoundHandle()                                     │
│  • BeginDrag / UpdateDrag / EndDrag                         │
│  • ComputeNewBoundFromDrag()                                │
│  • DrawHandles / DrawPreviewBound                           │
│  • References:                                              │
│    - m_pView (ViewTransform*)  ← coordinate transforms      │
│    - m_pBounds (CBoundCtrls*)  ← data model                 │
│    - m_pImage (CImageCtrls*)   ← image dimensions           │
└─────────────────────────────────────────────────────────────┘
              ↓
       uses for transforms
              ↓
┌─────────────────────────────────────────────────────────────┐
│         ViewTransform (Coordinate System)                   │
│  • ScreenToWorld(CPoint) → CPoint2d                         │
│  • WorldToScreen(CPoint2d) → CPoint                         │
│  • Tracks: scale, offset (pan), zoom state                  │
└─────────────────────────────────────────────────────────────┘
              ↓
       accesses to compute bounds
              ↓
┌─────────────────────────────────────────────────────────────┐
│       CBoundCtrls (Data Model)                              │
│  • ExtBoundType / InsBoundType                              │
│  • GetBoundRect() / SetBound()                              │
│  • GetExtRealBound() / GetInsRealBound()                    │
└─────────────────────────────────────────────────────────────┘

Additionally:
┌─────────────────────────────────────────────────────────────┐
│     CommandDispatcher (Undo/Redo)                           │
│  • Dispatch(ICommand*) — executes and records               │
│  • Undo / Redo                                              │
│  ← Used by BoundsHandler::EndDrag() to record MoveBoundCmd  │
└─────────────────────────────────────────────────────────────┘
```

## Testing Layers

```
┌────────────────────────────────────────────────┐
│    Visual / Integration Tests                  │
│  • Full drag workflows                         │
│  • Multiple zoom/pan scenarios                 │
│  • Undo/redo verification                      │
│  • Coordinate accuracy over full range         │
└────────────────────────────────────────────────┘
        ↓           ↓           ↓
┌────────────────────────────────────────────────┐
│    Unit Tests (Isolated Layers)                │
│  ├─ ViewTransform tests                        │
│  │   • ScreenToWorld ↔ WorldToScreen           │
│  │   • Zoom accuracy                           │
│  │   • Pan accuracy                            │
│  ├─ BoundsHandler tests                        │
│  │   • HitTesting with ViewTransform           │
│  │   • Drag geometry calculations              │
│  │   • State machine (isDragging, etc.)        │
│  ├─ InputHandler tests                         │
│  │   • Mode switching                          │
│  │   • State cleanup on transitions            │
│  └─ Command tests                              │
│      • MoveBoundCommand execute/undo           │
└────────────────────────────────────────────────┘
```


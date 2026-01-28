# implement_ux_phase2-3_backlog_clear_pan.md

Purpose
- Clear backlog items blocking UX Phases 2 (Draw) → 3 (Selection & Navigate).
- Provide exact tickets with code snippets and call sites to change so the ImageView UI binds to new input/command/selection classes.

Notes
- Current state: core Draw commands + InputHandler logic implemented and unit-tested per `Docs/Phase2_DrawMode_Complete.md`.
- Gap: `ImageTempl/ImageView.cpp` still routes interactive input to legacy `CImageDoc` methods (`OnLButDown`, `LockDot`, `LockZapSection`, `OnKeyDown`, `LastOperationUndo`, etc.). Need adapter wiring to `InputHandler`, `HitTester`, `SelectionManager` and `CommandDispatcher`.

How to apply these tickets
- Apply in order. Tickets 1-4 are high priority and low risk (thin adapters). Tickets 5-9 implement missing core classes and tests.

Ticket 001 — Add UI-side members to `CImageView` (glue objects)
- Priority: High
- Goal: Add `InputHandler`, `HitTester`, `SelectionManager`, `CommandDispatcher` members to `CImageView`.
- Files to edit:
  - `ImageTempl/ImageView.h` (create if necessary)
- Changes (exact snippet to add inside `CImageView` class definition, private section):

```cpp
#include "DigitMode/InputHandler.h"
#include "DigitMode/HitTester.h"
#include "DigitMode/SelectionManager.h"
#include "DigitMode/CommandDispatcher.h"

private:
    InputHandler m_inputHandler;
    HitTester m_hitTester;
    SelectionManager m_selectionMgr;
    CommandDispatcher m_cmdDispatcher;
```

- Rationale: provide persistent instances used by message handlers.
- Tests: build only.
- Estimate: 15–30min

---

Ticket 002 — Route `OnLButtonDown` to `InputHandler` (start/continue/connect)
- Priority: High
- Goal: Replace legacy quick-paths when `ActiveEditMode` maps to drawing with calls to `m_inputHandler` and `m_cmdDispatcher`.
- File: `ImageTempl/ImageView.cpp`
- Call site: inside `CImageView::OnLButtonDown(UINT nFlags, CPoint point)` after `ClientToDoc(l_point); CursorPos = l_point;` and before legacy `if (pDoc->IsFotoSections())` block.
- Exact snippet to insert (safe adapter; keep legacy as fallback):

```cpp
// Adapter: route draw-mode interactions to new InputHandler
ModifierState mods = ModifierState::FromKeyboard();
int hitSeg = -1, hitDot = -1;
SelectionLevel hitLevel = m_hitTester.HitTest(l_point, hitSeg, hitDot);

CControls* pCtrls = GetControls();
CImageDoc* pDoc = (CImageDoc*)GetDocument();

// Treat toolbar/menu edit modes as Draw mode if appropriate
bool uiRequestsDraw = (pCtrls->ActiveEditMode == E_ADD_DOT || pCtrls->ActiveEditMode == E_ADD_SECTION);

if (uiRequestsDraw || m_inputHandler.IsInDrawMode()) {
    // Start new segment when clicking empty space
    if (hitLevel == SelectionLevel::None) {
        // Create command via InputHandler (InputHandler must call CommandDispatcher internally or expose StartNewSegment that accepts a CommandDispatcher reference)
        m_inputHandler.StartNewSegment(l_point, &pDoc->Digit, &m_cmdDispatcher);
        Invalidate(FALSE);
        return; // consumed
    }
    // Continue drawing when clicking an existing dot (attach to its segment)
    if (hitLevel == SelectionLevel::Dot) {
        m_inputHandler.ContinueSegment(hitSeg, hitDot, &pDoc->Digit);
        Invalidate(FALSE);
        return;
    }
}
```

- Required API adjustments: `InputHandler::StartNewSegment(CPoint, CDigitInfo*, CommandDispatcher*)`, `InputHandler::ContinueSegment(int, int, CDigitInfo*)`. If not implemented, add thin forwarding overloads.
- Tests: unit test that simulates `OnLButtonDown` calling the adapter and verifies `m_cmdDispatcher` received an `AddDotCommand` (mock or spy) and `CDigitInfo::Fringes` changed.
- Estimate: 2–3 hours (including small InputHandler signature changes)

---

Ticket 003 — Route `OnMouseMove` for drag preview and hit testing
- Priority: High
- Goal: Use `HitTester` for cursor and `InputHandler` for live preview during draw/drag.
- File: `ImageTempl/ImageView.cpp`
- Call site: `CImageView::OnMouseMove(UINT nFlags, CPoint point)` replace branch that checks `pDoc->IsLockedDot()` / `IsLockedZapSection()` with adapter that prefers `m_inputHandler` preview.
- Replace snippet (inside `if (pDoc == pActDoc) { ... }`):

```cpp
// Compute doc-space point and modifiers already available earlier
ModifierState mods = ModifierState::FromKeyboard();
int hitSeg = -1, hitDot = -1;
SelectionLevel hoverLevel = m_hitTester.HitTest(l_point, hitSeg, hitDot);

// Update cursor using hit test result
m_selectionMgr.SetHover(hoverLevel, hitSeg, hitDot);

// If draw-mode active and we have a preview active, let InputHandler render preview via ImageView hooks
if (m_inputHandler.IsInDrawMode()) {
    m_inputHandler.OnMouseMove(l_point, mods, &GetDocument()->Digit, &m_cmdDispatcher);
    Invalidate(FALSE);
    return; // consumed
}

// Fallback to existing drag behaviours if legacy locking active
if (pDoc->IsLockedDot()) {
    DragDot(l_point);
    return;
}
if (pDoc->IsLockedZapSection()) {
    DragZapSection(l_point);
    return;
}
```

- API notes: `InputHandler::OnMouseMove(CPoint, ModifierState, CDigitInfo*, CommandDispatcher*)` should be added to support preview.
- Tests: manual visual test + unit test verifying `OnMouseMove` calls `InputHandler::OnMouseMove` in draw mode.
- Estimate: 2–4 hours

---

Ticket 004 — Route `OnLButtonUp` to commit draw actions and end drag
- Priority: High
- Goal: On release commit the current draw command (or forward to legacy `DropDot` when using dot-lock fallback).
- File: `ImageTempl/ImageView.cpp`
- Call site: `CImageView::OnLButtonUp` inside branch where `pDoc->IsLockedDot()` previously released capture; add adapter call before `pDoc->LockDot(l_point, FALSE);`.
- Exact snippet to insert:

```cpp
// If InputHandler is managing a drag (draw/insert), commit via InputHandler
if (m_inputHandler.IsManagingDrag()) {
    m_inputHandler.OnLButtonUp(l_point, &GetDocument()->Digit, &m_cmdDispatcher);
    ReleaseCapture();
    Invalidate(FALSE);
    return;
}
```

- Tests: simulate press-move-release and assert command executed and undo available.
- Estimate: 1–2 hours

---

Ticket 005 — Route `OnKeyDown` for Draw/Selection keyboard bindings
- Priority: High
- Goal: Map Backspace, arrow keys, Esc, +/-, Enter to `InputHandler`/`CommandDispatcher` or selection management.
- File: `ImageTempl/ImageView.cpp` (function `OnKeyDown`)
- Call site: At end of `OnKeyDown`, currently calls `pDoc->OnKeyDown(nChar,...)`. Replace or short-circuit when `m_inputHandler.IsActive()`.
- Snippets to add:

```cpp
// If draw mode active handle Backspace here via command dispatcher
if (m_inputHandler.IsInDrawMode()) {
    if (nChar == VK_BACK) {
        auto cmd = std::make_unique<RemoveLastDotCommand>(&GetDocument()->Digit);
        m_cmdDispatcher.Execute(std::move(cmd));
        Invalidate(FALSE);
        return; // consumed
    }
    if (nChar == VK_ESCAPE) {
        m_inputHandler.CancelDraw(&GetDocument()->Digit);
        Invalidate(FALSE);
        return;
    }
    // Arrow keys navigate within current fringe
    if (nChar == VK_LEFT || nChar == VK_RIGHT || nChar == VK_UP || nChar == VK_DOWN) {
        m_inputHandler.OnKeyDown(nChar, &GetDocument()->Digit);
        Invalidate(FALSE);
        return;
    }
}
// else fallback to legacy doc behavior (preserve compatibility)
```

- API changes: `InputHandler::CancelDraw`, `InputHandler::OnKeyDown` should be available.
- Tests: unit test for Backspace trigger and undo stack.
- Estimate: 2–3 hours

---

Ticket 006 — Implement `HitTester::HitTest` using `CDigitInfo`/`Fringes` fast path
- Priority: Medium
- Goal: Provide reliable hit testing for dots, edges, segments required by `ImageView` and `SelectionManager`.
- Files to create/modify: `DigitMode/HitTester.h` + `DigitMode/HitTester.cpp`.
- Minimal implementation (exact logic):

```cpp
// HitTester::HitTest(CPoint P, int& outSegment, int& outDot)
// 1) Query Fringes (if fringe model active): for each fringe iF from top to bottom
//    - for each point j: if within DotTolerance -> outSegment = iF; outDot = j; return SelectionLevel::Dot
//    - for each segment (p[j], p[j+1]): if distanceToSegment < EdgeTolerance -> outSegment = iF; outDot = j; return SelectionLevel::Edge
// 2) Fallback: use CDigitInfo::IsDotUnderCursor and friends for legacy Dots array

// Use Cartesian distance squared for speed. Use Document mapping to doc-space before calling.
```

- Provide exact code skeleton in `HitTester.cpp` and unit tests in `Tests/HitTesterTest.cpp`.
- Estimate: 4–8 hours

---

Ticket 007 — Implement `SelectionManager` + visual highlight integration
- Priority: Medium
- Goal: Manage hover/selection state and provide helpers for rendering highlighted dot/segment.
- Files: `DigitMode/SelectionManager.h/.cpp`, modify `ImageTempl/ImageView.cpp::DrawDigitInfo` to call `m_selectionMgr` draw helpers.
- Snippet to add in `DrawDigitInfo` before/after `pDoc->Digit.Draw(...)`:

```cpp
// Draw selection highlights for hover/selected object
m_selectionMgr.DrawHighlights(pDC, &GetDocument()->Digit, DotSide);
```

- Tests: `SelectionManagerTest` to validate selection adds/removes and `ImageView` renders highlights without crash.
- Estimate: 6–10 hours

---

Ticket 008 — Wire `OnUndo` to `CommandDispatcher`
- Priority: High
- Goal: Replace `pDoc->LastOperationUndo()` with `m_cmdDispatcher.Undo()` and update `OnUpdateUndo` to use `m_cmdDispatcher.CanUndo()`.
- File: `ImageTempl/ImageView.cpp`
- Replace `OnUndo()` body with:

```cpp
void CImageView::OnUndo()
{
    if (m_cmdDispatcher.CanUndo()) {
        m_cmdDispatcher.Undo();
        Invalidate(FALSE);
    }
}
```

- Replace `OnUpdateUndo` with reading `m_cmdDispatcher.CanUndo()`.
- Tests: Undo/Redo integration tests verifying command stack behavior.
- Estimate: 1–2 hours

---

Ticket 009 — Add integration tests for UI routing (headless) and image view adapters
- Priority: High
- Goal: Ensure adapters correctly call `InputHandler` and `CommandDispatcher`.
- Files: `Tests/UIIntegration/ImageViewAdapterTest.cpp`.
- Example test (pseudo):

```cpp
TEST(ImageViewAdapter, ClickStartsNewSegmentAndAddsDot) {
    CImageView view;
    view.Create(...);
    // initialize document with empty Digit
    CPoint pt(100,100);
    view.OnLButtonDown(0, pt);
    // assert digit.Fringe size increased by 1 and undo available
}
```

- Estimate: 6–12 hours

---

Ticket 010 — Safety & fallback feature-flag
- Priority: Medium
- Goal: Add compile-time or runtime flag to enable new UI adapters. Keep legacy behavior when disabled.
- Implementation: Add `#define ENABLE_FRINGE_UI_ADAPTER 1` or use `GetControls()->UseFringeUI` check.
- Example guard in `OnLButtonDown`:

```cpp
#ifdef ENABLE_FRINGE_UI_ADAPTER
  // use new adapter
#else
  pDoc->OnLButDown(l_point);
#endif
```

- Estimate: 1 hour

---

Deliverables checklist
- [ ] `ImageView.h` updated with new members (Ticket 001)
- [ ] `ImageView.cpp` adapter edits for `OnLButtonDown`, `OnMouseMove`, `OnLButtonUp`, `OnKeyDown` (Tickets 002–005)
- [ ] `HitTester` implemented + unit tests (Ticket 006)
- [ ] `SelectionManager` implemented + draw integration (Ticket 007)
- [ ] `CommandDispatcher` wired to `OnUndo` (Ticket 008)
- [ ] UI integration tests (Ticket 009)
- [ ] Feature-flag / fallback (Ticket 010)

Estimated total effort: 2–3 workdays (conservative) or 1–2 days (if parallelized and some classes already exist).

Notes about signatures and small refactors required
- `InputHandler` must expose these methods (add overloads if necessary):
  - `StartNewSegment(CPoint P, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp)`
  - `ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit)`
  - `OnMouseMove(CPoint, ModifierState, CDigitInfo*, CommandDispatcher*)`
  - `OnLButtonUp(CPoint, CDigitInfo*, CommandDispatcher*)`
  - `IsInDrawMode()` and `IsManagingDrag()` and `CancelDraw()`
- `HitTester::HitTest(CPoint, int& outSegment, int& outDot)` must be available and fast.
- `CommandDispatcher` must provide `Execute(unique_ptr<Command>)`, `Undo()`, `Redo()`, `CanUndo()`, `CanRedo()`.

Appendix: exact replacement call sites summary
- Replace `pDoc->OnLButDown(l_point);` in `CImageView::OnLButtonDown` with adapter block (Ticket 002).
- Replace `pDoc->IsLockedDot()` checks in `CImageView::OnMouseMove`/`OnLButtonUp` by calling `m_inputHandler` preview/commit methods first (Tickets 003 & 004).
- Replace `pDoc->OnKeyDown(nChar, ...)` in `CImageView::OnKeyDown` with early-return handling for draw-mode keys (Ticket 005).
- Replace `pDoc->LastOperationUndo()` in `CImageView::OnUndo` with `m_cmdDispatcher.Undo()` (Ticket 008).

If you approve, I will create PR-ready patches per ticket in order, starting with minimal safe adapters (Tickets 001–004) and add tests. If you want me to apply the first patch now (add members in `ImageView.h` and the `OnLButtonDown` adapter), confirm and I will proceed.

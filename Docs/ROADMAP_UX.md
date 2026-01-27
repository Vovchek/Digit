# Phase 3: Implementation Roadmap — UX v1.0 to Code

**Status**: Detailed step-by-step development plan  
**Date**: 2026-01-26  
**Purpose**: Guide developers from architecture (IMPLEMENT_UX.md) ? working UI with full UX v1.0 compliance  
**Audience**: Developers, QA, project leads

---

## Overview

This roadmap converts the **architecture design** into a **phased, testable implementation** with clear deliverables, dependencies, and success criteria.

**Key Principle**: Start with **simplest workflows first** (Draw mode), expand to **selection/editing** (Navigate, Dot Edit), then **advanced commands**.

**Total Effort**: ~8-10 weeks for full UX v1.0 implementation (can be done incrementally).

---

## Table of Contents

1. [Phase Overview](#phase-overview)
2. [Phase 1: Foundation (Weeks 1–2)](#phase-1-foundation-weeks-12)
3. [Phase 2: Draw Mode (Weeks 3–4)](#phase-2-draw-mode-weeks-34)
4. [Phase 3: Selection & Navigate (Weeks 5–6)](#phase-3-selection--navigate-weeks-56)
5. [Phase 4: Dot Edit Mode (Week 7)](#phase-4-dot-edit-mode-week-7)
6. [Phase 5: Commands & Undo/Redo (Weeks 8–9)](#phase-5-commands--undoredo-weeks-89)
7. [Phase 6: Polish & Testing (Week 10)](#phase-6-polish--testing-week-10)
8. [File Organization](#file-organization)
9. [Testing Strategy](#testing-strategy)
10. [Success Criteria & Sign-Off](#success-criteria--sign-off)

---

## Phase Overview

| Phase | Focus | Weeks | Deliverable | Risk |
|-------|-------|-------|-------------|------|
| **1** | Core classes + basic infrastructure | 1–2 | InputHandler, ModifierState, basic drawing | Low |
| **2** | Draw mode (create/continue curves) | 3–4 | Full Draw workflow tested | Medium |
| **3** | Selection + Navigate mode | 5–6 | Selection state machine, hit testing, Navigation | Medium |
| **4** | Dot Edit mode (move/insert/delete dots) | 7 | Geometry editing tested | Medium |
| **5** | Command pattern + Undo/Redo | 8–9 | All commands implemented + atomic transactions | High |
| **6** | Integration + edge cases + polish | 10 | Full regression test + UX v1.0 acceptance | Medium |

---

## Phase 1: Foundation (Weeks 1–2)

**Goal**: Establish core classes and wire basic infrastructure into existing `ImageView`.

### Files to Create

```
DigitMode/
├── InputHandler.h              (NEW)
├── InputHandler.cpp            (NEW)
├── SelectionManager.h          (NEW)
├── SelectionManager.cpp        (NEW)
├── HitTester.h                 (NEW)
├── HitTester.cpp               (NEW)
├── CommandDispatcher.h         (NEW)
├── CommandDispatcher.cpp       (NEW)
├── Commands/
│   ├── Command.h               (NEW) - Base class
│   └── (specific commands added in Phase 5)
├── CursorManager.h             (NEW)
├── CursorManager.cpp           (NEW)
├── TooltipGenerator.h          (NEW)
└── TooltipGenerator.cpp       (NEW)

Tests/DigitMode/
├── InputHandlerTest.cpp        (NEW)
├── SelectionManagerTest.cpp    (NEW)
├── HitTesterTest.cpp           (NEW)
└── CommandDispatcherTest.cpp   (NEW)
```

### Tasks

#### 1.1 Create Core Classes (No Implementation Yet)

**InputHandler.h**
```cpp
#pragma once
#include "DigitMode/DigitInfo.h"

enum class EditMode { Navigate, Draw, DotEdit };

class InputHandler {
private:
    EditMode currentMode = EditMode::Navigate;
    
    // Draw mode state
    int iActiveFringe = -1;  // Current fringe being drawn
    int iActiveCurve = -1;   // Current curve being drawn
    
public:
    void SetMode(EditMode newMode);
    EditMode GetMode() const { return currentMode; }
    bool IsInDrawMode() const { return currentMode == EditMode::Draw; }
    
    void StartNewCurve(CPoint P);
    void ContinueCurve(int iFringe, int iCurve, int iDot);
    void ConnectCurves(int iFringe, int iCurve, int iDot);
    void EndCurrentCurve();
};
```

**SelectionManager.h**
```cpp
#pragma once

enum class SelectionLevel { None, Dot, Edge, Curve, Fringe };

class SelectionManager {
public:
    struct SelectedObject {
        SelectionLevel level = SelectionLevel::None;
        int iFringe = -1;
        int iCurve = -1;
        int iDot = -1;
        int iEdge = -1;
        
        bool IsValid() const { return level != SelectionLevel::None; }
    };
    
private:
    CArray<SelectedObject> selection;
    
public:
    void SelectDot(int iFringe, int iCurve, int iDot);
    void SelectEdge(int iFringe, int iCurve, int iEdge);
    void SelectCurve(int iFringe, int iCurve);
    void SelectFringe(int iFringe);
    
    bool AddToSelection(const SelectedObject& obj);
    void PromoteToFringe();
    
    SelectionLevel GetLevel() const;
    int GetCount() const { return selection.GetSize(); }
    const SelectedObject& GetAt(int i) const { return selection[i]; }
    void Clear() { selection.RemoveAll(); }
    bool IsEmpty() const { return selection.GetSize() == 0; }
};
```

**HitTester.h**
```cpp
#pragma once
#include "SelectionManager.h"

class HitTester {
private:
    static const int HIT_TOLERANCE = 5;
    
public:
    SelectionLevel HitTest(CPoint P, int& outFringe, int& outCurve, int& outDot);
    
private:
    double DotDistance(CPoint P, CDPoint dot) const;
    double DistanceToSegment(CPoint P, CDPoint A, CDPoint B) const;
};
```

**Command.h**
```cpp
#pragma once

class Command {
public:
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() { Execute(); }
    virtual CString GetName() const { return "Command"; }
    virtual ~Command() {}
};
```

**CommandDispatcher.h**
```cpp
#pragma once
#include "Commands/Command.h"

class CommandDispatcher {
private:
    CArray<Command*> undoStack;
    CArray<Command*> redoStack;
    
public:
    ~CommandDispatcher();
    
    void Execute(Command* cmd);
    void Undo();
    void Redo();
    
    bool CanUndo() const { return undoStack.GetSize() > 0; }
    bool CanRedo() const { return redoStack.GetSize() > 0; }
    
    CString GetUndoLabel() const;
    CString GetRedoLabel() const;
};
```

**ModifierState (in InputHandler.h or separate)**
```cpp
struct ModifierState {
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    
    bool None() const { return !ctrl && !shift && !alt; }
    
    static ModifierState FromKeyboard() {
        return {
            (GetKeyState(VK_CONTROL) & 0x8000) != 0,
            (GetKeyState(VK_SHIFT) & 0x8000) != 0,
            (GetKeyState(VK_MENU) & 0x8000) != 0
        };
    }
};
```

**CursorManager.h**
```cpp
#pragma once
#include "InputHandler.h"
#include "SelectionManager.h"

class CursorManager {
private:
    HCURSOR hCursorArrow, hCursorCrosshair, hCursorVertex;
    
public:
    CursorManager();
    void UpdateCursor(EditMode mode, ModifierState mods, SelectionLevel under);
};
```

**TooltipGenerator.h**
```cpp
#pragma once
#include "SelectionManager.h"

class TooltipGenerator {
public:
    CString GetTooltip(const SelectionManager::SelectedObject& obj, const CDigitInfo& digit);
};
```

#### 1.2 Implement Core Classes (Minimal Working Version)

**Priority**: InputHandler, SelectionManager, HitTester only.  
**Others**: CursorManager, TooltipGenerator can be stubbed (return empty/default values).

**InputHandler.cpp** - Minimal stubs:
```cpp
void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        EndCurrentCurve();
    }
    currentMode = newMode;
    // TODO: Update cursor
}

void InputHandler::StartNewCurve(CPoint P) {
    // TODO: Implement
    TRACE("StartNewCurve at (%d, %d)\n", P.x, P.y);
}

void InputHandler::EndCurrentCurve() {
    // TODO: Implement
    TRACE("EndCurrentCurve\n");
}
// ... etc
```

**SelectionManager.cpp** - Full implementation (straightforward):
```cpp
void SelectionManager::SelectDot(int iFringe, int iCurve, int iDot) {
    selection.RemoveAll();
    SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iFringe = iFringe;
    obj.iCurve = iCurve;
    obj.iDot = iDot;
    selection.Add(obj);
}

// ... etc (from IMPLEMENT_UX.md)
```

**HitTester.cpp** - Full implementation:
```cpp
SelectionLevel HitTester::HitTest(CPoint P, int& outFringe, int& outCurve, int& outDot) {
    // Implementation from IMPLEMENT_UX.md
    return SelectionLevel::None;  // Stub for now
}

// ... etc
```

#### 1.3 Wire into ImageView

**ImageView.h** - Add members:
```cpp
class CImageView : public CBaseImageView {
private:
    InputHandler inputHandler;
    SelectionManager selectionMgr;
    HitTester hitTester;
    CommandDispatcher cmdDispatcher;
    CursorManager cursorMgr;
    TooltipGenerator tooltipGen;
    
    EditMode currentMode = EditMode::Navigate;
};
```

**ImageView.cpp** - Add handlers:
```cpp
afx_msg void CImageView::OnLButtonDown(UINT nFlags, CPoint point) {
    ModifierState mods = ModifierState::FromKeyboard();
    
    int iFringe, iCurve, iDot;
    SelectionLevel hitLevel = hitTester.HitTest(point, iFringe, iCurve, iDot);
    
    TRACE("LButtonDown at (%d, %d), mode=%d, hit=%d\n", point.x, point.y, currentMode, hitLevel);
    
    // TODO: Route to handlers
    Invalidate(FALSE);
}

afx_msg void CImageView::OnMouseMove(UINT nFlags, CPoint point) {
    ModifierState mods = ModifierState::FromKeyboard();
    cursorMgr.UpdateCursor(currentMode, mods, SelectionLevel::None);
    // TODO: Update tooltip
}
```

#### 1.4 Unit Tests (Phase 1)

**Tests/DigitMode/SelectionManagerTest.cpp**:
```cpp
TEST(SelectionManager, SelectDotWorks) {
    SelectionManager sel;
    sel.SelectDot(0, 0, 5);
    
    EXPECT_EQ(SelectionLevel::Dot, sel.GetLevel());
    EXPECT_EQ(1, sel.GetCount());
    EXPECT_EQ(0, sel.GetAt(0).iFringe);
    EXPECT_EQ(5, sel.GetAt(0).iDot);
}

TEST(SelectionManager, PromoteToCurve) {
    SelectionManager sel;
    sel.SelectDot(0, 0, 5);
    sel.SelectDot(0, 1, 2);  // Add another dot in different curve
    
    // Promote should group by curve
    sel.PromoteToFringe();
    EXPECT_EQ(SelectionLevel::Fringe, sel.GetLevel());
    EXPECT_EQ(1, sel.GetCount());  // Only one fringe
}

// ... etc
```

**Tests/DigitMode/HitTesterTest.cpp**:
```cpp
// Mock fringe data, test DotDistance, DistanceToSegment, HitTest
TEST(HitTester, DotDistanceCorrect) {
    HitTester tester;
    double dist = tester.DotDistance(CPoint(10, 10), CDPoint(10, 10));
    EXPECT_EQ(0.0, dist);
    
    dist = tester.DotDistance(CPoint(13, 14), CDPoint(10, 10));
    EXPECT_NEAR(5.0, dist, 0.1);  // 3-4-5 triangle
}
```

### Deliverables (Phase 1)

- ? All core classes created with public interfaces
- ? SelectionManager, HitTester fully implemented + unit tested
- ? InputHandler, CommandDispatcher stubbed + wired into ImageView
- ? Basic TRACE logging for debugging
- ? No functional UI yet, but architecture in place

### Success Criteria

- [ ] Project compiles without errors
- [ ] InputHandler, SelectionManager, HitTester unit tests pass
- [ ] CImageView initializes new components without crashing
- [ ] Can turn on/off trace logging

---

## Phase 2: Draw Mode (Weeks 3–4)

**Goal**: Implement full Draw workflow (create ? continue ? connect ? end curves).

### Tasks

#### 2.1 Complete InputHandler Implementation

**InputHandler.cpp** - Full Draw mode logic:
```cpp
void InputHandler::StartNewCurve(CPoint P) {
    // Create new fringe with next number
    double newNumber = digitInfo.GetNextNumber();
    iActiveFringe = digitInfo.CreateFringe(newNumber);
    iActiveCurve = 0;
    
    // Add first dot
    digitInfo.AddDotToFringe(iActiveFringe, iActiveCurve, CDPoint(P.x, P.y));
    TRACE("Started new curve: fringe=%d, dot=0\n", iActiveFringe);
}

void InputHandler::ContinueCurve(int iFringe, int iCurve, int iDot) {
    iActiveFringe = iFringe;
    iActiveCurve = iCurve;
    TRACE("Continuing curve: fringe=%d, curve=%d\n", iFringe, iCurve);
}

void InputHandler::ConnectCurves(int iFringe, int iCurve, int iDot) {
    // Connect iActiveCurve to the specified curve
    // Transfer drawing to the free end of the target curve
    TRACE("Connecting curves\n");
    
    iActiveFringe = iFringe;
    iActiveCurve = iCurve;
    // TODO: Mark curves as connected
}

void InputHandler::EndCurrentCurve() {
    if (iActiveFringe >= 0) {
        TRACE("Ended curve: fringe=%d, curve=%d\n", iActiveFringe, iActiveCurve);
        iActiveFringe = -1;
        iActiveCurve = -1;
    }
}
```

#### 2.2 Add Drawing Commands

**Commands/AddDotCommand.h & .cpp**:
```cpp
class AddDotCommand : public Command {
private:
    int iFringe, iCurve, iDot;
    CDPoint point;
    CDigitInfo* pDigit;
    
public:
    AddDotCommand(CDigitInfo* pD, int iF, int iC, int iD_idx, CDPoint p);
    void Execute() override { pDigit->InsertDot(iFringe, iCurve, iDot, point); }
    void Undo() override { pDigit->RemoveDot(iFringe, iCurve, iDot); }
    CString GetName() const override { return "Add Dot"; }
};
```

**Commands/RemoveLastDotCommand.h & .cpp**: (Similar pattern)

#### 2.3 Integrate Draw Mode into ImageView

**ImageView.cpp** - OnLButtonDown for Draw mode:
```cpp
void CImageView::OnLButtonDownDraw(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                                   int iFringe, int iCurve, int iDot) {
    if (mods.None()) {
        if (hitLevel == SelectionLevel::None) {
            // Start new curve
            inputHandler.StartNewCurve(P);
            // Execute command
            AddDotCommand* pCmd = new AddDotCommand(&digitInfo, 
                inputHandler.iActiveFringe, 
                inputHandler.iActiveCurve, 0, 
                CDPoint(P.x, P.y));
            cmdDispatcher.Execute(pCmd);
        }
        else if (hitLevel == SelectionLevel::Dot) {
            // Continue from curve end
            inputHandler.ContinueCurve(iFringe, iCurve, iDot);
        }
    }
    else if (mods.Ctrl() && hitLevel == SelectionLevel::Dot) {
        // Connect curves
        inputHandler.ConnectCurves(iFringe, iCurve, iDot);
    }
}
```

**ImageView.cpp** - OnRButtonDown:
```cpp
afx_msg void CImageView::OnRButtonDown(UINT nFlags, CPoint point) {
    if (currentMode == EditMode::Draw) {
        inputHandler.EndCurrentCurve();
    }
}
```

**ImageView.cpp** - OnKeyDown (Backspace):
```cpp
case VK_BACK:
    if (currentMode == EditMode::Draw) {
        RemoveLastDotCommand* pCmd = new RemoveLastDotCommand(&digitInfo);
        cmdDispatcher.Execute(pCmd);
    }
    break;
```

#### 2.4 Add Draw Mode Tests

**Tests/DigitMode/DrawModeTest.cpp** (NEW):
```cpp
class DrawModeTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    InputHandler inputHandler;
    CommandDispatcher cmdDispatcher;
    
    void SetUp() override {
        digitInfo.Init();
    }
};

TEST_F(DrawModeTest, StartNewCurveCreatesNewFringe) {
    inputHandler.StartNewCurve(CPoint(100, 100));
    
    EXPECT_GE(inputHandler.iActiveFringe, 0);
    EXPECT_EQ(0, inputHandler.iActiveCurve);
}

TEST_F(DrawModeTest, AddDotCommandWorksWithUndo) {
    // Create fringe
    inputHandler.StartNewCurve(CPoint(100, 100));
    
    // Add first dot via command
    AddDotCommand* pCmd = new AddDotCommand(&digitInfo,
        inputHandler.iActiveFringe, 0, 0, CDPoint(100, 100));
    cmdDispatcher.Execute(pCmd);
    
    EXPECT_TRUE(cmdDispatcher.CanUndo());
    EXPECT_FALSE(cmdDispatcher.CanRedo());
    
    // Undo
    cmdDispatcher.Undo();
    EXPECT_FALSE(cmdDispatcher.CanUndo());
    EXPECT_TRUE(cmdDispatcher.CanRedo());
}

TEST_F(DrawModeTest, ContinueCurveTransfersDrawing) {
    inputHandler.StartNewCurve(CPoint(100, 100));
    int iF1 = inputHandler.iActiveFringe;
    
    // End first curve
    inputHandler.EndCurrentCurve();
    
    // Continue from a dot (simulated)
    inputHandler.ContinueCurve(iF1, 0, 0);
    
    EXPECT_EQ(iF1, inputHandler.iActiveFringe);
}
```

### Deliverables (Phase 2)

- ✅ InputHandler fully implements Draw mode
- ✅ AddDotCommand, RemoveLastDotCommand working
- ✅ ImageView mouse handlers route to Draw logic
- ✅ Draw mode unit tests pass
- ✅ Can draw curves interactively (no visual feedback yet, just commands execute)

### Success Criteria

- [ ] All Draw mode tests pass
- [ ] Backspace removes dots correctly
- [ ] Right-click ends curves
- [ ] Can add dots to undo/redo stack
- [ ] No crashes with empty/complex drawings

---

## Phase 3: Selection & Navigate (Weeks 5–6)

**Goal**: Implement Navigate mode with full selection (Dot ? Edge ? Curve ? Fringe hierarchy).

### Tasks

#### 3.1 Complete HitTester Implementation

**HitTester.cpp**:
```cpp
SelectionLevel HitTester::HitTest(CPoint P, int& outFringe, int& outCurve, int& outDot) {
    // Iterate fringes in reverse (top to bottom z-order)
    for (int iF = digitInfo.Fringes.GetSize() - 1; iF >= 0; iF--) {
        // Check dots first (highest priority)
        // Check edges
        // Return SelectionLevel::None if nothing found
    }
}
```

#### 3.2 Implement Navigate Mode Handlers

**ImageView.cpp** - OnLButtonDownNavigate:
```cpp
void CImageView::OnLButtonDownNavigate(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                                       int iFringe, int iCurve, int iDot) {
    SelectionManager::SelectedObject obj;
    
    if (hitLevel == SelectionLevel::None) {
        if (mods.None()) {
            selectionMgr.Clear();
        }
    }
    else {
        obj.iFringe = iFringe;
        obj.iCurve = iCurve;
        obj.iDot = iDot;
        obj.level = hitLevel;
        
        if (mods.None()) {
            selectionMgr.SelectDot(iFringe, iCurve, iDot);
        }
        else if (mods.Ctrl()) {
            selectionMgr.AddToSelection(obj);
        }
        else if (mods.Alt()) {
            selectionMgr.SelectFringe(iFringe);
        }
    }
}
```

#### 3.3 Implement Box Selection

**SelectionManager.h** - Add:
```cpp
void BoxSelect(CRect box, BOOL bAddToSelection = FALSE);
```

**SelectionManager.cpp**:
```cpp
void SelectionManager::BoxSelect(CRect box, BOOL bAddToSelection) {
    CArray<SelectedObject> boxSelection;
    
    // Hit test all dots/edges/curves in box
    // Apply inclusion rules (Edge if intersects or inside, Curve if ALL edges, etc.)
    
    if (!bAddToSelection) selection.RemoveAll();
    for (auto& obj : boxSelection) {
        AddToSelection(obj);
    }
}
```

#### 3.4 Implement Selection Display

**CImageView.OnPaint** - Draw selection feedback:
```cpp
void CImageView::OnPaint() {
    // ... existing render code ...
    
    // Draw selection highlights
    SelectionLevel level = selectionMgr.GetLevel();
    for (int i = 0; i < selectionMgr.GetCount(); i++) {
        auto& obj = selectionMgr.GetAt(i);
        DrawSelectionHighlight(pDC, obj);
    }
}

void CImageView::DrawSelectionHighlight(CDC* pDC, const SelectionManager::SelectedObject& obj) {
    // Draw colored outline or crosshair at selected dot
    // Draw thicker edge or curve outline
    // TODO: Color scheme for selection
}
```

#### 3.5 Selection Tests

**Tests/DigitMode/SelectionWorkflowTest.cpp** (NEW):
```cpp
class SelectionWorkflowTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    SelectionManager selectionMgr;
    HitTester hitTester;
    
    void SetUp() override {
        digitInfo.Init();
        // Create test fringe with 3 curves, 5 dots each
    }
};

TEST_F(SelectionWorkflowTest, SelectDotViaMouse) {
    int iF, iC, iD;
    SelectionLevel hit = hitTester.HitTest(CPoint(100, 100), iF, iC, iD);
    
    if (hit == SelectionLevel::Dot) {
        selectionMgr.SelectDot(iF, iC, iD);
        EXPECT_EQ(SelectionLevel::Dot, selectionMgr.GetLevel());
        EXPECT_EQ(1, selectionMgr.GetCount());
    }
}

TEST_F(SelectionWorkflowTest, CtrlClickAddsToSelection) {
    // Select first dot
    selectionMgr.SelectDot(0, 0, 0);
    
    // Ctrl+click second dot
    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iFringe = 0;
    obj.iCurve = 0;
    obj.iDot = 1;
    
    bool success = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success);
    EXPECT_EQ(2, selectionMgr.GetCount());
}

TEST_F(SelectionWorkflowTest, AltClickPromotesToFringe) {
    selectionMgr.SelectDot(0, 0, 0);
    selectionMgr.SelectDot(0, 1, 0);  // Two different curves, same fringe
    
    selectionMgr.PromoteToFringe();
    
    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(1, selectionMgr.GetCount());  // Only one fringe
}
```

### Deliverables (Phase 3)

- ✅ HitTester fully implemented
- ✅ Navigate mode handlers wired
- ✅ Box selection working
- ✅ Selection state visual feedback
- ✅ Navigation workflow tests pass

### Success Criteria

- [ ] Can click to select dots/curves/fringes
- [ ] Ctrl+Click adds to selection
- [ ] Alt+Click promotes to fringe
- [ ] Box select works with inclusion rules
- [ ] Selection highlights visible on screen

---

## Phase 4: Dot Edit Mode (Week 7)

**Goal**: Implement Dot Edit mode (move/insert/delete dots, drag edges).

### Tasks

#### 4.1 Add Dragging State to ImageView

**ImageView.h**:
```cpp
BOOL bDragging = FALSE;
CPoint dragStart;
MoveGeometryCommand* pMoveCmd = NULL;
```

#### 4.2 Implement OnLButtonDownDotEdit

**ImageView.cpp**:
```cpp
void CImageView::OnLButtonDownDotEdit(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                                      int iFringe, int iCurve, int iDot) {
    if (hitLevel == SelectionLevel::Dot) {
        if (mods.Alt()) {
            // Delete dot
            RemoveDotCommand* pCmd = new RemoveDotCommand(&digitInfo, iFringe, iCurve, iDot);
            cmdDispatcher.Execute(pCmd);
        }
        else {
            // Start drag to move
            bDragging = TRUE;
            dragStart = P;
            pMoveCmd = new MoveGeometryCommand(&digitInfo);
            CDPoint oldPos = digitInfo.GetDot(iFringe, iCurve, iDot);
            pMoveCmd->AddPoint(iFringe, iCurve, iDot, oldPos, CDPoint(P.x, P.y));
        }
    }
    else if (hitLevel == SelectionLevel::Edge) {
        // Insert dot on edge
        InsertDotCommand* pCmd = new InsertDotCommand(&digitInfo, iFringe, iCurve, iDot, CDPoint(P.x, P.y));
        cmdDispatcher.Execute(pCmd);
    }
}
```

#### 4.3 Implement OnMouseMove (Drag Preview)

**ImageView.cpp**:
```cpp
afx_msg void CImageView::OnMouseMove(UINT nFlags, CPoint point) {
    // ... existing cursor/tooltip code ...
    
    if (bDragging && pMoveCmd != NULL) {
        // Update move command with current position
        pMoveCmd->UpdatePreviewPosition(point);
        Invalidate(FALSE);  // Live preview
    }
}
```

#### 4.4 Implement OnLButtonUp (Commit Drag)

**ImageView.cpp**:
```cpp
afx_msg void CImageView::OnLButtonUp(UINT nFlags, CPoint point) {
    if (bDragging && pMoveCmd != NULL) {
        // Finalize and execute
        pMoveCmd->FinalizePosition(point);
        cmdDispatcher.Execute(pMoveCmd);
        pMoveCmd = NULL;
    }
    bDragging = FALSE;
}
```

#### 4.5 Dot Edit Commands

**Commands/RemoveDotCommand.h & .cpp**:
```cpp
class RemoveDotCommand : public Command {
private:
    int iFringe, iCurve, iDot;
    CDPoint savedPoint;
    CDigitInfo* pDigit;
    
public:
    RemoveDotCommand(CDigitInfo* pD, int iF, int iC, int iD);
    void Execute() override;
    void Undo() override;
    CString GetName() const override { return "Remove Dot"; }
};
```

**Commands/InsertDotCommand.h & .cpp**: (Similar)

**Commands/MoveGeometryCommand.h & .cpp**: (From IMPLEMENT_UX.md, enhance for live preview)

### Deliverables (Phase 4)

- ✅ Move dots with drag
- ✅ Insert dots on edges
- ✅ Delete dots with Alt+Click
- ✅ Live preview during drag
- ✅ Undo/Redo for all operations

### Success Criteria

- [ ] Can drag dots smoothly
- [ ] Live preview visible
- [ ] Inserting dots works
- [ ] Alt+Click deletes
- [ ] All operations undoable

---

## Phase 5: Commands & Undo/Redo (Weeks 8–9)

**Goal**: Implement all remaining commands (Renumber, Simplify, Split, Merge, Subdivide, Auto-number, etc.).

### Tasks

#### 5.1 Implement Core Selection-Based Commands

**Commands/RenumberCommand.h & .cpp**
**Commands/SimplifyCommand.h & .cpp**
**Commands/DeleteSelectionCommand.h & .cpp**

#### 5.2 Implement Advanced Geometry Commands

**Commands/SplitCurveCommand.h & .cpp**
**Commands/MergeCurvesCommand.h & .cpp**
**Commands/SubdivideCommand.h & .cpp**

#### 5.3 Wire Commands to ImageView Keyboard

**ImageView.cpp** - OnKeyDown:
```cpp
case '+':
case VK_OEM_PLUS:
    OnKeyNumberIncrement();
    break;
case 'S':
    OnKeySimplify();
    break;
case VK_SHIFT + 'S':  // Shift+S for Subdivide
    OnKeySubdivide();
    break;
case 'M':
    OnKeyMergeCurves();
    break;
case 'X':
    OnKeySplitCurve();
    break;
```

**Implementation** (from IMPLEMENT_UX.md, wire to ImageView):
```cpp
void CImageView::OnKeyNumberIncrement() {
    if (selectionMgr.GetLevel() == SelectionLevel::Fringe ||
        selectionMgr.GetLevel() == SelectionLevel::Curve) {
        
        CArray<int> fringes;
        for (int i = 0; i < selectionMgr.GetCount(); i++) {
            fringes.Add(selectionMgr.GetAt(i).iFringe);
        }
        
        double currentNum = digitInfo.GetFringe(fringes[0]).GetNumber();
        double step = digitInfo.GetNumberStep();
        
        RenumberCommand* pCmd = new RenumberCommand(&digitInfo, fringes, currentNum + step);
        cmdDispatcher.Execute(pCmd);
    }
}
```

#### 5.4 Command Tests

**Tests/DigitMode/CommandsTest.cpp** (NEW):
```cpp
class CommandsTest : public ::testing::Test {
protected:
    CDigitInfo digitInfo;
    CommandDispatcher cmdDispatcher;
    
    void SetUp() override {
        digitInfo.Init();
        // Create test data
    }
};

TEST_F(CommandsTest, RenumberCommand) {
    CArray<int> fringes = { 0 };
    RenumberCommand cmd(&digitInfo, fringes, 2.5);
    
    double oldNum = digitInfo.GetFringe(0).GetNumber();
    cmd.Execute();
    EXPECT_EQ(2.5, digitInfo.GetFringe(0).GetNumber());
    
    cmd.Undo();
    EXPECT_EQ(oldNum, digitInfo.GetFringe(0).GetNumber());
}

TEST_F(CommandsTest, SimplifyCommand) {
    SimplifyCommand cmd(&digitInfo, 0, 0, 1.0);
    int dotsBefore = digitInfo.GetFringe(0).DotCount(0);
    
    cmd.Execute();
    int dotsAfter = digitInfo.GetFringe(0).DotCount(0);
    
    EXPECT_LE(dotsAfter, dotsBefore);
    
    cmd.Undo();
    EXPECT_EQ(dotsBefore, digitInfo.GetFringe(0).DotCount(0));
}

// ... etc for Split, Merge, Subdivide
```

### Deliverables (Phase 5)

- ✅ All commands from UX v1.0 spec implemented
- ✅ Keyboard shortcuts wired
- ✅ All commands undoable/redoable
- ✅ Menu items (optional, for discoverability)
- ✅ Command tests pass

### Success Criteria

- [ ] +/- changes fringe numbers
- [ ] S simplifies curves
- [ ] Shift+S subdivides
- [ ] M merges curves
- [ ] X splits curves
- [ ] All operations undoable

---

## Phase 6: Polish & Testing (Week 10)

**Goal**: Integration, edge cases, visual polish, comprehensive testing.

### Tasks

#### 6.1 Complete Cursor & Tooltip Feedback

**CursorManager.cpp** - Full implementation:
- Mode-based cursors (Arrow, Crosshair, Vertex)
- Modifier overlays (+ for Ctrl, ? for Shift, ! for Alt)
- Composite cursor generation

**TooltipGenerator.cpp** - Full implementation:
- Format: `#2.5 / 1(1) / 12` for dots
- Format: `Curve – 34 dots` for curves
- Format: `Fringe #2.5 (3 curves)` for fringes

#### 6.2 Context Menus

**ImageView.cpp** - OnRButtonDown (in Navigate mode):
```cpp
void CImageView::OnContextMenu(CPoint P) {
    CMenu menu;
    menu.CreatePopupMenu();
    
    if (selectionMgr.GetLevel() == SelectionLevel::Dot) {
        menu.AppendMenu(MF_STRING, ID_MENU_DELETE, "Delete Dot");
        menu.AppendMenu(MF_STRING, ID_MENU_INSERT, "Insert Dot");
        menu.AppendMenu(MF_STRING, ID_MENU_SELECT_CURVE, "Select Curve");
    }
    else if (selectionMgr.GetLevel() == SelectionLevel::Curve) {
        menu.AppendMenu(MF_STRING, ID_MENU_DELETE, "Delete Curve");
        menu.AppendMenu(MF_STRING, ID_MENU_SPLIT, "Split Curve");
        menu.AppendMenu(MF_STRING, ID_MENU_SIMPLIFY, "Simplify");
    }
    // ... etc
    
    CPoint screenP = P;
    ClientToScreen(&screenP);
    menu.TrackPopupMenu(TPM_LEFTALIGN, screenP.x, screenP.y, this);
}
```

#### 6.3 Edge Case Testing

**Tests/DigitMode/EdgeCaseTest.cpp** (NEW):
```cpp
TEST(EdgeCases, EmptyFringeCanBeDeleted) {
    // Create and delete empty fringe
}

TEST(EdgeCases, SinglePointFringeWorks) {
    // Create fringe with 1 dot, all operations
}

TEST(EdgeCases, ConnectSelfDoesNotCrash) {
    // Try to connect curve to itself
}

TEST(EdgeCases, DeleteAllDotsRemovesFringe) {
    // Delete all dots in fringe
}

TEST(EdgeCases, RenumberToSameNumberWorks) {
    // Renumber fringe to its current number (no-op)
}

TEST(EdgeCases, UndoRedoStackLimits) {
    // 100+ operations, still works?
}
```

#### 6.4 Regression Test Suite

**Tests/DigitMode/RegressionTest.cpp** (NEW):
```cpp
class RegressionTest : public ::testing::Test {
protected:
    // Load real sample files from corpus
    void SetUp() override {
        // Load 5-10 representative .zap files
    }
};

TEST_F(RegressionTest, LoadSaveRoundTrip) {
    // Load ? modify ? save ? load ? verify
}

TEST_F(RegressionTest, ComplexEditingWorkflow) {
    // Simulate real user: draw, edit, renumber, simplify, save
}

TEST_F(RegressionTest, LargeFilePerformance) {
    // 1000+ fringes: draw, hit-test, select, command speed
}
```

#### 6.5 Visual Testing

**Manual Test Protocol**:
1. **Draw**: Click to draw curves, Shift for range, Ctrl to connect
2. **Edit**: Switch to Dot Edit, drag points, insert/delete
3. **Renumber**: Select fringe, press +/-, verify tooltip updates
4. **Simplify**: Select curve, press S, verify dots reduced
5. **Undo/Redo**: Ctrl+Z/Y through entire workflow
6. **Cursors**: Verify cursor changes with mode + modifiers
7. **Tooltips**: Hover over dots/curves, verify format

#### 6.6 Documentation

**Docs/UX_IMPLEMENTATION.md** (NEW):
- User guide (frozen from spec)
- Developer guide (architecture + file structure)
- Troubleshooting (common issues)

### Deliverables (Phase 6)

- ✅ Full cursor feedback system
- ✅ Context menus for all object types
- ✅ Comprehensive edge case handling
- ✅ Regression test suite
- ✅ Manual testing protocol completed
- ✅ User/developer documentation

### Success Criteria

- [ ] All regression tests pass
- [ ] No crashes on edge cases
- [ ] Cursors update correctly
- [ ] Tooltips display correctly
- [ ] Context menus show correct options
- [ ] Can complete 10-minute user walkthrough (from UX spec) without issues

---

## File Organization

### Directory Structure

```
Digit/
├── DigitMode/
│   ├── DigitInfo.h
│   ├── DigitInfo.cpp
│   ├── CFringe.h
│   ├── CFringe.cpp
│   ├── InputHandler.h                (Phase 1)
│   ├── InputHandler.cpp              (Phase 1)
│   ├── SelectionManager.h            (Phase 1)
│   ├── SelectionManager.cpp          (Phase 1)
│   ├── HitTester.h                   (Phase 1)
│   ├── HitTester.cpp                 (Phase 1)
│   ├── CursorManager.h               (Phase 1)
│   ├── CursorManager.cpp             (Phase 1)
│   ├── TooltipGenerator.h            (Phase 1)
│   ├── TooltipGenerator.cpp          (Phase 1)
│   ├── CommandDispatcher.h           (Phase 1)
│   ├── CommandDispatcher.cpp         (Phase 1)
│   └── Commands/                     (Phase 2+)
│       ├── Command.h                 (Phase 1)
│       ├── AddDotCommand.h/.cpp      (Phase 2)
│       ├── RemoveDotCommand.h/.cpp   (Phase 2)
│       ├── MoveGeometryCommand.h/.cpp(Phase 4)
│       ├── RenumberCommand.h/.cpp    (Phase 5)
│       ├── SimplifyCommand.h/.cpp    (Phase 5)
│       ├── SplitCurveCommand.h/.cpp  (Phase 5)
│       ├── MergeCurvesCommand.h/.cpp (Phase 5)
│       ├── SubdivideCommand.h/.cpp   (Phase 5)
│       └── DeleteSelectionCommand.h/.cpp (Phase 5)
├── Tests/DigitMode/                  (New test files)
│   ├── InputHandlerTest.cpp          (Phase 1)
│   ├── SelectionManagerTest.cpp      (Phase 1)
│   ├── HitTesterTest.cpp             (Phase 1)
│   ├── DrawModeTest.cpp              (Phase 2)
│   ├── SelectionWorkflowTest.cpp     (Phase 3)
│   ├── CommandsTest.cpp              (Phase 5)
│   ├── EdgeCaseTest.cpp              (Phase 6)
│   └── RegressionTest.cpp            (Phase 6)
└── Docs/
    ├── FRINGES_EDITOR_UX_SPECIFICATIONS.md
    ├── IMPLEMENT_UX.md
    ├── ROADMAP_UX.md                 (this file)
    └── UX_IMPLEMENTATION.md          (Phase 6)
```

### Compilation Order

1. **Core Data Model**: CFringe.h/.cpp (already done)
2. **Core Infrastructure**: InputHandler, SelectionManager, HitTester, CommandDispatcher
3. **Commands**: Base Command class, then specific commands as phases add them
4. **UI Integration**: CursorManager, TooltipGenerator, ImageView modifications
5. **Tests**: Corresponding unit tests for each component

---

## Testing Strategy

### Unit Testing (By Phase)

| Phase | Component | Test File | Coverage Goal |
|-------|-----------|-----------|---------------|
| 1 | SelectionManager | SelectionManagerTest.cpp | 95%+ |
| 1 | HitTester | HitTesterTest.cpp | 90%+ |
| 2 | InputHandler (Draw) | DrawModeTest.cpp | 85%+ |
| 2 | AddDotCommand | CommandsTest.cpp | 95%+ |
| 3 | HitTester (full) | SelectionWorkflowTest.cpp | 90%+ |
| 4 | Move/Insert/Delete | DotEditModeTest.cpp | 85%+ |
| 5 | All Commands | CommandsTest.cpp | 90%+ |
| 6 | Edge Cases | EdgeCaseTest.cpp | 80%+ |
| 6 | Regressions | RegressionTest.cpp | 80%+ |

### Integration Testing (By Phase)

| Phase | Scenario | Success Criteria |
|-------|----------|-----------------|
| 2 | Draw 3 curves | Can add dots, undo, redo |
| 3 | Select dots | Ctrl+Click adds, Alt+Click promotes |
| 4 | Edit geometry | Drag, insert, delete all work |
| 5 | Renumber selection | +/- updates all fringes |
| 6 | Complete workflow | 10-min user scenario works |

### Manual Testing (Phase 6)

**Test Matrix**:
- 3 modes × 4 modifier combinations × 5 object types = 60 basic interactions
- Each interaction: click, drag, keyboard, right-click
- Undo/Redo for each: forward + backward
- Large file (1000+ fringes) performance check

---

## Success Criteria & Sign-Off

### Phase 1 Success Criteria
- [ ] All core classes compile without errors
- [ ] SelectionManager, HitTester unit tests 100% pass
- [ ] ImageView initializes without crashing
- [ ] TRACE logging shows expected flow

### Phase 2 Success Criteria
- [ ] Can draw curves interactively
- [ ] Backspace removes last dot
- [ ] Right-click ends curve
- [ ] All Draw mode tests pass
- [ ] Undo/Redo works for drawing

### Phase 3 Success Criteria
- [ ] Can select dots/curves/fringes by clicking
- [ ] Ctrl+Click adds to selection
- [ ] Alt+Click promotes to Fringe
- [ ] Box selection works with inclusion rules
- [ ] Hit testing tests pass

### Phase 4 Success Criteria
- [ ] Can drag dots to move them
- [ ] Can insert dots on edges
- [ ] Alt+Click deletes dots
- [ ] Live preview visible during drag
- [ ] All Dot Edit tests pass

### Phase 5 Success Criteria
- [ ] +/? changes fringe numbers
- [ ] S simplifies curves
- [ ] Shift+S subdivides
- [ ] M merges curves
- [ ] X splits curves
- [ ] All command tests pass

### Phase 6 Success Criteria (Sign-Off)
- [ ] All regression tests pass
- [ ] No crashes on edge cases
- [ ] Cursors update correctly
- [ ] Tooltips display correctly
- [ ] Manual test protocol completed
- [ ] User walkthrough (10-min from UX spec) successful
- [ ] Code review approved
- [ ] Integration tests green

### Final Acceptance (UX v1.0)

Project sign-off when:
1. **All phases complete**: Features 1–6 working
2. **All tests pass**: Unit + Integration + Regression + Manual
3. **UX v1.0 checklist**: All items in IMPLEMENT_UX.md Appendix checked
4. **Performance acceptable**: <5% regression vs baseline
5. **Documentation complete**: User guide + Developer guide
6. **Code review passed**: Senior developer approval

---

## Estimated Timeline

```
Week 1-2:  Phase 1 (Foundation)
Week 3-4:  Phase 2 (Draw Mode)
Week 5-6:  Phase 3 (Navigation)
Week 7:    Phase 4 (Dot Edit)
Week 8-9:  Phase 5 (Commands)
Week 10:   Phase 6 (Polish & Testing)

Total: 10 weeks (2.5 months)
Can be done incrementally: Each phase deliverable independently
```

---

## Risks & Mitigation

| Risk | Likelihood | Mitigation |
|------|-----------|-----------|
| **Mode dispatch bugs** | Medium | Early testing, TRACE logging |
| **Hit testing accuracy** | Medium | Extensive geometry tests |
| **Performance regression** | Low | Benchmark early + often |
| **Undo/Redo consistency** | Medium | Command pattern discipline |
| **Selection state bugs** | High | Comprehensive state tests |
| **Integration complexity** | Medium | Phased approach, integration tests |

---

## Next Steps

1. **Start Phase 1**: Create core classes (Week 1)
2. **Daily builds**: Ensure no compilation errors
3. **Weekly reviews**: Check progress against milestones
4. **Continuous testing**: Run unit tests after each phase
5. **User feedback**: Manual testing with real workflows
6. **Documentation**: Update as you go, consolidate at end

---

**End of Implementation Roadmap**

**Ready to start Phase 1? Begin with InputHandler, SelectionManager core classes.**


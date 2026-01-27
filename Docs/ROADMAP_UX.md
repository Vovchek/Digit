# Phase 3: Implementation Roadmap — UX v1.0 to Code (Segment-Primary Model)

**Status**: Detailed step-by-step development plan  
**Date**: 2026-01-26  
**Purpose**: Guide developers from architecture (IMPLEMENT_UX.md) → working UI with full UX v1.0 compliance  
**Audience**: Developers, QA, project leads

**Model**: Segment-primary architecture. Fringes are logical groupings, not containers.

**Modernization**: Uses STL (std::vector, std::string, std::unique_ptr) instead of MFC auxiliary types.

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
| **2** | Draw mode (create/continue/connect segments) | 3–4 | Full Draw workflow tested | Medium |
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
├── CFringeSegment.h            (RENAME from CFringe.h)
├── CFringeSegment.cpp          (RENAME from CFringe.cpp)
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
└── TooltipGenerator.cpp        (NEW)

Tests/DigitMode/
├── CFringeSegmentTest.cpp      (RENAME from CFringeTest.cpp)
├── CFringeSegmentFileIOTest.cpp (RENAME from CFringeFileIOTest.cpp)
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
    int iActiveSegment = -1;  // Current segment being drawn
    
public:
    void SetMode(EditMode newMode);
    EditMode GetMode() const { return currentMode; }
    bool IsInDrawMode() const { return currentMode == EditMode::Draw; }
    
    void StartNewSegment(CPoint P);
    void ContinueSegment(int iSegment, int iDot);
    void ConnectSegments(int iSegment, int iDot);
    void EndCurrentSegment();
};
```

**SelectionManager.h**
```cpp
#pragma once
#include <vector>

enum class SelectionLevel { None, Dot, Edge, Segment, Fringe };

class SelectionManager {
public:
    struct SelectedObject {
        SelectionLevel level = SelectionLevel::None;
        int iSegment = -1;
        int iDot = -1;
        int iEdge = -1;
        double Number = 0.0;  // For Fringe-level selection
        
        bool IsValid() const { return level != SelectionLevel::None; }
    };
    
private:
    std::vector<SelectedObject> selection;
    
public:
    void SelectDot(int iSegment, int iDot);
    void SelectEdge(int iSegment, int iEdge);
    void SelectSegment(int iSegment);
    void SelectFringe(double number);  // Selects all segments with this Number
    
    bool AddToSelection(const SelectedObject& obj);
    void PromoteToFringe();
    
    SelectionLevel GetLevel() const;
    size_t GetCount() const { return selection.size(); }
    const SelectedObject& GetAt(size_t i) const { return selection[i]; }
    void Clear() { selection.clear(); }
    bool IsEmpty() const { return selection.empty(); }
};
```

**HitTester.h**
```cpp
#pragma once
#include "SelectionManager.h"

class HitTester {
private:
    static constexpr int HIT_TOLERANCE = 5;
    
public:
    SelectionLevel HitTest(CPoint P, int& outSegment, int& outDot);
    
private:
    double DotDistance(CPoint P, CDPoint dot) const;
    double DistanceToSegment(CPoint P, CDPoint A, CDPoint B) const;
};
```

**Command.h**
```cpp
#pragma once
#include <string>

class Command {
public:
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() { Execute(); }
    virtual std::string GetName() const { return "Command"; }
    virtual ~Command() = default;
};
```

**CommandDispatcher.h**
```cpp
#pragma once
#include "Commands/Command.h"
#include <vector>
#include <memory>

class CommandDispatcher {
private:
    std::vector<std::unique_ptr<Command>> undoStack;
    std::vector<std::unique_ptr<Command>> redoStack;
    
public:
    ~CommandDispatcher() = default;
    
    void Execute(std::unique_ptr<Command> cmd);
    void Undo();
    void Redo();
    
    bool CanUndo() const { return !undoStack.empty(); }
    bool CanRedo() const { return !redoStack.empty(); }
    
    std::string GetUndoLabel() const;
    std::string GetRedoLabel() const;
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
#include <string>

class TooltipGenerator {
public:
    std::string GetTooltip(const SelectionManager::SelectedObject& obj, const CDigitInfo& digit);
};
```

#### 1.2 Implement Core Classes (Minimal Working Version)

**Priority**: InputHandler, SelectionManager, HitTester only.  
**Others**: CursorManager, TooltipGenerator can be stubbed (return empty/default values).

**InputHandler.cpp** - Minimal stubs:
```cpp
void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        EndCurrentSegment();
    }
    currentMode = newMode;
    // TODO: Update cursor
}

void InputHandler::StartNewSegment(CPoint P) {
    // TODO: Implement
    // Log or TRACE for debugging
}

void InputHandler::EndCurrentSegment() {
    // TODO: Implement
}
// ... etc
```

**SelectionManager.cpp** - Full implementation (straightforward):
```cpp
void SelectionManager::SelectDot(int iSegment, int iDot) {
    selection.clear();
    SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = iSegment;
    obj.iDot = iDot;
    selection.push_back(obj);
}

void SelectionManager::SelectFringe(double number) {
    selection.clear();
    // Query digitInfo for all segments with this Number
    // For each matching segment, add to selection
    // (Implementation requires access to digitInfo, pass as parameter or make member)
}

// ... etc (from IMPLEMENT_UX.md)
```

**HitTester.cpp** - Full implementation:
```cpp
SelectionLevel HitTester::HitTest(CPoint P, int& outSegment, int& outDot) {
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
    
    int iSegment, iDot;
    SelectionLevel hitLevel = hitTester.HitTest(point, iSegment, iDot);
    
    // Log or TRACE for debugging
    
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
    sel.SelectDot(0, 5);
    
    EXPECT_EQ(SelectionLevel::Dot, sel.GetLevel());
    EXPECT_EQ(1, sel.GetCount());
    EXPECT_EQ(0, sel.GetAt(0).iSegment);
    EXPECT_EQ(5, sel.GetAt(0).iDot);
}

TEST(SelectionManager, SelectFringeSelectsAllMatchingSegments) {
    SelectionManager sel;
    CDigitInfo digitInfo;
    // Create test data: 3 segments with Number=1.0, 2 with Number=2.0
    
    sel.SelectFringe(1.0);
    
    EXPECT_EQ(SelectionLevel::Fringe, sel.GetLevel());
    EXPECT_EQ(3, sel.GetCount());  // All 3 segments with Number=1.0
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

- ✅ All core classes created with public interfaces
- ✅ SelectionManager, HitTester fully implemented + unit tested
- ✅ InputHandler, CommandDispatcher stubbed + wired into ImageView
- ✅ Basic logging for debugging
- ✅ CFringe renamed to CFringeSegment
- ✅ No functional UI yet, but architecture in place

### Success Criteria

- [ ] Project compiles without errors
- [ ] InputHandler, SelectionManager, HitTester unit tests pass
- [ ] CImageView initializes new components without crashing
- [ ] Can turn on/off logging
- [ ] CFringeSegment class exists and works with existing tests

---

## Phase 2: Draw Mode (Weeks 3–4)

**Goal**: Implement full Draw workflow (create → continue → connect → end segments).

### Tasks

#### 2.1 Complete InputHandler Implementation

**InputHandler.cpp** - Full Draw mode logic:
```cpp
void InputHandler::StartNewSegment(CPoint P) {
    // Create new segment with next number
    double newNumber = digitInfo.GetNextNumber();
    iActiveSegment = digitInfo.CreateSegment(newNumber);
    
    // Add first dot
    digitInfo.AddDotToSegment(iActiveSegment, CDPoint(P.x, P.y));
    // Log: "Started new segment: segment=%d, dot=0\n", iActiveSegment
}

void InputHandler::ContinueSegment(int iSegment, int iDot) {
    iActiveSegment = iSegment;
    // Log: "Continuing segment: segment=%d\n", iSegment
}

void InputHandler::ConnectSegments(int iSegment, int iDot) {
    // Connect iActiveSegment to the specified segment
    // Transfer drawing to the free end of the target segment
    // Log: "Connecting segments\n"
    
    iActiveSegment = iSegment;
    // TODO: Mark segments as connected
}

void InputHandler::EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        // Log: "Ended segment: segment=%d\n", iActiveSegment
        iActiveSegment = -1;
    }
}
```

#### 2.2 Add Drawing Commands

**Commands/AddDotCommand.h & .cpp**:
```cpp
class AddDotCommand : public Command {
private:
    int iSegment, iDot;
    CDPoint point;
    CDigitInfo* pDigit;
    
public:
    AddDotCommand(CDigitInfo* pD, int iSeg, int iD_idx, CDPoint p);
    void Execute() override { pDigit->InsertDot(iSegment, iDot, point); }
    void Undo() override { pDigit->RemoveDot(iSegment, iDot); }
    std::string GetName() const override { return "Add Dot"; }
};
```

**Commands/RemoveLastDotCommand.h & .cpp**: (Similar pattern)

#### 2.3 Integrate Draw Mode into ImageView

**ImageView.cpp** - OnLButtonDown for Draw mode:
```cpp
void CImageView::OnLButtonDownDraw(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                                   int iSegment, int iDot) {
    if (mods.None()) {
        if (hitLevel == SelectionLevel::None) {
            // Start new segment
            inputHandler.StartNewSegment(P);
            // Execute command
            auto pCmd = std::make_unique<AddDotCommand>(&digitInfo, 
                inputHandler.iActiveSegment, 0, 
                CDPoint(P.x, P.y));
            cmdDispatcher.Execute(std::move(pCmd));
        }
        else if (hitLevel == SelectionLevel::Dot) {
            // Continue from segment end
            inputHandler.ContinueSegment(iSegment, iDot);
        }
    }
    else if (mods.ctrl && hitLevel == SelectionLevel::Dot) {
        // Connect segments
        inputHandler.ConnectSegments(iSegment, iDot);
    }
}
```

**ImageView.cpp** - OnRButtonDown:
```cpp
afx_msg void CImageView::OnRButtonDown(UINT nFlags, CPoint point) {
    if (currentMode == EditMode::Draw) {
        inputHandler.EndCurrentSegment();
    }
}
```

**ImageView.cpp** - OnKeyDown (Backspace):
```cpp
case VK_BACK:
    if (currentMode == EditMode::Draw) {
        auto pCmd = std::make_unique<RemoveLastDotCommand>(&digitInfo);
        cmdDispatcher.Execute(std::move(pCmd));
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

TEST_F(DrawModeTest, StartNewSegmentCreatesNewSegment) {
    inputHandler.StartNewSegment(CPoint(100, 100));
    
    EXPECT_GE(inputHandler.iActiveSegment, 0);
}

TEST_F(DrawModeTest, AddDotCommandWorksWithUndo) {
    // Create segment
    inputHandler.StartNewSegment(CPoint(100, 100));
    
    // Add first dot via command
    auto pCmd = std::make_unique<AddDotCommand>(&digitInfo,
        inputHandler.iActiveSegment, 0, CDPoint(100, 100));
    cmdDispatcher.Execute(std::move(pCmd));
    
    EXPECT_TRUE(cmdDispatcher.CanUndo());
    EXPECT_FALSE(cmdDispatcher.CanRedo());
    
    // Undo
    cmdDispatcher.Undo();
    EXPECT_FALSE(cmdDispatcher.CanUndo());
    EXPECT_TRUE(cmdDispatcher.CanRedo());
}

TEST_F(DrawModeTest, ContinueSegmentTransfersDrawing) {
    inputHandler.StartNewSegment(CPoint(100, 100));
    int iSeg1 = inputHandler.iActiveSegment;
    
    // End first segment
    inputHandler.EndCurrentSegment();
    
    // Continue from a dot (simulated)
    inputHandler.ContinueSegment(iSeg1, 0);
    
    EXPECT_EQ(iSeg1, inputHandler.iActiveSegment);
}
```

### Deliverables (Phase 2)

- ✅ InputHandler fully implements Draw mode
- ✅ AddDotCommand, RemoveLastDotCommand working
- ✅ ImageView mouse handlers route to Draw logic
- ✅ Draw mode unit tests pass
- ✅ Can draw segments interactively (no visual feedback yet, just commands execute)

### Success Criteria

- [ ] All Draw mode tests pass
- [ ] Backspace removes dots correctly
- [ ] Right-click ends segments
- [ ] Can add dots to undo/redo stack
- [ ] No crashes with empty/complex drawings

---

## Phase 3: Selection & Navigate (Weeks 5–6)

**Goal**: Implement Navigate mode with full selection (Dot → Edge → Segment → Fringe hierarchy).

### Tasks

#### 3.1 Complete HitTester Implementation

**HitTester.cpp**:
```cpp
SelectionLevel HitTester::HitTest(CPoint P, int& outSegment, int& outDot) {
    // Iterate segments in reverse (top to bottom z-order)
    for (int iSeg = static_cast<int>(digitInfo.Segments.size()) - 1; iSeg >= 0; iSeg--) {
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
                                       int iSegment, int iDot) {
    SelectionManager::SelectedObject obj;
    
    if (hitLevel == SelectionLevel::None) {
        if (mods.None()) {
            selectionMgr.Clear();
        }
    }
    else {
        obj.iSegment = iSegment;
        obj.iDot = iDot;
        obj.level = hitLevel;
        
        if (mods.None()) {
            selectionMgr.SelectDot(iSegment, iDot);
        }
        else if (mods.ctrl) {
            selectionMgr.AddToSelection(obj);
        }
        else if (mods.alt) {
            selectionMgr.SelectFringe();  // No segment, selects all with same Number
        }
    }
}
```

#### 3.3 Implement Box Selection

**SelectionManager.h** - Add:
```cpp
void BoxSelect(CRect box, bool bAddToSelection = false);
```

**SelectionManager.cpp**:
```cpp
void SelectionManager::BoxSelect(CRect box, bool bAddToSelection) {
    std::vector<SelectedObject> boxSelection;
    
    // Hit test all dots/edges/segments in box
    // Apply inclusion rules (Edge if intersects or inside, Segment if ALL edges, etc.)
    
    if (!bAddToSelection) selection.clear();
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
    for (size_t i = 0; i < selectionMgr.GetCount(); i++) {
        auto& obj = selectionMgr.GetAt(i);
        DrawSelectionHighlight(pDC, obj);
    }
}

void CImageView::DrawSelectionHighlight(CDC* pDC, const SelectionManager::SelectedObject& obj) {
    // Draw colored outline or crosshair at selected dot
    // Draw thicker edge or segment outline
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
        // Create test fringe with 3 segments, 5 dots each
    }
};

TEST_F(SelectionWorkflowTest, SelectDotViaMouse) {
    int iSeg, iD;
    SelectionLevel hit = hitTester.HitTest(CPoint(100, 100), iSeg, iD);
    
    if (hit == SelectionLevel::Dot) {
        selectionMgr.SelectDot(iSeg, iD);
        EXPECT_EQ(SelectionLevel::Dot, selectionMgr.GetLevel());
        EXPECT_EQ(1, selectionMgr.GetCount());
    }
}

TEST_F(SelectionWorkflowTest, CtrlClickAddsToSelection) {
    // Select first dot
    selectionMgr.SelectDot(0, 0);
    
    // Ctrl+click second dot
    SelectionManager::SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = 0;
    obj.iDot = 1;
    
    bool success = selectionMgr.AddToSelection(obj);
    EXPECT_TRUE(success);
    EXPECT_EQ(2, selectionMgr.GetCount());
}

TEST_F(SelectionWorkflowTest, AltClickSelectsAllWithSameNumber) {
    // Segments 0, 1, 2 have Number=1.0; 3, 4 have Number=2.0
    selectionMgr.SelectDot(0, 0);
    
    // Alt+click on first dot
    ModifierState mods;
    mods.alt = true;
    HitTester tester;
    int iSeg, iD;
    tester.HitTest(CPoint(100, 100), iSeg, iD);  // Assume hits segment 0
    
    selectionMgr.OnLButtonDown(CPoint(100, 100), mods);
    
    EXPECT_EQ(SelectionLevel::Fringe, selectionMgr.GetLevel());
    EXPECT_EQ(3, selectionMgr.GetCount());  // All segments with Number=1.0
}
```

### Deliverables (Phase 3)

- ✅ HitTester fully implemented
- ✅ Navigate mode handlers wired
- ✅ Box selection working
- ✅ Selection state visual feedback
- ✅ Navigation workflow tests pass

### Success Criteria

- [ ] Can click to select dots/segments/fringes
- [ ] Ctrl+Click adds to selection
- [ ] Alt+Click selects all with same Number
- [ ] Box select works with inclusion rules
- [ ] Selection highlights visible on screen

---

## Phase 4: Dot Edit Mode (Week 7)

**Goal**: Implement Dot Edit mode (move/insert/delete dots, drag segments).

### Tasks

#### 4.1 Add Dragging State to ImageView

**ImageView.h**:
```cpp
bool bDragging = false;
CPoint dragStart;
std::unique_ptr<MoveGeometryCommand> pMoveCmd;
```

#### 4.2 Implement OnLButtonDownDotEdit

**ImageView.cpp**:
```cpp
void CImageView::OnLButtonDownDotEdit(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                                      int iSegment, int iDot) {
    if (hitLevel == SelectionLevel::Dot) {
        if (mods.alt) {
            // Delete dot
            auto pCmd = std::make_unique<RemoveDotCommand>(&digitInfo, iSegment, iDot);
            cmdDispatcher.Execute(std::move(pCmd));
        }
        else {
            // Start drag to move
            bDragging = true;
            dragStart = P;
            pMoveCmd = std::make_unique<MoveGeometryCommand>(&digitInfo);
            CDPoint oldPos = digitInfo.GetDot(iSegment, iDot);
            pMoveCmd->AddPoint(iSegment, iDot, oldPos, CDPoint(P.x, P.y));
        }
    }
    else if (hitLevel == SelectionLevel::Edge) {
        // Insert dot on edge
        auto pCmd = std::make_unique<InsertDotCommand>(&digitInfo, iSegment, iDot, CDPoint(P.x, P.y));
        cmdDispatcher.Execute(std::move(pCmd));
    }
}
```

#### 4.3 Implement OnMouseMove (Drag Preview)

**ImageView.cpp**:
```cpp
afx_msg void CImageView::OnMouseMove(UINT nFlags, CPoint point) {
    // ... existing cursor/tooltip code ...
    
    if (bDragging && pMoveCmd) {
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
    if (bDragging && pMoveCmd) {
        // Finalize and execute
        pMoveCmd->FinalizePosition(point);
        cmdDispatcher.Execute(std::move(pMoveCmd));
    }
    bDragging = false;
}
```

#### 4.5 Dot Edit Commands

**Commands/RemoveDotCommand.h & .cpp**:
```cpp
class RemoveDotCommand : public Command {
private:
    int iSegment, iDot;
    CDPoint savedPoint;
    CDigitInfo* pDigit;
    
public:
    RemoveDotCommand(CDigitInfo* pD, int iSeg, int iD);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Remove Dot"; }
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
- [ ] Alt+Click deletes dots
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
        selectionMgr.GetLevel() == SelectionLevel::Segment) {
        
        std::vector<int> fringes;
        for (size_t i = 0; i < selectionMgr.GetCount(); i++) {
            fringes.push_back(selectionMgr.GetAt(i).iSegment);
        }
        
        double currentNum = digitInfo.GetSegment(fringes[0]).GetNumber();
        double step = digitInfo.GetNumberStep();
        
        auto pCmd = std::make_unique<RenumberCommand>(&digitInfo, fringes, currentNum + step);
        cmdDispatcher.Execute(std::move(pCmd));
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
    std::vector<int> fringes = { 0 };
    auto cmd = std::make_unique<RenumberCommand>(&digitInfo, fringes, 2.5);
    
    double oldNum = digitInfo.GetSegment(0).GetNumber();
    cmd->Execute();
    EXPECT_EQ(2.5, digitInfo.GetSegment(0).GetNumber());
    
    cmd->Undo();
    EXPECT_EQ(oldNum, digitInfo.GetSegment(0).GetNumber());
}

TEST_F(CommandsTest, SimplifyCommand) {
    auto cmd = std::make_unique<SimplifyCommand>(&digitInfo, 0, 0, 1.0);
    int dotsBefore = digitInfo.GetSegment(0).DotCount(0);
    
    cmd->Execute();
    int dotsAfter = digitInfo.GetSegment(0).DotCount(0);
    
    EXPECT_LE(dotsAfter, dotsBefore);
    
    cmd->Undo();
    EXPECT_EQ(dotsBefore, digitInfo.GetSegment(0).DotCount(0));
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
- Modifier overlays (+ for Ctrl, ↕ for Shift, ! for Alt)
- Composite cursor generation

**TooltipGenerator.cpp** - Full implementation:
- Format: `#2.5 / 1(1) / 12` for dots
- Format: `Segment – 34 dots` for segments
- Format: `Fringe #2.5 (3 segments)` for fringes

#### 6.2 Context Menus

**ImageView.cpp** - OnRButtonDown (in Navigate mode):
```cpp
void CImageView::OnContextMenu(CPoint P) {
    CMenu menu;
    menu.CreatePopupMenu();
    
    if (selectionMgr.GetLevel() == SelectionLevel::Dot) {
        menu.AppendMenu(MF_STRING, ID_MENU_DELETE, "Delete Dot");
        menu.AppendMenu(MF_STRING, ID_MENU_INSERT, "Insert Dot");
        menu.AppendMenu(MF_STRING, ID_MENU_SELECT_SEGMENT, "Select Segment");
    }
    else if (selectionMgr.GetLevel() == SelectionLevel::Segment) {
        menu.AppendMenu(MF_STRING, ID_MENU_DELETE, "Delete Segment");
        menu.AppendMenu(MF_STRING, ID_MENU_SPLIT, "Split Segment");
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
TEST(EdgeCases, EmptySegmentCanBeDeleted) {
    // Create and delete empty segment
}

TEST(EdgeCases, SinglePointSegmentWorks) {
    // Create segment with 1 dot, all operations
}

TEST(EdgeCases, ConnectSelfDoesNotCrash) {
    // Try to connect segment to itself
}

TEST(EdgeCases, DeleteAllDotsRemovesSegment) {
    // Delete all dots in segment
}

TEST(EdgeCases, RenumberToSameNumberWorks) {
    // Renumber segment to its current number (no-op)
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
    // Load → modify → save → load → verify
}

TEST_F(RegressionTest, ComplexEditingWorkflow) {
    // Simulate real user: draw, edit, renumber, simplify, save
}

TEST_F(RegressionTest, LargeFilePerformance) {
    // 1000+ segments: draw, hit-test, select, command speed
}
```

#### 6.5 Visual Testing

**Manual Test Protocol**:
1. **Draw**: Click to draw segments, Shift for range, Ctrl to connect
2. **Edit**: Switch to Dot Edit, drag points, insert/delete
3. **Renumber**: Select fringe, press +/-, verify tooltip updates
4. **Simplify**: Select segment, press S, verify dots reduced
5. **Undo/Redo**: Ctrl+Z/Y through entire workflow
6. **Cursors**: Verify cursor changes with mode + modifiers
7. **Tooltips**: Hover over dots/segments, verify format

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
- [ ] Tooltips display correctly (std::string format)
- [ ] Context menus show correct options
- [ ] Can complete 10-minute user walkthrough (from UX spec) without issues

---


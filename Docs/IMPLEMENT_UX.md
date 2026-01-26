# Phase 2: Architecture Design for UX v1.0

**Status**: Complete architecture design for Digit/MFringe editor.  
**Date**: 2025-01-XX  
**Purpose**: Map UX v1.0 concepts ? Code architecture for implementation.  
**Audience**: Developers implementing interaction layer, command dispatch, and undo/redo.

---

## Overview

This document translates **UX v1.0 specification** into a working architecture that handles:
- Input handling (mode dispatch + modifier semantics)
- Selection management (persistent, hierarchical)
- Command execution (with undo/redo)
- Feedback (cursors, tooltips, state visualization)

**This is UX/architecture-level**, not a detailed implementation specification. Code details follow in Phase 3.

---

## Table of Contents

1. [A. Input Handler Architecture](#a-input-handler-architecture)
2. [B. Selection Management](#b-selection-management)
3. [C. Command Execution & Undo/Redo](#c-command-execution--undoredo)
4. [D. Cursor & Feedback](#d-cursor--feedback)
5. [E. Integration Point: ImageView](#e-integration-point-imageview)
6. [F. Data Flow Diagram](#f-data-flow-diagram)
7. [G. Class Diagram (Summary)](#g-class-diagram-summary)
8. [H. Key Design Constraints](#h-key-design-constraints)

---

## A. Input Handler Architecture

### A.1 Mode State Machine

Three modes, mutually exclusive, state-driven.

```
???????????????
?   NAVIGATE  ? (default)
???????????????
      ? (mode switch)
???????????????
?    DRAW     ? (curve creation)
???????????????
      ? (mode switch)
???????????????
?  DOT EDIT   ? (geometry only)
???????????????
```

**Key Rules**:
- Only one mode active at a time
- Mode switch finalizes pending operations (e.g., end current curve when leaving Draw)
- Selection is **locked during Draw** mode

**Implementation Pattern:**

```cpp
enum class EditMode { Navigate, Draw, DotEdit };

class InputHandler {
    EditMode currentMode = EditMode::Navigate;
    
    void SetMode(EditMode newMode) {
        if (currentMode == EditMode::Draw) {
            // Finalize any active curve
            EndCurrentCurve();
        }
        currentMode = newMode;
        UpdateCursor();
    }
    
    EditMode GetMode() const { return currentMode; }
    bool IsInDrawMode() const { return currentMode == EditMode::Draw; }
};
```

---

### A.2 Modifier Dispatch (Context-Aware)

Each combination of **mode + modifier + click target** produces a specific behavior.

**Design Principle**: Same modifier has same semantic meaning across all modes.

- **Ctrl** = Add / Extend / Connect
- **Shift** = Range / Constrain / Promote  
- **Alt** = Alternate / Destructive / Structural

**Decision Tree Logic:**

```cpp
class ModifierDispatcher {
    
    // Mode: DRAW, Event: LeftClick
    void OnLeftClick(CPoint P, ModifierState mods, InputHandler& handler) {
        if (mods.None()) {
            if (IsEmpty(P)) 
                StartNewCurve(P);
            else if (IsCurveEnd(P)) 
                ContinueCurve(P);
        }
        else if (mods.Ctrl()) {
            if (IsCurveEnd(P)) 
                ConnectCurves(P);  // ? Free end becomes active
        }
        else if (mods.Alt()) {
            if (IsDot(P)) 
                DeleteDot(P);
        }
    }
    
    // Mode: NAVIGATE, Event: LeftClick
    void OnLeftClickNavigate(CPoint P, ModifierState mods, SelectionManager& sel) {
        if (mods.None()) 
            sel.SelectSingle(P);
        else if (mods.Ctrl()) 
            sel.AddToSelection(P);
        else if (mods.Shift()) 
            sel.RangeSelect(P);
    }
    
    // Mode: DOT_EDIT, Event: LeftClick
    void OnLeftClickDotEdit(CPoint P, ModifierState mods) {
        if (mods.None()) {
            if (IsEdge(P)) InsertDotOnEdge(P);
            else if (IsDot(P)) SelectDot(P);
        }
        else if (mods.Alt()) {
            if (IsDot(P)) DeleteDot(P);
        }
    }
};
```

### A.3 Modifier State Struct

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
    
    CString Debug() const {
        CString s;
        if (ctrl) s += "Ctrl+";
        if (shift) s += "Shift+";
        if (alt) s += "Alt+";
        return s.IsEmpty() ? "None" : s;
    }
};
```

---

## B. Selection Management

### B.1 Hierarchical Selection State

**Selection is persistent**: survives mode switches.  
**Selection is hierarchical**: Dot ? Edge ? Curve ? Fringe.  
**Selection is explicit**: no implicit promotion.

```cpp
enum class SelectionLevel { None, Dot, Edge, Curve, Fringe };

class SelectionManager {
    struct SelectedObject {
        SelectionLevel level;
        int iFringe = -1;
        int iCurve = -1;
        int iDot = -1;
        int iEdge = -1;  // Edge index: pair (dot[i], dot[i+1])
        
        bool IsValid() const { return level != SelectionLevel::None; }
        bool IsSameLevel(const SelectedObject& other) const {
            return level == other.level;
        }
    };
    
    CArray<SelectedObject> selection;
    
public:
    // ===== Selection Primitives =====
    
    void SelectDot(int iFringe, int iCurve, int iDot) {
        selection.RemoveAll();
        SelectedObject obj;
        obj.level = SelectionLevel::Dot;
        obj.iFringe = iFringe;
        obj.iCurve = iCurve;
        obj.iDot = iDot;
        selection.Add(obj);
    }
    
    void SelectEdge(int iFringe, int iCurve, int iEdge) {
        selection.RemoveAll();
        SelectedObject obj;
        obj.level = SelectionLevel::Edge;
        obj.iFringe = iFringe;
        obj.iCurve = iCurve;
        obj.iEdge = iEdge;  // Edge between dot[iE] and dot[iE+1]
        selection.Add(obj);
    }
    
    void SelectCurve(int iFringe, int iCurve) {
        selection.RemoveAll();
        SelectedObject obj;
        obj.level = SelectionLevel::Curve;
        obj.iFringe = iFringe;
        obj.iCurve = iCurve;
        selection.Add(obj);
    }
    
    void SelectFringe(int iFringe) {
        selection.RemoveAll();
        SelectedObject obj;
        obj.level = SelectionLevel::Fringe;
        obj.iFringe = iFringe;
        selection.Add(obj);
    }
    
    // ===== Multi-Selection =====
    
    bool AddToSelection(const SelectedObject& obj) {
        // Only add if same level as existing selection
        if (selection.GetSize() > 0) {
            if (!selection[0].IsSameLevel(obj)) {
                return false;  // Incompatible level
            }
            // Avoid duplicates
            for (int i = 0; i < selection.GetSize(); i++) {
                if (IsSameObject(selection[i], obj)) {
                    selection.RemoveAt(i);  // Toggle
                    return true;
                }
            }
        }
        selection.Add(obj);
        return true;
    }
    
    // ===== Promotion =====
    
    void PromoteToFringe() {
        // Collect unique fringes from current selection
        // Replace selection with one entry per fringe
        CArray<int> fringeIndices;
        for (int i = 0; i < selection.GetSize(); i++) {
            int iF = selection[i].iFringe;
            if (!fringeIndices.Find(iF)) {
                fringeIndices.Add(iF);
            }
        }
        
        selection.RemoveAll();
        for (int i = 0; i < fringeIndices.GetSize(); i++) {
            SelectedObject obj;
            obj.level = SelectionLevel::Fringe;
            obj.iFringe = fringeIndices[i];
            selection.Add(obj);
        }
    }
    
    // ===== Queries =====
    
    SelectionLevel GetLevel() const {
        if (selection.GetSize() == 0) return SelectionLevel::None;
        return selection[0].level;
    }
    
    int GetCount() const { return selection.GetSize(); }
    
    const SelectedObject& GetAt(int i) const { return selection[i]; }
    
    void Clear() { selection.RemoveAll(); }
    
    bool IsEmpty() const { return selection.GetSize() == 0; }
};
```

### B.2 Hit Testing (Selection Query)

```cpp
class HitTester {
    static const int HIT_TOLERANCE = 5;  // pixels
    
    // ===== Main Entry Point =====
    
    SelectionLevel HitTest(CPoint P, int& outFringe, int& outCurve, int& outDot) {
        // Priority: Dot > Edge > Curve > Fringe > Nothing
        
        // Iterate fringes in reverse (top to bottom z-order)
        for (int iF = Fringes.GetSize() - 1; iF >= 0; iF--) {
            for (int iC = 0; iC < Fringes[iF].CurveCount(); iC++) {
                
                // 1. Check dots (highest priority)
                for (int iD = 0; iD < Fringes[iF].DotCount(iC); iD++) {
                    if (DotDistance(P, Fringes[iF].GetDot(iC, iD)) < HIT_TOLERANCE) {
                        outFringe = iF; outCurve = iC; outDot = iD;
                        return SelectionLevel::Dot;
                    }
                }
                
                // 2. Check edges
                for (int iE = 0; iE < Fringes[iF].DotCount(iC) - 1; iE++) {
                    double dist = DistanceToSegment(P, 
                        Fringes[iF].GetDot(iC, iE), 
                        Fringes[iF].GetDot(iC, iE + 1));
                    if (dist < HIT_TOLERANCE) {
                        outFringe = iF; outCurve = iC; outDot = iE;
                        return SelectionLevel::Edge;
                    }
                }
            }
        }
        
        return SelectionLevel::None;
    }
    
    // ===== Helper Methods =====
    
    double DotDistance(CPoint P, CDPoint dot) const {
        double dx = P.x - dot.x;
        double dy = P.y - dot.y;
        return sqrt(dx*dx + dy*dy);
    }
    
    double DistanceToSegment(CPoint P, CDPoint A, CDPoint B) const {
        double ABx = B.x - A.x;
        double ABy = B.y - A.y;
        double APx = P.x - A.x;
        double APy = P.y - A.y;
        
        double dotProduct = APx * ABx + APy * ABy;
        double lenSq = ABx * ABx + ABy * ABy;
        
        double t = (lenSq > 0) ? dotProduct / lenSq : 0;
        t = max(0.0, min(1.0, t));
        
        double closestX = A.x + t * ABx;
        double closestY = A.y + t * ABy;
        
        double dx = P.x - closestX;
        double dy = P.y - closestY;
        return sqrt(dx*dx + dy*dy);
    }
};
```

---

## C. Command Execution & Undo/Redo

### C.1 Command Base Class

```cpp
class Command {
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual void Redo() { Execute(); }  // Default: redo = execute
    virtual CString GetName() const { return "Command"; }  // For debugging
    virtual ~Command() {}
};
```

### C.2 Command Examples (UX v1.0 Frozen)

```cpp
// ===== Drawing Commands =====

class AddDotCommand : public Command {
    int iFringe, iCurve, iDot;
    CDPoint point;
    CDigitInfo* pDigit;
    
public:
    AddDotCommand(CDigitInfo* pD, int iF, int iC, int iDot_idx, CDPoint p)
        : pDigit(pD), iFringe(iF), iCurve(iC), iDot(iDot_idx), point(p) {}
    
    void Execute() override {
        pDigit->InsertDot(iFringe, iCurve, iDot, point);
    }
    
    void Undo() override {
        pDigit->RemoveDot(iFringe, iCurve, iDot);
    }
    
    CString GetName() const override { return "Add Dot"; }
};

class RemoveLastDotCommand : public Command {
    int iFringe, iCurve, iDot;
    CDPoint savedPoint;
    CDigitInfo* pDigit;
    
public:
    RemoveLastDotCommand(CDigitInfo* pD) : pDigit(pD) {
        // Save state before removal
        pD->GetLastDot(iFringe, iCurve, iDot, savedPoint);
    }
    
    void Execute() override {
        pDigit->RemoveDot(iFringe, iCurve, iDot);
    }
    
    void Undo() override {
        pDigit->InsertDot(iFringe, iCurve, iDot, savedPoint);
    }
    
    CString GetName() const override { return "Remove Dot"; }
};

// ===== Geometry Editing Commands =====

class MoveGeometryCommand : public Command {
    CArray<int> affectedFringes, affectedCurves, affectedDots;
    CArray<CDPoint> oldPositions;
    CArray<CDPoint> newPositions;
    CDigitInfo* pDigit;
    
public:
    MoveGeometryCommand(CDigitInfo* pD) : pDigit(pD) {}
    
    void AddPoint(int iF, int iC, int iD, CDPoint oldPos, CDPoint newPos) {
        affectedFringes.Add(iF);
        affectedCurves.Add(iC);
        affectedDots.Add(iD);
        oldPositions.Add(oldPos);
        newPositions.Add(newPos);
    }
    
    void Execute() override {
        for (int i = 0; i < affectedFringes.GetSize(); i++) {
            pDigit->SetDotPosition(affectedFringes[i], affectedCurves[i], 
                affectedDots[i], newPositions[i]);
        }
    }
    
    void Undo() override {
        for (int i = 0; i < affectedFringes.GetSize(); i++) {
            pDigit->SetDotPosition(affectedFringes[i], affectedCurves[i], 
                affectedDots[i], oldPositions[i]);
        }
    }
    
    CString GetName() const override { return "Move Geometry"; }
};

// ===== Selection-Based Commands =====

class RenumberCommand : public Command {
    CArray<int> affectedFringes;
    CArray<double> oldNumbers;
    double newNumber;
    CDigitInfo* pDigit;
    
public:
    RenumberCommand(CDigitInfo* pD, const CArray<int>& fringes, double newNum)
        : pDigit(pD), affectedFringes(fringes), newNumber(newNum) {
        // Save old numbers
        for (int i = 0; i < fringes.GetSize(); i++) {
            oldNumbers.Add(pD->GetFringe(fringes[i]).GetNumber());
        }
    }
    
    void Execute() override {
        for (int i = 0; i < affectedFringes.GetSize(); i++) {
            pDigit->SetFringeNumber(affectedFringes[i], newNumber);
        }
    }
    
    void Undo() override {
        for (int i = 0; i < affectedFringes.GetSize(); i++) {
            pDigit->SetFringeNumber(affectedFringes[i], oldNumbers[i]);
        }
    }
    
    CString GetName() const override { return "Renumber Fringe"; }
};

class SimplifyCommand : public Command {
    int iFringe, iCurve;
    CArray<CDPoint> originalPoints;
    double epsilon;
    CDigitInfo* pDigit;
    
public:
    SimplifyCommand(CDigitInfo* pD, int iF, int iC, double eps)
        : pDigit(pD), iFringe(iF), iCurve(iC), epsilon(eps) {
        // Save original points
        pD->GetCurvePoints(iF, iC, originalPoints);
    }
    
    void Execute() override {
        pDigit->SimplifyCurve(iFringe, iCurve, epsilon);
    }
    
    void Undo() override {
        pDigit->SetCurvePoints(iFringe, iCurve, originalPoints);
    }
    
    CString GetName() const override { return "Simplify"; }
};

class DeleteSelectionCommand : public Command {
    // Saves entire deleted objects for undo
    CArray<int> deletedFringes;  // Indices
    CArray<CFringe> savedFringes;
    CDigitInfo* pDigit;
    
public:
    DeleteSelectionCommand(CDigitInfo* pD, const SelectionManager& sel)
        : pDigit(pD) {
        // Collect fringes to delete from selection
        // ...
    }
    
    void Execute() override {
        for (int i = deletedFringes.GetSize() - 1; i >= 0; i--) {
            pDigit->DeleteFringe(deletedFringes[i]);
        }
    }
    
    void Undo() override {
        for (int i = 0; i < deletedFringes.GetSize(); i++) {
            pDigit->InsertFringe(deletedFringes[i], savedFringes[i]);
        }
    }
    
    CString GetName() const override { return "Delete"; }
};
```

### C.3 Command Dispatcher

```cpp
class CommandDispatcher {
    CArray<Command*> undoStack;
    CArray<Command*> redoStack;
    
public:
    ~CommandDispatcher() {
        // Clean up stacks
        for (int i = 0; i < undoStack.GetSize(); i++) delete undoStack[i];
        for (int i = 0; i < redoStack.GetSize(); i++) delete redoStack[i];
    }
    
    // ===== Core Operations =====
    
    void Execute(Command* cmd) {
        ASSERT(cmd != NULL);
        cmd->Execute();
        undoStack.Add(cmd);
        
        // Clear redo stack (new command breaks redo chain)
        for (int i = 0; i < redoStack.GetSize(); i++) delete redoStack[i];
        redoStack.RemoveAll();
        
        OnStateChanged();
    }
    
    void Undo() {
        if (undoStack.GetSize() == 0) return;
        
        Command* cmd = undoStack[undoStack.GetSize() - 1];
        cmd->Undo();
        undoStack.RemoveAt(undoStack.GetSize() - 1);
        redoStack.Add(cmd);
        
        OnStateChanged();
    }
    
    void Redo() {
        if (redoStack.GetSize() == 0) return;
        
        Command* cmd = redoStack[redoStack.GetSize() - 1];
        cmd->Redo();
        redoStack.RemoveAt(redoStack.GetSize() - 1);
        undoStack.Add(cmd);
        
        OnStateChanged();
    }
    
    // ===== State Queries =====
    
    bool CanUndo() const { return undoStack.GetSize() > 0; }
    bool CanRedo() const { return redoStack.GetSize() > 0; }
    
    CString GetUndoLabel() const {
        if (undoStack.GetSize() == 0) return "Undo";
        Command* cmd = undoStack[undoStack.GetSize() - 1];
        CString label;
        label.Format("Undo %s", cmd->GetName());
        return label;
    }
    
    CString GetRedoLabel() const {
        if (redoStack.GetSize() == 0) return "Redo";
        Command* cmd = redoStack[redoStack.GetSize() - 1];
        CString label;
        label.Format("Redo %s", cmd->GetName());
        return label;
    }
    
private:
    void OnStateChanged() {
        // Notify UI to update undo/redo buttons
        // (Call parent view's Invalidate, update menu, etc.)
    }
};
```

---

## D. Cursor & Feedback

### D.1 Cursor State Machine

**Base cursor** = Mode icon.  
**Overlay** = Modifier feedback.

```cpp
enum class CursorType {
    Arrow,        // Navigate mode
    Crosshair,    // Draw mode
    Vertex,       // Dot Edit mode
    Wait,         // Processing
};

enum class CursorOverlay {
    None,
    Plus,         // Ctrl (add/connect)
    Range,        // Shift (range/constrain)
    Bang,         // Alt (destructive)
};

class CursorManager {
    HCURSOR hCursorArrow, hCursorCrosshair, hCursorVertex, hCursorWait;
    
public:
    CursorManager() {
        // Load cursor resources
        hCursorArrow = ::LoadCursor(NULL, IDC_ARROW);
        hCursorCrosshair = ::LoadCursor(NULL, IDC_CROSS);
        // hCursorVertex = ::LoadCursor(hInstance, IDC_CUSTOM_VERTEX);
        hCursorWait = ::LoadCursor(NULL, IDC_WAIT);
    }
    
    void UpdateCursor(EditMode mode, ModifierState mods, SelectionLevel under) {
        CursorType base;
        CursorOverlay overlay = CursorOverlay::None;
        
        // 1. Base cursor by mode
        switch (mode) {
            case EditMode::Navigate:  base = CursorType::Arrow; break;
            case EditMode::Draw:      base = CursorType::Crosshair; break;
            case EditMode::DotEdit:   base = CursorType::Vertex; break;
        }
        
        // 2. Overlay by modifier
        if (mods.Ctrl())       overlay = CursorOverlay::Plus;
        else if (mods.Shift()) overlay = CursorOverlay::Range;
        else if (mods.Alt())   overlay = CursorOverlay::Bang;
        
        // 3. Compose and set cursor
        SetCursorImage(base, overlay);
    }
    
private:
    void SetCursorImage(CursorType base, CursorOverlay overlay) {
        // Composite cursor from base + overlay
        // For now, just set base; overlay could be implemented with 
        // custom cursor resources or by drawing overlay at click time
        
        HCURSOR hCursor;
        switch (base) {
            case CursorType::Arrow:      hCursor = hCursorArrow; break;
            case CursorType::Crosshair:  hCursor = hCursorCrosshair; break;
            case CursorType::Vertex:     hCursor = hCursorVertex; break;
            case CursorType::Wait:       hCursor = hCursorWait; break;
        }
        
        ::SetCursor(hCursor);
    }
};
```

### D.2 Tooltip Generator

```cpp
class TooltipGenerator {
    static const int MAX_TOOLTIP_LEN = 256;
    
    CString GetTooltip(const SelectionManager::SelectedObject& obj, 
                       const CDigitInfo& digit) {
        CString tip;
        
        switch (obj.level) {
            case SelectionLevel::Dot: {
                const CFringe& fringe = digit.GetFringe(obj.iFringe);
                int curveCount = fringe.CurveCount();
                int dotCount = fringe.DotCount(obj.iCurve);
                double number = fringe.GetNumber();
                
                tip.Format("#%.1f / %d(%d) / %d", 
                    number, obj.iCurve + 1, curveCount, obj.iDot + 1);
                break;
            }
            case SelectionLevel::Edge: {
                const CFringe& fringe = digit.GetFringe(obj.iFringe);
                double number = fringe.GetNumber();
                int dotCount = fringe.DotCount(obj.iCurve);
                
                tip.Format("#%.1f Edge (%d–%d of %d)", 
                    number, obj.iEdge, obj.iEdge + 1, dotCount - 1);
                break;
            }
            case SelectionLevel::Curve: {
                const CFringe& fringe = digit.GetFringe(obj.iFringe);
                int dotCount = fringe.DotCount(obj.iCurve);
                double number = fringe.GetNumber();
                
                tip.Format("Fringe #%.1f Curve — %d dots", number, dotCount);
                break;
            }
            case SelectionLevel::Fringe: {
                const CFringe& fringe = digit.GetFringe(obj.iFringe);
                int curveCount = fringe.CurveCount();
                double number = fringe.GetNumber();
                
                tip.Format("Fringe #%.1f (%d curves)", number, curveCount);
                break;
            }
            default:
                tip = "Unknown";
                break;
        }
        
        return tip;
    }
};
```

---

## E. Integration Point: ImageView

### E.1 Input Routing (Existing ImageView ? InputHandler)

This shows how the architecture components wire together in the existing `CImageView` class.

```cpp
// In ImageView.cpp
class CImageView : public CBaseImageView {
    
private:
    InputHandler inputHandler;
    SelectionManager selectionMgr;
    HitTester hitTester;
    CommandDispatcher cmdDispatcher;
    CursorManager cursorMgr;
    TooltipGenerator tooltipGen;
    
    // State
    EditMode currentMode = EditMode::Navigate;
    CPoint lastMousePos;
    
    // During-drag state (for move geometry)
    BOOL bDragging = FALSE;
    CPoint dragStart;
    MoveGeometryCommand* pMoveCmd = NULL;
    
public:
    
    // ===== Mode Switching =====
    
    void SetEditMode(EditMode newMode) {
        if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
            inputHandler.EndCurrentCurve();
        }
        currentMode = newMode;
        inputHandler.SetMode(newMode);
        Invalidate(FALSE);
    }
    
    // ===== Mouse Events =====
    
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point) {
        ModifierState mods = ModifierState::FromKeyboard();
        
        int iFringe, iCurve, iDot;
        SelectionLevel hitLevel = hitTester.HitTest(point, iFringe, iCurve, iDot);
        
        if (currentMode == EditMode::Navigate) {
            OnLButtonDownNavigate(point, mods, hitLevel, iFringe, iCurve, iDot);
        }
        else if (currentMode == EditMode::Draw) {
            OnLButtonDownDraw(point, mods, hitLevel, iFringe, iCurve, iDot);
        }
        else if (currentMode == EditMode::DotEdit) {
            OnLButtonDownDotEdit(point, mods, hitLevel, iFringe, iCurve, iDot);
        }
        
        Invalidate(FALSE);
    }
    
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point) {
        if (bDragging && pMoveCmd != NULL) {
            cmdDispatcher.Execute(pMoveCmd);
            pMoveCmd = NULL;
        }
        bDragging = FALSE;
        Invalidate(FALSE);
    }
    
    afx_msg void OnMouseMove(UINT nFlags, CPoint point) {
        ModifierState mods = ModifierState::FromKeyboard();
        
        // 1. Handle dragging (for geometry move)
        if (bDragging && pMoveCmd != NULL) {
            CPoint delta = point - dragStart;
            // Update move command with new positions
            // (Would need to refactor MoveGeometryCommand to support preview)
        }
        
        // 2. Update cursor feedback
        int iFringe, iCurve, iDot;
        SelectionLevel hitLevel = hitTester.HitTest(point, iFringe, iCurve, iDot);
        cursorMgr.UpdateCursor(currentMode, mods, hitLevel);
        
        // 3. Update tooltip
        if (hitLevel != SelectionLevel::None) {
            SelectionManager::SelectedObject obj;
            obj.level = hitLevel;
            obj.iFringe = iFringe;
            obj.iCurve = iCurve;
            obj.iDot = iDot;
            SetToolTip(tooltipGen.GetTooltip(obj, digitInfo));
        }
        else {
            SetToolTip("");
        }
        
        lastMousePos = point;
        Invalidate(FALSE);
    }
    
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point) {
        if (currentMode == EditMode::Draw) {
            // End current curve
            inputHandler.EndCurrentCurve();
        }
        else {
            // Show context menu
            OnContextMenu(point);
        }
        Invalidate(FALSE);
    }
    
    // ===== Keyboard Events =====
    
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
        switch (nChar) {
            case VK_DELETE:
                OnKeyDelete();
                break;
            case VK_OEM_PLUS:
            case '+':
                OnKeyNumberIncrement();
                break;
            case VK_OEM_MINUS:
            case '-':
                OnKeyNumberDecrement();
                break;
            case 'S':
                OnKeySimplify();
                break;
            case VK_BACK:
                if (currentMode == EditMode::Draw) {
                    OnKeyRemoveLastDot();
                }
                break;
            case VK_ESCAPE:
                if (currentMode == EditMode::Draw) {
                    inputHandler.EndCurrentCurve();
                }
                selectionMgr.Clear();
                Invalidate(FALSE);
                break;
            case VK_TAB:
                OnKeyCycleSelection();
                break;
        }
    }
    
    // ===== Command Handlers (from Keyboard / Menu) =====
    
private:
    
    void OnKeyDelete() {
        // Create and execute delete command
        DeleteSelectionCommand* pCmd = new DeleteSelectionCommand(&digitInfo, selectionMgr);
        cmdDispatcher.Execute(pCmd);
        selectionMgr.Clear();
    }
    
    void OnKeyNumberIncrement() {
        if (selectionMgr.GetLevel() == SelectionLevel::Fringe ||
            selectionMgr.GetLevel() == SelectionLevel::Curve) {
            
            // Collect affected fringes
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
    
    void OnKeyNumberDecrement() {
        if (selectionMgr.GetLevel() == SelectionLevel::Fringe ||
            selectionMgr.GetLevel() == SelectionLevel::Curve) {
            
            CArray<int> fringes;
            for (int i = 0; i < selectionMgr.GetCount(); i++) {
                fringes.Add(selectionMgr.GetAt(i).iFringe);
            }
            
            double currentNum = digitInfo.GetFringe(fringes[0]).GetNumber();
            double step = digitInfo.GetNumberStep();
            
            RenumberCommand* pCmd = new RenumberCommand(&digitInfo, fringes, currentNum - step);
            cmdDispatcher.Execute(pCmd);
        }
    }
    
    void OnKeySimplify() {
        if (selectionMgr.GetLevel() == SelectionLevel::Curve) {
            for (int i = 0; i < selectionMgr.GetCount(); i++) {
                const auto& obj = selectionMgr.GetAt(i);
                SimplifyCommand* pCmd = new SimplifyCommand(&digitInfo, obj.iFringe, obj.iCurve, 1.0);
                cmdDispatcher.Execute(pCmd);
            }
        }
    }
    
    void OnKeyRemoveLastDot() {
        RemoveLastDotCommand* pCmd = new RemoveLastDotCommand(&digitInfo);
        cmdDispatcher.Execute(pCmd);
    }
    
    void OnKeyCycleSelection() {
        // Navigate to next object of same level, or cycle level
        // ...
    }
    
    // ===== Mode-Specific Handlers =====
    
    void OnLButtonDownNavigate(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                               int iFringe, int iCurve, int iDot) {
        SelectionManager::SelectedObject obj;
        
        if (hitLevel == SelectionLevel::None) {
            // Click on empty space
            if (mods.None()) {
                selectionMgr.Clear();
            }
        }
        else {
            // Click on object
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
            else if (mods.Shift()) {
                // Range select within same fringe/curve
                // ...
            }
            else if (mods.Alt()) {
                selectionMgr.SelectFringe(iFringe);
            }
        }
    }
    
    void OnLButtonDownDraw(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                           int iFringe, int iCurve, int iDot) {
        if (mods.None()) {
            if (hitLevel == SelectionLevel::None) {
                // Start new curve
                inputHandler.StartNewCurve(P);
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
        else if (mods.Alt() && hitLevel == SelectionLevel::Dot) {
            // Delete dot
            DeleteSelectionCommand* pCmd = new DeleteSelectionCommand(&digitInfo, selectionMgr);
            cmdDispatcher.Execute(pCmd);
        }
    }
    
    void OnLButtonDownDotEdit(CPoint P, ModifierState mods, SelectionLevel hitLevel,
                              int iFringe, int iCurve, int iDot) {
        if (hitLevel == SelectionLevel::Dot) {
            if (mods.Alt()) {
                // Delete dot
                // ...
            }
            else {
                // Start drag to move
                bDragging = TRUE;
                dragStart = P;
                pMoveCmd = new MoveGeometryCommand(&digitInfo);
                pMoveCmd->AddPoint(iFringe, iCurve, iDot, digitInfo.GetDotPosition(iFringe, iCurve, iDot), P);
            }
        }
        else if (hitLevel == SelectionLevel::Edge) {
            // Insert dot on edge
            // ...
        }
    }
};
```

---

## F. Data Flow Diagram

```
????????????????????????????????????????????????????????????
? User: Mouse/Keyboard Event                               ?
????????????????????????????????????????????????????????????
              ?
              v
     ??????????????????????
     ? ModifierState      ?
     ? (Ctrl/Shift/Alt)   ?
     ??????????????????????
              ?
              v
     ??????????????????????????????????
     ? InputHandler (Mode-Dispatch)   ?
     ?  - Determine intent            ?
     ?  - Route to correct handler    ?
     ??????????????????????????????????
              ?
       ???????????????
       ?             ?
       v             v
  ???????????  ????????????????
  ? HitTest ?  ? SelectionMgr  ?
  ? (Loc)   ?  ? (Hierarchy)   ?
  ???????????  ????????????????
       ?               ?
       ?????????????????
               ?
               v
       ????????????????????
       ? Command Factory  ?
       ? (Create command) ?
       ????????????????????
                ?
                v
       ????????????????????????
       ? CommandDispatcher    ?
       ? Execute ? Undo/Redo  ?
       ????????????????????????
                ?
                v
       ????????????????????????
       ? CDigitInfo           ?
       ? (Data model)         ?
       ????????????????????????
                ?
                v
       ????????????????????????
       ? Feedback:            ?
       ? - CursorManager      ?
       ? - TooltipGenerator   ?
       ? - Invalidate/Render  ?
       ????????????????????????
```

---

## G. Class Diagram (Summary)

```
InputHandler
??? SetMode(EditMode)
??? OnMouseDown(point, mods)
??? OnMouseMove(point, mods)
??? StartNewCurve(P)
??? ContinueCurve(iFringe, iCurve, iDot)
??? ConnectCurves(iFringe, iCurve, iDot)
??? EndCurrentCurve()
??? IsInDrawMode()

SelectionManager
??? SelectDot(iF, iC, iD)
??? SelectCurve(iF, iC)
??? SelectFringe(iF)
??? AddToSelection(obj)
??? PromoteToFringe()
??? GetLevel()
??? GetCount()
??? GetAt(i)
??? Clear()

HitTester
??? HitTest(P) ? SelectionLevel
??? DotDistance(P, dot)
??? DistanceToSegment(P, A, B)

CommandDispatcher
??? Execute(Command*)
??? Undo()
??? Redo()
??? CanUndo()
??? CanRedo()
??? GetUndoLabel()
??? GetRedoLabel()

Command (abstract)
??? AddDotCommand
??? RemoveLastDotCommand
??? MoveGeometryCommand
??? RenumberCommand
??? SimplifyCommand
??? DeleteSelectionCommand
??? ...

CursorManager
??? UpdateCursor(mode, mods, under)

TooltipGenerator
??? GetTooltip(obj, digit)
```

---

## H. Key Design Constraints

This architecture strictly follows UX v1.0 requirements:

### ? UX v1.0 Aligned

- **Mode dispatch is contextual**: Same Ctrl+Click = different action in Draw vs Navigate
- **Selection is persistent**: Survives mode switches; cleared explicitly
- **Commands are atomic**: One user intent = one undo step
- **Modifiers are globally consistent**: Ctrl = Add/Connect everywhere, Shift = Range/Constrain everywhere, Alt = Structural/Destructive everywhere
- **Hit testing obeys selection rules**: Edge if intersects or inside, Curve only if all edges included (unless modifier), Fringe only via Alt modifier

### ? No Topology Reconstruction

- Fringes are already ordered (CFringe polyline)
- No flat-dot logic
- Hit testing works directly on curve geometry
- Commands directly manipulate fringe data without rebuilding indices

### ? Scalable & Extensible

- **Command pattern**: Easy to add new commands (Split, Merge, Auto-number, Subdivide) by subclassing `Command`
- **Modifier dispatch**: Can extend with new modifiers without breaking existing logic
- **Selection manager**: Agnostic to data model details; works with any (iFringe, iCurve, iDot) tuple
- **Undo/Redo**: Framework supports arbitrary command types automatically

### ? Clear Separation of Concerns

- **InputHandler**: Pure mode/modifier logic (no data model knowledge)
- **SelectionManager**: Selection state machine (agnostic to modes or commands)
- **HitTester**: Geometry queries (no side effects)
- **CommandDispatcher**: History management (generic, reusable)
- **CursorManager, TooltipGenerator**: Pure feedback generation

---

## Next Steps

This architecture is **ready for implementation**:

1. **Phase 3 (Implementation Roadmap)**: Detailed code structure, file organization, and step-by-step development order
2. **Phase 1 (Test Validation)**: Unit tests for each component (InputHandler, SelectionManager, HitTester, CommandDispatcher)
3. **Prototyping**: Start with Draw mode (simplest workflow), then Navigate, then Dot Edit

---

**End of Phase 2: Architecture Design**

---

## Appendix: UX v1.0 Checklist for Architecture

Use this to verify architecture completeness before implementation:

### Selection Model
- [x] Hierarchical levels: Dot, Edge, Curve, Fringe
- [x] Persistent across mode switches
- [x] Explicit promotion (no implicit)
- [x] Ctrl adds, Shift ranges, Alt promotes
- [x] Box selection rules formalized

### Draw Mode
- [x] Start new curve on empty click
- [x] Continue from curve ends
- [x] Connect with Ctrl+Click; free end becomes active
- [x] Right-click ends curve
- [x] Backspace removes last dot
- [x] Alt+Click removes arbitrary dot
- [x] Selection locked during draw

### Navigate Mode
- [x] Full selection control
- [x] Box select with inclusion rules
- [x] Drag selection to move
- [x] Delete selection
- [x] Selection promotion via modifiers

### Dot Edit Mode
- [x] Move dots with drag
- [x] Insert dot on edge click
- [x] Delete dot with Alt+Click
- [x] Drag edge to move its dots

### Commands
- [x] Number +/?
- [x] Simplify
- [x] Split curve
- [x] Merge curves
- [x] All undoable (Command pattern)

### Feedback
- [x] Cursor reflects mode + modifiers
- [x] Tooltips follow spec (#N / C(T) / D format)
- [x] Invalid actions disabled (command availability check)

---

**Architecture Design Complete. Ready for Phase 3: Implementation Roadmap.**

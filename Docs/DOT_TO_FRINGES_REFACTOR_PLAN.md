Fringe Architecture Refactoring Plan   
Project: Digit - Interferometry Analysis Application   
Document Version: 1.1   
Date: 2026-01-21   
Author: Architecture Review Team   
Status: In Progress (Phase 1-2 complete)   

---
Executive Summary   
This document outlines a comprehensive plan to refactor the CDigitInfo class from a flat array-based architecture (`CArray<CDotInfo> Dots`) to a topology-aware fringe-based polyline model. The current architecture fundamentally misrepresents the domain model where fringes are continuous curves, not collections of independent points.

Key Findings:
* Current flat array loses topological information (point ordering, fringe continuity)
* Many operations require expensive O(N) scans and sorting
* File I/O does repeated reconstruction of structure already present in file format
* Critical operations (insert point between existing points, split/merge fringes) are impossible
Progress: Phases 1-2 delivered (`CFringe` class, file I/O via fringe model) with dual-model compatibility. Remaining phases focus on drawing, editing, advanced ops, and cleanup.
---

## Table of Contents
1.	-problem-analysis
2.	-proposed-architecture
3.	-migration-strategy
4.	-detailed-phase-breakdown
5.	-benefits-analysis
6.	-risk-assessment--mitigation
7.	-testing-strategy
8.	-success-criteria
9.	-appendix

---
### 1. Problem Analysis  
#### 1.1 Semantic Mismatch   

Physical Reality:
```
Fringe = continuous curve (ordered polyline)
         with spatial coherence and topological structure
```
Current Model: 
 ```cpp
 CArray<CDotInfo> Dots;  // Flat array of independent points
struct CDotInfo {
    CDPoint P;         // Position
    double Number;     // Fringe identifier (used for grouping)
    int iZapSec;       // Section index
};
```
Problem: Topology (point ordering, curve continuity) must be reconstructed from metadata on every operation.

---
1.2 Operation Complexity Examples
Drawing a Fringe (Current)


```cpp
// Drawing requires expensive reconstruction
void CDigitInfo::Draw(CDC* pDC, int DotSide) {
    if (pCtrls->ViewState & V_DOTLINES) {
        CList<double, double> Numbers;
        CArray<CDPoint> adP;
        
        GetDotNumbers(Numbers);  // Scan entire Dots array
        
        for each Number {
            GetFringeDots(Number, adP);  // Scan entire Dots array again
            // adP is unordered! Points may be out of sequence
            // (Drawing may connect wrong points)
            
            for (int ii = 0; ii < adP.GetSize(); ii++) {
                if (ii == 0) pDC->MoveTo(wP);
                else pDC->LineTo(wP);  // May zig-zag if unordered!
            }
        }
    }
}
```
With Fringes:

```cpp
void CDigitInfo::Draw(CDC* pDC, int DotSide) {
    if (pCtrls->ViewState & V_DOTLINES) {
        for (int iF = 0; iF < Fringes.GetSize(); iF++) {
            Fringes[iF].DrawPolyline(pDC);  // Points already ordered
        }
    }
}

```
---
// Navigation: "Next Point in Fringe" (Current)

```cpp
bool CDigitInfo::GetNextDotInSection(int iZapSec, int direct, int& idx, CDPoint& dP) {
    int minidx = -1;
    double minx = INT_MAX;
    double dif;
    
    // O(N) scan to find next point
    for (int iD = 0; iD < Dots.GetSize(); iD++) {
        if (Dots[iD].iZapSec == iZapSec) {
            dif = 0.;
            if (direct > 0 && Dots[iD].P.x > dP.x)
                dif = Dots[iD].P.x - dP.x;
            else if (direct < 0 && dP.x > Dots[iD].P.x)
                dif = dP.x - Dots[iD].P.x;
            if (dif == 0.) continue;
            if (dif < minx) {
                minx = dif;
                minidx = iD;
            }
        }
    }
    // ...
}
```
With Fringes:
```cpp
bool CFringePolyline::GetNextPoint(int currentIdx, int direction, int& nextIdx) {
    nextIdx = currentIdx + direction;
    if (nextIdx >= 0 && nextIdx < m_Points.GetSize()) {
        return true;
    }
    return false;
}
```
---



1.3 Impossible Operations

| Operation |	Current Status |	Impact |   
|---|---|---|
| Insert point between two existing points	| ❌ Impossible	| Cannot refine fringe interactively |   
| Split fringe at discontinuity	| ❌ Impossible	| Cannot handle broken fringes |   
| Merge two fringe segments	| ❌ Impossible	| Manual workarounds only |   
| Detect fringe gaps | ❌ Unreliable| Must scan and compare distances |   
| Reorder points in fringe | ❌ No structure | No concept of order exists |   

---
1.4 File I/O Impedance Mismatch
ZAP/FRN File Format (from ReadWriteData.cpp):

```
[FRINGES]
 <Y> <Number> <X1> <Number> <X2> <Number> <X3> ...  E
 <Y> <Number> <X1> <Number> <X2> ...  E
 ````
 ↑ Already organized as polylines (ordered points per line)
Current Load Code (WriteFRNData):

```cpp
// Expensive grouping and sorting to reconstruct polylines
for (i = in; i < NPnt; i++) {
    if (fabs(FCur - F) < EpsF) {
        Sampl.Add(X, Y, F);  // Collect all points with same Number
    }
}
Bln = XYBrokenLine(Sampl.XPnt, Sampl.YPnt);
Bln.Arrange();  // EXPENSIVE SORT to order points
WriteFringe(Fl, Sampl);
```
Problem: File already has structure → flatten to Dots → reconstruct structure on save.

---
2. Proposed Architecture
2.1 Core Data Structures

```cpp
/// <summary>
/// Represents a single fringe as an ordered polyline of points.
/// A fringe is a continuous curve with a constant fringe number.
/// </summary>
class CFringe {
private:
    double m_Number;      // Fringe number (e.g., 0, 0.5, 1.0, ...)
    int    m_Index;       // Segment index for discontinuous parts
    CArray<CDPoint> m_Points;
    BOOL   m_bClosed;
public:
    CFringe(double number = 0.0, int index = -1);
    // point management, queries, hit-testing, drawing, advanced ops
    int AddPoint(CDPoint p);
    void InsertPoint(int idx, CDPoint p);
    void RemovePoint(int idx);
    void MovePoint(int idx, CDPoint p);
    void AppendPoints(const CFringe& other);
    int GetPointCount() const;
    CDPoint GetPoint(int idx) const;
    void SetPoint(int idx, CDPoint p);
    int GetIndex() const;    void SetIndex(int index);
    double GetNumber() const;void SetNumber(double n);
    BOOL IsClosed() const;   void SetClosed(BOOL c);
    int FindNearestPoint(CPoint screenP, int tolerance);
    BOOL IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx);
    CRect GetBoundingRect() const;
    void DrawDots(CDC*, int dotSize, COLORREF color);
    void DrawPolyline(CDC*, COLORREF color);
    void DrawFull(CDC*, int dotSize, COLORREF lineColor, COLORREF dotColor);
    CFringe Split(int atIndex);
    double GetArcLength() const;
    void SubdivideSegments(double maxGap);
    void Simplify(double epsilon);
};
```
---
2.2 Updated CDigitInfo
```cpp
class CDigitInfo {
public:
    // Primary Storage (transition)
    CArray<CFringe> Fringes;
    CArray<CDotInfo> Dots;      // kept during transition
    BOOL m_bUseFringeModel;     // transition flag

    // Fringe operations
    int  CreateFringe(double number, int segment = -1);
    void DeleteFringe(int iFringe);
    CFringe* GetFringe(int i); const CFringe* GetFringe(int i) const;
    void FindFringesByNumber(double number, CArray<int>& indices);
    void AddPointToFringe(int iFringe, CDPoint p);
    void InsertPointInFringe(int iFringe, int iPoint, CDPoint p);
    void RemovePointFromFringe(int iFringe, int iPoint);
    void MovePointInFringe(int iFringe, int iPoint, CDPoint newP);
    BOOL FindPointUnderCursor(CPoint P, int tol, int& outFringe, int& outPoint);
    BOOL FindFringeUnderCursor(CPoint P, int tol, int& outFringe);
    void RenumberFringes(double oldNumber, double newNumber);
    void DeleteFringesByNumber(double number);
    BOOL MergeFringes(int iFringe1, int iFringe2);

    // Conversion (transition only)
    void ConvertDotsToFringes();
    void ConvertFringesToDots();
    void SyncFringesToDots();
};
```
---
3. Migration Strategy   

3.1 Guiding Principles
   1.	No Big Bang: Incremental migration with continuous testing
   2.	Parallel Storage: Maintain both models during transition
   3.	Backward Compatibility: Old files must load correctly
   4.	Feature Parity: New model matches all old features before switchover
   5.	Safety Nets: Extensive unit tests, visual regression tests     
---
3.2 Phase Overview

|Phase | Duration | Goal | Deliverable | Status |
|---|---|---|---|---|
|1. Foundation | 2 weeks | Create CFringe class + parallel storage | Tested class, no behavior change | **Done** |
|2. File I/O | 1 week | Load/save using fringes; dual model | Round-trip compatibility, tests | **Done** |
|3. Drawing | 1 week | Render from fringes | Visual parity verified | Pending |
|4. Editing | 2 weeks | All operations use fringes | Full feature parity | Pending |
|5. Advanced | 2 weeks | New capabilities (split/merge, simplify) | Added ops | Pending |
|6. Cleanup | 1 week | Remove legacy Dots path | Clean codebase | Pending |

---
4. Detailed Phase Breakdown

Phase 1: Foundation (Completed)
- Delivered: `CFringe` class with segment index, CDigitInfo parallel storage, selection model (`SelectedPoint`), transition flag, unit tests for CFringe.

Phase 2: File I/O (Completed)
- Delivered: Examine/Collect using fringe model (with segment index), LoadZAP/LoadFRN/SaveZAP/SaveFRN wired to fringe path, round-trip GoogleTests (CFringeFileIOTest).
- Dual-model compatibility maintained; Dots still supported during transition.

Phase 3: Drawing (Planned)
- Render from `Fringes`; maintain Dots rendering for compatibility until parity proven.

Phase 4: Editing (Planned)
- Migrate interactive operations (add/remove/move/lock) to Fringes and SelectedPoint.

Phase 5: Advanced (Planned)
- Enable split/merge, simplify, resample; expose arc-length, bounding boxes.

Phase 6: Cleanup (Planned)
- Remove legacy Dots path once parity and regression tests pass.

Success Criteria (updated)
- Round-trip I/O passes (fringe + dots) — **ACHIEVED**
- CFringe unit tests pass — **ACHIEVED**
- Drawing parity (to be validated) — Pending
- Editing parity — Pending
- Legacy removal after parity — Pending

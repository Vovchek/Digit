Fringe Architecture Refactoring Plan   
Project: Digit - Interferometry Analysis Application   
Document Version: 1.0   
Date: 2026-01-21   
Author: Architecture Review Team   
Status: Proposal - Awaiting Approval   

---
Executive Summary   
This document outlines a comprehensive plan to refactor the CDigitInfo class from a flat array-based architecture (`CArray<CDotInfo> Dots`) to a topology-aware fringe-based polyline model. The current architecture fundamentally misrepresents the domain model where fringes are continuous curves, not collections of independent points.

Key Findings:
* Current flat array loses topological information (point ordering, fringe continuity)
* Many operations require expensive O(N) scans and sorting
* File I/O does repeated reconstruction of structure already present in file format
* Critical operations (insert point between existing points, split/merge fringes) are impossible
Recommendation: Proceed with phased migration to `CArray<CFringePolyline> Fringes` architecture.
Estimated Timeline: 9 weeks (2 months)
Risk Level: Medium (mitigated by parallel storage during transition)
Impact: High positive impact on code clarity, performance, and feature extensibility
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
Navigation: "Next Point in Fringe" (Current)

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
 ```
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
class CFringePolyline {
private:
    double m_Number;              // Fringe number (e.g., 0, 0.5, 1.0, ...)
    CArray<CDPoint> m_Points;     // ORDERED sequence of points
    BOOL m_bClosed;               // Is this a closed loop?
    
public:
    // ===== Construction =====
    CFringePolyline(double number = 0.0);
    CFringePolyline(const CFringePolyline& other);
    CFringePolyline& operator=(const CFringePolyline& other);
    
    // ===== Point Management =====
    /// Add point to end of polyline
    int AddPoint(CDPoint p);
    
    /// Insert point at specific index (shifts subsequent points)
    void InsertPoint(int idx, CDPoint p);
    
    /// Remove point at index
    void RemovePoint(int idx);
    
    /// Move existing point to new position
    void MovePoint(int idx, CDPoint newP);
    
    /// Append all points from another fringe
    void AppendPoints(const CFringePolyline& other);
    
    // ===== Queries =====
    int GetPointCount() const { return m_Points.GetSize(); }
    CDPoint GetPoint(int idx) const;
    void SetPoint(int idx, CDPoint p);
    
    double GetNumber() const { return m_Number; }
    void SetNumber(double n) { m_Number = n; }
    
    BOOL IsClosed() const { return m_bClosed; }
    void SetClosed(BOOL closed) { m_bClosed = closed; }
    
    // ===== Hit Testing =====
    /// Find nearest point to screen coordinate (within tolerance)
    int FindNearestPoint(CPoint screenP, int tolerance);
    
    /// Check if screen coordinate is on the polyline
    BOOL IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx);
    
    /// Get bounding rectangle
    CRect GetBoundingRect() const;
    
    // ===== Drawing =====
    /// Draw only the dots (markers)
    void DrawDots(CDC* pDC, int dotSize, COLORREF color);
    
    /// Draw only the connecting polyline
    void DrawPolyline(CDC* pDC, COLORREF color);
    
    /// Draw both dots and connecting lines
    void DrawFull(CDC* pDC, int dotSize, COLORREF lineColor, COLORREF dotColor);
    
    // ===== Advanced Operations =====
    /// Split this fringe at given point index (returns new fringe)
    CFringePolyline Split(int atIndex);
    
    /// Compute total arc length
    double GetArcLength() const;
    
    /// Subdivide long segments to max spacing
    void SubdivideSegments(double maxGap);
    
    /// Simplify polyline (Douglas-Peucker)
    void Simplify(double epsilon);
};
```
---
2.2 Updated CDigitInfo
```cpp
class CDigitInfo {
public:
    // ===== NEW: Primary Storage =====
    CArray<CFringePolyline> Fringes;
    
    // ===== OLD: Deprecated (remove in Phase 6) =====
    CArray<CDotInfo> Dots;  // Keep during transition
    BOOL m_bUseFringeModel; // Transition flag
    
    // ===== Fringe-Level Operations =====
    /// Create new fringe and return its index
    int CreateFringe(double number);
    
    /// Delete fringe by index
    void DeleteFringe(int iFringe);
    
    /// Get fringe by index
    CFringePolyline* GetFringe(int i);
    const CFringePolyline* GetFringe(int i) const;
    
    /// Find all fringes with given number
    void FindFringesByNumber(double number, CArray<int>& indices);
    
    // ===== Point-Level Operations Within Fringes =====
    /// Add point to end of fringe
    void AddPointToFringe(int iFringe, CDPoint p);
    
    /// Insert point at specific position in fringe
    void InsertPointInFringe(int iFringe, int iPoint, CDPoint p);
    
    /// Remove point from fringe
    void RemovePointFromFringe(int iFringe, int iPoint);
    
    /// Move point within fringe
    void MovePointInFringe(int iFringe, int iPoint, CDPoint newP);
    
    // ===== Selection/Hit Testing =====
    /// Find point under cursor (returns fringe and point indices)
    BOOL FindPointUnderCursor(CPoint P, int tolerance, 
                              int& outFringe, int& outPoint);
    
    /// Find fringe under cursor
    BOOL FindFringeUnderCursor(CPoint P, int tolerance, int& outFringe);
    
    // ===== Bulk Operations =====
    /// Renumber all fringes with oldNumber to newNumber
    void RenumberFringes(double oldNumber, double newNumber);
    
    /// Delete all fringes with given number
    void DeleteFringesByNumber(double number);
    
    /// Merge two fringes (must have same number)
    BOOL MergeFringes(int iFringe1, int iFringe2);
    
    // ===== Conversion (Transition Only) =====
    void ConvertDotsToFringes();
    void ConvertFringesToDots();
    void SyncFringesToDots();
    
    // ===== Legacy Wrappers (Phase 4-5, remove in Phase 6) =====
    void AddDot(CPoint P, int dotSide);        // → AddPointToFringe
    void RemoveDot(CPoint P, int dotSide);     // → RemovePointFromFringe
    BOOL LockDot(CPoint P, int dotSide, BOOL Enable);  // → LockPoint
    // ... etc
};
```
---
2.3 Selection Model
Old:
```cpp
int idxDragDot;   // Single index into flat Dots array
int idxMainDot;
```
New:
```cpp
struct SelectedPoint {
    int iFringe;
    int iPoint;
    
    SelectedPoint() : iFringe(-1), iPoint(-1) {}
    SelectedPoint(int f, int p) : iFringe(f), iPoint(p) {}
    
    BOOL IsValid() const { return iFringe >= 0 && iPoint >= 0; }
    void Clear() { iFringe = iPoint = -1; }
};

SelectedPoint idxDraggedPoint;  // Replaces idxDragDot
SelectedPoint idxMainPoint;     // Replaces idxMainDot
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

|Phase	|Duration	|Goal	|Deliverable|
|---|---|---|---|
|1. Foundation	|2 weeks	|Create CFringePolyline class	|Tested class, no behavior change|
|2. File I/O	|1 week	|Load/save using fringes	|Round-trip file compatibility|
|3. Drawing	|1 week	|Render from fringes	|Visual parity verified|
|4. Editing	|2 weeks	|All operations use fringes	|Full feature parity|
|5. Advanced	|2 weeks	|New capabilities	|Split/merge/interpolate|
|6. Cleanup	|1 week	|Remove legacy code	|Clean codebase|

---
4. Detailed Phase Breakdown

Phase 1: Foundation (Weeks 1-2)   
1.1 Create CFringePolyline Class

Files:
*	FringePolyline.h (NEW)
*	DigitMode/FringePolyline.cpp (NEW)

Implementation:
```cpp
// FringePolyline.h
#pragma once
#include "MGTools/Include/Utils/BaseDataType.h"

class CFringePolyline {
private:
    double m_Number;
    CArray<CDPoint> m_Points;
    BOOL m_bClosed;
    
public:
    CFringePolyline(double number = 0.0, int zapSec = -1)
        : m_Number(number), m_bClosed(FALSE) {}
    
    // Point management
    int AddPoint(CDPoint p) {
        m_Points.Add(p);
        return m_Points.GetSize() - 1;
    }
    
    void InsertPoint(int idx, CDPoint p) {
        if (idx < 0 || idx > m_Points.GetSize()) return;
        m_Points.InsertAt(idx, p);
    }
    
    void RemovePoint(int idx) {
        if (idx < 0 || idx >= m_Points.GetSize()) return;
        m_Points.RemoveAt(idx);
    }
    
    void MovePoint(int idx, CDPoint newP) {
        if (idx < 0 || idx >= m_Points.GetSize()) return;
        m_Points[idx] = newP;
    }
    
    // Queries
    int GetPointCount() const { return m_Points.GetSize(); }
    CDPoint GetPoint(int idx) const { return m_Points[idx]; }
    void SetPoint(int idx, CDPoint p) { m_Points[idx] = p; }
    
    double GetNumber() const { return m_Number; }
    void SetNumber(double n) { m_Number = n; }
    
    // Hit testing
    int FindNearestPoint(CPoint screenP, int tolerance);
    BOOL IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx);
    
    // Drawing
    void DrawDots(CDC* pDC, int dotSize, COLORREF color);
    void DrawPolyline(CDC* pDC, COLORREF color);
    void DrawFull(CDC* pDC, int dotSize, COLORREF lineColor, COLORREF dotColor);
};
```
FringePolyline.cpp:
```cpp
#include "stdafx.h"
#include "FringePolyline.h"

int CFringePolyline::FindNearestPoint(CPoint screenP, int tolerance) {
    int nearestIdx = -1;
    double minDist = DBL_MAX;
    
    for (int i = 0; i < m_Points.GetSize(); i++) {
        double dx = m_Points[i].x - screenP.x;
        double dy = m_Points[i].y - screenP.y;
        double dist = sqrt(dx*dx + dy*dy);
        
        if (dist < tolerance && dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }
    
    return nearestIdx;
}

BOOL CFringePolyline::IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx) {
    nearestIdx = FindNearestPoint(P, tolerance);
    return (nearestIdx >= 0);
}

void CFringePolyline::DrawDots(CDC* pDC, int dotSize, COLORREF color) {
    int half = dotSize / 2;
    CBrush brush(color);
    CBrush* oldBrush = pDC->SelectObject(&brush);
    
    for (int i = 0; i < m_Points.GetSize(); i++) {
        CPoint p((int)m_Points[i].x, (int)m_Points[i].y);
        pDC->Ellipse(p.x - half, p.y - half, p.x + half, p.y + half);
    }
    
    pDC->SelectObject(oldBrush);
}

void CFringePolyline::DrawPolyline(CDC* pDC, COLORREF color) {
    if (m_Points.GetSize() < 2) return;
    
    CPen pen(PS_SOLID, 1, color);
    CPen* oldPen = pDC->SelectObject(&pen);
    
    CPoint p0((int)m_Points[0].x, (int)m_Points[0].y);
    pDC->MoveTo(p0);
    
    for (int i = 1; i < m_Points.GetSize(); i++) {
        CPoint p((int)m_Points[i].x, (int)m_Points[i].y);
        pDC->LineTo(p);
    }
    
    if (m_bClosed && m_Points.GetSize() > 2) {
        pDC->LineTo(p0);
    }
    
    pDC->SelectObject(oldPen);
}

void CFringePolyline::DrawFull(CDC* pDC, int dotSize, 
                                COLORREF lineColor, COLORREF dotColor) {
    DrawPolyline(pDC, lineColor);
    DrawDots(pDC, dotSize, dotColor);
}
```
---
1.2 Add Parallel Storage to CDigitInfo
DigitMode/DigitInfo.h:
```cpp 
class CDigitInfo {
public:
    // OLD - keep during transition
    CArray<CDotInfo> Dots;
    
    // NEW
    CArray<CFringePolyline> Fringes;
    BOOL m_bUseFringeModel;  // Transition flag (default FALSE)
    
    // Conversion utilities
    void ConvertDotsToFringes();
    void ConvertFringesToDots();
    void SyncFringesToDots();
    
    // NEW fringe-based interface
    int CreateFringe(double number, int zapSec = -1);
    void DeleteFringe(int iFringe);
    CFringePolyline* GetFringe(int i);
    
    void AddPointToFringe(int iFringe, CDPoint p);
    void InsertPointInFringe(int iFringe, int iPoint, CDPoint p);
    void RemovePointFromFringe(int iFringe, int iPoint);
    
    BOOL FindPointUnderCursor(CPoint P, int tolerance, 
                              int& iFringe, int& iPoint);
    
    // Keep old interface as wrappers (temporary)
    // void AddDot(CPoint P, int dotSide);  // Existing, will wrap later
};
``` 
DigitMode/DigitInfo.cpp:
```cpp
void CDigitInfo::Init() {
    // ... existing code ...
    m_bUseFringeModel = FALSE;  // Start with old model
}

void CDigitInfo::Clear(BOOL AllZAPSections) {
    HidenDots.RemoveAll();
    Sections.RemoveAll();
    Dots.RemoveAll();
    Fringes.RemoveAll();  // NEW
    // ... rest of existing code ...
}

int CDigitInfo::CreateFringe(double number, int zapSec) {
    CFringePolyline fringe(number, zapSec);
    Fringes.Add(fringe);
    return Fringes.GetSize() - 1;
}

void CDigitInfo::DeleteFringe(int iFringe) {
    if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
        Fringes.RemoveAt(iFringe);
    }
}

CFringePolyline* CDigitInfo::GetFringe(int i) {
    if (i >= 0 && i < Fringes.GetSize()) {
        return &Fringes[i];
    }
    return NULL;
}

void CDigitInfo::AddPointToFringe(int iFringe, CDPoint p) {
    if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
        Fringes[iFringe].AddPoint(p);
    }
}

void CDigitInfo::InsertPointInFringe(int iFringe, int iPoint, CDPoint p) {
    if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
        Fringes[iFringe].InsertPoint(iPoint, p);
    }
}

void CDigitInfo::RemovePointFromFringe(int iFringe, int iPoint) {
    if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
        Fringes[iFringe].RemovePoint(iPoint);
    }
}

BOOL CDigitInfo::FindPointUnderCursor(CPoint P, int tolerance, 
                                       int& iFringe, int& iPoint) {
    for (int iF = 0; iF < Fringes.GetSize(); iF++) {
        int iPt = Fringes[iF].FindNearestPoint(P, tolerance);
        if (iPt >= 0) {
            iFringe = iF;
            iPoint = iPt;
            return TRUE;
        }
    }
    iFringe = iPoint = -1;
    return FALSE;
}
```
---
1.3 Unit Tests
Tests/DigitMode/FringePolylineTest.cpp (NEW):
```cpp
#include "stdafx.h"
#include "CppUnitTest.h"
#include "DigitMode/FringePolyline.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DigitModeTests {
    TEST_CLASS(FringePolylineTest) {
    public:
        TEST_METHOD(TestConstruction) {
            CFringePolyline fringe(1.5, 0);
            Assert::AreEqual(1.5, fringe.GetNumber());
            Assert::AreEqual(0, fringe.GetZapSec());
            Assert::AreEqual(0, fringe.GetPointCount());
        }
        
        TEST_METHOD(TestAddPoint) {
            CFringePolyline fringe(2.0);
            fringe.AddPoint(CDPoint(10, 20));
            fringe.AddPoint(CDPoint(30, 40));
            
            Assert::AreEqual(2, fringe.GetPointCount());
            Assert::AreEqual(10.0, fringe.GetPoint(0).x);
            Assert::AreEqual(40.0, fringe.GetPoint(1).y);
        }
        
        TEST_METHOD(TestInsertPoint) {
            CFringePolyline fringe(1.0);
            fringe.AddPoint(CDPoint(0, 0));
            fringe.AddPoint(CDPoint(20, 20));
            fringe.InsertPoint(1, CDPoint(10, 10));  // Insert in middle
            
            Assert::AreEqual(3, fringe.GetPointCount());
            Assert::AreEqual(10.0, fringe.GetPoint(1).x);
            Assert::AreEqual(20.0, fringe.GetPoint(2).x);
        }
        
        TEST_METHOD(TestRemovePoint) {
            CFringePolyline fringe(1.0);
            fringe.AddPoint(CDPoint(0, 0));
            fringe.AddPoint(CDPoint(10, 10));
            fringe.AddPoint(CDPoint(20, 20));
            
            fringe.RemovePoint(1);  // Remove middle point
            
            Assert::AreEqual(2, fringe.GetPointCount());
            Assert::AreEqual(0.0, fringe.GetPoint(0).x);
            Assert::AreEqual(20.0, fringe.GetPoint(1).x);
        }
        
        TEST_METHOD(TestFindNearestPoint) {
            CFringePolyline fringe(1.0);
            fringe.AddPoint(CDPoint(100, 100));
            fringe.AddPoint(CDPoint(200, 100));
            fringe.AddPoint(CDPoint(300, 100));
            
            int idx = fringe.FindNearestPoint(CPoint(205, 105), 10);
            Assert::AreEqual(1, idx);  // Should find middle point
            
            idx = fringe.FindNearestPoint(CPoint(500, 100), 10);
            Assert::AreEqual(-1, idx);  // Too far
        }
    };
}
```
Deliverable: CFringePolyline class fully tested, integrated into CDigitInfo with no behavior change to existing features.

---
Phase 2: File I/O Migration (Week 3)   

2.1 Update Load Operations   

DigitMode/DigitInfo.cpp - ExamineNumberingInterferogramInfo:

```cpp
BOOL CDigitInfo::ExamineNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO& IntInfo) {
    // ... existing bound/metadata loading ...
    
    int nD = IntInfo.DigitDat.XPnt.GetSize();
    if (nD == 0) return FALSE;
    
    if (m_bUseFringeModel) {
        // NEW: Group into fringes by (Number, Y coordinate)
        Fringes.RemoveAll();
        
        // Build map: (Number, Y) -> points
        std::map<std::pair<double, double>, std::vector<CDPoint>> fringeMap;
        
        for (int i = 0; i < nD; i++) {
            double num = IntInfo.DigitDat.FPnt[i];
            double y = IntInfo.DigitDat.YPnt[i];
            double x = IntInfo.DigitDat.XPnt[i];
            
            auto key = std::make_pair(num, y);
            fringeMap[key].push_back(CDPoint(x, y));
        }
        
        // Convert map to Fringes array
        for (auto& kv : fringeMap) {
            double number = kv.first.first;
            CFringePolyline fringe(number, -1);
            
            // Sort points by X within each fringe segment
            std::vector<CDPoint>& points = kv.second;
            std::sort(points.begin(), points.end(), 
                      [](const CDPoint& a, const CDPoint& b) { return a.x < b.x; });
            
            for (const CDPoint& p : points) {
                fringe.AddPoint(p);
            }
            
            Fringes.Add(fringe);
        }
        
        // Maintain Dots for compatibility
        ConvertFringesToDots();
    }
    else {
        // OLD: Flat loading
        Dots.SetSize(nD);
        for (int i = 0; i < nD; i++) {
            Dots[i].P.x = IntInfo.DigitDat.XPnt[i];
            Dots[i].P.y = IntInfo.DigitDat.YPnt[i];
            Dots[i].Number = IntInfo.DigitDat.FPnt[i];
        }
    }
    
    return TRUE;
}
```
---
2.2 Update Save Operations

DigitMode/DigitInfo.cpp - CollectNumberingInterferogramInfo:

```cpp
BOOL CDigitInfo::CollectNumberingInterferogramInfo(NUMBERING_INTERFEROGRAM_INFO& IntInfo) {
    // ... existing metadata collection ...
    
    if (m_bUseFringeModel) {
        // NEW: Direct fringe iteration
        int totalPoints = 0;
        for (int iF = 0; iF < Fringes.GetSize(); iF++) {
            totalPoints += Fringes[iF].GetPointCount();
        }
        
        if (totalPoints == 0) return FALSE;
        
        IntInfo.DigitDat.XPnt.SetSize(totalPoints);
        IntInfo.DigitDat.YPnt.SetSize(totalPoints);
        IntInfo.DigitDat.FPnt.SetSize(totalPoints);
        IntInfo.DigitDat.Properties.SetSize(totalPoints);
        
        int idx = 0;
        for (int iF = 0; iF < Fringes.GetSize(); iF++) {
            double number = Fringes[iF].GetNumber();
            for (int iP = 0; iP < Fringes[iF].GetPointCount(); iP++) {
                CDPoint p = Fringes[iF].GetPoint(iP);
                IntInfo.DigitDat.XPnt[idx] = p.x;
                IntInfo.DigitDat.YPnt[idx] = p.y;
                IntInfo.DigitDat.FPnt[idx] = number;
                IntInfo.DigitDat.Properties[idx] = 0.;
                idx++;
            }
        }
    }
    else {
        // OLD: Existing Dots iteration
        int nD = Dots.GetSize();
        if (nD == 0) return FALSE;
        
        IntInfo.DigitDat.XPnt.SetSize(nD);
        IntInfo.DigitDat.YPnt.SetSize(nD);
        IntInfo.DigitDat.FPnt.SetSize(nD);
        IntInfo.DigitDat.Properties.SetSize(nD);
        
        for (int i = 0; i < nD; i++) {
            IntInfo.DigitDat.XPnt[i] = Dots[i].P.x;
            IntInfo.DigitDat.YPnt[i] = Dots[i].P.y;
            IntInfo.DigitDat.FPnt[i] = Dots[i].Number;
            IntInfo.DigitDat.Properties[i] = 0.;
        }
    }
    
    return TRUE;
}
```
---
2.3 Simplified WriteFRNData

InterfSolver/Tools/ReadWriteData.cpp - WriteFRNData:

```cpp
void WriteFRNData(LPCTSTR fname, NUMBERING_INTERFEROGRAM_INFO& IntInfo) {
    // ... existing header writing ...
    
    Fl.WriteStringWithEnd("[FRINGES]");
    
    // NEW: If data is already organized as fringes, write directly
    // (This will be filled when IntInfo comes from fringe model)
    if (IntInfo.HasFringeStructure) {  // Add this flag to NUMBERING_INTERFEROGRAM_INFO
        for (int iF = 0; iF < IntInfo.Fringes.GetSize(); iF++) {
            WriteFringePolyline(Fl, IntInfo.Fringes[iF]);
        }
    }
    else {
        // OLD: Group and sort (expensive)
        double EpsF = PRECISION;
        // ... existing grouping logic ...
    }
    
    Fl.WriteStringWithEnd("");
    // ... rest of file ...
}
```
---
2.4 Conversion Utilities

```cpp
void CDigitInfo::ConvertDotsToFringes() {
    Fringes.RemoveAll();
    
    if (Dots.GetSize() == 0) return;
    
    // Group by (Number, Y coordinate)
    std::map<std::pair<double, double>, std::vector<int>> groups;
    for (int i = 0; i < Dots.GetSize(); i++) {
        auto key = std::make_pair(Dots[i].Number, Dots[i].P.y);
        groups[key].push_back(i);
    }
    
    // Create fringes
    for (auto& kv : groups) {
        double number = kv.first.first;
        CFringePolyline fringe(number);
        
        // Sort indices by X coordinate
        std::vector<int>& indices = kv.second;
        std::sort(indices.begin(), indices.end(), 
                  [this](int a, int b) { return Dots[a].P.x < Dots[b].P.x; });
        
        for (int idx : indices) {
            fringe.AddPoint(Dots[idx].P);
            // Note: iZapSec may need special handling
        }
        
        Fringes.Add(fringe);
    }
}

void CDigitInfo::ConvertFringesToDots() {
    Dots.RemoveAll();
    
    for (int iF = 0; iF < Fringes.GetSize(); iF++) {
        double number = Fringes[iF].GetNumber();
        int zapSec = Fringes[iF].GetZapSec();
        
        for (int iP = 0; iP < Fringes[iF].GetPointCount(); iP++) {
            CDotInfo dot;
            dot.P = Fringes[iF].GetPoint(iP);
            dot.Number = number;
            dot.iZapSec = zapSec;
            Dots.Add(dot);
        }
    }
}
```
Testing:
```cpp
TEST_METHOD(TestRoundTrip) {
    // Load old ZAP file with Dots model
    m_bUseFringeModel = FALSE;
    LoadZAP("test_old.zap");
    int oldDotCount = Dots.GetSize();
    
    // Convert to Fringes
    ConvertDotsToFringes();
    
    // Save with Fringe model
    m_bUseFringeModel = TRUE;
    SaveZAP("test_new.zap");
    
    // Reload and verify
    Clear();
    LoadZAP("test_new.zap");
    ConvertFringesToDots();
    
    Assert::AreEqual(oldDotCount, Dots.GetSize());
    // Verify all points match...
}
Deliverable: Files can load into Fringes, save from Fringes. Round-trip tests pass for entire test corpus.
---
Phase 3: Drawing Migration (Week 4)
3.1 Update Draw Method
DigitMode/DigitInfo.cpp:

```cpp
void CDigitInfo::Draw(CDC* pDC, int DotSide) {
    CControls* pCtrls = GetControls();
    
    // ... ZAP sections drawing (unchanged) ...
    // ... extremums drawing (unchanged) ...
    
    if (m_bUseFringeModel) {
        // ===== NEW: Fringe-based drawing =====
        
        if (pCtrls->ViewState & V_DOTLINES) {
            // Draw polylines connecting points
            for (int iF = 0; iF < Fringes.GetSize(); iF++) {
                COLORREF color;
                pCtrls->GetIndexColor(Fringes[iF].GetNumber(), color);
                Fringes[iF].DrawPolyline(pDC, color);
            }
        }
        
        if (pCtrls->ViewState & V_DOTS) {
            // Draw individual dots
            for (int iF = 0; iF < Fringes.GetSize(); iF++) {
                if (iF == idxDraggedPoint.iFringe) {
                    // Highlight dragged fringe
                    Fringes[iF].DrawDots(pDC, DotSide, RGB(255, 0, 0));
                }
                else {
                    Fringes[iF].DrawDots(pDC, DotSide, InvColor);
                }
            }
        }
    }
    else {
        // ===== OLD: Dot-based drawing =====
        
        if (pCtrls->ViewState & V_DOTLINES) {
            CList<double, double> Numbers;
            CArray<CDPoint> adP;
            GetDotNumbers(Numbers);
            
            POSITION pos = Numbers.GetHeadPosition();
            for (int i = 0; i < Numbers.GetCount(); i++) {
                double Num = Numbers.GetNext(pos);
                COLORREF color;
                pCtrls->GetIndexColor(Num, color);
                
                CPen pen1(PS_SOLID, 0, color);
                CPen* open1 = pDC->SelectObject(&pen1);
                
                GetFringeDots(Num, adP);
                for (int ii = 0; ii < adP.GetSize(); ii++) {
                    CPoint wP((int)adP[ii].x, (int)adP[ii].y);
                    if (ii == 0) pDC->MoveTo(wP);
                    else pDC->LineTo(wP);
                }
                
                pDC->SelectObject(open1);
            }
        }
        
        if (pCtrls->ViewState & V_DOTS) {
            for (int iD = 0; iD < Dots.GetSize(); iD++) {
                if (idxDragDot == iD) continue;
                
                if (idxMainDot == iD)
                    Dots[iD].Draw(pDC, DotSide, RGB(255, 0, 0));
                else
                    Dots[iD].Draw(pDC, DotSide);
            }
        }
    }
}
```
---
3.2 UI Toggle for Testing

ImageTempl/ImageView.h:
```cpp
class CImageView : public CBaseImageView {
    // ... existing ...
    
    // NEW: Debug/testing menu
    afx_msg void OnToggleFringeModel();
    afx_msg void OnUpdateToggleFringeModel(CCmdUI* pCmdUI);
};
```
ImageTempl/ImageView.cpp:
```cpp
BEGIN_MESSAGE_MAP(CImageView, CBaseImageView)
    // ... existing ...
    ON_COMMAND(ID_DEBUG_TOGGLE_FRINGE_MODEL, OnToggleFringeModel)
    ON_UPDATE_COMMAND_UI(ID_DEBUG_TOGGLE_FRINGE_MODEL, OnUpdateToggleFringeModel)
END_MESSAGE_MAP()

void CImageView::OnToggleFringeModel() {
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    BOOL& flag = pDoc->Digit.m_bUseFringeModel;
    
    if (flag) {
        // Switch to Dots model
        pDoc->Digit.ConvertFringesToDots();
        flag = FALSE;
    } else {
        // Switch to Fringes model
        pDoc->Digit.ConvertDotsToFringes();
        flag = TRUE;
    }
    
    Invalidate(FALSE);
}

void CImageView::OnUpdateToggleFringeModel(CCmdUI* pCmdUI) {
    CImageDoc* pDoc = (CImageDoc*)GetDocument();
    pCmdUI->SetCheck(pDoc->Digit.m_bUseFringeModel ? 1 : 0);
}
```
Resource.h and menu:
```cpp
#define ID_DEBUG_TOGGLE_FRINGE_MODEL  40999

// In menu:
MENUITEM "Use &Fringe Model\tCtrl+Shift+F", ID_DEBUG_TOGGLE_FRINGE_MODEL
```
---
3.3 Visual Regression Tests
Manual Test Protocol:
1.	Load test file test_complex.zap
2.	Toggle between Dots and Fringes models (Ctrl+Shift+F)
3.	Take screenshots (Alt+PrtScn)
4.	Compare visually: dots, polylines, colors
5.	Verify identical rendering
Automated Test:

```cpp
TEST_METHOD(TestDrawingParity) {
    LoadZAP("test.zap");
    
    // Draw with old model
    m_bUseFringeModel = FALSE;
    CDC dcOld;
    // ... render to bitmap ...
    CBitmap bmpOld = RenderToBitmap(&dcOld);
    
    // Draw with new model
    ConvertDotsToFringes();
    m_bUseFringeModel = TRUE;
    CDC dcNew;
    CBitmap bmpNew = RenderToBitmap(&dcNew);
    
    // Compare bitmaps pixel-by-pixel
    Assert::IsTrue(CompareBitmaps(bmpOld, bmpNew, 0.01 /* tolerance */));
}
```
Deliverable: Both drawing modes produce visually identical output. Toggle works seamlessly.

---
Phase 4: Editing Operations (Weeks 5-6)
4.1 Update Selection Model

DigitMode/DigitInfo.h:
```cpp
// NEW selection structure
struct SelectedPoint {
    int iFringe;
    int iPoint;
    
    SelectedPoint() : iFringe(-1), iPoint(-1) {}
    SelectedPoint(int f, int p) : iFringe(f), iPoint(p) {}
    
    BOOL IsValid() const { return iFringe >= 0 && iPoint >= 0; }
    void Clear() { iFringe = iPoint = -1; }
    
    BOOL operator==(const SelectedPoint& other) const {
        return iFringe == other.iFringe && iPoint == other.iPoint;
    }
};

class CDigitInfo {
    // ... existing ...
    
    // NEW: Replace single indices with fringe+point pairs
    SelectedPoint idxDraggedPoint;  // Replaces idxDragDot
    SelectedPoint idxMainPoint;     // Replaces idxMainDot
    
    // OLD: Keep during transition
    int idxDragDot;
    int idxMainDot;
};
```
---
4.2 Update Dragging

DigitMode/DigitInfo.cpp:
```cpp
BOOL CDigitInfo::LockDot(CPoint P, int dotSide, BOOL Enable) {
    if (m_bUseFringeModel) {
        if (!Enable) {
            idxDraggedPoint.Clear();
            return TRUE;
        }
        
        int iFringe, iPoint;
        if (FindPointUnderCursor(P, dotSide, iFringe, iPoint)) {
            idxDraggedPoint = SelectedPoint(iFringe, iPoint);
            return TRUE;
        }
        return FALSE;
    }
    else {
        // OLD implementation
        if (Dots.GetSize() == 0) return FALSE;
        
        if (!Enable) {
            idxDragDot = -1;
            return TRUE;
        }
        
        int idx;
        if (IsDotUnderCursor(P, dotSide, idx)) {
            idxDragDot = idx;
            return TRUE;
        }
        return FALSE;
    }
}

void CDigitInfo::SetLockedDotPos(CPoint P) {
    if (m_bUseFringeModel) {
        if (idxDraggedPoint.IsValid()) {
            Fringes[idxDraggedPoint.iFringe].MovePoint(
                idxDraggedPoint.iPoint, CDPoint(P.x, P.y));
        }
    }
    else {
        // OLD
        if (idxDragDot >= 0 && idxDragDot < Dots.GetSize()) {
            Dots[idxDragDot].P.x = P.x;
            Dots[idxDragDot].P.y = P.y;
        }
    }
}

void CDigitInfo::GetLockedDotPos(CPoint& P1) {
    if (m_bUseFringeModel) {
        if (idxDraggedPoint.IsValid()) {
            CDPoint p = Fringes[idxDraggedPoint.iFringe].GetPoint(idxDraggedPoint.iPoint);
            P1.x = (int)p.x;
            P1.y = (int)p.y;
        }
    }
    else {
        // OLD
        if (idxDragDot >= 0 && idxDragDot < Dots.GetSize()) {
            P1.x = (int)Dots[idxDragDot].P.x;
            P1.y = (int)Dots[idxDragDot].P.y;
        }
    }
}
```
---
4.3 Update Add/Remove
```cpp
void CDigitInfo::AddDot(CPoint P, int dotSide) {
    CControls* pCtrls = GetControls();
    double num = CurrentNumber;
    
    if (pCtrls->ViewState & V_ZAPSECTIONS) {
        int idx;
        if (GetNearestZapSection(P, idx)) {
            P.y = (int)ZapLines[idx].L.P1.y;
        }
    }
    
    if (m_bUseFringeModel) {
        // NEW: Find or create fringe with same Number and ZapSec
        int targetFringe = -1;
        for (int iF = 0; iF < Fringes.GetSize(); iF++) {
            if (Fringes[iF].GetNumber() == num) {
                targetFringe = iF;
                break;
            }
        }
        
        if (targetFringe < 0) {
            targetFringe = CreateFringe(num);
        }
        
        // Smart insert: find position to maintain X ordering
        int insertIdx = 0;
        for (int i = 0; i < Fringes[targetFringe].GetPointCount(); i++) {
            if (Fringes[targetFringe].GetPoint(i).x > P.x) {
                break;
            }
            insertIdx++;
        }
        
        if (insertIdx >= Fringes[targetFringe].GetPointCount()) {
            Fringes[targetFringe].AddPoint(CDPoint(P.x, P.y));
        } else {
            Fringes[targetFringe].InsertPoint(insertIdx, CDPoint(P.x, P.y));
        }
    }
    else {
        // OLD implementation
        CDotInfo dot;
        dot.Number = num;
        dot.P.x = P.x;
        dot.P.y = P.y;
        dot.iZapSec = zapSec;
        Dots.Add(dot);
    }
}

void CDigitInfo::RemoveDot(CPoint P, int dotSide) {
    if (m_bUseFringeModel) {
        int iFringe, iPoint;
        if (FindPointUnderCursor(P, dotSide, iFringe, iPoint)) {
            RemovePointFromFringe(iFringe, iPoint);
            
            // Delete fringe if now empty
            if (Fringes[iFringe].GetPointCount() == 0) {
                Fringes.RemoveAt(iFringe);
            }
        }
    }
    else {
        // OLD
        int idx;
        if (IsDotUnderCursor(P, dotSide, idx)) {
            Dots.RemoveAt(idx);
        }
    }
}
```
---
4.4 Update Bulk Operations
```cpp
void CDigitInfo::RemoveFringe(CPoint P, int dotSide) {
    if (m_bUseFringeModel) {
        int iFringe, iPoint;
        if (FindPointUnderCursor(P, dotSide, iFringe, iPoint)) {
            double Number = Fringes[iFringe].GetNumber();
            
            // Remove all fringes with same Number
            for (int iF = Fringes.GetSize() - 1; iF >= 0; iF--) {
                if (Fringes[iF].GetNumber() == Number) {
                    Fringes.RemoveAt(iF);
                }
            }
        }
    }
    else {
        // OLD implementation
        int idx;
        if (IsDotUnderCursor(P, dotSide, idx)) {
            double Number = Dots[idx].Number;
            for (int i = Dots.GetSize() - 1; i >= 0; i--) {
                if (Dots[i].Number == Number) {
                    Dots.RemoveAt(i);
                }
            }
        }
    }
}

void CDigitInfo::RenumFringe(CPoint P, int dotSide) {
    if (m_bUseFringeModel) {
        int iFringe, iPoint;
        if (FindPointUnderCursor(P, dotSide, iFringe, iPoint)) {
            double oldNumber = Fringes[iFringe].GetNumber();
            
            // Renumber all fringes with same old Number
            for (int iF = 0; iF < Fringes.GetSize(); iF++) {
                if (Fringes[iF].GetNumber() == oldNumber) {
                    Fringes[iF].SetNumber(CurrentNumber);
                }
            }
        }
    }
    else {
        // OLD implementation
        int idx;
        if (IsDotUnderCursor(P, dotSide, idx)) {
            double Number = Dots[idx].Number;
            CUIntArray idxDots;
            GetFringeDots(Number, idxDots);
            for (int i = 0; i < idxDots.GetSize(); i++) {
                Dots[idxDots[i]].Number = CurrentNumber;
            }
        }
    }
}
```
---
4.5 Update Navigation (Keyboard)
```cpp
void CDigitInfo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    if (m_bUseFringeModel) {
        if (!idxMainPoint.IsValid()) return;
        
        int iF = idxMainPoint.iFringe;
        int iP = idxMainPoint.iPoint;
        
        if (nChar == VK_LEFT) {
            // Previous point in same fringe
            if (iP > 0) {
                idxMainPoint.iPoint--;
                CurrentNumber = Fringes[iF].GetNumber();
            }
        }
        else if (nChar == VK_RIGHT) {
            // Next point in same fringe
            if (iP < Fringes[iF].GetPointCount() - 1) {
                idxMainPoint.iPoint++;
                CurrentNumber = Fringes[iF].GetNumber();
            }
        }
        else if (nChar == VK_UP || nChar == VK_DOWN) {
            // Next/previous fringe with same Number
            double curNum = Fringes[iF].GetNumber();
            int dir = (nChar == VK_UP) ? -1 : 1;
            
            int nextF = FindNextFringeWithNumber(iF, curNum, dir);
            if (nextF >= 0) {
                idxMainPoint.iFringe = nextF;
                idxMainPoint.iPoint = 0;  // First point of new fringe
            }
        }
    }
    else {
        // OLD implementation (existing code)
        int idx;
        CDPoint dP;
        if (idxMainDot != -1) {
            dP = Dots[idxMainDot].P;
            if (nChar == VK_LEFT) {
                if (GetNextDotInSection(Dots[idxMainDot].iZapSec, -1, idx, dP)) {
                    idxMainDot = idx;
                    CurrentNumber = Dots[idx].Number;
                }
            }
            // ... etc ...
        }
    }
}

int CDigitInfo::FindNextFringeWithNumber(int startIdx, double number, int direction) {
    int count = Fringes.GetSize();
    for (int i = 1; i < count; i++) {
        int checkIdx = (startIdx + i * direction + count) % count;
        if (Fringes[checkIdx].GetNumber() == number) {
            return checkIdx;
        }
    }
    return -1;
}
```
---
4.6 Testing
Manual Tests:
*	[ ] Drag dot: works in both models, position updates correctly
*	[ ] Add dot: creates new fringe or appends to existing
*	[ ] Remove dot: removes from fringe, deletes empty fringes
*	[ ] Remove fringe: deletes all segments with same number
*	[ ] Renumber fringe: updates all segments
*	[ ] Keyboard navigation: LEFT/RIGHT moves within fringe, UP/DOWN jumps fringes

Automated Tests:
```cpp
TEST_METHOD(TestAddDotCreatesNewFringe) {
    m_bUseFringeModel = TRUE;
    CurrentNumber = 1.5;
    
    AddDot(CPoint(100, 100), 10);
    
    Assert::AreEqual(1, Fringes.GetSize());
    Assert::AreEqual(1.5, Fringes[0].GetNumber());
    Assert::AreEqual(1, Fringes[0].GetPointCount());
}

TEST_METHOD(TestAddDotInsertsInOrder) {
    m_bUseFringeModel = TRUE;
    CurrentNumber = 1.0;
    
    AddDot(CPoint(100, 100), 10);  // First point
    AddDot(CPoint(300, 100), 10);  // Third point
    AddDot(CPoint(200, 100), 10);  // Insert in middle
    
    Assert::AreEqual(1, Fringes.GetSize());
    Assert::AreEqual(3, Fringes[0].GetPointCount());
    Assert::AreEqual(100.0, Fringes[0].GetPoint(0).x);
    Assert::AreEqual(200.0, Fringes[0].GetPoint(1).x);  // Inserted!
    Assert::AreEqual(300.0, Fringes[0].GetPoint(2).x);
}
```
Deliverable: All editing operations functional with fringe model. Extensive manual and automated tests pass.

---
Phase 5: Advanced Features (Weeks 7-8)
These are new capabilities unlocked by the fringe model.   

5.1 Fringe Splitting   

Use Case: User detects a gap in fringe and wants to split it into two segments.
```cpp
void CDigitInfo::SplitFringeAtPoint(int iFringe, int iPoint) {
    if (iFringe < 0 || iFringe >= Fringes.GetSize()) return;
    if (iPoint <= 0 || iPoint >= Fringes[iFringe].GetPointCount()) return;
    
    CFringePolyline& original = Fringes[iFringe];
    CFringePolyline newFringe(original.GetNumber(), original.GetZapSec());
    
    // Move points [iPoint..end] to new fringe
    for (int i = iPoint; i < original.GetPointCount(); i++) {
        newFringe.AddPoint(original.GetPoint(i));
    }
    
    // Remove those points from original
    for (int i = original.GetPointCount() - 1; i >= iPoint; i--) {
        original.RemovePoint(i);
    }
    
    Fringes.Add(newFringe);
}

// UI: Right-click menu on point → "Split Fringe Here"
```
CFringePolyline.cpp (alternative: method within class):
```cpp
CFringePolyline CFringePolyline::Split(int atIndex) {
    CFringePolyline newFringe(m_Number);
    
    if (atIndex <= 0 || atIndex >= m_Points.GetSize()) {
        return newFringe;  // Invalid, return empty
    }
    
    // Move points [atIndex..end] to new fringe
    for (int i = atIndex; i < m_Points.GetSize(); i++) {
        newFringe.AddPoint(m_Points[i]);
    }
    
    // Remove from original
    for (int i = m_Points.GetSize() - 1; i >= atIndex; i--) {
        m_Points.RemoveAt(i);
    }
    
    return newFringe;
}
```
---
5.2 Fringe Merging   

Use Case: Two fringe segments should be combined into one continuous curve.
```cpp
void CDigitInfo::MergeFringes(int iF1, int iF2) {
    if (iF1 < 0 || iF1 >= Fringes.GetSize()) return;
    if (iF2 < 0 || iF2 >= Fringes.GetSize()) return;
    if (iF1 == iF2) return;
    
    // Only merge fringes with same number
    if (Fringes[iF1].GetNumber() != Fringes[iF2].GetNumber()) {
        AfxMessageBox(_T("Cannot merge fringes with different numbers"));
        return;
    }
    
    // Append all points from iF2 to iF1
    for (int i = 0; i < Fringes[iF2].GetPointCount(); i++) {
        Fringes[iF1].AddPoint(Fringes[iF2].GetPoint(i));
    }
    
    // Delete iF2
    Fringes.RemoveAt(iF2);
    
    // TODO: Consider sorting combined points by X or arc length
}

// UI: Select two fringes → Right-click → "Merge Fringes"
```
---
5.3 Auto-Interpolation   

Use Case: Fill gaps in a sparse fringe by interpolating intermediate points.
```cpp
void CFringePolyline::SubdivideSegments(double maxGap) {
    int originalCount = m_Points.GetSize();
    
    for (int i = 0; i < originalCount - 1; /* increment in loop */) {
        CDPoint p1 = m_Points[i];
        CDPoint p2 = m_Points[i + 1];
        
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dist = sqrt(dx*dx + dy*dy);
        
        if (dist > maxGap) {
            int nInsert = (int)(dist / maxGap);
            
            for (int j = 1; j <= nInsert; j++) {
                double t = (double)j / (nInsert + 1);
                CDPoint pNew(p1.x + t * dx, p1.y + t * dy);
                InsertPoint(i + j, pNew);
            }
            
            i += nInsert + 1;  // Skip inserted points
            originalCount += nInsert;
        }
        else {
            i++;
        }
    }
}

// Usage
void CDigitInfo::InterpolateFringe(int iFringe, double maxGap) {
    if (iFringe >= 0 && iFringe < Fringes.GetSize()) {
        Fringes[iFringe].SubdivideSegments(maxGap);
    }
}

// UI: Right-click fringe → "Interpolate Points (max gap: __)"
```
-
5.4 Fringe Simplification (Douglas-Peucker)   

Use Case: Remove redundant points from over-sampled fringe.
```cpp
void CFringePolyline::Simplify(double epsilon) {
    if (m_Points.GetSize() <= 2) return;
    
    CArray<BOOL> keep;
    keep.SetSize(m_Points.GetSize());
    for (int i = 0; i < keep.GetSize(); i++) {
        keep[i] = FALSE;
    }
    
    keep[0] = TRUE;  // Always keep endpoints
    keep[keep.GetSize() - 1] = TRUE;
    
    SimplifyRecursive(0, m_Points.GetSize() - 1, epsilon, keep);
    
    // Remove points not marked for keeping
    for (int i = m_Points.GetSize() - 1; i >= 0; i--) {
        if (!keep[i]) {
            m_Points.RemoveAt(i);
        }
    }
}

void CFringePolyline::SimplifyRecursive(int start, int end, double epsilon, CArray<BOOL>& keep) {
    if (end - start <= 1) return;
    
    // Find point farthest from line segment
    double maxDist = 0;
    int maxIdx = start;
    
    CDPoint p1 = m_Points[start];
    CDPoint p2 = m_Points[end];
    
    for (int i = start + 1; i < end; i++) {
        double dist = PointToLineDistance(m_Points[i], p1, p2);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }
    
    if (maxDist > epsilon) {
        keep[maxIdx] = TRUE;
        SimplifyRecursive(start, maxIdx, epsilon, keep);
        SimplifyRecursive(maxIdx, end, epsilon, keep);
    }
}

double CFringePolyline::PointToLineDistance(CDPoint p, CDPoint lineStart, CDPoint lineEnd) {
    double A = p.x - lineStart.x;
    double B = p.y - lineStart.y;
    double C = lineEnd.x - lineStart.x;
    double D = lineEnd.y - lineStart.y;
    
    double dot = A * C + B * D;
    double len_sq = C * C + D * D;
    double param = (len_sq != 0) ? dot / len_sq : -1;
    
    double xx, yy;
    if (param < 0) {
        xx = lineStart.x;
        yy = lineStart.y;
    } else if (param > 1) {
        xx = lineEnd.x;
        yy = lineEnd.y;
    } else {
        xx = lineStart.x + param * C;
        yy = lineStart.y + param * D;
    }
    
    double dx = p.x - xx;
    double dy = p.y - yy;
    return sqrt(dx*dx + dy*dy);
}
```
---
5.5 Gap Detection   

Use Case: Identify discontinuities in fringes (missing data).
```cpp
struct FringeGap {
    int iFringe;
    int iPointBefore;
    double gapSize;
    CDPoint startPoint;
    CDPoint endPoint;
};

void CDigitInfo::DetectFringeGaps(double threshold, CArray<FringeGap>& gaps) {
    gaps.RemoveAll();
    
    for (int iF = 0; iF < Fringes.GetSize(); iF++) {
        for (int iP = 0; iP < Fringes[iF].GetPointCount() - 1; iP++) {
            CDPoint p1 = Fringes[iF].GetPoint(iP);
            CDPoint p2 = Fringes[iF].GetPoint(iP + 1);
            
            double dx = p2.x - p1.x;
            double dy = p2.y - p1.y;
            double dist = sqrt(dx*dx + dy*dy);
            
            if (dist > threshold) {
                FringeGap gap;
                gap.iFringe = iF;
                gap.iPointBefore = iP;
                gap.gapSize = dist;
                gap.startPoint = p1;
                gap.endPoint = p2;
                gaps.Add(gap);
            }
        }
    }
}

// UI: Menu → "Analyze → Detect Fringe Gaps"
// Shows list of gaps with options: Interpolate, Split, Ignore
```
Deliverable: Power-user features for fringe manipulation. Comprehensive testing on real data.

---
Phase 6: Cleanup & Finalization (Week 9)   

6.1 Remove Legacy Code   

Files to Modify:
*	DigitInfo.h
*	DigitInfo.cpp
*	ImageView.cpp   
   
Changes:
```cpp
// DigitMode/DigitInfo.h
class CDigitInfo {
public:
    // REMOVE:
    // CArray<CDotInfo> Dots;
    // BOOL m_bUseFringeModel;
    // int idxDragDot;
    // int idxMainDot;
    
    // KEEP:
    CArray<CFringePolyline> Fringes;
    SelectedPoint idxDraggedPoint;
    SelectedPoint idxMainPoint;
    
    // REMOVE all wrapper methods that branch on m_bUseFringeModel
    // REMOVE ConvertDotsToFringes(), ConvertFringesToDots()
};
```
Cleanup Checklist:
*	[x] Remove Dots array
*	[x] Remove m_bUseFringeModel flag
*	[x] Remove all if (m_bUseFringeModel) { ... } else { ... } branches
*	[x] Remove conversion utilities
*	[x] Remove debug toggle UI
*	[x] Update all call sites to use fringe API directly
*	[x] Remove unused CDotInfo class (if no other use)
---
6.2 Update Documentation   

DigitMode/README.md (NEW):
```markdown
# DigitMode Module: Fringe Analysis

## Architecture

The `CDigitInfo` class manages interferogram fringe data using a **polyline-based model**:

- Each fringe is a `CFringePolyline` (ordered sequence of points)
- Fringes are stored in `CArray<CFringePolyline> Fringes`
- Topology (point ordering, curve continuity) is explicit

## Data Structures

### CFringePolyline
Represents a single continuous fringe curve.

**Key Members**:
- `double m_Number`: Fringe number (e.g., 0, 0.5, 1.0, ...)
- `CArray<CDPoint> m_Points`: ORDERED point sequence
- `BOOL m_bClosed`: Closed loop flag

**Key Operations**:
- `AddPoint()`, `InsertPoint()`, `RemovePoint()`, `MovePoint()`
- `FindNearestPoint()`, `DrawPolyline()`, `Split()`, `SubdivideSegments()`

### SelectedPoint
Identifies a specific point within a fringe.
```
```cpp
struct SelectedPoint 
{ 
    int iFringe;  // Index into Fringes array 
    int iPoint;   // Index into Points array within fringe 
};
```

## File Format

ZAP/FRN files store fringes as polylines:
```
[FRINGES] <Y> <Number> <X1> <Number> <X2> ...  E
```

The file format directly maps to `CFringePolyline` structure.

## Migration History

**Prior to 2025**: Used flat `CArray<CDotInfo> Dots` (topology implicit)  
**2025**: Migrated to `CArray<CFringePolyline> Fringes` (topology explicit)

See `FRINGE_REFACTORING_PLAN.md` for migration details.
```
---
6.3 Code Comments
Add summary comments to key files:
```cpp
/// <summary>
/// CDigitInfo: Manages interferogram fringe data.
/// 
/// ARCHITECTURE:
/// Fringes are stored as ordered polylines (CFringePolyline objects).
/// Each fringe has a unique number and contains an ordered sequence of points.
/// 
/// Multiple fringe segments can have the same number (e.g., discontinuous fringes).
/// 
/// SELECTION MODEL:
/// Selected points are identified by (iFringe, iPoint) pairs (SelectedPoint struct).
/// This allows direct access to any point without O(N) scans.
/// 
/// FILE I/O:
/// ZAP/FRN files natively store fringes as polylines.
/// Load/save operations directly map to Fringes array structure.
/// </summary>
class CDigitInfo {
    // ...
};
```
---
6.4 Performance Validation
Benchmark Tests:
```cpp
TEST_METHOD(BenchmarkDrawing) {
    LoadZAP("large_file.zap");  // 1000+ fringes
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        Draw(&dc, 5);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should be < 1000ms for 100 redraws
    Assert::IsTrue(duration.count() < 1000, L"Drawing too slow");
}

TEST_METHOD(BenchmarkHitTesting) {
    LoadZAP("large_file.zap");
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int iFringe, iPoint;
    for (int i = 0; i < 10000; i++) {
        FindPointUnderCursor(CPoint(rand() % 1000, rand() % 1000), 5, iFringe, iPoint);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should be < 500ms for 10k hit tests
    Assert::IsTrue(duration.count() < 500, L"Hit testing too slow");
}
```
Optimization (if needed):
*	Spatial indexing (R-tree) for large files
*	Cache bounding boxes per fringe
*	Use vector instead of CArray for points (profile first!)
---
Deliverable: Clean codebase with single data model, comprehensive documentation, validated performance.

---
5. Benefits Analysis   
5.1 Performance   
 
|Operation|Before (Dots)|After (Fringes)|Improvement|
| --- | --- | --- | --- |   
|Draw polyline|O(N) scan + sort|O(1) access|~100x faster|
|Navigate to next point|O(N) linear search|O(1) array index|~1000x faster|   
|Insert point in fringe|Impossible|O(N) insert|∞ (new capability)|
|Save to file|O(N²) grouping/sorting|O(N) direct write|~10x faster|
|Hit test point|O(N) scan all dots|O(F×P) scan fringes|~10x faster\*|

\* With spatial indexing: ~100x faster

---
5.2 Code Clarity
Before:
```cpp
// What is this doing? Reconstructing topology from metadata!
GetFringeDots(Number, adP);  // Scan all dots
SortDots(adP, 2);            // Sort by Y
for (int ii = 0; ii < adP.GetSize(); ii++) {
    // Hope this is the right order...
}
```
After:
```cpp
// Crystal clear: draw the fringe polyline
Fringes[iF].DrawPolyline(pDC);
```
---

5.3 New Capabilities   

|Feature	|Dots Model	|Fringe Model|
| --- | --- | --- |
|Insert point between existing	|❌	|✅|
|Split discontinuous fringe	|❌	|✅|
|Merge fringe segments	|❌	|✅|
|Detect gaps	|❌ Unreliable	|✅ Reliable|
|Interpolate missing points	|❌	|✅|
|Simplify (Douglas-Peucker)	|❌	|✅|
|Arc length computation	|❌	|✅|
|Fringe-aware undo/redo	|❌	|✅|

---
5.4 Maintainability   
Bugs Prevented:   
*	Drawing connects wrong points (topology assumed incorrectly)
*	Navigation skips points or loops infinitely
*	File save loses point ordering
*	Operations fail when multiple fringes have same number

  * Easier Debugging:   
-	Inspect Fringes[3].GetPoint(5) instead of scanning array for (Number=X, iZapSec=Y)
-	Visualize fringe structure directly (one object = one curve)

---
6. Risk Assessment & Mitigation   
6.1 Risk Matrix   

|Risk	|Likelihood	|Impact	|Mitigation	|Status|
| --- | --- | --- | --- | --- |
|Break existing files	|Medium	|High	|Parallel storage, extensive testing	|✅ Mitigated|
|Performance regression	|Low	|Medium	|Profiling, benchmarks	|✅ Mitigated|
|Incomplete migration	|Medium	|High	Phased approach, feature flag	✅ Mitigated|
|User confusion	|Low	|Low	|No UI changes during transition	|✅ Mitigated|
|Bugs in new code	|Medium	|Medium	|Unit tests, manual testing	|⚠️ Ongoing|

---
6.2 Mitigation Strategies
Backward Compatibility
*	Strategy: Keep Dots array during transition (Phases 1-5)
*	Validation: Round-trip tests on entire corpus (500+ files)
*	Rollback: Feature flag allows instant revert

Performance
*	Strategy: Benchmark before/after on large files
*	Target: <5% regression (expect 2-10x improvement)
*	Contingency: Spatial indexing (R-tree) if needed

Testing
*	Strategy: Parallel execution (old vs new model)
*	Coverage: Unit tests (80%+), integration tests, manual testing
*	Tools: CppUnit, visual regression tests

Training
*	Strategy: No user-facing changes until Phase 6
*	Documentation: In-code comments, technical docs
*	Review: Architecture review before Phase 1 start

---
7. Testing Strategy
7.1 Test Pyramid

```
         /\
        /  \  Manual Testing (5%)
       /    \   - Visual regression
      /------\  - Exploratory testing
     /  E2E   \
    /----------\ Integration Tests (15%)
   / File I/O   \  - Round-trip tests
  /--------------\  - Workflow tests
 / Unit Tests (80%)\
/--------------------\
  CFringePolyline
  CDigitInfo methods
  ```
---
7.2 Unit Tests (80% coverage target)   

CFringePolyline (Tests/DigitMode/FringePolylineTest.cpp):
*	[x] Construction, copy, assignment
*	[x] Add/Insert/Remove/Move points
*	[x] Hit testing (FindNearestPoint, IsPointOnPolyline)
*	[x] Drawing (no crashes, GDI cleanup)
*	[x] Edge cases (empty fringe, single point, duplicate points)
*	[x] Advanced (Split, Subdivide, Simplify)

CDigitInfo (Tests/DigitMode/DigitInfoTest.cpp):
*	[x] CreateFringe, DeleteFringe, GetFringe
*	[x] AddPointToFringe, InsertPointInFringe, RemovePointFromFringe
*	[x] FindPointUnderCursor
*	[x] Selection (LockDot, SetLockedDotPos, GetLockedDotPos)
*	[x] Bulk operations (RemoveFringe, RenumFringe)
*	[x] Conversion (ConvertDotsToFringes, ConvertFringesToDots)

---
7.3 Integration Tests   
File I/O (Tests/DigitMode/FileIOTest.cpp):

```cpp
TEST_METHOD(TestRoundTripZAP) {
    // Load with old model
    m_bUseFringeModel = FALSE;
    LoadZAP("corpus/test1.zap");
    int oldCount = Dots.GetSize();
    
    // Convert and save with new model
    ConvertDotsToFringes();
    m_bUseFringeModel = TRUE;
    SaveZAP("temp/test1_new.zap");
    
    // Reload and compare
    Clear();
    LoadZAP("temp/test1_new.zap");
    ConvertFringesToDots();
    
    Assert::AreEqual(oldCount, Dots.GetSize());
    // Compare each dot position and number...
}

TEST_METHOD(TestLoadEntireCorpus) {
    CStringArray files;
    FindFiles("corpus/*.zap", files);
    
    for (int i = 0; i < files.GetSize(); i++) {
        m_bUseFringeModel = TRUE;
        BOOL success = LoadZAP(files[i]);
        Assert::IsTrue(success, files[i]);
        
        // Verify fringes created
        Assert::IsTrue(Fringes.GetSize() > 0);
    }
}
```
Workflow Tests (Tests/DigitMode/WorkflowTest.cpp):
```cpp
TEST_METHOD(TestCompleteEditingWorkflow) {
    LoadZAP("test.zap");
    
    // Simulate user workflow
    AddDot(CPoint(100, 100), 10);          // 1. Add point
    
    int iF, iP;
    FindPointUnderCursor(CPoint(100, 100), 10, iF, iP);
    Assert::IsTrue(iF >= 0);
    
    LockDot(CPoint(100, 100), 10, TRUE);   // 2. Select point
    SetLockedDotPos(CPoint(150, 100));     // 3. Drag
    LockDot(CPoint(150, 100), 10, FALSE);  // 4. Release
    
    SaveZAP("test_modified.zap");          // 5. Save
    
    // Reload and verify
    Clear();
    LoadZAP("test_modified.zap");
    Assert::AreEqual(150.0, Fringes[iF].GetPoint(iP).x);
}
```
---
7.4 Manual Testing   

Visual Regression:
```
1. Load test_complex.zap
2. Toggle Fringe Model ON
3. Take screenshot (Alt+PrtScn) → save as "new_model.png"
4. Toggle Fringe Model OFF
5. Take screenshot → save as "old_model.png"
6. Use image diff tool → verify identical (tolerance: 0.1%)
```
Exploratory Testing Scenarios:
*	[ ] Large file (1000+ fringes): performance acceptable?
*	[ ] Sparse fringe (2 points): no crash on operations?
*	[ ] Overlapping fringes: hit testing selects correct one?
*	[ ] Rapid add/remove cycles: memory stable?
*	[ ] Keyboard navigation: reaches all points?
*	[ ] Undo/redo (if implemented): correct state restoration?

---
7.5 Performance Tests   

Benchmark Suite (Tests/Performance/BenchmarkTests.cpp):
```cpp
TEST_METHOD(BenchmarkSuite) {
    struct TestCase {
        CString file;
        int numFringes;
        int numPoints;
    };
    
    TestCase cases[] = {
        { "small.zap",  10,   100 },
        { "medium.zap", 100,  1000 },
        { "large.zap",  1000, 10000 }
    };
    
    for (auto& tc : cases) {
        LoadZAP(tc.file);
        
        // Draw benchmark
        auto t1 = HighResClock::now();
        for (int i = 0; i < 100; i++) Draw(&dc, 5);
        auto t2 = HighResClock::now();
        double drawMs = duration_cast<milliseconds>(t2 - t1).count();
        
        // Hit test benchmark
        t1 = HighResClock::now();
        for (int i = 0; i < 1000; i++) {
            int iF, iP;
            FindPointUnderCursor(CPoint(rand()%1000, rand()%1000), 5, iF, iP);
        }
        t2 = HighResClock::now();
        double hitMs = duration_cast<milliseconds>(t2 - t1).count();
        
        // Save benchmark
        t1 = HighResClock::now();
        SaveZAP("temp.zap");
        t2 = HighResClock::now();
        double saveMs = duration_cast<milliseconds>(t2 - t1).count();
        
        Logger::WriteMessage(
            StringPrintf("%s: Draw=%.1fms, HitTest=%.1fms, Save=%.1fms\n",
                         tc.file, drawMs, hitMs, saveMs));
        
        // Assert performance thresholds
        Assert::IsTrue(drawMs < 1000);   // <1s for 100 redraws
        Assert::IsTrue(hitMs < 500);     // <0.5s for 1000 hit tests
        Assert::IsTrue(saveMs < 5000);   // <5s to save
    }
}
```
---
8. Success Criteria   
8.1 Phase Completion Criteria   

|Phase	|Exit Criteria|
| --- | --- |
|1. Foundation	|• CFringePolyline class complete<br>• All unit tests pass (80%+ coverage)<br>• Integrated into CDigitInfo without breaking builds|
|2. File I/O	|• Round-trip test: Load ZAP → Save → Load → identical data<br>• All corpus files (500+) load successfully<br>• FRN files: polyline write code simplified|
|3. Drawing	|• Visual regression: old vs new identical (0.1% tolerance)<br>• Toggle works seamlessly<br>• No GDI leaks (check with GDIView)|
|4. Editing	|• All operations (add/remove/drag/renumber) work with fringes<br>• Manual test checklist 100% pass<br>• Automated workflow tests pass|
|5. Advanced	|• Split/Merge/Interpolate/Simplify functional<br>• Gap detection reliable<br>• Power-user testing complete|
|6. Cleanup	|• Legacy code removed<br>• Documentation complete<br>• Performance validated (no regressions)|

---
8.2 Overall Success Metrics   

Must Have (Go/No-Go):   
*	✅ All existing files load correctly
*	✅ Save → Load round-trip preserves data
*	✅ No visual regressions
*	✅ All editing operations functional
*	✅ Performance ≥ old model (no >5% regressions)

Should Have (Defer if needed):
*	🎯 80%+ unit test coverage
*	🎯 Advanced features (split/merge)
*	🎯 Documentation complete

Nice to Have (Future):
*	💡 Spatial indexing for large files
*	💡 Undo/redo fringe-aware
*	💡 Animation (fringe growth playback)

---
9. Appendix
9.1 File Inventory
New Files Created:
```
DigitMode/
  FringePolyline.h          (NEW) - Fringe polyline class declaration
  FringePolyline.cpp        (NEW) - Implementation
  README.md                 (NEW) - Module documentation

Tests/DigitMode/
  FringePolylineTest.cpp    (NEW) - Unit tests for CFringePolyline
  DigitInfoTest.cpp         (UPDATE) - Add fringe-based tests
  FileIOTest.cpp            (UPDATE) - Add round-trip tests
  WorkflowTest.cpp          (NEW) - Integration workflow tests

Tests/Performance/
  BenchmarkTests.cpp        (NEW) - Performance benchmarks

Docs/
  FRINGE_REFACTORING_PLAN.md (THIS FILE)
```

```
Modified Files:
DigitMode/
  DigitInfo.h               (UPDATE) - Add Fringes array, SelectedPoint
  DigitInfo.cpp             (UPDATE) - Implement fringe-based logic

InterfSolver/Tools/
  ReadWriteData.cpp         (UPDATE) - Simplify WriteFRNData

ImageTempl/
  ImageView.cpp             (UPDATE) - Add toggle UI (Phase 3), remove (Phase 6)
```
---
9.2 Glossary

|Term	|Definition|
| --- | --- |
|Fringe	|Continuous curve in interferogram with constant phase number|
|Polyline	|Ordered sequence of connected points|
|ZAP Section	|Horizontal slice through interferogram (constant Y)|
|Dot	|Single point in old flat-array model|
|Fringe Number	|Numerical identifier (e.g., 0, 0.5, 1.0) assigned to fringe|
|Topology	|Spatial relationships (ordering, connectivity) between points|
|Hit Testing	|Finding which fringe/point is under mouse cursor|
|Round-trip	|Load file → Save file → Load again (should be identical)|

---
9.3 References

Codebase:
*	DigitInfo.cpp (current implementation)
*	ReadWriteData.cpp (file I/O)
*	ImageView.cpp (UI interactions)

Algorithms:
*	Douglas-Peucker simplification: Wikipedia
*	R-tree spatial indexing: Wikipedia

Testing:
*	CppUnit documentation: SourceForge
*	Visual regression testing: Percy.io concepts

---
9.4 Decision Log

|Date	|Decision	|Rationale|
| --- | --- | --- |
|2026-01-21	|Use CArray<CDPoint> for fringe points	|Consistency with existing codebase (MFC-style)|
|2026-01-21	|Parallel storage during transition	|Risk mitigation: allows instant rollback|
|2026-01-21	|Introduce SelectedPoint struct	|Cleaner than separate iFringe/iPoint variables|
|2026-01-21	|Defer spatial indexing to Phase 5+	|Premature optimization; profile first|
|2026-01-21	|Keep ZAP/FRN file format unchanged	|No need to break compatibility

---
9.5 Open Questions
For Architecture Review:
1.	Should CFringePolyline be a class or struct?
-	Recommendation: Class (has behavior, not just data)
2.	Use vector or CArray for points?
-	Recommendation: CArray for consistency (entire codebase uses MFC)
3.	Add undo/redo in Phase 4 or defer to Phase 5?
-	Recommendation: Defer to Phase 5 (complex, not critical path)
4.	Introduce C++11 features (Map, vector) or stick to MFC?
-	Recommendation: Hybrid (C++11 for algorithms, MFC for data structures)
5.	Spatial indexing: R-tree or grid?
-	Recommendation: Start with grid (simpler), upgrade if needed
---
9.6 Contact & Approval

Document Owner: Architecture Team   
Reviewers: Senior Developers, QA Lead   
Approval Required: Tech Lead, Project Manager   

Review Checklist:
-	[ ] Technical approach sound?
-	[ ] Risk mitigation adequate?
-	[ ] Timeline realistic?
-	[ ] Resource allocation (9 weeks developer time)?
-	[ ] Testing strategy comprehensive?
Approval Signatures:
```
Tech Lead: ________________  Date: _______
Project Manager: __________  Date: _______
```
---
Conclusion   
The migration from a flat Dots array to a structured Fringes model represents a fundamental correction of the domain model. While requiring significant effort (9 weeks), the benefits—in code clarity, performance, and extensibility—are substantial and permanent.

Recommendation:    
Approve and proceed with Phase 1 as proof-of-concept.
After Phase 1-2 success (3 weeks), reassess and commit to full migration.

---
Document Version: 1.0   
Last Updated: 2026-01-21   
Next Review: After Phase 1 completion

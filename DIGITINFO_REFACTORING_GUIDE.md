# CDigitInfo Extraction Refactoring Guide

## Quick Reference: Seams to Break

This document provides **concrete code examples** for breaking the 5 key dependencies.

---

## SEAM #1: Global Control Access (Dependency Injection)

### Current Implementation (TIGHT COUPLING)
```cpp
// In DigitMode\DigitInfo.cpp

void CDigitInfo::CreateBufLineApertureSimple()
{
    CBoundCtrls* pB = GetBoundCtrls();      // ? GLOBAL ACCESS - PROBLEM
    CImageCtrls* pI = GetImageCtrls();      // ? GLOBAL ACCESS - PROBLEM
    // ...uses pB and pI...
}

void CDigitInfo::CreateRedCenters()
{
    // ...
    CBoundCtrls* pB = GetBoundCtrls();      // ? GLOBAL ACCESS - PROBLEM
    CImageCtrls* pI = GetImageCtrls();      // ? GLOBAL ACCESS - PROBLEM
    CControls* pCtrls = GetControls();      // ? GLOBAL ACCESS - PROBLEM
    // ...uses all three...
}
```

### Solution: Dependency Injection Pattern

**Step 1: Update CDigitInfo class definition**
```cpp
// In DigitMode\DigitInfo.h

class CDigitInfo
{
public:
    // ... existing public methods ...
    
    // NEW: Setter for dependencies (call once at initialization)
    void SetDependencies(CBoundCtrls* pBounds, 
                        CImageCtrls* pImage, 
                        CControls* pControls);

private:
    // NEW: Store injected dependencies
    CBoundCtrls* m_pBounds;
    CImageCtrls* m_pImage;
    CControls* m_pControls;
    
    // Keep existing private methods unchanged
};
```

**Step 2: Implement dependency setter in cpp**
```cpp
// In DigitMode\DigitInfo.cpp

void CDigitInfo::SetDependencies(CBoundCtrls* pBounds, 
                                 CImageCtrls* pImage, 
                                 CControls* pControls)
{
    m_pBounds = pBounds;
    m_pImage = pImage;
    m_pControls = pControls;
}
```

**Step 3: Replace global calls with member access**
```cpp
// BEFORE:
void CDigitInfo::CreateBufLineApertureSimple()
{
    CBoundCtrls* pB = GetBoundCtrls();      // ? GLOBAL
    CImageCtrls* pI = GetImageCtrls();      // ? GLOBAL
    // ... use pB and pI ...
}

// AFTER:
void CDigitInfo::CreateBufLineApertureSimple()
{
    // Just use member pointers
    CBoundCtrls* pB = m_pBounds;           // ? INJECTED
    CImageCtrls* pI = m_pImage;            // ? INJECTED
    // ... SAME CODE, no need to change ...
}
```

**Step 4: At calling site (in consuming project/application)**
```cpp
// In your new project, when creating CDigitInfo:

void YourApplication::InitializeDigitInfo()
{
    CDigitInfo* pDigitInfo = new CDigitInfo();
    
    // Inject the control objects
    pDigitInfo->SetDependencies(m_pBoundCtrls,    // Your bounds control
                               m_pImageCtrls,    // Your image control
                               m_pControls);     // Your app controls
    
    // Now CDigitInfo can be used without global functions
}
```

**Methods Affected (7 total):**
1. `CreateBufLineAperture()`
2. `CreateBufLineApertureSimple()`
3. `CreateBufLineApertureComplex()`
4. `CreateBufLineObstructionSimple()`
5. `CreateBufLineObstructionComplex()`
6. `CreateRedCenters()`
7. `CreateZAPSections()`
8. `CreateZAPSectionsOnLoadZAPFile()`

---

## SEAM #2: UI Rendering (Extract to Separate Class)

### Current Implementation (MIXED CONCERNS)
```cpp
// In DigitMode\DigitInfo.cpp

void CDigitInfo::Draw(CDC* pDC, int DotSide)
{
    int iS = 0;
    CControls* pCtrls = GetControls();      // ? NEEDS INJECTION OR REMOVAL
    
    // Drawing code mixed with data class
    COLORREF Color = RGB(255, 0, 0);
    CPen pen;
    pen.CreatePen(PS_SOLID, 0, InvColor);
    CPen* open = pDC->SelectObject(&pen);
    
    if (pCtrls->ViewState & V_ZAPSECTIONS) {
        for (int i = 0; i < ZapLines.GetSize(); i++) {
            if (idxDragZapLine == i || ZapLines[i].Removed)
                continue;
            ZapLines[i].Draw(pDC);  // ? ALSO DRAWS
        }
    }
    // ... many more drawing operations ...
}
```

### Solution: Create Separate Renderer Class

**Step 1: Create new renderer class header**
```cpp
// In new file: DigitMode\DigitInfoRenderer.h

#pragma once

class CDigitInfo;      // Forward declaration
class CControls;
class CDC;

/// Handles all rendering/visualization of CDigitInfo data
/// Separates UI concerns from data management
class CDigitInfoRenderer
{
public:
    CDigitInfoRenderer();
    ~CDigitInfoRenderer();
    
    /// Main render entry point
    void Draw(CDC* pDC, const CDigitInfo& digitInfo, int DotSide);
    
private:
    // Helper methods for each visualization component
    void DrawZapLines(CDC* pDC, const CDigitInfo& digitInfo);
    void DrawSections(CDC* pDC, const CDigitInfo& digitInfo);
    void DrawDots(CDC* pDC, const CDigitInfo& digitInfo, int DotSide);
    void DrawFringeDots(CDC* pDC, const CDigitInfo& digitInfo);
    void DrawExtremums(CDC* pDC, const CDigitInfo& digitInfo);
    
    // Optional: for UI settings (can be injected if needed)
    CControls* m_pControls;  // Optional, for ViewState flags
    
public:
    void SetControls(CControls* pCtrl) { m_pControls = pCtrl; }
};
```

**Step 2: Implement renderer**
```cpp
// In new file: DigitMode\DigitInfoRenderer.cpp

#include "DigitInfoRenderer.h"
#include "DigitInfo.h"
#include "Controls\controls.h"

CDigitInfoRenderer::CDigitInfoRenderer() 
    : m_pControls(NULL)
{
}

CDigitInfoRenderer::~CDigitInfoRenderer()
{
}

void CDigitInfoRenderer::Draw(CDC* pDC, const CDigitInfo& digitInfo, int DotSide)
{
    if (!pDC) return;
    
    // CDigitInfo is const - we only read data, don't modify it
    DrawZapLines(pDC, digitInfo);
    DrawExtremums(pDC, digitInfo);
    DrawSections(pDC, digitInfo);
    DrawFringeDots(pDC, digitInfo);
    DrawDots(pDC, digitInfo, DotSide);
}

void CDigitInfoRenderer::DrawZapLines(CDC* pDC, const CDigitInfo& digitInfo)
{
    if (!m_pControls) return;
    if (!(m_pControls->ViewState & V_ZAPSECTIONS)) return;
    
    COLORREF Color = RGB(255, 0, 0);
    CPen pen;
    pen.CreatePen(PS_SOLID, 0, InvColor);
    CPen* open = pDC->SelectObject(&pen);
    
    for (int i = 0; i < digitInfo.ZapLines.GetSize(); i++) {
        // NOTE: ZapLines[i].Draw() still uses CDC - that's OK here (rendering layer)
        digitInfo.ZapLines[i].Draw(pDC);
    }
    
    CPen* retPen = pDC->SelectObject(open);
    if (retPen) retPen->DeleteObject();
}

void CDigitInfoRenderer::DrawDots(CDC* pDC, const CDigitInfo& digitInfo, int DotSide)
{
    if (!m_pControls) return;
    if (!(m_pControls->ViewState & V_DOTS)) return;
    
    for (int iD = 0; iD < digitInfo.Dots.GetSize(); iD++) {
        // NOTE: CDotInfo::Draw() uses CDC - that's OK here
        digitInfo.Dots[iD].Draw(pDC, DotSide, 
                               iD == digitInfo.idxMainDot);
    }
}

// ... implement other Draw* methods similarly ...
```

**Step 3: Remove Draw() from CDigitInfo**
```cpp
// In DigitMode\DigitInfo.h

class CDigitInfo
{
public:
    // REMOVE THIS:
    // void Draw(CDC* pDC, int DotSide);  // TODO: remove UI dependency
    
    // Keep all data and algorithm methods
};
```

**Step 4: Update consuming code**
```cpp
// BEFORE (in view class):
void CMyView::OnDraw(CDC* pDC)
{
    CDigitInfo* pDigitInfo = GetDigitInfo();
    pDigitInfo->Draw(pDC, DotSide);  // ? CAN'T DO THIS ANYMORE
}

// AFTER (in view class):
void CMyView::OnDraw(CDC* pDC)
{
    CDigitInfo* pDigitInfo = GetDigitInfo();
    m_renderer.Draw(pDC, *pDigitInfo, DotSide);  // ? USE RENDERER
}

// In view class header:
class CMyView
{
    // ...
private:
    CDigitInfoRenderer m_renderer;  // ? ADD THIS
};
```

**Methods Affected:**
- Remove: `CDigitInfo::Draw(CDC*, int)`
- Remove: `CSectionInfo::Draw(CDC*, double)` [if not used elsewhere]
- Remove: `CDotInfo::Draw(CDC*, int, BOOL)` [if not used elsewhere]
- Remove: `CZapLineInfo::Draw(CDC*)` [if not used elsewhere]

---

## SEAM #3: Keyboard Input Event Handling

### Current Implementation
```cpp
// In DigitMode\DigitInfo.cpp

void CDigitInfo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
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
        // ... more key handling ...
    }
}
```

### Solution Options:

**Option A: Keep but Remove from DigitInfo (Move to View)**
```cpp
// Move the logic to your view class

void CMyView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CDigitInfo* pDigitInfo = GetDigitInfo();
    
    if (pDigitInfo->idxMainDot != -1) {
        int idx;
        CDPoint dP;
        
        if (nChar == VK_LEFT) {
            if (pDigitInfo->GetNextDotInSection(
                    pDigitInfo->Dots[pDigitInfo->idxMainDot].iZapSec, 
                    -1, idx, dP)) {
                pDigitInfo->idxMainDot = idx;
                pDigitInfo->CurrentNumber = pDigitInfo->Dots[idx].Number;
            }
        }
        // ... etc ...
    }
}
```

**Option B: Create Callback/Delegate Interface**
```cpp
// In DigitMode\DigitInfo.h

class IDigitInfoKeyHandler
{
public:
    virtual ~IDigitInfoKeyHandler() {}
    virtual void OnKeyLeft(CDigitInfo& digitInfo) = 0;
    virtual void OnKeyRight(CDigitInfo& digitInfo) = 0;
    virtual void OnKeyUp(CDigitInfo& digitInfo) = 0;
    virtual void OnKeyDown(CDigitInfo& digitInfo) = 0;
};

class CDigitInfo
{
private:
    IDigitInfoKeyHandler* m_pKeyHandler;
    
public:
    void SetKeyHandler(IDigitInfoKeyHandler* pHandler) 
    { 
        m_pKeyHandler = pHandler; 
    }
    
    void OnKeyDown_Internal(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (!m_pKeyHandler || idxMainDot == -1) return;
        
        switch(nChar) {
            case VK_LEFT:
                m_pKeyHandler->OnKeyLeft(*this);
                break;
            case VK_RIGHT:
                m_pKeyHandler->OnKeyRight(*this);
                break;
            case VK_UP:
                m_pKeyHandler->OnKeyUp(*this);
                break;
            case VK_DOWN:
                m_pKeyHandler->OnKeyDown(*this);  // Note: confusing name!
                break;
        }
    }
};
```

**Recommendation:** Option A is simpler. Just move it to the view class in your consuming project.

---

## SEAM #4: Cursor State (SetCursor)

### Current Implementation
```cpp
// In DigitInfo.cpp, Auto() method

void CDigitInfo::Auto()
{
    ::SetCursor(::LoadCursor(NULL, IDC_WAIT));    // ? UI CALL
    Clear(FALSE);
    CreateBufLine();
    CreateRedCenters();
    // ... processing ...
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));   // ? UI CALL
}
```

### Solution: Wrap in Progress Callback

**Step 1: Add progress/status interface**
```cpp
// In new file: DigitMode\IDigitInfoProgress.h

class IDigitInfoProgress
{
public:
    virtual ~IDigitInfoProgress() {}
    
    /// Called when long operation starts
    virtual void OnProcessingStarted() { }
    
    /// Called when long operation completes
    virtual void OnProcessingCompleted() { }
    
    /// Called periodically during processing
    virtual void OnProgress(int percentComplete) { }
};
```

**Step 2: Use interface in CDigitInfo**
```cpp
// In DigitMode\DigitInfo.h

class CDigitInfo
{
private:
    IDigitInfoProgress* m_pProgress;
    
public:
    void SetProgressHandler(IDigitInfoProgress* pProgress)
    {
        m_pProgress = pProgress;
    }
    
    // ... rest of class ...
};

// In DigitMode\DigitInfo.cpp

void CDigitInfo::Auto()
{
    if (m_pProgress) m_pProgress->OnProcessingStarted();
    
    // NO MORE: ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    
    Clear(FALSE);
    CreateBufLine();
    CreateRedCenters();
    SelectFringeStep();
    SelectMainSection();
    CreateNumLines();
    if (!isInsideScreen) {
        SelectMainFringe();
        CorrectNumbers();
    }
    CreateZAPSections();
    SelectMainDot();
    Delete_buf_line();
    
    if (m_pProgress) m_pProgress->OnProcessingCompleted();
    
    // NO MORE: ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
```

**Step 3: Implement in your consuming view**
```cpp
// In your application

class CMyDigitInfoProgress : public IDigitInfoProgress
{
public:
    void OnProcessingStarted() override
    {
        ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
    }
    
    void OnProcessingCompleted() override
    {
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    }
};

// Usage:
CMyDigitInfoProgress progress;
m_pDigitInfo->SetProgressHandler(&progress);
m_pDigitInfo->Auto();  // Will call progress handler
```

**Methods Affected:**
- `Auto()` - Has SetCursor calls
- `CreateRedCenters()` - Has SetCursor calls

---

## SEAM #5: File I/O (Define Serialization Interface)

### Current State (Stubs)
```cpp
// In DigitInfo.cpp - these are minimal stubs

BOOL CDigitInfo::Load(LPCTSTR fname)      { return FALSE; }
BOOL CDigitInfo::Save(LPCTSTR fname, int extIdx) { return FALSE; }
BOOL CDigitInfo::LoadZAP(LPCTSTR fname)   { return FALSE; }
BOOL CDigitInfo::SaveZAP(LPCTSTR fname, int extIdx) { return FALSE; }
BOOL CDigitInfo::LoadFRN(LPCTSTR fname)   { return FALSE; }
BOOL CDigitInfo::SaveFRN(LPCTSTR fname)   { return FALSE; }
```

### Solution: Define Serializer Interface

**Step 1: Create serialization interface**
```cpp
// New file: DigitMode\IDigitInfoSerializer.h

#pragma once

class CDigitInfo;

class IDigitInfoSerializer
{
public:
    virtual ~IDigitInfoSerializer() {}
    
    /// Load complete DigitInfo from file
    virtual BOOL Load(const CString& filePath, CDigitInfo& outDigitInfo) = 0;
    
    /// Save complete DigitInfo to file
    virtual BOOL Save(const CString& filePath, 
                     const CDigitInfo& digitInfo, 
                     int extIdx = 0) = 0;
    
    /// Load ZAP section data
    virtual BOOL LoadZAP(const CString& filePath, CDigitInfo& outDigitInfo) = 0;
    
    /// Save ZAP section data
    virtual BOOL SaveZAP(const CString& filePath, 
                        const CDigitInfo& digitInfo, 
                        int extIdx = 0) = 0;
    
    /// Load FRN (fringe) data
    virtual BOOL LoadFRN(const CString& filePath, CDigitInfo& outDigitInfo) = 0;
    
    /// Save FRN (fringe) data
    virtual BOOL SaveFRN(const CString& filePath, const CDigitInfo& digitInfo) = 0;
};
```

**Step 2: Update CDigitInfo to use interface**
```cpp
// In DigitMode\DigitInfo.h

class CDigitInfo
{
private:
    IDigitInfoSerializer* m_pSerializer;
    
public:
    void SetSerializer(IDigitInfoSerializer* pSerializer)
    {
        m_pSerializer = pSerializer;
    }
    
    // Keep existing signatures for compatibility, but delegate:
    BOOL Load(LPCTSTR fname)
    {
        if (!m_pSerializer) return FALSE;
        return m_pSerializer->Load(fname, *this);
    }
    
    BOOL Save(LPCTSTR fname, int extIdx)
    {
        if (!m_pSerializer) return FALSE;
        return m_pSerializer->Save(fname, *this, extIdx);
    }
    
    // ... similar for other Load/Save methods ...
};
```

**Step 3: Implement in consuming project**
```cpp
// In your project, file: MyDigitInfoSerializer.h/.cpp

class CMyDigitInfoSerializer : public IDigitInfoSerializer
{
public:
    BOOL Load(const CString& filePath, CDigitInfo& outDigitInfo) override;
    BOOL Save(const CString& filePath, const CDigitInfo& digitInfo, int extIdx) override;
    // ... implement all methods ...
};

// Implement based on your file format requirements
BOOL CMyDigitInfoSerializer::Load(const CString& filePath, CDigitInfo& outDigitInfo)
{
    // Your implementation here
    // Read from file, populate outDigitInfo
    return TRUE;
}
```

**Methods Affected:**
- `Load()`, `Save()`
- `LoadZAP()`, `SaveZAP()`
- `LoadFRN()`, `SaveFRN()`
- `ResolveImagePath()` - static helper
- `CreateFakeGrayImage()` - static helper

---

## Summary: Refactoring Checklist

| Seam | Priority | Effort | Impact | Pattern |
|------|----------|--------|--------|---------|
| 1. Global Control Access | **P0** | **HIGH** | 8 methods | Dependency Injection |
| 2. UI Rendering | **P1** | **MEDIUM** | 4 Draw() methods | Extract Renderer |
| 3. Keyboard Input | **P2** | **LOW** | 1 method | Move to View |
| 4. Cursor State | **P2** | **LOW** | 2 methods | Progress Handler |
| 5. File I/O | **P3** | **MEDIUM** | 6 methods | Serializer Interface |

---

## Integration Pattern in New Project

```cpp
// In your application startup code

void YourApp::InitializeDigitInfo()
{
    // 1. Create core object
    CDigitInfo* pDigitInfo = new CDigitInfo();
    
    // 2. Inject dependencies (P0 - CRITICAL)
    pDigitInfo->SetDependencies(m_pBoundCtrls, m_pImageCtrls, m_pControls);
    
    // 3. Setup rendering (P1 - UI)
    m_renderer.SetControls(m_pControls);
    
    // 4. Setup progress handling (P2 - Optional)
    pDigitInfo->SetProgressHandler(&m_progressHandler);
    
    // 5. Setup serialization (P3 - Optional)
    pDigitInfo->SetSerializer(&m_serializer);
    
    // Now CDigitInfo is ready to use without global functions!
}

// In your view class
void CMyView::OnDraw(CDC* pDC)
{
    // Instead of: pDigitInfo->Draw(pDC, DotSide);
    m_renderer.Draw(pDC, *pDigitInfo, DotSide);  // ? Use renderer
}

void CMyView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // Can either:
    // A) Keep logic here and use pDigitInfo directly
    // B) Use callback interface
    // C) Keep method but don't use global functions
}
```

This pattern allows CDigitInfo to be used in any project without dependency on the original application's global control objects or UI framework choices.

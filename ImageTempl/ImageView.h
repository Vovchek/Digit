#if !defined(AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_)
#define AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageView.h : header file
//
#include "BaseImageView.h"
#include "Utils\contour.h"
#include "DigitMode/FringeInputHandler.h"
#include "DigitMode/BoundsInputHandler.h"
#include "DigitMode/BoundsToolAdapter.h"
#include "DigitMode/FringeToolAdapter.h"
#include "DigitMode/HitTester.h"
#include "DigitMode/SelectionManager.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/CursorManager.h"
#include "DigitMode/TooltipGenerator.h"
#include "DigitMode/Rendering/ShapeDrawDispatcher.h"
#include <afxcmn.h>
#include <memory>

class CBaseImageView;
/////////////////////////////////////////////////////////////////////////////
// CImageView view

class CImageView : public CBaseImageView
{
private:
    CDocument* m_pDoc{nullptr}; // Cached pointer to document (for convenience)
    // ========================================================================
    // Phase 5: Tool Input Handlers (CAD-Grade Architecture)
    // ========================================================================
    
    DigitMode::FringeInputHandler m_fringeInputHandler;    ///< Fringe editing tool
    DigitMode::BoundsInputHandler m_boundsInputHandler;    ///< Bounds editing tool
    DigitMode::ShapeDrawDispatcher m_shapeDrawDispatcher; ///< Shape rendering dispatcher (Phase 4)
    
    // ========================================================================
    // Day 3: Tool Adapters for InteractionManager
    // ========================================================================
    
    DigitMode::BoundsToolAdapter* m_boundsToolAdapter{nullptr};   ///< Adapter for InteractionManager (raw ptr)
    DigitMode::FringeToolAdapter* m_fringeToolAdapter{nullptr};   ///< Adapter for InteractionManager (raw ptr)
    
    // ========================================================================
    // Legacy support infrastructure (will be integrated into tool handlers)
    // ========================================================================
    
	DigitMode::SelectionManager m_selectionMgr;
	DigitMode::HitTester m_hitTester;
	DigitMode::CommandDispatcher m_cmdDispatcher;
	DigitMode::CursorManager m_cursorMgr;
	DigitMode::TooltipGenerator tooltipGen;
    CToolTipCtrl m_tooltip; // dynamic tooltip for dots/segments
    CString m_lastTip; // last shown tooltip text
    
public:
    /**
     * @brief Get view transform (from base class)
     * 
     * Phase 5: ViewTransform now owned by CBaseImageView.
     * This accessor delegates to base class.
     */
    ViewTransform& GetViewTransform() { return CBaseImageView::m_viewTransform; }
    
    /**
     * @brief Activate default editing tool in default mode
     *
     * Sets default tool handler as active tool in InteractionManager.
     * Called from toolbar/menu handlers.
     */
    void ActivateDefaultTool();

    /**
     * @brief Activate fringe editing tool
     * 
     * Sets fringe handler as active tool in InteractionManager.
     * Called from toolbar/menu handlers.
     */
    void ActivateFringeTool();
    
    /**
     * @brief Activate bounds editing tool
     * 
     * Sets bounds handler as active tool in InteractionManager.
     * Called from toolbar/menu handlers.
     */
    void ActivateBoundsTool();
    
    /**
     * @brief Update tooltip control (override from BaseImageView)
     * @param tooltip Tooltip text to display
     */
    void UpdateTooltip(const CString& tooltip) override;

protected:
	CImageView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CImageView)

// Attributes
public:

    HDC hDC;

// Operations
public:
    CDocument* GetDocument() { 
        if(!m_pDoc)
            return GetWIActiveDocument(); 
		return m_pDoc;
    }
    void Init();
    bool GetXPixelLine(CPoint P, double*& pR, double*& pF, int& nP);
    bool GetYPixelLine(CPoint P, double*& pR, double*& pF, int& nP);

	void SingleIsoline(int pn, ISO_POINT *plist, double level, int ilevel);
    // Draw image bitmap with view-aware sampling
    void DrawImage(CDC* pDC);
    // Center image in viewport
    void CenterImageInView();
    // Override zoom methods to use ViewTransform
    void OnZoomIn();
    void OnZoomOut();
    void OnZoomFit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HBRUSH bkColorBrush;
    CRect MeasureLine;
    int PisActive;
    BOOL m_bLinning;
	BOOL m_Captured;
	UINT m_nTimer;
	
protected:
    void DrawCrossedLines(CDC* pDC);
    void DrawBounds(CDC* pDC);
    void DrawMouseMoveCrossedLines(CPoint P /* Screen */);
    void BeginLine(CPoint P /* Screen */);
    void EndLine(CPoint P2 /* Screen */);
    void DrawMouseMoveMeasureLine(CPoint P2 /* Screen */);
    void DrawMeasureLine(CDC* pDC);
    void DrawDigitInfo(CDC* pDC);
    void DrawAproximation(CDC* pDC);

	// deperecated/eliminated use firnges editing methods (replaced by FringeInputHandler)
    //void BeginDragDot(CPoint P /* Screen */);
    //void DragDot(CPoint P /* Screen */, BOOL newPos=TRUE);
    //void DropDot(CPoint P /* Screen */);

    //void BeginDragZapSection(CPoint P /* Screen */);
    //void DragZapSection(CPoint P /* Screen */, BOOL newPos=TRUE);
    //void DropZapSection(CPoint P /* Screen */);

protected:
	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CImageView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
    afx_msg void OnMeasure();
    afx_msg void OnUpdateMeasure(CCmdUI* pCmdUI);
    afx_msg void OnFotoSections();
    afx_msg void OnUpdateFotoSections(CCmdUI* pCmdUI);
	afx_msg void OnAutoDigit();
	afx_msg void OnUpdateAutoDigit(CCmdUI* pCmdUI);
	afx_msg void OnFCMax();
	afx_msg void OnUpdateFCMax(CCmdUI* pCmdUI);
	afx_msg void OnFCMin();
	afx_msg void OnUpdateFCMin(CCmdUI* pCmdUI);
	afx_msg void OnFCMinMax();
	afx_msg void OnUpdateFCMinMax(CCmdUI* pCmdUI);
	afx_msg void OnClearDigit();
	afx_msg void OnUpdateClearDigit(CCmdUI* pCmdUI);
	afx_msg void OnCalcAproximation();
	afx_msg void OnUpdateCalcAproximation(CCmdUI* pCmdUI);
	
	// ========================================================================
	// Undo/Redo Command Handlers
	// ========================================================================
	
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	
	// ========================================================================
	// Bounds Editing Command Handlers
	// ========================================================================

    // Add shapes
    afx_msg void OnAddBoundCircle();
    afx_msg void OnUpdateAddBound(CCmdUI* pCmdUI);
    afx_msg void OnAddBoundEllipse();
    afx_msg void OnAddBoundRect();
    afx_msg void OnAddBoundPolygon();
    // Mode switching
    afx_msg void OnBoundVisisbility();
    afx_msg void OnUpdateBoundVisibility(CCmdUI* pCmdUI);
    afx_msg void OnBoundModeSelect();
    afx_msg void OnUpdateBoundModeSelect(CCmdUI* pCmdUI);
    afx_msg void OnBoundModeDelete();
    afx_msg void OnUpdateBoundModeDelete(CCmdUI* pCmdUI);

    // Fringes edit (temporary)
    afx_msg void OnFringesEdit();
    afx_msg void OnUpdateFringesEdit(CCmdUI* pCmdUI);
    afx_msg void OnDotEdit();
    afx_msg void OnUpdateDotEdit(CCmdUI* pCmdUI);
    afx_msg void OnFringesSelect();
    afx_msg void OnUpdateFringesSelect(CCmdUI* pCmdUI);
    afx_msg void OnNumberFringesForward();
    afx_msg void OnUpdateNumberFringesForward(CCmdUI* pCmdUI);
    afx_msg void OnNumberFringesBackward();
    afx_msg void OnUpdateNumberFringesBackward(CCmdUI* pCmdUI);
    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_)

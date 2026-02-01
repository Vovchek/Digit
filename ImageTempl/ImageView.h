#if !defined(AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_)
#define AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageView.h : header file
//
#include "BaseImageView.h"
#include "Utils\contour.h"
#include "DigitMode/InputHandler.h"
#include "DigitMode/HitTester.h"
#include "DigitMode/SelectionManager.h"
#include "DigitMode/CommandDispatcher.h"
#include "DigitMode/CursorManager.h"
#include "DigitMode/TooltipGenerator.h"
#include <afxcmn.h>
#include "ViewTransform.h"

class CBaseImageView;
/////////////////////////////////////////////////////////////////////////////
// CImageView view

class CImageView : public CBaseImageView
{
private:

	DigitMode::InputHandler m_inputHandler;
	DigitMode::SelectionManager m_selectionMgr;
	DigitMode::HitTester m_hitTester;
	DigitMode::CommandDispatcher m_cmdDispatcher;
	DigitMode::CursorManager m_cursorMgr;
	DigitMode::TooltipGenerator tooltipGen;
    CToolTipCtrl m_tooltip; // dynamic tooltip for dots/segments
    CString m_lastTip; // last shown tooltip text
    ViewTransform m_viewTransform;
    // Expose view transform accessor
public:
    ViewTransform& GetViewTransform() { return m_viewTransform; }

protected:
	CImageView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CImageView)

// Attributes
public:
    CPoint CursorPos;

    HDC hDC;

// Operations
public:
    void Init();
    bool GetXPixelLine(CPoint P, double*& pR, double*& pF, int& nP);
    bool GetYPixelLine(CPoint P, double*& pR, double*& pF, int& nP);

	void SingleIsoline(int pn, ISO_POINT *plist, double level, int ilevel);
    // Draw image bitmap with view-aware sampling
    void DrawImage(CDC* pDC);
    // Center image in viewport
    void CenterImageInView();

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
    void DrawMouseMoveCrossedLines(CPoint P);
    void BeginLine(CPoint P);
    void EndLine(CPoint P2);
    void DrawMouseMoveMeasureLine(CPoint P2);
    void DrawMeasureLine(CDC* pDC);
    void DrawDigitInfo(CDC* pDC);
    void DrawAproximation(CDC* pDC);

    void BeginDragDot(CPoint P);
    void DragDot(CPoint P, BOOL newPos=TRUE);
    void DropDot(CPoint P);

    void BeginDragZapSection(CPoint P);
    void DragZapSection(CPoint P, BOOL newPos=TRUE);
    void DropZapSection(CPoint P);

protected:
	virtual ~CImageView();
#ifdef _DEBUG
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
#if _MSC_VER >= 1400
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
#endif
#else
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CImageView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
    afx_msg void OnMeasure();
    afx_msg void OnUpdateMeasure(CCmdUI* pCmdUI);
    afx_msg void OnFotoSections();
    afx_msg void OnUpdateFotoSections(CCmdUI* pCmdUI);
    afx_msg void OnExtBounds();
    afx_msg void OnUpdateExtBounds(CCmdUI* pCmdUI);
    afx_msg void OnInsBounds();
    afx_msg void OnUpdateInsBounds(CCmdUI* pCmdUI);
    afx_msg void OnZoom();
    afx_msg void OnUpdateZoom(CCmdUI* pCmdUI);
    afx_msg void OnUndo();
    afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnAutoDigit();
	afx_msg void OnFCMax();
	afx_msg void OnUpdateFCMax(CCmdUI* pCmdUI);
	afx_msg void OnFCMin();
	afx_msg void OnUpdateFCMin(CCmdUI* pCmdUI);
	afx_msg void OnFCMinMax();
	afx_msg void OnUpdateFCMinMax(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAutoDigit(CCmdUI* pCmdUI);
	afx_msg void OnClearDigit();
	afx_msg void OnUpdateClearDigit(CCmdUI* pCmdUI);
	afx_msg void OnCalcAproximation();
    afx_msg void OnUpdateCalcAproximation(CCmdUI* pCmdUI);
	afx_msg void OnAddDot();
	afx_msg void OnUpdateAddDot(CCmdUI* pCmdUI);
	afx_msg void OnRemoveDot();
	afx_msg void OnUpdateRemoveDot(CCmdUI* pCmdUI);
	afx_msg void OnRemoveFringe();
	afx_msg void OnUpdateRemoveFringe(CCmdUI* pCmdUI);
	afx_msg void OnAddZAPSection();
	afx_msg void OnUpdateAddSection(CCmdUI* pCmdUI);
	afx_msg void OnDelZAPSection();
	afx_msg void OnUpdateDelSection(CCmdUI* pCmdUI);
	afx_msg void OnShiftDotLeft();
	afx_msg void OnUpdateShiftDotLeft(CCmdUI* pCmdUI);
	afx_msg void OnShiftDotRight();
	afx_msg void OnUpdateShiftDotRight(CCmdUI* pCmdUI);
	afx_msg void OnRenumFringe();
	afx_msg void OnUpdateRenumFringe(CCmdUI* pCmdUI);
	afx_msg void OnRenumDot();
	afx_msg void OnUpdateRenumDot(CCmdUI* pCmdUI);
	afx_msg void OnNumberMinus();
	afx_msg void OnUpdateNumberMinus(CCmdUI* pCmdUI);
	afx_msg void OnNumberPlus();
	afx_msg void OnUpdateNumberPlus(CCmdUI* pCmdUI);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEVIEW_H__0EB4F04B_79B3_4FDE_A46E_A1CA7E62D173__INCLUDED_)

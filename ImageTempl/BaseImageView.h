#if !defined(AFX_BASEIMAGEVIEW_H__1941C47C_4C98_4938_AEA4_62BAA2AB0489__INCLUDED_)
#define AFX_BASEIMAGEVIEW_H__1941C47C_4C98_4938_AEA4_62BAA2AB0489__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BaseImageView.h : header file
//
#include "Utils\seczmvw.h"
typedef enum {NORMAL,ZOOMINPOINT,ZOOMOUTPOINT,ZOOMRECT} DigitViewMode;

/////////////////////////////////////////////////////////////////////////////
// CBaseImageView view

class CBaseImageView : public SECZoomView
{
protected:
	CBaseImageView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBaseImageView)

// Attributes
public:
    DigitViewMode m_cvwMode;              //Current 'mode'
    BOOL          m_bCaptured;            //Is the mouse captured?
    CRect         m_rectDrag;             //The dragging rectangle for zoomrect mode.
    
    //Cursors for Zooming UI.
    HCURSOR       m_hZoomInPointCursor;   
    HCURSOR       m_hZoomOutPointCursor;
    HCURSOR       m_hZoomRectCursor;
    HCURSOR       m_hZoomRectDragCursor;

// Operations
public:
    void ClientToDoc(CPoint& point);
    void ClientToDoc(CRect& rect);
    void ClientToDoc(CSize& size);
    void DocToClient(CPoint& point);
    void DocToClient(CRect& rect);
    void DocToClient(CSize& size);

    void GetZoomCoefficent(double& kZ);
    CRect GetImageRegion(bool absReg=false);
    void DrawImage(CDC* pDC);
	void DrawBounds(CDC* pDC);
    void DrawBackGround(CDC* pDC);
    void DrawCurBound(CDC* pDC);
    void DrawMarker(CDC* pDC, CPoint cP, double zoomMark=1., COLORREF Color=RGB(0,0,0));
    void DrawCustomDots(CDC* pDC);
    void SetCustomDot(CPoint point);
	void DragCustomDot(CPoint point, bool ReDraw=true);
	void DropCustomDot(CPoint point);
    void DrawTracker(CDC* pDC);
    CPoint GetHandle(int nHandle);
    void SetTrackerRect(CRect wR = CRect(-1,-1,-1,-1));

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseImageView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
  
protected:
	virtual ~CBaseImageView();
    void OnCopyClipboard();
    void CreateDefaultMenu(CPoint point);
    void CreateBoundMenu(CPoint point);

	void BeginTracker(CPoint P1);
    void DragTracker(CPoint P2);
    void DropTracker(CPoint P2);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CBaseImageView)
    afx_msg void OnCopyScn();
    afx_msg void OnPasteScn();
    afx_msg void OnUpdateCopyScn(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePasteScn(CCmdUI* pCmdUI);
    afx_msg void OnLoadScn();
    afx_msg void OnSaveScn();
    afx_msg void OnUpdateLoadScn(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSaveScn(CCmdUI* pCmdUI);
    afx_msg void OnCloneDoc();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnZoomFit();
    afx_msg void OnCopy();
    afx_msg void OnApplyBound();
    afx_msg void OnRemoveLastBound();
	afx_msg void OnRemoveCurBound();
    afx_msg void OnRemoveAllBound();
    afx_msg void OnSetupDotsBound();
    afx_msg void OnSetupRectBound();
    afx_msg void OnScrRoundBound();
    afx_msg void OnScrEllipseBound();
    afx_msg void OnScrRectBound();
	afx_msg void OnScrPlgBound();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEIMAGEVIEW_H__1941C47C_4C98_4938_AEA4_62BAA2AB0489__INCLUDED_)

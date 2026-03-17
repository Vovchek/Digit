// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__B990BF0A_613E_4AB6_AA52_43946A86F83A__INCLUDED_)
#define AFX_MAINFRM_H__B990BF0A_613E_4AB6_AA52_43946A86F83A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommentsFileDlg.h"
#include <afxmdiframewndex.h>
#include <afxvisualmanager.h>

class CDiditBar : public CToolBar
{
public:
    CEdit m_Edit;
    CFont m_font;
};

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
    void SetCurrentNumber(double CurrentNumber);
	void SetScaleFactor(double scaleFactor) { m_wndInfoDlgBar.ScaleFactor = scaleFactor; };
	void SetImageInfo(LPCTSTR Title, double ScaleFactor, double Rotation);
	void GetImageInfo(CString& Title, double& ScaleFactor, double& Rotation);
	// Helper to set main status bar text from views/tools
	void SetStatusText(LPCTSTR text);

// Attributes
public:
// Operations
public:
//	CToolBar* GetImageToolBar(){return &m_wndImageBar;}
    CDialogBar* GetMeasureDlgBar(){return &m_wndMeasureDlgBar;}
    void ShowMeasurePane(BOOL Visual);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	//CMFCStatusBar m_wndStatusBar;
	CMFCToolBar   m_wndMainBar;
	CStatusBar  m_wndStatusBar;
	CMFCToolBar    m_wndViewBar;
	CMFCToolBar    m_wndKitBar;
	CMFCToolBar    m_wndDigitBar;
    CMFCToolBar    m_wndApertureBar; // bounds/aperture editing toolbar
    CImageList  m_ilApertureHot;  // hot/pressed state images for aperture toolbar
    CDiditBar   m_wndEditBar;
	int m_nImagePaneCol;
	CReBar      m_wndMeasureBar;
	CDialogBar  m_wndMeasureDlgBar;
	CReBar      m_wndInfoBar;
	CCommentsFile  m_wndInfoDlgBar;

// Generated message map functions
protected:
    void DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf);
	
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
    afx_msg void OnToolBars();
    afx_msg void OnUpdateToolBars(CCmdUI* pCmdUI);
	afx_msg void OnAdvParameters();
	afx_msg void OnUpdateAdvParameters(CCmdUI* pCmdUI);
	afx_msg void OnIterfView();
	afx_msg void OnUpdateIterfView(CCmdUI* pCmdUI);
	afx_msg void OnExtremeView();
	afx_msg void OnUpdateExtremeView(CCmdUI* pCmdUI);
	afx_msg void OnSectionView();
	afx_msg void OnUpdateSectionView(CCmdUI* pCmdUI);
	afx_msg void OnDotLineView();
	afx_msg void OnUpdateDotLineView(CCmdUI* pCmdUI);
	afx_msg void OnDotsView();
	afx_msg void OnUpdateDotsView(CCmdUI* pCmdUI);
	afx_msg void OnApproximation();
	afx_msg void OnUpdateApproximation(CCmdUI* pCmdUI);
	afx_msg void OnClearMeasure();
	afx_msg void OnChangeLang();
    afx_msg void OnHelp();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
    afx_msg LONG OnOpenMsg(UINT, LONG lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__B990BF0A_613E_4AB6_AA52_43946A86F83A__INCLUDED_)

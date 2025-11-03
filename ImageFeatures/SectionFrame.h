#if !defined(AFX_SECTIONFRAME_H__D26133B8_F4B3_4792_A7DA_BE50FA143890__INCLUDED_)
#define AFX_SECTIONFRAME_H__D26133B8_F4B3_4792_A7DA_BE50FA143890__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SectionFrame.h : header file
//C:\Ilya\Programming\cpp\Numbering\ImageFeatures\SectionFrame.h
#include "MGTools\Include\Graph\2DGraph.h"
/////////////////////////////////////////////////////////////////////////////
// CSectionFrame frame

class CSectionFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CSectionFrame)
protected:

// Attributes
public:
  C2DGraph* pGr;

// Operations
public:
	virtual ~CSectionFrame();
	CSectionFrame(); 
    void SetRedrawOnClose(CView* pV);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSectionFrame)
	public:
    virtual void PostNcDestroy();
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CView* pRedrawView;
	// Generated message map functions
	//{{AFX_MSG(CSectionFrame)
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECTIONFRAME_H__D26133B8_F4B3_4792_A7DA_BE50FA143890__INCLUDED_)

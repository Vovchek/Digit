#if !defined(AFX_BOOKMARKFOLDER_H__7295CD4E_8ADA_11D3_9437_86637E89661A__INCLUDED_)
#define AFX_BOOKMARKFOLDER_H__7295CD4E_8ADA_11D3_9437_86637E89661A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BookMarkFolder.h : header file
//

#include "fixmem.h"
#include "MGToolsExport.h"

/////////////////////////////////////////////////////////////////////////////
// CBookMarkFolder window

class CBookMarkDlg;

struct CBookMarkInfo
{
	CBookMarkDlg *pDlg;
    CString Title;


	CBookMarkInfo();
	void operator=(CBookMarkInfo &B) { pDlg=B.pDlg; Title=B.Title;}
};

class MGTOOLS_API CBookMarkControl : public CTabCtrl
{
// Construction
public:
	CBookMarkControl();

// Attributes
public:
	Mem<CBookMarkInfo> BI;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBookMarkControl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void Refresh();
	void SetMSDrawStyle(BOOL bEnable=false);
	BOOL bDrawMSStyle;
	CPen PenG;
	CPen PenW;
	CPen PenB;
	CBrush BrushD;
	COLORREF GrayColor;
	COLORREF TxtColor;
	COLORREF SelTxtColor;
	CFont SelFont;
	CFont EasyFont;
	int iCurrDlg;
	int iDefDlg;

	void Init();
	virtual ~CBookMarkControl();
	int AddPage(CBookMarkDlg *pD, UINT id, CString Text);
	int AddPage(CBookMarkDlg *pD, CString Text);

	Mem<CBookMarkInfo> *GetBookMarkInfo() {return &BI;}
	CBookMarkDlg *GetBookMarkInFocus() {return BI(iCurrDlg).pDlg;}
	int GetCurrentBookMarkNum() {return iCurrDlg;}

	CBookMarkDlg *GetBookMark(int Num);
	int GetBookMark(CBookMarkDlg *pDlg);

	void ChangeBookMark(int iBookMark);
	void ChangeBookMark(CBookMarkDlg *pBookMark);

	void SetBookMarkDefault(int iBookMark);
	void SetBookMarkDefault(CBookMarkDlg *pBookMark);
	void Enable(int iDlg, int State);

	virtual BOOL PrintBookMark();
	virtual BOOL IsBookMarkPrinted();

protected:
//	void DeleteAll();
	BOOL ShowDlg(int N);
	BOOL ShowDlg(CBookMarkDlg *pDlg);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBookMarkControl)
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP();
};

class MGTOOLS_API CBookMarkDlg : public CDialog
{
// Construction
public:
	CBookMarkDlg();
	CBookMarkDlg(UINT Id, CWnd *pParent=0);

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBookMarkDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnOK();
	CBookMarkControl *pTabControl;
	virtual BOOL DoSetFocus();
	virtual BOOL DoKillFocus();
	virtual void DoShow();

	virtual BOOL PrintBookMark();
	virtual BOOL IsBookMarkPrinted();

	void Enable(int State=TRUE);
	int IsEnabled() {return bEnable;}

	void SetTitleColor(COLORREF TxtColor=RGB(0,0,0),
					   COLORREF SelTxtColor=RGB(0,0,0));
	void SetGrayColor(COLORREF GrayColor=RGB(128,128,128));


	int bEnable;

	COLORREF  TitleTxtColor;
	COLORREF TitleSelTxtColor;
	COLORREF TitleGrayColor;

		
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOOKMARKFOLDER_H__7295CD4E_8ADA_11D3_9437_86637E89661A__INCLUDED_)

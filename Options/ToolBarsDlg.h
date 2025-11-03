#if !defined(AFX_TOOLBARSDLG_H__0D5BFF09_A9A1_434C_8E64_C42F2D2CC9EA__INCLUDED_)
#define AFX_TOOLBARSDLG_H__0D5BFF09_A9A1_434C_8E64_C42F2D2CC9EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarsDlg.h : header file
//
#include "Utils\StateListBox.h"

/////////////////////////////////////////////////////////////////////////////
// CToolBarsDlg dialog

class CToolBarsDlg : public CDialog
{
// Construction
public:
	CToolBarsDlg(CWnd* pParent = NULL);   // standard constructor

    CStringArray listToolBars;
	CUIntArray listStates;

// Dialog Data
	//{{AFX_DATA(CToolBarsDlg)
	enum { IDD = IDD_TOOLBARS_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    CStateListBox m_CheckBox;
	// Generated message map functions
	//{{AFX_MSG(CToolBarsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARSDLG_H__0D5BFF09_A9A1_434C_8E64_C42F2D2CC9EA__INCLUDED_)

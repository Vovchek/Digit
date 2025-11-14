// Digit.h : main header file for the DIGIT application
//

#if !defined(AFX_DIGIT_H__67FE1E4F_CADD_4448_A196_0ACB1D7D5538__INCLUDED_)
#define AFX_DIGIT_H__67FE1E4F_CADD_4448_A196_0ACB1D7D5538__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AppDef.h"  

#include "Controls\controls.h"  

/////////////////////////////////////////////////////////////////////////////
// CDigitApp:
// See Digit.cpp for the implementation of this class
//

class CDigitApp : public CWinApp
{
public:
	CDigitApp();
	LPCTSTR GetIniFile(){return LPCTSTR(fINI);}
    int GetCurrentLang(){return intrLang;}
    bool ChangeIntrLang(int iLang=-1);
    CControls* GetControls(){return &Ctrls;}
    void AddAdvancedTemplates();
    bool TreatCmdLine(LPCTSTR CmdLine);

public:
    static int m_nOpenMsg;
	
protected:
    HINSTANCE rc_hInstance;
    int  intrLang;
    CString fINI;
    CControls Ctrls;
	
protected:
    BOOL FirstInstance(LPCTSTR CmdLine);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDigitApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle(LONG lCount);
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CDigitApp)
	afx_msg void OnAppAbout();
    afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIGIT_H__67FE1E4F_CADD_4448_A196_0ACB1D7D5538__INCLUDED__)

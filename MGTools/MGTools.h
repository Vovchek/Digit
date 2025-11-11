// MGTools.h : main header file for the MGTOOLS DLL
//

#if !defined(AFX_MGTOOLS_H__032A8067_D22A_4641_AD13_91C21133ABE9__INCLUDED_)
#define AFX_MGTOOLS_H__032A8067_D22A_4641_AD13_91C21133ABE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//#include "resource.h"		// main symbols
#include "MGToolsExport.h"

/////////////////////////////////////////////////////////////////////////////
// CMGToolsApp
// See MGTools.cpp for the implementation of this class
//

class CMGToolsApp : public CWinApp
{
public:
	CMGToolsApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMGToolsApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMGToolsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern "C" BOOL MGTOOLS_API PASCAL FilterDllMsg(LPMSG lpMsg);
extern "C" void MGTOOLS_API PASCAL ProcessDllIdle();

#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MGTOOLS_H__032A8067_D22A_4641_AD13_91C21133ABE9__INCLUDED_)

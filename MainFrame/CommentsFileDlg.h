#if !defined(AFX_CSHIFTOFFDLGBAR_H__CAF994D8_00D1_43FA_88EA_9267DDC5F102__INCLUDED_)
#define AFX_CSHIFTOFFDLGBAR_H__CAF994D8_00D1_43FA_88EA_9267DDC5F102__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CCommentsFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommentsFile dialog
#include <afxpanedialog.h>

class CCommentsFile : public CPaneDialog
{
// Construction
public:
	CCommentsFile(CWnd* pParent = NULL);   // standard constructor

	void SetComments(LPCTSTR _Comments);
    void SetScaleFactor(double _ScaleFactor);
    void SetRotation(double _Rotation);
    void GetComments(CString& _Comments);
    void GetScaleFactor(double& _ScaleFactor);
    void GetRotation(double& _Rotation);

// Dialog Data
	//{{AFX_DATA(CCommentsFile)
	enum { IDD = IDR_IMAGEINFO};
	CString Comments;
	double ScaleFactor;
	double Rotation;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommentsFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommentsFile)
	afx_msg void OnChangeComments();
	afx_msg void OnChangeScaleFactor();
	afx_msg void OnChangeRotation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSHIFTOFFDLGBAR_H__CAF994D8_00D1_43FA_88EA_9267DDC5F102__INCLUDED_)

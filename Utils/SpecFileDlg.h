#ifndef __SPECFILEDLG_H__
#define __SPECFILEDLG_H__

// SpecFileDlg.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CSpecialFileDialog dialog
#define WM_LOAD_PICTURE (WM_USER + 100)
class CSpecialFileDialog : public CFileDialog
{
public:
	CString oldFilename;
	BOOL m_bDeleteAll;
	CString m_pendingPicturePath;

// Construction
public:
	CSpecialFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(CSpecialFileDialog)
	enum { IDD = IDD_FILESPECIAL };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpecialFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnFileNameChange();
	//}}AFX_VIRTUAL

// Implementation
protected:
   void LoadPicture(LPCTSTR Filename);
   afx_msg LRESULT OnDelayedLoadPicture(WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CSpecialFileDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDelete();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __SPECFILEDLG_H__

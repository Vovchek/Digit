#if !defined(AFX_ZOOMDLG_H__61B27A57_0403_4926_9A37_37727CC1D01F__INCLUDED_)
#define AFX_ZOOMDLG_H__61B27A57_0403_4926_9A37_37727CC1D01F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZoomDlg.h : header file
//

#include "resource.h"
#include "Utils\cmdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CZoomDlg dialog

class CZoomDlg : public CMDialog
{
// Construction
public:
	CZoomDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CZoomDlg)
	enum { IDD = IDD_ZOOM_DLG };
	CComboBox	ZoomScale;
	CSliderCtrl	ZoomScaleCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	double InkZoom;
    void ApplyNow(UINT nIDEvent);
    void ResetZoomScale();

	// Generated message map functions
	//{{AFX_MSG(CZoomDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeZoomScale();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClose();
	afx_msg void OnUndo();
	afx_msg void OnReleasedcapturezoomscale(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZOOMDLG_H__61B27A57_0403_4926_9A37_37727CC1D01F__INCLUDED_)

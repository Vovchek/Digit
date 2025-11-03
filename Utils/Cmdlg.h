#if !defined(_CM_DIALOG_)
#define _CM_DIALOG_

#include "stdafx.h"

//========================= Control ==============================
void EnableItem(CDialog *pDialog, int IdBtn, BOOL State);
//=================================================================

class CMDialog : public CDialog
{
  public:
    CMDialog();
    CMDialog(UINT Id, CWnd* pParent,bool _IsPosSaved=true);
    CMDialog(LPCTSTR Id, CWnd* pParent, bool _IsPosSaved=true);
    void SetWndCap(CString _WndCap) { WndCap =_WndCap; }
    CString GetWndCap(CString _WndCap) { return WndCap; }
	void SetNameINI(LPCTSTR fn) {fnameINI=fn;}

  protected:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnCancel();
    afx_msg void OnOK();
    afx_msg void OnClose();
    virtual void SetTextCtrlColor();
    virtual BOOL OnInitDialog();
    void SavePosition();

  protected:
    COLORREF TextColor;
    bool IsPosSaved;
    UINT IDDCtrl;
    CString WndCap;
    CString fnameINI;

  private:

    DECLARE_MESSAGE_MAP()
};

#endif

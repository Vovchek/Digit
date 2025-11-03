#include "cmdlg.h"

#include "MGTools\Include\Utils\utils.h"

//=========================================================================
void EnableItem(CDialog *pDialog, int Id, BOOL State)
{
  ::EnableWindow(pDialog->GetDlgItem(Id)->GetSafeHwnd(), State);
}


BEGIN_MESSAGE_MAP(CMDialog, CDialog)
	ON_WM_CTLCOLOR()
    ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

CMDialog::CMDialog()
            :CDialog()
{
}

CMDialog::CMDialog(UINT Id, CWnd* pParent, bool _IsPosSaved)
                   : CDialog(Id, pParent)
{
    IsPosSaved = _IsPosSaved;
	fnameINI = "";
}

CMDialog::CMDialog(LPCTSTR Id, CWnd* pParent, bool _IsPosSaved)
                   : CDialog(Id, pParent)
{
    IsPosSaved = _IsPosSaved;
	fnameINI = "";
}

BOOL CMDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    CRect R; GetWindowRect(R);
    if(IsPosSaved) {
       CString Cap;
       WINDOWPLACEMENT wp;
       if(WndCap.IsEmpty())
         GetWindowText(Cap);
       else
         Cap = WndCap;
       if(ReadWndPlacement(&wp, "WINDOW_PLACEMENT", LPCTSTR(Cap),
                                         LPCTSTR(fnameINI))) {
         wp.rcNormalPosition.right = wp.rcNormalPosition.left + R.Width();
         wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + R.Height();
         SetWindowPlacement(&wp);
         ShowWindow(wp.showCmd);
       }
    }
	return TRUE;
}

void CMDialog::SavePosition()
{
  CString Cap;
  WINDOWPLACEMENT wp;
  if(WndCap.IsEmpty())
     GetWindowText(Cap);
  else
     Cap = WndCap;
	wp.length = sizeof wp;
    if(GetWindowPlacement(&wp)) {
		wp.flags = 0;
		if (IsZoomed())
			wp.flags |= WPF_RESTORETOMAXIMIZED;
        WriteWndPlacement(&wp,"WINDOW_PLACEMENT", LPCTSTR(Cap),
                                        LPCTSTR(fnameINI));
    }
}

void CMDialog::SetTextCtrlColor()
{
}

void CMDialog::OnCancel()
{
   if(IsPosSaved) SavePosition();
   CDialog::OnCancel();
}

void CMDialog::OnOK()
{
   if(IsPosSaved) SavePosition();
   CDialog::OnOK();
}

void CMDialog::OnClose()
{
   if(IsPosSaved) SavePosition();
   CDialog::OnClose();
}

HBRUSH CMDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    IDDCtrl = ::GetDlgCtrlID(pWnd->GetSafeHwnd());
/*    if (nCtlColor == CTLCOLOR_STATIC || CTLCOLOR_EDIT || CTLCOLOR_LISTBOX) {
       SetTextCtrlColor();
       pDC->SetTextColor(TextColor);
    }*/
    return hbr;
}

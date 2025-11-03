// ApproxSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "digit.h"
#include "ApproxSetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CApproxSetDlg dialog


CApproxSetDlg::CApproxSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CApproxSetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApproxSetDlg)
	Eps = 0.0;
	Pow = 0;
	//}}AFX_DATA_INIT
}


void CApproxSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApproxSetDlg)
	DDX_Text(pDX, IDE_approx_dist, Eps);
	DDX_Text(pDX, IDE_approx_pow, Pow);
	DDV_MinMaxInt(pDX, Pow, 1, 8);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApproxSetDlg, CDialog)
	//{{AFX_MSG_MAP(CApproxSetDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApproxSetDlg message handlers

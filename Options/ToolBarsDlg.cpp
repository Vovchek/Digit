// ToolBarsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ToolBarsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarsDlg dialog


CToolBarsDlg::CToolBarsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToolBarsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CToolBarsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CToolBarsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToolBarsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolBarsDlg, CDialog)
	//{{AFX_MSG_MAP(CToolBarsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBarsDlg message handlers

BOOL CToolBarsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  VERIFY(m_CheckBox.SubclassDlgItem(IDCB_Toolbar_list, this));

  m_CheckBox.SetListItems(listToolBars, listStates);
	
  return TRUE; 
}

void CToolBarsDlg::OnOK() 
{
	UpdateData(TRUE);
	m_CheckBox.GetStateList(listStates);
	
	CDialog::OnOK();
}

void CToolBarsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

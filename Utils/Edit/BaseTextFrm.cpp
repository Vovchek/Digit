// BaseTextFrm.cpp : implementation file
//

#include "StdAfx.h"
#include "BaseTextFrm.h"
#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseTextFrm

IMPLEMENT_DYNCREATE(CBaseTextFrm, CFrameWnd)

HWND CBaseTextFrm::hwndParent=NULL;

CBaseTextFrm::CBaseTextFrm()
{
  pEditView = NULL;
}

CBaseTextFrm::~CBaseTextFrm()
{
}

BEGIN_MESSAGE_MAP(CBaseTextFrm, CFrameWnd)
	//{{AFX_MSG_MAP(CBaseTextFrm)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseTextFrm message handlers

int CBaseTextFrm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CBaseTextFrm::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	// TODO: Add your message handler code here
}

void CBaseTextFrm::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
}

BOOL CBaseTextFrm::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.hwndParent = hwndParent;

    WINDOWPLACEMENT wp;
    if(ReadWndPlacement(&wp,"TEXTINFO","PLACEMENT", GetIniFile())){
	  cs.x = wp.rcNormalPosition.left;	
	  cs.y = wp.rcNormalPosition.top;	
	  cs.cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	  cs.cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	}
	
	return CFrameWnd::PreCreateWindow(cs);
}


BOOL CBaseTextFrm::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CBaseTextFrm::SavePlacement()
{
  WINDOWPLACEMENT wp;
  wp.length = sizeof wp;
  if (GetWindowPlacement(&wp))
    WriteWndPlacement(&wp,"TEXTINFO","PLACEMENT", GetIniFile());
}

void CBaseTextFrm::PostNcDestroy() 
{
	CFrameWnd::PostNcDestroy();
}

void CBaseTextFrm::OnClose() 
{
	SavePlacement();

	CControls* pCtrls = GetControls();
//    pCtrls->EnableOptions &= ~I_INFO_WINDOW;

   CBaseTextDoc* pDoc = (CBaseTextDoc*)GetActiveDocument();

	CFrameWnd::OnClose();
    ::SetFocus(hwndParent);
}

// TextChildFrm.cpp : implementation file
//

#include "stdafx.h"
#include "TextChildFrm.h"
#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrm

IMPLEMENT_DYNCREATE(CTextChildFrm, CMDIChildWnd)

CTextChildFrm::CTextChildFrm()
{
	IniName.Empty();
}

CTextChildFrm::~CTextChildFrm()
{
}

void CTextChildFrm::SetIniName(LPCTSTR _IniName)
{
	IniName = "TEXTFRAME";
}

BEGIN_MESSAGE_MAP(CTextChildFrm, CMDIChildWnd)
	//{{AFX_MSG_MAP(CTextChildFrm)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrm message handlers

void CTextChildFrm::ActivateFrame(int nCmdShow) 
{
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

BOOL CTextChildFrm::PreCreateWindow(CREATESTRUCT& cs) 
{
    WINDOWPLACEMENT wp;
    if(ReadWndPlacement(&wp,LPCTSTR(IniName),"PLACEMENT", GetIniFile())){
	  cs.cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	  cs.cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	}
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

int CTextChildFrm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CTextChildFrm::SavePlacement()
{
  WINDOWPLACEMENT wp;
  wp.length = sizeof wp;
  if (GetWindowPlacement(&wp))
    WriteWndPlacement(&wp,LPCTSTR(IniName),"PLACEMENT", GetIniFile());
}

void CTextChildFrm::OnClose() 
{
	SavePlacement();
	CMDIChildWnd::OnClose();
}

void CTextChildFrm::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
}

void CTextChildFrm::OnMove(int x, int y) 
{
	CMDIChildWnd::OnMove(x, y);
}


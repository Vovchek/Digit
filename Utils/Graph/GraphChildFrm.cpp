// GraphChildFrm.cpp : implementation file
//
//C:\Ilya\Programming\cpp\Numbering\Utils\Graph\GraphChildFrm.cpp
#include "StdAfx.h"
#include "GraphChildFrm.h"
#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphChildFrm

IMPLEMENT_DYNCREATE(CGraphChildFrm, CMDIChildWnd)

CGraphChildFrm::CGraphChildFrm()
{
	IniName.Empty();
}

CGraphChildFrm::~CGraphChildFrm()
{
}

void CGraphChildFrm::SetIniName(LPCTSTR _IniName)
{
	IniName = _IniName;
}

BEGIN_MESSAGE_MAP(CGraphChildFrm, CMDIChildWnd)
	//{{AFX_MSG_MAP(CGraphChildFrm)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphChildFrm message handlers

void CGraphChildFrm::ActivateFrame(int nCmdShow) 
{
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

BOOL CGraphChildFrm::PreCreateWindow(CREATESTRUCT& cs) 
{
    WINDOWPLACEMENT wp;
	if(!IniName.IsEmpty()){
		if(ReadWndPlacement(&wp,LPCTSTR(IniName),"PLACEMENT", GetIniFile())){
			cs.cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			cs.cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		}
	}
	
	return CMDIChildWnd::PreCreateWindow(cs);
}

int CGraphChildFrm::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CGraphChildFrm::SavePlacement()
{
	if(!IniName.IsEmpty()){
		WINDOWPLACEMENT wp;
		wp.length = sizeof wp;
		if (GetWindowPlacement(&wp))
			WriteWndPlacement(&wp,LPCTSTR(IniName),"PLACEMENT", GetIniFile());
	}
}

void CGraphChildFrm::OnClose() 
{
	SavePlacement();
	CMDIChildWnd::OnClose();
}

void CGraphChildFrm::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
}

void CGraphChildFrm::OnMove(int x, int y) 
{
	CMDIChildWnd::OnMove(x, y);
}


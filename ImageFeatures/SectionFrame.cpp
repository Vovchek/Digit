// SectionFrame.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SectionFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSectionFrame

IMPLEMENT_DYNCREATE(CSectionFrame, CMDIChildWnd)

CSectionFrame::CSectionFrame()
{
}

CSectionFrame::~CSectionFrame()
{
}

void CSectionFrame::SetRedrawOnClose(CView* pV)
{
	pRedrawView = pV;
}

BEGIN_MESSAGE_MAP(CSectionFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CSectionFrame)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSectionFrame message handlers
BOOL CSectionFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle &= ~WS_EX_TOPMOST; 
	return CMDIChildWnd::PreCreateWindow(cs);
}

int CSectionFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    CRect clientRect;
    GetClientRect(&clientRect);

     pGr = new C2DGraph(NULL);
     pGr->Create(NULL,"",WS_VISIBLE|WS_CHILD, clientRect, this, WND_2D_GRAPH);
     SetActiveView(pGr);

	return 0;
}

BOOL CSectionFrame::DestroyWindow()
{
    if(pRedrawView && pRedrawView->m_hWnd)
       pRedrawView->Invalidate(FALSE);
	return CMDIChildWnd::DestroyWindow();
}

void CSectionFrame::OnClose()
{
	CMDIChildWnd::OnClose();
}

void CSectionFrame::OnDestroy()
{
	CMDIChildWnd::OnDestroy();
}

void CSectionFrame::PostNcDestroy()
{
    this->m_hWnd = NULL;
}


void CSectionFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);

    CRect clientRect;
    GetClientRect(&clientRect);
    CalcWindowRect(&clientRect);

    CSize cs;
    cs.cx = clientRect.Width();
    cs.cy = clientRect.Height();
    if(pGr){
      pGr->SetWindowPos(&(CWnd::wndTop), 0, 0, cs.cx, cs.cy,
               SWP_NOMOVE);
      pGr->RefreshPicture();
    }
}

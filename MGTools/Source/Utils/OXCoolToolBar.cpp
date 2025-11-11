// OXCoolToolBar.cpp : implementation file
//

#include "stdafx.h"
#include <stdlib.h>

#include "Include\Utils\OXCoolToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int ID_OXGRIPPER_WIDTH=7;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COXCoolToolBar idle update through COXCoolToolBarCmdUI class. 

void COXCoolToolBarCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	COXCoolToolBar* pCoolToolBar = (COXCoolToolBar*)m_pOther;
	CToolBarCtrl* pToolBarCtrl = &pCoolToolBar->GetToolBarCtrl();
	ASSERT(pToolBarCtrl != NULL);
	
	ASSERT(m_nIndex < m_nIndexMax);
	
	// Get toolbar button state
	TBBUTTON TB;
	pToolBarCtrl->GetButton(m_nIndex, &TB);
	UINT nNewState = TB.fsState & ~TBSTATE_ENABLED;
	if (bOn)
		nNewState |= TBSTATE_ENABLED;
	if (nNewState != TB.fsState)
		pToolBarCtrl->SetState(m_nID, nNewState);
}


void COXCoolToolBarCmdUI::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
	COXCoolToolBar* pCoolToolBar = (COXCoolToolBar*)m_pOther;
	CToolBarCtrl* pToolBarCtrl = &pCoolToolBar->GetToolBarCtrl();
	ASSERT(pToolBarCtrl != NULL);
	
	ASSERT(m_nIndex < m_nIndexMax);
	
	// Get toolbar button state
	TBBUTTON TB;
	pToolBarCtrl->GetButton(m_nIndex, &TB);	
	UINT nNewState = TB.fsState & ~ (TBSTATE_CHECKED | TBSTATE_INDETERMINATE);
	
	if (nCheck == 1)
		nNewState |= TBSTATE_CHECKED;
	else
		if (nCheck == 2)
		nNewState |= TBSTATE_INDETERMINATE;
	
	if (nNewState != TB.fsState)
		pToolBarCtrl->SetState(m_nID, nNewState);
	
	// should we set the button style too ?
	//	pToolBarCtrl->_SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX); 
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COXCoolToolBar
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

DWORD COXCoolToolBar::m_dwComCtlVersion=0;

IMPLEMENT_DYNAMIC(COXCoolToolBar,CToolBar)

COXCoolToolBar::COXCoolToolBar():CToolBar()
{
#if _MFC_VER>=0x0420
	m_bCool=FALSE;
	// by default draw Gripper and separators in flat mode
	m_bGripper=TRUE;
	m_bSeparator=TRUE; 

	m_nIndent=0;
	
	// means not set
	m_sizeMinMaxWidth.cx=-1;
	m_sizeMinMaxWidth.cy=-1;

	m_crDefaultTextColor=::GetSysColor(COLOR_BTNTEXT);
	m_crHotTextColor=::GetSysColor(COLOR_BTNTEXT);

	m_bDropDownArrow=FALSE;
#endif

	m_ttID=TTID_NOTSET;

	m_pBitmapIds=NULL;
	m_nBitmapButtons=0;

	m_bWindowsNTRunning=IsWindowsNTRunning();

	if(m_dwComCtlVersion==0)
	{
		DWORD dwMajor, dwMinor;
		if(SUCCEEDED(GetComCtlVersion(&dwMajor, &dwMinor)))
		{
			m_dwComCtlVersion=MAKELONG((WORD)dwMinor, (WORD)dwMajor);
		}
		else
		{
			// assume that neither IE 3.0 nor IE 4.0 installed
			m_dwComCtlVersion=0x00040000;
		}
	}

	m_nDropDownArrowWidth=GetDropDownArrowWidth();

	m_hIcon=NULL;
	m_iconRect.SetRectEmpty();

    m_bPrevFloating = 3;            // neither TRUE not FALSE;

	m_dwPrevDockSide=(DWORD)-1;			// none of side
}

COXCoolToolBar::~COXCoolToolBar()
{
	LPTSTR lpszResourceName;

	// delete all bitmaps that we associated with CoolToolBar
	HBITMAP hBitmap;
	POSITION pos=m_allBitmaps.GetStartPosition();
	while(pos!=NULL)
	{
		m_allBitmaps.GetNextAssoc(pos,lpszResourceName,hBitmap);
		::DeleteObject(hBitmap);
	}
	m_allBitmaps.RemoveAll();

	// delete all image lists that we associated with CoolToolBar
	HANDLE hImageList;
	pos=m_allImageLists.GetStartPosition();
	while(pos!=NULL)
	{
		m_allImageLists.GetNextAssoc(pos,lpszResourceName,hImageList);
		ImageList_Destroy((HIMAGELIST)hImageList);
	}
	m_allImageLists.RemoveAll();
}

BEGIN_MESSAGE_MAP(COXCoolToolBar, CToolBar)
	//{{AFX_MSG_MAP(COXCoolToolBar)
	ON_WM_NCCREATE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCCALCSIZE()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP

	// reflect messages to make customization work
	ON_NOTIFY_REFLECT(TBN_QUERYINSERT, OnTBNQueryInsert)
	ON_NOTIFY_REFLECT(TBN_QUERYDELETE, OnTBNQueryDelete)
	ON_NOTIFY_REFLECT(TBN_BEGINADJUST, OnTBNBeginAdjust)
	ON_NOTIFY_REFLECT(TBN_TOOLBARCHANGE, OnTBNToolBarChange)
	ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnTBNGetButtonInfo)

#if _MFC_VER>=0x0420
	// reflect message to provide custom draw functionality
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDraw)
#endif

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COXCoolToolBar message handlers

BOOL COXCoolToolBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
//#if _WIN32_IE>=0x0400
	if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
	{
		ASSERT_VALID(pParentWnd);   // must have a parent
		ASSERT (!((dwStyle & CBRS_SIZE_FIXED) && (dwStyle & CBRS_SIZE_DYNAMIC)));

#if _MFC_VER<=0x0421
		// save the style
		m_dwStyle = dwStyle;
		if (nID == AFX_IDW_TOOLBAR)
			m_dwStyle |= CBRS_HIDE_INPLACE;

		dwStyle &= ~CBRS_ALL;
		dwStyle |= CCS_NOPARENTALIGN|CCS_NODIVIDER|CCS_NORESIZE;

		// by default set flat style
		dwStyle|=TBSTYLE_FLAT;

		// Initialize bar common controls
		static BOOL bInitCoolToolBar = FALSE;
		if (!bInitCoolToolBar) 
		{
			INITCOMMONCONTROLSEX sex;
			sex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			sex.dwICC = ICC_BAR_CLASSES;
			InitCommonControlsEx(&sex);
			bInitCoolToolBar = TRUE;
		}

		// Create the CoolToolBar using style and parent.
		CRect rc;
		rc.SetRectEmpty();
		if(CWnd::CreateEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
			dwStyle, rc.left, rc.top, rc.Width(), rc.Height(), 
			pParentWnd->m_hWnd, (HMENU)nID))
#else
		if(CToolBar::Create(pParentWnd, dwStyle, nID))
#endif
		{
#if _MFC_VER>0x0421
			// by default set flat style
			ModifyStyle(0,TBSTYLE_FLAT|TBSTYLE_TRANSPARENT);
#else
			SendMessage(TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON));
#endif
			if(pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
				VERIFY(ModifyStyle(TBSTYLE_TRANSPARENT,0));
			return TRUE;
		}
		else
			return FALSE;
	}
//#else
	else
	{
		if(!CToolBar::Create(pParentWnd,dwStyle,nID))
			return FALSE;

#if _MFC_VER>=0x0420
		// by default set flat style
		SetFlat();
#endif //_MFC_VER>=0x0420
	}
//#endif //_WIN32_IE>=0x0400

	return TRUE;
}

BOOL COXCoolToolBar::OnNcCreate(LPCREATESTRUCT lpCreateStruct) 
{
#if _MFC_VER>=0x0420
	if(IsCool())
	{
		// bypass CToolBar/CControlBar
		return (BOOL)Default();
	}
	else
	{
		return CToolBar::OnNcCreate(lpCreateStruct);
	}
#else
	return CToolBar::OnNcCreate(lpCreateStruct);
#endif
}

void COXCoolToolBar::OnNcPaint()
{
	EraseNonClient();

	CWindowDC dc(this);
	CRect rect(0,0,0,0);
	GetWindowRect(rect);
	ScreenToClient(rect);
	rect.OffsetRect(-rect.left, -rect.top);

#if _MFC_VER>=0x0420
	if(IsCool())
	{
		// bypass CToolBar/CControlBar
		Default();	
	}
	else
	{
		if(IsFlat() && m_bGripper)
			DrawGripper(dc,rect);
	}
#endif

	if(m_hIcon!=NULL)
		DrawIcon(dc,rect);

	DrawInBookedSpace(dc,rect);
}

void COXCoolToolBar::OnPaint()
{
	// CToolBar has some problems with painting when CToolBar is dockable
	// (due to a bug in comctl32.dll version 4.70 which is supplied with IE 3.0)
	CRect rectUpdate(0,0,0,0);
	GetUpdateRect(&rectUpdate,TRUE);

#if _MFC_VER>=0x0420
	if(IsCool())
	{
		// bypass CToolBar/CControlBar
		Default();	
	}
	else
	{
		if(IsFloating())
		{
			CToolBar::OnPaint();
		}
		else
		{
			Default();	
			if(!IsFlat())
			{
				InvalidateRect(&rectUpdate);
				CToolBar::OnPaint();
			}
		}
	}
#else
	if(IsFloating())
	{
		CToolBar::OnPaint();
	}
	else
	{
		Default();	
		InvalidateRect(&rectUpdate);
		CToolBar::OnPaint();
	}
#endif
}

void COXCoolToolBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
#if _MFC_VER>=0x0420
	if(IsCool())
	{
		// bypass CToolBar/CControlBar
		Default();	

		// Change the non-client area of CoolToolBar
		// to make it look pretty in CoolBar
		int nAddIntension=IsFlat() ? 2 : 0;
		if(m_dwStyle&CBRS_ORIENT_HORZ) 
		{
			lpncsp->rgrc[0].top+=nAddIntension;
			lpncsp->rgrc[0].bottom+=nAddIntension;
		} 
		else 
		{
			lpncsp->rgrc[0].left+=nAddIntension;
			lpncsp->rgrc[0].right+=nAddIntension;
		}
	}
	else
	{
		CToolBar::OnNcCalcSize(bCalcValidRects, lpncsp);
		if(IsFlat())
		{
			// adjust non-client area
#if _MFC_VER<=0x0421
			lpncsp->rgrc[0].top+=2;
			lpncsp->rgrc[0].bottom+=2;
#endif
			// adjust non-client area for gripper at left or top
			if(m_dwStyle&CBRS_ORIENT_HORZ) 
			{
				if(IsGripper() && !(m_dwStyle & CBRS_FLOATING))
				{
					lpncsp->rgrc[0].left+=ID_OXGRIPPER_WIDTH;
//					lpncsp->rgrc[0].right+=ID_OXGRIPPER_WIDTH;
				}
			} 
			else 
			{
				if(IsGripper() && !(m_dwStyle & CBRS_FLOATING))
				{
					lpncsp->rgrc[0].top+=ID_OXGRIPPER_WIDTH;
//					lpncsp->rgrc[0].bottom+=ID_OXGRIPPER_WIDTH;
				}
			}
		}
	}
#else
	CToolBar::OnNcCalcSize(bCalcValidRects, lpncsp);
#endif

	if(m_hIcon!=NULL)
	{
		// adjust non-client area for icon at left or top
		if(m_dwStyle&CBRS_ORIENT_HORZ) 
		{
			int nIconWidth=::GetSystemMetrics(SM_CXSMICON);
			lpncsp->rgrc[0].left+=nIconWidth+2;
		} 
		else 
		{
			int nIconHeight=::GetSystemMetrics(SM_CYSMICON);
			lpncsp->rgrc[0].top+=nIconHeight+2;
		}
	}

	CRect rectBookedSpace(0,0,0,0);
	BookSpace(rectBookedSpace,(m_dwStyle&CBRS_ORIENT_HORZ) ? LM_HORZ : 0);
	lpncsp->rgrc[0].left+=rectBookedSpace.left;
	lpncsp->rgrc[0].right-=rectBookedSpace.right;
	lpncsp->rgrc[0].top+=rectBookedSpace.top;
	lpncsp->rgrc[0].bottom-=rectBookedSpace.bottom;
}

void COXCoolToolBar::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
#if _MFC_VER>=0x0420
	if(IsCool())
	{
		// bypass CToolBar/CControlBar
		Default();	
	}
	else
	{
		CToolBar::OnWindowPosChanging(lpwndpos);
	}
	
	// TODO: Add your message handler code here
	if (IsFlat() && !(lpwndpos->flags & SWP_NOMOVE)) 
	{										 // if moved:
		CRect rc;							 //   Fill rectangle with..
		GetWindowRect(&rc);					 //   ..my (toolbar) rectangle.
		CWnd* pParent = GetParent();		 //   get parent (dock bar/frame) win..
		pParent->ScreenToClient(&rc);		 //   .. and convert to parent coords

		// Ask parent window to paint the area beneath my old location.
		// Typically, this is just solid grey.
		//
		pParent->InvalidateRect(&rc);		 // paint old rectangle

		// Now paint my non-client area at the new location.
		// This is the extra bit of border space surrounding the buttons.
		// Without this, you will still have a partial display bug
		//
		if(m_bWindowsNTRunning)
		{
			// we need this code on NT systems 
			// because we've got problem with redrawing
			CRect rect;
			GetWindowRect(&rect);
			// redraw 
			SetWindowPos(NULL,0,0,rect.Width(),rect.Height(),
				SWP_NOMOVE|SWP_NOZORDER|SWP_DRAWFRAME|
				SWP_FRAMECHANGED|SWP_NOREDRAW);
		}
		else
		{
			PostMessage(WM_NCPAINT);
		}
	}
#else
	CToolBar::OnWindowPosChanging(lpwndpos);
#endif
}

void COXCoolToolBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	// TODO: Add your message handler code here
	
	if(m_pDockContext!=NULL)
		m_pDockContext->m_bDragging=FALSE;

    BOOL bFloating=IsFloating();

	if(bFloating!=m_bPrevFloating)
	{
		m_bPrevFloating=bFloating;
		OnFloatingDocking(bFloating);
	}

	DWORD dwDockSide=GetBarStyle()&CBRS_ALIGN_ANY;
	if(m_dwPrevDockSide!=dwDockSide)
	{
		m_dwPrevDockSide=dwDockSide;
		OnChangeDockSide(m_dwPrevDockSide);
	}

	RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME);
	
	CToolBar::OnWindowPosChanged(lpwndpos);
}

void COXCoolToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	COXCoolToolBarCmdUI state;
	state.m_pOther = this;
	
	state.m_nIndexMax = GetToolBarCtrl().GetButtonCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		// get buttons state
		TBBUTTON TB;
		GetToolBarCtrl().GetButton(state.m_nIndex, &TB);
		state.m_nID = TB.idCommand;

		// ignore separators
		if (!(TB.fsStyle & TBSTYLE_SEP))
		{
			// allow the toolbar itself to have update handlers
			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
				continue;

			// allow the owner to process the update
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	// update any dialog controls added to the toolbar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void COXCoolToolBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	UNREFERENCED_PARAMETER(uFlags);
	UNREFERENCED_PARAMETER(lpszSection);

	m_nDropDownArrowWidth=GetDropDownArrowWidth();
	RedrawToolBar();

	CToolBar::OnSettingChange(uFlags, lpszSection);
}

#if _MFC_VER>=0x0420
////////////////////
void COXCoolToolBar::EraseNonClient()
{
	// get window DC that is clipped to the non-client area
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);

	// draw borders in non-client area
	if(!IsCool())
	{
		// only if CoolToolBar is not used in CoolBar
		DrawBorders(&dc, rectWindow);
		// fixing for spurious edges
		if((m_dwStyle&CBRS_BORDER_3D)==0)
		{
			rectWindow.InflateRect(1,1);
		}
	}

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	dc.ExcludeClipRect(rectClient);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);
}

void COXCoolToolBar::DrawGripper(CDC& dc, CRect& rect)
{ 
    if(m_dwStyle & CBRS_FLOATING)
		return;		// no gripper if floating

	COLORREF clrHilight=::GetSysColor(COLOR_BTNHILIGHT);
	COLORREF clrShadow=::GetSysColor(COLOR_BTNSHADOW);
	
	CRect gripperRect=rect;

	if(m_dwStyle&CBRS_ORIENT_HORZ) 
	{
		gripperRect.DeflateRect(1,1);
		gripperRect.OffsetRect(0,1);
		// gripper at left
		if(m_pDockContext!=NULL)
		{
			// EnableDocking was called
			gripperRect.left+=2;
			gripperRect.bottom-=2;
		}
		else
		{
			if(m_dwStyle & CBRS_BORDER_BOTTOM)
			{
				gripperRect.top-=2;
				gripperRect.bottom-=2;
			}
		}
		gripperRect.right=gripperRect.left+3;
        dc.Draw3dRect(gripperRect,clrHilight,clrShadow);
		gripperRect.OffsetRect(3,0);
        dc.Draw3dRect(gripperRect,clrHilight,clrShadow);

		rect.left=gripperRect.right+1;
	} 
	else 
	{
		gripperRect.DeflateRect(0,1,2,1);
		gripperRect.OffsetRect(0,1);
		// gripper at top
		if(m_pDockContext==NULL)
		{
			// EnableDocking was not called
			gripperRect.right+=2;
		}
		else
		{
			gripperRect.left+=2;
		}
		gripperRect.bottom=gripperRect.top+3;
		dc.Draw3dRect(gripperRect,clrHilight,clrShadow);
		gripperRect.OffsetRect(0,3);
        dc.Draw3dRect(gripperRect,clrHilight,clrShadow);

		rect.top=gripperRect.bottom+1;
	}

}

void COXCoolToolBar::DrawSeparator(CDC& dc, CRect& rc)
{
	COLORREF clrHilight=::GetSysColor(COLOR_BTNHILIGHT);
	COLORREF clrShadow=::GetSysColor(COLOR_BTNSHADOW);
	
    BOOL bHorz=((m_dwStyle&CBRS_ORIENT_HORZ)!=0) ? TRUE : FALSE;
	// make sure, this separator is not a placeholder for
	// another control.
	if(rc.Width()<=8) 
	{
		if(bHorz) 
		{
			// draw the separator bar in the middle
			int x=(rc.left+rc.right)/2;
			rc.left=x-1;
			rc.right=x+1;
			dc.Draw3dRect(rc,clrShadow,clrHilight);
		} 
		else 
		{
			// draw the separator bar in the middle
			UINT nButtonWidth=GetButtonsSize().cx;
			rc.left=rc.left-nButtonWidth;
			rc.right=rc.left+nButtonWidth;
			rc.top=rc.bottom+1;
			rc.bottom=rc.top+3;
			int y=(rc.top+rc.bottom)/2;
			rc.top=y-1;
			rc.bottom=y+1;
			dc.Draw3dRect(rc,clrShadow,clrHilight);
		}
	}
}

void COXCoolToolBar::DrawIcon(CDC& dc, CRect& rect)
{ 
	if(m_hIcon==NULL)
		return;

	m_iconRect=rect;

	if(m_dwStyle&CBRS_ORIENT_HORZ) 
	{
		if(m_iconRect.left==0)
			m_iconRect.left+=5;

		m_iconRect.right=m_iconRect.left+::GetSystemMetrics(SM_CXSMICON);
		m_iconRect.top=(rect.Height()-::GetSystemMetrics(SM_CYSMICON))/2;
		m_iconRect.bottom=m_iconRect.top+::GetSystemMetrics(SM_CYSMICON);

		rect.left=m_iconRect.right+1;
	} 
	else 
	{
		if(m_iconRect.top==0)
			m_iconRect.top+=5;

		m_iconRect.bottom=m_iconRect.top+::GetSystemMetrics(SM_CYSMICON);
		m_iconRect.left=(rect.Width()-::GetSystemMetrics(SM_CXSMICON))/2;
		m_iconRect.right=m_iconRect.left+::GetSystemMetrics(SM_CXSMICON);

		rect.top=m_iconRect.bottom+1;
	}

	::DrawIconEx(dc,m_iconRect.left,m_iconRect.top,m_hIcon,
		m_iconRect.Width(),m_iconRect.Height(),0,NULL,DI_NORMAL);

}

void COXCoolToolBar::SetDropDownArrow(BOOL bDropDownArrow) 
{ 
	if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
	{
		DWORD dwStyleEx=GetStyleEx();
		dwStyleEx=bDropDownArrow ? dwStyleEx|TBSTYLE_EX_DRAWDDARROWS : 
			dwStyleEx&~TBSTYLE_EX_DRAWDDARROWS;
		SetStyleEx(dwStyleEx); 
	}

	m_bDropDownArrow=bDropDownArrow;
}

BOOL COXCoolToolBar::IsDropDownArrow() const
{ 
	if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
	{
		return GetStyleEx()&TBSTYLE_EX_DRAWDDARROWS ? TRUE : FALSE; 
	}

	return m_bDropDownArrow;
}

////////////////////
#endif

void COXCoolToolBar::SetCustomizable(BOOL bCustomizable)
{
	// set CoolToolBar customizable only if array of bitmap IDs was previously set 
	// by function SetBitmapIds()

	if(m_pBitmapIds==NULL)
	{
		bCustomizable=FALSE;
	}
	
	if(bCustomizable)
	{
		ModifyStyle(0,CCS_ADJUSTABLE);
	}
	else
	{
		ModifyStyle(CCS_ADJUSTABLE,0);
	}
}

void COXCoolToolBar::SetBitmapIds(UINT* pIds, int nButtons)
{
	m_pBitmapIds=pIds;
	m_nBitmapButtons=nButtons;

	// if we call this function then we probably want our CoolToolBar 
	// to be customizable
	SetCustomizable();
}


int COXCoolToolBar::FindBitmapIndex(UINT nID) const
{
	// helper function to find the number of element in array of bitmap IDs
	// by its ID
	ASSERT(m_pBitmapIds != NULL);
	for (int i = 0; i < m_nBitmapButtons ; i++)
	{
		if (m_pBitmapIds[i] == (int)nID)
			return i;
	}
	return -1;
}



// Return information for bitmap indexes in the toolbar
afx_msg void COXCoolToolBar::OnTBNGetButtonInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	TBNOTIFY * pTBN = (TBNOTIFY *)pNMHDR;
	
	int nIndex = pTBN->iItem;
	if (nIndex < m_nBitmapButtons)
	{
		*pResult = TRUE;
		
		UINT nButtonId = m_pBitmapIds[nIndex];
		pTBN->tbButton.iBitmap = nIndex;
		pTBN->tbButton.idCommand = nButtonId;
		pTBN->tbButton.fsState = TBSTATE_ENABLED;
		pTBN->tbButton.fsStyle = TBSTYLE_BUTTON;
		pTBN->tbButton.iString = 0;		
		// use as tooltip as text associated with button shown in Customize dialog
		if (pTBN->pszText != NULL)
		{		
			CString strText;
			if (strText.LoadString(nButtonId))
			{
				LPCTSTR pTipText = _tcschr(strText, _T('\n'));   // tool tip is after "\n" in the string
				if (pTipText != NULL)
				{	
					_tcsncpy(pTBN->pszText, pTipText + 1, pTBN->cchText);
					return;
				}
			}
			TRACE1("COXCoolToolBar:No Tooltip prompt for ID=%d\n", nButtonId);
			_tcsncpy(pTBN->pszText, _T("???"), pTBN->cchText);
		}
	}
	else 
		*pResult = FALSE;
}


afx_msg void COXCoolToolBar::OnTBNBeginAdjust(NMHDR* /* pNMHDR */, LRESULT* /* pResult */)
{
}

afx_msg void COXCoolToolBar::OnTBNQueryInsert(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = TRUE;		// always allow buttons to be inserted
}


afx_msg void COXCoolToolBar::OnTBNQueryDelete(NMHDR* /* pNMHDR */, LRESULT* pResult)
{
	*pResult = TRUE;		// always allow buttons to be deleted
}

afx_msg void COXCoolToolBar::OnTBNToolBarChange(NMHDR* /* pNMHDR */, LRESULT* /* pResult */)
{
	SetButtonTextFromID(m_ttID);
}

#if _MFC_VER>=0x0420
//////////////////////
// custom draw of a toolbar is available since MFC 4.2
// in you derived class you can provide your own custom draw routines
afx_msg void COXCoolToolBar::OnCustomDraw(NMHDR* pNotify, LRESULT* pResult)
{
	// in new version of comctl32.dll(4.71)
	// we use toolbar specific custom draw structure 
	LPNMTBCUSTOMDRAW pTBCDRW=NULL;
	LPNMCUSTOMDRAW pCDRW=NULL;
	if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
	{
		pTBCDRW=(LPNMTBCUSTOMDRAW)pNotify;
		pCDRW=&pTBCDRW->nmcd;
	}
	else
	{
		pCDRW=(LPNMCUSTOMDRAW)pNotify;
	}

	OXCTB_CUSTOMDRAW cd;
	if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
	{
		ASSERT(pTBCDRW!=NULL);
		cd.pCustomDraw=(LPNMCUSTOMDRAW)pTBCDRW;
	}
	else
	{
		cd.pCustomDraw=pCDRW;
	}
	cd.hdr.hwndFrom=pCDRW->hdr.hwndFrom;
	cd.hdr.idFrom=pCDRW->hdr.idFrom;
	cd.hdr.code=OXCTBN_CUSTOMDRAW;
	cd.result=CDRF_DODEFAULT;
	cd.bDoDefault=TRUE;

	CWnd* pParentWnd=GetParent();
	ASSERT(pParentWnd);

	pParentWnd->SendMessage(WM_NOTIFY,cd.hdr.idFrom,(LPARAM)&cd);

	if(pCDRW->dwDrawStage==CDDS_PREPAINT && 
		GetToolBarCtrl().GetButtonCount()>0)
	{
		CRect rect;
		GetItemRect(0,&rect);
	}

	if(!cd.bDoDefault)
	{
		*pResult=cd.result;
	}
	else
	{
		*pResult = CDRF_DODEFAULT;
		switch( pCDRW->dwDrawStage )
		{
		case CDDS_PREPAINT:
			{
				/////////
				// draw separators
				/////////
				// in new version of comctl32.dll(4.71)
				// separators are drawn automatically
				// well, you have a chance to draw your own separator even
				// if new version of comctl32.dll installed. But you shouldn't 
				// forget that native separators will be drawn anyway
				if(m_dwComCtlVersion<_IE40_COMCTL_VERSION)
				{
					if(IsFlat() && m_bSeparator)
					{
						CDC dc;
						dc.Attach(pCDRW->hdc);
						int nBitmap; 
						UINT uID, uStyleState;
						CRect rc;
						for(int nIndex=0; nIndex<GetToolBarCtrl().GetButtonCount(); nIndex++)
						{
							GetButtonInfo(nIndex, uID, uStyleState, nBitmap);
							WORD wStyle=LOWORD(uStyleState);
							WORD wState=HIWORD(uStyleState);
							if(wStyle==TBSTYLE_SEP && wState!=TBSTATE_HIDDEN) 
							{
								if(!(wState&TBSTATE_WRAP))
								{
									GetItemRect(nIndex,&rc);
									DrawSeparator(dc,rc);
								}
								else
								{
									CBrush brush(::GetSysColor(COLOR_BTNFACE));
									dc.FillRect(&rc,&brush);
								}
							}
						}
						dc.Detach();
					}
				}
				*pResult=CDRF_DODEFAULT|CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYITEMDRAW;
				break;
			}

		case CDDS_ITEMPREPAINT:
			{
				*pResult=CDRF_NOTIFYPOSTPAINT;	// request for ITEMPOSTPAINT
 
				// set colors of text in default and/or hot mode if needed
				if(!GetToolBarCtrl().IsButtonHidden(pCDRW->dwItemSpec) && 
					IsFlat()) 
				{
					if(m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
					{
						if(m_crDefaultTextColor!=::GetSysColor(COLOR_BTNTEXT))
						{
							pTBCDRW->clrText=m_crDefaultTextColor;
						}
						if(pCDRW->uItemState&CDIS_HOT)
						{
							if(m_crHotTextColor!=m_crDefaultTextColor)
							{
								pTBCDRW->clrText=m_crHotTextColor;
							}
						}
					}
					else
					{
						if(m_crDefaultTextColor!=::GetSysColor(COLOR_BTNTEXT))
						{
							::SetTextColor(pCDRW->hdc, m_crDefaultTextColor);
						}

						if(m_crHotTextColor!=m_crDefaultTextColor)
						{
							if(pCDRW->uItemState&CDIS_HOT)
							{
								::SetTextColor(pCDRW->hdc, m_crHotTextColor);
							}
						}
					}

				}

				break;
			}
		case CDDS_ITEMPOSTPAINT:
			{
				int nIndex=CommandToIndex(pCDRW->dwItemSpec);

				if(pCDRW->uItemState&CDIS_CHECKED)
				{
					CRect rect;
					GetItemRect(nIndex,rect);

					if(IsDropDownArrow() && GetButtonStyle(nIndex)&TBSTYLE_DROPDOWN)
					{
						rect.right-=m_nDropDownArrowWidth;
					}

					CDC dc;
					COLORREF clrHilight=::GetSysColor(COLOR_BTNHILIGHT);
					COLORREF clrShadow=::GetSysColor(COLOR_BTNSHADOW);
					dc.Attach(pCDRW->hdc);
					dc.Draw3dRect(rect,clrShadow,clrHilight);
					dc.Detach();
				}
				break;
			}
		case CDDS_POSTPAINT:
			{
				break;
			}
		default:
			{
				// CToolBar does not send other draw-stages than the above.
				TRACE(_T("Unknown draw stage ...\n"));
				break;
			}
		}
	}
}
//////////////////////
#endif

// helper function to keep array of all used bitmaps
HBITMAP COXCoolToolBar::AddBitmap(LPTSTR lpszResourceName)
{
	HBITMAP hBitmap=NULL;
	if(!m_allBitmaps.Lookup(lpszResourceName,hBitmap))
	{
		if((hBitmap=::LoadBitmap(AfxGetResourceHandle(),
			lpszResourceName))==NULL)
		{
			TRACE(_T("COXCoolToolBar::AddBitmap: unable to load bitmap\n"));
			return NULL;
		}
		m_allBitmaps.SetAt(lpszResourceName,hBitmap);
	}

	return hBitmap;
}

// helper function to keep array of all used image lists
HIMAGELIST COXCoolToolBar::AddImageList(LPTSTR lpszResourceName, int cx, int cGrow, 
										COLORREF crMask, UINT uType, UINT uFlags)
{
	HANDLE hImageList=NULL;
	if(!m_allImageLists.Lookup(lpszResourceName,hImageList))
	{
		if((hImageList=(HANDLE)::ImageList_LoadImage(AfxGetResourceHandle(),
			lpszResourceName,cx,cGrow,crMask,uType,uFlags))==NULL)
		{
			TRACE(_T("COXCoolToolBar::AddImageList: unable to load image list\n"));
			return NULL;
		}
		m_allImageLists.SetAt(lpszResourceName,hImageList);
	}

	return (HIMAGELIST)hImageList;
}

// helper function to keep array of all used image lists
HIMAGELIST COXCoolToolBar::AddImageList(LPTSTR lpszResourceName, int cx, 
										COLORREF crMask, UINT uFlags, 
										int cInitial, int cGrow)
{
	HANDLE hImageList=NULL;
	if(!m_allImageLists.Lookup(lpszResourceName,hImageList))
	{
		CBitmap bitmap;
		if(!bitmap.LoadBitmap(lpszResourceName))
		{
			TRACE(_T("COXCoolToolBar::AddImageList: unable to load image\n"));
			return NULL;
		}
		BITMAP bitmapInfo;
		VERIFY(bitmap.GetBitmap(&bitmapInfo)!=0);
		if((hImageList=(HANDLE)::ImageList_Create(cx,bitmapInfo.bmHeight,
			uFlags,cInitial,cGrow))==NULL)
		{
			TRACE(_T("COXCoolToolBar::AddImageList: unable to create image list\n"));
			return NULL;
		}
		if(ImageList_AddMasked((HIMAGELIST)hImageList,bitmap,crMask)==-1)
		{
			TRACE(_T("COXCoolToolBar::AddImageList: unable to populate image list\n"));
			ImageList_Destroy((HIMAGELIST)hImageList);
			return NULL;
		}
		m_allImageLists.SetAt(lpszResourceName,hImageList);
	}

	return (HIMAGELIST)hImageList;
}


// helper function to automatically set text to buttons
//	TTID_PLAIN - lookup string resource for var equals button's ID
//			if found, set text in button to string text 
//			(not includded tool tip text)
//	TTID_TOOLTIP - lookup string resource for var equals button's ID
//			if found, set text in button to tooltip text 
//	TTID_MENU - lookup menu resource for var equals button's ID
//			if found, set text in button to text of corresponding Item in menu
//	TTID_NONE - remove all text associated with buttons
//	TTID_NOTSET - default value (there is no any text associated with toolbar)
void COXCoolToolBar::SetButtonTextFromID(UINT nFirstButtonID, 
										 UINT nLastButtonID, TextTypeFromID ttID)
{
	m_ttID=ttID;

	if(ttID==TTID_NOTSET)
	{
		return;
	}

	ASSERT(nFirstButtonID<=nLastButtonID);
	CToolBarCtrl* pToolBarCtrl = &GetToolBarCtrl();
	ASSERT(pToolBarCtrl != NULL);

	BOOL bMenuLoaded=FALSE;
	CMenu* pMenu=NULL;
	if(ttID==TTID_MENU)
	{
		// try to get a menu associated with framework
		if(AfxGetApp()->m_pMainWnd!=NULL)
		{
			if(AfxGetApp()->m_pMainWnd->GetMenu()!=NULL)
			{
				pMenu=AfxGetApp()->m_pMainWnd->GetMenu();
				bMenuLoaded=TRUE;
			}
			else
			{
				TRACE(_T("COXCoolToolBar::SetButtonTextFromID: cannot get framework menu!/n"));
			}
		}
		else
		{
			TRACE(_T("COXCoolToolBar::SetButtonTextFromID: m_pMainWnd of app is not defined yet menu!/n"));
		}
	}
	
	UINT nID;
	CString strText;
	for(int i=0; i<pToolBarCtrl->GetButtonCount(); i++)
	{
		nID=GetItemID(i);
		if(nID>=nFirstButtonID && nID<=nLastButtonID)
		{
			switch(ttID)
			{
			// text in string resourse (not included tooltip)
			case TTID_PLAIN:
				{
					if (strText.LoadString(nID))
					{
						int nPlacement=strText.Find(_T('\n'));   // tool tip is after "\n" in the string
						if (nPlacement!=-1)
						{	
							strText=strText.Left(nPlacement);
						}
					}
					break;
				}
			// text in string resourse (only tooltip)
			case TTID_TOOLTIP:
				{
					if (strText.LoadString(nID))
					{
						int nPlacement=strText.Find(_T('\n'));   // tool tip is after "\n" in the string
						if (nPlacement!=-1)
						{	
							strText=strText.Mid(nPlacement+1);
						}
						else
						{
							strText=_T("");
						}
					}
					break;
				}
			// text in menu item resourse
			case TTID_MENU:
				{
					if(bMenuLoaded)
					{
						ASSERT(pMenu!=NULL);
						pMenu->GetMenuString(nID,strText,MF_BYCOMMAND);
						if(!strText.IsEmpty())
						{
							int nPlacement=strText.Find(_T('\t'));   // accelerator keys are placed after "\t" 
							if (nPlacement!=-1)
							{	
								strText=strText.Left(nPlacement);
							}
						}
					}
					break;
				}
			// remove any text
			case TTID_NONE:
				{
					strText=_T("");
					break;
				}
			default:
				{
					m_ttID=TTID_NONE;
					strText=_T("");
					break;
				}
			}
			SetButtonText(i,strText);
		}
	}
}

// Save to registry state of buttons in CoolToolBar
BOOL COXCoolToolBar::SaveBarState(LPCTSTR lpszSubKey, LPCTSTR lpszValueName, 
								  BOOL bProperties)
{
#ifndef _MAC
	CWinApp* pApp=AfxGetApp();
	CString sProfileName;
	sProfileName.Format(_T("%s_%s"),lpszSubKey,lpszValueName);

	// make sure you called SetButtonsIds()  
	// makes no sense to load/store buttons state info for uncustomizable CoolToolBar
	if(m_pBitmapIds!=NULL && IsCustomizable())
	{
		// make sure you called CWinApp::SetRegistryKey() functions before
		if(pApp->m_pszRegistryKey==NULL || pApp->m_pszProfileName==NULL)
		{
			TRACE(_T("COXCoolToolBar::SaveBarState: haven't called	SetRegistryKey()\n"));
			return FALSE;
		}
		// we use default registry key assigned to your application by MFC
		HKEY hSecKey=pApp->GetSectionKey(_T(""));
		if (hSecKey==NULL)
		{
			TRACE(_T("COXCoolToolBar::SaveBarState: unable to get section key\n"));
			return FALSE;
		}
		GetToolBarCtrl().SaveState(hSecKey,lpszSubKey,lpszValueName);
		::RegCloseKey(hSecKey);
	}

	// save info about all buttons that have TBSTYLE_DROPDOWN style set
	CString sInfo=_T("");
	CString sButtonIndex;
	for(int nIndex=0; nIndex<GetToolBarCtrl().GetButtonCount(); nIndex++)
	{
		if(GetButtonStyle(nIndex)&TBSTYLE_DROPDOWN)
		{
			sButtonIndex.Format(_T("%d,"),nIndex);
			sInfo+=sButtonIndex;
		}
	}
	if(!sInfo.IsEmpty())
	{
//		sInfo.Left(sInfo.GetLength()-1);
		pApp->WriteProfileString(sProfileName,szDropDownButtonList,sInfo);
	}


	// save CoolToolBar properties
	if(bProperties)
	{
#if _MFC_VER>=0x0420
		pApp->WriteProfileInt(sProfileName,szCool,IsCool());
		pApp->WriteProfileInt(sProfileName,szGripper,IsGripper());
		pApp->WriteProfileInt(sProfileName,szSeparator,IsSeparator());
		pApp->WriteProfileInt(sProfileName,szFlat,IsFlat());
		pApp->WriteProfileInt(sProfileName,szList,IsList());
		pApp->WriteProfileInt(sProfileName,szDropDownArrow,IsDropDownArrow());
		pApp->WriteProfileInt(sProfileName,szIndent,GetIndent());
		pApp->WriteProfileInt(sProfileName,szDefaultTextColor,GetDefaultTextColor());
		pApp->WriteProfileInt(sProfileName,szHotTextColor,GetHotTextColor());
		pApp->WriteProfileInt(sProfileName,szTextType,GetTextType());
		pApp->WriteProfileInt(sProfileName,szTextRows,GetTextRows());
		pApp->WriteProfileInt(sProfileName,szMinButtonsWidth,GetButtonsMinMaxWidth().cx);
		pApp->WriteProfileInt(sProfileName,szMaxButtonsWidth,GetButtonsMinMaxWidth().cy);
#endif
		pApp->WriteProfileInt(sProfileName,szCustomizable,IsCustomizable());
	}

	return TRUE;
#endif
	return FALSE;
}

// Load from registry state of buttons in CoolToolBar
BOOL COXCoolToolBar::LoadBarState(LPCTSTR lpszSubKey, LPCTSTR lpszValueName, 
								  BOOL bProperties)
{
#ifndef _MAC
	CWinApp* pApp=AfxGetApp();
	CString sProfileName;
	sProfileName.Format(_T("%s_%s"),lpszSubKey,lpszValueName);
	int nResult;

	if(bProperties)
	{
		nResult=pApp->GetProfileInt(sProfileName,szCustomizable,-1);
		if(nResult!=-1)
		{
			SetCustomizable(nResult);
		}
	}

	// make sure you called SetButtonsIds()  
	// makes no sense to load/store buttons state info for uncustomizable CoolToolBar
	if(m_pBitmapIds!=NULL && IsCustomizable())
	{
		// make sure you called CWinApp::SetRegistryKey() functions before
		if(pApp->m_pszRegistryKey==NULL || pApp->m_pszProfileName==NULL)
		{
			TRACE(_T("COXCoolToolBar::LoadBarState: haven't called	SetRegistryKey()\n"));
			return FALSE;
		}
		// we use default registry key assigned to your application by MFC
		HKEY hSecKey=pApp->GetSectionKey(_T(""));
		if (hSecKey==NULL)
		{
			TRACE(_T("COXCoolToolBar::LoadBarState: unable to get section key\n"));
			return FALSE;
		}
		GetToolBarCtrl().RestoreState(hSecKey,lpszSubKey,lpszValueName);
		::RegCloseKey(hSecKey);
	}

	SetButtonTextFromID(m_ttID);

	// apply TBSTYLE_DROPDOWN style to corresponding buttons
	CString sInfo=pApp->GetProfileString(sProfileName,szDropDownButtonList,_T(""));
	while(!sInfo.IsEmpty())
	{
		int nEndPos=sInfo.Find(_T(","));
		CString sButtonIndex=sInfo.Left(nEndPos);
		int nButtonIndex=_ttoi(sButtonIndex.GetBuffer(sButtonIndex.GetLength()));
		sButtonIndex.ReleaseBuffer();
		SetButtonStyle(nButtonIndex,GetButtonStyle(nButtonIndex)|TBSTYLE_DROPDOWN);
		sInfo=sInfo.Mid(nEndPos+1);
	}

	// load and apply CoolToolBar properties
	if(bProperties)
	{
#if _MFC_VER>=0x0420
		BOOL bUpdate=FALSE;

		nResult=pApp->GetProfileInt(sProfileName,szCool,-1);
		if(nResult!=-1)
		{
			SetCool(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szGripper,-1);
		if(nResult!=-1)
		{
			SetGripper(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szSeparator,-1);
		if(nResult!=-1)
		{
			SetSeparator(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szFlat,-1);
		if(nResult!=-1)
		{
			SetFlat(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szList,-1);
		if(nResult!=-1)
		{
			SetList(nResult);
			bUpdate=TRUE;
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szDropDownArrow,-1);
		if(nResult!=-1)
		{
			SetDropDownArrow(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szIndent,-1);
		if(nResult!=-1)
		{
			SetIndent(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szDefaultTextColor,-1);
		if(nResult!=-1)
		{
			SetDefaultTextColor(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szHotTextColor,-1);
		if(nResult!=-1)
		{
			SetHotTextColor(nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szTextType,-1);
		if(nResult!=-1 && nResult!=GetTextType())
		{
			if(nResult==TTID_NOTSET)
			{
				SetButtonTextFromID(TTID_NONE);
			}
			bUpdate=TRUE;
			SetButtonTextFromID((TextTypeFromID)nResult);
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szTextRows,-1);
		if(nResult!=-1 && nResult!=GetTextRows())
		{
			SetMaxTextRows(nResult);
			bUpdate=TRUE;
		}
	
		nResult=pApp->GetProfileInt(sProfileName,szMinButtonsWidth,-1);
		if(nResult!=-1 && nResult!=m_sizeMinMaxWidth.cx)
		{
			m_sizeMinMaxWidth.cx=nResult;
			SetButtonsMinMaxWidth(m_sizeMinMaxWidth.cx,m_sizeMinMaxWidth.cy);
		}
		nResult=pApp->GetProfileInt(sProfileName,szMaxButtonsWidth,-1);
		if(nResult!=-1 && nResult!=m_sizeMinMaxWidth.cy)
		{
			m_sizeMinMaxWidth.cy=nResult;
			SetButtonsMinMaxWidth(m_sizeMinMaxWidth.cx,m_sizeMinMaxWidth.cy);
			bUpdate=TRUE;
		}

		// change the size of CoolToolBar
		if(bUpdate)
		{
			if(GetTextRows()==0)
			{
				IniSizes(m_sizeImage);
			}
			else
			{
				UpdateSizes();
			}

			RedrawToolBar();
		}
#endif
	}
	return TRUE;
#endif
	return FALSE;
}

void COXCoolToolBar::DockControlBarLeftOf(CToolBar *leftOf)
{
	ASSERT_VALID(this);
	// make sure CControlBar::EnableDocking has been called
	ASSERT(m_pDockContext!=NULL);

	if(leftOf!=NULL)
	{
		ASSERT_VALID(leftOf);
	}

	CFrameWnd* pFrameWnd=GetDockingFrame();
	ASSERT_VALID(pFrameWnd);

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	pFrameWnd->RecalcLayout();

	UINT nDockBarID=0;
	CRect rect(0,0,0,0);
	if(leftOf!=NULL)
	{
		leftOf->GetWindowRect(&rect);
		rect.OffsetRect(1,0);

		DWORD dwStyle=leftOf->GetBarStyle();
		nDockBarID=(dwStyle & CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP :
			(dwStyle & CBRS_ALIGN_BOTTOM) ? AFX_IDW_DOCKBAR_BOTTOM :
			(dwStyle & CBRS_ALIGN_LEFT) ? AFX_IDW_DOCKBAR_LEFT :
			(dwStyle & CBRS_ALIGN_RIGHT) ? AFX_IDW_DOCKBAR_RIGHT : 0;
	}


	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. Calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	pFrameWnd->DockControlBar(this, nDockBarID, &rect);
}

#if _MFC_VER<=0x0421

#define OXCY_BORDER		1

void COXCoolToolBar::SetHeight(int cyHeight)
{
	ASSERT_VALID(this);

	int nHeight = cyHeight;
	if (m_dwStyle & CBRS_BORDER_TOP)
		cyHeight -= OXCY_BORDER;
	if (m_dwStyle & CBRS_BORDER_BOTTOM)
		cyHeight -= OXCY_BORDER;
	m_cyBottomBorder = (cyHeight - m_sizeButton.cy) / 2;
	// if there is an extra pixel, m_cyTopBorder will get it
	m_cyTopBorder = cyHeight - m_sizeButton.cy - m_cyBottomBorder;
	if (m_cyTopBorder < 0)
	{
		TRACE(_T("Warning: COXCoolToolBar::SetHeight(%d) is smaller than button.\n"),
			nHeight);
		m_cyBottomBorder += m_cyTopBorder;
		m_cyTopBorder = 0;  // will clip at bottom
	}

	// recalculate the non-client region
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
	Invalidate();   // just to be nice if called when toolbar is visible
}
#endif

BOOL COXCoolToolBar::IsWindowsNTRunning()
{
#if _WIN32_WINNT <= 0x0603
	BOOL bResult = FALSE;

	OSVERSIONINFO verInfo;
	verInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&verInfo))
	{
		if (verInfo.dwPlatformId==VER_PLATFORM_WIN32_NT && 
			verInfo.dwMajorVersion>=4)
		{
			bResult=TRUE;
		}
	}
	return bResult;
#else
	return ::IsWindowsVersionOrGreater(4, 0, 0);
#endif
}

int COXCoolToolBar::GetDropDownArrowWidth() const
{
#if _MFC_VER>=0x0420
	HDC hDC = ::GetDC(NULL);
	ASSERT(hDC != NULL);

	HFONT hFont;
	HFONT oldFont=NULL;
	int nDropDownArrowWidth=0;
	if((hFont=CreateFont(GetSystemMetrics(SM_CYMENUCHECK), 0, 0, 0, 
		FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett")))!=NULL)
	{
		oldFont = (HFONT)SelectObject(hDC, hFont);
	}
	
	VERIFY(GetCharWidth(hDC, '6', '6', &nDropDownArrowWidth));

	if(oldFont!=NULL)
	{
		SelectObject(hDC, oldFont);
	}
	if(hFont!=NULL)
	{
		DeleteObject(hFont);
	}
	::ReleaseDC(NULL, hDC);
	
	return nDropDownArrowWidth;
#else
	return 0;
#endif
}

#if _MFC_VER>=0x0420
////////////////////////////////
void COXCoolToolBar::_GetButton(int nIndex, TBBUTTON* pButton) const
{
	COXCoolToolBar* pBar = (COXCoolToolBar*)this;
	VERIFY(pBar->DefWindowProc(TB_GETBUTTON, nIndex, (LPARAM)pButton));
	// TBSTATE_ENABLED == TBBS_DISABLED so invert it
	pButton->fsState ^= TBSTATE_ENABLED;
}

void COXCoolToolBar::_SetButton(int nIndex, TBBUTTON* pButton)
{
	// get original button state
	TBBUTTON button;
	VERIFY(DefWindowProc(TB_GETBUTTON, nIndex, (LPARAM)&button));

	// prepare for old/new button comparsion
	button.bReserved[0] = 0;
	button.bReserved[1] = 0;
	// TBSTATE_ENABLED == TBBS_DISABLED so invert it
	pButton->fsState ^= TBSTATE_ENABLED;
	pButton->bReserved[0] = 0;
	pButton->bReserved[1] = 0;

	// nothing to do if they are the same
	if (memcmp(pButton, &button, sizeof(TBBUTTON)) != 0)
	{
		// don't redraw anything while setting the button
		BOOL bWasVisible=(GetStyle()&WS_VISIBLE);
		if(bWasVisible)
			ModifyStyle(WS_VISIBLE, 0);
		VERIFY(DefWindowProc(TB_DELETEBUTTON, nIndex, 0));
		VERIFY(DefWindowProc(TB_INSERTBUTTON, nIndex, (LPARAM)pButton));
		if(bWasVisible)
			ModifyStyle(0,WS_VISIBLE);

		// invalidate appropriate parts
		if (((pButton->fsStyle ^ button.fsStyle) & TBSTYLE_SEP) ||
			((pButton->fsStyle & TBSTYLE_SEP) && pButton->iBitmap != button.iBitmap))
		{
			// changing a separator
			Invalidate(FALSE);
		}
		else
		{
			// invalidate just the button
			CRect rect;
			if (DefWindowProc(TB_GETITEMRECT, nIndex, (LPARAM)&rect))
				InvalidateRect(rect, FALSE);    // don't erase background
		}
	}
}

#ifdef _MAC
	#define CX_OVERLAP  1
#else
	#define CX_OVERLAP  0
#endif

int COXCoolToolBar::WrapToolBar(TBBUTTON* pData, int nCount, int nWidth)
{
	ASSERT(pData != NULL && nCount > 0);

	int nResult = 0;
	int x = 0;
	for (int i = 0; i < nCount; i++)
	{
		pData[i].fsState &= ~TBSTATE_WRAP;

		if (pData[i].fsState & TBSTATE_HIDDEN)
			continue;

		int dx, dxNext;
		if (pData[i].fsStyle & TBSTYLE_SEP)
		{
			dx = pData[i].iBitmap;
			dxNext = dx;
		}
		else
		{
			dx = m_sizeButton.cx;
			if(pData[i].fsStyle & TBSTYLE_AUTOSIZE)
			{
				CRect rect;
				GetToolBarCtrl().GetItemRect(i,&rect);
				dx=rect.Width();
			}
			// check for dropdown style, but only if the buttons are being drawn
			if((pData[i].fsStyle & TBSTYLE_DROPDOWN) && IsDropDownArrow())
			{
				if(!IsFlat() || m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
				{
					// add size of drop down
					if(m_dwComCtlVersion<_IE40_COMCTL_VERSION)
					{
						m_nDropDownArrowWidth=GetDropDownArrowWidth();
					}
					dx += m_nDropDownArrowWidth;
				}
			}
			dxNext = dx - CX_OVERLAP;
		}

		if (x + dx > nWidth)
		{
			BOOL bFound = FALSE;
			for (int j = i; j >= 0  &&  !(pData[j].fsState & TBSTATE_WRAP); j--)
			{
				// Find last separator that isn't hidden
				// a separator that has a command ID is not
				// a separator, but a custom control.
				if ((pData[j].fsStyle & TBSTYLE_SEP) &&
					(pData[j].idCommand == 0) &&
					!(pData[j].fsState & TBSTATE_HIDDEN))
				{
					bFound = TRUE; i = j; x = 0;
					pData[j].fsState |= TBSTATE_WRAP;
					nResult++;
					break;
				}
			}
			if (!bFound)
			{
				for (int j = i - 1; j >= 0 && 
					!(pData[j].fsState & TBSTATE_WRAP); j--)
				{
					// Never wrap anything that is hidden,
					// or any custom controls
					if ((pData[j].fsState & TBSTATE_HIDDEN) ||
						((pData[j].fsStyle & TBSTYLE_SEP) &&
						(pData[j].idCommand != 0)))
						continue;

					bFound = TRUE; i = j; x = 0;
					pData[j].fsState |= TBSTATE_WRAP;
					nResult++;
					break;
				}
				if (!bFound)
					x += dxNext;
			}
		}
		else
			x += dxNext;
	}
	return nResult + 1;
}

void COXCoolToolBar::SizeToolBar(TBBUTTON* pData, int nCount, 
								 int nLength, BOOL bVert/*=FALSE*/)
{
	ASSERT(pData != NULL && nCount > 0);

	if (!bVert)
	{
		int nMin, nMax, nTarget, nCurrent, nMid;

		// Wrap ToolBar as specified
		nMax = nLength;
		nTarget = WrapToolBar(pData, nCount, nMax);

		// Wrap ToolBar vertically
		nMin = 0;
		nCurrent = WrapToolBar(pData, nCount, nMin);

		if (nCurrent != nTarget)
		{
			while (nMin < nMax)
			{
				nMid = (nMin + nMax) / 2;
				nCurrent = WrapToolBar(pData, nCount, nMid);

				if (nCurrent == nTarget)
					nMax = nMid;
				else
				{
					if (nMin == nMid)
					{
						WrapToolBar(pData, nCount, nMax);
						break;
					}
					nMin = nMid;
				}
			}
		}
		CSize size = CalcSize(pData, nCount);
		WrapToolBar(pData, nCount, size.cx);
	}
	else
	{
		CSize sizeMax, sizeMin, sizeMid;

		// Wrap ToolBar vertically
		WrapToolBar(pData, nCount, 0);
		sizeMin = CalcSize(pData, nCount);

		// Wrap ToolBar horizontally
		WrapToolBar(pData, nCount, 32767);
		sizeMax = CalcSize(pData, nCount);

		while (sizeMin.cx < sizeMax.cx)
		{
			sizeMid.cx = (sizeMin.cx + sizeMax.cx) / 2;
			WrapToolBar(pData, nCount, sizeMid.cx);
			sizeMid = CalcSize(pData, nCount);

			if (nLength < sizeMid.cy)
			{
				if (sizeMin == sizeMid)
				{
					WrapToolBar(pData, nCount, sizeMax.cx);
					return;
				}
				sizeMin = sizeMid;
			}
			else if (nLength > sizeMid.cy)
				sizeMax = sizeMid;
			else
				return;
		}
	}
}

CSize COXCoolToolBar::CalcSize(TBBUTTON* pData, int nCount)
{
	ASSERT(pData != NULL && nCount > 0);

	CPoint cur(0,0);
	CSize sizeResult(0,0);

	for (int i = 0; i < nCount; i++)
	{
		int cySep = pData[i].iBitmap;
		if(!IsFlat() || m_dwComCtlVersion<_IE40_COMCTL_VERSION)
		{
			cySep = cySep * 2 / 3;
		}

		if(pData[i].fsState & TBSTATE_HIDDEN)
			continue;

		int cx = m_sizeButton.cx;
		if(pData[i].fsStyle & TBSTYLE_AUTOSIZE)
		{
			CRect rect;
			GetToolBarCtrl().GetItemRect(i,&rect);
			cx=rect.Width();
		}


		if(pData[i].fsStyle & TBSTYLE_SEP)
		{
			// a separator represents either a height or width
			if (pData[i].fsState & TBSTATE_WRAP)
				sizeResult.cy = __max(cur.y + m_sizeButton.cy + cySep, 
					sizeResult.cy);
			else
				sizeResult.cx = __max(cur.x + pData[i].iBitmap, 
					sizeResult.cx);
		}
		else 
		{
			// check for dropdown style, but only if the buttons are being drawn
			if((pData[i].fsStyle & TBSTYLE_DROPDOWN) && IsDropDownArrow())
			{
				if(!IsFlat() || m_dwComCtlVersion>=_IE40_COMCTL_VERSION)
				{
					// add size of drop down
					if(m_dwComCtlVersion<_IE40_COMCTL_VERSION)
					{
						m_nDropDownArrowWidth=GetDropDownArrowWidth();
					}
					cx += m_nDropDownArrowWidth;
				}
			}
			sizeResult.cx = __max(cur.x + cx, sizeResult.cx);
			sizeResult.cy = __max(cur.y + m_sizeButton.cy, sizeResult.cy);
		}

		if(pData[i].fsStyle & TBSTYLE_SEP)
		{
			cur.x += pData[i].iBitmap;
		}
		else
		{
			cur.x += cx - CX_OVERLAP;
		}

		if(pData[i].fsState & TBSTATE_WRAP)
		{
			cur.x = 0;
			cur.y += m_sizeButton.cy;
			if(pData[i].fsStyle & TBSTYLE_SEP)
				cur.y += cySep;
		}
	}

	// add indention
	sizeResult.cx+=m_nIndent;

	return sizeResult;
}

struct _AFX_CONTROLPOS
{
	int nIndex, nID;
	CRect rectOldPos;
};

CSize COXCoolToolBar::CalcLayout(DWORD dwMode, int nLength)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));
	if(dwMode & LM_HORZDOCK)
	{
		ASSERT(dwMode & LM_HORZ);
	}

	int nCount;
	TBBUTTON* pData=NULL;
	CSize sizeResult(0,0);

	// Load Buttons
	{
		nCount = DefWindowProc(TB_BUTTONCOUNT, 0, 0);
		if (nCount != 0)
		{
			int i;
			pData = new TBBUTTON[nCount];
			for (i = 0; i < nCount; i++)
				_GetButton(i, &pData[i]);
		}
	}

	if (nCount > 0)
	{
		if (!(m_dwStyle & CBRS_SIZE_FIXED))
		{
			BOOL bDynamic = m_dwStyle & CBRS_SIZE_DYNAMIC;

			ASSERT(pData!=NULL);

			if (bDynamic && (dwMode & LM_MRUWIDTH))
				SizeToolBar(pData, nCount, m_nMRUWidth);
			else if (bDynamic && (dwMode & LM_HORZDOCK))
				SizeToolBar(pData, nCount, 32767);
			else if (bDynamic && (dwMode & LM_VERTDOCK))
				SizeToolBar(pData, nCount, 0);
			else if (bDynamic && (nLength != -1))
			{
				CRect rect; rect.SetRectEmpty();
				CalcInsideRect(rect, (dwMode & LM_HORZ));
				BOOL bVert = (dwMode & LM_LENGTHY);
				int nLen = nLength + (bVert ? rect.Height() : rect.Width());

				SizeToolBar(pData, nCount, nLen, bVert);
			}
			else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
				SizeToolBar(pData, nCount, m_nMRUWidth);
			else
				SizeToolBar(pData, nCount, (dwMode & LM_HORZ) ? 32767 : 0);
		}

		sizeResult = CalcSize(pData, nCount);

		if (dwMode & LM_COMMIT)
		{
			int i = 0;
			_AFX_CONTROLPOS* pControl = NULL;
			int nControlCount = 0;
			BOOL bIsDelayed = m_bDelayedButtonLayout;
			m_bDelayedButtonLayout = FALSE;

			for(i = 0; i < nCount; i++)
				if ((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand != 0))
					nControlCount++;

			if (nControlCount > 0)
			{
				pControl = new _AFX_CONTROLPOS[nControlCount];
				nControlCount = 0;

				for(i = 0; i < nCount; i++)
				{
					if ((pData[i].fsStyle & TBSTYLE_SEP) && (pData[i].idCommand != 0))
					{
						pControl[nControlCount].nIndex = i;
						pControl[nControlCount].nID = pData[i].idCommand;

						CRect rect;
						GetItemRect(i, &rect);
						ClientToScreen(&rect);
						pControl[nControlCount].rectOldPos = rect;

						nControlCount++;
					}
				}
			}

			if ((m_dwStyle & CBRS_FLOATING) && (m_dwStyle & CBRS_SIZE_DYNAMIC))
				m_nMRUWidth = sizeResult.cx;
			for (i = 0; i < nCount; i++)
				_SetButton(i, &pData[i]);

			if (nControlCount > 0)
			{
				for (i = 0; i < nControlCount; i++)
				{
					CWnd* pWnd = GetDlgItem(pControl[i].nID);
					if (pWnd != NULL)
					{
						CRect rect;
						pWnd->GetWindowRect(&rect);
						CPoint pt = rect.TopLeft() - pControl[i].rectOldPos.TopLeft();
						GetItemRect(pControl[i].nIndex, &rect);
						pt = rect.TopLeft() + pt;
						pWnd->SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
					}
				}
				delete[] pControl;
			}
			m_bDelayedButtonLayout = bIsDelayed;
		}
		delete[] pData;
	}
	else
	{
		CSize sizeButtons=GetButtonsSize();
		if(dwMode & LM_HORZ)
		{
			sizeResult.cx=sizeButtons.cx/2;
			sizeResult.cy=sizeButtons.cy-1;
		}
		else
		{
			sizeResult.cx=sizeButtons.cx+1;
			sizeResult.cy=sizeButtons.cy/2;
		}
	}

	//BLOCK: Adjust Margins
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, (dwMode & LM_HORZ));
		sizeResult.cy -= rect.Height();
		sizeResult.cx -= rect.Width();

		CSize size = CControlBar::CalcFixedLayout((dwMode & LM_STRETCH), 
			(dwMode & LM_HORZ));
		sizeResult.cx = __max(sizeResult.cx, size.cx);
		sizeResult.cy = __max(sizeResult.cy, size.cy);
	}

	return sizeResult;
}

CSize COXCoolToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	DWORD dwMode = bStretch ? LM_STRETCH : 0;
	dwMode |= bHorz ? LM_HORZ : 0;

	return CalcLayout(dwMode);
}

CSize COXCoolToolBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if ((nLength == -1) && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT) &&
		((dwMode & LM_HORZDOCK) || (dwMode & LM_VERTDOCK)))
	{
		return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZDOCK);
	}
	return CalcLayout(dwMode, nLength);
}

void COXCoolToolBar::CalcInsideRect(CRect& rect, BOOL bHorz)
{
	CToolBar::CalcInsideRect(rect,bHorz);

	if(m_hIcon)
	{
		// Icon size
		CSize szIcon(::GetSystemMetrics(SM_CXSMICON),
			::GetSystemMetrics(SM_CYSMICON));
		if(bHorz)
			rect.left+=szIcon.cx+2;
		else
			rect.top+=szIcon.cy+2;
	}

	CRect rectBookedSpace(0,0,0,0);
	BookSpace(rectBookedSpace,(bHorz ? LM_HORZ : 0));
	rect.left+=rectBookedSpace.left;
	rect.top+=rectBookedSpace.top;
	rect.right-=rectBookedSpace.right;
	rect.bottom-=rectBookedSpace.bottom;

	if(IsGripper())
	{
		if(bHorz)
			rect.left+=ID_OXGRIPPER_WIDTH;
		else
			rect.top+=ID_OXGRIPPER_WIDTH;
	}
}

HRESULT COXCoolToolBar::GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor) const
{
	if(IsBadWritePtr(pdwMajor, sizeof(DWORD)) || 
		IsBadWritePtr(pdwMinor, sizeof(DWORD)))
	{
		return E_INVALIDARG;
	}

	// get handle of the common control DLL
	BOOL bAlreadyLoaded=TRUE;
	HINSTANCE hComCtl=::GetModuleHandle(_T("comctl32.dll"));
	if(hComCtl==NULL)
	{
		// load the DLL
		hComCtl=::LoadLibrary(_T("comctl32.dll"));
		bAlreadyLoaded=FALSE;
	}

	if(hComCtl)
	{
		HRESULT           hr=S_OK;
		DLLGETVERSIONPROC pDllGetVersion;
   
		/*
		You must get this function explicitly because earlier versions of the DLL 
		don't implement this function. That makes the lack of implementation of the 
		function a version marker in itself.
		*/
		pDllGetVersion=(DLLGETVERSIONPROC)GetProcAddress(hComCtl, "DllGetVersion");
   
		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
      
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize=sizeof(dvi);
   
			hr = (*pDllGetVersion)(&dvi);
      
			if(SUCCEEDED(hr))
			{
				*pdwMajor = dvi.dwMajorVersion;
				*pdwMinor = dvi.dwMinorVersion;
			}
			else
			{
				hr = E_FAIL;
			}   
		}
		else
		{
			// If GetProcAddress failed, then the DLL is a version previous 
			// to the one shipped with IE 3.x.
			*pdwMajor = 4;
			*pdwMinor = 0;
		}
   
		if(!bAlreadyLoaded)
			::FreeLibrary(hComCtl);

		return hr;
	}

	return E_FAIL;
}

////////////////////////////////
#endif //_MFC_VER>=0x0420

// BaseTextView.cpp : implementation file
//

#include "StdAfx.h"
#include "BaseTextView.h"
#include "BaseTextDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseTextView

IMPLEMENT_DYNCREATE(CBaseTextView, CEditView)

CBaseTextView::CBaseTextView()
{
  mF = NULL;
}

CBaseTextView::~CBaseTextView()
{
    if(mF) delete mF;
}

void CBaseTextView::WriteTextStr()
{
  CEdit& EditCtrl = GetEditCtrl();
  EditCtrl.SetWindowText(LPCTSTR(TextInfo));
}

BEGIN_MESSAGE_MAP(CBaseTextView, CEditView)
	//{{AFX_MSG_MAP(CBaseTextView)
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
    ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseTextView drawing

void CBaseTextView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CBaseTextView diagnostics

#ifdef _DEBUG
void CBaseTextView::AssertValid() const
{
	CEditView::AssertValid();
}

void CBaseTextView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBaseTextView message handlers

void CBaseTextView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CEditView::OnRButtonDown(nFlags, point);
}

void CBaseTextView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
  CString sFaceName = "Courier";
  LOGFONT LogFont;
   LogFont.lfHeight=16;
     LogFont.lfWidth=0;
     LogFont.lfEscapement = 0;
     LogFont.lfOrientation = 0;
     LogFont.lfWeight = FW_NORMAL;
     LogFont.lfItalic = 0;
     LogFont.lfUnderline = 0;
     LogFont.lfStrikeOut = 0;
     LogFont.lfCharSet = DEFAULT_CHARSET;
     LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
     LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
     LogFont.lfQuality = DRAFT_QUALITY;
     LogFont.lfPitchAndFamily = VARIABLE_PITCH | FF_MODERN;
     strcpy(LogFont.lfFaceName,sFaceName.GetBuffer(81));
     mF = new CFont;
     mF->CreateFontIndirect(&LogFont);
     SetFont(mF);
	
}

void CBaseTextView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
   CMenu menu;
   menu.CreatePopupMenu();

   UINT FlagUndo = MF_STRING;
   UINT FlagCut = MF_STRING;
   UINT FlagCopy = MF_STRING;
   UINT FlagPaste = MF_STRING;
   UINT FlagClear = MF_STRING;
   UINT FlagSelectAll = MF_STRING;

   if(!GetEditCtrl().CanUndo()){
	   FlagUndo |= MF_GRAYED;
   }
   int nStartChar, nEndChar;
   GetEditCtrl().GetSel(nStartChar, nEndChar);
   if(nStartChar == nEndChar){
	   FlagCut |= MF_GRAYED;
	   FlagCopy |= MF_GRAYED;
	   FlagClear |= MF_GRAYED;
   }
   if(!::IsClipboardFormatAvailable(CF_TEXT)){
	   FlagPaste |= MF_GRAYED;
   }
   if(GetWindowTextLength() == 0){
	   FlagSelectAll|= MF_GRAYED;
   }

   menu.AppendMenu(FlagUndo, ID_EDIT_UNDO, _T("Undo"));   
   menu.AppendMenu(MF_SEPARATOR);
   menu.AppendMenu(FlagCut, ID_EDIT_CUT, _T("Cut"));
   menu.AppendMenu(FlagCopy, ID_EDIT_COPY, _T("Copy"));
   menu.AppendMenu(FlagPaste, ID_EDIT_PASTE, _T("Paste"));
   menu.AppendMenu(FlagClear, ID_EDIT_CLEAR, _T("Delete"));
   menu.AppendMenu(MF_SEPARATOR);
   menu.AppendMenu(FlagSelectAll, ID_EDIT_SELECT_ALL, _T("Select All"));

   menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
}

void CBaseTextView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBaseTextDoc* pDoc = (CBaseTextDoc*)GetDocument();
	if(pDoc->ReadOnly)
	  return;
	else   
	  CEditView::OnChar(nChar, nRepCnt, nFlags);
}

void CBaseTextView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CBaseTextDoc* pDoc = (CBaseTextDoc*)GetDocument();
	if(pDoc->ReadOnly && nChar==VK_DELETE)
	  return;
	else   
      CEditView::OnKeyDown(nChar, nRepCnt, nFlags);
}


// BaseTextDoc.cpp : implementation file
//

#include "StdAfx.h"
#include "BaseTextDoc.h"
#include "BaseTextView.h"

#include "Utils\mutils.h"
#include "MGTools\Include\Utils\Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseTextDoc

IMPLEMENT_DYNCREATE(CBaseTextDoc, CDocument)

CBaseTextDoc::CBaseTextDoc()
{
	ReadOnly = FALSE;
}

BOOL CBaseTextDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CBaseTextDoc::~CBaseTextDoc()
{
}


BEGIN_MESSAGE_MAP(CBaseTextDoc, CDocument)
	//{{AFX_MSG_MAP(CBaseTextDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseTextDoc diagnostics

#ifdef _DEBUG
void CBaseTextDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBaseTextDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBaseTextDoc serialization

void CBaseTextDoc::Serialize(CArchive& ar)
{
   // CEditView contains an edit control which handles all serialization
   ((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CBaseTextDoc commands

BOOL CBaseTextDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDocument::CanCloseFrame(pFrame);
}

void CBaseTextDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CDocument::OnCloseDocument();
}

BOOL CBaseTextDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	CString ext = lpszPathName;
	ext = ext.Right(3);
    ext.MakeUpper();
    if(ext == "FRN" || ext == "ZAP")
		ReadOnly = TRUE;
    else
		ReadOnly = FALSE;


    POSITION pos = GetFirstViewPosition();
    CBaseTextView* pView = (CBaseTextView*)GetNextView(pos);
	CStdioFile fl;
	if(!fl.Open(lpszPathName, CStdioFile::modeRead))
		return FALSE;
	CString Text, s;
	BOOL res = fl.ReadString(s);
	Text.Empty();
    while(res){
		Text += s; Text += "\r\n";
	    res = fl.ReadString(s);
    }
    pView->SetWindowText(LPCTSTR(Text));
    fl.Close();
	return TRUE;
}

BOOL CBaseTextDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	BOOL res = CDocument::OnSaveDocument(lpszPathName);
	MakeLoopMessage();
	return TRUE;
}

void CBaseTextDoc::SetContents(LPCTSTR Title, LPCTSTR Contents)
{
   POSITION Pos = GetFirstViewPosition();
   CEditView *PCEditView = (CEditView *)GetNextView (Pos); 
   if(PCEditView){
      SetTitle(Title);
      PCEditView->SetWindowText(Contents);
   } 
}

void CBaseTextDoc::DeleteContents() 
{
   POSITION Pos = GetFirstViewPosition ();
   CEditView *PCEditView = (CEditView *)GetNextView (Pos); 
   if (PCEditView) 
      PCEditView->SetWindowText ("");
   
   CDocument::DeleteContents();
}

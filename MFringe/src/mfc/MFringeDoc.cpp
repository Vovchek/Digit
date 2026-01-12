#include "../../StdAfx.h"
#include "MFringeDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMFringeDoc, CDocument)

CMFringeDoc::CMFringeDoc()
{
}

CMFringeDoc::~CMFringeDoc()
{
}

BOOL CMFringeDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

void CMFringeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef _DEBUG
void CMFringeDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFringeDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif

BEGIN_MESSAGE_MAP(CMFringeDoc, CDocument)
END_MESSAGE_MAP()
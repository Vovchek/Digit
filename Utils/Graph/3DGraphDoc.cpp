// 3DGraphDoc.cpp : implementation file
//

#include "stdafx.h"
#include "digit.h"
#include "ImageTempl\ImageDoc.h"
#include "Utils\mutils.h"
#include "3DGraphDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DGraphDoc

IMPLEMENT_DYNCREATE(C3DGraphDoc, CDocument)

C3DGraphDoc::C3DGraphDoc()
{
}

BOOL C3DGraphDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    POSITION pos = GetFirstViewPosition();

    C3DGraph* pGr = (C3DGraph*)GetNextView(pos);

    DataSimulation(pGr);
    pGr->RefreshPicture();
	return TRUE;
}

void C3DGraphDoc::DataSimulation(C3DGraph* pGr)
{
   CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();

   int n = pDoc->nx*pDoc->ny;
   float* pM = new float[n];
   float val;
   for(int i=0; i < n; i++){
       val = (float)pDoc->pMatr[i];
       pM[i] = val;
   }
   pGr->SetBound(-1., 1., -1., 1.);
   pGr->Set3DMatrix(pM, pDoc->nx, pDoc->ny);
}

C3DGraphDoc::~C3DGraphDoc()
{
}


BEGIN_MESSAGE_MAP(C3DGraphDoc, CDocument)
	//{{AFX_MSG_MAP(C3DGraphDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DGraphDoc diagnostics

#ifdef _DEBUG
void C3DGraphDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void C3DGraphDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// C3DGraphDoc serialization

void C3DGraphDoc::Serialize(CArchive& ar)
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

/////////////////////////////////////////////////////////////////////////////
// C3DGraphDoc commands

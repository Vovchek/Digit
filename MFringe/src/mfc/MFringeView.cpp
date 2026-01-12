#include "../../StdAfx.h"
#include "MFringeView.h"
#include "MFringeDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CMFringeView, CView)

CMFringeView::CMFringeView()
{
}

CMFringeView::~CMFringeView()
{
}

BOOL CMFringeView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CMFringeView::OnDraw(CDC* pDC)
{
	CMFringeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code here
}

void CMFringeView::OnPaint()
{
	CPaintDC dc(this);
	OnDraw(&dc);
}

#ifdef _DEBUG
void CMFringeView::AssertValid() const
{
	CView::AssertValid();
}

void CMFringeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFringeDoc* CMFringeView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFringeDoc)));
	return (CMFringeDoc*)m_pDocument;
}
#endif

BEGIN_MESSAGE_MAP(CMFringeView, CView)
	ON_WM_PAINT()
END_MESSAGE_MAP()
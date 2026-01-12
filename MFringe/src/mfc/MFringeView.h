#pragma once

class CMFringeDoc;

class CMFringeView : public CView
{
protected:
	CMFringeView();
	DECLARE_DYNCREATE(CMFringeView)

public:
	CMFringeDoc* GetDocument();

	virtual ~CMFringeView();

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

#ifndef _DEBUG  // debug version in MFringeView.cpp
inline CMFringeDoc* CMFringeView::GetDocument()
	{ return (CMFringeDoc*)m_pDocument; }
#endif
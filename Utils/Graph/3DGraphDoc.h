#if !defined(AFX_3DGRAPHDOC_H__C7D6710E_1488_4686_BADC_C74941782E9B__INCLUDED_)
#define AFX_3DGRAPHDOC_H__C7D6710E_1488_4686_BADC_C74941782E9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DGraphDoc.h : header file
//
//C:\Ilya\Programming\cpp\Numbering\Utils\Graph\3DGraphDoc.h
/////////////////////////////////////////////////////////////////////////////
// C3DGraphDoc document
#include "MGTools\Include\Graph\3dGraph.h"

class C3DGraphDoc : public CDocument
{
protected:
	C3DGraphDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(C3DGraphDoc)

// Attributes
public:

// Operations
public:
    void DataSimulation(C3DGraph* pGr);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DGraphDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~C3DGraphDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(C3DGraphDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DGRAPHDOC_H__C7D6710E_1488_4686_BADC_C74941782E9B__INCLUDED_)

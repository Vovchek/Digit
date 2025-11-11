#if !defined(AFX_ISOLINEGRAPH_H__03971D0D_CD62_4970_8026_C6321E4980C4__INCLUDED_)
#define AFX_ISOLINEGRAPH_H__03971D0D_CD62_4970_8026_C6321E4980C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IsolineGraph.h : header file
//
#include "MGToolsExport.h"
#include "2DGraph.h"
#include "contour.h"

/////////////////////////////////////////////////////////////////////////////
// CIsolineGraph view

class MGTOOLS_API CIsolineGraph : public C2DGraph
{
protected:
	DECLARE_DYNCREATE(CIsolineGraph)

// Attributes
public:
	CIsolineGraph(LPCTSTR _iniFile=NULL);
	virtual ~CIsolineGraph();
    static CRuntimeClass* GetRunTime();

	void Set3DMatrix(float* _pMatr, int _nx, int _ny);
    void SetLimits(int iArea, double xMin, double xMax, double yMin, double yMax);
	void SetLevelsCount(int _nLevel);
	void SetLevelsRange(double minLevel, double maxLevel);
	void RestoreLevelsRange();
	void AddDrawLimits(int iArea, CDRect limR, int limType, double Anfle, int EllType);
    void AddDrawLimits(int iArea, CArray<CDPolyLine, CDPolyLine>& PolyLines);
    void CreatePalette();
    void RefreshPicture(BOOL key=FALSE);
    void SingleIsoline(int pn, ISO_POINT *plist, double level, int ilevel);

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIsolineGraph)
    public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

	
// Implementation
protected:
    void Delete();
	void DrawIsolines(CDC* pDC);
    void DrawLegend(CDC* pDC);
	
protected:
   double minZAct;
   double maxZAct;
   double minZLog;
   double maxZLog;
	double* pMatr;
	double* pMatrAct;
	double* pMatrLog;
	int nx;
	int ny;
    int nLevel;
	double* pLevel;
	double* pLevelAct;
	double* pLevelLog;
	double* pX;
	double*	pY;
    double minZ, maxZ;
    double InitialminZ, InitialmaxZ;
    int maxNLevels;
	HDC hDC;
    HPALETTE  hPal;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CIsolineGraph)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLogarithmScale();
	afx_msg void OnIsolineNumbers();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISOLINEGRAPH_H__03971D0D_CD62_4970_8026_C6321E4980C4__INCLUDED_)

#if !defined(AFX_MAPGRAPH_H__9494EFCA_4E26_432D_BF70_756F653ADC4B__INCLUDED_)
#define AFX_MAPGRAPH_H__9494EFCA_4E26_432D_BF70_756F653ADC4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapGraph.h : header file
//
#include "MGToolsExport.h"
#include "2DGraph.h"

/////////////////////////////////////////////////////////////////////////////
// CMapGraph view

class MGTOOLS_API CMapGraph : public C2DGraph
{
protected:
	DECLARE_DYNCREATE(CMapGraph)

public:
	CMapGraph(LPCTSTR _iniFile=NULL);
	virtual ~CMapGraph();
    static CRuntimeClass* GetRunTime();

	void Set3DMatrix(float* _pMatr, int _nx, int _ny);
    void SetLimits(int iArea, double xMin, double xMax, double yMin, double yMax);
    void AddDrawLimits(int iArea, CDRect limR, int limType,double Anfle, int EllType);
    void AddDrawLimits(int iArea, CArray<CDPolyLine, CDPolyLine>& PolyLines);
	void SetLevelsCount(int _nLevel);
    void CreatePalette();
    void RefreshPicture(BOOL key=FALSE);

public:
	
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
	double* pX;
	double*	pY;
    double minZ, maxZ;
    double view_minZ, view_maxZ;
    int nLevel;
	
    HPALETTE  hPal;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapGraph)
    public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Delete();
    COLORREF GetColor(int ix1, int iy1, int ix2, int iy2);
	void DrawSmoothingLegend(CDC* pDC);
    void DrawSmoothingMap(CDC* pDC);
	void DrawSteppedLegend(CDC* pDC);
    void DrawSteppedMap(CDC* pDC);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMapGraph)
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLogarithmScale();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnWriteMetafile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPGRAPH_H__9494EFCA_4E26_432D_BF70_756F653ADC4B__INCLUDED_)

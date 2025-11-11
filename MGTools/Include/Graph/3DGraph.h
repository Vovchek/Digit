#if !defined(AFX_3DGRAPH_H__51F9CAB5_1AE1_44D8_A0EF_DED713CCEA64__INCLUDED_)
#define AFX_3DGRAPH_H__51F9CAB5_1AE1_44D8_A0EF_DED713CCEA64__INCLUDED_
//0707
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DGraph.h : header file
//
#include "glut.h"
#include "2DGraph.h"
#include "MGToolsExport.h"

#define NURBS_SOLID   0x01
#define NURBS_WIRE    0x02
#define SOLID_COLOR   0x04
#define WIRE_COLOR    0x08
#define SOLID_ISOLINE 0x10
#define WIRE_ISOLINE  0x20

#define VIEW_MONOCHROM 0
#define VIEW_COLOR    1

void normalizedcross(GLfloat* u, GLfloat* v, GLfloat* n);

/////////////////////////////////////////////////////////////////////////////
// C3DGraph view

class MGTOOLS_API C3DGraph : public C2DGraph
{
protected:
	DECLARE_DYNCREATE(C3DGraph)

// Attributes
public:
	C3DGraph(LPCTSTR _iniFile=NULL);
	virtual ~C3DGraph();
    static CRuntimeClass* GetRunTime();

   void SetBackColor(COLORREF _bkColor);
   void SetBound(float _minX, float _maxX, float _minY, float _maxY);
   BOOL Set3DMatrix(float* _pMatr, int _nx, int _ny);
   void SetEnableMenuPlay(BOOL _EnableMenuPlay){EnableMenuPlay = _EnableMenuPlay;}
   void CreatePalette();
   void RefreshPicture();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DGraph)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
    UINT m_nTimer;
	BOOL RightMatrix;
    BOOL SetMemDcPixelFormat(HDC hMemDC, BOOL bUseAPI = FALSE);
    BOOL SetPixelFormat(HDC hdc);
    BOOL CreateGLContext(HDC hdc);
    void CreateMenu(CPoint P);
    void SetDefAngles();
	void DrawLights();
    void DrawText(BOOL Rot, float x, float y, float z, float scale, char *format, ...);
    void DrawAxis3D();
	void InitTriangSurface();
    void TriangSurfList();
	void TriangSurfListLog();
    void DrawTriangleSurf();
    void DrawLegend();
    void WriteText(int X, int Y, LPCTSTR Str, UINT Align);
	void Delete();
    void DrawPicture(CDC* pDC);
    void GetCurrent(HDC& hDC, HGLRC& hRC);
    BOOL SetCurrent(HDC hDC, HGLRC hRC);

protected:
	int m_DocPrintHeight;
	BOOL m_bStretchDIBits;
	BOOL m_bStretchBlt;
    HGLRC hrc;
	HDC   hDC;
    HPALETTE  mesh_pal;

	BOOL isInvertBkColor;
	BOOL EnablePlay;
	BOOL EnableMenuPlay;
	BOOL IsPointsList;
	BOOL IsNormalList;
	BOOL IsPointsListLog;
	BOOL IsNormalListLog;
	int  ViewColor;
	int WidthLegeng;


	GLfloat rotX, rotY, rotZ;
	GLfloat TransX, TransY, TransZ;
    GLfloat viewScale;
    GLfloat viewScaleZ;
    GLfloat ScaleXY;

    float LX,LY,LZ;    // ligts position
    float rotLight;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
    float realminZ, realmaxZ;

	GLfloat** pMatr;

    GLfloat*** normals;
    GLfloat*** ctlpoints;
    GLfloat*** facetnormals;
    int nx, ny;

	BOOL m_LButtonDown;
	CPoint m_LDownPos;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(C3DGraph)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnInvertBackground();
    afx_msg void OnColorView();
	afx_msg void OnMonochomView();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLogarithmScale();
	afx_msg void OnCopyClipboard();
	afx_msg void OnSaveAs();
	afx_msg void OnPlay();
    afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DGRAPH_H__51F9CAB5_1AE1_44D8_A0EF_DED713CCEA64__INCLUDED_)

#if !defined(AFX_2DGRAPH_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)
#define AFX_2DGRAPH_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 2DGraph.h : header file
//
const float F_EXCLUDE_VAL = 0.00005f;
const double D_EXCLUDE_VAL = 0.00005;

#define MAXDOUBLE   1.797693E+308
#define MAXFLOAT    3.37E+38F
#define MINDOUBLE   2.225074E-308
#define MINFLOAT    8.43E-37F
#define GRID_PARTIAL 0
#define GRID_FULL    1

#define VIEW_CLIENT   0
#define FRAME_CLIENT  1

#define MG_ENG_LANG  0
#define MG_RUS_LANG  1

#define WND_2D_GRAPH          (WM_USER+1000)
#define IDMR_InvertBackGround (WM_USER+1010)
#define IDMR_WriteMetaFile    (WM_USER+1011)
#define IDMR_CopyClipboard    (WM_USER+1012)
#define IDMR_ColorView        (WM_USER+1013)
#define IDMR_MonoChromView    (WM_USER+1014)
#define IDMR_LogarithmScale   (WM_USER+1015)
#define IDMR_IsolineNumbers   (WM_USER+1016)
#define IDMR_Play             (WM_USER+1017)
#define IDMR_Coordinates      (WM_USER+1018)

#include "Include\Utils\DibApi.h"
#include "DataType.h"
#include "MGToolsExport.h"
#include <math.h>

class CMPreviewView;
/////////////////////////////////////////////////////////////////////////////
// C2DGraph view

class MGTOOLS_API C2DGraph : public CScrollView
{
protected:
	DECLARE_DYNCREATE(C2DGraph)

// Attributes
public:
	C2DGraph(LPCTSTR _iniFile=NULL);
	virtual ~C2DGraph();
    void RemoveAllData();
    static CRuntimeClass* GetRunTime();

	void GetClientRect(CRect& ClientRect);
	void Enable0OnAxis(BOOL _need0OnAxis){need0OnAxis = _need0OnAxis;}
	void EnableLegend(BOOL _needLegend){needLegend = _needLegend;}
	void SetClientWnd(int _ClientWnd){ClientWnd = _ClientWnd;}
	void SetBackColor(COLORREF _bkColor){bkColor = _bkColor;}
    void SetWtoHRatio(double _WtoHRatio);
	void SetEnablePreView(BOOL _EnablePreView){EnablePreView =_EnablePreView;}
    void SetRegionNumber(int totalReg);
	void SetkYForPrint(double _kY){kY = _kY;}
	void SetRegion(int iArea, double left, double top, double right, double bottom);
	void RemoveAllDrawLimits();
    void AddDrawLimits(int iArea, CDRect limR, int limType, double Anfle, int EllType);
    void AddDrawLimits(int iArea, CArray<CDPolyLine, CDPolyLine>& PolyLines);
    void SetLimits(int iArea, double xMin, double xMax, double yMin, double yMax);
    void SetGrid(int iArea, double Ybeg, double Xbeg, double Ux,  double Uy,
					   int nx, int ny, CString XName="",  CString YName="",
					   CFont* pFont=0,  UINT YAlign=TA_RIGHT|TA_TOP, UINT XAlign=TA_RIGHT|TA_TOP);
    void SetGridColor(int iArea, COLORREF Yaxis, COLORREF Ytext,
					             COLORREF Xaxis=RGB(255,255,255), COLORREF Xtext=RGB(255,255,255));
    void SetGridDrawStatus(int iArea, BOOL isXAxis, BOOL isYAxis, BOOL isDotGrid);
    void RemoveAllLines(int iArea);
    void RemoveAllEllipses(int iArea);
    void AddLine(int iArea, COLORREF Color, int Width, int nP, double* pX, double* pY, 
		         LPCTSTR _LegendText=NULL, LPCTSTR format=NULL);
    void AddEllipse(int iArea, COLORREF Color, int Width, double x1, double y1, double x2, double y2, 
		            LPCTSTR _LegendText=NULL, LPCTSTR format=NULL);
    void AddValueText(int iArea, double x, double y, double val, UINT Align=TA_LEFT|TA_BOTTOM);
    void AddText(int iArea, double x, double y, LPCTSTR text, UINT Align=TA_RIGHT|TA_BOTTOM);
    void AddSmithDiagramVal(int iArea, int nP,
  			    			  double* pFreq, double* pRe, double* pIm);
    void RefreshPicture();
	void RestoreSettings();

	void SetLangType(int iLang);
    void SetPathForSave(LPCTSTR _path);
    CRect GetRegion(int iArea);
	
	friend class CMPreviewView;
// Operations

public:
   CString PathForSave;	
   int LangType;
   BOOL bRender;
   BOOL isEnableCoor;
   void PrintHeader(CDC* pDC, CPrintInfo* pInfo, CString strHeader);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C2DGraph)
	public:
	virtual void OnInitialUpdate();
    virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL


// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
   int ClientWnd;	
   LPCTSTR iniFile;
   BOOL isPrinting;
   BOOL EnablePreView;
   BOOL EnableLogarithmScale;
   BOOL isWMF;
   BOOL isIsolineNumbers;
   BOOL EnableIsolineNumbers;
   BOOL isCoordinates;
   CRect TotalPrintArea;
   COLORREF bkColor;
   CArray<DRAW_AREA, DRAW_AREA> Areas;
   int ReginRow;
   double kY;
   CPoint XAxisNamePos;
   CPoint YAxisNamePos;
   CToolTipCtrl m_ToolTips;
   double WtoHRatio;
   double LogDelta;
   double Coef_z_Log;
   double LogShift;
   int maxNLevels;
   BOOL needLegend;
   BOOL need0OnAxis;

   BOOL ConvertPointToWMFCoord(CPoint& P, CDC* pDC);
   BOOL ConvertRectToWMFCoord(CRect& R, CDC* pDC);
   void MyHiMetrictoDP(CDC* pDC, CPoint* pt);
   void MyDPtoHiMetric(CDC* pDC, CPoint* pt);
   void OnMetaFileDraw(CDC* pDC);
   void DrawLegend(CDC* pDC);

// Axes
    inline double sign(double x) {return (x!=0.)? x/fabs(x) : 1.;}
	double _SetBigStep(double s_old);
    void  _DelAxs(double xmn, double xmx, double *masstab, char *text);
    void _Nprob(char *text);
    void _razm_x_for(CDC *pGrp, int iArea, double masstab, double x, double xmax,
         double dx, int nx, double y, unsigned LC, char *Format);
    void _razm_y_for(CDC *pDC, int iArea, double masstab, double y, double ymax,
         double dy, int ny, double x, unsigned LC, char *Format, UINT Align);

	void InitPlotMode(CDC* pDC, int iArea);
	double AxesX(CDC* pDC, int iArea, double Ybeg, double Ux, int nx);
    double AxesY(CDC *pDC, int iArea, double Xbeg, double Uy, int ny, UINT Align=TA_RIGHT|TA_TOP);
    void AxesGridX(CDC *pDC, int iArea, double Ux, int Type=GRID_PARTIAL);
    void AxesGridY(CDC *pDC, int iArea, double Uy, int Type=GRID_PARTIAL);
    void DotLineX(CDC *pDC, int iArea, double Y, double X1, double X2, COLORREF Color, BOOL FullLine);
    void DotLineY(CDC *pDC, int iArea, double Y, double X1, double X2, COLORREF Color, BOOL FullLine);
// end Axes
	CRect GetPrintRegion(CDC* pDC, int iArea);
    void WriteText(CDC* pDC, int X, int Y, LPCTSTR Str, UINT Align);
	void WriteTextInfo(CDC* pDC, int iArea);
    void Line(CDC* pDC, int iArea, double x, double y, BOOL IsVisible, BOOL IsCliped);
    void Ellipse(CDC* pDC, int iArea, double x1, double y1, double x2, double y2);
    void MatToWndCoor(int iArea, double x, double y, long int& xC, long int& yC);
    void WndCoorToMat(int iArea, CPoint P, double &x, double &y);
    void SetClipReg(CDC *pDC, int iArea, BOOL IsSet);

	BOOL GetToolText(CString& text);
    BOOL GetNearestMatPoint(int iArea, double& x, double& y, CString& s);
    BOOL GetSDNearestMatPoint(int iArea, double& x, double& y,
		double& F, double& Re, double& Im, CString& format);
    HENHMETAFILE GetMetafile(LPCTSTR FileName);

	void InitPath(CDC* pDC);
	void DeInitPath(CDC* pDC);
    void StrokeDrawPath(CDC* pDC);

    void InitPathLimitPolylines(CDC* pDC);
	void StrokeLimitPolylines(CDC* pDC);
    void DeInitPathLimitPolylines(CDC* pDC);
	
    void CreateMenu(CPoint P);
    void OnInvertBackground();
	void WriteMetafile(bool IncludeWMF=true);
    void OnCopy();

	//{{AFX_MSG(C2DGraph)
    afx_msg BOOL DoPrintPreview(UINT nIDResource, CScrollView* pPrintView,
    CRuntimeClass* pPreviewViewClass, CPrintPreviewState* pState);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	void OnFilePrintPreview();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLogarithmScale();
	afx_msg void OnIsolineNumbers();
	afx_msg void OnCoordinates();
	afx_msg void OnWriteMetafile();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CMPreviewView : public CPreviewView
{
  DECLARE_DYNCREATE(CMPreviewView)

  public:
    afx_msg void OnPreviewPrint();
    afx_msg void OnPreviewClose();
  protected:
    friend class C2DGraph;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_2DGRAPH_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)

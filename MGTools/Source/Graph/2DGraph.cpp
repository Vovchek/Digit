// 2DGraph.cpp : implementation file
//
#include "..\..\Include\Graph\def.h"
#include "..\..\Include\Graph\2DGraph.h"
#include "..\..\Include\Utils\Utils.h"
#include "..\..\Include\Graph\GraphTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFrameWnd* pFr=NULL;
static RGBQUAD rgbStd256[] =
{
    {   0,  0,  0, 0 }, {   0,  0,128, 0 }, {   0,128,  0, 0 }, {   0,128,128, 0 },
    { 128,  0,  0, 0 }, { 128,  0,128, 0 }, { 128,128,  0, 0 }, { 192,192,192, 0 },
    { 192,220,192, 0 }, { 240,202,166, 0 }, { 238,238,238, 0 }, { 221,221,221, 0 },
    { 204,204,204, 0 }, { 187,187,187, 0 }, { 170,170,170, 0 }, { 153,153,153, 0 },
    { 136,136,136, 0 }, { 119,119,119, 0 }, { 102,102,102, 0 }, {  85, 85, 85, 0 },
    {  68, 68, 68, 0 }, {  51, 51, 51, 0 }, {  34, 34, 34, 0 }, {  17, 17, 17, 0 },
    { 204,255,255, 0 }, { 153,255,255, 0 }, { 102,255,255, 0 }, {  51,255,255, 0 },
    { 255,204,255, 0 }, { 204,204,255, 0 }, { 153,204,255, 0 }, { 102,204,255, 0 },
    {  51,204,255, 0 }, {   0,204,255, 0 }, { 255,153,255, 0 }, { 204,153,255, 0 },
    { 153,153,255, 0 }, { 102,153,255, 0 }, {  51,153,255, 0 }, {   0,153,255, 0 },
    { 255,102,255, 0 }, { 204,102,255, 0 }, { 153,102,255, 0 }, { 102,102,255, 0 },
    {  51,102,255, 0 }, {   0,102,255, 0 }, { 255, 51,255, 0 }, { 204, 51,255, 0 },
    { 153, 51,255, 0 }, { 102, 51,255, 0 }, {  51, 51,255, 0 }, {   0, 51,255, 0 },
    { 204,  0,255, 0 }, { 153,  0,255, 0 }, { 102,  0,255, 0 }, {  51,  0,255, 0 },
    { 255,255,204, 0 }, { 204,255,204, 0 }, { 153,255,204, 0 }, { 102,255,204, 0 },
    {  51,255,204, 0 }, {   0,255,204, 0 }, { 255,204,204, 0 }, { 153,204,204, 0 },
    { 102,204,204, 0 }, {  51,204,204, 0 }, {   0,204,204, 0 }, { 255,153,204, 0 },
    { 204,153,204, 0 }, { 153,153,204, 0 }, { 102,153,204, 0 }, {  51,153,204, 0 },
    {   0,153,204, 0 }, { 255,102,204, 0 }, { 204,102,204, 0 }, { 153,102,204, 0 },
    { 102,102,204, 0 }, {  51,102,204, 0 }, {   0,102,204, 0 }, { 255, 51,204, 0 },
    { 204, 51,204, 0 }, { 153, 51,204, 0 }, { 102, 51,204, 0 }, {  51, 51,204, 0 },
    {   0, 51,204, 0 }, { 255,  0,204, 0 }, { 204,  0,204, 0 }, { 153,  0,204, 0 },
    { 102,  0,204, 0 }, {  51,  0,204, 0 }, { 255,255,153, 0 }, { 204,255,153, 0 },
    { 153,255,153, 0 }, { 102,255,153, 0 }, {  51,255,153, 0 }, {   0,255,153, 0 },
    { 255,204,153, 0 }, { 204,204,153, 0 }, { 153,204,153, 0 }, { 102,204,153, 0 },
    {  51,204,153, 0 }, {   0,204,153, 0 }, { 255,153,153, 0 }, { 204,153,153, 0 },
    { 102,153,153, 0 }, {  51,153,153, 0 }, {   0,153,153, 0 }, { 255,102,153, 0 },
    { 204,102,153, 0 }, { 153,102,153, 0 }, { 102,102,153, 0 }, {  51,102,153, 0 },
    {   0,102,153, 0 }, { 255, 51,153, 0 }, { 204, 51,153, 0 }, { 153, 51,153, 0 },
    { 102, 51,153, 0 }, {  51, 51,153, 0 }, {   0, 51,153, 0 }, { 255,  0,153, 0 },
    { 204,  0,153, 0 }, { 153,  0,153, 0 }, { 102,  0,153, 0 }, {  51,  0,153, 0 },
    { 255,255,102, 0 }, { 204,255,102, 0 }, { 153,255,102, 0 }, { 102,255,102, 0 },
    {  51,255,102, 0 }, {   0,255,102, 0 }, { 255,204,102, 0 }, { 204,204,102, 0 },
    { 153,204,102, 0 }, { 102,204,102, 0 }, {  51,204,102, 0 }, {   0,204,102, 0 },
    { 255,153,102, 0 }, { 204,153,102, 0 }, { 153,153,102, 0 }, { 102,153,102, 0 },
    {  51,153,102, 0 }, {   0,153,102, 0 }, { 255,102,102, 0 }, { 204,102,102, 0 },
    { 153,102,102, 0 }, {  51,102,102, 0 }, {   0,102,102, 0 }, { 255, 51,102, 0 },
    { 204, 51,102, 0 }, { 153, 51,102, 0 }, { 102, 51,102, 0 }, {  51, 51,102, 0 },
    {   0, 51,102, 0 }, { 255,  0,102, 0 }, { 204,  0,102, 0 }, { 153,  0,102, 0 },
    { 102,  0,102, 0 }, {  51,  0,102, 0 }, { 255,255, 51, 0 }, { 204,255, 51, 0 },
    { 153,255, 51, 0 }, { 102,255, 51, 0 }, {  51,255, 51, 0 }, {   0,255, 51, 0 },
    { 255,204, 51, 0 }, { 204,204, 51, 0 }, { 153,204, 51, 0 }, { 102,204, 51, 0 },
    {  51,204, 51, 0 }, {   0,204, 51, 0 }, { 255,153, 51, 0 }, { 204,153, 51, 0 },
    { 153,153, 51, 0 }, { 102,153, 51, 0 }, {  51,153, 51, 0 }, {   0,153, 51, 0 },
    { 255,102, 51, 0 }, { 204,102, 51, 0 }, { 153,102, 51, 0 }, { 102,102, 51, 0 },
    {  51,102, 51, 0 }, {   0,102, 51, 0 }, { 255, 51, 51, 0 }, { 204, 51, 51, 0 },
    { 153, 51, 51, 0 }, { 102, 51, 51, 0 }, {   0, 51, 51, 0 }, { 255,  0, 51, 0 },
    { 204,  0, 51, 0 }, { 153,  0, 51, 0 }, { 102,  0, 51, 0 }, {  51,  0, 51, 0 },
    { 204,255,  0, 0 }, { 153,255,  0, 0 }, { 102,255,  0, 0 }, {  51,255,  0, 0 },
    { 255,204,  0, 0 }, { 204,204,  0, 0 }, { 153,204,  0, 0 }, { 102,204,  0, 0 },
    {  51,204,  0, 0 }, { 255,153,  0, 0 }, { 204,153,  0, 0 }, { 153,153,  0, 0 },
    { 102,153,  0, 0 }, {   0,  0,238, 0 }, {   0,  0,221, 0 }, {   0,  0,204, 0 },
    {   0,  0,187, 0 }, {   0,  0,170, 0 }, {   0,  0,153, 0 }, {   0,  0,136, 0 },
    {   0,  0,119, 0 }, {   0,  0,102, 0 }, {   0,  0, 85, 0 }, {   0,  0, 68, 0 },
    {   0,  0, 51, 0 }, {   0,  0, 34, 0 }, {   0,  0, 17, 0 }, {   0,238,  0, 0 },
    {   0,221,  0, 0 }, {   0,204,  0, 0 }, {   0,187,  0, 0 }, {   0,170,  0, 0 },
    {   0,153,  0, 0 }, {   0,136,  0, 0 }, {   0,119,  0, 0 }, {   0,102,  0, 0 },
    {   0, 85,  0, 0 }, {   0, 68,  0, 0 }, {   0, 51,  0, 0 }, {   0, 34,  0, 0 },
    {   0, 17,  0, 0 }, { 238,  0,  0, 0 }, { 221,  0,  0, 0 }, { 204,  0,  0, 0 },
    { 187,  0,  0, 0 }, { 170,  0,  0, 0 }, { 153,  0,  0, 0 }, { 136,  0,  0, 0 },
    { 119,  0,  0, 0 }, { 102,  0,  0, 0 }, {  85,  0,  0, 0 }, {  68,  0,  0, 0 },
    {  51,  0,  0, 0 }, {  34,  0,  0, 0 }, { 240,251,255, 0 }, { 164,160,160, 0 },
    { 128,128,128, 0 }, {   0,  0,255, 0 }, {   0,255,  0, 0 }, {   0,255,255, 0 },
    { 255,  0,  0, 0 }, { 255,  0,255, 0 }, { 255,255,  0, 0 }, { 255,255,255, 0 }
};

// Aldus Placeable Header ===================================================
// Since we are a 32bit app, we have to be sure this structure compiles to
// be identical to a 16 bit app's version. To do this, we use the #pragma
// to adjust packing, we use a WORD for the hmf handle, and a SMALL_RECT
// for the bbox rectangle.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    DWORD       dwKey;
    WORD        hmf;
    SMALL_RECT  bbox;
    WORD        wInch;
    DWORD       dwReserved;
    WORD        wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )
// ==========================================================================
WORD CalculateAPMCheckSum( APMHEADER apmfh )
{
    LPWORD  lpWord;
    WORD    wResult, i;
    
    // Start with the first word
    wResult = *(lpWord = (LPWORD)(&apmfh));
    // XOR in each of the other 9 words
    for(i=1;i<=9;i++)
    {
        wResult ^= lpWord[i];
    }
    return wResult;
}
BOOL WriteEnhMetaFileAsPlaceable(HENHMETAFILE hMeta, LPCTSTR szFileName)
{
    APMHEADER       APMHeader;
    ENHMETAHEADER   emh;
    HDC             hDC;
    DWORD           dwSize, dwBytes;
    LPBYTE          pBits;
    HANDLE          hFile;

    // Initialize the header
    ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
    emh.nSize = sizeof(ENHMETAHEADER);
    // Fill in the enhanced metafile header
    if( GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh ) == 0 )
        return FALSE;

    // Fill in the Aldus Placeable Header
    APMHeader.dwKey = 0x9ac6cdd7l;
    APMHeader.hmf = 0;
    APMHeader.bbox.Top = static_cast<SHORT>(1000 * emh.rclFrame.top/2540);
    APMHeader.bbox.Left = static_cast<SHORT>(1000 * emh.rclFrame.left/2540);
    APMHeader.bbox.Right = static_cast<SHORT>(1000 * emh.rclFrame.right/2540);
    APMHeader.bbox.Bottom = static_cast<SHORT>(1000 * emh.rclFrame.bottom/2540);
    APMHeader.wInch = 1000;
    APMHeader.dwReserved = 0;
    APMHeader.wCheckSum = CalculateAPMCheckSum( APMHeader );

    // We need a reference DC
    hDC = GetDC( NULL );
    // How big are the windows metafile bits going to be?
    dwSize = GetWinMetaFileBits( hMeta, 0, NULL, MM_ANISOTROPIC, hDC  );
    // Allocate that much memory
    pBits = (LPBYTE)malloc( dwSize );
    // Let Windows convert our enhanced metafile to a regular windows metafile
    GetWinMetaFileBits( hMeta, dwSize, pBits, MM_ANISOTROPIC, hDC );
    // Release the reference DC
    ReleaseDC( NULL, hDC );

    // Open the file
    hFile = CreateFile((LPTSTR)szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    // Write the Aldus Placeable Header
    WriteFile( hFile, &APMHeader, sizeof(APMHEADER), &dwBytes, NULL );
    // Write the metafile bits
    WriteFile( hFile, pBits, dwSize, &dwBytes, NULL );

    // Clean up
    free( pBits );
    CloseHandle( hFile );

    return TRUE;
}

IMPLEMENT_DYNCREATE(CMPreviewView, CPreviewView)

BEGIN_MESSAGE_MAP(CMPreviewView, CPreviewView)
	//{{AFX_MSG_MAP(CMPreviewView)
    ON_COMMAND(AFX_ID_PREVIEW_PRINT, OnPreviewPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMPreviewView::OnPreviewClose()
{
	CPreviewView::OnPreviewClose();
}

void CMPreviewView::OnPreviewPrint()
{
    OnPreviewClose();               // force close of Preview

    ASSERT_VALID(pFr);
    pFr->SendMessage(WM_COMMAND, ID_FILE_PRINT);
}

BOOL CALLBACK _PreviewCloseProc(CFrameWnd* pFrameWnd);

BOOL CALLBACK _PreviewCloseProc(CFrameWnd* pFrameWnd)
{
    ASSERT_VALID(pFrameWnd);

    CMPreviewView* pView = (CMPreviewView*) pFrameWnd->GetDlgItem(AFX_IDW_PANE_FIRST);
    ASSERT_KINDOF(CMPreviewView, pView);

    pView->OnPreviewClose();
    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// C2DGraph

IMPLEMENT_DYNCREATE(C2DGraph, CScrollView)

void C2DGraph::SetLangType(int iLang)
{
	LangType = iLang;
}

void C2DGraph::SetPathForSave(LPCTSTR _path)
{
	PathForSave = _path;
}

C2DGraph::C2DGraph(LPCTSTR _iniFile/*NULL*/)
{
    iniFile = _iniFile;
	ReginRow = 1;
	kY = 1.;
	bkColor = RGB(0,0,0);
	isPrinting = isWMF = FALSE;
	XAxisNamePos = CPoint(I_EMPTY, I_EMPTY);
	YAxisNamePos = CPoint(I_EMPTY, I_EMPTY);
	TotalPrintArea = CRect(0,0,0,0);
	WtoHRatio = 0;
    LogDelta= 1.e-5;
    Coef_z_Log = 0;
    LogShift = 0;
	maxNLevels = 20;
	need0OnAxis = TRUE;
	needLegend = FALSE;
	EnablePreView = FALSE;
	EnableLogarithmScale = FALSE;
    isIsolineNumbers = FALSE;
    EnableIsolineNumbers = FALSE;
    isCoordinates = FALSE;
	ClientWnd = VIEW_CLIENT;
    bRender = FALSE;
    LangType = MG_ENG_LANG;
    isEnableCoor = TRUE;
}

C2DGraph::~C2DGraph()
{
}

void C2DGraph::OnDestroy()
{
    RemoveAllData();
    CScrollView::OnDestroy();
}

CRuntimeClass* C2DGraph::GetRunTime()
{
	return RUNTIME_CLASS(C2DGraph);
}

BEGIN_MESSAGE_MAP(C2DGraph, CScrollView)
	//{{AFX_MSG_MAP(C2DGraph)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
    ON_WM_SETCURSOR()
	ON_COMMAND(IDMR_InvertBackGround, OnInvertBackground)
	ON_COMMAND(IDMR_WriteMetaFile, OnWriteMetafile)
	ON_COMMAND(IDMR_CopyClipboard, OnCopy)
	ON_COMMAND(IDMR_LogarithmScale, OnLogarithmScale)
//	ON_COMMAND(IDMR_IsolineNumbers, OnIsolineNumbers)
	ON_COMMAND(IDMR_Coordinates, OnCoordinates)
	//}}AFX_MSG_MAP
    ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

void C2DGraph::RemoveAllData()
{
	Areas.RemoveAll();
}

void C2DGraph::SetWtoHRatio(double _WtoHRatio)
{
	WtoHRatio = _WtoHRatio;
}

void C2DGraph::SetRegionNumber(int totalReg)
{
    RemoveAllData();
	Areas.SetSize(totalReg);
	for(int i=0; i < Areas.GetSize(); i++){
		Areas[i].Init();
	}
}

void C2DGraph::SetRegion(int iArea, double left, double top, double right, double bottom)
{
	Areas[iArea].Region.left = left;
    Areas[iArea].Region.top = top;
    Areas[iArea].Region.right = right;
    Areas[iArea].Region.bottom = bottom;
}

void C2DGraph::SetLimits(int iArea, double xMin, double xMax, double yMin, double yMax)
{
    if(fabs(yMax-yMin) <= 0.00000001) {
      yMin = yMin - 0.0000001;
      yMax = yMax + 0.0000001;
  }
	Areas[iArea].Limits.xMin = xMin;
    Areas[iArea].Limits.xMax = xMax;
    Areas[iArea].Limits.yMin = yMin;
    Areas[iArea].Limits.yMax = yMax;
}

void C2DGraph::RemoveAllDrawLimits()
{
	for(int i=0; i < Areas.GetSize(); i++){
		Areas[i].DrawLimits.RemoveAll();
	}
}

void C2DGraph::AddDrawLimits(int iArea, CDRect limR, int limType, double Angle, int EllType)
{
	DRAW_LIMITS Limit;
	Limit.limR = limR;
    Limit.limType = limType;
	Limit.Angle = Angle;
	Limit.EllType = EllType;

	Areas[iArea].DrawLimits.Add(Limit);
}

void C2DGraph::AddDrawLimits(int iArea, CArray<CDPolyLine, CDPolyLine>& PolyLines)
{
	Areas[iArea].DrawLimitPolylines.RemoveAll();
	Areas[iArea].DrawLimitPolylines.Append(PolyLines);
}

void C2DGraph::SetGrid(int iArea, double Ybeg, double Xbeg, double Ux,  double Uy,
					   int nx, int ny, CString XName,  CString YName,
					   CFont* pFont, UINT YAlign, UINT XAlign)
{
	Areas[iArea].Grid.Ybeg = Ybeg;
    Areas[iArea].Grid.Xbeg = Xbeg;
	Areas[iArea].Grid.Ux = Ux;
	Areas[iArea].Grid.Uy = Uy;
	Areas[iArea].Grid.nx = nx;
	Areas[iArea].Grid.ny = ny;
	Areas[iArea].Grid.pFont = pFont;
	Areas[iArea].Grid.XAlign = XAlign;
	Areas[iArea].Grid.YAlign = YAlign;
	Areas[iArea].Grid.XName = XName;
	Areas[iArea].Grid.YName = YName;
}

void C2DGraph::SetGridColor(int iArea, COLORREF Yaxis, COLORREF Ytext,
							           COLORREF Xaxis, COLORREF Xtext)
{
    COLORREF axColor;
	if(bkColor == RGB(0,0,0)){
		axColor = RGB(255,255,255);
	}
	else{
		axColor = RGB(0,0,0);
	}

	Areas[iArea].Grid.YpenColor = axColor;
    Areas[iArea].Grid.YtextColor = axColor;
    Areas[iArea].Grid.XpenColor = axColor;
    Areas[iArea].Grid.XtextColor = axColor;
}

void C2DGraph::SetGridDrawStatus(int iArea, BOOL isXAxis, BOOL isYAxis, BOOL isDotGrid)
{
	Areas[iArea].Grid.isXAxis = isXAxis;
    Areas[iArea].Grid.isYAxis = isYAxis;
	Areas[iArea].Grid.isDotGrid = isDotGrid;
}

void C2DGraph::AddLine(int iArea, COLORREF Color, int Width,
					   int nP, double* pX, double* pY,
					   LPCTSTR _LegendText, LPCTSTR format)
{
	DRAW_LINE Line;
	Line.penColor = Color;
	Line.penWidth = Width;
	Line.X.SetSize(nP);
	Line.Y.SetSize(nP);
	for(int i=0; i < nP; i++){
		Line.X[i] = pX[i];
		Line.Y[i] = pY[i];
	}
	if(_LegendText){
	   Line.LegendText = _LegendText;
	}
	if(format){
       Line.ToolTipsFormat = format;
	   Areas[iArea].EnableToolTips = TRUE;
	}
	Areas[iArea].Lines.Add(Line);
}

void C2DGraph::RemoveAllLines(int iArea)
{
	Areas[iArea].Lines.RemoveAll();
}

void C2DGraph::AddEllipse(int iArea, COLORREF Color, int Width, 
						  double x1, double y1, double x2, double y2, 
				          LPCTSTR _LegendText, LPCTSTR format)
{
	DRAW_ELLIPSE Ellipse;
	Ellipse.penColor = Color;
	Ellipse.penWidth = Width;
	Ellipse.x1 = x1;
	Ellipse.y1 = y1;
	Ellipse.x2 = x2;
	Ellipse.y2 = y2;

	if(_LegendText){
		Ellipse.LegendText = _LegendText;
	}
	if(format){
		Ellipse.ToolTipsFormat = format;
		Areas[iArea].EnableToolTips = TRUE;
	}
	Areas[iArea].Ellipses.Add(Ellipse);
}

void C2DGraph::RemoveAllEllipses(int iArea)
{
	Areas[iArea].Ellipses.RemoveAll();
}

void C2DGraph::AddSmithDiagramVal(int iArea, int nP,
								  double* pFreq, double* pRe, double* pIm)
{
	Areas[iArea].arFreq.SetSize(nP);
	Areas[iArea].arRe.SetSize(nP);
	Areas[iArea].arIm.SetSize(nP);
	for(int i=0; i < nP; i++){
		Areas[iArea].arFreq[i] = pFreq[i];
		Areas[iArea].arRe[i] = pRe[i];
		Areas[iArea].arIm[i] = pIm[i];
	}
}

void C2DGraph::AddValueText(int iArea, double x, double y, double val, UINT Align)
{
	TEXT_POSITION Text;
	Text.x = x;
	Text.y = y;
	Text.val = val;
	Text.Align = Align;
	Areas[iArea].Texts.Add(Text);
}

void C2DGraph::AddText(int iArea, double x, double y, LPCTSTR text, UINT Align)
{
	TEXT_POSITION Text;
	Text.x = x;
	Text.y = y;
	Text.text = text;
	Text.Align = Align;
	Areas[iArea].Texts.Add(Text);
}

/////////////////////////////////////////////////////////////////////////////
// C2DGraph drawing
void C2DGraph::GetClientRect(CRect& clientR)
{
	   if(ClientWnd == VIEW_CLIENT){
		   CScrollView::GetClientRect(&clientR);
	   }
	   else if(ClientWnd == FRAME_CLIENT){
	     CFrameWnd* pFr = GetParentFrame();
         pFr->GetClientRect(&clientR);
	   }
}

void C2DGraph::OnDraw(CDC* pDC)
{
    CRect clientR;
    if(!bRender){
        GetClientRect(clientR);
        pDC->FillRect(&clientR, &CBrush(RGB(255,255,255)));
        CPen colPen;
        colPen.CreatePen(PS_SOLID, 0, RGB(255,0,0));
        CPen* _colPen = pDC->SelectObject(&colPen);
		CPoint TL = clientR.TopLeft();
		CPoint BR = clientR.BottomRight();
        pDC->MoveTo(TL);
        pDC->LineTo(BR);
        pDC->MoveTo(TL.x, BR.y);
        pDC->LineTo(BR.x, TL.y);
        CPen* retPen = pDC->SelectObject(_colPen);
        if(retPen) retPen->DeleteObject();
        return;
    }
    
    if(pDC->IsPrinting())
      isPrinting = TRUE;
    else
      isPrinting = FALSE;

    CDC dcMem;
    CDC* pDrawDC=NULL;
    CPen* oldPen=NULL;
    CPen* retPen=NULL;
    CFont* oldFont=NULL;
    CBitmap *pBitmap=NULL;

    CBrush BrushWnd(bkColor);
    CPoint AxisNamePos;

    if(isPrinting){
       clientR = TotalPrintArea;
       pDrawDC = pDC;
    }
    else if(isWMF){
       clientR = TotalPrintArea;
       pDrawDC = pDC;
    }
    else{
       GetClientRect(clientR);
       pDrawDC = &dcMem;
    }

    if(pDrawDC != pDC){
      pDrawDC->CreateCompatibleDC(pDC);
      GetClientRect(clientR);
      CBitmap bitmap;
      bitmap.CreateCompatibleBitmap(pDC, clientR.right, clientR.bottom);
      pBitmap = pDrawDC->SelectObject(&bitmap);
    }

    CSize sizeTotal(clientR.Width(), clientR.Height());
    pDC->FillRect(clientR, &BrushWnd);
    pDrawDC->FillRect(clientR, &BrushWnd);
    
    int obkMode{TRANSPARENT};
    GRID_VIEW grid;

  CFont* pFont=NULL;     
  if(Areas.GetSize()){
    InitPlotMode(pDrawDC, 0);
    obkMode = pDrawDC->SetBkMode(TRANSPARENT);
    
    CPen defPen;
    defPen.CreatePen(PS_SOLID, 0, RGB(0,0,0));
    oldPen = pDrawDC->SelectObject(&defPen);
      grid = Areas[0].Grid;
      if(grid.pFont==0){
          pFont = new CFont;
          if(isWMF){
              LOGFONT lf;
              memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
              lf.lfHeight = 400;                      // request a 12-pixel-height font
              strcpy(lf.lfFaceName, "Arial");        // request a face name "Arial"
              pFont->CreateFontIndirect(&lf);  // create the font
          }
          else
              pFont->CreatePointFont(70, "Arial", pDrawDC);
      }
	  else
		 pFont = grid.pFont; 
    }
    if(pFont)
      oldFont = pDrawDC->SelectObject(pFont);
    
    for(int i=0; i < Areas.GetSize(); i++){
      grid = Areas[i].Grid;
      if(i>0)
          InitPlotMode(pDrawDC, i);
      pDrawDC->SetTextColor(grid.XtextColor);
      CPen XgridPen;
      XgridPen.CreatePen(grid.penType, grid.penWidth, grid.XpenColor);
      retPen = pDrawDC->SelectObject(&XgridPen);
      if(retPen) retPen->DeleteObject();
      double Dx = AxesX(pDrawDC, i, grid.Ybeg, grid.Ux, grid.nx);
      LPCTSTR xName =  LPCTSTR(Areas[i].Grid.XName);
      if(xName){
        MatToWndCoor(i, Areas[i].Limits.xMax, grid.Ybeg, AxisNamePos.x, AxisNamePos.y);
        AxisNamePos.x += 5;    // 28.03.2006
        AxisNamePos.y -= 7;    // 28.03.2006
        WriteText(pDrawDC, AxisNamePos.x, AxisNamePos.y, xName, TA_LEFT|TA_TOP);
      }

      CPen YgridPen;
            if(bkColor == RGB(0,0,0)){
             YgridPen.CreatePen(grid.penType, grid.penWidth, RGB(255,255,255));
             pDrawDC->SetTextColor(RGB(255,255,255));
            }
            else{
             YgridPen.CreatePen(grid.penType, grid.penWidth, RGB(0,0,0));
             pDrawDC->SetTextColor(RGB(0,0,0));
            }
      retPen = pDrawDC->SelectObject(&YgridPen);
      if(retPen) retPen->DeleteObject();
      double Dy = AxesY(pDrawDC, i, grid.Xbeg, grid.Uy, grid.ny, grid.YAlign);
      LPCTSTR yName =  LPCTSTR(Areas[i].Grid.YName);
      if(yName){
          MatToWndCoor(i, grid.Xbeg, Areas[i].Limits.yMax, AxisNamePos.x, AxisNamePos.y);
          AxisNamePos.x -= 15;    // 28.03.2006
          AxisNamePos.y += 10;    // 28.03.2006
          WriteText(pDrawDC, AxisNamePos.x, AxisNamePos.y, yName, TA_LEFT|TA_BOTTOM);
      }
      if(grid.isDotGrid){
          AxesGridX(pDrawDC, i, Dx);
          AxesGridY(pDrawDC, i, Dy);
      }
/*
      if(isPrinting || isWMF){
          pDrawDC->SetTextColor(RGB(0,0,0));
          CPen XgridPen;
          XgridPen.CreatePen(grid.penType, grid.penWidth, RGB(0,0,0));
          retPen = pDrawDC->SelectObject(&XgridPen);
          if(retPen) retPen->DeleteObject();
          double Dx = AxesX(pDrawDC, i, grid.Ybeg, grid.Ux, grid.nx);

          double Dy = AxesY(pDrawDC, i, grid.Xbeg, grid.Uy, grid.ny, grid.YAlign);
          LPCTSTR yName =  LPCTSTR(Areas[i].Grid.YName);
      }
*/
      for(int iL=0; iL < Areas[i].Lines.GetSize(); iL++){
          CPen linePen;
          linePen.CreatePen(Areas[i].Lines[iL].penType,Areas[i].Lines[iL].penWidth,Areas[i].Lines[iL].penColor);
          retPen = pDrawDC->SelectObject(&linePen);
          if(retPen) retPen->DeleteObject();
          for(int iP=0; iP < Areas[i].Lines[iL].X.GetSize(); iP++){
           if(iP==0)
             Line(pDrawDC, i, Areas[i].Lines[iL].X[iP], Areas[i].Lines[iL].Y[iP], FALSE, TRUE);
           else if(Areas[i].Lines[iL].Y[iP-1] == D_EXCLUDE_VAL || Areas[i].Lines[iL].Y[iP] == D_EXCLUDE_VAL ||
                   Areas[i].Lines[iL].Y[iP-1] == F_EXCLUDE_VAL || Areas[i].Lines[iL].Y[iP] == F_EXCLUDE_VAL)
             Line(pDrawDC, i, Areas[i].Lines[iL].X[iP], Areas[i].Lines[iL].Y[iP], FALSE, TRUE);
           else
             Line(pDrawDC, i, Areas[i].Lines[iL].X[iP], Areas[i].Lines[iL].Y[iP], TRUE, TRUE);
          }
      }
      for(int iE=0; iE < Areas[i].Ellipses.GetSize(); iE++)
	  {
		  if (iE == 0)
			  pDrawDC->SelectStockObject(HOLLOW_BRUSH);

          CPen linePen;
          linePen.CreatePen(Areas[i].Ellipses[iE].penType,Areas[i].Ellipses[iE].penWidth,Areas[i].Ellipses[iE].penColor);
          retPen = pDrawDC->SelectObject(&linePen);
          if(retPen) retPen->DeleteObject();

		  Ellipse(pDrawDC, i, Areas[i].Ellipses[iE].x1, Areas[i].Ellipses[iE].y1,
			                  Areas[i].Ellipses[iE].x2, Areas[i].Ellipses[iE].y2);
      }
      WriteTextInfo(pDrawDC, i);

      if(needLegend)
          DrawLegend(pDrawDC);
    }

    pDrawDC->SetBkMode(obkMode); // TODO: initialize obkMode
    retPen = pDrawDC->SelectObject(oldPen);
    if(retPen) retPen->DeleteObject();
    CFont* retFont = pDrawDC->SelectObject(oldFont);
    if(retFont) retFont->DeleteObject();

    if(pDrawDC != pDC){
      pDC->BitBlt(0, 0, clientR.right, clientR.bottom, pDrawDC, 0, 0, SRCCOPY);
    }

    if(pBitmap){
        CBitmap* retBM = pDrawDC->SelectObject(pBitmap);
        if(retBM) retBM->DeleteObject();
    }
    dcMem.DeleteDC();

    if(pFont)
		delete pFont;
}

void C2DGraph::InitPath(CDC* pDC)
{
      long int xC, yC;
      double x, y;
      CPoint P1;
      CPoint P2;
	  int i =0;
    pDC->BeginPath();
	int idx = 0;
	for(i=0; i < Areas[0].DrawLimits.GetSize(); i++){
		if(Areas[0].DrawLimits[i].EllType==1)
		  idx++;
	}
    for(i=0; i < idx; i++){
      x = Areas[0].DrawLimits[i].limR.left;
      y = Areas[0].DrawLimits[i].limR.bottom;
      MatToWndCoor(0, x, y, xC, yC);
      P1.x = xC; P1.y = yC;
      x = Areas[0].DrawLimits[i].limR.right;
      y = Areas[0].DrawLimits[i].limR.top;
      MatToWndCoor(0, x, y, xC, yC);
      P2.x = xC; P2.y = yC;
      CRect limR(P1, P2);
	  limR.NormalizeRect();
	  limR.left += 2;
	  limR.top += 2;
	  limR.right -= 2;
	  limR.bottom -= 2;
      int nP = 0;
      CPoint* pPoints = Areas[0].DrawLimits[i].GetPolyline(limR, nP);
         pDC->MoveTo(pPoints[0].x,pPoints[0].y);
		 for(int i=1; i < nP; i++)
          pDC->LineTo(pPoints[i].x,pPoints[i].y);
    }
    pDC->EndPath();
	if(idx==1)
      pDC->SelectClipPath(RGN_COPY);
	if(idx==2)
      pDC->SelectClipPath(RGN_AND);
}

void C2DGraph::DeInitPath(CDC* pDC)
{
	int i =0;
	if(Areas[0].DrawLimits.GetSize()){
		pDC->SelectClipRgn(NULL);
	}

	long int xC, yC;
	double x, y;
	CPoint P1;
	CPoint P2;
    CPen pen;

	COLORREF Color = RGB(127, 127, 127);
    pen.CreatePen(PS_SOLID, 1, Color);
    CPen* pPen = pDC->SelectObject(&pen);

	int idx = 0;
	for(int i=0; i < Areas[0].DrawLimits.GetSize(); i++){
		if(Areas[0].DrawLimits[i].EllType==1)
		  idx++;
	}
	for(i=0; i < idx; i++){
	  x = Areas[0].DrawLimits[i].limR.left;
	  y = Areas[0].DrawLimits[i].limR.bottom;
      MatToWndCoor(0, x, y, xC, yC);
	  P1.x = xC; P1.y = yC;
	  x = Areas[0].DrawLimits[i].limR.right;
	  y = Areas[0].DrawLimits[i].limR.top;
      MatToWndCoor(0, x, y, xC, yC);
	  P2.x = xC; P2.y = yC;

      CRect limR(P1, P2);
      limR.NormalizeRect();
	  limR.InflateRect(-1,-1);
      int nP = 0;
      CPoint* pPoints = Areas[0].DrawLimits[i].GetPolyline(limR, nP);
      pDC->MoveTo(pPoints[0].x,pPoints[0].y);
	    for(int i=1; i < nP; i++)
         pDC->LineTo(pPoints[i].x,pPoints[i].y);
	}
    if(pPen){
       CPen* pRetPen = pDC->SelectObject(pPen);
	   if(pRetPen) pRetPen->DeleteObject();
	 }
}

void C2DGraph::StrokeDrawPath(CDC* pDC)
{
    CPen pen;
	COLORREF Color = RGB(127, 127, 127);
    pen.CreatePen(PS_SOLID, 1, Color);
    CPen* pPen = pDC->SelectObject(&pen);
	COLORREF ColorBr;
    if(isWMF || isPrinting)
      ColorBr = RGB(255,255,255);
	else
      ColorBr = bkColor;
    CBrush br(ColorBr);
    CBrush* pBrush = pDC->SelectObject(&br);

	long int xC, yC;
	double x, y;
	CPoint P1;
	CPoint P2;
	int polymode;

  for(int iT =0; iT < 2; iT++){
	if(iT == 1)
		pDC->BeginPath();
	for(int i=1; i < Areas[0].DrawLimits.GetSize(); i++){
	  x = Areas[0].DrawLimits[i].limR.left;
	  y = Areas[0].DrawLimits[i].limR.bottom;
      MatToWndCoor(0, x, y, xC, yC);
	  P1.x = xC; P1.y = yC;
	  x = Areas[0].DrawLimits[i].limR.right;
	  y = Areas[0].DrawLimits[i].limR.top;
      MatToWndCoor(0, x, y, xC, yC);
	  P2.x = xC; P2.y = yC;
      CRect limR(P1, P2);
	  limR.NormalizeRect();
          limR.left -= 3;
          limR.top -= 3;
          limR.right += 3;
          limR.bottom += 3;
	  if(Areas[0].DrawLimits[i].limType == LIM_RECT){
		  pDC->FrameRect(&limR, &br);
	  }
	  else{
         int nP = 0;
         CPoint* pPoints = Areas[0].DrawLimits[i].GetPolyline(limR, nP);
         pDC->MoveTo(pPoints[0].x,pPoints[0].y);
    	    for(int i=1; i < nP; i++)
              pDC->LineTo(pPoints[i].x,pPoints[i].y);
	       }
	}
    if(iT == 1){
	  pDC->EndPath();
      polymode = pDC->SetPolyFillMode(WINDING);
      pDC->FillPath();
      pDC->SetPolyFillMode(polymode);
	  pDC->SelectClipRgn(NULL);
    }
  }
    if(pPen){
       CPen* pRetPen = pDC->SelectObject(pPen);
	   if(pRetPen) pRetPen->DeleteObject();
	 }
	if(pBrush){
       CBrush* pRetBrush = pDC->SelectObject(pBrush);
	   if(pRetBrush) pRetBrush->DeleteObject();
	}
}

void C2DGraph::InitPathLimitPolylines(CDC* pDC)
{
	if(Areas[0].DrawLimitPolylines.GetSize() == 0)
		return;
    CPen pen;
	COLORREF Color = RGB(127, 127, 127);
    pen.CreatePen(PS_SOLID, 1, Color);
    CPen* pPen = pDC->SelectObject(&pen);
	COLORREF ColorBr;
    if(isWMF || isPrinting)
      ColorBr = RGB(255,255,255);
	else
      ColorBr = bkColor;
    CBrush br(ColorBr);
    CBrush* pBrush = pDC->SelectObject(&br);

	long int xC, yC;
	double x, y;
	
    pDC->BeginPath();

	for(int i=0; i < 1; i++){
		int nP = Areas[0].DrawLimitPolylines[i].Line.GetSize();
		CPoint* pwLine = new CPoint[nP];
		for(int ii=0; ii < Areas[0].DrawLimitPolylines[i].Line.GetSize(); ii++){
		  x = Areas[0].DrawLimitPolylines[i].Line[ii].x;
		  y = Areas[0].DrawLimitPolylines[i].Line[ii].y;
          MatToWndCoor(0, x, y, xC, yC);
          MatToWndCoor(0, x, y, xC, yC);
		  pwLine[ii].x = xC;
		  pwLine[ii].y = yC;
		}
		InflateContour(pwLine, nP, 2);
		pDC->Polyline(pwLine, nP);
		delete[] pwLine;
	}
    pDC->EndPath();
    pDC->SelectClipPath(RGN_COPY);
	
    if(pPen){
       CPen* pRetPen = pDC->SelectObject(pPen);
	   if(pRetPen) pRetPen->DeleteObject();
	 }
	if(pBrush){
       CBrush* pRetBrush = pDC->SelectObject(pBrush);
	   if(pRetBrush) pRetBrush->DeleteObject();
	}
}

void C2DGraph::DeInitPathLimitPolylines(CDC* pDC)
{
	if(Areas[0].DrawLimitPolylines.GetSize()){
		pDC->SelectClipRgn(NULL);
	}
}

void C2DGraph::StrokeLimitPolylines(CDC* pDC)
{
	int i =0;
	if(Areas[0].DrawLimitPolylines.GetSize() == 0)
		return;
    CPen pen;
	COLORREF Color = RGB(127, 127, 127);
    pen.CreatePen(PS_SOLID, 1, Color);
    CPen* pPen = pDC->SelectObject(&pen);
	COLORREF ColorBr;
    if(isWMF || isPrinting)
      ColorBr = RGB(255,255,255);
	else
      ColorBr = bkColor;
    CBrush br(ColorBr);
    CBrush* pBrush = pDC->SelectObject(&br);

	long int xC, yC;
	double x, y;
//Outside contour	
	for(i=0; i < 1; i++){
		int nP = Areas[0].DrawLimitPolylines[i].Line.GetSize();
		CPoint* pwLine = new CPoint[nP];
		for(int ii=0; ii < Areas[0].DrawLimitPolylines[i].Line.GetSize(); ii++){
		  x = Areas[0].DrawLimitPolylines[i].Line[ii].x;
		  y = Areas[0].DrawLimitPolylines[i].Line[ii].y;
          MatToWndCoor(0, x, y, xC, yC);
		  pwLine[ii].x = xC;
		  pwLine[ii].y = yC;
		}
		InflateContour(pwLine, nP, 2);
		pDC->Polyline(pwLine, nP);
		delete[] pwLine;
	}

//Inside contour
	for(i=1; i < Areas[0].DrawLimitPolylines.GetSize(); i++){
		int nP = Areas[0].DrawLimitPolylines[i].Line.GetSize();
		CPoint* pwLine = new CPoint[nP];
		for(int ii=0; ii < nP; ii++){
		  x = Areas[0].DrawLimitPolylines[i].Line[ii].x;
		  y = Areas[0].DrawLimitPolylines[i].Line[ii].y;
          MatToWndCoor(0, x, y, xC, yC);
		  pwLine[ii].x = xC;
		  pwLine[ii].y = yC;
		}
		InflateContour(pwLine, nP, -2);
		pDC->Polygon(pwLine, nP);
		delete[] pwLine;
	}
	
    if(pPen){
       CPen* pRetPen = pDC->SelectObject(pPen);
	   if(pRetPen) pRetPen->DeleteObject();
	 }
	if(pBrush){
       CBrush* pRetBrush = pDC->SelectObject(pBrush);
	   if(pRetBrush) pRetBrush->DeleteObject();
	}
}

void C2DGraph::RefreshPicture()
{
    bRender = TRUE;
	Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// C2DGraph message handlers

BOOL C2DGraph::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass=AfxRegisterWndClass(CS_OWNDC);
	return CScrollView::PreCreateWindow(cs);
}

int C2DGraph::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

    m_ToolTips.Create(this);
    m_ToolTips.AddTool(this, _T("X=;Y=;"));
	m_ToolTips.Activate(TRUE);

	return 0;
}

void C2DGraph::InitPlotMode(CDC* pDC, int iArea)
{
/*   CRect Region = GetRegion(iArea);
   CRect clR; GetClientRect(&clR);
   CPoint VOPoint;
   CSize viewExt;
   CSize wndExt;

   if(!isWMF){
     pDC->SetROP2(R2_COPYPEN);
     pDC->SetMapMode(MM_ANISOTROPIC);
     VOPoint = CPoint(Region.left, Region.top);
     viewExt = wndExt = CSize(Region.Width(), Region.Height());
     pDC->SetWindowExt(wndExt);
     pDC->SetWindowOrg(VOPoint);
     pDC->SetViewportExt(viewExt);
     pDC->SetViewportOrg(VOPoint);
   }*/
}


CRect C2DGraph::GetRegion(int iArea)
{
   CClientDC dc(this);
   OnPrepareDC(&dc, NULL);
   CRect clientR;
   if(isPrinting)
	  clientR = TotalPrintArea;
   else{
      GetClientRect(clientR);
   }

   if(isWMF){
    ConvertRectToWMFCoord(clientR, &dc);
   }

   if(Areas.GetSize()==0)
	   return CRect(0,0,0,0);

   double left = Areas[iArea].Region.left;
   double top = Areas[iArea].Region.top;
   double right = Areas[iArea].Region.right;
   double bottom = Areas[iArea].Region.bottom;

   CPoint VOPoint;
   CSize sizeExt;
   if(left == 1. && top == 1. && right == 1. && bottom == 1.){
	   VOPoint = CPoint(0,0);
	   sizeExt = CSize(clientR.Width(), clientR.Height());
   }
   else{
	   int x1 =(int)(clientR.Width()*left);
	   int y1 = (int)(clientR.Height()*top);
	   int cx = (int)(clientR.Width()*right);
	   int cy = (int)(clientR.Height()*bottom);
	   VOPoint = CPoint(x1, y1);
	   sizeExt = CSize(cx, cy);
   }

   CRect reg(VOPoint, sizeExt);

   if(WtoHRatio){
     CDC* pDC = GetDC();
     int regX = reg.Width();
     int regY = reg.Height();
     double rangeX = Areas[0].Limits.xMax - Areas[0].Limits.xMin;
     double rangeY = Areas[0].Limits.yMax - Areas[0].Limits.yMin;
     double scale = 1;
     GetDCRegion(pDC, rangeX, rangeY, regX, regY,  scale);
        int sW = (reg.Width() - regX)/2;
        int sH = (reg.Height() - regY)/2;
        reg.left += sW;
        reg.top += sH;
        reg.right = reg.left + regX;
        reg.bottom = reg.top + regY;
     ReleaseDC(pDC);
   }

   return reg;
}

void C2DGraph::SetClipReg(CDC* pDC, int iArea, BOOL IsSet)
{
  CRect Clip = GetRegion(iArea);
  if(IsSet){
	  if(isPrinting){
		  pDC->BeginPath();
		  pDC->Rectangle(Clip);
		  pDC->EndPath();
          pDC->SelectClipPath(RGN_COPY);
	  }
      else if(isWMF){
          Clip.top = -Clip.top;
          Clip.bottom = -Clip.bottom;
          pDC->BeginPath();
          pDC->Rectangle(Clip);
          pDC->EndPath();
          pDC->SelectClipPath(RGN_COPY);
      }
	  else{
		  CRgn C;
		  C.CreateRectRgn(Clip.left-1,Clip.top-1,Clip.right+1,Clip.bottom+1);
		  pDC->SelectClipRgn(&C);
	  }
  }
   else {
     pDC->SelectClipRgn(NULL);
   }
}

void C2DGraph::MatToWndCoor(int iArea, double x, double y, long int& xC, long int& yC)
{
  CRect wR = GetRegion(iArea);
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

  double Cx = wR.Width() / (xMax - xMin);
  double Cy = wR.Height() / (yMax - yMin);
  xC = wR.left + (int)((x - xMin)*Cx);
  yC = wR.top + (int)((yMax - y)*Cy);
  if(isWMF)
      yC *= (-1);
}

void C2DGraph::WndCoorToMat(int iArea, CPoint P, double &x, double &y)
{
  CRect wR = GetRegion(iArea);
  CPoint L = wR.BottomRight();
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

  double Cx, Cy;
      Cx =(xMax - xMin)/wR.Width();
      Cy =(yMax - yMin)/wR.Height();
      x = xMax - (L.x - P.x)*Cx;
      y = yMin - (P.y - L.y)*Cy;
}

void C2DGraph::Line(CDC* pDC, int iArea, double x, double y, BOOL IsVisible, BOOL IsCliped)
{
    long int xC,yC;
    xC = yC = 0;
    MatToWndCoor(iArea, x, y, xC, yC);
    if(IsVisible) {
      if(IsCliped) SetClipReg(pDC, iArea, IsCliped);
      pDC->LineTo(xC, yC);
      if(IsCliped) SetClipReg(pDC, iArea, FALSE);
    }
    else {
      pDC->MoveTo(xC, yC);
    }
}

void C2DGraph::Ellipse(CDC* pDC, int iArea, double x1, double y1, double x2, double y2)
{
	long int wx1, wy1, wx2, wy2;
	wx1 = wy1 = wx2 = wy2 = 0;
	MatToWndCoor(iArea, x1, y1, wx1, wy1);
	MatToWndCoor(iArea, x2, y2, wx2, wy2);
	pDC->Ellipse(wx1, wy1, wx2, wy2);
}

void C2DGraph::WriteTextInfo(CDC* pDC, int iArea)
{
    long int xC, yC;
	CString s;
	for(int i=0; i < Areas[iArea].Texts.GetSize(); i++){
		xC = yC = 0;
		MatToWndCoor(iArea, Areas[iArea].Texts[i].x, Areas[iArea].Texts[i].y, xC, yC);
		if(!Areas[iArea].Texts[i].text.IsEmpty()){
			WriteText(pDC, xC, yC, LPCTSTR(Areas[iArea].Texts[i].text), Areas[iArea].Texts[i].Align);
		}
		else{
			s.Format("%0.1f",Areas[iArea].Texts[i].val);
			WriteText(pDC, xC, yC, LPCTSTR(s), Areas[iArea].Texts[i].Align);
		}
	}
}

void C2DGraph::WriteText(CDC* pDC, int X, int Y, LPCTSTR Str, UINT Align)
{
   pDC->SetBkColor(bkColor);
   pDC->SetTextAlign(Align);
   pDC->TextOut(X,Y,Str,lstrlen(Str));
}

////////////////////////////// Axes /////////////////////////////////////////

double C2DGraph::_SetBigStep(double s_old)
{
    static double stand[]={ 1.,2.,2.5,5.,10.};
    double min=10.;
    double step=1.;
    double s,coef;
    int i,imin;

    s=fabs(s_old);
    if(s<1.) {
        coef=10.;
        while(s<1.) {
            s*=coef;
            step/=coef;
        }
    }
    else {
        coef=0.1;
        while(s>=1.) {
            s*=coef;
            step/=coef;
        }
        s/=coef; step*=coef;
    }

    for(i=0;i<5;i++) {
        if(fabs(s-stand[i])<min) {
            min=fabs(s-stand[i]);
            imin=i;
        }
    }
    return s_old/fabs(s_old)*stand[imin]*step;
}

void  C2DGraph::_DelAxs(double xmn, double xmx, double *masstab, char *text)
{
    double e1, e2;

    if(fabs(xmn)>MINFLOAT) e1=log10(fabs(xmn));
    else         e1=-1.e20;

    if(fabs(xmx)>MINFLOAT) e2=log10(fabs(xmx));
    else        e2=-1.e20;

    int Eexp=int(max(e1,e2));

    if(abs(Eexp) > 2) {
        sprintf(text,"*E%3d",Eexp);
        if(Eexp>0.) text[2]='+';
        if(text[3]=='-') strncpy(&text[2],"-0",2);
        if(text[3]==' ') text[3]='0';
    }
    else Eexp=0;

    *masstab=pow(10.,Eexp);
}

void C2DGraph::_Nprob(char *text)
{
    for(int i=strlen(text)-1; i>0; i--)
        if((text[i]==' ')||(text[i]=='0')) text[i]=0;
        else break;
}

void C2DGraph::_razm_x_for(CDC *pDC, int iArea, double masstab, double x, double xmax,
         double dx, int nx, double y, unsigned LC, char *Format)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

  double lReg = Areas[iArea].Region.left;
  double bReg = Areas[iArea].Region.bottom;

    CRect clR = GetRegion(iArea);
    TEXTMETRIC Tm;
    pDC->GetTextMetrics(&Tm);

    char text[20];
    int n=0;
	int Dx, Dy;
    Dx = Dy = int(clR.Width()*lReg*0.1);
    if(isWMF)
        Dy = -Dy;
    CPoint XY;
    Line(pDC, iArea, xMin, y, 0,FALSE);
    Line(pDC, iArea, xMax, y, 1,FALSE);
    UINT Align;
    Align=TA_CENTER|TA_TOP;
    while(sign(dx)*(xmax-x)>=0.) {
         MatToWndCoor(iArea, x,y, XY.x, XY.y);
		 if(!need0OnAxis && x == 0.){
			 ;
		 }
         else{
			 if((n%nx)==0) {
				 if(LC) {
					 sprintf(text,Format,x/masstab);
					 _Nprob(text);
					 if(n==0) Dx=-Dx;
					 else Dx=Tm.tmAscent/2;
					 WriteText(pDC, XY.x+Dx,XY.y+Dy,text,Align);
				 }
			 }
         }
         x+=dx; n++;
    }
}

void C2DGraph::_razm_y_for(CDC *pDC, int iArea, double masstab, double y, double ymax,
         double dy, int ny, double x, unsigned LC, char *Format, UINT Align)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

  double lReg = Areas[iArea].Region.left;
  double bReg = Areas[iArea].Region.bottom;

    CRect clR = GetRegion(iArea);
    char text[20];

    Line(pDC, iArea, x, yMin, 0, FALSE);
    Line(pDC, iArea, x, yMax, 1, FALSE);

    TEXTMETRIC Tm;
    pDC->GetTextMetrics(&Tm);

    int n=0;
    CPoint XY;
	int Dx, Dy;
    Dx = Dy = int(clR.Width()*lReg*0.1);
    if(Align == (TA_RIGHT|TA_TOP)) Dx = -Dx;
    while(sign(dy)*(ymax-y)>=0.) {
          MatToWndCoor(iArea, x,y, XY.x,XY.y);
		  if(!need0OnAxis && y == 0.){
			  ;
		  }
		  else{
			  if((n%ny)==0) {
				  if(LC) {
					  sprintf(text,Format,y/masstab);
					  _Nprob(text);
					  if(n==0) Dy=-Dy;
					  else Dy=Tm.tmAscent/2;
					  WriteText(pDC, XY.x+Dx,XY.y+Dy,text, Align /*| TA_BASELINE*/);
				  }
			  }
		  }
         y+=dy; n++;
    }
}

double C2DGraph::AxesX(CDC *pDC, int iArea, double Ybeg, double Ux, int nx)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

    if(fabs(Ux)<=MINFLOAT) Ux=_SetBigStep((xMax-xMin)/5.);
    else                          Ux=_SetBigStep(Ux);
    double x_axes_beg;
    double Xm=(sign(xMin)==sign(xMax))? min(xMin,xMax) : 0.;
    x_axes_beg=(floor(Xm/Ux))*Ux;
    if(x_axes_beg<Xm) x_axes_beg+=Ux;

    double y_axes_beg=Ybeg;

    unsigned LC=(nx > 0)? 1 : 0;

    double dx=Ux/nx;

    char text1[6];
    double masstab;

    _DelAxs(xMin,xMax,&masstab,text1);

    char Format[30];
    Format[0]='%';

    double Val=max(fabs(xMax/masstab),fabs(xMin/masstab));
    double E=log10(Val);
    int Ndig1=int(fabs(E))+2;

    Val=fabs(dx/masstab);
    E=log10(Val);
    int Ndig2=int(fabs(E))+2;

    sprintf(&Format[1],"%-d.%-df",Ndig1+Ndig2+2,Ndig2);

    _razm_x_for(pDC, iArea, masstab,x_axes_beg, xMax, dx, nx,
             y_axes_beg,LC,Format);
    _razm_x_for(pDC, iArea, masstab,x_axes_beg, xMin, -dx, nx,
             y_axes_beg,LC,Format);
    CPoint XY;
    MatToWndCoor(iArea, xMax,Ybeg,XY.x,XY.y);
    XY.x += 5; XY.y += 3;
    XAxisNamePos = XY;

    if(LC&&(masstab!=1.)) {
        MatToWndCoor(iArea, xMax,Ybeg,XY.x,XY.y);
        WriteText(pDC, XY.x-2,XY.y-2,text1,TA_RIGHT|TA_BOTTOM);
    }
    return Ux;
}

double C2DGraph::AxesY(CDC *pDC, int iArea, double Xbeg, double Uy, int ny, UINT Align)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;

    if(fabs(Uy)<=MINFLOAT) Uy=_SetBigStep((yMax-yMin)/5.);
    else                   Uy=_SetBigStep(Uy);

    double y_axes_beg;
    double Ym=(sign(yMin)==sign(yMax))? min(yMin,yMax) : 0.;
    y_axes_beg=(floor(Ym/Uy))*Uy;
    if(y_axes_beg<Ym) y_axes_beg+=Uy;

    double x_axes_beg=Xbeg;

    unsigned LC=(ny > 0)? 1 : 0;

    double dy=Uy/ny;

    char text1[6];
    double masstab;

    _DelAxs(yMin,yMax,&masstab,text1);

    char Format[30];
    Format[0]='%';

    double Val=max(fabs(yMax/masstab),fabs(yMin/masstab));
    double E=log10(Val);
    int Ndig1=int(fabs(E))+2;

    Val=fabs(dy/masstab);
    E=log10(Val);
    int Ndig2=int(fabs(E))+2;

    if(Align == (TA_RIGHT|TA_TOP))
      sprintf(&Format[1],"%-d.%-df",Ndig1+Ndig2+2,Ndig2);
    else
      sprintf(&Format[1],"%d.%df",Ndig1+Ndig2+2,Ndig2);

    _razm_y_for(pDC, iArea, masstab,y_axes_beg, yMax, dy, ny,
             x_axes_beg, LC,Format, Align);
    _razm_y_for(pDC, iArea, masstab,y_axes_beg, yMin, -dy, ny,
             x_axes_beg, LC, Format, Align);

    CPoint XY;
    MatToWndCoor(iArea, xMax, Xbeg, XY.x,XY.y);
    XY.x += 5; XY.y -= 3;
    YAxisNamePos = XY;

    if(LC&&(masstab!= 1.)) {
      MatToWndCoor(iArea, Xbeg,yMax,XY.x, XY.y);
      WriteText(pDC, XY.x+3,XY.y,text1, Align);
    }
    return Uy;
}

void C2DGraph::DotLineY(CDC *pDC, int iArea, double Y, double X1, double X2, COLORREF Color, BOOL FullLine)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;
    CPoint P1, P2;
     MatToWndCoor(iArea,X1,Y,P1.x,P1.y);
     MatToWndCoor(iArea,X2,Y,P2.x,P2.y);

    int xBeg=min(P1.x,P2.x), xEnd=max(P1.x,P2.x);
    if(!FullLine) xBeg+=5;
    if(isPrinting || isWMF){
		CPen pen;
		COLORREF Color = RGB(0,0,0);
		pen.CreatePen(PS_DOT, 0, Color);

		CPen* pPen = pDC->SelectObject(&pen);
		if(pPen) pPen->DeleteObject();
		pDC->MoveTo(P1);
		pDC->LineTo(P2);

		CPen* retPen = pDC->SelectObject(pPen);
		if(retPen)retPen->DeleteObject();
    }
	else{
		for(int x=xBeg; x<xEnd; x+=5) {
			pDC->SetPixelV(x, P1.y, Color);
		}
	}
}

void C2DGraph::DotLineX(CDC *pDC, int iArea, double X, double Y1, double Y2, COLORREF Color, BOOL FullLine=TRUE)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;
    CPoint P1, P2;
      MatToWndCoor(iArea, X,Y1,P1.x,P1.y);
      MatToWndCoor(iArea, X,Y2,P2.x,P2.y);
    int yBeg=__min(P1.y,P2.y), yEnd=__max(P1.y,P2.y);
    if(!FullLine) yBeg+=5;
    if(isPrinting || isWMF){
		CPen pen;
		COLORREF Color = RGB(0,0,0);
		pen.CreatePen(PS_DOT, 0, Color);

		CPen* pPen = pDC->SelectObject(&pen);
		if(pPen) pPen->DeleteObject();
		pDC->MoveTo(P1);
		pDC->LineTo(P2);

		CPen* retPen = pDC->SelectObject(pPen);
		if(retPen)retPen->DeleteObject();
    }
	else{
		for(int y=yBeg; y<yEnd; y+=5) {
			pDC->SetPixelV(P1.x, y, Color);
		}
	}
}

void C2DGraph::AxesGridX(CDC *pDC, int iArea, double Ux, int Type)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;
    if(fabs(Ux)>MINFLOAT) {
        COLORREF Color = Areas[iArea].Grid.XpenColor;
        double Xm=__min(xMin,xMax);
        double x_axes_beg=(floor(Xm/Ux+0.5*sign(Ux)))*Ux;
        if(x_axes_beg<Xm) x_axes_beg+=Ux;

        double _xMax=__max(xMax, xMin);
        Ux=fabs(Ux);

        for(double x=x_axes_beg; x-_xMax< Ux/2.; x+=Ux){
			if(Areas[iArea].Limits.xMin <= x && x <= Areas[iArea].Limits.xMax)
              DotLineX(pDC,iArea, x, yMin,yMax, Color, FALSE);
        }

        if(Type==GRID_FULL) {
            DotLineX(pDC, iArea, xMin, yMin, yMax, Color);
            DotLineX(pDC, iArea, xMax, yMin, yMax, Color);
        }
    }
}

void C2DGraph::AxesGridY(CDC *pDC, int iArea, double Uy, int Type)
{
  double xMin = Areas[iArea].Limits.xMin;
  double yMax = Areas[iArea].Limits.yMax;
  double xMax = Areas[iArea].Limits.xMax;
  double yMin = Areas[iArea].Limits.yMin;
    if(fabs(Uy)>MINFLOAT) {
        COLORREF Color = Areas[iArea].Grid.XpenColor;
        double Ym=__min(yMin,yMax);
        double y_axes_beg=(floor(Ym/Uy+0.5*sign(Uy)))*Uy;
        if(y_axes_beg<Ym) y_axes_beg+=Uy;

        double _yMax=__max(yMax, yMin);
        Uy=fabs(Uy);

        for(double y=y_axes_beg; y-_yMax<Uy/2.; y+=Uy){
//			if(Areas[iArea].Limits.yMin <= y && y <= Areas[iArea].Limits.yMax)
              DotLineY(pDC, iArea, y, xMin, xMax, Color, FALSE);
        }

        if(Type==GRID_FULL) {
            DotLineY(pDC, iArea, yMin, xMin, xMax, Color, TRUE);
            DotLineY(pDC, iArea, yMax, xMin, xMax, Color, TRUE);
        }
    }
}

void C2DGraph::DrawLegend(CDC* pDC)
{
  int i =0;
  int stepDown = 18; // !!!!
  int y=0;
  int idy=0;

   CRect clientR;
   if(isPrinting)
	  clientR = TotalPrintArea;
   else if(isWMF){
      GetClientRect(clientR);
      ConvertRectToWMFCoord(clientR, pDC);
   }
   else
      GetClientRect(clientR);

  CRect regR;
  regR = GetRegion(0);
  CRect legR(regR.right, regR.top, (int)(regR.right+regR.Width()*0.3), regR.bottom);

  int left = int(regR.right+legR.Width() * 0.15);
  int right = int(left + legR.Width()*0.25);
  stepDown = int(legR.Height() / maxNLevels);
  y = regR.top;

  if(isWMF || isPrinting)
       pDC->SetTextColor(RGB(0,0,0));
  else if(bkColor == RGB(255,255,255))
       pDC->SetTextColor(RGB(0,0,0));
  else
       pDC->SetTextColor(RGB(255,255,255));

  for(i=0; i < Areas[0].Lines.GetSize(); i++){
      y += stepDown;
      CPoint LP(left, y);
      CPoint RP(right, y);
      CPoint textP(int(left + legR.Width()*0.25), y);
      CPoint numP(left+(right - left)/2, y);

      CPen pen;
  	  pen.CreatePen(PS_SOLID, 0, Areas[0].Lines[i].penColor);
	  CPen* pRetPen = pDC->SelectObject(&pen);
	  if(pRetPen) pRetPen->DeleteObject();
	  if(isWMF){
		  LP.y = -LP.y;
		  RP.y = -RP.y;
	  }
      pDC->MoveTo(LP.x, LP.y);
      pDC->LineTo(RP.x, RP.y);
	  if(isWMF)
		  textP.y = -textP.y;
      WriteText(pDC, textP.x, textP.y, LPCTSTR(Areas[0].Lines[i].LegendText), TA_LEFT|TA_BASELINE);
  }
  for(i=0; i < Areas[0].Ellipses.GetSize(); i++){
      y += stepDown;
      CPoint LP(left, y);
      CPoint RP(right, y);
      CPoint textP(int(left + legR.Width()*0.25), y);
      CPoint numP(left+(right - left)/2, y);
	  
      CPen pen;
	  pen.CreatePen(PS_SOLID, 0, Areas[0].Ellipses[i].penColor);
	  CPen* pRetPen = pDC->SelectObject(&pen);
	  if(pRetPen) pRetPen->DeleteObject();
	  if(isWMF){
		  LP.y = -LP.y;
		  RP.y = -RP.y;
	  }
      pDC->MoveTo(LP.x, LP.y);
      pDC->LineTo(RP.x, RP.y);
	  if(isWMF)
		  textP.y = -textP.y;
      WriteText(pDC, textP.x, textP.y, LPCTSTR(Areas[0].Ellipses[i].LegendText), TA_LEFT|TA_BASELINE);
  }
}

void C2DGraph::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
}

void C2DGraph::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	CRect clR; GetClientRect(clR);

	sizeTotal.cx = clR.Width();
	sizeTotal.cy = clR.Height();
	SetScaleToFitSize(sizeTotal);
}

BOOL C2DGraph::GetSDNearestMatPoint(int iArea, double& x, double& y,
									double& F, double& Re, double& Im, CString& format)
{
	int i = 0;
	int iL = -1;
	int ix = -1;
	double x1, x2;
	CArray<int, int> idxX;
	for(int i=0; i < /*Areas[iArea].Lines.GetSize()*/1; i++){
		for(int ii = 1; ii < Areas[iArea].Lines[i].X.GetSize(); ii++){
			x1 =  Areas[iArea].Lines[i].X[ii-1];
			x2 =  Areas[iArea].Lines[i].X[ii];
			if((x1 <= x && x <= x2)||(x2 <= x && x <= x1)){
				iL = i;
				idxX.Add(ii);
			}
		}
	}

	if(iL == -1 || idxX.GetSize()==0)
		return FALSE;

    for(i=0; i < idxX.GetSize(); i++){
		ix = idxX[i];
		double _y = Areas[iArea].Lines[iL].Y[ix];
		double dy = fabs(_y*0.1);
		if(dy >= fabs(_y - y)){
	  	    Re = Areas[iArea].arRe[ix];
	  	    Im = Areas[iArea].arIm[ix];
		    F = Areas[iArea].arFreq[ix];
			format = Areas[iArea].Lines[iL].ToolTipsFormat;
			return TRUE;
		}
    }
	return FALSE;
}

BOOL C2DGraph::GetNearestMatPoint(int iArea, double& x, double& y, CString& format)
{
	int i = 0;
    double Eps = fabs(Areas[iArea].Limits.yMax - Areas[iArea].Limits.yMin) * 0.02;
	int iL = -1;
	int ix = -1;
	double x1, x2;
	double y1, y2;
    double yL;
	CArray<int, int> idxX;
	double min_y = 1.e10;
  if (isCoordinates)
    {
    format = Areas[iArea].Lines[0].ToolTipsFormat;
    return TRUE;
    }
	for(int i=0; i < Areas[iArea].Lines.GetSize(); i++){
		for(int ii = 1; ii < Areas[iArea].Lines[i].X.GetSize(); ii++){
			x1 =  Areas[iArea].Lines[i].X[ii-1];
			x2 =  Areas[iArea].Lines[i].X[ii];
       	    double _y = Areas[iArea].Lines[i].Y[ii];
            double dy = fabs(_y - y);
			if(x1 <= x && x <= x2 && dy < min_y){
				min_y = dy;
				iL = i;
				idxX.Add(ii);
			}
		}
	}

	if(iL == -1 || idxX.GetSize()==0)
		return FALSE;

    for (i=0; i < idxX.GetSize(); i++)
      {
      ix = idxX[i];
      if (ix >= Areas[iArea].Lines[iL].Y.GetSize())
        return FALSE;
      x1 = Areas[iArea].Lines[iL].X[ix-1];
      x2 = Areas[iArea].Lines[iL].X[ix];
      y1 = Areas[iArea].Lines[iL].Y[ix-1];
      y2 = Areas[iArea].Lines[iL].Y[ix];
      yL = (y1 * (x2 - x) + y2 * (x - x1)) / (x2 - x1);
      if (fabs(yL - y) <= Eps)
        {
        y = yL;
        format = Areas[iArea].Lines[iL].ToolTipsFormat;
        return TRUE;
        }
      }
     /*
    for(i=0; i < idxX.GetSize(); i++){
		ix = idxX[i];
		if(ix >= Areas[iArea].Lines[iL].Y.GetSize())
			return FALSE;
        double Eps = fabs(Areas[iArea].Limits.yMax - Areas[iArea].Limits.yMin) * 0.05;
		double _y = Areas[iArea].Lines[iL].Y[ix];
		double dy = fabs(_y*0.05);
		if(dy >= fabs(_y - y)){
			x = Areas[iArea].Lines[iL].X[ix];
			y = Areas[iArea].Lines[iL].Y[ix];
			format = Areas[iArea].Lines[iL].ToolTipsFormat;
			return TRUE;
		}
    }
*/

	return FALSE;
}

BOOL C2DGraph::GetToolText(CString& Text)
{
    CPoint pos;
	CString format;
	CString s;
	double x, y;
	double F, Re, Im;
	GetCursorPos(&pos);
	ScreenToClient(&pos);
	for(int i=0; i < Areas.GetSize(); i++){
	    CRect R = GetRegion(i);
		if(R.PtInRect(pos)){
          if(!Areas[i].EnableToolTips)
			  return FALSE;
          WndCoorToMat(i, pos, x, y);
		  if(Areas[i].arFreq.GetSize()>0) // Smith diagram
			  if(GetSDNearestMatPoint(i, x, y, F, Re, Im, format)){
				  s.Format(LPCTSTR(format), F, Re, Im);
				  Text = s;
				  return TRUE;
			  }
			  else{
				  return FALSE;
			  }
			  else{
				  if(GetNearestMatPoint(i, x, y, format)){
					  s.Format(LPCTSTR(format), x, y);
					  Text = s;
					  return TRUE;
				  }
				  else{
					  return FALSE;
				  }
			  }
		}
	}
	return FALSE;
}

BOOL C2DGraph::PreTranslateMessage(MSG* pMsg)
{
  switch(pMsg->message)
    {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_MOUSEMOVE:
	  CString toolText;
	  if(GetToolText(toolText)){
		  m_ToolTips.UpdateTipText(LPCTSTR(toolText), this);
          m_ToolTips.RelayEvent(pMsg);
	  }
	  else{
		  m_ToolTips.Pop();
	  }
    }
  return CScrollView::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// C2DGraph diagnostics

#ifdef _DEBUG
void C2DGraph::AssertValid() const
{
	CScrollView::AssertValid();
}

void C2DGraph::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG



void C2DGraph::OnFilePrintPreview()
{
    // In derived classes, implement special window handling here
    // Be sure to Unhook Frame Window close if hooked.
    // must not create this on the frame.  Must outlive this function
    CPrintPreviewState* pState = new CPrintPreviewState;
	pState->nIDMainPane = WND_2D_GRAPH;
	pState->lpfnCloseProc = _PreviewCloseProc;
	pFr = GetParentFrame();
//	pState->pViewActiveOld = this;

    // DoPrintPreview's return value does not necessarily indicate that
    // Print preview succeeded or failed, but rather what actions are necessary
    // at this point.  If DoPrintPreview returns TRUE, it means that
    // OnEndPrintPreview will be (or has already been) called and the
    // pState structure will be/has been deleted.
    // If DoPrintPreview returns FALSE, it means that OnEndPrintPreview
    // WILL NOT be called and that cleanup, including deleting pState
    // must be done here.

    if (!DoPrintPreview(AFX_IDD_PREVIEW_TOOLBAR/*IDD_PREVIEW*/, this,
                            RUNTIME_CLASS(CMPreviewView), pState))
    {
        // In derived classes, reverse special window handling here for
        // Preview failure case

        TRACE0("Error: DoPrintPreview failed.\n");
        AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
        delete pState;      // preview failed to initialize, delete State now
    }
}

BOOL C2DGraph::DoPrintPreview(UINT nIDResource, CScrollView* pPrintView,
    CRuntimeClass* pPreviewViewClass, CPrintPreviewState* pState)
{
    ASSERT_VALID_IDR(nIDResource);
    ASSERT_VALID(pPrintView);
    ASSERT(pPreviewViewClass != NULL);
    ASSERT(pPreviewViewClass->IsDerivedFrom(RUNTIME_CLASS(CMPreviewView)));
    ASSERT(pState != NULL);

    CFrameWnd* pParent = STATIC_DOWNCAST(CFrameWnd, GetParentFrame());
    ASSERT_VALID(pParent);

    CCreateContext context;
    context.m_pCurrentFrame = pParent;
    context.m_pCurrentDoc = GetDocument();
    context.m_pLastView = this;

    // Create the preview view object
    CMPreviewView* pView = (CMPreviewView*)pPreviewViewClass->CreateObject();
    if (pView == NULL)
    {
        TRACE0("Error: Failed to create preview view.\n");
        return FALSE;
    }
    ASSERT_KINDOF(CMPreviewView, pView);
    pView->m_pPreviewState = pState;        // save pointer
    pParent->OnSetPreviewMode(TRUE, pState);    // Take over Frame Window

    // Create the toolbar from the dialog resource
    pView->m_pToolBar = new CDialogBar;
    if (!pView->m_pToolBar->Create(pParent, MAKEINTRESOURCE(nIDResource),
        CBRS_TOP, AFX_IDW_PREVIEW_BAR))
    {
        TRACE0("Error: Preview could not create toolbar dialog.\n");
        pParent->OnSetPreviewMode(FALSE, pState);   // restore Frame Window
        delete pView->m_pToolBar;       // not autodestruct yet
        pView->m_pToolBar = NULL;
        pView->m_pPreviewState = NULL;  // do not delete state structure
        delete pView;
        return FALSE;
    }
    pView->m_pToolBar->m_bAutoDelete = TRUE;    // automatic cleanup

    // Create the preview view as a child of the App Main Window.  This
    // is a sibling of this view if this is an SDI app.  This is NOT a sibling
    // if this is an MDI app.

    if (!pView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
        CRect(0,0,0,0), pParent, AFX_IDW_PANE_FIRST, &context))
    {
        TRACE0("Error: couldn't create preview view for frame.\n");
        pParent->OnSetPreviewMode(FALSE, pState);   // restore Frame Window
        pView->m_pPreviewState = NULL;  // do not delete state structure
        delete pView;
        return FALSE;
    }

    // Preview window shown now

    pState->pViewActiveOld = pParent->GetActiveView();
    C2DGraph* pActiveView = (C2DGraph*)pParent->GetActiveFrame()->GetActiveView();
    if (pActiveView != NULL)
        pActiveView->OnActivateView(FALSE, pActiveView, pActiveView);

	BOOL res = pView->SetPrintView(pPrintView);
    if (!res)
    {
        pView->OnPreviewClose();
        return TRUE;            // signal that OnEndPrintPreview was called
    }

    pParent->SetActiveView(pView);  // set active view - even for MDI

    // update toolbar and redraw everything
    pView->m_pToolBar->SendMessage(WM_IDLEUPDATECMDUI, (WPARAM)TRUE);
    pParent->RecalcLayout();            // position and size everything
    pParent->UpdateWindow();
    return TRUE;
}

void C2DGraph::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	CScrollView::OnBeginPrinting(pDC, pInfo);
}

void C2DGraph::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	CScrollView::OnEndPrinting(pDC, pInfo);
}

void C2DGraph::OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* _pView)
{
	CMPreviewView* pView = (CMPreviewView*) _pView;

	ASSERT_VALID(pDC);
    ASSERT_VALID(pView);

    if (pView->m_pPrintView != NULL)
        OnEndPrinting(pDC, pInfo);

    CFrameWnd* pParent = STATIC_DOWNCAST(CFrameWnd, GetParentFrame());
    ASSERT_VALID(pParent);

    // restore the old main window
    pParent->OnSetPreviewMode(FALSE, pView->m_pPreviewState);

    // Force active view back to old one
    pParent->SetActiveView(pView->m_pPreviewState->pViewActiveOld);
    if (pParent != GetParentFrame())
        OnActivateView(TRUE, this, this);   // re-activate view in real frame
    pView->DestroyWindow();     // destroy preview view
            // C++ object will be deleted in PostNcDestroy

    // restore main frame layout and idle message
    pParent->RecalcLayout();
    pParent->SendMessage(WM_SETMESSAGESTRING, (WPARAM)AFX_IDS_IDLEMESSAGE, 0L);
    pParent->UpdateWindow();

//	CScrollView::OnFilePrint();
}

void C2DGraph::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	CView::OnPrepareDC(pDC, pInfo);
}

BOOL C2DGraph::OnPreparePrinting(CPrintInfo* pInfo)
{
	pInfo->SetMaxPage(1);
    return DoPreparePrinting(pInfo);
//	return CScrollView::OnPreparePrinting(pInfo);
}

void C2DGraph::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CWinApp* pApp = AfxGetApp();
	CDC dc;
	pApp->CreatePrinterDC(dc);

	CSize sizePaper = CSize(dc.GetDeviceCaps(HORZSIZE)*10,
		                    dc.GetDeviceCaps(VERTSIZE)*10);

	TotalPrintArea = CRect(0, 0, sizePaper.cx, sizePaper.cy);


	  CRect clRect; GetClientRect(clRect);
      int cxPage, cyPage, cxInch, cyInch;
      cxPage = pDC->GetDeviceCaps(HORZRES);
      cyPage = pDC->GetDeviceCaps(VERTRES);
      cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
      cyInch = pDC->GetDeviceCaps(LOGPIXELSY);
      TotalPrintArea.top = TotalPrintArea.left = 0;
      TotalPrintArea.bottom = (int)(((double)sizePaper.cy * kY * cxPage * cyInch)
          / ((double)sizePaper.cx * cxInch));
      TotalPrintArea.right = cxPage;

/*	CRect m_rtMargin;
	m_rtMargin.left = 150;
    m_rtMargin.right = 150;
    m_rtMargin.bottom = 100;
    m_rtMargin.top = 200;

	TotalPrintArea.left += m_rtMargin.left;
	TotalPrintArea.right -= m_rtMargin.right;
	TotalPrintArea.top -= m_rtMargin.top;
	TotalPrintArea.bottom += m_rtMargin.bottom;
//  m_rtOrientation = DMORIENT_LANDSCAPE;
*/
	pDC->LPtoDP(&TotalPrintArea);
	pDC->SetViewportOrg(TotalPrintArea.left, TotalPrintArea.top);

//	CString strHeader = GetDocument()->
//	CString strHeader = "Plot 2D";
//	PrintHeader(pDC, pInfo, strHeader);
	COLORREF _bkColor = bkColor;
	bkColor = RGB(255,255,255);
	OnDraw(pDC);
	bkColor = _bkColor;
//	CScrollView::OnPrint(pDC, pInfo);
}

void C2DGraph::PrintHeader(CDC* pDC, CPrintInfo* pInfo, CString strHeader)
{
	pDC->SetTextAlign(TA_LEFT);
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int y = tm.tmHeight + 15;
	pDC->TextOut(25, y, strHeader);
}

void C2DGraph::OnRButtonDown(UINT nFlags, CPoint point)
{
	CreateMenu(point);
	CScrollView::OnRButtonDown(nFlags, point);
}

void C2DGraph::RestoreSettings()
{
}

void C2DGraph::CreateMenu(CPoint P)
{
   CMenu Main;
   Main.CreatePopupMenu();

   UINT FlagPrint;
   UINT FlagPrintPreview;
   UINT FlagInvertBkground;
   UINT FlagWriteMetafile;
   UINT FlagClipboard;
   UINT FlagLogarithmScale;
   UINT FlagEnableNumbers;
   UINT FlagCoordinates;
   LPCTSTR Title;
//   FlagMenuZoomIn =  MF_DISABLED | MF_GRAYED | MF_STRING;
   FlagPrint = MF_STRING;
   FlagPrintPreview = MF_STRING;
   FlagInvertBkground = MF_STRING;
   FlagWriteMetafile = MF_STRING;
   FlagClipboard = MF_STRING;
   FlagCoordinates = MF_STRING; 
   if(EnableLogarithmScale)
	   FlagLogarithmScale = (MF_STRING | MF_CHECKED);
   else
	   FlagLogarithmScale = MF_STRING|MF_GRAYED;
   if(LangType == MG_ENG_LANG) Title = _T("Invert Background");
   else Title = _T("Инвертировать фон");
   Main.AppendMenu(FlagInvertBkground, IDMR_InvertBackGround, Title);
   Main.AppendMenu(MF_SEPARATOR);
   if(LangType == MG_ENG_LANG) Title = _T("Copy");
   else Title = _T("Копировать");
   Main.AppendMenu(FlagWriteMetafile, IDMR_CopyClipboard, Title);
   if(LangType == MG_ENG_LANG) Title = _T("Save As");
   else Title = _T("Сохранить как");
   Main.AppendMenu(FlagWriteMetafile, IDMR_WriteMetaFile, Title);
   Main.AppendMenu(MF_SEPARATOR);
//   Main.AppendMenu(FlagLogarithmScale, IDMR_LogarithmScale, _T("Logarithm Scale"));
//   Main.AppendMenu(MF_SEPARATOR);
   if(isIsolineNumbers){
	  if(EnableIsolineNumbers)
		  FlagEnableNumbers = (MF_STRING | MF_CHECKED);
	  else
		  FlagEnableNumbers = MF_STRING;
      if(LangType == MG_ENG_LANG) Title = _T("Enable Numbers");
      else Title = _T("Номеровать линии");
      Main.AppendMenu(FlagEnableNumbers, IDMR_IsolineNumbers, Title);
   }
  if (isEnableCoor)
    {
    if(LangType == MG_ENG_LANG) Title = _T("Coordinates");
    else Title = _T("Координаты");
    if(isCoordinates) FlagCoordinates = FlagCoordinates | MF_CHECKED; 
    Main.AppendMenu(FlagCoordinates, IDMR_Coordinates, Title);
    }
   if (isIsolineNumbers || isEnableCoor)
     Main.AppendMenu(MF_SEPARATOR);
   if(EnablePreView){
      if(LangType == MG_ENG_LANG) Title = _T("Print Preview");
      else Title = _T("Просмотр");
      Main.AppendMenu(FlagPrintPreview, ID_FILE_PRINT_PREVIEW, Title);
   }
   if(LangType == MG_ENG_LANG) Title = _T("Print");
   else Title = _T("Печать");
   Main.AppendMenu(FlagPrint, ID_FILE_PRINT, Title);
   ClientToScreen(&P);
   Main.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                              P.x, P.y, this, NULL);
}

void C2DGraph::OnInvertBackground()
{
    COLORREF axColor;
	if(bkColor == RGB(0,0,0)){
		bkColor = RGB(255,255,255);
		axColor = RGB(0,0,0);
	}
	else{
		bkColor = RGB(0,0,0);
		axColor = RGB(255,255,255);
	}

    for(int i=0; i < Areas.GetSize(); i++){
		if(Areas[i].Grid.XpenColor == Areas[i].Grid.YpenColor){
          Areas[i].Grid.XpenColor = Areas[i].Grid.YpenColor = axColor;
		  Areas[i].Grid.XtextColor = Areas[i].Grid.YtextColor = axColor;
		}
		else{
			Areas[i].Grid.XpenColor = axColor;
			Areas[i].Grid.XtextColor = axColor;
			for(int ii=0; ii < Areas[i].Lines.GetSize(); ii++){
				COLORREF Color = Areas[i].Lines[ii].penColor;
				BYTE R = GetRValue(Color);
				BYTE G = GetGValue(Color);
				BYTE B = GetBValue(Color);
				if(bkColor == RGB(255,255,255)){
					R = (BYTE)(R/2); G = (BYTE)(G/2); B = (BYTE)(B/2);
				}
				else if(bkColor == RGB(0,0,0)){
					R = (BYTE)(R*2); G = (BYTE)(G*2); B = (BYTE)(B*2);
				}
				Areas[i].Lines[ii].penColor = RGB(R, G, B);
				Areas[i].Grid.YpenColor =  RGB(R, G, B);
				Areas[i].Grid.YtextColor =  RGB(R, G, B);
			}
		}
    }

	RefreshPicture();
}

void C2DGraph::OnLogarithmScale()
{
	if(EnableLogarithmScale) EnableLogarithmScale = FALSE;
	else  EnableLogarithmScale = TRUE;
}

void C2DGraph::OnIsolineNumbers()
{
}

void C2DGraph::OnCoordinates()
{
  isCoordinates = !isCoordinates;
}

void C2DGraph::OnWriteMetafile()
{
	WriteMetafile(true);
}

void C2DGraph::WriteMetafile(bool IncludeWMF/*true*/)
{
  TCHAR title[] = "Save File";
  LPCTSTR lan;
  CFileDialog fileDlg(FALSE);
  lan = "BMP Files (bmp)";
  CString str = lan; str += (TCHAR)NULL;
  str += "*.bmp"; str += (TCHAR)NULL;
  if(IncludeWMF){
	  str += ("Metafiles (emf)"); str += (TCHAR)NULL;
	  str += "*.emf"; str += (TCHAR)NULL;

	  str += ("Metafiles (wmf)"); str += (TCHAR)NULL;
	  str += "*.wmf"; str += (TCHAR)NULL;
  }

  fileDlg.m_ofn.lpstrFilter = str;

  fileDlg.m_ofn.nFilterIndex = 1;

  TCHAR strName[_MAX_PATH];

  strName[0] = (TCHAR)NULL;
  fileDlg.m_ofn.lpstrFile = strName;

  fileDlg.m_ofn.lpstrTitle = title;
  fileDlg.m_ofn.lpstrDefExt = "emf";

  fileDlg.m_ofn.Flags |= (OFN_EXTENSIONDIFFERENT|OFN_OVERWRITEPROMPT);
//  TCHAR Str[_MAX_PATH];
  if(!PathForSave.IsEmpty())
    fileDlg.m_ofn.lpstrInitialDir = PathForSave.GetBuffer(PathForSave.GetLength());
  else
    fileDlg.m_ofn.lpstrInitialDir = NULL;
  if (fileDlg.DoModal() == IDOK){
    MakeLoopMessage();
    CString FileName = fileDlg.m_ofn.lpstrFile;
    int iP = FileName.ReverseFind('\\');
    CString sav = FileName.Left(iP);
    SavePath("WMF file", sav, LPCTSTR(iniFile));

		if(fileDlg.m_ofn.nFilterIndex == 1){
           HBITMAP hBit = CopyWindowToBitmap(m_hWnd, PW_CLIENT);
           HPALETTE hPal = GetSystemPalette();
           HDIB hWDib = BitmapToDIB(hBit, hPal);
           CFile file;
           CFileException fe;
           if (!file.Open(LPCTSTR(FileName), CFile::modeCreate|CFile::modeWrite, &fe))
           {
              TCHAR szError[1024];
              fe.GetErrorMessage(szError, 1024);
              MessageBox(szError, "", MB_OK);
              return;
           }
           SaveDIB(hWDib, file);
           file.Close();
           HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, hWDib);
           EndWaitCursor();
		}
		else if(fileDlg.m_ofn.nFilterIndex == 2){
          HENHMETAFILE hF = GetMetafile(NULL);
          WriteEnhMetaFileAsPlaceable(hF, LPCTSTR(FileName));
          DeleteEnhMetaFile(hF);
		}
		else if(fileDlg.m_ofn.nFilterIndex == 3){
          HENHMETAFILE hF = GetMetafile(LPCTSTR(FileName));
          DeleteEnhMetaFile(hF);
		}
  }
  PathForSave.ReleaseBuffer();
}

void C2DGraph::OnCopy()
{
    // Clear clipboard of contents, and copy the DIB.
	MakeLoopMessage();
    if (OpenClipboard())
    {
     BeginWaitCursor();
     EmptyClipboard();
     CDC* pDC = GetDC();
     HBITMAP hBit = CopyWindowToBitmap(m_hWnd, PW_CLIENT);
     HPALETTE hPal = GetSystemPalette();
     HDIB hWDib = BitmapToDIB(hBit, hPal);
     SetClipboardData (CF_DIB, CopyHandle((HANDLE) hWDib));
     ReleaseDC(pDC);
     CloseClipboard();
     HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, hWDib);
     EndWaitCursor();
    }

/*    HENHMETAFILE hF = GetMetafile(NULL);
    OpenClipboard();
    EmptyClipboard();
    SetClipboardData( CF_ENHMETAFILE, CopyEnhMetaFile( hF, NULL ) );
    CloseClipboard();
    DeleteEnhMetaFile(hF);*/
}

BOOL C2DGraph::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

HENHMETAFILE C2DGraph::GetMetafile(LPCTSTR FileName)
{
    CDC* pDC = GetDC();
    CRect rect; GetClientRect(rect);
    ConvertRectToWMFCoord(rect, pDC);

    CMetaFileDC mfDC;
    TCHAR   szDesc[] = "WinFringe\0Image Description\0\0";
    mfDC.CreateEnhanced(pDC, FileName, rect, NULL);

    mfDC.SetAttribDC(pDC->m_hAttribDC);
    mfDC.SetMapMode(MM_HIMETRIC);
    mfDC.SetROP2(R2_COPYPEN);
    CPoint VOPoint, WOPoint;
    CSize viewExt, wndExt;
    VOPoint = mfDC.GetViewportOrg();
    WOPoint = mfDC.GetWindowOrg();
    viewExt = mfDC.GetViewportExt();
    wndExt = mfDC.GetWindowExt();

    RGBQUAD m_rgbPalette[256];
    CPalette m_Palette;
    memcpy( m_rgbPalette, rgbStd256, sizeof(RGBQUAD) * 256 );
    LPLOGPALETTE lpLogPal;
    lpLogPal = (LPLOGPALETTE) new BYTE[sizeof(LOGPALETTE) + ((255) * sizeof(PALETTEENTRY))];
    lpLogPal->palVersion = 0x0300;
    lpLogPal->palNumEntries = 256;

    for (int i = 0; i < 256; i++){
        lpLogPal->palPalEntry[i].peRed = m_rgbPalette[i].rgbRed;
        lpLogPal->palPalEntry[i].peGreen = m_rgbPalette[i].rgbGreen;
        lpLogPal->palPalEntry[i].peBlue = m_rgbPalette[i].rgbBlue;
        lpLogPal->palPalEntry[i].peFlags = 0;
    }

    m_Palette.CreatePalette(lpLogPal);
    mfDC.SelectPalette(&m_Palette, FALSE);

    isWMF = TRUE;
    COLORREF oldbkColor = bkColor;
    bkColor = RGB(255, 255, 255);
    OnDraw(&mfDC);
    mfDC.SetMapMode(MM_TEXT);
    isWMF = FALSE;
    bkColor = oldbkColor;

   mfDC.SelectStockObject(DEFAULT_PALETTE);
    HENHMETAFILE hF = mfDC.CloseEnhanced();
    ReleaseDC(pDC);
    delete[] (BYTE *)lpLogPal;
    return hF;
}

BOOL C2DGraph::ConvertPointToWMFCoord(CPoint& P, CDC* pDC)
{
// Determine the picture frame dimensions.
// iWidthMM is the display width in millimeters.
// iHeightMM is the display height in millimeters.
// iWidthPels is the display width in pixels.
// iHeightPels is the display height in pixels

  int iWidthMM = pDC->GetDeviceCaps(HORZSIZE);
  int iHeightMM = pDC->GetDeviceCaps(VERTSIZE);
  int iWidthPels = pDC->GetDeviceCaps(HORZRES);
  int iHeightPels = pDC->GetDeviceCaps(VERTRES);

// Convert client coordinates to .01-mm units.
// Use iWidthMM, iWidthPels, iHeightMM, and
// iHeightPels to determine the number of
// .01-millimeter units per pixel in the x-
//  and y-directions.

   P.x = (P.x * iWidthMM * 100)/iWidthPels;
   P.y = (P.y * iHeightMM * 100)/iHeightPels;

   return TRUE;
}

BOOL C2DGraph::ConvertRectToWMFCoord(CRect& R, CDC* pDC)
{
   CPoint TL = R.TopLeft();
   CPoint BR = R.BottomRight();
   ConvertPointToWMFCoord(TL, pDC);
   ConvertPointToWMFCoord(BR, pDC);
   CRect _R(TL, BR);
   R = _R;
   
   return TRUE;
}

void C2DGraph::MyHiMetrictoDP(CDC* pDC, CPoint* pt)
{
   pt->x = MulDiv(pt->x, pDC->GetDeviceCaps(LOGPIXELSX), 2540);
   pt->y = MulDiv(pt->y, pDC->GetDeviceCaps(LOGPIXELSY), 2540);
}

void C2DGraph::MyDPtoHiMetric(CDC* pDC, CPoint* pt)
{
   pt->x = MulDiv(pt->x, 2540, pDC->GetDeviceCaps(LOGPIXELSX));
   pt->y = MulDiv(pt->y, 2540, pDC->GetDeviceCaps(LOGPIXELSY));
}

void C2DGraph::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
	Invalidate(FALSE);
}

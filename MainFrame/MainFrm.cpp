// MainFrm.cpp : implementation of the CMainFrame class
//
/***********************************************************************************
 МОДУЛЬ: MainFrm.cpp
 НАЗНАЧЕНИЕ:
         Фрейм основного окна приложения       
************************************************************************************/

#include "stdafx.h"
#include "Digit.h"

#include "MainFrm.h"
#include "Utils\mutils.h"
#include "ImageTempl\ImageDoc.h"
#include "ImageTempl\ImageView.h"
#include "Options\ToolBarsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
    ON_COMMAND(IDD_TOOLBARS, OnToolBars)
    ON_UPDATE_COMMAND_UI(IDD_TOOLBARS, OnUpdateToolBars)
	ON_COMMAND(IDD_ADV_PARAMETERS, OnAdvParameters)
	ON_UPDATE_COMMAND_UI(IDD_ADV_PARAMETERS, OnUpdateAdvParameters)
	ON_COMMAND(IDD_INTERFEROGRAM_VIEW, OnIterfView)
	ON_UPDATE_COMMAND_UI(IDD_INTERFEROGRAM_VIEW, OnUpdateIterfView)
	ON_COMMAND(IDD_EXTRFRINGE_VIEW, OnExtremeView)
	ON_UPDATE_COMMAND_UI(IDD_EXTRFRINGE_VIEW, OnUpdateExtremeView)
	ON_COMMAND(IDD_SECTIONS_VIEW, OnSectionView)
	ON_UPDATE_COMMAND_UI(IDD_SECTIONS_VIEW, OnUpdateSectionView)
	ON_COMMAND(IDD_DOTLINES_VIEW, OnDotLineView)
	ON_UPDATE_COMMAND_UI(IDD_DOTLINES_VIEW, OnUpdateDotLineView)
	ON_COMMAND(IDD_DOTS_VIEW, OnDotsView)
	ON_UPDATE_COMMAND_UI(IDD_DOTS_VIEW, OnUpdateDotsView)
	ON_COMMAND(IDD_APPROX_VIEW, OnApproximation)
	ON_UPDATE_COMMAND_UI(IDD_APPROX_VIEW, OnUpdateApproximation)
	ON_BN_CLICKED(IDB_clear_measure, OnClearMeasure)
	ON_COMMAND(IDD_CHANGE_LANG, OnChangeLang)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
    ON_REGISTERED_MESSAGE(CDigitApp::m_nOpenMsg, OnOpenMsg)
//	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
//	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
//	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
//	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

static UINT BASED_CODE MainButtons[] =
{
      ID_FILE_OPEN,
      ID_FILE_SAVE,
      ID_FILE_SAVE_AS,
	  IDD_ZOOM_FIT,
      IDD_ZOOM_IN,
      IDD_ZOOM_OUT,
};

static UINT BASED_CODE KitButtons[] =
{
   IDD_FOTO_SECTIONS,
   IDD_MEASURE,
};

static UINT BASED_CODE ViewButtons[] =
{
	IDD_INTERFEROGRAM_VIEW,
	IDD_EXTRFRINGE_VIEW,
	IDD_SECTIONS_VIEW,
    IDD_DOTLINES_VIEW,
    IDD_DOTS_VIEW,
    IDD_APPROX_VIEW,
};

static UINT BASED_CODE DigitButtons[] =
{
    //IDD_BOUNDS_EXT,
    //IDD_BOUNDS_INS,
    //ID_SEPARATOR,
    IDD_AUTO_D,
    IDD_FC_MAX,
    IDD_FC_MIN,
    IDD_FC_MINMAX,
    IDD_CLEAR_D,
	IDD_CALC_APROX,
};

// Aperture/bounds toolbar button layout (uses bounds command IDs)
static UINT BASED_CODE ApertureButtons[] =
{
	ID_ADD_BOUND_CIRCLE,
    ID_ADD_BOUND_RECT,
    ID_ADD_BOUND_ELLIPSE,
	ID_ADD_BOUND_POLYGON,
	ID_SEPARATOR,
    ID_BOUND_VISIBILITY,
    ID_SEPARATOR,
    ID_BOUND_MODE_SELECT,
	//ID_BOUND_MODE_DELETE,
};

static UINT BASED_CODE EditButtons[] =
{
    IDD_FRINGE_EDIT,
    IDD_DOT_EDIT,
    ID_SEPARATOR,
    IDD_FRINGE_NAVIGATE,
    ID_SEPARATOR,
    IDD_ADD_SEC_D,
    IDD_DEL_SEC_D,
    IDD_SEC_LEFT_D,
    IDD_SEC_RIGHT_D,
    ID_SEPARATOR,
    IDD_NUMBER_FRINGES_FORWARD,
    IDD_NUMBER_FRINGES_BACKWARD,
    IDD_NUM_OFF_MINUS,
    ID_SEPARATOR,
    ID_SEPARATOR,/* combo goes here, slot 14 */
    ID_SEPARATOR,
    IDD_NUM_OFF_PLUS,
};

static int posNumFringe = 14;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
  m_nImagePaneCol = 2;
}

CMainFrame::~CMainFrame()
{
}

// Response на сообщение DigitOpenMessage 
// lParam содержит список файлов коммандной строки 
LONG CMainFrame::OnOpenMsg(UINT, LONG lParam)
{
  TCHAR szAtomName[_MAX_PATH];
  szAtomName[0] = NULL;
  GlobalGetAtomName((ATOM)lParam, szAtomName, _MAX_PATH);

  CDigitApp* pApp = (CDigitApp*) AfxGetApp();
  CString CmdLine = szAtomName;
  if(!CmdLine.IsEmpty())
    pApp->TreatCmdLine(LPCTSTR(CmdLine));

  return TRUE;
}

// Смотри Microsoft Visual C++ документацию
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

   CSize wB, wI; 
   wI.cx = 22;
   wI.cy = 21;
   wB.cx = wI.cx+7;
   wB.cy = wI.cy+6;

   // main toolbar 
   if (!m_wndMainBar.CreateEx(this, TBSTYLE_FLAT,
       WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
       !m_wndMainBar.LoadToolBar(IDR_MAINFRAME))
   {
       TRACE0("Failed to create main toolbar\n");
       return -1;
   }
   m_wndMainBar.SetSizes(wB, wI);

// view toolbar 
    if (!m_wndViewBar.CreateEx(this, TBSTYLE_FLAT, 
        WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |
            CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY) ||
        !m_wndViewBar.LoadToolBar(IDR_VIEWTOOLS))
    {
        TRACE0("Failed to create mainbar\n");
        return -1;      // fail to create
    }
	m_wndViewBar.SetSizes(wB, wI);

// kit toolbar 
    if (!m_wndKitBar.CreateEx(this, TBSTYLE_FLAT, 
        WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |
        CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY) ||
        !m_wndKitBar.LoadToolBar(IDR_KITTOOL))
    {
        TRACE0("Failed to create mainbar\n");
        return -1;      // fail to create
    }
	m_wndKitBar.SetSizes(wB, wI);

// digit toolbar 
    if (!m_wndDigitBar.CreateEx(this, TBSTYLE_FLAT, 
        WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |
        CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY) ||
        !m_wndDigitBar.LoadToolBar(IDR_DIGITTOOLS))
    {
        TRACE0("Failed to create mainbar\n");
        return -1;      // fail to create
    }
	m_wndDigitBar.SetSizes(wB, wI);

    // Create aperture toolbar
    if (!m_wndApertureBar.CreateEx(this, TBSTYLE_FLAT,
        WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS |
        CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndApertureBar.LoadToolBar(IDR_TOOLBAR_APERTURE))
    {
        TRACE0("Failed to create aperture toolbar\n");
        return -1;
    }
    m_wndApertureBar.SetSizes(wB, wI);

    // edit toolbar 
    if (!m_wndEditBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |
            CBRS_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_EDITTOOLS) ||
        !m_wndEditBar.LoadBitmap(IDR_EDITTOOLS) ||
        !m_wndEditBar.SetButtons(EditButtons, sizeof(EditButtons)/sizeof(UINT)))
    {
        TRACE0("Failed to create mainbar\n");
        return -1;      // fail to create
    }
	m_wndEditBar.SetSizes(wB, wI);
	
    //if (!m_wndStatusBar.Create(this))
    //{
    //    TRACE0("Failed to create status bar\n");
    //    return -1;
    //}

    if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
    if (!m_wndMeasureDlgBar.Create(this, IDR_RULER, 
        CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
        TRACE0("Failed to create dialogbar\n");
        return -1;      // fail to create
    }

    if (!m_wndMeasureBar.Create(this) ||
        !m_wndMeasureBar.AddBar(&m_wndMeasureDlgBar))
    {
        TRACE0("Failed to create rebar\n");
        return -1;      // fail to create
    }
	
    if (!m_wndInfoDlgBar.Create(this, IDR_IMAGEINFO, 
        CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
        TRACE0("Failed to create dialogbar\n");
        return -1;      // fail to create
    }

    if (!m_wndInfoBar.Create(this, RBS_BANDBORDERS,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM) ||
        !m_wndInfoBar.AddBar(&m_wndInfoDlgBar))
    {
        TRACE0("Failed to create rebar\n");
        return -1;      // fail to create
    }

	EnableDocking(CBRS_ALIGN_ANY);

    m_wndKitBar.SetWindowText(_T("KitTools"));
    m_wndKitBar.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndKitBar);

    m_wndViewBar.SetWindowText(_T("View"));
    m_wndViewBar.EnableDocking(CBRS_ALIGN_ANY);
    DockPaneLeftOf(&m_wndViewBar, &m_wndKitBar);

    m_wndDigitBar.SetWindowText(_T("Digit"));
    m_wndDigitBar.EnableDocking(CBRS_ALIGN_ANY);
	DockPaneLeftOf(&m_wndDigitBar, &m_wndViewBar);

    m_wndMainBar.SetWindowText(_T("Main"));
    m_wndMainBar.EnableDocking(CBRS_ALIGN_ANY);
    DockPaneLeftOf(&m_wndMainBar, &m_wndDigitBar);

    m_wndApertureBar.SetWindowText(_T("Aperture"));
    m_wndApertureBar.EnableDocking(CBRS_ALIGN_ANY);
    DockPaneLeftOf(&m_wndApertureBar, &m_wndMainBar);

    //m_wndApertureBar.SetWindowText(_T("Aperture"));
    //m_wndApertureBar.EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBarLeftOf(&m_wndApertureBar, &m_wndDigitBar);

    m_wndEditBar.SetWindowText(_T("Edit"));
    m_wndEditBar.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndEditBar,AFX_IDW_DOCKBAR_BOTTOM);

	
    //Create edit.
	int nDropHeight = 20;
    m_wndEditBar.SetButtonInfo(posNumFringe-1,ID_SEPARATOR,TBBS_SEPARATOR,2);
    m_wndEditBar.SetButtonInfo(posNumFringe,IDW_EDIT,TBBS_SEPARATOR,30);
    m_wndEditBar.SetButtonInfo(posNumFringe+1,ID_SEPARATOR,TBBS_SEPARATOR,2);
    CRect rect;
    m_wndEditBar.GetItemRect(posNumFringe, &rect);
    rect.top = 3;
    rect.bottom = rect.top + nDropHeight;
    if (!m_wndEditBar.m_Edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rect, &m_wndEditBar, IDW_EDIT)){
      TRACE0("Failed to create edit-box\n");
      return FALSE;
    }
    m_wndEditBar.m_Edit.EnableWindow(FALSE);
	//LoadBarState(_T("ToolBar_State"));

    ShowControlBar(&m_wndMeasureBar, FALSE, FALSE);
    ShowControlBar(&m_wndInfoBar, TRUE, FALSE);

	SetImageInfo(" ", 1., 0.);

	return 0;
}

// Выравнивание панели Bar относительно LeftOf панели
void CMainFrame::DockControlBarLeftOf(CToolBar* Bar, CToolBar* LeftOf)
{
    CRect rect;
    DWORD dw;
    UINT n;

    // get MFC to adjust the dimensions of all docked ToolBars
    // so that GetWindowRect will be accurate
    RecalcLayout();
    LeftOf->GetWindowRect(&rect);
    rect.OffsetRect(1,0);
    dw=LeftOf->GetBarStyle();
    n = 0;
    n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
    n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
    n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
    n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

    // When we take the default parameters on rect, DockControlBar will dock
    // each Toolbar on a seperate line.  By calculating a rectangle, we in effect
    // are simulating a Toolbar being dragged to that location and docked.
    DockControlBar(Bar,n,&rect);
}
//Скрыть/показать панель инструментов
void CMainFrame::ShowMeasurePane(BOOL Visual)
{
    ShowControlBar(&m_wndMeasureBar, Visual, FALSE);
}

// Смотри Microsoft Visual C++ документацию
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszClass = _T("DigitClass");
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

// Смотри Microsoft Visual C++ документацию
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
// Переключение библиотеки ресурсов (English/Russian)
void CMainFrame::OnChangeLang()
{
  CDigitApp* pApp = (CDigitApp*) AfxGetApp(); 
  pApp->ChangeIntrLang();

  HMENU m_hMenuEmbedding = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
  HMENU m_hMenuShared = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_IMAGETYPE));
  MDISetMenu(CMenu::FromHandle(m_hMenuEmbedding), NULL);

  ::DestroyMenu(m_hMenuDefault);
  m_hMenuDefault = m_hMenuEmbedding; 
  
  POSITION pos = pApp->GetFirstDocTemplatePosition();
  CMultiDocTemplate* pDT;
  pDT = (CMultiDocTemplate*)pApp->GetNextDocTemplate(pos);
  ::DestroyMenu(pDT->m_hMenuShared);
  pDT->m_hMenuShared = m_hMenuShared;

  DrawMenuBar();
  ShowControlBar(&m_wndInfoBar, TRUE, FALSE);
  
  m_wndInfoDlgBar.DestroyWindow();
  m_wndInfoBar.DestroyWindow();
  
    if (!m_wndInfoDlgBar.Create(this, IDR_IMAGEINFO, 
        CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
        TRACE0("Failed to create dialogbar\n");
    }

    if (!m_wndInfoBar.Create(this, RBS_BANDBORDERS,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM) ||
        !m_wndInfoBar.AddBar(&m_wndInfoDlgBar))
    {
        TRACE0("Failed to create rebar\n");
    }
	SetImageInfo(" ", 1., 0.);
}

// Настройка панели инструментов
void CMainFrame::OnToolBars()
{
	CString s;
	UINT bVisible;

    CToolBarsDlg D(this);
	
	s = CRS("Стандартная", "Standard");
    bVisible = (UINT)((m_wndMainBar.GetStyle() & WS_VISIBLE) != 0);
	D.listToolBars.Add(s);
	D.listStates.Add(bVisible);
	
	s = CRS("Сервис", "Service");
    bVisible = (UINT)((m_wndKitBar.GetStyle() & WS_VISIBLE) != 0);
	D.listToolBars.Add(s);
	D.listStates.Add(bVisible);

	s = CRS("Оцифровка", "Numbering");
    bVisible = (UINT)((m_wndDigitBar.GetStyle() & WS_VISIBLE) != 0);
	D.listToolBars.Add(s);
	D.listStates.Add(bVisible);

	s = CRS("Редактирование", "Edit");
    bVisible = (UINT)((m_wndEditBar.GetStyle() & WS_VISIBLE) != 0);
	D.listToolBars.Add(s);
	D.listStates.Add(bVisible);

	s = CRS("Вид", "View");
    bVisible = (UINT)((m_wndViewBar.GetStyle() & WS_VISIBLE) != 0);
	D.listToolBars.Add(s);
	D.listStates.Add(bVisible);
	
	int res = D.DoModal();
    if(res){
        m_wndMainBar.ShowPane(D.listStates[0], FALSE, FALSE);
        m_wndKitBar.ShowPane(D.listStates[1], FALSE, FALSE);
        m_wndDigitBar.ShowPane(D.listStates[2], FALSE, FALSE);
        m_wndApertureBar.ShowPane(D.listStates[3], FALSE, FALSE);
    }
  RecalcLayout();
}

// Смотри Microsoft Visual C++ документацию
void CMainFrame::OnUpdateToolBars(CCmdUI* pCmdUI)
{
}

void CMainFrame::OnAdvParameters()
{
    CControls* pCtrls = GetControls();
	pCtrls->IsAdvBar = pCtrls->IsAdvBar ? FALSE : TRUE;
	if(pCtrls->IsAdvBar)
      ShowControlBar(&m_wndInfoBar, TRUE, FALSE);
	else
      ShowControlBar(&m_wndInfoBar, FALSE, FALSE);
}

void CMainFrame::OnUpdateAdvParameters(CCmdUI* pCmdUI)
{
    CControls* pCtrls = GetControls();
    pCmdUI->SetCheck(pCtrls->IsAdvBar);
}

// Очистка параметров панели измерений
void CMainFrame::OnClearMeasure()
{
    CDialogBar* pDB = GetMeasureDlgBar();
    CMeasureCtrls* pMCtrls = GetMeasureCtrls();
    CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();

    pDoc->ActivateMeasure(FALSE);

	pMCtrls->Init();
	CWnd* pW;
	CString s;
	s = "0.00";
	pW = pDB->GetDlgItem(IDT_X_measure);
	pW->SetWindowText(LPCTSTR(s)); 
	pW = pDB->GetDlgItem(IDT_Y_measure);
	pW->SetWindowText(LPCTSTR(s)); 
	pW = pDB->GetDlgItem(IDT_W_measure);
	pW->SetWindowText(LPCTSTR(s)); 
	pW = pDB->GetDlgItem(IDT_H_measure);
	pW->SetWindowText(LPCTSTR(s)); 
	pW = pDB->GetDlgItem(IDT_A_measure);
	pW->SetWindowText(LPCTSTR(s)); 
	pW = pDB->GetDlgItem(IDT_D_measure);
	pW->SetWindowText(LPCTSTR(s)); 
}

// Смотри Microsoft Visual C++ документацию
void CMainFrame::OnClose() 
{
	if(!CheckForSaveAllImageDoc())
      return;
    BOOL res = ::DestroyMenu(m_hMenuDefault);
	m_hMenuDefault = NULL;
    ShowControlBar(&m_wndMeasureBar, FALSE, FALSE);

	SaveBarState(_T("ToolBar_State"));
	CMDIFrameWndEx::OnClose();
}

// Смотри Microsoft Visual C++ документацию
void CMainFrame::OnHelp()
{
	WinHelp(NULL, HELP_CONTENTS);
}

void CMainFrame::SetCurrentNumber(double CurrentNumber)
{
	CString s;
	s.Format("%g", CurrentNumber);
	m_wndEditBar.m_Edit.SetWindowText(LPCTSTR(s));
}

void CMainFrame::SetImageInfo(LPCTSTR Title, double ScaleFactor, double Rotation)
{
	m_wndInfoDlgBar.SetComments(Title);
	m_wndInfoDlgBar.SetScaleFactor(ScaleFactor);
	m_wndInfoDlgBar.SetRotation(Rotation);
    m_wndInfoDlgBar.UpdateData(FALSE);
}

void CMainFrame::GetImageInfo(CString& Title, double& ScaleFactor, double& Rotation)
{
    m_wndInfoDlgBar.UpdateData(TRUE);
	m_wndInfoDlgBar.GetComments(Title);
	m_wndInfoDlgBar.GetScaleFactor(ScaleFactor);
	m_wndInfoDlgBar.GetRotation(Rotation);
}

void CMainFrame::SetStatusText(LPCTSTR text)
{
	// Update primary pane of the main status bar with provided text
	if (::IsWindow(m_wndStatusBar.GetSafeHwnd()))
	{
		m_wndStatusBar.SetPaneText(0, text ? text : _T(""));
	}
}

void CMainFrame::OnIterfView()
{
  CControls* pCtrls = GetControls();
  if(pCtrls->ViewState & V_INTERFEROGRAM)
     pCtrls->ViewState &= ~V_INTERFEROGRAM;
  else
     pCtrls->ViewState |= V_INTERFEROGRAM;
  UpdateAllImageViews();
}

void CMainFrame::OnUpdateIterfView(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
/*
  if(pDoc->IsInterferogram())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);*/
  if(pCtrls->ViewState & V_INTERFEROGRAM)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnExtremeView()
{
  CControls* pCtrls = GetControls();
  if(pCtrls->ViewState & V_EXTREMUMS)
     pCtrls->ViewState &= ~V_EXTREMUMS;
  else
     pCtrls->ViewState |= V_EXTREMUMS;
  UpdateAllImageViews();
}

void CMainFrame::OnUpdateExtremeView(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
/*
  if(pDoc->IsSections())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
*/  
  if(pCtrls->ViewState & V_EXTREMUMS)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnSectionView()
{
  CControls* pCtrls = GetControls();
  if(pCtrls->ViewState & V_ZAPSECTIONS){
     pCtrls->ActiveEditMode = -1;
     pCtrls->ViewState &= ~V_ZAPSECTIONS;
  }
  else{
     pCtrls->ViewState |= V_ZAPSECTIONS;
  }
  UpdateAllImageViews();
}

void CMainFrame::OnUpdateSectionView(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();

/*  if(pDoc->IsZapSections())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
*/  
  if(pCtrls->ViewState & V_ZAPSECTIONS)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnDotLineView()
{
  CControls* pCtrls = GetControls();
  if(pCtrls->ViewState & V_DOTLINES)
     pCtrls->ViewState &= ~V_DOTLINES;
  else
     pCtrls->ViewState |= V_DOTLINES;
  UpdateAllImageViews();
}

void CMainFrame::OnUpdateDotLineView(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();

/*  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
*/  
  if(pCtrls->ViewState & V_DOTLINES)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnDotsView()
{
  CControls* pCtrls = GetControls();
  if(pCtrls->ViewState & V_DOTS)
     pCtrls->ViewState &= ~V_DOTS;
  else
     pCtrls->ViewState |= V_DOTS;
  UpdateAllImageViews();
}

void CMainFrame::OnUpdateDotsView(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();

/*  if(pDoc->IsDots())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
*/  
  if(pCtrls->ViewState & V_DOTS)
    pCmdUI->SetCheck(1);
  else
    pCmdUI->SetCheck(0);
}

void CMainFrame::OnApproximation()
{
::SetCursor(::LoadCursor(NULL, IDC_WAIT));
  CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
  CControls* pCtrls = GetControls();
  
  if(pCtrls->ViewState & V_APPROXIMATION)
     pCtrls->ViewState &= ~V_APPROXIMATION;
  else
     pCtrls->ViewState |= V_APPROXIMATION;

  UpdateAllImageViews();
::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

void CMainFrame::OnUpdateApproximation(CCmdUI* pCmdUI)
{
  CControls* pCtrls = GetControls();
  CImageDoc* pDoc = (CImageDoc*)GetWIActiveDocument();
  if(!pDoc)
	  return;

  if(pDoc->IsAproximation())
    pCmdUI->Enable(TRUE);
  else
    pCmdUI->Enable(FALSE);
  
  if(pCtrls->ViewState & V_APPROXIMATION)
    pCmdUI->SetRadio(TRUE);
  else
    pCmdUI->SetRadio(FALSE);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
    return TRUE;	
//	return CMDIFrameWndEx::OnEraseBkgnd(pDC);
}

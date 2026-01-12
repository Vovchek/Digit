#include "../../StdAfx.h"
#include "../../Resource.h"
#include "MFringeApp.h"
#include "MainFrm.h"
#include "CPipelineTreePane.h"
#include "CParametersPane.h"
#include "COutputPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFringeMainFrame

IMPLEMENT_DYNCREATE(CMFringeMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMFringeMainFrame, CFrameWndEx)
	//{{AFX_MSG_MAP(CMFringeMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_PIPELINE_TREE, &CMFringeMainFrame::OnViewPipelineTree)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PIPELINE_TREE, &CMFringeMainFrame::OnUpdateViewPipelineTree)
	ON_COMMAND(ID_VIEW_PARAMETERS, &CMFringeMainFrame::OnViewParameters)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PARAMETERS, &CMFringeMainFrame::OnUpdateViewParameters)
	ON_COMMAND(ID_VIEW_OUTPUT, &CMFringeMainFrame::OnViewOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, &CMFringeMainFrame::OnUpdateViewOutput)
	ON_WM_SETTINGCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_RMS,
	ID_INDICATOR_PV,
	ID_INDICATOR_STREHL,
	ID_INDICATOR_WAVELENGTH,
};

/////////////////////////////////////////////////////////////////////////////
// CMFringeMainFrame construction/destruction

CMFringeMainFrame::CMFringeMainFrame()
{
	m_wndPipelineTree = NULL;
	m_wndParameters = NULL;
	m_wndOutput = NULL;
	m_bCanConvertControlBarToMDI = FALSE;
}

CMFringeMainFrame::~CMFringeMainFrame()
{
	// Explicitly delete docking panes created with 'new'
	// MFC doesn't auto-delete them because they were heap-allocated
	if (m_wndPipelineTree != NULL)
	{
		delete m_wndPipelineTree;
		m_wndPipelineTree = NULL;
	}
	
	if (m_wndParameters != NULL)
	{
		delete m_wndParameters;
		m_wndParameters = NULL;
	}
	
	if (m_wndOutput != NULL)
	{
		delete m_wndOutput;
		m_wndOutput = NULL;
	}
}

void CMFringeMainFrame::OnDestroy()
{
	CFrameWndEx::OnDestroy();
	
	// Clean up any resources that might cause memory leaks
	// Icons are automatically destroyed by the panes, but we can be explicit
}

int CMFringeMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	// Enable customization button on all toolbars
	CMFCToolBar::m_bExtCharTranslation = TRUE;

	// Create menu bar
	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// Prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	// Create toolbar - TEMPORARILY DISABLED (no toolbar resource yet)
	/*
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	*/

	// Create status bar
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

	// Enable docking
	EnableDocking(CBRS_ALIGN_ANY);

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	// m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);  // Disabled - no toolbar yet

	DockPane(&m_wndMenuBar);
	// DockPane(&m_wndToolBar);  // Disabled - no toolbar yet

	// Enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);

	// Enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	// Enable toolbar and docking window menu replacement
	CString strCustomize{"Customize String"};
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// Enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	return 0;
}

BOOL CMFringeMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

BOOL CMFringeMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	// Note: User toolbars customization would be initialized here if needed

	return TRUE;
}

BOOL CMFringeMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create pipeline tree pane
	CString strPipelineTree;
	bNameValid = strPipelineTree.LoadString(IDS_PIPELINE_TREE);
	ASSERT(bNameValid);

	m_wndPipelineTree = new CPipelineTreePane();
	if (!m_wndPipelineTree->Create(strPipelineTree, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PIPELINE_TREE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Pipeline Tree pane\n");
		return FALSE;
	}

	// Create parameters pane
	CString strParameters;
	bNameValid = strParameters.LoadString(IDS_PARAMETERS);
	ASSERT(bNameValid);

	m_wndParameters = new CParametersPane();
	if (!m_wndParameters->Create(strParameters, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PARAMETERS,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Parameters pane\n");
		return FALSE;
	}

	// Create output pane
	CString strOutput;
	bNameValid = strOutput.LoadString(IDS_OUTPUT);
	ASSERT(bNameValid);

	m_wndOutput = new COutputPane();
	if (!m_wndOutput->Create(strOutput, this, CRect(0, 0, 150, 150), TRUE, ID_VIEW_OUTPUT,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Output pane\n");
		return FALSE;
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);

	m_wndPipelineTree->EnableDocking(CBRS_ALIGN_ANY);
	m_wndParameters->EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput->EnableDocking(CBRS_ALIGN_ANY);

	DockPane(m_wndPipelineTree);
	CDockablePane* pTabbedBar = NULL;
	m_wndParameters->AttachToTabWnd(m_wndPipelineTree, DM_SHOW, TRUE, &pTabbedBar);
	DockPane(m_wndOutput);

	return TRUE;
}

void CMFringeMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hPipelineTreeIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PIPELINE_TREE_HC : IDI_PIPELINE_TREE), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndPipelineTree->SetIcon(hPipelineTreeIcon, FALSE);

	HICON hParametersIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PARAMETERS_HC : IDI_PARAMETERS), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndParameters->SetIcon(hParametersIcon, FALSE);

	HICON hOutputIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_HC : IDI_OUTPUT), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput->SetIcon(hOutputIcon, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CMFringeMainFrame diagnostics

#ifdef _DEBUG
void CMFringeMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMFringeMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMFringeMainFrame message handlers

void CMFringeMainFrame::OnViewPipelineTree()
{
	ShowPane(m_wndPipelineTree, !(m_wndPipelineTree->IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMFringeMainFrame::OnUpdateViewPipelineTree(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPipelineTree->IsVisible());
}

void CMFringeMainFrame::OnViewParameters()
{
	ShowPane(m_wndParameters, !(m_wndParameters->IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMFringeMainFrame::OnUpdateViewParameters(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndParameters->IsVisible());
}

void CMFringeMainFrame::OnViewOutput()
{
	ShowPane(m_wndOutput, !(m_wndOutput->IsVisible()), FALSE, TRUE);
	RecalcLayout();
}

void CMFringeMainFrame::OnUpdateViewOutput(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndOutput->IsVisible());
}

void CMFringeMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput->UpdateFonts();
}
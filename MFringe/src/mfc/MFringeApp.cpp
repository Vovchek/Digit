#include "../../StdAfx.h"
#include "../../Resource.h"
#include "MFringeApp.h"
#include "MainFrm.h"
#include "MFringeDoc.h"
#include "MFringeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFringeApp

BEGIN_MESSAGE_MAP(CMFringeApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only CMFringeApp object

CMFringeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMFringeApp construction

CMFringeApp::CMFringeApp()
{
	m_bHiColorIcons = TRUE;
	// TODO: add construction code here
}

CMFringeApp::~CMFringeApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMFringeApp initialization

BOOL CMFringeApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Standard initialization
	SetRegistryKey(_T("MFringe Application"));

	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMFringeDoc),
		RUNTIME_CLASS(CMFringeMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMFringeView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFringeAboutDlg dialog used for App About

class CMFringeAboutDlg : public CDialog
{
public:
	CMFringeAboutDlg();

// Dialog Data
	//{{AFX_DATA(CMFringeAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFringeAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CMFringeAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CMFringeAboutDlg::CMFringeAboutDlg() : CDialog(CMFringeAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CMFringeAboutDlg)
	//}}AFX_DATA_INIT
}

void CMFringeAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFringeAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFringeAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CMFringeAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMFringeApp::OnAppAbout()
{
	AfxMessageBox(_T("MFringe\nInterferogram Processing Application\nVersion 0.1"), MB_OK | MB_ICONINFORMATION);
}

/////////////////////////////////////////////////////////////////////////////
// CMFringeApp message handlers

int CMFringeApp::ExitInstance()
{
	return CWinAppEx::ExitInstance();
}
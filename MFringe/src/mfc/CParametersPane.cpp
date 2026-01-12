#include "../../StdAfx.h"
#include "CParametersPane.h"
#include "../../Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParametersPane

BEGIN_MESSAGE_MAP(CParametersPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

CParametersPane::CParametersPane()
{
}

CParametersPane::~CParametersPane()
{
	// Clean up font resource
	if (m_fntPropList.GetSafeHandle())
	{
		m_fntPropList.DeleteObject();
	}
	
	// Property list is automatically destroyed as child window
	// Properties are managed by CMFCPropertyGridCtrl
}

int CParametersPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Parameters grid\n");
		return -1;
	}

	InitPropList();

	AdjustLayout();
	return 0;
}

void CParametersPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CParametersPane::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	// Sample properties (will be replaced with actual stage parameters)
	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Stage Parameters"));
	
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Parameter 1"), (_variant_t) _T("Value"), _T("Description of parameter 1")));
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Parameter 2"), (_variant_t) 1.0, _T("Description of parameter 2")));

	m_wndPropList.AddProperty(pGroup1);
}

void CParametersPane::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CParametersPane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CParametersPane::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}

void CParametersPane::AdjustLayout()
{
	if (GetSafeHwnd() == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CParametersPane::SetVSDotNetLook(BOOL bSet)
{
	m_wndPropList.SetVSDotNetLook(bSet);
	m_wndPropList.SetGroupNameFullWidth(bSet);
}

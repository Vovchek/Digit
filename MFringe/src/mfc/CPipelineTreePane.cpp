#include "../../StdAfx.h"
#include "CPipelineTreePane.h"
#include "../../Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPipelineTreePane

BEGIN_MESSAGE_MAP(CPipelineTreePane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

CPipelineTreePane::CPipelineTreePane()
{
}

CPipelineTreePane::~CPipelineTreePane()
{
	// Clean up image list to prevent memory leaks
	if (m_PipelineImages.GetSafeHandle())
	{
		m_PipelineImages.DeleteImageList();
	}
	
	// Clear stages array
	m_stages.RemoveAll();
}

void CPipelineTreePane::InitializeImageList()
{
	// Create image list with status icons
	// Icons: 0=Pending, 1=Running, 2=Success, 3=Warning, 4=Error, 5=Skipped
	
	if (m_PipelineImages.GetSafeHandle())
	{
		m_PipelineImages.DeleteImageList();
	}
	
	// Create 16x16 image list with 6 icons
	m_PipelineImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 6, 1);
	
	// Load or create status icons
	// For now, create colored rectangles as placeholders
	// TODO: Replace with actual icon resources
	
	CBitmap bmp;
	CDC memDC;
	CDC* pDC = GetDC();
	memDC.CreateCompatibleDC(pDC);
	
	// Create each status icon
	COLORREF colors[] = {
		RGB(128, 128, 128),  // 0: Pending (Gray)
		RGB(0, 120, 215),    // 1: Running (Blue)
		RGB(16, 124, 16),    // 2: Success (Green)
		RGB(255, 185, 0),    // 3: Warning (Yellow/Orange)
		RGB(232, 17, 35),    // 4: Error (Red)
		RGB(160, 160, 160)   // 5: Skipped (Light Gray)
	};
	
	for (int i = 0; i < 6; i++)
	{
		bmp.CreateCompatibleBitmap(pDC, 16, 16);
		CBitmap* pOldBmp = memDC.SelectObject(&bmp);
		
		memDC.FillSolidRect(0, 0, 16, 16, RGB(255, 255, 255)); // White background
		
		// Draw colored circle
		CBrush brush(colors[i]);
		CBrush* pOldBrush = memDC.SelectObject(&brush);
		memDC.Ellipse(2, 2, 14, 14);
		memDC.SelectObject(pOldBrush);
		
		m_PipelineImages.Add(&bmp, RGB(255, 255, 255));
		
		memDC.SelectObject(pOldBmp);
		bmp.DeleteObject();
	}
	
	ReleaseDC(pDC);
	
	m_wndPipelineTree.SetImageList(&m_PipelineImages, TVSIL_NORMAL);
}

void CPipelineTreePane::SetStageStatus(int stageIndex, EPipelineStageStatus status)
{
	// Find stage by index and update its status
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		if (m_stages[i].stageIndex == stageIndex)
		{
			m_stages[i].status = status;
			UpdateStageIcon(stageIndex);
			break;
		}
	}
}

EPipelineStageStatus CPipelineTreePane::GetStageStatus(int stageIndex) const
{
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		if (m_stages[i].stageIndex == stageIndex)
		{
			return m_stages[i].status;
		}
	}
	return STAGE_PENDING;
}

void CPipelineTreePane::ResetAllStages()
{
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		m_stages[i].status = STAGE_PENDING;
	}
	
	// Update all icons
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		UpdateStageIcon(m_stages[i].stageIndex);
	}
}

void CPipelineTreePane::ClearPipeline()
{
	m_wndPipelineTree.DeleteAllItems();
	m_stages.RemoveAll();
}

void CPipelineTreePane::AddStage(LPCTSTR lpszName, int stageIndex)
{
	// Add stage to tree with pending status
	HTREEITEM hItem = m_wndPipelineTree.InsertItem(lpszName, STAGE_PENDING, STAGE_PENDING);
	
	PipelineStageItem item;
	item.hItem = hItem;
	item.strName = lpszName;
	item.status = STAGE_PENDING;
	item.stageIndex = stageIndex;
	
	m_stages.Add(item);
}

void CPipelineTreePane::UpdateStageIcon(int stageIndex)
{
	// Find stage and update its icon
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		if (m_stages[i].stageIndex == stageIndex)
		{
			HTREEITEM hItem = m_stages[i].hItem;
			int iconIndex = (int)m_stages[i].status;
			
			m_wndPipelineTree.SetItemImage(hItem, iconIndex, iconIndex);
			break;
		}
	}
}

int CPipelineTreePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tree control:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;

	if (!m_wndPipelineTree.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Pipeline Tree\n");
		return -1;
	}

	// Initialize image list with status icons
	InitializeImageList();

	// Add pipeline stages with status indicators
	AddStage(_T("S0  Raw Images"), 0);
	AddStage(_T("S0a Aperture"), 1);
	AddStage(_T("S0b Markers"), 2);
	AddStage(_T("S0c Geometry"), 3);
	AddStage(_T("S1  Digitization"), 4);
	AddStage(_T("S2  Phase"), 5);
	AddStage(_T("S3  Unwrap"), 6);
	AddStage(_T("S4  Wavefront"), 7);
	AddStage(_T("S4b Calibration"), 8);
	AddStage(_T("S5  Polynomial"), 9);
	AddStage(_T("S6  Diffraction"), 10);
	AddStage(_T("S7  Synthesis"), 11);

	AdjustLayout();

	return 0;
}

void CPipelineTreePane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPipelineTreePane::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_wndPipelineTree.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CPipelineTreePane::OnPaint()
{
	CPaintDC dc(this);

	CRect rectTree;
	m_wndPipelineTree.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CPipelineTreePane::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPipelineTree.SetFocus();
}

void CPipelineTreePane::OnChangeVisualStyle()
{
	// Reinitialize image list when visual style changes
	InitializeImageList();
	
	// Refresh all stage icons
	for (int i = 0; i < m_stages.GetCount(); i++)
	{
		UpdateStageIcon(m_stages[i].stageIndex);
	}
}

void CPipelineTreePane::SimulateProcessing()
{
	// Demo method to show status changes
	// This would be called by actual processing pipeline
	
	// Example: Set stage 0 to running
	SetStageStatus(0, STAGE_RUNNING);
	
	// After completion, set to success
	// SetStageStatus(0, STAGE_SUCCESS);
	
	// Example: Stage 1 completed with warning
	// SetStageStatus(1, STAGE_WARNING);
	
	// Example: Stage 2 failed
	// SetStageStatus(2, STAGE_ERROR);
}

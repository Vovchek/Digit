#pragma once

// Pipeline stage status enumeration
enum EPipelineStageStatus
{
	STAGE_PENDING = 0,      // Not yet executed (gray icon)
	STAGE_RUNNING = 1,      // Currently executing (blue icon with animation)
	STAGE_SUCCESS = 2,      // Completed successfully (green checkmark)
	STAGE_WARNING = 3,      // Completed with warnings (yellow warning)
	STAGE_ERROR = 4,        // Failed with error (red X)
	STAGE_SKIPPED = 5       // Skipped (gray icon, strikethrough text)
};

// Pipeline stage item structure
struct PipelineStageItem
{
	HTREEITEM hItem;
	CString strName;
	EPipelineStageStatus status;
	int stageIndex;
};

class CPipelineTreePane : public CDockablePane
{
public:
	CPipelineTreePane();
	virtual ~CPipelineTreePane();

protected:
	CTreeCtrl m_wndPipelineTree;
	CImageList m_PipelineImages;
	CArray<PipelineStageItem, PipelineStageItem&> m_stages;

public:
	void AdjustLayout();
	void OnChangeVisualStyle();
	
	// Stage status management
	void SetStageStatus(int stageIndex, EPipelineStageStatus status);
	EPipelineStageStatus GetStageStatus(int stageIndex) const;
	void ResetAllStages();
	
	// Pipeline control
	void ClearPipeline();
	void AddStage(LPCTSTR lpszName, int stageIndex);
	
	// Test/Demo methods
	void SimulateProcessing(); // Demo: simulate pipeline execution

protected:
	void UpdateStageIcon(int stageIndex);
	void InitializeImageList();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};

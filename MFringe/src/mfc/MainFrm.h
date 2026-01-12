#pragma once

// Forward declarations
class CPipelineTreePane;
class CParametersPane;
class COutputPane;

class CMFringeMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMFringeMainFrame();
	DECLARE_DYNCREATE(CMFringeMainFrame)

// Attributes
public:
	BOOL m_bCanConvertControlBarToMDI;

protected:
	// Dockable panes
	CPipelineTreePane* m_wndPipelineTree;
	CParametersPane* m_wndParameters;
	COutputPane* m_wndOutput;

	// Toolbar and menu bar
	CMFCMenuBar m_wndMenuBar;
	CMFCToolBar m_wndToolBar;
	CMFCStatusBar m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMFringeMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnViewPipelineTree();
	afx_msg void OnUpdateViewPipelineTree(CCmdUI* pCmdUI);
	afx_msg void OnViewParameters();
	afx_msg void OnUpdateViewParameters(CCmdUI* pCmdUI);
	afx_msg void OnViewOutput();
	afx_msg void OnUpdateViewOutput(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	
	// Processing menu commands
	afx_msg void OnProcessingStart();
	afx_msg void OnProcessingPause();
	afx_msg void OnProcessingStop();
	afx_msg void OnProcessingReset();
	afx_msg void OnProcessingRunSelected();
	
	// Test status commands
	afx_msg void OnTestStatusPending();
	afx_msg void OnTestStatusRunning();
	afx_msg void OnTestStatusSuccess();
	afx_msg void OnTestStatusWarning();
	afx_msg void OnTestStatusError();
	afx_msg void OnTestStatusResetAll();
	
	DECLARE_MESSAGE_MAP()
};
#pragma once

class CParametersPane : public CDockablePane
{
public:
	CParametersPane();
	virtual ~CParametersPane();

protected:
	CMFCPropertyGridCtrl m_wndPropList;
	CFont m_fntPropList;

public:
	void AdjustLayout();
	void SetVSDotNetLook(BOOL bSet);

protected:
	void InitPropList();
	void SetPropListFont();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()
};

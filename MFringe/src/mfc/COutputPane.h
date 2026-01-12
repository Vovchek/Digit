#pragma once

class COutputPane : public CDockablePane
{
public:
	COutputPane();
	virtual ~COutputPane();

protected:
	CListBox m_wndOutputBuild;
	CFont m_Font;

public:
	void UpdateFonts();
	void Clear();
	void AddMessage(LPCTSTR lpszMessage);

protected:
	void AdjustHorzScroll(CListBox& wndListBox);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

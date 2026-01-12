#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CMFringeApp : public CWinAppEx
{
public:
	CMFringeApp();
	virtual ~CMFringeApp();

// Attributes
public:
	BOOL m_bHiColorIcons;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFringeApp theApp;
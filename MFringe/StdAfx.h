// StdAfx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently

#if !defined(AFX_STDAFX_H__A1B2C3D4_E5F6_4A7B_8C9D_0E1F2A3B4C5D__INCLUDED_)
#define AFX_STDAFX_H__A1B2C3D4_E5F6_4A7B_8C9D_0E1F2A3B4C5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

// MFC core includes
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

// MFC Feature Pack includes (required for modern UI)
#include <afxwinappex.h>    // CWinAppEx
#include <afxframewndex.h>  // CFrameWndEx
#include <afxdockablepane.h> // CDockablePane
#include <afxmenubar.h>     // CMFCMenuBar
#include <afxtoolbar.h>     // CMFCToolBar
#include <afxstatusbar.h>   // CMFCStatusBar
#include <afxpropertygridctrl.h> // CMFCPropertyGridCtrl
#include <afxvisualmanager.h>    // CMFCVisualManager
#include <afxcontextmenumanager.h>
#include <afxkeyboardmanager.h>
#include <afxtooltipmanager.h>
#include <afxtooltipctrl.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <math.h>
#include <functional>

#endif // !defined(AFX_STDAFX_H__A1B2C3D4_E5F6_4A7B_8C9D_0E1F2A3B4C5D__INCLUDED_)
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// TODO: reference additional headers your program requires here
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxcview.h>
#include <afxtempl.h>
#include <afxole.h>
#include <afxcoll.h>
#include <afxpriv.h>
#include <afxpriv.h>        // for WM_IDLEUPDATECMDUI
#include <afxmt.h>        // for WM_IDLEUPDATECMDUI

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#if _WIN32_WINNT > 0x0603
#include <VersionHelpers.h>
#include <winternl.h>
#endif

#include "gtest/gtest.h"

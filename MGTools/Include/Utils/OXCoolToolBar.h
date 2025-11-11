// ==========================================================================
// 					Class Specification : 
//			COXCoolToolBar & COXCoolToolBarCmdUI
// ==========================================================================

// Header file : OXCoolToolBar.h

// Copyright © Dundas Software Ltd. 1997 - 1998, All Rights Reserved
                         
// //////////////////////////////////////////////////////////////////////////

// Properties:
//	NO	Abstract class 
//	YES	Derived from CToolBar / CCmdUI

//	YES / NO	Is a Cwnd. 
//	YES / NO	Two stage creation 
//	YES / NO	Has a message map

//	NO	Persistent objects (saveable on disk)      
//	NO	Uses exceptions

// //////////////////////////////////////////////////////////////////////////

// Desciption :         
//
// COXCoolToolBarCmdUI   - CCmdUI derived class.
//
// COXCoolToolBar - New toolbar that utilizes new functionality provided by new
//					commctrl.dll (supplied with IE 3.0 and now redistibutable.
//					You have to use MFC >=4.2). 
//					New features:
//						1)	flat mode;
//						2)	cool mode (can be used in CoolBars);
//						3)	in flat mode draws gripper and separators (Office'97 look);
//						4)	list mode (looks like Links toolbar in IE 3.0);
//						5)	set different image lists for default, disabled and hot mode
//							(to get the effect like in IE 3.0);
//						6)	set text color for default and hot mode;
//						7)	set of functions to set text to buttons in CoolToolBar 
//							(you can set MinMaxWidth of text in row and 
//							the number of text rows in buttons, 
//							you can automatically set text to buttons
//							using string or menu resources);
//						8)	change CoolToolBar style at run-time;
//						9)	minor customization functionality using standard 
//							Customize dialog, 
//						10) persistence - you can save and load state of CoolToolBar
//						11) now you've got easy way to place toolbars on one line
//
//
//					To utilize this class you have to do (in case with standard project
//					created using MFC App Wizard):
//					(*)	1)	add #include "OXCoolToolBar.h" to your "MainFrm.h" header file;
//					(*)	2)	change CToolBar on COXCoolToolBar;
//						after creation of CoolToolBar:
//						3)	use IniSizes(CSize& sizeImage) if you use 
//								non-standard size of images for buttons
//						4)	CoolToolBar created with flat style set by default, 
//							if you don't want this feature call SetFlat(FALSE);
//						5)	if you are going to use CoolToolBar within CoolBar then 
//							call SetCool(TRUE);
//						6)	if you want you CoolToolBar to be customizable call 
//							SetBitmapIds(UINT* pIds, int nButtons) function to set
//							an array of button Ids to use in Customize dialog andin 
//							save and load state functions (if you don't call this 
//							function your CoolToolBar uncustomizable by default).
//							It is up to you to create and delete (if needed) 
//							array of buttons ID;
//						7)	if you want to set text to buttons then set 
//							max width of buttons and 
//							max number of text rows in a button;
//						8)	if needed call functions to set images to show in 
//							default, disabled and hot mode:
//							HIMAGELIST SetDefaultImageList(HIMAGELIST hImageList);
//							HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList);
//							HIMAGELIST SetHotImageList(HIMAGELIST hImageList);
//							If you reside Image Lists in resource file 
//							it is recommended to use to create Image Lists
//							COXCoolToolBar-inner function: 
//							HIMAGELIST AddImageList(UINT nImageListID, int cx=16, 
//								int cGrow=0, COLORREF crMask=RGB(255,0,255));
//							In that case you shouldn't care about deleting used 
//							Image Lists, COXCoolToolBar destructor 
//							will do all job for you
//						9)	if needed call functions to set Text Color in 
//							default and/or hot mode:
//							void SetDefaultTextColor(COLORREF crTextColor);
//							void SetHotTextColor(COLORREF crTextColor);
//						10)	if needed set indent to CoolToolBar
//					(*)	11)	call UpdateSizes() to correctly set sizes of buttons
//
//			(*) - marked steps that have to be taken to use CoolToolBar;
//			steps from 4 to 10 can be taken in any order
//							
// Remark:
//					1)	although COXCoolToolBar heavily uses new commctrl.dll and 
//						most features available only if you use MFC>=4.2 
//						you still can use customization functionality plus all functions
//						that are not based on SendMessage() functions;
//					2)	there is one problem using CoolToolBar Customize dialog:
//						if flat style is set then in customize dialog all buttons 
//						are drawn as transparent and button for separator is invisible;
//
// Prerequisites (necessary conditions):
//					To use this class, you must include the
//					header files:
//						 <afxcmn.h>
//						 <afxext.h>
//						 <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////

#ifndef _COOLTOOLBAR_H
#define _COOLTOOLBAR_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "OXDllExt.h"
#include "MGToolsExport.h"


// OXCoolToolBar.h : header file
//

// there are some changes in new common control dll version 4.71
// which is supplied with IE 4.0. We use constant defined in the 
// new dll - _WIN32_IE that define the version of using Internet Explorer 
// (it is set to 0x04000). Actually we can use this constant to define 
// version of using common control dll
//
//	_WIN32_IE		comctl32.dll version	
//	  0x4000				4.71, 4.72		IE 4.0-4.01
//	  0x3000				4.70			IE 3.0
//	  0x2000				4.00
//
#ifndef _WIN32_IE
#if _MFC_VER>=0x0420
#define _WIN32_IE 0x0300
#else
#define _WIN32_IE 0x0200
#endif
#endif

#ifndef __AFXCMN_H__
#include <afxcmn.h>
#define __AFXCMN_H__
#endif

#ifndef __AFXEXT_H__
#include <afxext.h>
#define __AFXEXT_H__
#endif

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#define __AFXTEMPL_H__
#endif

#ifndef __AFXPRIV_H__
#include <afxpriv.h>
#define __AFXPRIV_H__
#endif

#ifndef __OXMFCIMPL_H__
//#include <..\src\afximpl.h>
#include <..\src\mfc\afximpl.h>
#define __OXMFCIMPL_H__
#endif


#if _MFC_VER<0x0421
#define TBSTYLE_LIST				0x1000
#define TB_SETBUTTONWIDTH			(WM_USER + 59)
#define TB_SETMAXTEXTROWS		    (WM_USER + 60)
#define TB_GETTEXTROWS				(WM_USER + 61)
#endif

// we need these type definitions to get information about dll version
// we use it in our GetComCtlVersion function
#ifndef _OXDLLVERSIONINFO_DECL
#define _OXDLLVERSIONINFO_DECL
#if _MFC_VER >0x0421
#include <shlwapi.h>
#else
typedef struct _DllVersionInfo
{
	DWORD cbSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformID;
}DLLVERSIONINFO;
#define DLLVER_PLATFORM_WINDOWS		0	// Windows platform (95/98)
#define DLLVER_PLATFORM_NT			1	// Windows NT platform
typedef int (FAR WINAPI *DLLGETVERSIONPROC) (DLLVERSIONINFO *);
#endif
#endif
///////////////////////////

// this structure is defined in new commctrl.h header file
// as long as we want our class to work with all version of 
// comctl32.dll we have to provide definiton of this structure
//
// Custom Draw Structure
#if _WIN32_IE<0x0400
typedef struct _NMTBCUSTOMDRAW 
{
    NMCUSTOMDRAW nmcd;
    HBRUSH hbrMonoDither;
    HBRUSH hbrLines;                // For drawing lines on buttons
    HPEN hpenLines;                 // For drawing lines on buttons

    COLORREF clrText;               // Color of text
    COLORREF clrMark;               // Color of text bk when marked. (only if TBSTATE_MARKED)
    COLORREF clrTextHighlight;      // Color of text when highlighted
    COLORREF clrBtnFace;            // Background of the button
    COLORREF clrBtnHighlight;       // 3D highlight
    COLORREF clrHighlightHotTrack;  // In conjunction with fHighlightHotTrack
                                    // will cause button to highlight like a menu
    RECT rcText;                    // Rect for text

    int nStringBkMode;
    int nHLStringBkMode;
} NMTBCUSTOMDRAW, * LPNMTBCUSTOMDRAW;
#endif

#if _WIN32_IE<0x0400
#define TBSTYLE_EX_DRAWDDARROWS 0x00000001
#define TBSTYLE_TRANSPARENT     0x8000
#define TBSTYLE_AUTOSIZE        0x0010 // automatically calculate the cx of the button
#define TBSTYLE_REGISTERDROP    0x4000
#define TBSTYLE_NOPREFIX        0x0020 // if this button should not have accel prefix
#endif

#if _WIN32_IE<0x0400
#define TBN_GETOBJECT           (TBN_FIRST - 12)

// Structure for TBN_HOTITEMCHANGE notification
//
typedef struct tagNMTBHOTITEM
{
    NMHDR   hdr;
    int     idOld;
    int     idNew;
    DWORD   dwFlags;           // HICF_*
} NMTBHOTITEM, * LPNMTBHOTITEM;

// Hot item change flags
#define HICF_OTHER          0x00000000
#define HICF_MOUSE          0x00000001          // Triggered by mouse
#define HICF_ARROWKEYS      0x00000002          // Triggered by arrow keys
#define HICF_ACCELERATOR    0x00000004          // Triggered by accelerator
#define HICF_DUPACCEL       0x00000008          // This accelerator is not unique
#define HICF_ENTERING       0x00000010          // idOld is invalid
#define HICF_LEAVING        0x00000020          // idNew is invalid
#define HICF_RESELECT       0x00000040          // hot item reselected


#define TBN_HOTITEMCHANGE       (TBN_FIRST - 13)
#define TBN_DRAGOUT             (TBN_FIRST - 14) // this is sent when the user clicks down on a button then drags off the button
#define TBN_DELETINGBUTTON      (TBN_FIRST - 15) // uses TBNOTIFY
#define TBN_GETDISPINFOA        (TBN_FIRST - 16) // This is sent when the  toolbar needs  some display information
#define TBN_GETDISPINFOW        (TBN_FIRST - 17) // This is sent when the  toolbar needs  some display information
#define TBN_GETINFOTIPA         (TBN_FIRST - 18)
#define TBN_GETINFOTIPW         (TBN_FIRST - 19)


typedef struct tagNMTBGETINFOTIPA
{
    NMHDR hdr;
    LPSTR pszText;
    int cchTextMax;
    int iItem;
    LPARAM lParam;
} NMTBGETINFOTIPA, *LPNMTBGETINFOTIPA;

typedef struct tagNMTBGETINFOTIPW
{
    NMHDR hdr;
    LPWSTR pszText;
    int cchTextMax;
    int iItem;
    LPARAM lParam;
} NMTBGETINFOTIPW, *LPNMTBGETINFOTIPW;

#ifdef UNICODE
#define TBN_GETINFOTIP          TBN_GETINFOTIPW
#define NMTBGETINFOTIP          NMTBGETINFOTIPW
#define LPNMTBGETINFOTIP        LPNMTBGETINFOTIPW
#else
#define TBN_GETINFOTIP          TBN_GETINFOTIPA
#define NMTBGETINFOTIP          NMTBGETINFOTIPA
#define LPNMTBGETINFOTIP        LPNMTBGETINFOTIPA
#endif

#define TBNF_IMAGE              0x00000001
#define TBNF_TEXT               0x00000002
#define TBNF_DI_SETITEM         0x10000000

typedef struct {
    NMHDR  hdr;
    DWORD dwMask;     // [in] Specifies the values requested .[out] Client ask the data to be set for future use
    int idCommand;    // [in] id of button we're requesting info for
    DWORD lParam;     // [in] lParam of button
    int iImage;       // [out] image index
    LPSTR pszText;    // [out] new text for item
    int cchText;      // [in] size of buffer pointed to by pszText
} NMTBDISPINFOA, *LPNMTBDISPINFOA;

typedef struct {
    NMHDR hdr;
    DWORD dwMask;      //[in] Specifies the values requested .[out] Client ask the data to be set for future use
    int idCommand;    // [in] id of button we're requesting info for
    DWORD lParam;     // [in] lParam of button
    int iImage;       // [out] image index
    LPWSTR pszText;   // [out] new text for item
    int cchText;      // [in] size of buffer pointed to by pszText
} NMTBDISPINFOW, *LPNMTBDISPINFOW;


#ifdef UNICODE
#define TBN_GETDISPINFO       TBN_GETDISPINFOW
#define NMTBDISPINFO	      NMTBDISPINFOW
#define LPNMTBDISPINFO        LPNMTBDISPINFOW
#else
#define TBN_GETDISPINFO       TBN_GETDISPINFOA
#define NMTBDISPINFO          NMTBDISPINFOA
#define LPNMTBDISPINFO        LPNMTBDISPINFOA
#endif

// Return codes for TBN_DROPDOWN
#define TBDDRET_DEFAULT         0
#define TBDDRET_NODEFAULT       1
#define TBDDRET_TREATPRESSED    2       // Treat as a standard press button

#endif

#if _WIN32_IE<0x0400
#define TBSTATE_MARKED          0x80
#endif

#if _WIN32_IE<0x0400
#define TB_MARKBUTTON           (WM_USER + 6)
#define TB_ISBUTTONHIGHLIGHTED  (WM_USER + 14)
#define TB_GETOBJECT            (WM_USER + 62)  // wParam == IID, lParam void **ppv
#define TB_GETHOTITEM           (WM_USER + 71)
#define TB_SETHOTITEM           (WM_USER + 72)  // wParam == iHotItem
#define TB_SETANCHORHIGHLIGHT   (WM_USER + 73)  // wParam == TRUE/FALSE
#define TB_GETANCHORHIGHLIGHT   (WM_USER + 74)
#define TB_MAPACCELERATORA      (WM_USER + 78)  // wParam == ch, lParam int * pidBtn

typedef struct {
    int   iButton;
    DWORD dwFlags;
} TBINSERTMARK, * LPTBINSERTMARK;
#define TBIMHT_AFTER      0x00000001 // TRUE = insert After iButton, otherwise before
#define TBIMHT_BACKGROUND 0x00000002 // TRUE iff missed buttons completely

#define TB_GETINSERTMARK        (WM_USER + 79)  // lParam == LPTBINSERTMARK
#define TB_SETINSERTMARK        (WM_USER + 80)  // lParam == LPTBINSERTMARK
#define TB_INSERTMARKHITTEST    (WM_USER + 81)  // wParam == LPPOINT lParam == LPTBINSERTMARK
#define TB_MOVEBUTTON           (WM_USER + 82)
#define TB_GETMAXSIZE           (WM_USER + 83)  // lParam == LPSIZE
#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#define TB_GETEXTENDEDSTYLE     (WM_USER + 85)  // For TBSTYLE_EX_*
#define TB_GETPADDING           (WM_USER + 86)
#define TB_SETPADDING           (WM_USER + 87)
#define TB_SETINSERTMARKCOLOR   (WM_USER + 88)
#define TB_GETINSERTMARKCOLOR   (WM_USER + 89)

#define TB_SETCOLORSCHEME       CCM_SETCOLORSCHEME  // lParam is color scheme
#define TB_GETCOLORSCHEME       CCM_GETCOLORSCHEME	// fills in COLORSCHEME pointed to by lParam

#define TB_SETUNICODEFORMAT     CCM_SETUNICODEFORMAT
#define TB_GETUNICODEFORMAT     CCM_GETUNICODEFORMAT

#define TB_MAPACCELERATORW      (WM_USER + 90)  // wParam == ch, lParam int * pidBtn
#ifdef UNICODE
#define TB_MAPACCELERATOR       TB_MAPACCELERATORW      
#else
#define TB_MAPACCELERATOR       TB_MAPACCELERATORA
#endif
#endif


#if _WIN32_IE<0x0400
#define TBIF_IMAGE              0x00000001
#define TBIF_TEXT               0x00000002
#define TBIF_STATE              0x00000004
#define TBIF_STYLE              0x00000008
#define TBIF_LPARAM             0x00000010
#define TBIF_COMMAND            0x00000020
#define TBIF_SIZE               0x00000040

typedef struct {
    UINT cbSize;
    DWORD dwMask;
    int idCommand;
    int iImage;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;
    DWORD lParam;
    LPSTR pszText;
    int cchText;
} TBBUTTONINFOA, *LPTBBUTTONINFOA;

typedef struct {
    UINT cbSize;
    DWORD dwMask;
    int idCommand;
    int iImage;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;
    DWORD lParam;
    LPWSTR pszText;
    int cchText;
} TBBUTTONINFOW, *LPTBBUTTONINFOW;

#ifdef UNICODE
#define TBBUTTONINFO TBBUTTONINFOW
#define LPTBBUTTONINFO LPTBBUTTONINFOW
#else
#define TBBUTTONINFO TBBUTTONINFOA
#define LPTBBUTTONINFO LPTBBUTTONINFOA
#endif


// BUTTONINFO APIs do NOT support the string pool.
#define TB_GETBUTTONINFOW        (WM_USER + 63)
#define TB_SETBUTTONINFOW        (WM_USER + 64)
#define TB_GETBUTTONINFOA        (WM_USER + 65)
#define TB_SETBUTTONINFOA        (WM_USER + 66)
#ifdef UNICODE
#define TB_GETBUTTONINFO        TB_GETBUTTONINFOW
#define TB_SETBUTTONINFO        TB_SETBUTTONINFOW
#else
#define TB_GETBUTTONINFO        TB_GETBUTTONINFOA
#define TB_SETBUTTONINFO        TB_SETBUTTONINFOA
#endif


#define TB_INSERTBUTTONW        (WM_USER + 67)
#define TB_ADDBUTTONSW          (WM_USER + 68)

#define TB_HITTEST              (WM_USER + 69)

#define TB_SETDRAWTEXTFLAGS     (WM_USER + 70)  // wParam == mask lParam == bit values

#endif         // _WIN32_IE < 0x0400



// notification sent to parent window so you can provide custom drawing 
// without deriving your own class. OXCTB_CUSTOMDRAW is sent as NMHDR for 
// this notification
#define OXCTBN_CUSTOMDRAW		WM_APP+150

// this structure will be sent to the parent window to provide Custom Draw
// functionality:
//
// hdr				-	standard NMHDR
// pCustomDraw		-	pointer to NMCUSTOMDRAW (version comctl <= 4.70) or 
//						NMTBCUSTOMDRAW (version comctl >= 4.71). See SDK documentation
//						for description of these structures and also refer to the 
//						documentation for NM_CUSTOMDRAW notification. As an example 
//						of using these structures in your OXCTBN_CUSTOMDRAW handler
//						you can use COXCoolToolBar::OnCustomDraw function
// result			-	The value your application can return depends on the current 
//						drawing stage. dwDrawStage member of the associated NMCUSTOMDRAW 
//						structure holds a value that specifies the drawing stage. You must 
//						return one of the following values. 
//
//						When dwDrawStage equals CDDS_PREPAINT:  
//
//						CDRF_DODEFAULT			-	The control will draw itself. It will 
//													not send any additional OXCTBN_CUSTOMDRAW 
//													messages for this paint cycle. 
//						CDRF_NOTIFYITEMDRAW		-	The control will notify the parent of 
//													any item-related drawing operations. 
//													It will send OXCTBN_CUSTOMDRAW 
//													notification messages before and after 
//													drawing items. 
//						CDRF_NOTIFYITEMERASE	-	The control will notify the parent when 
//													an item will be erased. It will send 
//													OXCTBN_CUSTOMDRAW notification messages 
//													before and after erasing items. 
//						CDRF_NOTIFYPOSTERASE	-	The control will notify the parent after
//													erasing an item. 
//						CDRF_NOTIFYPOSTPAINT	-	The control will notify the parent after 
//													painting an item. 
//						CDRF_NOTIFYSUBITEMDRAW	-	Version 4.71. The control will notify 
//													the parent when a list view subitem is 
//													being drawn.  
//
//						When dwDrawStage equals CDDS_ITEMPREPAINT:  
//
//						CDRF_NEWFONT			-	Your application specified a new font 
//													for the item; the control will use the 
//													new font. 
//						CDRF_SKIPDEFAULT		-	Your application drew the item manually.
//													The control will not draw the item. 
//						TBCDRF_NOEDGES			-	Version 4.71. Don't draw button edges. 
//						TBCDRF_HILITEHOTTRACK	-	Version 4.71. Use the clrHighlightHotTrack 
//													member of the NMTBCUSTOMDRAW structure 
//													to drawing the background of hot-tracked 
//													items. 
//						TBCDRF_NOOFFSET			-	Version 4.71. Don't offset button when 
//													pressed. 
//						TBCDRF_NOMARK			-	Version 4.71. Don't draw default 
//													highlight of items that have the 
//													TBSTATE_MARKED state. 
//						TBCDRF_NOETCHEDEFFECT	-	Version 4.71. Don't draw etched effect 
//													for disabled items.  
//
// bDoDefault		-	if TRUE then default implementation will be used. Note that in that
//						case result member of this structure won't be used
//
typedef struct tagOXCTB_CUSTOMDRAW
{
	NMHDR hdr;
	LPNMCUSTOMDRAW pCustomDraw;
	LRESULT result;
	BOOL bDoDefault;
} OXCTB_CUSTOMDRAW, * LPOXCTB_CUSTOMDRAW;

/////////////////////////////////////////

// IE 4.0 uses comctl32.dll version 4.71
// i.e. major=4, minor=71 
// then we will get _IE40_COMCTL_VERSION as MAKELONG(minor,major) 
#define _IE40_COMCTL_VERSION 0x00040047

// helper class 
class OX_CLASS_DECL COXCoolToolBarCmdUI : public CCmdUI    
{
// Data members -------------------------------------------------------------
public:

protected:

private :

// Member functions ---------------------------------------------------------
public: // re-implementations only

	virtual void Enable(BOOL bOn);
	// --- In  : See CCmdUI::Enable
	// --- Out : See CCmdUI::Enable
	// --- Returns : See CCmdUI::Enable
	// --- Effect : See CCmdUI::Enable

	virtual void SetCheck(int nCheck);
	// --- In  : See CCmdUI::SetCheck
	// --- Out : See CCmdUI::SetCheck
	// --- Returns : See CCmdUI::SetCheck
	// --- Effect : See CCmdUI::SetCheck

protected:

private :

};


#if _MFC_VER>=0x0420
static const TCHAR szTextType[]=_T("TextType");
static const TCHAR szCool[]=_T("Cool");
static const TCHAR szGripper[]=_T("Gripper");
static const TCHAR szSeparator[]=_T("Separator");
static const TCHAR szFlat[]=_T("Flat");
static const TCHAR szList[]=_T("List");
static const TCHAR szIndent[]=_T("Indent");
static const TCHAR szDefaultTextColor[]=_T("DefaultTextColor");
static const TCHAR szHotTextColor[]=_T("HotTextColor");
static const TCHAR szTextRows[]=_T("TextRows");
static const TCHAR szMinButtonsWidth[]=_T("MinButtonsWidth");
static const TCHAR szMaxButtonsWidth[]=_T("MaxButtonsWidth");
static const TCHAR szDropDownArrow[]=_T("DropDownArrow");
static const TCHAR szDropDownButtonList[]=_T("DropDownButtonList");
#endif
static const TCHAR szCustomizable[]=_T("Customizable");

/////////////////////////////////////////////////////////////////////////////
// COXCoolToolBar window

class MGTOOLS_API OX_CLASS_DECL COXCoolToolBar : public CToolBar
{
	DECLARE_DYNAMIC(COXCoolToolBar)
// Construction
public:
	// --- In  :
	// --- Out : 
	// --- Returns:
	// --- Effect : Constructs the object
	COXCoolToolBar();

// Attributes
public:
	enum TextTypeFromID	{ TTID_PLAIN, TTID_TOOLTIP, TTID_MENU, TTID_NOTSET, TTID_NONE };
	// functions to automatically set text to buttons
	//	TTID_PLAIN - lookup string resource for var equals button's ID
	//			if found, set text in button to string text 
	//			(not includded tool tip text)
	//	TTID_TOOLTIP - lookup string resource for var equals button's ID
	//			if found, set text in button to tooltip text 
	//	TTID_MENU - lookup current active menu for item that equals to button's ID
	//			if found, set text in button to text of corresponding Item in menu
	//	TTID_NOTSET - default value (there is no any text associated with toolbar)
	//	TTID_NONE - remove all text associated with buttons


protected:

	// set to TRUE if WindowsNT running
	// some draw routines behave differently under Windows95 and WindowsNT
	BOOL m_bWindowsNTRunning;

	// defines version of comctl32.dll currently installed on the computer
	// GetComCtlVersion function returns major and minor version of this dll
	// and we combine these values using MAKELONG macro
	static DWORD m_dwComCtlVersion;

	// defines size of dropdown arrow which is drawn if utton has TBSTYLE_DROPDOWN set
	// and CoolToolBar has TBSTYLE_EX_DRAWDDARROWS extended style set
	int m_nDropDownArrowWidth;

	// type of text that is set to CoolToolBar
	TextTypeFromID m_ttID;

#if _MFC_VER>=0x0420
	// if we use CoolToolBar in CoolBar then this flag must be set 
	// in order to correctly render bar on screen
	BOOL m_bCool;
	// if set to TRUE and CoolToolBar is Flat and not Cool then
	// it draws on the left side or on the top of Bar special Gripper
	BOOL m_bGripper;
	// if set to TRUE and CoolToolBar is Flat and not Cool then
	// it draws the separator
	BOOL m_bSeparator; 
	// if set to TRUE and any button in the toolbar have TBSTYLE_DROPDOWN style
	// it would be drawn altogether with dropdown arrow, otherwise it wouldn't
	BOOL m_bDropDownArrow;

	// Save the width of indention because there is no such message as TB_GETINDENT
	int m_nIndent;

	// Save the min and max width of buttons because 
	// there is no such message as TB_GETBUTTONWIDTH
	CSize m_sizeMinMaxWidth;

	// colors of text in default and hot mode
	COLORREF m_crDefaultTextColor;
	COLORREF m_crHotTextColor;
#endif

	// flag that specifies the previous state (docking/floating) 
	// of the control
	BOOL m_bPrevFloating;

	// variable that specifies the previous docking side of the control
	DWORD m_dwPrevDockSide;

	// icon to be displayed at the left or top side
	HICON m_hIcon;
	// rectangle for icon
	CRect m_iconRect;


protected:
	// array of IDs to show in the Bar
	UINT* m_pBitmapIds;				// ID pallette in the bitmap 
	// the number of all buttons
	int	m_nBitmapButtons;			// number of buttons in the toolbar palette

	// map array to keep all created handles to bitmap used by CoolToolBar
	typedef CMap<LPTSTR, LPTSTR, HBITMAP, HBITMAP> allBitmaps;
	allBitmaps m_allBitmaps;

	// map array to keep all created handles to image lists used by CoolToolBar
	typedef CMap<LPTSTR, LPTSTR, HANDLE, HANDLE> allImageLists;
	allImageLists m_allImageLists;

// Operations

public:
	
	// functions you can use to create bitmaps and image lists from resource file
	// to use them in CoolToolBar. In this case you shouldn't care about destroying 
	// created GDI object, CoolToolBar will destroy it automatically
	inline HBITMAP AddBitmap(UINT nBitmapID)
	{
		return AddBitmap(MAKEINTRESOURCE(nBitmapID));
	}
	HBITMAP AddBitmap(LPTSTR lpszResourceName);

	inline HIMAGELIST AddImageList(UINT nImageListID, int cx=16, 
								   int cGrow=0, COLORREF crMask=RGB(192, 192, 192), 
								   UINT uType=IMAGE_BITMAP, UINT uFlags=LR_DEFAULTCOLOR)
	{
		return AddImageList(MAKEINTRESOURCE(nImageListID),cx,cGrow,crMask,uType,uFlags);
	}
	HIMAGELIST AddImageList(LPTSTR lpszResourceName, int cx=16, 
							int cGrow=0, COLORREF crMask=RGB(192, 192, 192), 
							UINT uType=IMAGE_BITMAP, 
							UINT uFlags=LR_DEFAULTCOLOR);

	inline HIMAGELIST AddImageList(UINT nImageListID, int cx, 
								   COLORREF crMask=RGB(192, 192, 192), 
								   UINT uFlags=ILC_COLOR|ILC_MASK,
								   int cInitial=0, int cGrow=0)
	{
		return AddImageList(MAKEINTRESOURCE(nImageListID),cx,crMask,uFlags,cInitial,cGrow);
	}
	HIMAGELIST AddImageList(LPTSTR lpszResourceName, int cx, 
							COLORREF crMask=RGB(192, 192, 192), 
							UINT uFlags=ILC_COLOR|ILC_MASK,
							int cInitial=0, int cGrow=0);

	// standard create function
	// as soon as we set flat mode by default we do that in that function
	BOOL Create(CWnd* pParentWnd, DWORD dwStyle=WS_CHILD|WS_VISIBLE|CBRS_TOP,
		UINT nID=AFX_IDW_TOOLBAR);

	// --- In  : pIds : Points to an array of button bitmap Ids.
	//			 nButtons : Number of buttons in the above array.
	// --- Out : 
	// --- Returns :
	// --- Effect : It specifies an array of command IDs that the images 
	//				in the bitmap represent. Common control customization
	//				dialog uses it to determine which buttons could possible be 
	//				on the toolbar. The idea is that you call SetBitmapIds() 
	//				to set the ID’s that correspond  to the images in the bitmap, 
	//				and SetButtons() to determine the command ID’s
	//				that are actually on the toolbar. 
	//
	//				The common control customization allows you to drag buttons,
	//				by holding the shift button and clicking and dragging the button, 
	//				either to a new position on the toolbar, or off it 
	//				(to remove it from the toolbar). Double-clicking displays 
	//				the configuration dialog.
	///////////////////////////////////
	void SetBitmapIds(UINT* pIds, int nButtons);

	// if parameter is TRUE then providing that you call function SetBitmapIds() 
	// before, you can customize the CoolToolBar
	void SetCustomizable(BOOL bCustomizable=TRUE);
	inline BOOL IsCustomizable() const { 
		return (GetStyle()&CCS_ADJUSTABLE ? TRUE : FALSE); 
	}

#if _MFC_VER>=0x0420
	// set whether CoolToolBar in flat mode or not
	inline void SetFlat(BOOL bFlat=TRUE) { ModifyStyle(bFlat ? 0 : TBSTYLE_FLAT, 
		bFlat ? TBSTYLE_FLAT : 0); }
	// set whether CoolToolBar in list mode or not
	inline void SetList(BOOL bList=TRUE) { ModifyStyle(bList ? 0 : TBSTYLE_LIST, 
		bList ? TBSTYLE_LIST : 0); }
	// set flag m_bCool which defines the way of painting CoolToolBar 
	// in CoolBars
	inline void SetCool(BOOL bCool=TRUE) { m_bCool=bCool; }
	// whether draw gripper in flat mode or not
	inline void SetGripper(BOOL bGripper=TRUE) { m_bGripper=bGripper; }
	// whether draw separator in flat mode or not
	inline void SetSeparator(BOOL bSeparator=TRUE) { m_bSeparator=bSeparator; }
	// whether draw dropdown arrow on dropdown buttons or not
	void SetDropDownArrow(BOOL bDropDownArrow=TRUE);

	inline BOOL IsFlat() const { return GetStyle()&TBSTYLE_FLAT ? TRUE : FALSE; }
	inline BOOL IsList() const { return GetStyle()&TBSTYLE_LIST ? TRUE : FALSE; }
	inline BOOL IsCool() const { return m_bCool; }
	inline BOOL IsGripper() const { return m_bGripper; }
	inline BOOL IsSeparator() const { return m_bSeparator; }
	BOOL IsDropDownArrow() const;

	// set color of the text (default/hot mode)
	// by default we use COLOR_BTNTEXT color
	inline void SetDefaultTextColor(COLORREF crTextColor) { 
		m_crDefaultTextColor=crTextColor; }
	inline COLORREF GetDefaultTextColor() const { return m_crDefaultTextColor; }

	inline void SetHotTextColor(COLORREF crTextColor) { m_crHotTextColor=crTextColor; }
	inline COLORREF GetHotTextColor() const { return m_crHotTextColor; }
#endif

	// if we want to use non-standard image (not 16x15) we have to initialize
	// sizes of images and buttons to let CoolToolBar to handle them correctly
	inline void IniSizes(CSize csBitmapSize) 
	{ 
		SetSizes(CSize(csBitmapSize.cx+8,csBitmapSize.cy+6),csBitmapSize); 
		GetToolBarCtrl().AutoSize();
	}
#if _MFC_VER>=0x0420
	// we have to call this function to correctly set the size of buttons.
	// usually, this is the last function in the set of functions that we use
	// to initialize CoolToolBar
	inline void UpdateSizes() 
	{ 
		SetSizes(GetButtonsSize(),m_sizeImage); 
		GetToolBarCtrl().AutoSize();
	}
#endif



#if _MFC_VER>=0x0420
	///////////////////
	// Message wrappers
	///////////////////

	// use to change style of CoolToolBar dynamically
	inline void SetStyle(DWORD dwStyle)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		SendMessage(TB_SETSTYLE, 0, (LPARAM)dwStyle); 
	}
	// Get style of bar 
	inline DWORD GetStyle() const
	{
		ASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd,TB_GETSTYLE,0,(LPARAM)0); 
	}

	// use to change extended style of CoolToolBar dynamically
	inline void SetStyleEx(DWORD dwStyleEx)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM)dwStyleEx); 
	}
	// Get extended style of CoolToolBar 
	inline DWORD GetStyleEx() const
	{
		ASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd,TB_GETEXTENDEDSTYLE,0,(LPARAM)0); 
	}

	// wParam is the Cmd not index
	// does not work correctly
	// for separator it returns always coordinates of first one
/*	BOOL GetRect(UINT nItemID, LPRECT lpRect)
		{ ASSERT(::IsWindow(m_hWnd));
		  return (BOOL)SendMessage(TB_GETRECT, nItemID, (LPARAM)lpRect); }*/

	// you can set image list for buttons for default, disabled and hot conditions
	// using hot image list you can make your CoolToolBar looks like one found
	// in the Internet Explorer. The fastest way to do that just create hot image list
	// in resource editor and set it to CoolToolBar using SetHotImageList(), 
	// you don't have to create default and disabled image lists providing 
	// that you call LoadBitmap() or LoadToolbar() functions before.

	// if the handle to the image list is NULL, then
	// no images will be displayed in the buttons
	inline HIMAGELIST SetDefaultImageList(HIMAGELIST hImageList)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)SendMessage(TB_SETIMAGELIST, 0, (LPARAM)hImageList); 
	}
	inline HIMAGELIST GetDefaultImageList() const
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd,TB_GETIMAGELIST,0,(LPARAM)0); 
	}
	inline HIMAGELIST SetDisabledImageList(HIMAGELIST hImageList)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)SendMessage(TB_SETDISABLEDIMAGELIST, 0, 
			(LPARAM)hImageList); 
	}
	inline HIMAGELIST GetDisabledImageList() const
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd,TB_GETDISABLEDIMAGELIST,0,(LPARAM)0); 
	}
	inline HIMAGELIST SetHotImageList(HIMAGELIST hImageList)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageList); 
	}
	inline HIMAGELIST GetHotImageList() const
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd,TB_GETHOTIMAGELIST,0,(LPARAM)0); 
	}

	// you can load any bitmap from any loaded application as default image.
	// There is new set of system defined images that you can use in your application
	// IDB_HIST_SMALL_COLOR and IDB_HIST_LARGE_COLOR.
	// To load these images use IDB_XXX as nBitmapID and HINST_COMMCTRL as hInstance
	// !!! The default image list must have already been set using SetDefaultImageList()
	// !!! before use this function
	// returns an INT value that represents the count of images in the image list, 
	// not including the one just added. Returns zero if the toolbar has no 
	// image list or if the existing image list is empty. 
	inline int LoadImage(int nBitmapID, HINSTANCE hInstance)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return (int)SendMessage(TB_LOADIMAGES, (WPARAM)nBitmapID, (LPARAM)hInstance); 
	}

	// set minimum and maximum width of button
	// very useful when you want to set descriptive text to button
	// !!! Note that like in case with standard SetSizes() function
	// !!! nMaxWidth must be equal or more 
	// !!! than size of bitmap associated with button + 7 points
	// returns nonzero if successful, or zero otherwise
	BOOL SetButtonsMinMaxWidth(UINT nMinWidth, UINT nMaxWidth)
	{ 
		ASSERT(::IsWindow(m_hWnd));
		ASSERT(nMaxWidth-m_sizeImage.cx>=7);
		if(SendMessage(TB_SETBUTTONWIDTH, 0, MAKELPARAM(nMinWidth, nMaxWidth)))
		{
			m_sizeMinMaxWidth.cx=nMinWidth;
			m_sizeMinMaxWidth.cy=nMaxWidth;
			return TRUE;
		}
		return FALSE; 
	}

	// returns CSize object with cx set to min buttons width
	// and cy set to max buttons width
	inline CSize GetButtonsMinMaxWidth() const
	{ 
		ASSERT(::IsWindow(m_hWnd));
		return m_sizeMinMaxWidth; 
	}

	// now you don't have to calculate the size of buttons in CoolToolBar yourself
	// GetButtonSize will do all job. You can use result in SetSizes function
	inline CSize GetButtonsSize() const
	{ 
		ASSERT(::IsWindow(m_hWnd));
		DWORD result=(DWORD)::SendMessage(m_hWnd,TB_GETBUTTONSIZE,0,(LPARAM)0); 
		return CSize(LOWORD(result),HIWORD(result)); 
	}

	// now you can set max number of text rows in the CoolToolBar
	// returns nonzero if successful, or zero otherwise
	inline BOOL SetMaxTextRows(int nRows)
	{
		ASSERT(::IsWindow(m_hWnd));
		return (BOOL)SendMessage(TB_SETMAXTEXTROWS, nRows, (LPARAM)0); 
	}
	inline int GetTextRows() const
	{
		ASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd,TB_GETTEXTROWS,0,(LPARAM)0); 
	}

	// you can set indent for CoolToolBar
	// returns nonzero if successful, or zero otherwise
	inline BOOL SetIndent(int nIndent)
	{
		ASSERT(::IsWindow(m_hWnd));
		if(SendMessage(TB_SETINDENT, nIndent, (LPARAM)0)) 
		{
			m_nIndent=nIndent;
			return TRUE;
		}
		return FALSE;
	}
	inline int GetIndent() const { return m_nIndent; }
#endif

	// use this function to associate buttons with text that is 
	// located in corresponding string or menu resources
	inline void SetButtonTextFromID(TextTypeFromID ttID=TTID_PLAIN) 
	{ 
		SetButtonTextFromID(0, 0xffff, ttID); 
	}
	
	void SetButtonTextFromID(UINT nFirstButtonID, UINT nLastButtonID, 
		TextTypeFromID ttID=TTID_PLAIN);
	inline TextTypeFromID GetTextType() const { return m_ttID; }

	// save to and load from registry state of buttons in CoolToolBar
	// we use default registry key assigned to your application by MFC
	// make sure you called SetButtonsIds() and 
	// CWinApp::SetRegistryKey() functions before.
	// Makes no sense to load/store buttons state info for uncustomizable CoolToolBar
	//
	////////////////////////
	// if bProperties is TRUE you can save/load next CoolToolBar properties:
	//		
	//		modes: cool, flat, list, customizable
	//		draw properties: gripper, separator
	//		size of the indent
	//		hot and default text color
	//		text type
	//		max text rows
	//		min/max buttons width
	////////////////////////
	BOOL SaveBarState(LPCTSTR lpszSubKey, LPCTSTR lpszValueName, 
		BOOL bProperties=TRUE);
	BOOL LoadBarState(LPCTSTR lpszSubKey, LPCTSTR lpszValueName, 
		BOOL bProperties=TRUE);

	// dock CoolToolBar to the left side of another docked ToolBar
	// if use NULL as parameter then this function work just as common
	// CFrameWnd::DockControlBar()
	void DockControlBarLeftOf(CToolBar *leftOf=NULL);

	inline DWORD GetComCtlVersion() const { return m_dwComCtlVersion; }

#if _MFC_VER<=0x0421
	void SetHeight(int cyHeight);
#endif

	// --- In  :	nIconID		-	resource ID of the icon
	//				lpszIcon	-	resource string of the icon
	//				hIcon		-	handle to the icon. If NULL then 
	//								current toolbar icon will be removed
	//				szIcon		-	size of the icon
	//				hInstance	-	handle of the module that owns the
	//								specified icon resource
	//				bRedraw		-	flag that specifies whether to redraw 
	//								the toolbar after setting new icon or not
	// --- Out : 
	// --- Returns:
	// --- Effect : Sets/removes icon to be displayed in the toolbar
	inline void SetIcon(const UINT nIconID, 
		CSize szIcon=CSize(::GetSystemMetrics(SM_CXSMICON),
		::GetSystemMetrics(SM_CYSMICON)), HINSTANCE hInstance=NULL, 
		const BOOL bRedraw=TRUE) {
		SetIcon(MAKEINTRESOURCE(nIconID),szIcon,hInstance,bRedraw);
	}
	inline void SetIcon(LPCTSTR lpszIcon, 
		CSize szIcon=CSize(::GetSystemMetrics(SM_CXSMICON),
		::GetSystemMetrics(SM_CYSMICON)), HINSTANCE hInstance=NULL, 
		const BOOL bRedraw=TRUE) 
	{
		if(hInstance==NULL)
			hInstance=AfxFindResourceHandle(lpszIcon,RT_ICON);
		if(hInstance!=NULL)
		{
			HICON hIcon=(HICON)::LoadImage(hInstance,lpszIcon,
				IMAGE_ICON,szIcon.cx,szIcon.cy,LR_DEFAULTCOLOR);
			if(hIcon)
				SetIcon(hIcon,bRedraw);
		}
	}
	inline void SetIcon(const HICON hIcon, const BOOL bRedraw=TRUE) {
		if(m_hIcon!=hIcon)
		{
			m_iconRect.SetRectEmpty();
			m_hIcon=hIcon;
			if(bRedraw)
				RedrawToolBar();
		}
	}

	// --- In  :
	// --- Out : 
	// --- Returns:	handle to the toolbar icon or NULL if there is none set
	// --- Effect : Retrieves icon that is displayed in the toolbar
	inline HICON GetIcon() const { return m_hIcon; }


	// --- In  :	bRecalcLayout	-	if TRUE then docking frame will
	//									recalculate its layout
	//				bOnlyFrame		-	if TRUE then only non-client area of 
	//									the control will be redrawn
	// --- Out : 
	// --- Returns:	
	// --- Effect : Redraws the toolbar
	inline void RedrawToolBar(const BOOL bRecalcLayout=TRUE, 
		const BOOL bOnlyFrame=FALSE)
	{
		SetWindowPos(NULL,0,0,0,0,
			SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_DRAWFRAME);
		if(!bOnlyFrame)
			RedrawWindow(NULL,NULL,
				RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME|RDW_ALLCHILDREN);

		if(bRecalcLayout)
		{
			CFrameWnd* pFrameWnd=GetDockingFrame();
			if(pFrameWnd!=NULL)
				pFrameWnd->RecalcLayout();
		}
	}


	// --- In  :	nButtonIndex	-	toolbar button index
	// --- Out : 
	// --- Returns:	tooltip text for the specified toolbar button
	// --- Effect : Retrieves the tooltip text for the specified toolbar button
	inline CString GetButtonTooltip(int nButtonIndex) const
	{ 
		ASSERT(::IsWindow(m_hWnd));

		CString sTooltip(_T(""));
		UINT nID=GetItemID(nButtonIndex); 
		if(nID!=0) // will be zero on a separator
		{
			TCHAR szFullText[256];
			// don't handle the message if no string resource found
			if(AfxLoadString(nID,szFullText))
			{
				AfxExtractSubString(sTooltip,szFullText,1,'\n');
			}
		}
		return sTooltip;
	}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	//}}AFX_VIRTUAL

protected:

#if _MFC_VER>=0x0420
	void _GetButton(int nIndex, TBBUTTON* pButton) const;
	void _SetButton(int nIndex, TBBUTTON* pButton);
	virtual CSize CalcLayout(DWORD nMode, int nLength = -1);
	virtual CSize CalcSize(TBBUTTON* pData, int nCount);
	virtual int WrapToolBar(TBBUTTON* pData, int nCount, int nWidth);
	virtual void SizeToolBar(TBBUTTON* pData, int nCount, 
		int nLength, BOOL bVert=FALSE);

	// virtual functions for drawing gripper and separators
	// if you don't like the way we draw them then you can 
	// use your own algorithm
	virtual void DrawGripper(CDC& dc, CRect& rect);
	virtual void DrawSeparator(CDC& dc, CRect& rc);
	// virtual functions for drawing icon
	virtual void DrawIcon(CDC& dc, CRect& rect);
	// virtual function for designating some space
	virtual void BookSpace(CRect& rectBookedSpace, DWORD dwMode) {
		UNREFERENCED_PARAMETER(rectBookedSpace);
		UNREFERENCED_PARAMETER(dwMode);
	}
	// Draw in Booked space
	virtual void DrawInBookedSpace(CDC& dc, CRect& rectToDrawIn) { 
		UNREFERENCED_PARAMETER(dc);
		UNREFERENCED_PARAMETER(rectToDrawIn);
	};
#endif

	// function called everytime when toolbar changes its state from
	// docking to floating and vise versa
	virtual void OnFloatingDocking(BOOL bFloating) {
		UNREFERENCED_PARAMETER(bFloating);
	}

	// function called everytime when toolbar changes its docking side
	virtual void OnChangeDockSide(DWORD dwDockSide) {
		UNREFERENCED_PARAMETER(dwDockSide);
	}


// Implementation
public:
	virtual ~COXCoolToolBar();

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
#if _MFC_VER>=0x0420
	// we have to overwrite this function from CToolBar class
	// to draw non-client area of CoolToolBar when it is used in CoolToolBar
	void EraseNonClient();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual void CalcInsideRect(CRect& rect, BOOL bHorz);

#endif

	// Generated message map functions
protected:
	// overwrite standard handlers to overcome some problems with MFC
	// standard painting routine
	//{{AFX_MSG(COXCoolToolBar)
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG

	// reflect the messages to let the CoolToolBar provide 
	// customization functionality
	afx_msg void OnTBNBeginAdjust(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTBNQueryInsert(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTBNQueryDelete(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTBNToolBarChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTBNGetButtonInfo(NMHDR* pNMHDR, LRESULT* pResult);

#if _MFC_VER>=0x0420
	// reflect custom draw notification
	afx_msg void OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result );
#endif

	DECLARE_MESSAGE_MAP()

private:
	// helper function to find button in m_pBitmapIds array
	int FindBitmapIndex(UINT nID) const;
	// helper function to define the version of comctl32.dll
	// if succeed it sets major number of version to pdwMajor and
	// minor number to pdwMinor
	HRESULT GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor) const;

public:
	// helper function to define whether WindowsNT >=4.0 is running
	BOOL IsWindowsNTRunning();
	// helper function to define the size dropdown arrow 
	int GetDropDownArrowWidth() const;
};

/////////////////////////////////////////////////////////////////////////////
#endif //_COOLTOOLBAR_H

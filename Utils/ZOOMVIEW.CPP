// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		Scot Wingo
//  Description:	Implementation for SECZoomView : CScrollView
//  Created:		Fri Jun 28 14:01:33 1995
//

// stdafx.h for SEC includes secall.h
#include "stdafx.h"

#ifdef _SECDLL
#undef AFXAPI_DATA
#define AFXAPI_DATA __based(__segname("_DATA"))
#endif //_SECDLL

#ifndef __SECZMVW_H__
#include "seczmvw.h"
#endif



#ifdef _SECDLL
#undef AFXAPP_DATA
#define AFXAPP_DATA AFXAPI_DATA
#endif //_SECDLL

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

BEGIN_MESSAGE_MAP(SECZoomView, CScrollView)
   //{{AFX_MSG_MAP(SECZoomView)
	ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//	
//	SECZoomView
//	
//	Default to no zoom scale, and zooming is offf
//
//@doc SECZoomView
//@mfunc If you derive your class from SECZoomView instead of CScrollView, construction will take place automatically, and
// you do not need to explicitly call the SECZoomView constructor.  SECZoomView is created with the default zoom mode set
// to SEC_ZOOMNORMAL.
//@rdesc void
//@xref<c SECZoomView>
SECZoomView::SECZoomView() : CScrollView()
{
   // Init zoom mode to nothing
    m_zoomMode    = SECZoomView::SEC_ZOOMOFF;
    m_fZoomScale  = 1.0f;
    m_fZoomMin  = -1.0f;
    m_fZoomMax = -1.0f;
    ZoomLevelChanged();
    m_bCenter = TRUE;   
} 

//	
//	~SECZoomView
//	
//	
SECZoomView::~SECZoomView()
{
    
}


// SetZoomLevel
// Makes sure the new scale is within bounds, if not,
// calls "ZoomOutOfRange()" which can be overriden to
// detect the situation.
//
// SetZoomLevel can be overriden for different behavior
//

//@doc SECZoomView
//@mfunc Sets the zoom factor. Can be overridden for different behavior.
//@rdesc void 
//@parm float | fNewScale | New zoom factor.
//@xref<mf SECZoomView::GetZoomLevel>
//@xref<c SECZoomView>
void SECZoomView::SetZoomLevel(float fNewScale)
{
    //Developer has not changed min/max, no need to check.
    if (m_fZoomMax == -1 && m_fZoomMin == -1){
		m_fZoomScale = fNewScale;
		UpdateViewport(NULL);
		return;
    }

    if (fNewScale <=m_fZoomMax && fNewScale >=m_fZoomMin){
		m_fZoomScale = fNewScale;
		UpdateViewport(NULL);
		return;
    }
    
    //Problems here.
    if (fNewScale < m_fZoomMin)
		ZoomOutOfRange(TRUE,fNewScale); //Underflow
    else
		ZoomOutOfRange(FALSE,fNewScale); //Overflow
    
}

//@doc SECZoomView
//@mfunc Specifies the minimum and maximum zoom factors that act as a floor and ceiling for the zoom view.  The floor and 
// ceiling are checked when one of the Zoom calls is made and, if the new zoom value is below the floor or above the 
// ceiling, no zooming is performed.  Optimal minimum and maximum values vary per application, so no defaults are specified.
//@rdesc void 
//@parm float | fMin | The minimum zoom factor allowed.  A good value for Win32 is .02.  In most cases, anything smaller
// is microscopic.
//@parm  float | fMax | The maximum zoom factor allowed. A good value for Win32 is 22.0.
//@xref<mf SECZoomView::GetZoomLevel>
//@xref<mf SECZoomView::ZoomLevelChanged>
//@xref<c SECZoomView>
void SECZoomView::SetZoomMinMax(float fMin, float fMax)
{
    ASSERT_VALID(this);
    ASSERT(fMin < fMax);
    
    //Make sure we're already in a groovy state
    ASSERT(m_fZoomScale <= fMax);
    ASSERT(m_fZoomScale >= fMin);
    
    m_fZoomMin = fMin;
    m_fZoomMax = fMax;
}



//@doc SECZoomView
//@mfunc Specifies the minimum and maximum zoom percents that act as a floor and ceiling for the zoom view.  The floor and
// ceiling are checked when one of the Zoom calls is made and if the new zoom percent is below the floor or above the 
// ceiling, no zooming is performed.  Optimal minimum and maximum values vary per application, so no defaults are specified.
//<nl>If no minimum or maximum is specified, it is possible for the zooming to become so large or small as to overflow the 
// internal zoom values causing ‘zoom wrap’.
//@rdesc void 
//@parm int | nMin | The minimum allowed zoom percent.
//@parm  int | nMax | The maximum allowed zoom percent.
//@xref<mf SECZoomView::GetZoomLevel>
//@xref<mf SECZoomView::ZoomLevelChanged>
//@xref<c SECZoomView>
void SECZoomView::SetZoomMinMaxPercent(int nMin, int nMax)
{
    ASSERT_VALID(this);
    ASSERT(nMin < nMax);
    
    //Let this guy do the work!
    SetZoomMinMax(nMin/100.0f,nMax/100.0f);
    
}

//
// Override this to 
//
//

//@doc SECZoomView
//@mfunc If a zoom factor is set which is out of bounds as defined by SetZoomMinMax, this member is called.  A derived
// class can override this member to detect and react to the situation.
//@rdesc void 
//@parm BOOL | bUnderFlow | TRUE if zoom factor is less than the lower bound.
//@parm float | fAttempt | Zoom factor which was attempted.
//@xref<c SECZoomView>
void SECZoomView::ZoomOutOfRange(BOOL bUnderFlow,float fAttempt)
{
    fAttempt; //UNUSED
    if (bUnderFlow)
	TRACE2("SECZoomView detected zoom underflow level: %f attempt: %f\n",
	       m_fZoomScale,fAttempt);
    else
	TRACE2("SECZoomView detected zoom overflow: level: %f attempt: %f\n",
	       m_fZoomScale,fAttempt);
    
}

//
// SECZoomView printing
//

BOOL SECZoomView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

//
// SECZoomView overridden CScrollView members
//

//	
//	SetScrollSizes
//	
//	Store the original values in member data for later, so
//	we always know what our original setup was.
//
//      Most code borrowed from CScrollView, we're just in ANISOTROPIC mode.
//

void SECZoomView::SetScrollSizes (int nMapMode,
				  SIZE sizeTotal,
				  const SIZE& sizePage /* = sizeDefault */, // in logical units
				  const SIZE& sizeLine /* = sizeDefault */) // in logical units
{
    ASSERT_VALID(this);
    
    nMapMode; //UNUSED
    // Set up the defaults
    if (sizeTotal.cx == 0 && sizeTotal.cy == 0){
	sizeTotal.cx = 1;
	sizeTotal.cy = 1;
    }
    //SetWindowExt doesn't like 0,0 will bomb out.
    ASSERT(sizeTotal.cx > 0 && sizeTotal.cy > 0);
    
    m_nMapMode    = MM_ANISOTROPIC; // Need for arbitrary scaling
    m_totalLog    = sizeTotal;
    
    // Setup default Viewport extent to be conversion of Window extent
    // into device units.
    
    //BLOCK for DC
    {
	CWindowDC dc(NULL);
	dc.SetMapMode(m_nMapMode);
	
	// total size
	m_totalDev = m_totalLog;
	dc.LPtoDP((LPPOINT)&m_totalDev);
    } // Release DC here
    
    m_szOrigTotalDev = m_totalDev;
    m_szOrigPageDev  = sizePage;
    m_szOrigLineDev  = sizeLine;
    ReCalcBars();
    
    ZoomLevelChanged(); //Notify app that there's a new zoom level 1.0f.   
}

//	
//	OnPrepareDC
//	
//	Does all the work for SECZoomView.
//	
//	I copied lots of CScrollView code to here, because I can't call
//	CScrollView::OnPrepareDC, it will hose my dc that I've setup.
//	
void SECZoomView::OnPrepareDC (
    CDC* pDC, 
    CPrintInfo* pInfo)
{
#ifdef _DEBUG
    if (m_nMapMode != MM_ANISOTROPIC) {
	TRACE0("Error: must call SetScrollSizes() before painting view\n");
	ASSERT(FALSE);
	return;
    }
#endif //_DEBUG
    
    ASSERT_VALID(pDC);
    ASSERT(m_totalLog.cx >= 0 && m_totalLog.cy >= 0);
    ASSERT(m_totalDev.cx >= 0 && m_totalDev.cx >= 0);
    
    // Set the Mapping mode, and the window and viewport extents
    // Note: This will always be ANISOTROPIC for SECZoomView
    pDC->SetMapMode(m_nMapMode);
    pDC->SetWindowExt(m_totalLog);  //Set up the logical window

    //Now figure out the origin for the viewport, depends on
    //This code is from CSCrollView
    CPoint ptVpOrg(0, 0);
    
    if (!pDC->IsPrinting()) {
	pDC->SetViewportExt(m_totalDev); // in device coordinates
	
	// by default shift viewport origin in negative direction of scroll
	ASSERT(pDC->GetWindowOrg() == CPoint(0,0));
	
	if (m_zoomMode == SECZoomView::SEC_ZOOMPANOVER){ //Special mode for pan overviewing
	    ptVpOrg.x = 0; //m_totalDev.cx /2;
	    ptVpOrg.y = 0; //m_totalDev.cy /2;
	} else 
	    ptVpOrg = -GetDeviceScrollPosition();
	
	// Center full fit
      if (m_bCenter) {
	  CRect rect;
	  GetClientRect(&rect);
	  
	  // if client area is larger than total device size,
	  // override scroll positions to place origin such that
	  // output is centered in the window
	  if (m_totalDev.cx < rect.Width())
	      ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
	  if (m_totalDev.cy < rect.Height())
	      ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
      }
    } else {
	// Special case for printing
	CSize  printSize;
	printSize.cx = pDC->GetDeviceCaps(HORZRES);
	printSize.cy = pDC->GetDeviceCaps(VERTRES);
	// Maintain the origional ratio, setup origin shift
	PersistRatio(m_totalLog, printSize, ptVpOrg);
	// Zoom completely out
	pDC->SetViewportExt(printSize);
    }
    
    // Set the new viewport origin, call CView for printing behavior
    pDC->SetViewportOrg(ptVpOrg);
    CView::OnPrepareDC(pDC, pInfo);
} 

//	SECZoomView::ReCalcBars
//	
//	
//	Since we're changing the viewport around, we'll need to
//	modify the scrollbars where CScrollView just sets them
//	up at start of day and scrolls.
//	
//      This code originally lived in CScrollView::SetScrollSizes, but
//      I had to move it here to facilitate updating the scroll bars.
//
void SECZoomView::ReCalcBars (void)
{
    ASSERT_VALID(this);
    {  // BLOCK for DC
      CWindowDC dc(NULL);
      dc.SetMapMode(m_nMapMode);

      // Calculate new device units for scrollbar
      // Start with origional logical units from SetScrollPos
      m_pageDev = m_szOrigPageDev;
      dc.LPtoDP((LPPOINT)&m_pageDev);
      m_lineDev = m_szOrigLineDev;
      dc.LPtoDP((LPPOINT)&m_lineDev);
   } // Free DC

   // Make sure of the range
   if (m_pageDev.cy < 0)  m_pageDev.cy = -m_pageDev.cy;
   if (m_lineDev.cy < 0)  m_lineDev.cy = -m_lineDev.cy;

   // If none specified - use one tenth, Just like CScrollView
   
   //ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
   if (m_totalDev.cx < 0 || m_totalDev.cy < 0){
       TRACE0("SECZoomView: Zoom level too low, divide by zero possible\n");
       ASSERT(FALSE);
   }
   
   if (m_pageDev.cx == 0) m_pageDev.cx = m_totalDev.cx / 10;
   if (m_pageDev.cy == 0) m_pageDev.cy = m_totalDev.cy / 10;
   if (m_lineDev.cx == 0) m_lineDev.cx = m_pageDev.cx  / 10;
   if (m_lineDev.cy == 0) m_lineDev.cy = m_pageDev.cy  / 10;

   // Now update the scrollbars
   if (m_hWnd != NULL) {
      UpdateBars();
      Invalidate(TRUE); // Zoom scale changed, redraw all
   }
}


#ifdef _DEBUG

void SECZoomView::Dump(CDumpContext& dc) const
{
    CScrollView::Dump(dc);

    dc << "zoom scale = " << m_fZoomScale << "\n";
    dc << "zoom min = " << m_fZoomMin << "\n";
    dc << "zoom max = " << m_fZoomMax << "\n";
    dc << "zoom mode = " << m_zoomMode << "\n";
    dc << "original size = " << m_szOrigTotalDev << "\n";
    dc << "original page size = " << m_szOrigPageDev << "\n";
    dc << "line size = " << m_szOrigLineDev << "\n";
    dc << "mapping mode = " << m_nMapMode << "\n";
}

void SECZoomView::AssertValid() const
{
    //Verify min/max condition if set
    if (m_fZoomMin != -1 && m_fZoomMax !=-1) {
	   ASSERT (m_fZoomMin <= m_fZoomScale);
	   ASSERT (m_fZoomScale <= m_fZoomMax);
	   }
    
    CView::AssertValid();
} 
#endif //_DEBUG

//
// SECZoomView custom members to implement zooming functionality
//
// rect in logical coordinates

//@doc SECZoomView
//@mfunc Increases the zoom level. The ZoomIn() version that takes a rectangle argument, zooms the view to fit in the rectangle,
// and the version that takes a point uses the point as the center of the zoom-in operation.
//@syntax ZoomIn (CRect &rect) 
//@syntax ZoomIn (CPoint *point, float delta) 
//@rdesc void  
//@parm CRect & | rect | Specifies the rectangle to zoom in on.  
//@parm CPoint * | point | The point which you want to be the central point of the zoom action.  If NULL (the default),
// SECZoomView calculates this for you using the center of the physical window.
//@parm float | delta | Specifies the amount to zoom in.  The default is 25%.
//@xref<mf SECZoomView::ZoomOut>
//@xref<mf SECZoomView::ZoomPercent>
//@xref<c SECZoomView>
void  SECZoomView::ZoomIn (CRect &rect)   
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    if (m_zoomMode == SECZoomView::SEC_ZOOMNORMAL){ //Only if we're in normal zoom mode.
	// Make sure that the rect is normalized
	CRect zoomRect = rect;
	zoomRect.NormalizeRect();
	
	// Get the center of rect
       CPoint ptCenter;
       ptCenter.x = ((zoomRect.left + zoomRect.right)  / 2);
       ptCenter.y = ((zoomRect.top  + zoomRect.bottom) / 2);
       
       CRect clientRect;
       GetClientRect(&clientRect);
       
       // Calculate the new zoom scale based on input rect
       // and the current client rect.
       
       float fHorizScale =
	   (float) (clientRect.right  + 1) / (float) zoomRect.Width();
       float fVertScale  =
	   (float) (clientRect.bottom + 1) / (float) zoomRect.Height();
       
       // Keep the scale Isotropic
	SetZoomLevel(min(fHorizScale, fVertScale));
       
       UpdateViewport(&ptCenter);
    } 
   return;
} 

//	
//	ZoomIn
//	
//	
void SECZoomView::ZoomIn (
    CPoint *point,   // point in logical coordinates
    float  delta)    // scale factor
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    if (m_zoomMode == SECZoomView::SEC_ZOOMNORMAL){
	// Increase the zoom scale.
	SetZoomLevel(m_fZoomScale * delta);
	
	UpdateViewport(point);
    } //Not in zoom mode, simply ignore it.
    
    return;
} 

//	
//	ZoomOut
//	
//@doc SECZoomView
//@mfunc Decreases the zoom level.
//@rdesc void 
//@parm CPoint * | point | The point which you want to be the central point of the zoom action.  If NULL (the default),
// SECZoomview calculates this for you using the center of the physical window.
//@parm float | delta | Specifies the amount to zoom out.  The default is 25%.
//@xref<mf SECZoomView::ZoomIn>
//@xref<mf SECZoomView::ZoomPercent>
//@xref<c SECZoomView>
void SECZoomView::ZoomOut (
    CPoint *point,   // point in logical coordinates
    float  delta)    // scale factor
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    if (m_zoomMode == SECZoomView::SEC_ZOOMNORMAL){
	
	// Decrease the zoom scale.
	SetZoomLevel(m_fZoomScale / delta);
	
	UpdateViewport(point);
    }// Not in zoom mode, ignore.
    
    return;
} 

//	ZoomFit
//	
//	Zoom based on client's size.
//	Make sure client is always filled by using zoom level
//
//@doc SECZoomView
//@mfunc Calling ZoomFit zooms the SECZoomView to fit the current window that contains the view.  This does NOT update the
// zoom mode.  It is automatically called when the zoom mode is set to SEC_ZOOMFIT.
//@rdesc void  
//@xref<mf SECZoomView::SetZoomMode>
//@xref<c SECZoomView>
void  SECZoomView::ZoomFit (void)
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    
    CRect  rc;
    CPoint pt;
    CSize  sizeSb;
    
    if (m_zoomMode == SECZoomView::SEC_ZOOMFIT){
	GetTrueClientSize(m_totalDev, sizeSb);    
	PersistRatio(m_totalLog, m_totalDev, pt);
	
	SetZoomLevel((float) m_totalDev.cx / m_szOrigTotalDev.cx);
	
	UpdateBars(); //Scrollbars should go away, view fills client
	Invalidate(TRUE);
	
	ZoomLevelChanged();
    } //Not in zoom fit mode, ignore.
    return;
} 

//	
//	ZoomOriginal
//	Convenience function for zooming to original proportions.
//	
//@doc SECZoomView
//@mfunc Returns SECZoomView to a zoomlevel of 1.0, or 100%, an unzoomed level.
//@rdesc void  
//@parm CPoint * | point | The point which you want to be the new center of the window once the zoom level is returned to 100%.
//@xref<mf SECZoomView::ZoomIn>
//@xref<mf SECZoomView::ZoomOut>
//@xref<mf SECZoomView::ZoomPercent>
//@xref<c SECZoomView>
void  SECZoomView::ZoomOriginal (
    CPoint *point)   // point in logical coordinates
{
    //Ok for point to be null.
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    // Reset the zoom scale to 1.0
    if (m_zoomMode == SECZoomView::SEC_ZOOMNORMAL){
	m_fZoomScale = 1.0f;
	
	UpdateViewport(point);
    } // We're not in zoom mode, ignore.
    return;
} 

//	ZoomPercent
//	
//	Zoom based on an integer percent..
//	
//	
//@doc SECZoomView
//@mfunc Lets you zoom the view by a percent instead of a floating point zoom level.
//@rdesc void  
//@parm int | nPercent |An integer percent to zoom in or out.  <lt> 100% zooms out, while <gt> 100% zooms in.
//@parm CPoint * | point |The point which should be displayed in the center of the window after the zoom action.  If NULL
// (the default), SECZoomView automatically uses the center of the pre-zoomed window.
//@xref<mf SECZoomView::ZoomIn>
//@xref<mf SECZoomView::ZoomOut>
//@xref<c SECZoomView>
//@end
void  SECZoomView::ZoomPercent (
    int      nPercent /* = 25% */,
    CPoint * point    /* = NULL */)
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    ASSERT(nPercent != 0);
    
    if (m_zoomMode == SECZoomView::SEC_ZOOMNORMAL){
	//Calculate new zoom scale based on %
	m_fZoomScale = nPercent/100.0f;
	
	UpdateViewport(point);
    }
    return;
} 

//	
//	SetZoomMode
//	
//	
//@doc SECZoomView
//@mfunc Sets the zoom mode.
//@rdesc void 
//@parm UINT | zoomMode | Specifies one of either SEC_ZOOMOFF, SEC_ZOOMNORMAL, or SEC_ZOOMFIT
void SECZoomView::SetZoomMode (
    UINT zoomMode)
{
    ASSERT_VALID(this);
    ASSERT(m_nMapMode == MM_ANISOTROPIC);
    if (zoomMode != m_zoomMode)
	m_zoomMode = zoomMode;
    return;
} 

//	
//	CenterOnLogicalPoint
//	
//	Same as CScrollView::CenterOnPoint, but for log point.
//	
//	
void SECZoomView::CenterOnLogicalPoint(CPoint pt)
{
    // Convert the point to device coordinates
    ViewLPtoDP(&pt);
    // Account for scroll bar position
    ClientToDevice(pt);
    // Use CScrollView's function for device coordinates
    CScrollView::CenterOnPoint(pt);
} 

//	
//	GetLogicalCenterPoint
//	
CPoint SECZoomView::GetLogicalCenterPoint (void)  
{
    CPoint pt;
    CRect rect;
    // Get the center of screen
    GetClientRect(&rect);
    pt.x = (rect.Width()  / 2);
    pt.y = (rect.Height() / 2);
    
    // Convert the point to logical coordinates
    ViewDPtoLP(&pt);
    return pt;
} 

//	
//	ViewDPtoLP
//	
//	Same as DPtoLP, but uses Client DC.
void SECZoomView::ViewDPtoLP (
    LPPOINT lpPoints,
    int     nCount)
{
    // Convert to logical units
    // Called from View when no DC is available
    ASSERT(m_nMapMode > 0); // must be set
    CWindowDC dc(this);
    OnPrepareDC(&dc);
    dc.DPtoLP(lpPoints, nCount);
} 

//	
//	Same as LPtoDP, but uses Client DC
//	
void SECZoomView::ViewLPtoDP (
    LPPOINT lpPoints,
    int     nCount)
{
    // Convert to logical units
    // Called from View when no DC is available
    ASSERT(m_nMapMode > 0); // must be set
    CWindowDC dc(this);
    OnPrepareDC(&dc);
    dc.LPtoDP(lpPoints, nCount);
} 

void SECZoomView::ClientToDevice (
    CPoint &point)
{
    // Need to account for scrollbar position
    CPoint scrollPt = GetDeviceScrollPosition();
    point.x += scrollPt.x;
    point.y += scrollPt.y;
} 

//	
//	PersistRatio
//	
//	Since we're not ISOTROPIC, make sure we maintain
//	the same x/y ratios.
//	
void SECZoomView::PersistRatio (
    const CSize &orig,
    CSize       &dest,
    CPoint      &remainder)
{
    float ratio1 = (float) orig.cx / orig.cy;
    float ratio2 = (float) dest.cx / dest.cy;
    int   newSize;
    
    // Do nothing if they are the same
    if (ratio1 > ratio2) {
	// Shrink hieght
	newSize = (int)(dest.cx / ratio1);
	remainder.x = 0;
	remainder.y = dest.cy - newSize;
	dest.cy = newSize;
    } else if (ratio2 > ratio1) {
	// Shrink width
	newSize = (int)(dest.cy * ratio1);
	remainder.x = dest.cx - newSize;
	remainder.y = 0;
	dest.cx = newSize;
    }
} 

//	
//	OnSize handler
//	
//	If we're in ZOOMFIT mode, go ahead and zoomfit.
//	
void SECZoomView::OnSize(UINT nType, int cx, int cy) 
{
    ASSERT_VALID(this);
    
    if (GetZoomMode() == SECZoomView::SEC_ZOOMFIT)
	ZoomFit();

    CScrollView::OnSize(nType, cx, cy);
} 

//	UpdateViewport
//	
//      Called after the scale factor has
//	changed, calculates center if needed,
//	then updates the viewport, updates the
//	scroll bars and finally notifies our decendents
//      that the zoom factor has changed fia ZoomLevelChanged.
//	
void SECZoomView::UpdateViewport(CPoint * ptNewCenter)
{
    //Ok for ptNewCenter = NULL
    ASSERT_VALID(this);
    CPoint ptCenter;
    
    if (!ptNewCenter)
	ptCenter = GetLogicalCenterPoint();
    else 
	ptCenter = *ptNewCenter;
    
    // Modify the Viewport extent
    m_totalDev.cx = (int) ((float) m_szOrigTotalDev.cx * m_fZoomScale);
    m_totalDev.cy = (int) ((float) m_szOrigTotalDev.cy * m_fZoomScale);
    ReCalcBars();
    
    // Set the current center point.
    CenterOnLogicalPoint(ptCenter);
    
    // Notify the class that a new zoom scale was done
    ZoomLevelChanged();
    return;
}

IMPLEMENT_DYNAMIC(SECZoomView, CScrollView)

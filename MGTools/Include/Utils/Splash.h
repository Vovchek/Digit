////////////////////////////////////////////////////////////////
// CSplash implements a splash thread
//
#ifndef _SPLASH_H
#define _SPLASH_H

#include "MGToolsExport.h"
////////////////
// Handy class to lock a critical section.
// ctor/dtor ==> lock/unlock
//
class CLockData {
  private:
    CSyncObject& m_syncObj;
  public:
    CLockData(CSyncObject& so) : m_syncObj(so) { VERIFY(m_syncObj.Lock()); }
    ~CLockData() { m_syncObj.Unlock(); }
  };

//////////////////////////////////////////////////////////////////
// Splash screen. To use it, write:
//
// CSplash *pSplash = new CSplash(
//		IDB_MYBITMAP,			// resource ID of bitmap
//		duration,				// min time to display, in msec
//		flags,					// see below
//		&pSplash);				// address of back pointer
//
// If you want to kill the screen, you can call
//
//		CSplash::Kill(pSplash);
//
// but this is usually unecessary. You don't have to call delete either;
// CSplash will delete itself. When it does, it sets your pointer to NULL so
// you won't try to call Kill on a bad pointer.
//
// NOTE: pSplash MUST be a static/global variable, not a local variable that
// can go out of scope, since CSplash will set it to NULL when the splash dies.
//
class MGTOOLS_API CSplash : public CWinThread 
  {
  public:
    CSplash(UINT nIDRes,				// resource ID of bitmap
            UINT duration,				// how long to show (minimum)
            WORD flags = 0,				// see below
            CSplash** ppBackPtr = NULL);	        // pointer to NULL when destroyed
    ~CSplash();
    
    enum 
      { // flags
      KillOnClick = 0x0001,		// any key/mouse dismisses splash
      IgnoreCmdLine = 0x0002,		// need I say more?
      NoWaitForMainWnd = 0x0004,	// don't wait for main window to expire
      };
    
    // override to create a different kind of splash window
    virtual CWnd* OnCreateSplashWnd(UINT nIDRes, UINT duration, WORD flags);
    static void Kill(CSplash* pSplash);
    
  protected:
    CSplash**	m_ppBackPtr;		// caller's back pointer to me
    UINT	m_nIDRes;		// bitmap resource ID
    UINT	m_duration;		// how long to display
    WORD	m_flags;		// CSplashWnd creation flags
    
    virtual BOOL InitInstance();
    DECLARE_DYNAMIC(CSplash)
  };

//////////////////
// Splash window. This class is private to CSplash--Don't use it unless
// you are doing some hairy stuff to override the splash window, like
// create animated effects, etc.
// 
class CSplashWnd : public CWnd 
  {
  public:
    // these are public so you can change easily
    static DWORD g_dwStyleDefaultEx;
    static DWORD g_dwStyleDefault;
  protected:
    friend CSplash;
    CSplashWnd(CSplash* pThread);
    ~CSplashWnd();
    
    CSplash* m_pSplash;		// ptr to splash thread
    CBitmap  m_bitmap;		// ordinary MFC bitmap
    UINT     m_duration;	// duration (msec)
    WORD     m_flags;		// see below
    
    // override to do weird stuff
    virtual BOOL Create(UINT nIDRes, UINT duration, WORD flags);
    
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void PostNcDestroy();
    
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nIDEvent);
    DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC(CSplashWnd)
  };

#endif

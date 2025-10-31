#if !defined(AFX_TRACKER_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_)
#define AFX_TRACKER_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_

#include "..\stdafx.h"
#include "..\MGTools\Include\Utils\BaseDataType.h"

constexpr int DIGIT_VM_NORMAL = 0; // VM_NORMAL was a redefinition of MFC's #define in Richedit.h 
constexpr int VM_SHIFTROT = 1; // chenged to comply the common pattern

class CMTraker
{
  public:
	  CArray<CPoint, CPoint> Dots;
	  int idxObject;

  public:
	  ~CMTraker();
	  CMTraker();
	  CMTraker(CRect Bound);
      void Init();

      void SetHandleSize(int _size);
      int GetHandleSize();
      void SetBoundPolygon(CDPolygon Plg);
 	  void SetBoundRect(CDPolygon Plg);
 	  void SetBoundRect(CRect Bound);
	  bool GetBoundRect(CRect& Bound);
      BOOL GetEnableState();
      void SetEnableState(BOOL _Enable);
      BOOL GetDragingState(){return Draging;}
      void SetDragingState(BOOL _Draging){Draging = _Draging;}
	  void SetViewMode(int _ViewMode){ViewMode = _ViewMode;}
	  int GetViewMode(){return ViewMode;}

      void SetStaticColor(COLORREF _Color){StaticColor = _Color;}
      void SetDynamicColor(COLORREF _Color){DynamicColor = _Color;}
 	  void DrawTracker(CDC* pDC);
 	  void DrawMovedTracker(CDC* pDC);
	  BOOL SetCursor(CWnd* pW, CDC* pDC, UINT nHitTest);
      int SelectPad(CWnd* pW, CDC* pDC);
      void Track(CWnd* pW, CDC* pDC, CPoint P, bool Init, bool newRect=false);
      BOOL IsPointInside(CDC* pDC, CPoint l_point);
      void SetPolygon(CDPolygon Plg);
	  bool GetPolygon(CDPolygon& Plg);

  protected:
     CPoint GetHandle(int nHandle);

  protected:
	  int iPad;
	  int ViewMode;
	  BOOL Enable;
	  BOOL Draging;
	  CSize ShiftTL;
	  CSize ShiftTR;
	  CSize ShiftBR;
	  CSize ShiftBL;
      int HandleSize;
      COLORREF StaticColor;
      COLORREF DynamicColor;
};

#endif // !defined(AFX_TRACKER_H__7A162971_1875_11D4_8A51_A0D170A2AAD8__INCLUDED_)

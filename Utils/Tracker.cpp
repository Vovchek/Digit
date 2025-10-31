#include <math.h>
#include "Tracker.h"
#include "mutils.h"

CMTraker::~CMTraker()
{
}

CMTraker::CMTraker()
{
	Init();
}

CMTraker::CMTraker(CRect Bound)
{
	Init();
	SetBoundRect(Bound);
}

void CMTraker::Init()
{
	Draging = FALSE;
	Enable = FALSE;
    HandleSize = 5;
	idxObject = -1;
	StaticColor = RGB(0,0,0);
	DynamicColor = RGB(255,255,255);
	Dots.SetSize(4);
	ViewMode = DIGIT_VM_NORMAL;
}

BOOL CMTraker::IsPointInside(CDC* pDC, CPoint l_point)
{
	if(!GetEnableState())
		return FALSE;
	CRect Bound;
	GetBoundRect(Bound);
    int ts = HandleSize;
 	CSize s_ts(ts, ts);
    pDC->DPtoLP(&s_ts);
	ts = s_ts.cx;
	Bound.InflateRect(ts, ts);
	return Bound.PtInRect(l_point);
}

int CMTraker::GetHandleSize()
{
	return HandleSize;
}

void CMTraker::SetHandleSize(int _size)
{
	HandleSize = _size;
}

BOOL CMTraker::GetEnableState()
{
	return Enable;
}

void CMTraker::SetEnableState(BOOL _Enable)
{
	Enable = _Enable;
	if(!Enable){
		CRect R(0,0,0,0);
		SetBoundRect(R);
	}
    ViewMode = DIGIT_VM_NORMAL;
}


void CMTraker::SetPolygon(CDPolygon Plg)
{
	Dots.SetSize(Plg.Dots.GetSize());
	for(int i=0; i < Plg.Dots.GetSize(); i++){
		Dots[i].x = (int)Plg.Dots[i].x;
		Dots[i].y = (int)Plg.Dots[i].y;
	}
}

bool CMTraker::GetPolygon(CDPolygon& Plg)
{
	if(Dots.GetSize()==0)
		return false;
	Plg.Dots.SetSize(Dots.GetSize());
	for(int i=0; i < Dots.GetSize(); i++){
		Plg.Dots[i].x = Dots[i].x;
		Plg.Dots[i].y = Dots[i].y;
	}
	return true;
}

void CMTraker::SetBoundRect(CRect Bound)
{
	Dots.SetSize(4);
	CPoint P;
	Bound.NormalizeRect();
	P.x = Bound.left; P.y = Bound.top; Dots[0] = P;
	P.x = Bound.right; P.y = Bound.top; Dots[1] = P;
	P.x = Bound.right; P.y = Bound.bottom; Dots[2] = P;
	P.x = Bound.left; P.y = Bound.bottom; Dots[3] = P;
}

void CMTraker::SetBoundRect(CDPolygon Plg)
{
	Dots.SetSize(Plg.Dots.GetSize());
	for(int i=0; i < Plg.Dots.GetSize(); i++){
		Dots[i].x = (int)Plg.Dots[i].x;
		Dots[i].y = (int)Plg.Dots[i].y;
	}
}

bool CMTraker::GetBoundRect(CRect& Bound)
{
	if(Dots.GetSize()==0)
		return false;

	CRect R;
	R.left = INT_MAX;
	R.right = INT_MIN;
	R.top = INT_MAX;
	R.bottom = INT_MIN;
	for(int i=0; i < Dots.GetSize(); i++){
		R.left = __min(R.left, Dots[i].x);
		R.right = __max(R.right, Dots[i].x);
		R.top = __min(R.top, Dots[i].y);
		R.bottom = __max(R.bottom, Dots[i].y);
	}
	Bound = R;
	return true;
}

void CMTraker::Track(CWnd* pW, CDC* pDC, CPoint P, bool Init, bool newRect/*false*/)
{
	CDPolygon Bound;
    bool res = GetPolygon(Bound);
	DrawMovedTracker(pDC);
	if(Init){
	   if(newRect)
		 iPad = -1;
	   else
	     iPad = SelectPad(pW, pDC);
	   if(iPad == -1){
		 Dots[0].x = Dots[1].x = Dots[2].x = Dots[3].x = P.x;
		 Dots[0].y = Dots[1].y = Dots[2].y = Dots[3].y = P.y;
         DrawMovedTracker(pDC);
	   }
	   if(iPad == 0){
		   ShiftTL.cx = P.x - Dots[0].x;
		   ShiftTR.cx = Dots[1].x - P.x;
		   ShiftBR.cx = Dots[2].x - P.x;
		   ShiftBL.cx = P.x - Dots[3].x;
		   
		   ShiftTL.cy = P.y - Dots[0].y;
		   ShiftTR.cy = P.y - Dots[1].y;
		   ShiftBR.cy = Dots[2].y - P.y;
		   ShiftBL.cy = Dots[3].y - P.y;
           DrawMovedTracker(pDC);
	   }
	}
	else{
	   CPoint BR(P);
	   if(iPad == 0){
		Dots[0].x = P.x - ShiftTL.cx;
		Dots[1].x = P.x + ShiftTR.cx;
		Dots[2].x = P.x + ShiftBR.cx;
		Dots[3].x = P.x - ShiftBL.cx;

		Dots[0].y = P.y - ShiftTL.cy;
		Dots[1].y = P.y - ShiftTR.cy;
		Dots[2].y = P.y + ShiftBR.cy;
		Dots[3].y = P.y + ShiftBL.cy;
	   }
	   if(iPad == -1){
//		   if(P.x >= (int)Bound.GetRight()){
    		 Dots[1].x = Dots[2].x = BR.x;
	    	 Dots[2].y = Dots[3].y = BR.y;
//		   }
//		   else{
//    		 Dots[0].x = Dots[3].x = BR.x;
//	    	 Dots[0].y = Dots[1].y = BR.y;
//		   }
	   }
	   if(iPad == 1 && ViewMode == DIGIT_VM_NORMAL){
		Dots[0].x = Dots[3].x = BR.x;
		Dots[0].y = Dots[1].y = BR.y;
	   }
	   else if(iPad == 1 && ViewMode == VM_SHIFTROT){
		   ;
	   }
	   if(iPad == 2 && ViewMode == DIGIT_VM_NORMAL){
         int H12 = Dots[1].y-Dots[0].y;
		 if(H12>0){
			 Dots[0].y = BR.y; Dots[1].y = BR.y+H12;
		 }
		 else{
			 Dots[1].y = BR.y; Dots[0].y = BR.y-H12;
		 }
	   }
	   else if(iPad == 2 && ViewMode == VM_SHIFTROT){
         int W12 = (Dots[1].x-Dots[0].x)/2;
		 Dots[0].x = BR.x - W12;
		 Dots[1].x = BR.x + W12;
	   }
	   if(iPad == 3 && ViewMode == DIGIT_VM_NORMAL){
		 Dots[1].x = Dots[2].x = BR.x;
		 Dots[0].y = Dots[1].y = BR.y;
	   }
	   else if(iPad == 3 && ViewMode == VM_SHIFTROT){
		   ;
	   }
	   if(iPad == 4 && ViewMode == DIGIT_VM_NORMAL){
         int W12 = Dots[1].x-Dots[2].x;
		 if(W12>0){
			 Dots[1].x = BR.x; Dots[2].x = BR.x-W12;
		 }
		 else{
			 Dots[2].x = BR.x; Dots[1].x = BR.x+W12;
		 }
	   }
	   else if(iPad == 4 && ViewMode == VM_SHIFTROT){
         int H12 = (Dots[2].y-Dots[1].y)/2;
		 Dots[1].y = BR.y - H12;
		 Dots[2].y = BR.y + H12;
	   }
	   if(iPad == 5 && ViewMode == DIGIT_VM_NORMAL){
   		 Dots[1].x = Dots[2].x = BR.x;
    	 Dots[2].y = Dots[3].y = BR.y;
	   }
	   else if(iPad == 5 && ViewMode == VM_SHIFTROT){
		   ;
	   }
	   if(iPad == 6 && ViewMode == DIGIT_VM_NORMAL){
         int H12 = Dots[2].y-Dots[3].y;
		 if(H12>0){
			 Dots[2].y = BR.y; Dots[3].y = BR.y-H12;
		 }
		 else{
			 Dots[3].y = BR.y; Dots[2].y = BR.y+H12;
		 }
	   }
	   else if(iPad == 6 && ViewMode == VM_SHIFTROT){
         int W12 = (Dots[2].x-Dots[3].x)/2;
		 Dots[2].x = BR.x + W12;
		 Dots[3].x = BR.x - W12;
	   }
	   if(iPad == 7 && ViewMode == DIGIT_VM_NORMAL){
		 Dots[0].x = Dots[3].x = BR.x;
		 Dots[2].y = Dots[3].y = BR.y;
	   }
	   else if(iPad == 7 && ViewMode == VM_SHIFTROT){
		   ;
	   }
	   if(iPad == 8 && ViewMode == DIGIT_VM_NORMAL){
         int W12 = Dots[0].x-Dots[3].x;
		 if(W12>0){
			 Dots[3].x = BR.x; Dots[0].x = BR.x-W12;
		 }
		 else{
			 Dots[0].x = BR.x; Dots[3].x = BR.x-W12;
		 }
	   }
	   else if(iPad == 8 && ViewMode == VM_SHIFTROT){
         int H12 = (Dots[3].y-Dots[0].y)/2;
		 Dots[0].y = BR.y - H12;
		 Dots[3].y = BR.y + H12;
	   }
       DrawMovedTracker(pDC);
	}
}

int CMTraker::SelectPad(CWnd* pW, CDC* pDC)
{
	  CPoint pt;
	  GetCursorPos(&pt);
	  pW->ScreenToClient(&pt);
	  pDC->DPtoLP(&pt);
		CRect wRect, R;
		GetBoundRect(R);
		if(R.IsRectNull())
			return -1;
        int ts = HandleSize;
		CSize s_ts(ts, ts);
 	    pDC->DPtoLP(&s_ts);
		ts = s_ts.cx;
		int Width = R.Width()-2*ts;
		int Height = R.Height()-2*ts;
        int nHandleCount = 8;
        for (int nHandle = 1; nHandle <= nHandleCount; nHandle++){
            CPoint handle = GetHandle(nHandle);
            if(nHandle==1){
              wRect = CRect(CPoint(handle.x - ts, handle.y - ts), CSize(ts, ts));
			  if(wRect.PtInRect(pt)){
				return 1;
			  }
			}
            else if(nHandle==2){
			  wRect = CRect(CPoint(handle.x - Width/2, handle.y - ts), CSize(Width, ts));
			  if(wRect.PtInRect(pt)){
				return 2;
			  }
            }
            else if(nHandle==3){
			  wRect = CRect(CPoint(handle.x, handle.y - ts), CSize(ts, ts));
			  if(wRect.PtInRect(pt)){
				return 3;
			  }
            }
            else if(nHandle==4){
			  wRect = CRect(CPoint(handle.x, handle.y - Height/2), CSize(ts, Height));
			  if(wRect.PtInRect(pt)){
				return 4;
			  }
            }
            else if(nHandle==5){
			  wRect = CRect(CPoint(handle.x, handle.y), CSize(ts, ts));
			  if(wRect.PtInRect(pt)){
				return 5;
			  }
            }
            else if(nHandle==6){
			  wRect = CRect(CPoint(handle.x - Width/2, handle.y), CSize(Width, ts));
			  if(wRect.PtInRect(pt)){
				return 6;
			  }
            }
            else if(nHandle==7){
			  wRect = CRect(CPoint(handle.x - ts, handle.y), CSize(ts, ts));
			  if(wRect.PtInRect(pt)){
				return 7;
			  }
            }
            else if(nHandle==8){
			  wRect = CRect(CPoint(handle.x - ts, handle.y-Height/2), CSize(ts, Height));
			  if(wRect.PtInRect(pt)){
				return 8;
			  }
            }
        }
		CRect Bound;
	    GetBoundRect(Bound);
		if(Bound.PtInRect(pt)){
           ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
 		   return 0;
		}
		else{
 		   return -1;
		}
}

BOOL CMTraker::SetCursor(CWnd* pW, CDC* pDC, UINT nHitTest)
{
	if(nHitTest == HTCLIENT){
		int iPad = SelectPad(pW, pDC);
		if(iPad == -1)
			return FALSE;
		switch(iPad){
		  case 0:
               ::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		       break;
		  case 1:
                ::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
		       break;
		  case 2:
                if(ViewMode == DIGIT_VM_NORMAL)::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
                if(ViewMode == VM_SHIFTROT)::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		       break;
		  case 3:
                ::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
		       break;
		  case 4:
                if(ViewMode == DIGIT_VM_NORMAL)::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
                if(ViewMode == VM_SHIFTROT)::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		       break;
		  case 5:
                ::SetCursor(::LoadCursor(NULL, IDC_SIZENWSE));
		       break;
		  case 6:
                if(ViewMode == DIGIT_VM_NORMAL)::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
                if(ViewMode == VM_SHIFTROT)::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		       break;
		  case 7:
                ::SetCursor(::LoadCursor(NULL, IDC_SIZENESW));
		       break;
		  case 8:
                if(ViewMode == DIGIT_VM_NORMAL)::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				if(ViewMode == VM_SHIFTROT)::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		       break;
		}
      return TRUE;
	}
   return FALSE;
}

// returns logical coords of center of handle
CPoint CMTraker::GetHandle(int nHandle)
{
    int x, y, xCenter, yCenter;

    // this gets the center regardless of left/right and top/bottom ordering
    CRect m_rect;
	bool res = GetBoundRect(m_rect);
	if(!res)
      return CPoint(0, 0);

    xCenter = m_rect.left + m_rect.Width() / 2;
    yCenter = m_rect.top + m_rect.Height() / 2;

    switch (nHandle)
    {
    default:
        ASSERT(FALSE);

    case 1:
        x = m_rect.left;
        y = m_rect.top;
        break;

    case 2:
        x = xCenter;
        y = m_rect.top;
        break;

    case 3:
        x = m_rect.right;
        y = m_rect.top;
        break;

    case 4:
        x = m_rect.right;
        y = yCenter;
        break;

    case 5:
        x = m_rect.right;
        y = m_rect.bottom;
        break;

    case 6:
        x = xCenter;
        y = m_rect.bottom;
        break;

    case 7:
        x = m_rect.left;
        y = m_rect.bottom;
        break;

    case 8:
        x = m_rect.left;
        y = yCenter;
        break;
    }

    return CPoint(x, y);
}

void CMTraker::DrawTracker(CDC* pDC)
{
	CRect Bound;
	bool res = GetBoundRect(Bound);
	Bound.NormalizeRect();
	if(!res || Bound.IsRectNull())
	  return;
	int oldRop;
    oldRop = pDC->SetROP2(R2_XORPEN);
 	   CPen pen;
	   pen.CreatePen(PS_SOLID, 0, DynamicColor);
	   CPen* oPen = pDC->SelectObject(&pen);
	   for(int i=0; i < Dots.GetSize()+1; i++){
		   if(i==0)
  	         pDC->MoveTo(Dots[i]);
		   if(i == Dots.GetSize())
  	         pDC->LineTo(Dots[0]);
		   else
  	         pDC->LineTo(Dots[i]);
	   }
    pDC->SetROP2(oldRop);
	if(Bound.Width()>2 && Bound.Height()>2){
        int ts = HandleSize;
		CSize s_ts(ts, ts);
 	    pDC->DPtoLP(&s_ts);
		ts = s_ts.cx;
        int nHandleCount = 8;
        for (int nHandle = 1; nHandle <= nHandleCount; nHandle++){
            CPoint handle = GetHandle(nHandle);
			if(ViewMode == DIGIT_VM_NORMAL){
				if(nHandle==1) pDC->PatBlt(handle.x - ts, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==2) pDC->PatBlt(handle.x - ts/2, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==3) pDC->PatBlt(handle.x, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==4) pDC->PatBlt(handle.x, handle.y - ts/2, ts, ts, DSTINVERT);
				else if(nHandle==5) pDC->PatBlt(handle.x, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==6) pDC->PatBlt(handle.x - ts/2, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==7) pDC->PatBlt(handle.x - ts, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==8) pDC->PatBlt(handle.x - ts, handle.y-ts/2, ts, ts, DSTINVERT);
			}
			else{
//				if(nHandle==1) pDC->PatBlt(handle.x - ts, handle.y - ts, ts, ts, DSTINVERT);
				if(nHandle==2) pDC->PatBlt(handle.x - 2*ts, handle.y - ts, 4*ts, ts/2, DSTINVERT);
//				else if(nHandle==3) pDC->PatBlt(handle.x, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==4) pDC->PatBlt(handle.x+ts, handle.y - 2*ts, ts/2, 4*ts, DSTINVERT);
//				else if(nHandle==5) pDC->PatBlt(handle.x, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==6) pDC->PatBlt(handle.x - 2*ts, handle.y+ts, 4*ts, ts/2, DSTINVERT);
//				else if(nHandle==7) pDC->PatBlt(handle.x - ts, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==8) pDC->PatBlt(handle.x - ts, handle.y-ts, ts/2, 4*ts, DSTINVERT);
			}
        }
	}
		if(oPen){
		   CPen* retPen = pDC->SelectObject(oPen);
		   if(retPen) retPen->DeleteObject();
	}
}

void CMTraker::DrawMovedTracker(CDC* pDC)
{
	CRect Bound;
	bool res = GetBoundRect(Bound);
	Bound.NormalizeRect();
	if(!res || Bound.IsRectNull())
	  return;
	int oldRop;
    oldRop = pDC->SetROP2(R2_XORPEN);
 	   CPen pen;
	   pen.CreatePen(PS_SOLID, 0, DynamicColor);
	   CPen* oPen = pDC->SelectObject(&pen);
	   for(int i=0; i < Dots.GetSize()+1; i++){
		   if(i==0)
  	         pDC->MoveTo(Dots[i]);
		   if(i == Dots.GetSize())
  	         pDC->LineTo(Dots[0]);
		   else
  	         pDC->LineTo(Dots[i]);
	   }
    pDC->SetROP2(oldRop);
	if(Bound.Width()>2 && Bound.Height()>2){
        int ts = HandleSize;
		CSize s_ts(ts, ts);
 	    pDC->DPtoLP(&s_ts);
		ts = s_ts.cx;
        int nHandleCount = 8;
        for (int nHandle = 1; nHandle <= nHandleCount; nHandle++){
            CPoint handle = GetHandle(nHandle);
			if(ViewMode == DIGIT_VM_NORMAL){
				if(nHandle==1) pDC->PatBlt(handle.x - ts, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==2) pDC->PatBlt(handle.x - ts/2, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==3) pDC->PatBlt(handle.x, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==4) pDC->PatBlt(handle.x, handle.y - ts/2, ts, ts, DSTINVERT);
				else if(nHandle==5) pDC->PatBlt(handle.x, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==6) pDC->PatBlt(handle.x - ts/2, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==7) pDC->PatBlt(handle.x - ts, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==8) pDC->PatBlt(handle.x - ts, handle.y-ts/2, ts, ts, DSTINVERT);
			}
			else{
//				if(nHandle==1) pDC->PatBlt(handle.x - ts, handle.y - ts, ts, ts, DSTINVERT);
				if(nHandle==2) pDC->PatBlt(handle.x - 2*ts, handle.y - ts, 4*ts, ts/2, DSTINVERT);
//				else if(nHandle==3) pDC->PatBlt(handle.x, handle.y - ts, ts, ts, DSTINVERT);
				else if(nHandle==4) pDC->PatBlt(handle.x+ts, handle.y - 2*ts, ts/2, 4*ts, DSTINVERT);
//				else if(nHandle==5) pDC->PatBlt(handle.x, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==6) pDC->PatBlt(handle.x - 2*ts, handle.y+ts, 4*ts, ts/2, DSTINVERT);
//				else if(nHandle==7) pDC->PatBlt(handle.x - ts, handle.y, ts, ts, DSTINVERT);
				else if(nHandle==8) pDC->PatBlt(handle.x - ts, handle.y-ts, ts/2, 4*ts, DSTINVERT);
			}
		}
	}
    if(oPen){
	   CPen* retPen = pDC->SelectObject(oPen);
	   if(retPen) retPen->DeleteObject();
	}
}

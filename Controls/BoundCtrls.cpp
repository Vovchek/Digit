#include "BoundCtrls.h"
#include "Utils\mutils.h"

CBoundCtrls::CBoundCtrls()
{
	Init();
}

CBoundCtrls::~CBoundCtrls()
{
}

void CBoundCtrls::Init()
{
    CustomDots.RemoveAll();
	CustomDot = CPoint(-1, -1);
	CurBound = ExtBoundRect = InsBoundRect = CRect(0,0,0,0);
    ArrEll.RemoveAll();
    ArrRect.RemoveAll();
    ArrPlg.RemoveAll();
	ArrContour.RemoveAll();
	NPntNax = N_CONT;
    ExtBoundType = BOUND_NONE;
    InsBoundType = BOUND_NONE;
	LastAddedBoundType.RemoveAll();
}

CBoundCtrls& CBoundCtrls::operator=(const CBoundCtrls& rhs)
{
	int i = 0;
  if(this==&rhs) return *this;
    CustomDot = rhs.CustomDot;
	CurBound = rhs.CurBound;
	ExtBoundRect = rhs.ExtBoundRect;
	InsBoundRect = rhs.InsBoundRect;
    CurPlg.RemoveAll();
    for(i=0; i < rhs.CurPlg.GetSize(); i++){
       CurPlg.Add(rhs.CurPlg[i]);
    }
    CustomDots.RemoveAll();
    for(i=0; i < rhs.CustomDots.GetSize(); i++){
       CustomDots.Add(rhs.CustomDots[i]);
    }
    ArrEll.RemoveAll();
    for(i=0; i < rhs.ArrEll.GetSize(); i++){
       ArrEll.Add(rhs.ArrEll[i]);
    }
    ArrRect.RemoveAll();
    for(i=0; i < rhs.ArrRect.GetSize(); i++){
       ArrRect.Add(rhs.ArrRect[i]);
    }
    ArrPlg.RemoveAll();
    for(i=0; i < rhs.ArrPlg.GetSize(); i++){
       ArrPlg.Add(rhs.ArrPlg[i]);
    }
    ArrContour.RemoveAll();
    for(i=0; i < rhs.ArrContour.GetSize(); i++){
       ArrContour.Add(rhs.ArrContour[i]);
    }
    LastAddedBoundType.RemoveAll();
    for(i=0; i < rhs.LastAddedBoundType.GetSize(); i++){
       LastAddedBoundType.Add(rhs.LastAddedBoundType[i]);
    }
	NPntNax = rhs.NPntNax;
    ExtBoundType = rhs.ExtBoundType;
    InsBoundType = rhs.InsBoundType;
    return *this;
}

void CBoundCtrls::AddCustomDot(CPoint P)
{
	  RemoveCustomDot(CustomDot);
	  CustomDots.Add(CustomDot);
}

bool CBoundCtrls::RemoveCustomDot(CPoint P)
{
    CControls* pCtrls = GetControls();
	int step = (int)((pCtrls->MarkerSide - 1)/2);
	CRect rP(CustomDot.x-step, CustomDot.y-step, CustomDot.x+step, CustomDot.y+step);
	rP.NormalizeRect();
	for(int i=0; i < CustomDots.GetSize(); i++){
		if(rP.PtInRect(CustomDots[i])){
			CustomDot = CustomDots[i];
			CustomDots.RemoveAt(i);
		    return true;
		}
	}
    return false;
}

bool CBoundCtrls::CustomDotInFocus(CPoint P)
{
    CControls* pCtrls = GetControls();
	int step = (int)((pCtrls->MarkerSide - 1)/2);
	CRect rP(CustomDot.x-step, CustomDot.y-step, CustomDot.x+step, CustomDot.y+step);
	rP.NormalizeRect();
	for(int i=0; i < CustomDots.GetSize(); i++){
		if(rP.PtInRect(CustomDots[i])){
			CustomDot = CustomDots[i];
		    return true;
		}
	}
    return false;
}

BOOL CBoundCtrls::IsExtBound()
{
/*   if(ArrContExt.GetSize())
	   return TRUE;
   else*/
	   return FALSE;
}

BOOL CBoundCtrls::IsInsBound()
{
/*   if(ArrContIns.GetSize())
	   return TRUE;
   else*/
	   return FALSE;
}

BOOL CBoundCtrls::GetExtCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor)
{
	if(!ExtBoundRect.IsRectNull()){
		Bound = ExtBoundRect;
		if(XCor){
			if(Bound.left < 0) Bound.left = 0;
			if(Bound.right > xDIB) Bound.right = xDIB;
		}
		if(YCor){
			if(Bound.top < 0) Bound.top = 0;
			if(Bound.bottom > yDIB) Bound.bottom = yDIB;
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CBoundCtrls::GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints)
{
   PlgPoints.RemoveAll();

	if(ArrContour.GetSize()==0)
		return FALSE;
	for(int i=0; i < ArrContour.GetSize(); i++){
		int TypeLimits = ArrContour[i].GetTypeLimits();
		if(TypeLimits == EXTERNAL){
			if(Type == BOUND_ROUND || Type == BOUND_ELLIPSE || Type == BOUND_RECT){
				auto Bnd = ArrContour[i].GetBounds();
				Bound.left = std::lround(Bnd.XLeft);
				Bound.right = std::lround(Bnd.XRight);
				Bound.top = std::lround(Bnd.YTop);
				Bound.bottom = std::lround(Bnd.YBottom);
		        if(Type == BOUND_RECT){
			      if(Bound.left < 0) Bound.left = 0;
			      if(Bound.right > xDIB) Bound.right = xDIB;
			      if(Bound.top < 0) Bound.top = 0;
			      if(Bound.bottom > yDIB) Bound.bottom = yDIB;
				}
				ExtBoundRect = Bound;
		        return TRUE;
			}
			else{
				CArrayDouble ArrX;
				CArrayDouble ArrY;
                ArrContour[i].GetArrX(ArrX);
                ArrContour[i].GetArrY(ArrY);
				int nS = ArrX.GetSize();
				PlgPoints.SetSize(nS);
				for(int ii=0; ii < nS; ii++){
					PlgPoints[ii].x = int(ArrX[ii]+0.5);
					PlgPoints[ii].y = int(ArrY[ii]+0.5);
				}
				auto Bnd = ArrContour[i].GetBounds();
				Bound.left = std::lround(Bnd.XLeft);
				Bound.right = std::lround(Bnd.XRight);
				Bound.top = std::lround(Bnd.YTop);
				Bound.bottom = std::lround(Bnd.YBottom);
				ExtBoundRect = Bound;
		     	return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CBoundCtrls::GetInsCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor)
{
	if(!InsBoundRect.IsRectNull()){
		Bound = InsBoundRect;
		if(XCor){
			if(Bound.left < 0) Bound.left = 0;
			if(Bound.right > xDIB) Bound.right = xDIB;
		}
		if(YCor){
			if(Bound.top < 0) Bound.top = 0;
			if(Bound.bottom > yDIB) Bound.bottom = yDIB;
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CBoundCtrls::GetInsRealBound(int Type, int xDIB, int yDIB,int& idx, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints)
{
   PlgPoints.RemoveAll();
	for(int i=idx; i < ArrContour.GetSize(); i++){
		int TypeLimits = ArrContour[i].GetTypeLimits();
		if(TypeLimits == INTERNAL){
			if(Type == BOUND_ROUND || Type == BOUND_ELLIPSE || Type == BOUND_RECT){
				auto Bnd = ArrContour[i].GetBounds();
				Bound.left = std::lround(Bnd.XLeft);
				Bound.right = std::lround(Bnd.XRight);
				Bound.top = std::lround(Bnd.YTop);
				Bound.bottom = std::lround(Bnd.YBottom);
				idx = i;
		        if(Type == BOUND_RECT){
			      if(Bound.left < 0) Bound.left = 0;
			      if(Bound.right > xDIB) Bound.right = xDIB;
			      if(Bound.top < 0) Bound.top = 0;
			      if(Bound.bottom > yDIB) Bound.bottom = yDIB;
				}
				InsBoundRect = Bound;
		     	return TRUE;
			}
			else{
				CArrayDouble ArrX;
				CArrayDouble ArrY;
                ArrContour[i].GetArrX(ArrX);
                ArrContour[i].GetArrY(ArrY);
				int nS = ArrX.GetSize();
				PlgPoints.SetSize(nS);
				for(int ii=0; ii < nS; ii++){
					PlgPoints[ii].x = int(ArrX[ii]+0.5);
					PlgPoints[ii].y = int(ArrY[ii]+0.5);
				}
				idx = i;
				auto Bnd = ArrContour[i].GetBounds();
				Bound.left = std::lround(Bnd.XLeft);
				Bound.right = std::lround(Bnd.XRight);
				Bound.top = std::lround(Bnd.YTop);
				Bound.bottom = std::lround(Bnd.YBottom);
				InsBoundRect = Bound;
		     	return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CBoundCtrls::IsCurArea()
{
   CControls* pCtrls = GetControls();
   if(pCtrls->CurTypeBound == BOUND_NONE || (CurBound == CRect(0,0,0,0) && CurPlg.GetSize()==0))
       return FALSE;
   else
       return TRUE;
}

bool CBoundCtrls::AddBound(int _Type, int idxExtIns)
{
   bool res;	
   res = SetBound(_Type, idxExtIns);
   CurBound.SetRectEmpty();
   CurPlg.RemoveAll();
   return res;
}

bool CBoundCtrls::SetCurBound(int _Type)
{
   int nP = CustomDots.GetSize();
   if(nP < 4)
	   return false;
   CControls* pCtrls = GetControls();
   CImageCtrls* pImCtrls = GetImageCtrls();
   int xDIB = pImCtrls->m_pDIB->m_dwPadWidth;
   int yDIB = pImCtrls->m_pDIB->m_dwHeight;

   bool res;
   CRect Bound;
   XYBrokenLine BLine;
   XYEllipse Ell;
   XYPolygon Plg;
   CDPoint dP;
   XYBounds Bnd;
   XYRect Rect;
   BLine.SetSize(nP);
   int TypeLimits;
   TypeLimits = EXTERNAL;
   XYBounds InputBnd;
   InputBnd.XLeft = INT_MAX;
   InputBnd.XRight = INT_MIN;
   InputBnd.YTop = INT_MAX;
   InputBnd.YBottom = INT_MIN;
   
   for(int i=0; i < nP; i++){
       BLine[i].X = CustomDots[i].x;
       BLine[i].Y = CustomDots[i].y;
	   if(pCtrls->EnableTracker){
		  InputBnd.XLeft = __min(InputBnd.XLeft, BLine[i].X);
		  InputBnd.XRight = __max(InputBnd.XRight, BLine[i].X);
		  InputBnd.YTop = __min(InputBnd.YTop, BLine[i].Y);
		  InputBnd.YBottom = __max(InputBnd.YBottom, BLine[i].Y);
	   }
   }
   if(_Type == BOUND_ROUND){
     if(pCtrls->EnableTracker)
        res = CalcCircleLimits(InputBnd, TypeLimits, Ell, Bnd);
	 else
        res = CalcCircleLimits(BLine, TypeLimits, Ell, Bnd);
   }
   else if(_Type == BOUND_ELLIPSE){
     res = CalcEllipseLimits(BLine, TypeLimits, Ell, Bnd);
   }
   else if(_Type == BOUND_RECT){
     res = CalcRectangleLimits(BLine, TypeLimits, Rect, Bnd);
   }
   else if(_Type == BOUND_POLYGON){
     res = CalcPolygonLimits(BLine, TypeLimits, Plg);
   }
   Bound.left = static_cast<LONG>(Bnd.XLeft);
   Bound.right = static_cast<LONG>(Bnd.XRight);
   Bound.top = static_cast<LONG>(Bnd.YTop);
   Bound.bottom = static_cast<LONG>(Bnd.YBottom);
   CurBound = Bound;

   pCtrls->CurTypeBound = _Type;
   if(_Type == BOUND_POLYGON){
   	   CArrayDouble ArrX;
       CArrayDouble ArrY;
       Plg.GetArrX(ArrX);
       Plg.GetArrY(ArrY);
       int nnP = ArrX.GetSize();
       CurPlg.SetSize(nnP);
	   for(int i=0; i < nnP; i++){
		   CurPlg[i].x = (int)ArrX[i];
		   CurPlg[i].y = (int)ArrY[i];
	   }
   }
   return res;
}

bool CBoundCtrls::SetBound(int _Type, int idxExtIns)
{
   int nP = CustomDots.GetSize();
   if(nP < 4)
	   return false;

   CControls* pCtrls = GetControls();
   bool res;
   XYBrokenLine BLine;
   XYEllipse Ell;
   XYPolygon Plg;
   CDPoint dP;
   XYBounds Bnd;
   XYRect Rect;
   BLine.SetSize(nP);
   int TypeLimits;
   TypeLimits = idxExtIns;
   XYBounds InputBnd;
   InputBnd.XLeft = INT_MAX;
   InputBnd.XRight = INT_MIN;
   InputBnd.YTop = INT_MAX;
   InputBnd.YBottom = INT_MIN;
   
   for(int i=0; i < nP; i++){
       BLine[i].X = CustomDots[i].x;
       BLine[i].Y = CustomDots[i].y;
	   if(pCtrls->EnableTracker){
		  InputBnd.XLeft = __min(InputBnd.XLeft, BLine[i].X);
		  InputBnd.XRight = __max(InputBnd.XRight, BLine[i].X);
		  InputBnd.YTop = __min(InputBnd.YTop, BLine[i].Y);
		  InputBnd.YBottom = __max(InputBnd.YBottom, BLine[i].Y);
	   }
   }
   if(_Type == BOUND_ROUND){
     if(pCtrls->EnableTracker)
        res = CalcCircleLimits(InputBnd, TypeLimits, Ell, Bnd);
	 else
        res = CalcCircleLimits(BLine, TypeLimits, Ell, Bnd);
	 if(res){
	   ArrEll.Add(Ell);
	   LastAddedBoundType.Add(BOUND_ROUND);
	 }
   }
   else if(_Type == BOUND_ELLIPSE){
     res = CalcEllipseLimits(BLine, TypeLimits, Ell, Bnd);
	 if(res){
	  ArrEll.Add(Ell);
      LastAddedBoundType.Add(BOUND_ELLIPSE);
	 }
   }
   else if(_Type == BOUND_RECT){
     res = CalcRectangleLimits(BLine, TypeLimits, Rect, Bnd);
	 if(res){
		ArrRect.Add(Rect); 
	    LastAddedBoundType.Add(BOUND_RECT);
	 } 
   }
   else if(_Type == BOUND_POLYGON){
     res = CalcPolygonLimits(BLine, TypeLimits, Plg);
	 if(res){
		 ArrPlg.Add(Plg);
	     LastAddedBoundType.Add(BOUND_POLYGON);
	 }
   }
   if(res){
     if(idxExtIns == EXTERNAL){
	  if(_Type == BOUND_ROUND && ExtBoundType == -1) ExtBoundType = BOUND_ROUND;
	  else if(_Type == BOUND_ELLIPSE && ExtBoundType == -1) ExtBoundType = BOUND_ELLIPSE;
	  else if(_Type == BOUND_RECT && ExtBoundType == -1) ExtBoundType = BOUND_RECT;
      else	ExtBoundType = BOUND_POLYGON;
	 }
	 else if(idxExtIns == INTERNAL){
	  if(_Type == BOUND_ROUND && InsBoundType == -1) InsBoundType = BOUND_ROUND;
	  else if(_Type == BOUND_ELLIPSE && InsBoundType == -1) InsBoundType = BOUND_ELLIPSE;
	  else if(_Type == BOUND_RECT && InsBoundType == -1) InsBoundType = BOUND_RECT;
      else	InsBoundType = BOUND_POLYGON;
	 }

	 CalcContour(ArrEll, ArrRect, ArrPlg, ArrContour, NPntNax);
	 if(TypeLimits == INTERNAL && ArrContour.GetSize()==1) // граница - серп
		ExtBoundType = BOUND_POLYGON; 

	 
   }
   return res;
}

void CBoundCtrls::RemoveAllBound()
{
	RemoveExtBound();
	RemoveInsBound();
}

void CBoundCtrls::RemoveLastBound()
{
   if(LastAddedBoundType.GetSize()==0)
	   return;
   int iB = LastAddedBoundType.GetSize()-1;
   if(ArrEll.GetSize() && LastAddedBoundType[iB] == BOUND_ROUND)
     ArrEll.RemoveAt(ArrEll.GetSize()-1);
   else if(ArrEll.GetSize() && LastAddedBoundType[iB] == BOUND_ELLIPSE)
     ArrEll.RemoveAt(ArrEll.GetSize()-1);
   else if(ArrRect.GetSize() && LastAddedBoundType[iB] == BOUND_RECT)
     ArrRect.RemoveAt(ArrRect.GetSize()-1);
   else if(ArrPlg.GetSize() && LastAddedBoundType[iB] == BOUND_POLYGON)
     ArrPlg.RemoveAt(ArrPlg.GetSize()-1);

   LastAddedBoundType.RemoveAt(iB);
   CalcContour(ArrEll, ArrRect, ArrPlg, ArrContour, NPntNax);
   if(ArrContour.GetSize()==0){
	  ExtBoundType = BOUND_NONE; 
	  InsBoundType = BOUND_NONE; 
   }
   else if(ArrContour.GetSize()==1){
	  InsBoundType = BOUND_NONE; 
   }
}

void CBoundCtrls::RemoveExtBound()
{
	RemoveCurBound();
	CustomDots.RemoveAll();
    ArrEll.RemoveAll();
    ArrRect.RemoveAll();
    ArrPlg.RemoveAll();
    ArrContour.RemoveAll();
	ExtBoundType = -1;
}

void CBoundCtrls::RemoveInsBound()
{
	RemoveCurBound();
	CustomDots.RemoveAll();
    ArrEll.RemoveAll();
    ArrRect.RemoveAll();
    ArrPlg.RemoveAll();
    ArrContour.RemoveAll();
	InsBoundType = -1;
}

void CBoundCtrls::RemoveCurBound()
{
   CurBound = CRect(0,0,0,0);
}

void CBoundCtrls::FormBoundsOnLoadFile()
{
	CalcContour(ArrEll, ArrRect, ArrPlg, ArrContour, NPntNax);

#ifdef _DEBUG
	{
		CString debugPath = "C:\\temp\\contour_points.txt";
		FILE* debugFile = fopen(CStringA(debugPath), "w");
		if (debugFile) {
			for (int i = 0; i < ArrContour.GetSize(); i++) {
				XYPolygon& plg = ArrContour[i];
				fprintf(debugFile, "Contour %d (Type: %d, Points: %d):\n",
					i, plg.GetTypeLimits(), plg.GetSize());

				CArrayDouble arrX, arrY;
				plg.GetArrX(arrX);
				plg.GetArrY(arrY);

				for (int j = 0; j < arrX.GetSize(); j++) {
					fprintf(debugFile, "%.6f %.6f\n", arrX[j], arrY[j]);
				}
				fprintf(debugFile, "\n");
			}
			fclose(debugFile);
			TRACE("Saved contour points to %s\n", CStringA(debugPath));
		}
	}
#endif

	 int Type;
	 int i = 0;
	 int nExtCont, nInsCont;
	 nExtCont = nInsCont = 0;
	 for(i=0; i < ArrContour.GetSize(); i++){
		 Type = ArrContour[i].GetTypeLimits();
		 if(Type == EXTERNAL) nExtCont += 1;
		 else if(Type == INTERNAL) nInsCont += 1;
	 }
	 int nExtEll, nInsEll;
	 nExtEll = nInsEll = 0;
	 for(i=0; i < ArrEll.GetSize(); i++){
		 Type = ArrEll[i].TypeLimits;
		 if(Type == EXTERNAL) nExtEll += 1;
		 else if(Type == INTERNAL) nInsEll += 1;
	 }

	 int nExtRect, nInsRect;
	 nExtRect = nInsRect = 0;
	 for(i=0; i < ArrRect.GetSize(); i++){
		 Type = ArrRect[i].GetTypeLimits();
		 if(Type == EXTERNAL) nExtRect += 1;
		 else if(Type == INTERNAL) nInsRect += 1;
	 }

	 int nExtPlg, nInsPlg;
	 nExtPlg = nInsPlg = 0;
	 for(i=0; i < ArrPlg.GetSize(); i++){
		 Type = ArrPlg[i].GetTypeLimits();
		 if(Type == EXTERNAL) nExtPlg += 1;
		 else if(Type == INTERNAL) nInsPlg += 1;
	 }

     if(nExtCont){
		if(nExtEll==1 && nExtRect==0 && nExtPlg==0)
		  ExtBoundType = BOUND_ELLIPSE;
		else if(nExtEll==0 && nExtRect==1 && nExtPlg==0)
		  ExtBoundType = BOUND_RECT;
		else
		  ExtBoundType = BOUND_POLYGON;
     }
	 
     if(nInsCont==1){
		if(nInsEll==1 && nInsRect==0 && nInsPlg==0)
		  InsBoundType = BOUND_ELLIPSE;
		else if(nInsEll==0 && nInsRect==1 && nInsPlg==0)
		  InsBoundType = BOUND_RECT;
		else
		  InsBoundType = BOUND_POLYGON;
	 }
	 else if(nInsCont>1){
		  InsBoundType = BOUND_POLYGON;
	 }

	 CImageCtrls* pImCtrls = GetImageCtrls();
     int xDIB = pImCtrls->ImageSize.cx;
     int yDIB = pImCtrls->ImageSize.cy;

	 CRect Bound; 
	 CArray<CPoint, CPoint> PlgPoints;
	 if(GetExtRealBound(ExtBoundType, xDIB, yDIB, Bound, PlgPoints))
        ExtBoundRect = Bound;
	 int idx = 1;
	 if(GetInsRealBound(InsBoundType, xDIB, yDIB, idx, Bound, PlgPoints))
        InsBoundRect = Bound;
}

BOOL CBoundCtrls::GetPartsOfContours(int Type, CArrayXYEllipse& _ArrEll, CArrayXYRect& _ArrRect, CArrayXYPolygon& _ArrPlg)
{
	int i=0;
	_ArrEll.RemoveAll();
	_ArrRect.RemoveAll();
	_ArrPlg.RemoveAll();
	BOOL res = FALSE;
	
	for(i=0; i < ArrEll.GetSize(); i++){
		if(ArrEll[i].TypeLimits==Type){
		  _ArrEll.Add(ArrEll[i]);
		  res = TRUE;
		}
	}

	for(i=0; i < ArrRect.GetSize(); i++){
		if(ArrRect[i].TypeLimits==Type){
		  _ArrEll.Add(ArrEll[i]);
		  res = TRUE;
		}
	}

	for(i=0; i < ArrPlg.GetSize(); i++){
		if(ArrPlg[i].GetTypeLimits()==Type){
		  _ArrPlg.Add(ArrPlg[i]);
		  res = TRUE;
		}
	}
	return res;
}


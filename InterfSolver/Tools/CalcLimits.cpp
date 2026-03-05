#include "Matin1.h"
#include "CalcLimits.h"
//=========================================================================
/***********************************************************************************
 ФУНКЦИЯ:    CalcCircleLimits

 НАЗНАЧЕНИЕ:
             Расчет параметров круга и границ области по заданным
             точкам на границе области

 АРГУМЕНТЫ: 
          На входе: 

          На выходе:

 ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: 
              true -  если NPnt >= 3
              false - если NPnt < 3
************************************************************************************/
bool CalcCircleLimits(const XYBrokenLine &BLine, int TypeLimits, XYEllipse &Ell,
                                                                                XYBounds &Bnd)
  {
  int iPnt, k, m, l;
  int NPnt = BLine.GetSize();
  CArrayDouble C;
  CArrayDouble B;
  if (NPnt < 3)
    return false;
  C.InsertAt(0, 0., 12);
  B.SetSize(4);
  for (iPnt = 0; iPnt < NPnt; iPnt++)
    {
    B[0] = BLine[iPnt].X;
    B[1] = BLine[iPnt].Y;
    B[2] = 1.;
    B[3] = pow(BLine[iPnt].X,2) + pow(BLine[iPnt].Y,2);
    l = 0;
    for (k = 0; k < 3; k++)
      {  
      for (m = 0; m < 3; m++)
        C[l++] += B[k] * B[m];
      }
    for (k = 0; k < 3; k++)
      C[l++] += B[k] * B[3];
    }
  int NeError;
  double Determ;
  Matin1(C.GetData(), 3, 1, NeError, Determ);
  if (!NeError)
    return false;

  int NBeg = 9;
  double A1 = C[NBeg];
  double A2 = C[NBeg+1];
  double A3 = C[NBeg+2];
  double Xc = A1 / 2.;
  double Yc = A2 / 2.;
  double R = sqrt(A3 + pow(Xc,2) + pow(Yc,2));
  Ell = XYEllipse(R, R, Xc, Yc, 0., TypeLimits);

  double XLeft = Xc - R;
  double YTop = Yc - R;
  double XRight = Xc + R;
  double YBottom = Yc + R;
  Bnd = XYBounds(XLeft, YTop, XRight, YBottom);

  return true;
  }
//=========================================================================
/***********************************************************************************
 ФУНКЦИЯ:    CalcCircleLimits

 НАЗНАЧЕНИЕ:
             Расчет параметров круга по заданным  границам области

 АРГУМЕНТЫ: 
          На входе: 

          На выходе:

 ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: true
************************************************************************************/
bool CalcCircleLimits(const XYBounds &SBnd, int TypeLimits, XYEllipse &Ell, XYBounds &Bnd)
  {
  double Xc, Yc, Diam, Rad;
  Xc = (SBnd.XLeft + SBnd.XRight) / 2;
  Yc = (SBnd.YBottom + SBnd.YTop) / 2;
  double Rx = fabs(SBnd.XRight - SBnd.XLeft);
  double Ry = fabs(SBnd.YTop - SBnd.YBottom);
  Diam = MINV(Rx, Ry);
  Rad = Diam / 2.;
  Ell = XYEllipse(Rad, Rad, Xc, Yc, 0., TypeLimits);

  double XLeft = Xc - Rad;
  double YTop = Yc - Rad;
  double XRight = Xc + Rad;
  double YBottom = Yc + Rad;
  Bnd = XYBounds(XLeft, YTop, XRight, YBottom);
  return true;
  }
//=========================================================================
/***********************************************************************************
 ФУНКЦИЯ:    CalcEllipseLimits

 НАЗНАЧЕНИЕ:
             Расчет параметров эллипса и границ области по заданным
             точкам на границе области

 АРГУМЕНТЫ: 
          На входе: 

          На выходе:
           ELL_LIMITS Ell -  параметры эллипса

 ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: 
              true -  если NPnt >= 4
              false - если NPnt < 4
************************************************************************************/
bool CalcEllipseLimits(const XYBrokenLine &BLine, int TypeLimits, XYEllipse &Ell, 
                                                                                XYBounds &Bnd)
  {
  int iPnt, k, m, l;
  CArrayDouble C;
  CArrayDouble B;
  int NPnt = BLine.GetSize();
  if (NPnt < 4)
    return false;
  C.InsertAt(0, 0., 20);
  B.SetSize(5);
  for (iPnt = 0; iPnt < NPnt; iPnt++)
    {
    B[0] = pow(BLine[iPnt].X,2);
    B[1] = BLine[iPnt].X;
    B[2] = BLine[iPnt].Y;
    B[3] = 1.;
    B[4] = pow(BLine[iPnt].Y,2);
    l = 0;
    for (k = 0; k < 4; k++)
      {  
      for (m = 0; m < 4; m++)
        C[l++] += B[k] * B[m];
      }
    for (k = 0; k < 4; k++)
      C[l++] += B[k] * B[4];
    }
  int NeError;
  double Determ;
  Matin1(C.GetData(), 4, 1, NeError, Determ);
  if (!NeError)
    return false;

  int NBeg = 16;
  double A1 = C[NBeg];
  double A2 = C[NBeg+1];
  double A3 = C[NBeg+2];
  double A4 = C[NBeg+3];
  double Xc = -A2 / (2. * A1);
  double Yc =  A3 / 2.;
  double By2 = A4 - A1 *pow(Xc,2) + pow(Yc,2);
  double By = sqrt(By2);
  double Ax = sqrt (-By2 / A1);
  Ell = XYEllipse(Ax, By, Xc, Yc, 0., TypeLimits);

  double XLeft = Xc - Ax;
  double YTop = Yc - By;
  double XRight = Xc + Ax;
  double YBottom = Yc + By;
  Bnd = XYBounds(XLeft, YTop, XRight, YBottom);

  return true;
  }
//=========================================================================
/***********************************************************************************
 ФУНКЦИЯ:    CalcEllipseLimits

 НАЗНАЧЕНИЕ:
             Расчет параметров эллпмса по заданным  границам области

 АРГУМЕНТЫ: 
          На входе: 

          На выходе:
           ELL_LIMITS Ell -  параметры эллипса

 ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: true
************************************************************************************/
bool CalcEllipseLimits(const XYBounds &Bnd, int TypeLimits, XYEllipse &Ell)
  {
  Ell = XYEllipse(Bnd, TypeLimits);
  return true;
  }
//=========================================================================
/***********************************************************************************
 ФУНКЦИЯ:    CalcRectangleLimits

 НАЗНАЧЕНИЕ:
             Расчет параметров эллипсов, описывающих прямоугольник, и границ 
             области по заданным  точкам на границе области

 АРГУМЕНТЫ: 
          На входе: 

          На выходе:
           EMem<ELL_LIMITS> Ell -  массив параметров эллипсов

 ВОЗВРАЩАЕМОЕ ЗНАЧЕНИЕ: 
              true -  если NPnt >= 4
              false - если NPnt < 4
************************************************************************************/
bool CalcRectangleLimits(const XYBrokenLine &BLine, int TypeLimits, XYRect &Rect,
                                                                            XYBounds &Bnd)
  {
  int iPnt;
  double XMin, XMax;
  double YMin, YMax;
  int NPnt = BLine.GetSize();

  if (NPnt < 4)
    return false;

  XYPoint P;
  P = BLine[0];
  XMin = P.X;
  XMax = P.X;
  YMin = P.Y;
  YMax = P.Y;
  for (iPnt = 1; iPnt < NPnt; iPnt++)
    {
    P = BLine[iPnt];
    if (XMin < P.X)
      XMin = P.X;
    if (XMax > P.X)
      XMax = P.X;
    if (YMin < P.Y)
      YMin = P.Y;
    if (YMax > P.Y)
      YMax = P.Y;
    }

  if (NPnt == 4)
    {
    Bnd = XYBounds(XMin, YMin, XMax, YMax);
    Rect = XYRect(Bnd, TypeLimits);
    return true;
    }

  int iSide, iNearSide, NSidePnt;
  double DSide, DMin, MaxSide;
  int NBasePnt = NPnt;
  CArray<XYPoint,XYPoint> BaseLimPnt;
  BaseLimPnt.SetSize(NBasePnt);

  XYPoint CurPnt;
  MaxSide = MAXV(fabs(XMax-XMin), fabs(YMax-YMin));

  CArray<XYPoint,XYPoint> SidePnt[4];

  for (iPnt = 0; iPnt < NBasePnt; iPnt++)
    {
    P = BLine[iPnt];
    BaseLimPnt[iPnt].X = P.X;
    BaseLimPnt[iPnt].Y = P.Y;
    }

  while ((NBasePnt = BaseLimPnt.GetSize()) > 0)
    {
    CurPnt = BaseLimPnt[0];
    DMin = MaxSide;
    for (iSide = 0; iSide < 4; iSide++)
      {
      if (iSide == 0)
        DSide = fabs(XMin-CurPnt.X);
      else if (iSide == 1)
        DSide = fabs(YMax-CurPnt.Y);
      else if (iSide == 2)
        DSide = fabs(XMax-CurPnt.X);
      else if (iSide == 3)
        DSide = fabs(YMin-CurPnt.Y);
      if (DSide < DMin)
        {
        DMin = DSide;
        iNearSide = iSide;
        }
      }
    SidePnt[iNearSide].Add(CurPnt);
    BaseLimPnt.RemoveAt(0);
    }

  double XAve = 0.;
  NSidePnt = SidePnt[0].GetSize();
  for (iPnt = 0; iPnt < NSidePnt; iPnt++)
    XAve += SidePnt[0][iPnt].X;
  XAve /= NSidePnt;
  double XLeft = XAve;

  double YAve = 0.;
  NSidePnt = SidePnt[1].GetSize();
  for (iPnt = 0; iPnt < NSidePnt; iPnt++)
    YAve += SidePnt[1][iPnt].Y;
  YAve /= NSidePnt;
  double YBottom = YAve;

  XAve = 0.;
  NSidePnt = SidePnt[2].GetSize();
  for (iPnt = 0; iPnt < NSidePnt; iPnt++)
    XAve += SidePnt[2][iPnt].X;
  XAve /= NSidePnt;
  double XRight = XAve;


  YAve = 0.;
  NSidePnt = SidePnt[3].GetSize();
  for (iPnt = 0; iPnt < NSidePnt; iPnt++)
    YAve += SidePnt[3][iPnt].Y;
  YAve /= NSidePnt;
  double YTop = YAve;

  Bnd = XYBounds(XLeft, YTop, XRight, YBottom);
  Rect = XYRect(Bnd, TypeLimits);
  return true;
  }
//=========================================================================
bool CalcRectangleLimits(const XYBounds &Bnd, int TypeLimits, XYRect &Rect)
  {
  Rect = XYRect(Bnd, TypeLimits);
  return true;
  }
//=========================================================================
bool CalcPolygonLimits(const XYBrokenLine &BLine, int TypeLimits, XYPolygon &Plg)
  {
  Plg = XYPolygon(BLine, TypeLimits);
  return true;
  }
//=========================================================================
bool CalcBounds(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                                            const CArrayXYPolygon &ArrPlg, XYBounds &Bnd)
  {
  CArrayXYPolygon ArrCont;
  int iCont, NCont, iExtCont;

  CalcContour(ArrEll, ArrRect, ArrPlg, ArrCont);

  NCont = ArrCont.GetSize();
  for (iCont = 0; iCont < NCont; iCont++)
    {
    if (ArrCont[iCont].GetTypeLimits() == EXTERNAL)
      {
      iExtCont = iCont;
      break;
      }
    }
  Bnd = ArrCont[iExtCont].GetBounds();
  return true;
  }
//=========================================================================
bool CalcBoundCircle(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                     const CArrayXYPolygon &ArrPlg, double &Xc,  double &Yc, double &Rad)
{
  // resulting circle parameters
  Xc = 0.;
  Yc = 0.;
  Rad = 0.;

  CArrayXYPolygon ArrCont;

  CalcContour(ArrEll, ArrRect, ArrPlg, ArrCont, 51);

  int NCont = ArrCont.GetSize();
  
  if (!NCont)
      return false;

  int iExtCont = 0;
  for (int iCont = 0; iCont < NCont; iCont++)
    {
    if (ArrCont[iCont].GetTypeLimits() == EXTERNAL)
      {
      iExtCont = iCont;
      break;
      }
    }

  auto Pc = ArrCont[iExtCont].GetCentroid();
  int NPnt = ArrCont[iExtCont].GetSize();
  double Dist, MaxDist = 0.;
  for (int i = 0; i < NPnt; i++)
    {
    Dist = Distance(Pc, ArrCont[iExtCont][i]);
    if (Dist > MaxDist)
      MaxDist = Dist;
    }
  Xc = Pc.X;
  Yc = Pc.Y;
  Rad = MaxDist;
  return true;
}
//=========================================================================
bool CalcBoundCircle(const XYBounds &Bnd, double &Xc,  double &Yc, double &Rad)
  {
  double Diam = MAXV(fabs(Bnd.XRight - Bnd.XLeft), fabs(Bnd.YTop - Bnd.YBottom));
  Rad = Diam / 2.;
  Xc = (Bnd.XRight + Bnd.XLeft) / 2.;
  Yc = (Bnd.YTop + Bnd.YBottom) / 2.;
  return true;
  }
//=========================================================================

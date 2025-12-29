#include "XYPolygon.h"
//=========================================================================
XYPolygon :: XYPolygon()
  {
  TypeLimits = EXTERNAL;
  TypeSystCoor = MEASURING;
  }
//=========================================================================
XYPolygon :: XYPolygon(const XYPolygon &A) : XYBrokenLine (A)
  {
  TypeLimits = A.TypeLimits;
  TypeSystCoor = A.TypeSystCoor;
  }
//=========================================================================

XYPolygon :: XYPolygon(const XYBrokenLine &A, int TypLim, int _TypeSystCoor) : 
                                                                          XYBrokenLine (A)
  {
  int NPnt = A.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  TypeLimits = TypLim;
  TypeSystCoor = _TypeSystCoor;
  }
//=========================================================================
XYPolygon :: XYPolygon(int NPnt, double *pArrX, double *pArrY, int TypLim, 
                                       int _TypeSystCoor) : XYBrokenLine (NPnt, pArrX, pArrY)
  {
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  TypeLimits = TypLim;
  TypeSystCoor = _TypeSystCoor;
  }
//=========================================================================
XYPolygon :: XYPolygon(int NPnt, int *pArrX, int *pArrY, int TypLim, int _TypeSystCoor) : 
                                                            XYBrokenLine (NPnt, pArrX, pArrY)
  {
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  TypeLimits = TypLim;
  TypeSystCoor = _TypeSystCoor;
  }
//=========================================================================
XYPolygon :: XYPolygon(const CArrayDouble &ArrX, const CArrayDouble &ArrY, int TypLim,
                                                int _TypeSystCoor) : XYBrokenLine (ArrX, ArrY)
  {
  int NPnt = ArrX.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  TypeLimits = TypLim;
  TypeSystCoor = _TypeSystCoor;
  }
//=========================================================================
XYPolygon :: XYPolygon(const CArrayDouble &ArrXY, int TypLim, int _TypeSystCoor) :
                                                                        XYBrokenLine (ArrXY)
  {
  int NPnt = ArrPnt.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  TypeLimits = TypLim;
  TypeSystCoor = _TypeSystCoor;
  }
//=========================================================================
XYPolygon :: ~XYPolygon()
  {
  }
//=========================================================================
XYPolygon& XYPolygon :: operator= (const XYPolygon &A)
  {
  ArrPnt.Copy(A.ArrPnt);
  TypeLimits = A.TypeLimits;
  TypeSystCoor = A.TypeSystCoor;
  return *this;
  }
//=========================================================================
XYPolygon& XYPolygon :: operator= (const CArrayXYPoint &A)
  {
  int i;
  int Na = A.GetSize();
  ArrPnt.SetSize(Na);
  for (i = 0; i < Na; i++)
    ArrPnt[i] = A[i];
  TypeLimits = EXTERNAL;
  TypeSystCoor = MEASURING;
  return *this;
  }
//=========================================================================
double XYPolygon :: Perimeter() const
  {
  int i;
  double Perim = 0.;
  int NPnt = GetSize();
  for (i = 0; i < NPnt-1; i++)
    Perim += Distance(ArrPnt[i], ArrPnt[i]);
  return Perim;

  }
//=========================================================================
bool XYPolygon :: isInside(const XYPoint &P) const
  {
  int i;
  double Phi = 0.;
  int NPnt = GetSize();
  for (i = 0; i < NPnt - 1; i++)
    Phi += Angle(ArrPnt[i+1]-P, ArrPnt[i]-P);
  if (fabs(Phi) > 6.28)
    return true;
  else if (fabs(Phi) < 0.0001)
    return false;
  return true;
  }
//=========================================================================
bool XYPolygon :: isInside(double X, double Y)
  {
  XYPoint P(X, Y);
  return isInside(P);
  }
//=========================================================================
bool XYPolygon :: isVisible(const XYPoint &P) const
  {
  bool isIn = isInside(P);
  if (isIn && TypeLimits == INTERNAL)
    return false;
  else if (!isIn && TypeLimits == EXTERNAL)
    return false;
  return true;
  }
//=========================================================================
bool XYPolygon :: isVisible(double X, double Y) 
  {
  XYPoint P(X, Y);
  return isVisible(P);
  }
//=========================================================================
void XYPolygon :: Normalize(double Xo, double Yo, double Ro)
  {
  int i;
  int NPnt = GetSize();
  for (i = 0; i < NPnt; i++)
    ArrPnt[i].Normalize(Xo, Yo, Ro);
  TypeSystCoor = NORMALISED;
  }
//=========================================================================
XYBounds XYPolygon :: GetBounds()
  {
  int i;
  XYBounds Bnd;
  XYPoint P;
  int NPnt = GetSize();
  double XMin = -E18;
  double XMax = E18;
  double YMin = -E18;
  double YMax = E18;
  for (i = 0; i < NPnt; i++)
    {
    P = ArrPnt[i];
    if (P.X < XMin)
      XMin = P.X;
    if (P.X > XMax)
      XMax = P.X;
    if (P.Y < YMin)
      YMin = P.Y;
    if (P.Y > YMax)
      YMax = P.Y;
    }
  Bnd.XLeft = XMin;
  Bnd.YTop = YMin;
  Bnd.XRight = XMax;
  Bnd.YBottom = YMax;
  return Bnd;
  }
//=========================================================================
XYPoint XYPolygon :: GetCentroid()
  {
  int i;
  XYPoint P, SumP;
  int NPnt = GetSize();

  SumP = XYPoint(0.,0.);
  for (i = 0; i < NPnt; i++)
    SumP =  SumP + ArrPnt[i];

  P.X = SumP.X / NPnt;
  P.Y = SumP.Y / NPnt;

  return P;
  }
//=========================================================================
void XYPolygon :: GetBounds(XYBounds &Bnd)
  {
  int i;
  XYPoint P;
  int NPnt = GetSize();
  double XMin = -E18;
  double XMax = E18;
  double YMin = -E18;
  double YMax = E18;
  for (i = 0; i < NPnt; i++)
    {
    P = ArrPnt[i];
    if (P.X < XMin)
      XMin = P.X;
    if (P.X > XMax)
      XMax = P.X;
    if (P.Y < YMin)
      YMin = P.Y;
    if (P.Y > YMax)
      YMax = P.Y;
    }
  Bnd.XLeft = XMin;
  Bnd.YTop = YMin;
  Bnd.XRight = XMax;
  Bnd.YBottom = YMax;
  }
//=========================================================================
bool isInside(const XYPolygon &Plg, const XYPoint &P)
    {
    int i;
    double Phi = 0.;
    int NPnt = Plg.GetSize();
    for (i = 0; i < NPnt - 1; i++)
      Phi += Angle(Plg[i+1]-P, Plg[i]-P);
    if (fabs(Phi) > 6.28)
      return true;
    else if (fabs(Phi) < 0.0001)
      return false;
    return true;
    }
//=========================================================================
bool isVisible(const XYPolygon &Plg, const XYPoint &P)
  {
  bool isIn = isInside(Plg, P);
  if (isIn && Plg.TypeLimits == INTERNAL)
    return true;
  else if (!isIn && Plg.TypeLimits == EXTERNAL)
    return true;
  return false;
  }
//=========================================================================
XYBounds GetBounds(const XYPolygon &Plg)
  {
  int i;
  XYBounds Bnd;
  XYPoint P;
  int NPnt = Plg.GetSize();
  double XMin = -E18;
  double XMax = E18;
  double YMin = -E18;
  double YMax = E18;
  for (i = 0; i < NPnt; i++)
    {
    P = Plg[i];
    if (P.X < XMin)
      XMin = P.X;
    if (P.X > XMax)
      XMax = P.X;
    if (P.Y < YMin)
      YMin = P.Y;
    if (P.Y > YMax)
      YMax = P.Y;
    }
  Bnd.XLeft = XMin;
  Bnd.YTop = YMin;
  Bnd.XRight = XMax;
  Bnd.YBottom = YMax;
  return Bnd;
  }
//=========================================================================
void GetBounds(const XYPolygon &Plg, XYBounds &Bnd)
  {
  int i;
  XYPoint P;
  int NPnt = Plg.GetSize();
  double XMin = -E18;
  double XMax = E18;
  double YMin = -E18;
  double YMax = E18;
  for (i = 0; i < NPnt; i++)
    {
    P = Plg[i];
    if (P.X < XMin)
      XMin = P.X;
    if (P.X > XMax)
      XMax = P.X;
    if (P.Y < YMin)
      YMin = P.Y;
    if (P.Y > YMax)
      YMax = P.Y;
    }
  Bnd.XLeft = XMin;
  Bnd.YTop = YMin;
  Bnd.XRight = XMax;
  Bnd.YBottom = YMax;
  }
//=========================================================================

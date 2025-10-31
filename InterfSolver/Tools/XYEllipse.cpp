#include "XYEllipse.h"
//=========================================================================
XYEllipse :: XYEllipse(double _Ax, double _By, double _Xc, double _Yc, double _Fi, 
                                                          int _TypeLimits, int _TypeSystCoor)
  {
  Ax = _Ax;
  By = _By;
  Xc = _Xc;
  Yc = _Yc;
  Fi = _Fi;
  TypeLimits = _TypeLimits;
  TypeSystCoor = _TypeSystCoor;
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
XYEllipse :: XYEllipse(const XYEllipse &A)
  {
  Ax = A.Ax;
  By = A.By;
  Xc = A.Xc;
  Yc = A.Yc;
  Fi = A.Fi;
  TypeLimits = A.TypeLimits;
  TypeSystCoor = A.TypeSystCoor;   
  Si = A.Si;
  Co = A.Co;
  }
//=========================================================================
XYEllipse :: XYEllipse(const XYBounds &Bnd, int _TypeLimits, int _TypeSystCoor)
  {
  Xc = (Bnd.XLeft + Bnd.XRight) / 2;
  Yc = (Bnd.YBottom + Bnd.YTop) / 2;
  Ax = fabs((Bnd.XRight - Bnd.XLeft) / 2);
  By = fabs((Bnd.YTop - Bnd.YBottom) / 2);
  Fi = 0.;
  TypeLimits = _TypeLimits;
  TypeSystCoor = _TypeSystCoor;
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
void XYEllipse :: Set(double _Ax, double _By, double _Xc, double _Yc, double _Fi, 
                                                          int _TypeLimits, int _TypeSystCoor)
  {
  Ax = _Ax;
  By = _By;
  Xc = _Xc;
  Yc = _Yc;
  Fi = _Fi;
  TypeLimits = _TypeLimits;
  TypeSystCoor = _TypeSystCoor;
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
XYEllipse :: ~XYEllipse()
  {
  }
//=========================================================================
XYEllipse& XYEllipse ::operator= (const XYEllipse &A)
  {
  Ax = A.Ax;
  By = A.By;
  Xc = A.Xc;
  Yc = A.Yc;
  Fi = A.Fi;
  TypeLimits = A.TypeLimits;
  TypeSystCoor = A.TypeSystCoor;   
  Si = A.Si;
  Co = A.Co;
  return *this;
  }
//=========================================================================
double XYEllipse :: Perimeter()const
  {
  double Perim = PI * (1.5 * (Ax + By) - sqrt(Ax * By));
  return Perim;
  }
//=========================================================================
bool XYEllipse :: isInside(const XYPoint &P) const
  {
  double X1 =  (P.X - Xc) * Co + (P.Y - Yc) * Si;
  double Y1 = -(P.X - Xc) * Si + (P.Y - Yc) * Co;
  double R = (X1 * X1 / (Ax * Ax) + Y1 * Y1 / (By * By));
  double T = R - 1.;

  if (T <= 0.)
    return true;
  else if (T > 0.)
    return false;

  return true;
  }
//=========================================================================
bool XYEllipse :: isInside(double X, double Y)
  {
  XYPoint P(X, Y);
  return isInside(P);
  }
//=========================================================================
bool XYEllipse :: isVisible(const XYPoint &P) const
  {
  bool isIn = isInside(P);
  if (isIn && TypeLimits == INTERNAL)
    return false;
  else if (!isIn && TypeLimits == EXTERNAL)
    return false;
  return true;
  }
//=========================================================================
bool XYEllipse :: isVisible(double X, double Y)
  {
  XYPoint P(X, Y);
  return isVisible(P);
  }
//=========================================================================
bool XYEllipse :: GetContour(XYBrokenLine &BLine, int NFi) const 
  {
  BLine.RemoveAll();
  double Th, dTh, SiT, CoT, AveR, R;
  int j;
  AveR = fabs(Ax) + fabs(By);
  if (AveR <= PRECISION)
    return false;
  XYPoint P;
  BLine.SetSize(NFi);
  dTh = PI2 / (NFi);
  for (j = 0; j < NFi; j++)
    {
    Th = j * dTh;
    SiT = sin(Th);
    CoT = cos(Th);
    R = Ax * By / sqrt(pow(Ax*SiT,2) + pow(By*CoT,2));
    P.X = Xc + R * (Co * CoT - Si * SiT);
    P.Y = Yc + R * (Si * CoT + Co * SiT);
    BLine[j] = P;
    }
  return true;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYBrokenLine &BLine, double Step) const
  {
    if (Step <= 0.)
        return false;

  double Perim = Perimeter();
  int NFi = static_cast<int>(Perim / Step);
  bool isSuccess = GetContour(BLine, NFi);
  return isSuccess;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYPolygon &Plg, int NFi) const
  {
  Plg.RemoveAll();
  XYBrokenLine BLine;
  bool isSuccess = GetContour(BLine, NFi);
  if (isSuccess)
    Plg = XYPolygon(BLine, TypeLimits, TypeSystCoor);
  return isSuccess;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYPolygon &Plg, double Step) const
  {
  double Perim = Perimeter();
  int NFi = static_cast<int>(Perim / Step);
  bool isSuccess = GetContour(Plg, NFi);
  return isSuccess;
  }
//=========================================================================
void XYEllipse :: InverseY(double YcInv)
  {
  Yc = YcInv - Yc;
  Fi = -Fi;
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
void XYEllipse :: ShiftX(double dX)
  {
  Xc += dX;
  }
//=========================================================================
void XYEllipse :: ShiftY(double dY)
  {
  Yc += dY;
  }
//=========================================================================
void XYEllipse :: Normalize(double Xo, double Yo, double Ro)
  {
  Ax /= Ro;
  By /= Ro;
  Xc = (Xc - Xo) / Ro;
  Yc = (Yc - Yo) / Ro;
  TypeSystCoor = NORMALISED;
  }
//=========================================================================
void XYEllipse :: DeNormalize(double Xo, double Yo, double Ro)
  {
  Ax *= Ro;
  By *= Ro;
  Xc = Xc * Ro + Xo;
  Yc = Yc * Ro + Yo;
  TypeSystCoor = MEASURING;
  }
//=========================================================================
bool isInside(const XYEllipse &Ell, const XYPoint &P)
  {
  double X1 =  (P.X - Ell.Xc) * Ell.Co + (P.Y - Ell.Yc) * Ell.Si;
  double Y1 = -(P.X - Ell.Xc) * Ell.Si + (P.Y - Ell.Yc) * Ell.Co;
  double R = (X1 * X1 / (Ell.Ax * Ell.Ax) + Y1 * Y1 / (Ell.By * Ell.By));
  double T = R - 1.;

  if (T < 0.)
    return true;
  else if (T > 0.)
    return false;
  return true;
  }
//=========================================================================
bool isVisible(const XYEllipse &Ell, const XYPoint &P)
  {
  bool isIn = isInside(Ell, P);
  if (isIn && Ell.TypeLimits == INTERNAL)
    return false;
  else if (!isIn && Ell.TypeLimits == EXTERNAL)
    return false;
  return true;
  }
//=========================================================================
void GetContour(const XYEllipse &Ell, XYPolygon &Plg, int NFi)
  {
  Plg.RemoveAll();
  double Th, dTh, SiT, CoT, AveR, R;
  int j;
  AveR = fabs(Ell.Ax) + fabs(Ell.By);
  if (AveR <= PRECISION)
    return;
  XYPoint P;
  Plg.SetSize(NFi+1);
  dTh = PI2 / (NFi);
  for (j = 0; j < NFi; j++)
    {
    Th = j * dTh;
    SiT = sin(Th);
    CoT = cos(Th);
    R = Ell.Ax * Ell.By / sqrt(pow(Ell.Ax*SiT,2) + pow(Ell.By*CoT,2));
    P.X = Ell.Xc + R * (Ell.Co * CoT - Ell.Si * SiT);
    P.Y = Ell.Yc + R * (Ell.Si * CoT + Ell.Co * SiT);
    Plg[j] = P;
    }
  Plg[NFi] = Plg[0];

  Plg.SetTypeLimits(Ell.TypeLimits);
  Plg.SetTypeSystCoor(Ell.TypeSystCoor);
  }
//=========================================================================

#ifndef _XYPOINT_H_
#define _XYPOINT_H_

#include "InterfSolver\Include\Int_Cons.h"
#include <math.h>

struct XYPoint
  {
  double X;
  double Y;
  XYPoint()
    {
    X = 0.;
    Y = 0.;
    }
  XYPoint(double _X, double _Y)
    {
    X = _X;
    Y = _Y;
    }
  XYPoint(const XYPoint &A)
    {
    X = A.X;
    Y = A.Y;
    }
  ~XYPoint()
    {
    }
  XYPoint& operator= (const XYPoint &A)
    {
    X = A.X;
    Y = A.Y;
    return *this;
    }
  double Distance(const XYPoint P)
    {
    return sqrt(pow((X-P.X),2) + pow((Y-P.Y),2));
    }
  void Polar(double &R, double Fi)
    {
    R = sqrt(pow((X),2) + pow((Y),2));
    Fi = atan2(Y, X);
    }
  double Angle() const
    {
    return atan2(Y, X);
    }
  double AngleGrd() const
    {
    return atan2(Y, X) * RD_GRD;
    }
  void Normalize(double Xo, double Yo, double Ro)
    {
    X = (X - Xo) / Ro;
    Y = (Y - Yo) / Ro;
    }
  friend double Distance(const XYPoint &P1, const XYPoint &P2)
    {
    return sqrt(pow((P1.X-P2.X),2) + pow((P1.Y-P2.Y),2));
    }
  friend double Angle(const XYPoint &P1, const XYPoint &P2)
    {
    double dFi = P2.Angle() - P1.Angle();
    if (fabs(dFi) > PI)
      dFi -= SIGN(dFi) * PI2;
    return dFi;
    }
  friend XYPoint operator+ (const XYPoint &P1, const XYPoint &P2)
    {
    XYPoint P;
    P.X = P1.X + P2.X;
    P.Y = P1.Y + P2.Y;
    return P;
    }
  friend XYPoint operator- (const XYPoint &P1, const XYPoint &P2)
    {
    XYPoint P;
    P.X = P1.X - P2.X;
    P.Y = P1.Y - P2.Y;
    return P;
    }
  };
#endif

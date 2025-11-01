#ifndef _XYPOLYNOM_H_
#define _XYPOLYNOM_H_

#include "stdafx.h"
#include "Include\Int_Cons.h"
#include "include\CArrDef.h"

//============================================================================
class XYPolynom
  {
  protected:
    int NPow;
    CArrayDouble Coeff;

  public:
    XYPolynom();
    XYPolynom(const XYPolynom &A);
    XYPolynom(int NPow);
    XYPolynom(int NPow, const CArrayDouble &ArrCoeff);
    ~XYPolynom();
    int GetPow() const {return NPow;}
    int GetSize() const {return (NPow + 1) * (NPow + 2) / 2;}
    int GetIndex(int i, int j);
    void ChangePow(int NewPow);
    XYPolynom& operator= (const XYPolynom &A);
    const double operator[] (int iC) const {return Coeff[iC];}
    double &operator[] (int iC) {return Coeff[iC];}
    double CalcValue(double X, double Y);
    void Shift(double dX, double dY);
    void Rotation(double Fi);
    void AffineTransform(double a11,double a12, double a21,double a22);
    void Derivation(double X, double Y, double &Fx, double &Fy);
    void Delete();
    void Clear();

  friend XYPolynom operator+ (const XYPolynom &A, const XYPolynom &B);
  friend XYPolynom operator- (const XYPolynom &A, const XYPolynom &B);
  friend XYPolynom operator* (const XYPolynom &A, double C);
  friend XYPolynom operator* (double C, const XYPolynom &A);
  friend XYPolynom operator/ (const XYPolynom &A, double C);
  };

//============================================================================
#endif

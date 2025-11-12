#include "ClassicAberr.h"

//============================================================================
SPH_COEFF :: SPH_COEFF()
  {
  D = 0.;
  Lx = 0.;
  Ly = 0.;
  C = 0.;
  }
//============================================================================
SPH_COEFF :: SPH_COEFF(const SPH_COEFF &Sph)
  {
  D = Sph.D;
  Lx = Sph.Lx;
  Ly = Sph.Ly;
  C = Sph.C;
  }
//============================================================================
SPH_COEFF :: SPH_COEFF(double _D, double _Lx, double _Ly, double _C)
  {
  D = _D;
  Lx = _Lx;
  Ly = _Ly;
  C = _C;
  }
//============================================================================
SPH_COEFF :: ~SPH_COEFF()
  {
  D = 0.;
  Lx = 0.;
  Ly = 0.;
  C = 0.;
  }
//============================================================================
SPH_COEFF& SPH_COEFF :: operator= (const SPH_COEFF &Sph)
  {
  D = Sph.D;
  Lx = Sph.Lx;
  Ly = Sph.Ly;
  C = Sph.C;
  return *this;
  }
//============================================================================
double SPH_COEFF :: CalcValue(double X, double Y)
  {
  double S;
  S = D * (Y * Y + X * X) + Lx * X + Ly * Y + C;
  return S;
  }
//============================================================================
XYPolynom SPH_COEFF :: ConvToPolynom () const
  {
  XYPolynom Pol;
  Pol = XYPolynom(2);
  Pol[0] = C;
  Pol[1] = Ly;
  Pol[2] = D;
  Pol[3] = Lx;
  Pol[4] = 0.;
  Pol[5] = D;
  return Pol;
  }
//============================================================================
void SPH_COEFF :: Clear()
  {
  D = 0.;
  Lx = 0.;
  Ly = 0.;
  C = 0.;
  }
//============================================================================
void SPH_COEFF :: Delete()
  {
  D = 0.;
  Lx = 0.;
  Ly = 0.;
  C = 0.;
  }
//============================================================================
XYPolynom operator+ (const XYPolynom &A, const SPH_COEFF &B)
  {
  XYPolynom C, SphPol;
  SphPol = B.ConvToPolynom();
  C = A + SphPol;
  return C;
  }
//============================================================================
XYPolynom operator- (const XYPolynom &A, const SPH_COEFF &B)
  {
  XYPolynom C, SphPol;
  SphPol = B.ConvToPolynom();
  C = A - SphPol;
  return C;
  }
//============================================================================
//============================================================================
AST_COEFF :: AST_COEFF()
  {
  A = 0.;
  Fa = 0.;
  }
//============================================================================
AST_COEFF :: AST_COEFF(const AST_COEFF &Ast)
  {
  A = Ast.A;
  Fa = Ast.Fa;
  }
//============================================================================
AST_COEFF :: AST_COEFF(double _A, double _Fa)
  {
  A = _A;
  Fa = _Fa;
  }
//============================================================================
AST_COEFF :: ~AST_COEFF()
  {
  A = 0.;
  Fa = 0.;
  }
//============================================================================
AST_COEFF& AST_COEFF :: operator= (const AST_COEFF &Ast)
  {
  A = Ast.A;
  Fa = Ast.Fa;
  return *this;
  }
//============================================================================
double AST_COEFF :: CalcValue(double X, double Y)
  {
  double S;
  double A1 = A * cos(2.*Fa * GRD_RD);
  double A2 = -2 * A * sin(2.*Fa * GRD_RD);
  S = A1 * (Y * Y - X * X) + A2 * X * Y;
  return S;
  }
//============================================================================
XYPolynom AST_COEFF :: ConvToPolynom () const
  {
  XYPolynom Pol;
  Pol = XYPolynom(2);
  double A1 = A * cos(2.*Fa * GRD_RD);
  double A2 = -2. * A * sin(2.*Fa * GRD_RD);
  Pol[2] = A1;
  Pol[4] = A2;
  Pol[5] = -A1;
  return Pol;
  }
//============================================================================
void AST_COEFF :: Clear()
  {
  A = 0.;
  Fa = 0.;
  }
//============================================================================
void AST_COEFF :: Delete()
  {
  A = 0.;
  Fa = 0.;
  }
//============================================================================
XYPolynom operator+ (const XYPolynom &A, const AST_COEFF &B)
  {
  XYPolynom C, AstPol;
  AstPol = B.ConvToPolynom();
  C = A + AstPol;
  return C;
  }
//============================================================================
XYPolynom operator- (const XYPolynom &A, const AST_COEFF &B)
  {
  XYPolynom C, AstPol;
  AstPol = B.ConvToPolynom();
  C = A - AstPol;
  return C;
  }
//============================================================================
//============================================================================
COM_COEFF :: COM_COEFF()
  {
  C = 0.;
  Fc = 0.;
  }
//============================================================================
COM_COEFF :: COM_COEFF(const COM_COEFF &Coma)
  {
  C = Coma.C;
  Fc = Coma.Fc;
  }
//============================================================================
COM_COEFF :: COM_COEFF(double _C, double _Fc)
  {
  C = _C;
  Fc = _Fc;
  }
//============================================================================
COM_COEFF :: ~COM_COEFF()
  {
  C = 0.;
  Fc = 0.;
  }
//============================================================================
COM_COEFF& COM_COEFF :: operator= (const COM_COEFF &Coma)
  {
  C = Coma.C;
  Fc = Coma.Fc;
  return *this;
  }
//============================================================================
double COM_COEFF :: CalcValue(double X, double Y)
  {
  double S;
  double C1 = -C * sin(Fc * GRD_RD);
  double C2 = C * cos(Fc * GRD_RD);
  S = (X * X + Y * Y - 2./3.) * (C1 * X + C2 * Y);
  return S;
  }
//============================================================================
XYPolynom COM_COEFF :: ConvToPolynom () const
  {
  XYPolynom Pol;
  Pol = XYPolynom(3);
  double CC = 2./3.;
  double C1 = -C * sin(Fc * GRD_RD);
  double C2 = C * cos(Fc * GRD_RD);
  Pol[1] = -C2 * CC;
  Pol[3] = C2;
  Pol[4] = -C1 * CC;
  Pol[6] = C1;
  Pol[8] = C2;
  Pol[9] = C1;
  return Pol;
  }
//============================================================================
void COM_COEFF :: Clear()
  {
  C = 0.;
  Fc = 0.;
  }
//============================================================================
void COM_COEFF :: Delete()
  {
  C = 0.;
  Fc = 0.;
  }
//============================================================================
XYPolynom operator+ (const XYPolynom &A, const COM_COEFF &B)
  {
  XYPolynom C, ComPol;
  ComPol = B.ConvToPolynom();
  C = A + ComPol;
  return C;
  }
//============================================================================
XYPolynom operator- (const XYPolynom &A, const COM_COEFF &B)
  {
  XYPolynom C, ComPol;
  ComPol = B.ConvToPolynom();
  C = A - ComPol;
  return C;
  }
//============================================================================
//============================================================================
ZON_COEFF :: ZON_COEFF()
  {
  Type = POWER;
  }
//============================================================================
ZON_COEFF :: ZON_COEFF(const ZON_COEFF &A)
  {
  Type = A.Type;
  Coeff.Copy(A.Coeff);
  }
//============================================================================
ZON_COEFF :: ZON_COEFF(int NCoeff, int _Type)
  {
  Type = _Type;
  Coeff.SetSize(NCoeff);
  int i;
  for (i = 0; i < NCoeff; i++)
    Coeff[i] = 0.; 
  }
//============================================================================
ZON_COEFF :: ZON_COEFF(const CArrayDouble &ArrCoeff, int _Type)
  {
  Type = _Type;
  Coeff.Copy(ArrCoeff);
  }
//============================================================================
ZON_COEFF :: ~ZON_COEFF()
  {
  Type = POWER;
  Coeff.RemoveAll();
  }
//============================================================================
void ZON_COEFF :: ChangeSize(int NewSize)
  {
  int NSize = GetSize();
  if (NSize == NewSize)
    return;
  ZON_COEFF Zon(NewSize, Type);
  int i;
  if (NSize < NewSize)
    {
    for (i = 0; i < NSize; i++)
      Zon.Coeff[i] = Coeff[i];
    }
  else if (NSize > NewSize)
    {
    for (i = 0; i < NewSize; i++)
      Zon.Coeff[i] = Coeff[i];
    }
  Coeff.Copy(Zon.Coeff);
  }
//============================================================================
void ZON_COEFF :: ChangeType(int NewType)
  {
  if (NewType == Type)
    return;
  if (NewType == POWER)
    ConvToPower();
  else if (NewType == ZERNIKE)
    ConvToZernike();
  }
//============================================================================
ZON_COEFF& ZON_COEFF :: operator= (const ZON_COEFF &A)
  {
  Type = A.Type;
  Coeff.Copy(A.Coeff);
  return *this;
  }
//============================================================================
double ZON_COEFF :: CalcValue(double X, double Y)
  {
  int i;
  int NSize = GetSize();
  double S = 0.;
  if (NSize == 0)
    return S;

  double R = X * X + Y * Y;
  switch (Type)
    {
    case POWER:
      for (i = NSize-1; i >= 0; i--)
        S = S * R + Coeff[i];
      break;
    case ZERNIKE:
      S = Coeff[0];
      if (NSize < 2)
        break;
      S += Coeff[1] * (R - 0.5);
      if (NSize < 3)
        break;
      S += Coeff[2] * (pow(R,2) - R + 0.166666667);
      if (NSize < 4)
        break;
      S += Coeff[3] * (R * (R * (R - 1.5) + 0.6) - 0.05);
      if (NSize < 5)
        break;
      S += Coeff[4] * (R * (R * (R * (R - 2.) + 1.28571) - 0.285714) + 0.01428);
      if (NSize < 6)
        break;
      S += Coeff[5] * (R * (R * (R * (R * (R - 2.5) + 2.222222) - 0.8333333) +
                                                               0.11905)-0.003968);
      break;
    }
  return S;
  }
//============================================================================
XYPolynom ZON_COEFF :: ConvToPolynom () const
  {
  XYPolynom Pol;
  int NSize = GetSize();
  if (NSize == 0)
    return Pol;

  int i;
  double  G[12];
  for (i=0; i < 12; i++)
    G[i] = 0.;

  int nPA = 2 * (NSize - 1);
  int nP1 = (nPA + 2) / 2;

  Pol = XYPolynom(nPA);
  G[0] = 1;
  int j, lj, l;
  double B;
  for (l = 0; l < nP1; l++)
    {
    j = l;
    B = Coeff[l];
    for (i = 0; i <= l; i++)
      {
      if (j) G[j] = G[j] + G[j-1];
      lj = i * (2 * (nPA - i - 1) + 5 ) + 2 * j;
      Pol[lj] = Pol[lj] + B * G[j];
      j--;
      }
    }
  return Pol;
  }
//============================================================================
void ZON_COEFF :: Delete()
  {
  Type = POWER;
  Coeff.RemoveAll();
  }
//============================================================================
void ZON_COEFF :: Clear()
  {
  Type = POWER;
  Coeff.RemoveAll();
  }
//============================================================================
void ZON_COEFF :: ConvToPower()
  {
  if (Type == POWER)
    return;
  int NSize = GetSize();
  if (NSize == 0)
    return;

  ZON_COEFF S, B;
  double C;
   
  B =  ZON_COEFF(1);
  C = Coeff[0];
  B.Coeff[0] = C;
  S = S + B;
  if (NSize < 2)
    {
    Type = S.Type;
    Coeff.Copy(S.Coeff);
    return;
    }
  B =  ZON_COEFF(2);
  C = Coeff[1];
  B.Coeff[0] = -0.5 * C;
  B.Coeff[1] = C;
  S = S + B;
  if (NSize < 3)
    {
    Type = S.Type;
    Coeff.Copy(S.Coeff);
    return;
    }
  B =  ZON_COEFF(3);
  C = Coeff[2];
  B.Coeff[0] = 0.1666667 * C;
  B.Coeff[1] = -C;
  B.Coeff[2] = C;
  S = S + B;
  if (NSize < 4)
    {
    Type = S.Type;
    Coeff.Copy(S.Coeff);
    return;
    }
  B =  ZON_COEFF(4);
  C = Coeff[3];
  B.Coeff[0] = -0.05 * C;
  B.Coeff[1] = 0.6 * C;
  B.Coeff[2] = -1.5 * C;
  B.Coeff[3] = C;
  S = S + B;
  if (NSize < 5)
    {
    Type = S.Type;
    Coeff.Copy(S.Coeff);
    return;
    }
  B =  ZON_COEFF(5);
  C = Coeff[4];
  B.Coeff[0] = 0.01428 * C;
  B.Coeff[1] = -0.285714 * C;
  B.Coeff[2] = 1.28571 * C;
  B.Coeff[3] = -2 * C;
  B.Coeff[4] = C;
  S = S + B;
  if (NSize < 6)
    {
    Type = S.Type;
    Coeff.Copy(S.Coeff);
    return;
    }
  B =  ZON_COEFF(6);
  C = Coeff[5];
  B.Coeff[0] = -0.003968 * C;
  B.Coeff[1] = 0.11905 * C;
  B.Coeff[2] = -0.8333 * C;
  B.Coeff[3] = 2.2222 * C;
  B.Coeff[4] = -2.5 * C;
  B.Coeff[5] = C;
  S = S + B;
  Type = S.Type;
  Coeff.Copy(S.Coeff);
  }
//============================================================================
void ZON_COEFF :: ConvToZernike()
  {
// TODO: implement conversion ZON_COEFF to Zernike
  }
//============================================================================
ZON_COEFF operator+ (const ZON_COEFF &A, const ZON_COEFF &B)
  {
  ZON_COEFF S, A1, B1;
  A1 = A;
  B1 = B;
  if (A1.Type != B1.Type)
    {
    if (A1.Type == ZERNIKE)
      A1.ChangeType(POWER);
    else if (B1.Type == ZERNIKE)
      B1.ChangeType(POWER);
    }

  int i;
  int NSizeA = A1.GetSize();
  int NSizeB = B1.GetSize();

  if (NSizeA = 0)
    return B1;
  if (NSizeB = 0)
    return A1;

  if (NSizeA == NSizeB)
    {
    S = ZON_COEFF(NSizeA);
    for (i = 0; i < NSizeA; i++)
      S[i] = A1[i] + B1[i];
    }
  else if (NSizeA > NSizeB)
    {
    S = ZON_COEFF(NSizeA);
    B1.ChangeSize(NSizeA);
    for (i = 0; i < NSizeA; i++)
      S[i] = A1[i] + B1[i];
    }
  else if (NSizeA < NSizeB)
    {
    S = ZON_COEFF(NSizeB);
    A1.ChangeSize(NSizeB);
    for (i = 0; i < B.GetSize(); i++)
      S[i] = A1[i] + B1[i];
    }
  return S;
  }
//============================================================================
XYPolynom operator+ (const XYPolynom &A, const ZON_COEFF &B)
  {
  XYPolynom C, ZonPol;
  ZonPol = B.ConvToPolynom();
  C = A + ZonPol;
  return C;
  }
//============================================================================
XYPolynom operator- (const XYPolynom &A, const ZON_COEFF &B)
  {
  XYPolynom C, ZonPol;
  ZonPol = B.ConvToPolynom();
  C = A - ZonPol;
  return C;
  }
//============================================================================

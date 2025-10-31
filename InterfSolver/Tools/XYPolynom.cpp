#include "XYPolynom.h"

//============================================================================
XYPolynom :: XYPolynom()
  {
  NPow = -1;
  }
//============================================================================
XYPolynom :: XYPolynom(const XYPolynom &A)
  {
  NPow = A.NPow;
  Coeff.Copy(A.Coeff);
  }
//============================================================================
XYPolynom :: XYPolynom(int _NPow)
  {
  NPow = _NPow;
  int NCoeff = GetSize();
  Coeff.SetSize(NCoeff);
  int i;
  for (i = 0; i < NCoeff; i++)
    Coeff[i] = 0.; 

  }
//============================================================================
XYPolynom :: XYPolynom(int _NPow, const CArrayDouble &ArrCoeff)
  {
  NPow = _NPow;
  int NCoeff = GetSize();
  Coeff.SetSize(NCoeff);
  int i;
  for (i = 0; i < NCoeff; i++)
    Coeff[i] = ArrCoeff[i]; 
  }
//============================================================================
XYPolynom :: ~XYPolynom()
  {
  NPow = -1;
  Coeff.RemoveAll();
  }
//============================================================================
int XYPolynom :: GetIndex(int i, int j)
  {
  if (i > NPow || j > NPow - i)
    return -1;
  int Ind = (2 * NPow + 3 - i) * i / 2 + j;
  return Ind;
  }
//============================================================================
void XYPolynom :: ChangePow(int NewPow)
  {
  if (NPow == NewPow)
    return;
  XYPolynom Pol(NewPow);
  if (NPow == -1)
    {
    NPow = NewPow;
    Coeff.Copy(Pol.Coeff);
    return;
    }
  int i, j, l, l1;
  if (NPow < NewPow)
    {
    for (i = 0; i <= NPow; i++)
      {
      for (j = 0; j <= NPow-i; j++)
        {
        l = Pol.GetIndex(i, j);
        l1 = GetIndex(i, j);
        Pol.Coeff[l] = Coeff[l1];
        }
      }
    }
  else if (NPow > NewPow)
    {
    for (i = 0; i <= NewPow; i++)
      {
      for (j = 0; j <= NewPow-i; j++)
        {
        l = Pol.GetIndex(i, j);
        l1 = GetIndex(i, j);
        Pol[l] = Coeff[l1];
        }
      }
    }
  NPow = NewPow;
  Coeff.Copy(Pol.Coeff);
  }
//============================================================================
XYPolynom& XYPolynom :: operator= (const XYPolynom &A)
  {
  NPow = A.NPow;
  Coeff.Copy(A.Coeff);
  return *this;
  }
//============================================================================
double XYPolynom :: CalcValue(double X, double Y)
  {
  double Sx = 0., Sy;
  int i, j;
  if (NPow == -1)
    return Sx;
  int NCoeff = GetSize();
  if (NCoeff == 1)
    return Coeff[0];
  NCoeff--;
  int k = NCoeff;
  Sx = Coeff[k];
  for (i = 0; i < NPow ; i++) 
    {
    k--;
    Sy = Coeff[k];
    for (j = 0; j <= i; j++)
      {
      k--;
      Sy = Sy * Y + Coeff[k];
      }
    Sx = Sx * X + Sy;
    }
  return Sx;
  }
//============================================================================
void XYPolynom :: Shift(double dX, double dY)
  {
  if (NPow == -1)
    return;
  CArrayDouble G, Cx, Cy;
  int NCoeff = GetSize();
  G.SetSize(NPow+1);
  Cx.SetSize(NCoeff);
  Cy.SetSize(NCoeff);

  Cx[0] = 1.;
  Cy[0] = 1.;
  G[0] = 1.;
  int NP1 = NPow + 1;
  int NP2 = 2 * NP1;
  int iD, ND, k, i, l, k1, j;
  for (iD = 2; iD <= NP1; iD++)
    {
    ND = iD;
    int LCn = iD * (iD-1) / 2;
    for (k = 0; k < ND; k++)
      {
      k1 = LCn + k;
      Cx[k1] = 1.;
      Cy[k1] = 1.;
      }
    G[iD-1] = 0.;
    int NSt = iD - 1;
    for (i = 0; i < NSt; i++)
      {
      int jb = ND - i - 1;
      G[jb] = G[jb-1] + G[jb];
      int NMu = ND - i - 1;
      for (l = 0; l < NMu; l++)
        {
        int l1 = LCn + l;
        Cx[l1] *= dX;
        Cy[l1] *= dY;
        }
      }
    for (k = 0; k < ND; k++) 
      {
      k1 = LCn + k;
      Cx[k1] *= G[k];
      Cy[k1] *= G[k];
      }
    int NiD = NP1 - iD;
    for (l = 0; l < ND; l++)
      {
      int Lx = l + 1;
      int Ly = ND - l;
      int LCx = Lx * (Lx - 1) / 2;
      int LCy = Ly * ( Ly - 1) / 2;
      int Lp = (l+1)*(NP2 - (l+1) + 1)/2 - NiD - 1;
      double Pf = Coeff[Lp];
      Coeff[Lp] = 0.;
      for (i = 0; i < Lx; i++) 
        {
        int i1 = LCx + i;
        double CXp = Cx[i1] * Pf;
        int IJn = (NP2 - i + 1) * i / 2;
        for (j = 0; j < Ly; j++)
          {
          int ij = IJn + j;
          int j1 = LCy + j;
          Coeff[ij] += CXp * Cy[j1];
          }
        }
      }
    }
  }
//============================================================================
void XYPolynom :: Rotation(double Fi)
  {
  if (NPow == -1)
    return;
  double FiR = Fi * GRD_RD;
  double a11 = cos(FiR);
  double a12 = sin(FiR);
  double a21 = - a12;
  double a22 = a11;
  AffineTransform(a11, a12, a21, a22);
  }
//============================================================================
void XYPolynom :: AffineTransform(double a11,double a12, double a21,double a22)
  {
  if (NPow == -1)
    return;
  CArrayDouble PP, G, Cx, Cy;
  int NCoeff = GetSize();
  PP.SetSize(NPow+1);
  G.SetSize(NPow+1);
  Cx.SetSize(NCoeff);
  Cy.SetSize(NCoeff);
  int Id, *Nd;
  Nd = &Id;
  int NP1 = NPow + 1;
  int NP2 = 2 * NP1;
  Cx[0]=1.;
  Cy[0]=1.;
  G[0]=1.;
  int Lch, i, k, k1, Nst, Lck, Nmu, l, l1, l2, jb, Nid;
  int Lx, Ly, Lcx, Lcy, Lp, ij, j, j1, i1, Kp;
  double Pf, Cxp;
  for (Id = 2; Id < NP1+1; Id++)
    {
    Lch = Id * (Id-1)/2;
    for (k = 0; k < *Nd; k++)
      {
      PP[k] = 0.;
      k1 = Lch + k;
      Cx[k1] = 1.;
      Cy[k1] = 1.;
      }
    G[Id-1] = 0.;
    Nst = Id - 1;
    Lck = Lch + *Nd - 1;
    for (i = 0; i < Nst; i++) 
      {
       jb = *Nd - i - 1;
       G[jb] = G[jb-1] + G[jb];
       Nmu = *Nd - i - 1;
       for (l = 0; l < Nmu; l++)
         {
         l1 = Lch + l;
         l2 = Lck - l;
         Cx[l1] *= a12;
         Cx[l2] *= a11;
         Cy[l1] *= a22;
         Cy[l2] *= a21;
         }
      }
    for (k = 0; k < *Nd; k++)
      {
      k1 = Lch + k;
      Cx[k1] *= G[k];
      Cy[k1] *= G[k];
      }
    Nid = NP1 - Id;
    for (l = 0; l < *Nd; l++)
      {
      Lx = l + 1;
      Ly = *Nd - l;
      Lcx = Lx*(Lx-1)/2;
      Lcy = Ly*(Ly-1)/2;
      Lp = (l+1)*(NP2 - (l+1) + 1)/2 - Nid - 1;
      Pf = Coeff[Lp];
      for (i = 0; i < Lx; i++)
        {
        i1 = Lcx + i;
        Cxp = Cx[i1] * Pf;
        for (j = 0; j < Ly; j++)
          {
          ij = i + j;
          j1 = Lcy + j;
          PP[ij] = PP[ij] + Cxp * Cy[j1];
          }
        }
      }
    for (k = 0; k < *Nd; k++)
      {
      Kp = (k+1) * (NP2-(k+1)+1)/2 - Nid - 1;
      Coeff[Kp] = PP[k];
      }
    }
  }
//============================================================================
void XYPolynom :: Derivation(double X, double Y, double &Fx, double &Fy)
  {
  int i, iX, iY, j;
  double  X1, Y1;
  Fx = 0.;
  Fy = 0.;
  if (NPow == -1)
    return;
  iY = 0;
  iX = NPow;
  X1 = 1.;
  for (i = 0; i < NPow; i++)
    {
    iY++;
    if (i) X1 *= X;
    Y1 = 1.;
    for (j = 0; j < NPow - i; j++)
      {
      if (j) Y1 *= Y;
      iX++;
      Fy = Fy + (j + 1) * Coeff[iY] * X1 * Y1;
      Fx = Fx + (i + 1) * Coeff[iX] * X1 * Y1;
      iY++;
      }
    }
  }
//============================================================================
void XYPolynom :: Delete()
  {
  NPow = -1;
  Coeff.RemoveAll();
  }
//============================================================================
void XYPolynom :: Clear()
  {
  NPow = -1;
  Coeff.RemoveAll();
  }
//============================================================================
XYPolynom operator+ (const XYPolynom &A, const XYPolynom &B)
  {
  XYPolynom C, A1, B1;
  int i;
  int NPowA = A.GetPow();
  int NPowB = B.GetPow();

  if  (NPowA == -1)
    return B;
  else if  (NPowB == -1)
    return A;

  if (NPowA == NPowB)
    {
    C = XYPolynom(NPowA);
    for (i = 0; i < A.GetSize(); i++)
      C[i] = A[i] + B[i];
    }
  else if (NPowA > NPowB)
    {
    B1 = B;
    B1.ChangePow(NPowA);
    C = XYPolynom(NPowA);
    for (i = 0; i < A.GetSize(); i++)
      C[i] = A[i] + B1[i];
    }
  else if (NPowA < NPowB)
    {
    A1 = A;
    A1.ChangePow(NPowB);
    C = XYPolynom(NPowB);
    for (i = 0; i < B.GetSize(); i++)
      C[i] = A1[i] + B[i];
    }
  return C;
  }
//============================================================================
XYPolynom operator- (const XYPolynom &A, const XYPolynom &B)
  {
  XYPolynom C;
  C = A + (-1.) * B;
  return C;
  }
//============================================================================
XYPolynom operator* (const XYPolynom &A, double C)
  {
  if (A.NPow == -1)
    return A;
  XYPolynom B;
  B = XYPolynom(A);
  int i;
  for (i = 0; i < A.GetSize(); i++)
    B[i] = A[i] * C;
  return B;
  }
//============================================================================
XYPolynom operator* (double C, const XYPolynom &A)
  {
  if (A.NPow == -1)
    return A;
  XYPolynom B;
  B = XYPolynom(A);
  int i;
  for (i = 0; i < A.GetSize(); i++)
    B[i] = A[i] * C;
  return B;
  }
//============================================================================
XYPolynom operator/ (const XYPolynom &A, double C)
  {
  if (A.NPow == -1)
    return A;
  XYPolynom B;
  double C1 = 1. / C;
  B = C1 * A;
  return B;
  }
//============================================================================

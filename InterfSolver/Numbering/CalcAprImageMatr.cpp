#include <math.h>
#include "Include\Int_Cons.h"
#include "Tools\XYPoint.h"
#include "Tools\isPupil.h"
#include "Tools\CalcLimits.h"
#include "CalcAprImageMatr.h"

//=========================================================================
BOOL CalcPolAprImageMatr(NUMBERING_INTERFEROGRAM_INFO &IntInfo, int MaxPow, double Eps,
                                                                               Matrix &MApr)
  {
  double Xc, Yc, Rad;
  SAMPLE_DATA Sampl;

  Sampl = IntInfo.DigitDat;

  MApr.Clear();

  CalcBoundCircle(IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg, Xc, Yc, Rad);

  Sampl.Normalization(Xc, Yc, Rad);

  XYPolynom RPol;

  PolApproximation PolAppr;
  double Fish = 1;
  CArrayDouble ArrRMS, ArrDisp;
  PolAppr.Set(MaxPow, Eps, Fish, Sampl);
  if (!PolAppr.Calc(RPol, ArrRMS, ArrDisp))
    return FALSE;

  double Xn = 0.;
  double Yn = 0.;
  int Nx, Ny;
  Nx = IntInfo.ImageSize[0];
  Ny = IntInfo.ImageSize[1];
  MApr.SetSize(Ny, Nx);
  double dX = 1.;
  double dY = 1.;
  int i, j;
  double X, Y, XNorm, YNorm;
  XYPoint P;
  bool isInPupil;
  for (i = 0; i < Ny; i++)
    {
    Y = Yn + i * dY;  
    for (j = 0; j < Nx; j++)
      {
      X = Xn + j * dX;
      P = XYPoint(X, Y);
      isInPupil = isPupil(P, IntInfo.ArrEll, IntInfo.ArrRect, IntInfo.ArrPlg);
      if (isInPupil)
        {
        XNorm = (X - Xc) / Rad;
        YNorm = (Y - Yc) / Rad;
        MApr(i,j) = RPol.CalcValue(XNorm, YNorm);
        }
      else
        MApr(i,j) =  E18;
      }  
    }
  return TRUE;
  }
//=========================================================================


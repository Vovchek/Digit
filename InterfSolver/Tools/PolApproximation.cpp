#include <math.h>
#include "PolApproximation.h"
//============================================================================
PolApproximation :: PolApproximation ()
  {
  }
//============================================================================
PolApproximation :: ~PolApproximation()
  {
  Clear();
  }
//============================================================================
void PolApproximation :: Set(int _MaxPow, double _Epst, double _Fish, const SAMPLE_DATA &_Sampl)
  {
  Clear();
  MaxPow = _MaxPow;
  Epst = _Epst;
  Fish = _Fish;
  Sampl = _Sampl;
  PP = 2 * MaxPow + 1;
  PP1 = (PP + 1) * 2;
  PH = MaxPow + 1;
  PH1 = (PH + 1) * 2;
  PH2 = MaxPow + 2;
  SumFXY.SetSize((MaxPow + 1) * (MaxPow + 2) / 2);
  SumXY.SetSize(PP * (PP + 1) / 2);
  }
//============================================================================
bool PolApproximation::Calc(XYPolynom &Pol, CArrayDouble &ArrRMS, CArrayDouble &ArrDisp)
  {
  double PreRMS = 1.e10;
  double PreDisp = 1.e10;
  XYPolynom PrePol;
  int iPow;
  double RMS, Disp;
  Pol.Clear();
  ArrRMS.RemoveAll();
  ArrDisp.RemoveAll();

  CreateMaxMatr();

  for (iPow = 0; iPow <= MaxPow; iPow++)
    {
    CurPol = XYPolynom(iPow);

    if (!CalcCurMatr())
      {
      CurPol = PrePol;
      break;
      }

    CalcRMSDisp(RMS, Disp);

    if (RMS > PreRMS || Disp-PreDisp > Fish)
      {
      CurPol = PrePol;
      break;
      }

    ArrRMS.Add(RMS);
    ArrDisp.Add(Disp);

    if (RMS <= Epst)
      break;

    PreRMS = RMS;
    PreDisp = Disp;
    PrePol = CurPol;
    }
  Pol = CurPol;
  return true;
  }
//============================================================================
void PolApproximation::CreateMaxMatr()
  {
  int i, j, k;

  int NSumXY = PP * (PP + 1) / 2;
  SumXY.SetSize(NSumXY);
  for (i = 0; i < NSumXY; i++)
    SumXY[i] = 0.;

  int NSumFXY = (MaxPow +1 ) * (MaxPow + 2) / 2;
  SumFXY.SetSize(NSumFXY);
  for (i=0; i < NSumFXY; i++)
    SumFXY[i] = 0.;

  double Xs, Ys, Ts;
  int jk, lp, lh;
  int NPnt = Sampl.GetSize();
  for (k = 0; k < NPnt; k++)
    {
    jk = PP; //PP-Number of items in matrix row (col)
    lp = 0;
    lh = 0;
    Xs = 1.;
    for (i = 0; i < PP; i++)
      {
      Ys = 1.;
      for (j = 0; j < jk; j++)
        {
        Ts = Xs * Ys;
        SumXY[lp] += Ts;
        lp++;
        if (i + j + 2 <= PH2)
          {   //PH2 - matrix power + 2
          SumFXY[lh] += (Ts * Sampl[k].F.Number);
          lh++;
          }
        Ys *= Sampl[k].Y;
        }
      jk--;
      Xs *= Sampl[k].X;
      }
    }
  }
//============================================================================
bool PolApproximation :: CalcCurMatr()
// Bilding and deciding norm equalization matrix
  {
  int i, j;
  int NCoeff = CurPol.GetSize();
  AMtr.SetSize(NCoeff, NCoeff);
  Matrix BMtr(NCoeff,1);

//   bilding sums matrix
  int NPow = CurPol.GetPow();
  int NPow1 = NPow + 1;
  int jko = NPow1;
  int l = 0;
  int io, jo, ik, jk, lj;
  for (io = 0; io < NPow1; io++)
    {
    ik = io + 1 + NPow;
    for (jo = 0; jo < jko; jo++)
      {
      jk = jo + 1 + NPow;
      for (i = io; i < ik; i++) 
        {
        for (j = jo; j < jk; j++) 
          {
          lj = i * (PP1 - i - 1) / 2 + j;
          AMtr[l] = SumXY[lj];
          l++;
          }
        jk--;
        }
      }
    jko--;
    }
// building free terms
  jk = NPow1;
  l = 0;
  for (i = 0; i < NPow1; i++)
    {
    for (j = 0; j < jk; j++)
      {
      lj = i * (PH1 - i - 1) / 2 + j;
      BMtr[l] = SumFXY[lj];
      l++;
      }
    jk--;
    }
// norming system equalization
  double RR = 1. / double(Sampl.GetSize());
  for (i = 0; i < NCoeff * NCoeff; i++) 
     AMtr[i] *= RR;
  for (i = 0; i < NCoeff; i++) 
     BMtr[i] *= RR;

// deciding system equalization
  bool isSuccess;
  isSuccess = SystemSolution(AMtr, NCoeff, BMtr, 1);
  if (!isSuccess)
    return false;
  for (i=0; i < NCoeff; i++)
    CurPol[i] = BMtr[i];
  return true;
  }
//============================================================================
void PolApproximation :: CalcRMSDisp(double &RMS, double &Disp)
  {
//  calculating RMS
  int i;
  int NPnt = Sampl.GetSize();
  double dW;
  double Sum = 0.;
  for (i = 0; i < NPnt; i++)
    {
    dW = Sampl[i].F.Number - CurPol.CalcValue(Sampl[i].X, Sampl[i].Y);
    Sum += (dW * dW);
    }
  RMS = sqrt(Sum / double(NPnt));
//  calculating Disp
  Disp = RMS * sqrt(1. / (1. - double(CurPol.GetSize()) / double(NPnt)));
  }
//============================================================================
void PolApproximation :: Clear()
  {
  Sampl.Clear();
  AMtr.Clear();
  SumFXY.RemoveAll();
  SumXY.RemoveAll();
  CurPol.Clear();
  }
//============================================================================

#include <math.h>
#include "Include\Int_Cons.h"
#include "CalcContour.h"

//  void GetContour(const EMem<ELL_LIMITS> &Ell, EMem<XY_CURVE> &ArrCont);
//=========================================================================
void CalcContour(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                   const CArrayXYPolygon &ArrPlg, CArrayXYPolygon &ArrCont, int NPntMax)
  {
  ArrCont.RemoveAll();
  int iElm, iPnt, NPnt;
  CArrayXYBrokenLine ArrBLn;
  XYBrokenLine CurCont;
  XYBrokenLine CurBLn;
  XYPoint P;
 //------------------------------------------------------------------------
  int NEll = ArrEll.GetSize();
  int NRect = ArrRect.GetSize();
  int NPlg = ArrPlg.GetSize();
 //------------------------------------------------------------------------
  double CurPerim, MaxPerim = 0., Step;
  for (iElm = 0; iElm < NEll; iElm++)
    {
    if ((CurPerim = ArrEll[iElm].Perimeter()) > MaxPerim)
      MaxPerim = CurPerim;
    }
  for (iElm = 0; iElm < NRect; iElm++)
    {
    if ((CurPerim = ArrRect[iElm].Perimeter()) > MaxPerim)
	      MaxPerim = CurPerim;
    }
  Step = MaxPerim / NPntMax;
 //------------------------------------------------------------------------
  CurBLn.RemoveAll();
  for (iElm = 0; iElm < NEll; iElm++)
    {
    ArrEll[iElm].GetContour(CurCont, Step);
    NPnt = CurCont.GetSize();
    for (iPnt = 0; iPnt < NPnt; iPnt++)
      {
      P = CurCont[iPnt];
      if (isPupil(P, ArrEll, iElm) && isPupil(P, ArrRect) && isPupil(P, ArrPlg))
        CurBLn.Add(P);
      else if (CurBLn.GetSize() > 0)
        {
        ArrBLn.Add(CurBLn);
        CurBLn.RemoveAll();
        }
      }
    if (CurBLn.GetSize() > 0)
      {
      ArrBLn.Add(CurBLn);
      CurBLn.RemoveAll();
      }
    }
 //------------------------------------------------------------------------
  CurBLn.RemoveAll();
  for (iElm = 0; iElm < NRect; iElm++)
    {
    ArrRect[iElm].GetContour(CurCont, Step);
    NPnt = CurCont.GetSize();
    for (iPnt = 0; iPnt < NPnt; iPnt++)
      {
      P = CurCont[iPnt];
      if (isPupil(P, ArrEll) && isPupil(P, ArrRect, iElm) && isPupil(P, ArrPlg))
        CurBLn.Add(P);
      else if (CurBLn.GetSize() > 0)
        {
        ArrBLn.Add(CurBLn);
        CurBLn.RemoveAll();
        }
      }
    if (CurBLn.GetSize() > 0)
      {
      ArrBLn.Add(CurBLn);
      CurBLn.RemoveAll();
      }
    }
 //------------------------------------------------------------------------
  CurBLn.RemoveAll();
  for (iElm = 0; iElm < NPlg; iElm++)
    {
    CurCont = ArrPlg[iElm];
    NPnt = CurCont.GetSize();
    CurCont.RemoveAt(NPnt - 1);
    NPnt--;
    for (iPnt = 0; iPnt < NPnt; iPnt++)
      {
      P = CurCont[iPnt];
      if (isPupil(P, ArrEll) && isPupil(P, ArrRect) && isPupil(P, ArrPlg, iElm))
        CurBLn.Add(P);
      else if (CurBLn.GetSize() > 0)
        {
        ArrBLn.Add(CurBLn);
        CurBLn.RemoveAll();
        }
      }
    if (CurBLn.GetSize() > 0)
      {
      ArrBLn.Add(CurBLn);
      CurBLn.RemoveAll();
      }
    }
 //------------------------------------------------------------------------
  CurBLn.RemoveAll();
  CurCont.RemoveAll();
  XYPolygon Plg;
 //------------------------------------------------------------------------
  int NBLn = ArrBLn.GetSize();
  if (NBLn == 1)
    {
    Plg = XYPolygon(ArrBLn[0]);
    ArrCont.Add(Plg);
    return; 
    }
 //------------------------------------------------------------------------
  int iBLn;
  int NCur;
  double Eps = 2.5 * Step;
  bool isFind = true;
  XYPoint Pn, Pk;
  while ((NBLn = ArrBLn.GetSize()) > 0)
    {
    CurCont = ArrBLn[0];
    ArrBLn.RemoveAt(0);
    NCur = CurCont.GetSize();
    Pn = CurCont[0];
    Pk = CurCont[NCur-1];
    isFind = true;
    while (isFind)
      {
      NBLn = ArrBLn.GetSize();
      if (NBLn == 0)
        break;
      for (iBLn = 0; iBLn < NBLn; iBLn++)
        {
        NPnt = ArrBLn[iBLn].GetSize();
        if (Distance(Pn, ArrBLn[iBLn][0]) < Eps)
          {
          ArrBLn[iBLn].Inverse();
          CurCont.InsertAt(0, ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          Pn = CurCont[0];
          isFind = true;
          break;
          }
        else if (Distance(Pn, ArrBLn[iBLn][NPnt-1]) < Eps)
          {
          CurCont.InsertAt(0, ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          Pn = CurCont[0];
          isFind = true;
          break;
          }
        else if (Distance(Pk, ArrBLn[iBLn][0]) < Eps)
          {
          CurCont.Append(ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          NCur = CurCont.GetSize();
          Pk = CurCont[NCur-1];
          isFind = true;
          break;
          }
        else if (Distance(Pk, ArrBLn[iBLn][NPnt-1]) < Eps)
          {
          ArrBLn[iBLn].Inverse();
          CurCont.Append(ArrBLn[iBLn]);
          ArrBLn.RemoveAt(iBLn);
          NCur = CurCont.GetSize();
          Pk = CurCont[NCur-1];
          isFind = true;
          break;
          }
        isFind = false;
        }
      }
    if (CurCont.GetSize() > 0)
      {
      Plg = XYPolygon(CurCont);
      ArrCont.Add(Plg);

      }
    }
 //------------------------------------------------------------------------
  int NCont = ArrCont.GetSize();
  if (NCont == 1)
    {
    ArrCont[0].SetTypeLimits(EXTERNAL);
    return;
    }
  int iExt, i;
  bool isIns;
  for (iElm = 0; iElm < NCont; iElm++)
    {
    P = ArrCont[iElm][0];
    for (i = 0; i < NCont; i++)
      {
      if (i == iElm)
        continue;
      if (isIns = ArrCont[i].isInside(P))
        break;
      }
    if (!isIns)
      {
      iExt = iElm;
      break;
      }
    }
  for (iElm = 0; iElm < NCont; iElm++)
    {
    if (iElm == iExt)
      ArrCont[iElm].SetTypeLimits(EXTERNAL);
    else
      ArrCont[iElm].SetTypeLimits(INTERNAL);
    }
  }
//=========================================================================

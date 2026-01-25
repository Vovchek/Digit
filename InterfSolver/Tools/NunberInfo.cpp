#include "Sort.h"
#include "NunberInfo.h"

//============================================================================
NUMBERING_INTERFEROGRAM_INFO :: NUMBERING_INTERFEROGRAM_INFO()
  {
  Title = "";
  ScaleFactor = 1.;
  FiScan = 0.;
  ImageSize[0] = 512; 
  ImageSize[1] = 512;
  ImageFileName = "";
  }
//============================================================================
NUMBERING_INTERFEROGRAM_INFO :: NUMBERING_INTERFEROGRAM_INFO
                                                     (const NUMBERING_INTERFEROGRAM_INFO &A)
  {
  Title = A.Title;
  ScaleFactor = A.ScaleFactor;
  FiScan = A.FiScan;
  ArrEll.Copy(A.ArrEll);
  ArrRect.Copy(A.ArrRect); 
  ArrPlg.Copy(A.ArrPlg);
  EBnd = A.EBnd;
  DigitDat = A.DigitDat;
  ImageSize[0] = A.ImageSize[0]; 
  ImageSize[1] = A.ImageSize[1];
  ImageFileName = A.ImageFileName;
  }
//============================================================================
NUMBERING_INTERFEROGRAM_INFO :: ~NUMBERING_INTERFEROGRAM_INFO()
  {
  Clear();
  }
//============================================================================
NUMBERING_INTERFEROGRAM_INFO&  NUMBERING_INTERFEROGRAM_INFO :: operator= 
                                                      (const NUMBERING_INTERFEROGRAM_INFO &A)
  {
  Title = A.Title;
  ScaleFactor = A.ScaleFactor;
  FiScan = A.FiScan;
  ArrEll.Copy(A.ArrEll);
  ArrRect.Copy(A.ArrRect); 
  ArrPlg.Copy(A.ArrPlg);
  EBnd = A.EBnd;
  DigitDat = A.DigitDat;
  ImageSize[0] = A.ImageSize[0]; 
  ImageSize[1] = A.ImageSize[1];
  ImageFileName = A.ImageFileName;
  return *this;
  }
//============================================================================
void NUMBERING_INTERFEROGRAM_INFO :: Clear()
  {
  Title = "";
  ScaleFactor = 1.;
  FiScan = 0.; 
  ArrEll.RemoveAll();
  ArrRect.RemoveAll(); 
  ArrPlg.RemoveAll();
  EBnd.Clear();
  DigitDat.Clear();
  ImageSize[0] = 512; 
  ImageSize[1] = 512;
  ImageFileName = "";
  }
//============================================================================
void NUMBERING_INTERFEROGRAM_INFO :: GetFringeNumbers(CArrayDouble &ArrFNumbers)
  {
  int i, k;
  ArrFNumbers.RemoveAll();

  int NPnt = DigitDat.GetSize();

  CArrayInt Ind;
  Ind.SetSize(NPnt);
  for (i = 0; i < NPnt; i++)
    Ind[i] = 1;

  double Fo = DigitDat[0].F.Number;
  int in = 1;
  while (in < NPnt)
    {
    for (i = in; i < NPnt; i++)
      {
      if (Ind[i] == 0)
        continue;
      if (fabs(Fo - DigitDat[1].F.Number) < PRECISION)
        Ind[i] = 0;
      }
    for (i = in; i < NPnt; i++)
      {
      if (Ind[i] == 0)
        continue;
      Fo = DigitDat[i].F.Number;
      in = i + 1;
      break;
      }
    if (i == NPnt)
      break;
    }

  int NFrn = 0;
  for (i = 0; i < NPnt; i++)
    {
    if (Ind[i] > 0)
      NFrn++;
    }

  ArrFNumbers.SetSize(NFrn);
  k = 0;
  for (i = 0; i < NPnt; i++)
    {
    if (Ind[i] == 0)
      continue;
    ArrFNumbers[k++] = DigitDat[i].F.Number;
    }
  SortIncrease(ArrFNumbers);
  }
//============================================================================

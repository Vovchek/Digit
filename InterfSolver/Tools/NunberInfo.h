#ifndef __NUMBERINFO_H
#define __NUMBERINFO_H

#include "include\CArrDef.h"
#include "include\Int_Cons.h"
#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYBounds.h"
#include "XYRect.h"
#include "SamplDat.h"

struct NUMBERING_INTERFEROGRAM_INFO
  {
  CString Title;
  CString Date;
  CString Time;
  double ScaleFactor;
  double FiScan;
  CArrayXYEllipse ArrEll;
  CArrayXYRect ArrRect;
  CArrayXYPolygon ArrPlg;
  XYBounds EBnd;
  SAMPLE_DATA DigitDat;
  int ImageSize[2];
  CString ImageFileName;

  NUMBERING_INTERFEROGRAM_INFO();
  NUMBERING_INTERFEROGRAM_INFO(const NUMBERING_INTERFEROGRAM_INFO &A);
  ~NUMBERING_INTERFEROGRAM_INFO();
  NUMBERING_INTERFEROGRAM_INFO& operator= (const NUMBERING_INTERFEROGRAM_INFO &A);
  void Clear();
  void GetFringeNumbers(CArrayDouble &ArrFNumbers);
  };
#endif

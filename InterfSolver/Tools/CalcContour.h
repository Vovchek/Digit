#ifndef _CALCCONTOUR_H_
#define _CALCCONTOUR_H_

#include "include\CArrDef.h"
#include "Include\Int_Cons.h"
#include "isPupil.h"

  void CalcContour(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
           const CArrayXYPolygon &ArrPlg, CArrayXYPolygon &ArrCont, int NPntNax = N_CONT);
//double GetContour() const;
#endif

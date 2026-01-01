#ifndef _CALCCONTOUR_H_
#define _CALCCONTOUR_H_

#include "InterfSolver\include\CArrDef.h"
#include "InterfSolver\Include\Int_Cons.h"
#include "isPupil.h"

  void CalcContour(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
           const CArrayXYPolygon &ArrPlg, CArrayXYPolygon &ArrCont, int NPntNax = N_CONT);

  void ConnectSegments(const CArrayXYBrokenLine& ArrBLn, CArrayXYPolygon& ArrCont, double Eps);

#endif

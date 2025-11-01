#ifndef _ISPUPIL_H_
#define _ISPUPIL_H_
#include "include\CArrDef.h"
#include "XYBrokenLine.h"
#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYBounds.h"
#include "XYRect.h"
  bool isPupil(const XYPoint &P, const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect, 
                                                               const CArrayXYPolygon &ArrPlg);
  bool isPupil(const XYPoint &P, const CArrayXYEllipse &ArrEll);
  bool isPupil(const XYPoint &P, const CArrayXYRect &ArrRect);
  bool isPupil(const XYPoint &P, const CArrayXYPolygon &ArrPlg);
  bool isPupil(const XYPoint &P, const CArrayXYEllipse &ArrEll, int ExceptElm);
  bool isPupil(const XYPoint &P, const CArrayXYRect &ArrRect, int ExceptElm);
  bool isPupil(const XYPoint &P, const CArrayXYPolygon &ArrPlg, int ExceptElm);
#endif

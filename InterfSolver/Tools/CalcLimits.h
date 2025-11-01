#ifndef _CALCLIMITS_H_
#define _CALCLIMITS_H_
#include "include\CArrDef.h"
#include "XYBrokenLine.h"
#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYBounds.h"
#include "XYRect.h"
#include "CalcContour.h"
  bool CalcCircleLimits(const XYBrokenLine &BLine, int TypeLimits, XYEllipse &Ell,
                                                                          XYBounds &Bnd);
  bool CalcCircleLimits(const XYBounds &SBnd, int TypeLimits, XYEllipse &Ell, XYBounds &Bnd);
  bool CalcEllipseLimits(const XYBrokenLine &BLine, int TypeLimits, XYEllipse &Ell,
                                                                          XYBounds &Bnd);
  bool CalcEllipseLimits(const XYBounds &Bnd, int TypeLimits, XYEllipse &Ell);
  bool CalcRectangleLimits(const XYBrokenLine &BLine, int TypeLimits, XYRect &Rect, 
                                                                          XYBounds &Bnd);
  bool CalcRectangleLimits(const XYBounds &Bnd, int TypeLimits, XYRect &Rect);
  bool CalcPolygonLimits(const XYBrokenLine &BLine, int TypeLimits, XYPolygon &Plg);
  bool CalcBounds(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                                            const CArrayXYPolygon &ArrPlg, XYBounds &Bnd);
  bool CalcBoundCircle(const CArrayXYEllipse &ArrEll, const CArrayXYRect &ArrRect,
                       const CArrayXYPolygon &ArrPlg, double &Xc,  double &Yc, double &Rad);
  bool CalcBoundCircle(const XYBounds &Bnd, double &Xc,  double &Yc, double &Rad);
#endif

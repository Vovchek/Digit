#ifndef _XYRECT_H_
#define _XYRECT_H_

#include "XYPoint.h"
#include "XYBounds.h"
#include "XYPolygon.h"


class XYRect 
  {
  public:
    double Ax;
    double By;
    double Xc;
    double Yc;
    double Fi;
    int TypeLimits;
    int TypeSystCoor;
    double Si;
    double Co;
  protected:     
  public:
    XYRect(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
                  double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYRect(const XYRect &A);
    XYRect(const XYBounds &Bnd, int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    void Set(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
                  double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    ~XYRect();
    XYRect& operator= (const XYRect &A);
    double Perimeter() const;
    bool isInside(const XYPoint &P) const;
    bool isInside(double X, double Y);
    bool isVisible(const XYPoint &P) const;
    bool isVisible(double X, double Y);
    bool GetContour(XYBrokenLine &BLine, int NFi) const;
    bool GetContour(XYBrokenLine &BLine, double Step) const;
    bool GetContour(XYPolygon &Plg, int NFi);
    bool GetContour(XYPolygon &Plg, double Step);
    void Normalize(double Xo, double Yo, double Ro);
    friend bool isInside(const XYRect &Rect, const XYPoint &P);
    friend bool isVisible(const XYRect &Rect, const XYPoint &P);
    friend void GetContour(const XYRect &Rect, XYPolygon &Plg, int NFi = N_CONT);
  };
#endif

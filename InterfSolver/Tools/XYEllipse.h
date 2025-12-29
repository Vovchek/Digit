#ifndef _XYELLIPSE_H_
#define _XYELLIPSE_H_

#include "XYPoint.h"
#include "XYBounds.h"
#include "XYPolygon.h"
#include "XYBrokenLine.h"
#include <vector>

class XYEllipse 
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
  public:
    XYEllipse(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
                  double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYEllipse(const XYEllipse &A);
    XYEllipse(const XYBounds &Bnd, int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYEllipse(const std::vector<XYPoint> &points, int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    void Set(double _Ax = 1., double _By = 1., double _Xc = 0., double _Yc = 0.,
                  double _Fi = 0., int _TypeLimits = EXTERNAL, int _TypeSystCoor = MEASURING);
    ~XYEllipse();
    XYEllipse& operator= (const XYEllipse &A);
    double Perimeter() const;
    bool isInside(const XYPoint &P) const;
    bool isInside(double X, double Y);
    bool isVisible(const XYPoint &P) const;
    bool isVisible(double X, double Y);
    bool GetContour(XYBrokenLine& BLine, int NFi) const;
    bool GetContour(XYBrokenLine &BLine, double Step) const;
    bool GetContour(XYPolygon& Plg, int NFi) const;
    bool GetContour(XYPolygon &Plg, double Step) const;
    void GetExtents(double &Xmin, double &Ymin, double &Xmax, double &Ymax) const;
    void InverseY(double YcInv);
    void ShiftX(double dX);
    void ShiftY(double dY);
    void Normalize(double Xo, double Yo, double Ro);
    void DeNormalize(double Xo, double Yo, double Ro);
    friend bool isInside(const XYEllipse &Ell, const XYPoint &P);
    friend bool isVisible(const XYEllipse &Ell, const XYPoint &P);
    friend void GetContour(const XYEllipse &Ell, XYPolygon &Plg, int NFi = N_CONT);
  };
#endif

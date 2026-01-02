#ifndef _XYELLIPSE_H_
#define _XYELLIPSE_H_

#include "XYShape.h"      // NEW: Include base class
#include "XYPoint.h"
#include "XYBounds.h"
#include <vector>

// Forward declarations
class XYPolygon;
class XYBrokenLine;

class XYEllipse : public XYShape  // NEW: Inherit from XYShape
  {
  public:
    // Shape-specific parameters (not in base class)
    double Ax;
    double By;
    double Xc;
    double Yc;
    double Fi;
    // REMOVED: int TypeLimits; (now in XYShape base class - PUBLIC)
    // REMOVED: int TypeSystCoor; (now in XYShape base class - PUBLIC)
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
    virtual ~XYEllipse();  // NEW: virtual destructor
    XYEllipse& operator= (const XYEllipse &A);
    
    // REMOVED: SetTypeLimits, GetTypeLimits, SetTypeSystCoor, GetTypeSystCoor
    // (inherited from XYShape base class)
    
    // Pure virtual implementations from XYShape (override keyword added)
    virtual double Perimeter() const override;
    virtual bool isInside(const XYPoint &P) const override;
    virtual bool isInside(double X, double Y) override;
    virtual bool GetContour(XYBrokenLine &BLine, int NFi) const override;
    virtual bool GetContour(XYBrokenLine &BLine, double Step) const override;
    virtual bool GetContour(XYPolygon &Plg, int NFi) override;      // Non-const to match base
    virtual bool GetContour(XYPolygon &Plg, double Step) override;  // Non-const to match base
    virtual void Normalize(double Xo, double Yo, double Ro) override;
    virtual void DeNormalize(double Xo, double Yo, double Ro) override;
    virtual XYBounds GetBounds() const override;
    virtual void InverseY(double YcInv) override;
    virtual void ShiftX(double dX) override;
    virtual void ShiftY(double dY) override;

    // REMOVED: isVisible() methods - using XYShape base class implementation
    
    // Shape-specific methods (not virtual)
    
    // DEPRECATED: Friend functions - these have incorrect isVisible logic!
    // Use member functions instead: ellipse.isInside(pt), ellipse.isVisible(pt)
    friend bool isInside(const XYEllipse &Ell, const XYPoint &P);
    friend void GetContour(const XYEllipse &Ell, XYPolygon &Plg, int NFi = N_CONT);
  };
#endif

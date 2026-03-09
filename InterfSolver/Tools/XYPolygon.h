#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "XYBrokenLine.h"
#include "XYBounds.h"
#include "XYShape.h"      // NEW: Include XYShape base class

// Multiple inheritance: XYBrokenLine for point storage, XYShape for type management
class XYPolygon : public XYBrokenLine, public XYShape  // NEW: Multiple inheritance
  {
  public:
    XYPolygon();
    XYPolygon(const XYPolygon &A);
    XYPolygon(const XYBrokenLine &A, int TypLim = EXTERNAL, int _TypeSystCoor = MEASURING);
    XYPolygon(int NPnt, double *pArrX, double *pArrY, int TypLim = EXTERNAL,
                                                               int _TypeSystCoor = MEASURING);
    XYPolygon(int NPnt, int *pArrX, int *pArrY, int TypLim = EXTERNAL,
                                                              int _TypeSystCoor = MEASURING);
    XYPolygon(const CArrayDouble &ArrX, const CArrayDouble &ArrY, int TypLim = EXTERNAL,
                                                               int _TypeSystCoor = MEASURING);
    XYPolygon(const CArrayDouble &ArrXY, int TypLim = EXTERNAL, int _TypeSystCoor = MEASURING);
    virtual ~XYPolygon();  // NEW: virtual destructor
    
    XYPolygon& operator= (const XYPolygon &A);
    XYPolygon& operator= (const XYBrokenLine& A);
    XYPolygon& operator= (const CArrayXYPoint &A);
    
    // Pure virtual implementations from XYShape (override keyword added)
    virtual double Perimeter() const override;
    virtual bool isInside(const XYPoint &P) const override;
    virtual bool isInside(double X, double Y) const override;
    virtual void Normalize(double Xo, double Yo, double Ro) override;
    virtual void DeNormalize(double Xo, double Yo, double Ro) override;
    virtual XYBounds GetBounds() const override;
    virtual void InverseY(double YcInv) override;
    virtual void ShiftX(double dX) override;
    virtual void ShiftY(double dY) override;

    // XYShape requires GetContour methods - XYPolygon IS a contour, so provide stubs
    // These are not used for XYPolygon but required by XYShape interface
    virtual bool GetContour(XYBrokenLine &BLine, int NFi) const override {
        BLine = *this;  // Polygon is already a broken line
        return true;
    }
    virtual bool GetContour(XYBrokenLine &BLine, double Step) const override {
        BLine = *this;  // Polygon is already a broken line
        return true;
    }
    virtual bool GetContour(XYPolygon &Plg, int NFi) override {
        Plg = *this;  // Polygon is already a polygon
        return true;
    }
    virtual bool GetContour(XYPolygon &Plg, double Step) override {
        Plg = *this;  // Polygon is already a polygon
        return true;
    }

    // XYPolygon-specific methods (not in XYShape)
    double Area() const;
    bool isDegenerate() const;
    XYPoint GetCentroid();

  };
#endif

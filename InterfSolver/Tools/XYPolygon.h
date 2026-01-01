#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "XYBrokenLine.h"
#include "XYBounds.h"

class XYPolygon : public XYBrokenLine
  {
  protected:
    int TypeLimits;
    int TypeSystCoor;
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
    ~XYPolygon();
    void SetTypeLimits (int TypLim) {TypeLimits = TypLim;}
    int GetTypeLimits () {return TypeLimits;}
    void SetTypeSystCoor (int _TypeSystCoor) {TypeSystCoor = _TypeSystCoor;}
    int GetTypeSystCoor () {return TypeSystCoor;}
    XYPolygon& operator= (const XYPolygon &A);
    XYPolygon& operator= (const XYBrokenLine &A);
    XYPolygon& operator= (const CArrayXYPoint &A);
    double Perimeter() const;
    double Area() const;
    bool isDegenerate() const;
    bool isInside(const XYPoint &P) const;
    bool isInside(double X, double Y);
    bool isVisible(const XYPoint &P) const;
    bool isVisible(double X, double Y);
    void Normalize(double Xo, double Yo, double Ro);
    XYBounds GetBounds();
    void GetBounds(XYBounds &Bnd);
    XYPoint GetCentroid();

    friend bool isInside(const XYPolygon &Plg, const XYPoint &P);
    friend bool isVisible(const XYPolygon &Plg, const XYPoint &P);
    friend XYBounds GetBounds(const XYPolygon &Plg);
    friend void GetBounds(const XYPolygon &Plg, XYBounds &Bnd);
  };
#endif

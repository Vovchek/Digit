#include "XYPolygon.h"
//=========================================================================
// Constructor - call both base class constructors
//=========================================================================
XYPolygon :: XYPolygon()
  : XYBrokenLine(),  // Initialize XYBrokenLine base
    XYShape(EXTERNAL, MEASURING)  // NEW: Initialize XYShape base
  {
  }
//=========================================================================
// Copy constructor - call both base class constructors
//=========================================================================
XYPolygon :: XYPolygon(const XYPolygon &A) 
  : XYBrokenLine(A),  // Initialize XYBrokenLine base
    XYShape(A.TypeLimits, A.TypeSystCoor)  // NEW: Initialize XYShape base
  {
  }
//=========================================================================

XYPolygon :: XYPolygon(const XYBrokenLine &A, int TypLim, int _TypeSystCoor) : 
  XYBrokenLine(A),  // Initialize XYBrokenLine base
  XYShape(TypLim, _TypeSystCoor)  // NEW: Initialize XYShape base
  {
  int NPnt = A.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  }
//=========================================================================
XYPolygon :: XYPolygon(int NPnt, double *pArrX, double *pArrY, int TypLim, 
                                       int _TypeSystCoor) 
  : XYBrokenLine(NPnt, pArrX, pArrY),  // Initialize XYBrokenLine base
    XYShape(TypLim, _TypeSystCoor)  // NEW: Initialize XYShape base
  {
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  }
//=========================================================================
XYPolygon :: XYPolygon(int NPnt, int *pArrX, int *pArrY, int TypLim, int _TypeSystCoor) : 
  XYBrokenLine(NPnt, pArrX, pArrY),  // Initialize XYBrokenLine base
  XYShape(TypLim, _TypeSystCoor)  // NEW: Initialize XYShape base
  {
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  }
//=========================================================================
XYPolygon :: XYPolygon(const CArrayDouble &ArrX, const CArrayDouble &ArrY, int TypLim,
                                                int _TypeSystCoor) 
  : XYBrokenLine(ArrX, ArrY),  // Initialize XYBrokenLine base
    XYShape(TypLim, _TypeSystCoor)  // NEW: Initialize XYShape base
  {
  int NPnt = ArrX.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  }
//=========================================================================
XYPolygon :: XYPolygon(const CArrayDouble &ArrXY, int TypLim, int _TypeSystCoor) :
  XYBrokenLine(ArrXY),  // Initialize XYBrokenLine base
  XYShape(TypLim, _TypeSystCoor)  // NEW: Initialize XYShape base
  {
  int NPnt = ArrPnt.GetSize();
  if (Distance(ArrPnt[0], ArrPnt[NPnt-1]) > PRECISION)
    ArrPnt.Add(ArrPnt[0]);
  }
//=========================================================================
XYPolygon :: ~XYPolygon()
  {
  }
//=========================================================================
// Assignment operator - copy both base class members
//=========================================================================
XYPolygon& XYPolygon :: operator= (const XYPolygon &A)
  {
  if (this != &A) {
    ArrPnt.Copy(A.ArrPnt);
    // Copy XYShape base class members
    TypeLimits = A.TypeLimits;
    TypeSystCoor = A.TypeSystCoor;
  }
  return *this;
  }
//=========================================================================
XYPolygon& XYPolygon :: operator= (const CArrayXYPoint &A)
  {
  auto Na = A.GetSize();
  ArrPnt.SetSize(Na);
  for (auto i = 0; i < Na; i++)
    ArrPnt[i] = A[i];
  if (Distance(ArrPnt[0], ArrPnt[Na - 1]) > PRECISION)
      ArrPnt.Add(ArrPnt[0]);
  // Reset to defaults
  TypeLimits = EXTERNAL;
  TypeSystCoor = MEASURING;
  return *this;
  }
//=========================================================================
XYPolygon& XYPolygon :: operator= (const XYBrokenLine& A)
{
    auto Na = A.GetSize();
    ArrPnt.SetSize(Na);
    for (auto i = 0; i < Na; i++)
        ArrPnt[i] = A[i];
    if (Distance(ArrPnt[0], ArrPnt[Na - 1]) > PRECISION)
        ArrPnt.Add(ArrPnt[0]);
    // Reset to defaults
    TypeLimits = EXTERNAL;
    TypeSystCoor = MEASURING;
    return *this;
}
//=========================================================================
double XYPolygon :: Perimeter() const
  {
  int i;
  double Perim = 0.;
  int NPnt = GetSize();
  for (i = 0; i < NPnt-1; i++)
    Perim += Distance(ArrPnt[i], ArrPnt[i]);
  return Perim;

  }
//=========================================================================
double XYPolygon :: Area() const
  {
  int NPnt = GetSize();
  if (NPnt < 3)
    return 0.0;
    
  // Use shoelace formula (Gauss's area formula)
  // Area = 0.5 * |sum((x[i] * y[i+1]) - (x[i+1] * y[i]))|
  double area = 0.0;
  
  for (int i = 0; i < NPnt - 1; i++)
    {
    area += ArrPnt[i].X * ArrPnt[i + 1].Y;
    area -= ArrPnt[i + 1].X * ArrPnt[i].Y;
    }
  
  return fabs(area) * 0.5;
  }
//=========================================================================
bool XYPolygon :: isDegenerate() const
  {
  int NPnt = GetSize();
  
  // Check for minimum point count
  if (NPnt < 3)
    return true;
    
  // Check for zero area
  double area = Area();
  if (area < PRECISION)
    return true;
    
  // Check if all points are collinear for small polygons
  // For a non-degenerate polygon, at least 3 non-collinear points are needed
  if (NPnt == 3)
    {
    // Check if triangle area is effectively zero
    XYPoint p1 = ArrPnt[0];
    XYPoint p2 = ArrPnt[1];
    XYPoint p3 = ArrPnt[2];
    
    // Cross product magnitude for collinearity test
    double cross = (p2.X - p1.X) * (p3.Y - p1.Y) - 
                   (p2.Y - p1.Y) * (p3.X - p1.X);
    
    if (fabs(cross) < PRECISION)
      return true;
    }
  
  return false;
  }
//=========================================================================
bool XYPolygon :: isInside(const XYPoint &P) const
  {
  int i;
  double Phi = 0.;
  int NPnt = GetSize();
  for (i = 0; i < NPnt - 1; i++)
    Phi += Angle(ArrPnt[i+1]-P, ArrPnt[i]-P);
  if (fabs(Phi) > 6.28)
    return true;
  else if (fabs(Phi) < 0.0001)
    return false;
  return true;
  }
//=========================================================================
bool XYPolygon :: isInside(double X, double Y)
  {
  XYPoint P(X, Y);
  return isInside(P);
  }
//=========================================================================
void XYPolygon :: Normalize(double Xo, double Yo, double Ro)
  {
  int i;
  int NPnt = GetSize();
  for (i = 0; i < NPnt; i++)
    ArrPnt[i].Normalize(Xo, Yo, Ro);
  TypeSystCoor = NORMALISED;  // Base class member
  }
//=========================================================================
void XYPolygon::DeNormalize(double Xo, double Yo, double Ro)
{
  int i;
  int NPnt = GetSize();
  for (i = 0; i < NPnt; i++)
    {
    ArrPnt[i].X = ArrPnt[i].X * Ro + Xo;
    ArrPnt[i].Y = ArrPnt[i].Y * Ro + Yo;
    }
  TypeSystCoor = MEASURING;  // Base class member
}
//=========================================================================
XYBounds XYPolygon::GetBounds() const
{
  auto NPnt = GetSize();
  XYBounds Bnd { -E18, -E18, E18, E18}; // E18 = -1e18
  for (auto i = 0; i < NPnt; i++)
    {
    auto P = ArrPnt[i];
    if (P.X < Bnd.XLeft)
        Bnd.XLeft = P.X;
    if (P.X > Bnd.XRight)
        Bnd.XRight = P.X;
    if (P.Y < Bnd.YTop)
        Bnd.YTop = P.Y;
    if (P.Y > Bnd.YBottom)
        Bnd.YBottom = P.Y;
    }
  return Bnd;
}
//=========================================================================
void XYPolygon::InverseY(double YcInv)
{
  auto NPnt = GetSize();
  for (auto i = 0; i < NPnt; i++)
	  ArrPnt[i].Y = YcInv - ArrPnt[i].Y;
}
//=========================================================================
void XYPolygon::ShiftX(double dX)
{
  auto NPnt = GetSize();
  for (auto i = 0; i < NPnt; i++)
	  ArrPnt[i].X += dX;
}
//=========================================================================
void XYPolygon::ShiftY(double dY)
{
  auto NPnt = GetSize();
  for (auto i = 0; i < NPnt; i++)
	  ArrPnt[i].Y += dY;
}
//=========================================================================
XYPoint XYPolygon :: GetCentroid()
  {
    double area = 0.0;
    double centroid_x = 0.0;
    double centroid_y = 0.0;
    int n = GetSize();

    if (n < 3) {
		// For polygons with less than 3 points, return the average of the points
        XYPoint sum{ 0, 0 };
        for (auto i = 0; i < n; ++i) {
            sum.X += ArrPnt[i].X;
            sum.Y += ArrPnt[i].Y;
        }
        return { sum.X / n, sum.Y / n };
    }

	// Main formula
    for (int i = 0; i < n; ++i) {
		int j = (i + 1) % n; // next vertex index, wrapping around

        double xi = ArrPnt[i].X;
        double yi = ArrPnt[i].Y;
        double xj = ArrPnt[j].X;
        double yj = ArrPnt[j].Y;

        double cross = xi * yj - xj * yi; // (xᵢ*yⱼ - xⱼ*yᵢ)

        area += cross;

        centroid_x += (xi + xj) * cross;
        centroid_y += (yi + yj) * cross;
    }

    area *= 0.5;
    double factor = 1.0 / (6.0 * area);

    centroid_x *= factor;
    centroid_y *= factor;

    return { centroid_x, centroid_y };
  }
//=========================================================================

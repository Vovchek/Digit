#include "XYEllipse.h"
#include "XYPolygon.h"
#include "XYBrokenLine.h"
#include "Matrix.h"
#include "Vector.h"
#include <vector>

//=========================================================================
// Constructor - call base class constructor
//=========================================================================
XYEllipse :: XYEllipse(double _Ax, double _By, double _Xc, double _Yc, double _Fi, 
                                                          int _TypeLimits, int _TypeSystCoor)
  : XYShape(_TypeLimits, _TypeSystCoor),  // NEW: Initialize base class
    Ax(_Ax), By(_By), Xc(_Xc), Yc(_Yc), Fi(_Fi)
  {
  // REMOVED: TypeLimits = _TypeLimits; (now in base class)
  // REMOVED: TypeSystCoor = _TypeSystCoor; (now in base class)
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
// Copy constructor - call base class constructor
//=========================================================================
XYEllipse :: XYEllipse(const XYEllipse &A)
  : XYShape(A.TypeLimits, A.TypeSystCoor),  // NEW: Initialize base class
    Ax(A.Ax), By(A.By), Xc(A.Xc), Yc(A.Yc), Fi(A.Fi), Si(A.Si), Co(A.Co)
  {
  // REMOVED: TypeLimits = A.TypeLimits; (now in base class)
  // REMOVED: TypeSystCoor = A.TypeSystCoor; (now in base class)
  }
//=========================================================================
XYEllipse :: XYEllipse(const XYBounds &Bnd, int _TypeLimits, int _TypeSystCoor)
  : XYShape(_TypeLimits, _TypeSystCoor)  // NEW: Initialize base class
  {
  Xc = (Bnd.XLeft + Bnd.XRight) / 2;
  Yc = (Bnd.YBottom + Bnd.YTop) / 2;
  Ax = fabs((Bnd.XRight - Bnd.XLeft) / 2);
  By = fabs((Bnd.YTop - Bnd.YBottom) / 2);
  Fi = 0.;
  // REMOVED: TypeLimits = _TypeLimits; (now in base class)
  // REMOVED: TypeSystCoor = _TypeSystCoor; (now in base class)
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
void XYEllipse :: Set(double _Ax, double _By, double _Xc, double _Yc, double _Fi, 
                                                          int _TypeLimits, int _TypeSystCoor)
  {
  Ax = _Ax;
  By = _By;
  Xc = _Xc;
  Yc = _Yc;
  Fi = _Fi;
  TypeLimits = _TypeLimits;      // Base class member
  TypeSystCoor = _TypeSystCoor;  // Base class member
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
XYEllipse :: ~XYEllipse()
  {
  }
//=========================================================================
// Assignment operator - copy base class members
//=========================================================================
XYEllipse& XYEllipse ::operator= (const XYEllipse &A)
  {
  if (this != &A) {
    // Copy base class members
    TypeLimits = A.TypeLimits;
    TypeSystCoor = A.TypeSystCoor;
    
    // Copy derived class members
    Ax = A.Ax;
    By = A.By;
    Xc = A.Xc;
    Yc = A.Yc;
    Fi = A.Fi;
    Si = A.Si;
    Co = A.Co;
  }
  return *this;
  }
// =========================================================================
// Comparison operator
// =========================================================================
bool XYEllipse :: operator== (const XYEllipse& A) const
  {
  constexpr double PREC = 1e-3; // Define a precision threshold for comparison
                                // that matches %.3lf formatting for non-normalized
								// and %.6lf for normalized coordinates (considering typical scale factors)
  return (fabs(Ax - A.Ax) < PREC &&
          fabs(By - A.By) < PREC &&
          fabs(Xc - A.Xc) < PREC &&
          fabs(Yc - A.Yc) < PREC &&
          fabs(Fi - A.Fi) < PREC &&
          TypeLimits == A.TypeLimits &&
          TypeSystCoor == A.TypeSystCoor);
}

//=========================================================================
double XYEllipse :: Perimeter() const
  {
	constexpr double dPI = 3.14159265358979323846;
  double Perim = dPI * (1.5 * (Ax + By) - sqrt(Ax * By));
  return Perim;
  }
//=========================================================================
bool XYEllipse :: isInside(const XYPoint &P) const
  {
  double X1 =  (P.X - Xc) * Co + (P.Y - Yc) * Si;
  double Y1 = -(P.X - Xc) * Si + (P.Y - Yc) * Co;
  double R = (X1 * X1 / (Ax * Ax) + Y1 * Y1 / (By * By));
  double T = R - 1.;

  if (T <= HIGH_PRECISION) // give a little allowance for numerical precision
    return true;
  else if (T > 0.)
    return false;

  return true;
  }
//=========================================================================
bool XYEllipse :: isInside(double X, double Y) const
  {
  XYPoint P(X, Y);
  return isInside(P);
  }
//=========================================================================
// REMOVED: isVisible() implementations - using XYShape base class
// The base class template method provides CORRECT implementation:
//   - EXTERNAL shapes (apertures): visible if point is INSIDE
//   - INTERNAL shapes (obstructions): visible if point is OUTSIDE
//=========================================================================
bool XYEllipse :: GetContour(XYBrokenLine &BLine, int NFi) const 
  {
  BLine.RemoveAll();
  double Th, dTh, SiT, CoT, AveR, R;
  int j;
  AveR = fabs(Ax) + fabs(By);
  if (AveR <= PRECISION)
    return false;
  XYPoint P;
  BLine.SetSize(NFi);
  dTh = PI2 / (NFi);
  for (j = 0; j < NFi; j++)
    {
    Th = j * dTh;
    SiT = sin(Th);
    CoT = cos(Th);
    R = Ax * By / sqrt(pow(Ax*SiT,2) + pow(By*CoT,2));
    P.X = Xc + R * (Co * CoT - Si * SiT);
    P.Y = Yc + R * (Si * CoT + Co * SiT);
    BLine[j] = P;
    }
  return true;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYBrokenLine &BLine, double Step) const
  {
    if (Step <= 0.)
        return false;

  double Perim = Perimeter();
  int NFi = static_cast<int>(Perim / Step);
  bool isSuccess = GetContour(BLine, NFi);
  return isSuccess;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYPolygon &Plg, int NFi)  // REMOVED const - match base
  {
  Plg.RemoveAll();
  XYBrokenLine BLine;
  bool isSuccess = GetContour(BLine, NFi);  // Call const version for BrokenLine
  if (isSuccess)
    Plg = XYPolygon(BLine, TypeLimits, TypeSystCoor);
  return isSuccess;
  }
//=========================================================================
bool XYEllipse :: GetContour(XYPolygon &Plg, double Step)  // REMOVED const - match base
  {
  double Perim = Perimeter();
  int NFi = static_cast<int>(Perim / Step);
  bool isSuccess = GetContour(Plg, NFi);
  return isSuccess;
  }
//=========================================================================
XYBounds XYEllipse :: GetBounds() const
  {
  // For a rotated ellipse, the bounding box extents are found by:
  // - Computing the extreme points where dx/dt = 0 and dy/dt = 0
  // - For parametric ellipse: x(t) = Xc + Ax*cos(t)*cos(Fi) - By*sin(t)*sin(Fi)
  //                           y(t) = Yc + Ax*cos(t)*sin(Fi) + By*sin(t)*cos(Fi)
  
  XYBounds Bnd;
  if (fabs(Fi) < PRECISION || fabs(Ax) < PRECISION || fabs(By) < PRECISION)
    {
    // Non-rotated ellipse or degenerate case - simple calculation
    Bnd.XLeft = Xc - Ax;
    Bnd.XRight = Xc + Ax;
    Bnd.YTop = Yc - By;
    Bnd.YBottom = Yc + By;
    }
  else
    {
    // For rotated ellipse, extrema occur at specific angles
    // X extrema: tan(t) = -(By/Ax) * tan(Fi)
    // Y extrema: tan(t) = (By/Ax) * cot(Fi)
    
    double CosSq = Co * Co;
    double SinSq = Si * Si;
    double AxSq = Ax * Ax;
    double BySq = By * By;
    
    // X extents
    double deltaX = sqrt(AxSq * CosSq + BySq * SinSq);
    Bnd.XLeft = Xc - deltaX;
    Bnd.XRight = Xc + deltaX;
    
    // Y extents
    double deltaY = sqrt(AxSq * SinSq + BySq * CosSq);
    Bnd.YTop = Yc - deltaY;
    Bnd.YBottom = Yc + deltaY;
    }
  return Bnd;
  }
//=========================================================================
void XYEllipse :: InverseY(double YcInv)
  {
  Yc = YcInv - Yc;
  Fi = -Fi;
  Si = sin(GRD_RD * Fi);
  Co = cos(GRD_RD * Fi);
  }
//=========================================================================
void XYEllipse :: ShiftX(double dX)
  {
  Xc += dX;
  }
//=========================================================================
void XYEllipse :: ShiftY(double dY)
  {
  Yc += dY;
  }
//=========================================================================
void XYEllipse :: Normalize(double Xo, double Yo, double Ro)
  {
  Ax /= Ro;
  By /= Ro;
  Xc = (Xc - Xo) / Ro;
  Yc = (Yc - Yo) / Ro;
  TypeSystCoor = NORMALISED;
  }
//=========================================================================
void XYEllipse :: DeNormalize(double Xo, double Yo, double Ro)
  {
  Ax *= Ro;
  By *= Ro;
  Xc = Xc * Ro + Xo;
  Yc = Yc * Ro + Yo;
  TypeSystCoor = MEASURING;
  }
//=========================================================================
// DEPRECATED FRIEND FUNCTIONS
// These functions are kept for backward compatibility but should not be used.
// They have INCORRECT isVisible() logic (inverted from correct implementation).
// Use member functions instead: ellipse.isInside(pt), ellipse.isVisible(pt)
//=========================================================================

/**
 * @brief DEPRECATED: Friend function for isInside - use member function instead
 * @deprecated Use ellipse.isInside(P) instead
 * 
 * This friend function is maintained for backward compatibility but duplicates
 * the member function. Prefer using the member function for clarity.
 */
bool isInside(const XYEllipse &Ell, const XYPoint &P)
  {
  double X1 =  (P.X - Ell.Xc) * Ell.Co + (P.Y - Ell.Yc) * Ell.Si;
  double Y1 = -(P.X - Ell.Xc) * Ell.Si + (P.Y - Ell.Yc) * Ell.Co;
  double R = (X1 * X1 / (Ell.Ax * Ell.Ax) + Y1 * Y1 / (Ell.By * Ell.By));
  double T = R - 1.;

  if (T <= HIGH_PRECISION)  // Changed from < to <= to match member function
    return true;
  else if (T > 0.)
    return false;
  return true;
  }

//=========================================================================
/**
 * @brief DEPRECATED: Friend function for GetContour - use member function instead
 * @deprecated Use ellipse.GetContour(Plg, NFi) instead
 */
void GetContour(const XYEllipse &Ell, XYPolygon &Plg, int NFi)
  {
  Plg.RemoveAll();
  double Th, dTh, SiT, CoT, AveR, R;
  int j;
  AveR = fabs(Ell.Ax) + fabs(Ell.By);
  if (AveR <= PRECISION)
    return;
  XYPoint P;
  Plg.SetSize(NFi+1);
  dTh = PI2 / (NFi);
  for (j = 0; j < NFi; j++)
    {
    Th = j * dTh;
    SiT = sin(Th);
    CoT = cos(Th);
    R = Ell.Ax * Ell.By / sqrt(pow(Ell.Ax*SiT,2) + pow(Ell.By*CoT,2));
    P.X = Ell.Xc + R * (Ell.Co * CoT - Ell.Si * SiT);
    P.Y = Ell.Yc + R * (Ell.Si * CoT + Ell.Co * SiT);
    Plg[j] = P;
    }
  Plg[NFi] = Plg[0];

  Plg.SetTypeLimits(Ell.TypeLimits);
  Plg.SetTypeSystCoor(Ell.TypeSystCoor);
  }
//=========================================================================
XYEllipse::XYEllipse(const std::vector<XYPoint>& points, int _TypeLimits, int _TypeSystCoor) :
    XYShape(_TypeLimits, _TypeSystCoor),  // NEW: Initialize base class    
    Ax(0.), By(0.), Xc(0.), Yc(0.), Fi(0.), Si(0.), Co(1.)
{

    size_t n = points.size();

    if (n == 0)
    {
        // all member vars are set in initializer
    }
    else if (n == 1)
    {
        // Set center only, zero radii circle
        Xc = points[0].X;
        Yc = points[0].Y;
    }
    else if (n == 2)
    {
        // Two points define a circle diameter
        Xc = (points[0].X + points[1].X) / 2.;
        Yc = (points[0].Y + points[1].Y) / 2.;
        double R = Distance(points[0], points[1]) / 2.;
        Ax = R;
        By = R;
    }
    else if (n == 3)
    {
        // Three points define a circle
        // Using geometric circle fitting through three points
        double x1 = points[0].X, y1 = points[0].Y;
        double x2 = points[1].X, y2 = points[1].Y;
        double x3 = points[2].X, y3 = points[2].Y;

        double A = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;

        if (fabs(A) < PRECISION)
        {
            // Points are nearly collinear, create largest diameter circle
            auto R12 = Distance(points[0], points[1]) / 2.;
            auto R13 = Distance(points[0], points[2]) / 2.;
            auto R23 = Distance(points[1], points[2]) / 2.;
            if (R12 >= R13 && R12 >= R23) { // 1-2 is best
                Ax = By = R12;
                Xc = (x1 + x2) / 2.;
                Yc = (y1 + y2) / 2.;
            }
            else if (R13 >= R12 && R13 >= R23) { // 1-3 is best
                Ax = By = R13;
                Xc = (x1 + x3) / 2.;
                Yc = (y1 + y3) / 2.;
            }
            else { // 2-3 is best
                Ax = By = R23;
                Xc = (x3 + x2) / 2.;
                Yc = (y3 + y2) / 2.;
            }
        }
        else
        {
            double B = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
            double C = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);

            Xc = -B / (2. * A);
            Yc = -C / (2. * A);

            double R = sqrt((x1 - Xc) * (x1 - Xc) + (y1 - Yc) * (y1 - Yc));
            Ax = R;
            By = R;
        }
    }
    else if (n == 4)
    {
        // Four points define ellipse with axes aligned to reference system
        // Calculate center as average
        Xc = (points[0].X + points[1].X + points[2].X + points[3].X) / 4.;
        Yc = (points[0].Y + points[1].Y + points[2].Y + points[3].Y) / 4.;

        // Find max extents in X and Y directions (axes aligned)
        double maxX = 0., maxY = 0.;
        for (size_t i = 0; i < 4; i++)
        {
            double dx = fabs(points[i].X - Xc);
            double dy = fabs(points[i].Y - Yc);
            if (dx > maxX) maxX = dx;
            if (dy > maxY) maxY = dy;
        }

        Ax = maxX;
        By = maxY;
    }
    else if (n == 5)
    {
        // Five points define exact ellipse (general conic through 5 points)
        // Using algebraic ellipse fitting
        Matrix A(5, 5);
        Vector B(5);

        for (size_t i = 0; i < 5; i++)
        {
            double x = points[i].X;
            double y = points[i].Y;
            A(i, 0) = x * x;
            A(i, 1) = x * y;
            A(i, 2) = y * y;
            A(i, 3) = x;
            A(i, 4) = y;
            B[i] = 1.;
        }

        // Solve system
        if (SystemSolution(A, B))
        {
            double a = B[0];
            double b = B[1] / 2.;
            double c = B[2];
            double d = B[3] / 2.;
            double e = B[4] / 2.;

            // Convert from general conic to ellipse parameters
            double det = b * b - a * c;
            if (fabs(det) < PRECISION || det >= 0.)
            {
                // Not an ellipse, fall back to 4-point method
                std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
                *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
                return;
            }

            Xc = (c * d - b * e) / det;
            Yc = (a * e - b * d) / det;

            double num = 2. * (a * e * e + c * d * d - b * d * e + det);
            double den1 = det * (sqrt((a - c) * (a - c) + 4. * b * b) - (a + c));
            double den2 = det * (-sqrt((a - c) * (a - c) + 4. * b * b) - (a + c));

            if (den1 <= 0. || den2 <= 0.)
            {
                std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
                *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
                return;
            }

            Ax = sqrt(num / den1);
            By = sqrt(num / den2);

            if (fabs(b) < PRECISION)
                Fi = 0.;
            else
                Fi = atan2(c - a - sqrt((a - c) * (a - c) + 4. * b * b), 2. * b) * RD_GRD;

            Si = sin(GRD_RD * Fi);
            Co = cos(GRD_RD * Fi);
        }
        else
        {
            // Fall back to 4-point method
            std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
            *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
        }
    }
    else // n > 5
    {
        // Least squares ellipse fitting
        // Using algebraic distance minimization with constraint
        size_t np = points.size();
        Matrix D(np, 6);

        for (size_t i = 0; i < np; i++)
        {
            double x = points[i].X;
            double y = points[i].Y;
            D(i, 0) = x * x;
            D(i, 1) = x * y;
            D(i, 2) = y * y;
            D(i, 3) = x;
            D(i, 4) = y;
            D(i, 5) = 1.;
        }

        // Form scatter matrix S = D'*D
        Matrix S(6, 6);
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 6; j++)
            {
                double sum = 0.;
                for (size_t k = 0; k < np; k++)
                    sum += D(k, i) * D(k, j);
                S(i, j) = sum;
            }

        // Constraint matrix for ellipse (4*a*c - b*b = 1)
        Matrix C(6, 6);
        for (int i = 0; i < 6; i++)
            for (int j = 0; j < 6; j++)
                C(i, j) = 0.;
        C(0, 2) = 2.;
        C(2, 0) = 2.;
        C(1, 1) = -1.;

        // Solve generalized eigenproblem (simplified)
        // For LSM we use a direct algebraic fit
        Matrix Sinv = Inverse(S);
        if (Sinv.GetSizeX() == 0)
        {
            // Singular matrix, fall back
            std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
            *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
            return;
        }

        // Extract normalized ellipse coefficients
        // Simplified: use direct least squares fit with normalization
        Matrix A(np, 5);
        Vector B(np);

        for (size_t i = 0; i < np; i++)
        {
            double x = points[i].X;
            double y = points[i].Y;
            A(i, 0) = x * x;
            A(i, 1) = x * y;
            A(i, 2) = y * y;
            A(i, 3) = x;
            A(i, 4) = y;
            B[i] = -1.;
        }

        Matrix AT = Trans(A);
        Matrix ATA = AT * A;
        Vector ATb = AT * B;

        if (SystemSolution(ATA, ATb))
        {
            double a = ATb[0];
            double b = ATb[1] / 2.;
            double c = ATb[2];
            double d = ATb[3] / 2.;
            double e = ATb[4] / 2.;
            double f = 1.;

            double det = b * b - a * c;
            if (fabs(det) < PRECISION || det >= 0.)
            {
                std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
                *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
                return;
            }

            Xc = (c * d - b * e) / det;
            Yc = (a * e - b * d) / det;

            double num = 2. * (a * e * e + c * d * d + f * b * b - b * d * e - a * c * f);
            double den1 = det * (sqrt((a - c) * (a - c) + 4. * b * b) - (a + c));
            double den2 = det * (-sqrt((a - c) * (a - c) + 4. * b * b) - (a + c));

            if (den1 <= 0. || den2 <= 0.)
            {
                std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
                *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
                return;
            }

            Ax = sqrt(fabs(num / den1));
            By = sqrt(fabs(num / den2));

            if (fabs(b) < PRECISION)
                Fi = 0.;
            else
                Fi = atan2(c - a - sqrt((a - c) * (a - c) + 4. * b * b), 2. * b) * RD_GRD;

            Si = sin(GRD_RD * Fi);
            Co = cos(GRD_RD * Fi);
        }
        else
        {
            std::vector<XYPoint> fourPoints(points.begin(), points.begin() + 4);
            *this = XYEllipse(fourPoints, _TypeLimits, _TypeSystCoor);
        }
    }
}
//=========================================================================
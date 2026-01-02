#include "XYRect.h"
#include "XYPolygon.h"
#include "XYBrokenLine.h"

//=========================================================================
// Constructor - call base class constructor
//=========================================================================
XYRect::XYRect(double _Ax, double _By, double _Xc, double _Yc, double _Fi,
	int _TypeLimits, int _TypeSystCoor)
	: XYShape(_TypeLimits, _TypeSystCoor),  // NEW: Initialize base class
	  Ax(_Ax), By(_By), Xc(_Xc), Yc(_Yc), Fi(_Fi)
{
	Si = sin(GRD_RD * Fi);
	Co = cos(GRD_RD * Fi);
}
//=========================================================================
// Copy constructor - call base class constructor
//=========================================================================
XYRect::XYRect(const XYRect& A)
	: XYShape(A.TypeLimits, A.TypeSystCoor),  // NEW: Initialize base class
	  Ax(A.Ax), By(A.By), Xc(A.Xc), Yc(A.Yc), Fi(A.Fi), Si(A.Si), Co(A.Co)
{
}
//=========================================================================
XYRect::XYRect(const XYBounds& Bnd, int _TypeLimits, int _TypeSystCoor)
	: XYShape(_TypeLimits, _TypeSystCoor)  // NEW: Initialize base class
{
	Xc = (Bnd.XLeft + Bnd.XRight) / 2;
	Yc = (Bnd.YBottom + Bnd.YTop) / 2;
	Ax = fabs((Bnd.XRight - Bnd.XLeft) / 2);
	By = fabs((Bnd.YTop - Bnd.YBottom) / 2);
	Fi = 0.;
	Si = sin(GRD_RD * Fi);
	Co = cos(GRD_RD * Fi);
}
//=========================================================================
void XYRect::Set(double _Ax, double _By, double _Xc, double _Yc, double _Fi,
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
XYRect :: ~XYRect()
{
}
//=========================================================================
// Assignment operator - copy base class members
//=========================================================================
XYRect& XYRect ::operator= (const XYRect& A)
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
//=========================================================================
double XYRect::Perimeter() const
{
	double Perim = 4 * (Ax + By);
	return Perim;
}
//=========================================================================
bool XYRect::isInside(const XYPoint& P) const
{
	double X1 = (P.X - Xc) * Co + (P.Y - Yc) * Si;
	double Y1 = -(P.X - Xc) * Si + (P.Y - Yc) * Co;
	if (fabs(X1) <= Ax && fabs(Y1) <= By)
		return true;
	else
		return false;
}
//=========================================================================
bool XYRect::isInside(double X, double Y)
{
	XYPoint P(X, Y);
	return isInside(P);
}
//=========================================================================
bool XYRect::GetContour(XYBrokenLine& BLine, int NFi) const
{
	if (NFi < 2)
		return false;

	BLine.RemoveAll();
	double  AveR, X, Y;
	int j;

	AveR = fabs(Ax) + fabs(By);
	if (AveR <= PRECISION)
		return false;

	double Perim = 4. * AveR;
	double dSide = Perim / (NFi - 1);
	int NSide0 = int(2. * By / dSide + 1);
	double dSide0 = 2. * By / (NSide0 - 1);
	int NSide1 = int(2. * Ax / dSide);
	double dSide1 = 2. * Ax / (NSide1);
	int NSide2 = NSide0 - 1;
	int NSide3 = NSide1 - 1;
	XYPoint P;
	BLine.SetSize(NSide0 + NSide1 + NSide2 + NSide3);

	int k = 0;

	X = -Ax;
	for (j = 0; j < NSide0; j++)
	{
		Y = -By + j * dSide0;
		P.X = Xc + X * Co - Y * Si;
		P.Y = Yc + X * Si + Y * Co;
		BLine[k++] = P;
	}

	Y = By;
	for (j = 0; j < NSide1; j++)
	{
		X = -Ax + (j + 1) * dSide1;
		P.X = Xc + X * Co - Y * Si;
		P.Y = Yc + X * Si + Y * Co;
		BLine[k++] = P;
	}

	X = Ax;
	for (j = 0; j < NSide2; j++)
	{
		Y = By - (j + 1) * dSide0;
		P.X = Xc + X * Co - Y * Si;
		P.Y = Yc + X * Si + Y * Co;
		BLine[k++] = P;
	}

	Y = -By;
	for (j = 0; j < NSide3; j++)
	{
		X = Ax - (j + 1) * dSide1;
		P.X = Xc + X * Co - Y * Si;
		P.Y = Yc + X * Si + Y * Co;
		BLine[k++] = P;
	}
	return true;
}
//=========================================================================
bool XYRect::GetContour(XYBrokenLine& BLine, double Step) const
{
	if (Step <= 0.)
		return false;

	double Perim = Perimeter();
	int NFi = static_cast<int>(Perim / Step);
	bool isSuccess = GetContour(BLine, NFi);
	return isSuccess;
}
//=========================================================================
bool XYRect::GetContour(XYPolygon& Plg, int NFi)
{
	Plg.RemoveAll();
	XYBrokenLine BLine;
	bool isSuccess = GetContour(BLine, NFi);
	if (isSuccess)
		Plg = XYPolygon(BLine, TypeLimits, TypeSystCoor);
	return isSuccess;
}
//=========================================================================
bool XYRect::GetContour(XYPolygon& Plg, double Step)
{
	if (Step <= 0.)
		return false;

	double Perim = Perimeter();
	int NFi = static_cast<int>(Perim / Step);
	if (!NFi)
		return true;
	return GetContour(Plg, NFi);
}
//=========================================================================
void XYRect::Normalize(double Xo, double Yo, double Ro)
{
	Ax /= Ro;
	By /= Ro;
	Xc = (Xc - Xo) / Ro;
	Yc = (Yc - Yo) / Ro;
	TypeSystCoor = NORMALISED;
}
//=========================================================================
void XYRect::DeNormalize(double Xo, double Yo, double Ro)
{
	Ax *= Ro;
	By *= Ro;
	Xc = Xc * Ro + Xo;
	Yc = Yc * Ro + Yo;
	TypeSystCoor = MEASURING;
}
//=========================================================================
XYBounds XYRect::GetBounds() const
{
	auto dx = fabs(Ax * Co - By * Si);
	auto dy = fabs(Ax * Si + By * Co);
	return {Xc-dx, Yc-dy, Xc+dx, Yc+dy};
}
//=========================================================================
void XYRect::InverseY(double YcInv)
{
	Yc = YcInv - Yc;
}
//=========================================================================
void XYRect::ShiftX(double dX)
{
	Xc += dX;
}
//=========================================================================
void XYRect::ShiftY(double dY)
{
	Yc += dY;
}

//=========================================================================
// DEPRECATED FRIEND FUNCTIONS
// These functions are kept for backward compatibility but should not be used.
// They have INCORRECT isVisible() logic (inverted from correct implementation).
// Use member functions instead: rect.isInside(pt), rect.isVisible(pt)
//=========================================================================

/**
 * @brief Friend function for isInside - call member function
 * 
 * This friend function is maintained for backward compatibility but duplicates
 * the member function. Prefer using the member function for clarity.
 */
bool isInside(const XYRect& Rect, const XYPoint& P)
{
	return Rect.isInside(P);
}
//=========================================================================
/**
 * @brief DEPRECATED: Friend function for GetContour - use member function instead
 * @deprecated Use rect.GetContour(Plg, NFi) instead
 */
void GetContour(const XYRect& Rect, XYPolygon& Plg, int NFi)
{
	Plg.RemoveAll();
	double  AveR, X, Y;
	int j;

	AveR = fabs(Rect.Ax) + fabs(Rect.By);
	if (AveR <= PRECISION)
		return;

	double Perim = 4. * AveR;
	double dSide = Perim / (NFi - 1);
	int NSide0 = int(2. * Rect.By / dSide + 1);
	double dSide0 = 2. * Rect.By / (NSide0 - 1);
	int NSide1 = int(2. * Rect.Ax / dSide);
	double dSide1 = 2. * Rect.Ax / (NSide1);
	int NSide2 = NSide0 - 1;
	int NSide3 = NSide1;
	XYPoint P;
	Plg.SetSize(NSide0 + NSide1 + NSide2 + NSide3);

	int k = 0;

	X = -Rect.Ax;
	for (j = 0; j < NSide0; j++)
	{
		Y = -Rect.By + j * dSide0;
		P.X = Rect.Xc + X * Rect.Co - Y * Rect.Si;
		P.Y = Rect.Yc + X * Rect.Si + Y * Rect.Co;
		Plg[k++] = P;
	}

	Y = Rect.By;
	for (j = 0; j < NSide1; j++)
	{
		X = -Rect.Ax + (j + 1) * dSide1;
		P.X = Rect.Xc + X * Rect.Co - Y * Rect.Si;
		P.Y = Rect.Yc + X * Rect.Si + Y * Rect.Co;
		Plg[k++] = P;
	}

	X = Rect.Ax;
	for (j = 0; j < NSide2; j++)
	{
		Y = Rect.By - (j + 1) * dSide0;
		P.X = Rect.Xc + X * Rect.Co - Y * Rect.Si;
		P.Y = Rect.Yc + X * Rect.Si + Y * Rect.Co;
		Plg[k++] = P;
	}

	Y = -Rect.By;
	for (j = 0; j < NSide3; j++)
	{
		X = Rect.Ax - (j + 1) * dSide1;
		P.X = Rect.Xc + X * Rect.Co - Y * Rect.Si;
		P.Y = Rect.Yc + X * Rect.Si + Y * Rect.Co;
		Plg[k++] = P;
	}
	Plg.SetTypeLimits(Rect.TypeLimits);
	Plg.SetTypeSystCoor(Rect.TypeSystCoor);
}
//=========================================================================

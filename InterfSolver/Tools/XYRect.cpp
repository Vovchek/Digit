#include "XYRect.h"
//=========================================================================
XYRect::XYRect(double _Ax, double _By, double _Xc, double _Yc, double _Fi,
	int _TypeLimits, int _TypeSystCoor)
{
	Ax = _Ax;
	By = _By;
	Xc = _Xc;
	Yc = _Yc;
	Fi = _Fi;
	TypeLimits = _TypeLimits;
	TypeSystCoor = _TypeSystCoor;
	Si = sin(GRD_RD * Fi);
	Co = cos(GRD_RD * Fi);
}
//=========================================================================
XYRect::XYRect(const XYRect& A)
{
	Ax = A.Ax;
	By = A.By;
	Xc = A.Xc;
	Yc = A.Yc;
	Fi = A.Fi;
	TypeLimits = A.TypeLimits;
	TypeSystCoor = A.TypeSystCoor;
	Si = A.Si;
	Co = A.Co;
}
//=========================================================================
XYRect::XYRect(const XYBounds& Bnd, int _TypeLimits, int _TypeSystCoor)
{
	Xc = (Bnd.XLeft + Bnd.XRight) / 2;
	Yc = (Bnd.YBottom + Bnd.YTop) / 2;
	Ax = fabs((Bnd.XRight - Bnd.XLeft) / 2);
	By = fabs((Bnd.YTop - Bnd.YBottom) / 2);
	Fi = 0.;
	TypeLimits = _TypeLimits;
	TypeSystCoor = _TypeSystCoor;
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
	TypeLimits = _TypeLimits;
	TypeSystCoor = _TypeSystCoor;
	Si = sin(GRD_RD * Fi);
	Co = cos(GRD_RD * Fi);
}
//=========================================================================
XYRect :: ~XYRect()
{
}
//=========================================================================
XYRect& XYRect ::operator= (const XYRect& A)
{
	Ax = A.Ax;
	By = A.By;
	Xc = A.Xc;
	Yc = A.Yc;
	Fi = A.Fi;
	TypeLimits = A.TypeLimits;
	TypeSystCoor = A.TypeSystCoor;
	Si = A.Si;
	Co = A.Co;
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
	return true;
}
//=========================================================================
bool XYRect::isInside(double X, double Y)
{
	XYPoint P(X, Y);
	return isInside(P);
}
//=========================================================================
bool XYRect::isVisible(const XYPoint& P) const
{
	bool isIn = isInside(P);
	if (isIn && TypeLimits == INTERNAL)
		return false;
	else if (!isIn && TypeLimits == EXTERNAL)
		return false;
	return true;
}
//=========================================================================
bool XYRect::isVisible(double X, double Y)
{
	XYPoint P(X, Y);
	return isVisible(P);
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
bool isInside(const XYRect& Rect, const XYPoint& P)
{
	double X1 = (P.X - Rect.Xc) * Rect.Co + (P.Y - Rect.Yc) * Rect.Si;
	double Y1 = -(P.X - Rect.Xc) * Rect.Si + (P.Y - Rect.Yc) * Rect.Co;
	double R = (X1 * X1 / (Rect.Ax * Rect.Ax) + Y1 * Y1 / (Rect.By * Rect.By));
	double T = R - 1.;

	if (T < 0.)
		return true;
	else if (T > 0.)
		return false;
	return true;
}
//=========================================================================
bool isVisible(const XYRect& Rect, const XYPoint& P)
{
	bool isIn = isInside(Rect, P);
	if (isIn && Rect.TypeLimits == INTERNAL)
		return false;
	else if (!isIn && Rect.TypeLimits == EXTERNAL)
		return false;
	return true;
}
//=========================================================================
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

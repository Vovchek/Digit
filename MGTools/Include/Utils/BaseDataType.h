#if !defined(AFX_DATA_TYPE_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)
#define AFX_DATA_TYPE_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_

#include "MGToolsExport.h"

const double pi=3.1415926535897932;
const int DIGIT_PRECISION = 3;
#define INCLINE_NONE -1
#define INCLINE_PLUS 0
#define INCLINE_MINUS 1

class MGTOOLS_API CDPoint
{
 public:
    double x;
    double y;

 public:
    CDPoint();
    CDPoint(CPoint wP);
    CDPoint(double x, double y);

    CDPoint(const CDPoint& rhs){
      { operator=(rhs);}
     }

    CDPoint& operator=(const CDPoint& rhs);
    BOOL operator==(const CDPoint& rhs) const;
    BOOL operator!=(const CDPoint& rhs) const;
};

class MGTOOLS_API CDLine
{
 public:
    CDPoint P1;
    CDPoint P2;

 public:
    CDLine();
    CDLine(CDPoint _P1, CDPoint _P2);
    void Init();
	double GetH();
	double GetW();
	double GetD();
	double GetA();

    CDLine(const CDLine& rhs){
      { operator=(rhs);}
     }

    CDLine& operator=(const CDLine& rhs);
    BOOL operator>(const CDLine& rhs);
    BOOL operator<(const CDLine& rhs);
    BOOL operator==(const CDLine& rhs) const;
};

class MGTOOLS_API CDPolyLine
{
 public:
    CArray<CDPoint, CDPoint> Line;

 public:
    CDPolyLine();
    void Init();

    CDPolyLine(const CDPolyLine& rhs){
      { operator=(rhs);}
     }

    CDPolyLine& operator=(const CDPolyLine& rhs);
};

class  MGTOOLS_API CDRect
{
public:
    double left;
    double top;
    double right;
    double bottom;

public:
    CDRect();
    CDRect(double left, double top, double right, double bottom);
    CDRect(CRect wR);

    double Width();
    double Height();
	CDPoint TopLeft();
	CDPoint BottomRight();
	CDPoint GetCenter(bool TopIsMax=false);
	void InflateRect(double infX, double infY);
	void NormalizeRect();
    void MultiplyRect(double cst);

    CDRect(const CDRect& rhs){
      { operator=(rhs);}
     }

    BOOL IsPointInside(CDPoint& rhs, bool TopIsMax=true);
    BOOL IsRelRectInside(CDRect& rhs);
    BOOL IsAbsRectInside(CDRect& rhs);

    CDRect& operator=(const CDRect& rhs);
    BOOL operator==(const CDRect& rhs) const;

};

class MGTOOLS_API CDPolygon
{
 public:
	 CArray<CDPoint, CDPoint> Dots;
	 int InclineType;

 public:
    ~CDPolygon();
    CDPolygon();
	void Init();

	BOOL IsEmpty();
    BOOL IsPointInside(CDPoint& rhs);
    double Width();
    double Height();
	void ApplyAngle(double Angle);
    void SetBoundRect(CDRect _dRect);
    void SetDots(CArray<CDPoint, CDPoint>& _Dots);
	CDPoint GetCenter();
	double GetLeft();
	double GetTop();
	double GetRight();
	double GetBottom();
	void Inflate(double tolX, double tolY);
	void SetPrecision(int didgit=DIGIT_PRECISION);
    void Multiplay(double mp);
    bool SetNewCenter(double x, double y);
    bool ShiftCenter(double x, double y);
    void ShiftToX0Y0();
	void MakeMirror();

    CDPolygon(const CDPolygon& rhs){
      { operator=(rhs);}
     }

    CDPolygon& operator=(const CDPolygon& rhs);
    BOOL operator==(const CDPolygon& rhs) const;
};

double DtoD(double _d, int iDigit, bool needTol=true);

#endif // !defined(AFX_DATA_TYPE_H__D9193454_4B82_410B_84DF_87251FB1CA1D__INCLUDED_)

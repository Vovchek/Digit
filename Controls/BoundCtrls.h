#if !defined(AFX_BOUND_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_BOUND_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_
//C:\Ilya\Programming\cpp\Numbering\Controls\BoundCtrls.h
#include "MGTools\stdafx.h"
#include "Appdef.h"
#include "MGTools\Include\Utils\BaseDataType.h"

#include "InterfSolver\Tools\CalcLimits.h"

class CBoundCtrls
{
public:
   CRect CurBound;
   CPoint CustomDot;
   CArray<CPoint, CPoint> CustomDots;
   CArray<CPoint, CPoint> CurPlg;
   CArrayXYEllipse ArrEll;
   CArrayXYRect ArrRect;
   CArrayXYPolygon ArrPlg;
   CArrayXYPolygon ArrContour;
   int ExtBoundType;
   int InsBoundType;
   CArray<int,int> LastAddedBoundType;
   int NPntNax;

public:
    CBoundCtrls();
    ~CBoundCtrls();
	void Init();
    CBoundCtrls(const CBoundCtrls& rhs){
      { operator=(rhs);}
     }

    CBoundCtrls& operator=(const CBoundCtrls& rhs);

	void AddCustomDot(CPoint P);
    bool RemoveCustomDot(CPoint P);
    bool CustomDotInFocus(CPoint P);

	BOOL IsExtBound();
	BOOL IsInsBound();
	BOOL IsCurArea();
    BOOL GetPartsOfContours(int Type, CArrayXYEllipse& _ArrEll, CArrayXYRect& _ArrRect, CArrayXYPolygon& _ArrPlg);
	BOOL GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints);
    BOOL GetExtCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor);
	BOOL GetInsRealBound(int Type, int xDIB, int yDIB, int& idx, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints);
    BOOL GetInsCorBound(int Type, int xDIB, int yDIB, CRect& Bound, BOOL XCor, BOOL YCor);
    void RemoveExtBound();
    void RemoveInsBound();
    void RemoveAllBound();
	void RemoveLastBound();
    bool AddBound(int _Type, int idxExtIns);
    bool SetBound(int _Type, int idxExtIns);
    bool SetCurBound(int _Type);
    void RemoveCurBound();
    void FormBoundsOnLoadFile();
	
protected:	
   CRect ExtBoundRect;
   CRect InsBoundRect;
};

#endif // !defined(AFX_BOUND_CONTROLS_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)

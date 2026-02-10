#pragma once

// Mock implementations of required global functions for testing
// These would normally come from the application framework

// Minimal mock classes with fields used by DigitInfoFringe methods
class CBoundCtrls {
public:
    CArrayXYEllipse ArrEll;
    CArrayXYRect ArrRect;
    CArrayXYPolygon ArrPlg;
    CArrayXYPolygon ArrContour;
    int ExtBoundType = -1;
    int InsBoundType = -1;

    void CBoundCtrls::Init();

    BOOL CBoundCtrls::GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints);

    void FormBoundsOnLoadFile() {}
};

class CControls {};

class CImageCtrls {
public:
    CSize ImageSize;
    CString ImageFileName;
    CString OriginalPath;
};

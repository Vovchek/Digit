#pragma once

#include "DigitMode/IBoundsData.h"  // For interfaces
#include "InterfSolver/Tools/XYRect.h"
#include "InterfSolver/Tools/XYEllipse.h"
#include "InterfSolver/Tools/XYPolygon.h"

// Minimal mock classes with fields used by DigitInfoFringe methods
/*
class CBoundCtrls : public IBoundsData {
public:
    CBoundCtrls() {
        // Explicitly initialize all arrays and members
        ArrEll.RemoveAll();
        ArrRect.RemoveAll();
        ArrPlg.RemoveAll();
        ArrContour.RemoveAll();
        ExtBoundType = -1;
        InsBoundType = -1;
    }
    
    CArrayXYEllipse ArrEll;
    CArrayXYRect ArrRect;
    CArrayXYPolygon ArrPlg;
    CArrayXYPolygon ArrContour;
    int ExtBoundType;
    int InsBoundType;

    void Init();
    
    // IBoundsData interface implementation
    virtual int GetExtBoundType() const override { return ExtBoundType; }
    virtual int GetInsBoundType() const override { return InsBoundType; }
    virtual BOOL GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints) override;
    virtual BOOL GetInsRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints) override;

    void FormBoundsOnLoadFile() {}
};
*/
class CControls {};

class CImageCtrls : public IImageData {
public:
    CImageCtrls() 
        : ImageSize(0, 0)  // Initialize to zero
    {
    }
    
    CSize ImageSize;
    CString ImageFileName;
    CString OriginalPath;
    
    // IImageData interface implementation
    virtual CSize GetImageSize() const override { return ImageSize; }
};

#include "../DigitMode/DigitInfo.h"

#include "tests/MockControls.h"

void CBoundCtrls::Init() {
    ArrEll.RemoveAll();
    ArrRect.RemoveAll();
    ArrPlg.RemoveAll();
    ArrContour.RemoveAll();
    ExtBoundType = -1;
    InsBoundType = -1;
}

BOOL CBoundCtrls::GetExtRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints) {
    if (ExtBoundType == -1 || ExtBoundType != Type) {
        return FALSE;
    }

    ASSERT_VALID(&ArrRect);

    if (Type == 2 && ArrRect.GetSize() > 0) { // BOUND_RECT = 2
        const XYRect& rect = ArrRect[0];
        Bound.left = static_cast<int>(rect.Xc - rect.Ax);
        Bound.top = static_cast<int>(rect.Yc - rect.By);
        Bound.right = static_cast<int>(rect.Xc + rect.Ax);
        Bound.bottom = static_cast<int>(rect.Yc + rect.By);
        return TRUE;
    }

    if (Type == 0 && ArrEll.GetSize() > 0) { // BOUND_ROUND = 0
        const XYEllipse& ell = ArrEll[0];
        Bound.left = static_cast<int>(ell.Xc - ell.Ax);
        Bound.top = static_cast<int>(ell.Yc - ell.By);
        Bound.right = static_cast<int>(ell.Xc + ell.Ax);
        Bound.bottom = static_cast<int>(ell.Yc + ell.By);
        return TRUE;
    }

    return FALSE;
}

BOOL CBoundCtrls::GetInsRealBound(int Type, int xDIB, int yDIB, CRect& Bound, CArray<CPoint, CPoint>& PlgPoints) {
    // Not implemented in mock - internal bounds not needed for Phase 1 tests
    return FALSE;
}

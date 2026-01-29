#pragma once

#include "AllCommands.h"

namespace DigitMode {

inline RemoveLastDotCommand::RemoveLastDotCommand(CDigitInfo* pD, int iSeg)
    : pDigit(pD), iSegment(iSeg) {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    iDot = segment.GetPointCount() - 1;
    savedPoint = segment.GetPoint(iDot);
}

inline void RemoveLastDotCommand::Execute() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.RemovePoint(iDot);
}

inline void RemoveLastDotCommand::Undo() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.InsertPoint(iDot, savedPoint);
}

} // namespace DigitMode

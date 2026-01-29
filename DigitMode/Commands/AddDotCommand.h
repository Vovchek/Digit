#pragma once

#include "AllCommands.h"

namespace DigitMode {

inline AddDotCommand::AddDotCommand(CDigitInfo* pD, int iSeg, int iD, CDPoint p)
    : pDigit(pD), iSegment(iSeg), iDot(iD), point(p) {}

inline void AddDotCommand::Execute() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.InsertPoint(iDot, point);
}

inline void AddDotCommand::Undo() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.RemovePoint(iDot);
}

} // namespace DigitMode

#pragma once

#include "Command.h"
#include "DigitMode/DigitInfo.h"
#include "MGTools/Include/Utils/BaseDataType.h"

namespace DigitMode {

/**
 * @brief Command to add a dot to a segment
 * 
 * Used during Draw mode to incrementally build segments.
 * Each dot added creates one undo step.
 * 
 * @note Phase 2: Draw Mode implementation
 */
class AddDotCommand : public Command {
private:
    CDigitInfo* pDigit;
    int iSegment;      ///< Index into Fringes array
    int iDot;          ///< Index where dot was inserted
    CDPoint point;     ///< Dot position

public:
    /**
     * @brief Construct command to add dot to segment
     * @param pD Pointer to DigitInfo
     * @param iSeg Segment index
     * @param iD Dot index (where to insert)
     * @param p Dot position
     */
    AddDotCommand(CDigitInfo* pD, int iSeg, int iD, CDPoint p)
        : pDigit(pD), iSegment(iSeg), iDot(iD), point(p) {
        ASSERT(pD != nullptr);
        ASSERT(iSeg >= 0 && iSeg < pD->Fringes.size());
    }

    void Execute() override {
        CFringeSegment& segment = pDigit->Fringes[iSegment];
        segment.InsertPoint(iDot, point);
    }

    void Undo() override {
        CFringeSegment& segment = pDigit->Fringes[iSegment];
        segment.RemovePoint(iDot);
    }

    std::string GetName() const override {
        return "Add Dot";
    }
};

} // namespace DigitMode

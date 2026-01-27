#pragma once

#include "Command.h"
#include "DigitMode/DigitInfo.h"
#include "MGTools/Include/Utils/BaseDataType.h"

namespace DigitMode {

/**
 * @brief Command to remove the last dot from the active segment
 * 
 * Used in Draw mode with Backspace key to undo last dot placement.
 * Saves the removed dot position for undo support.
 * 
 * @note Phase 2: Draw Mode implementation
 */
class RemoveLastDotCommand : public Command {
private:
    CDigitInfo* pDigit;
    int iSegment;         ///< Segment index
    int iDot;             ///< Dot index that was removed
    CDPoint savedPoint;   ///< Saved dot position for undo

public:
    /**
     * @brief Construct command to remove last dot from segment
     * @param pD Pointer to DigitInfo
     * @param iSeg Segment index
     * 
     * Automatically determines which dot to remove (last one)
     * and saves its position for undo.
     */
    RemoveLastDotCommand(CDigitInfo* pD, int iSeg)
        : pDigit(pD), iSegment(iSeg) {
        ASSERT(pD != nullptr);
        ASSERT(iSeg >= 0 && iSeg < pD->Fringes.size());
        
        CFringeSegment& segment = pD->Fringes[iSeg];
        ASSERT(segment.GetPointCount() > 0 && "Cannot remove dot from empty segment");
        
        // Save state before removal
        iDot = segment.GetPointCount() - 1;  // Last dot index
        savedPoint = segment.GetPoint(iDot);
    }

    void Execute() override {
        CFringeSegment& segment = pDigit->Fringes[iSegment];
        segment.RemovePoint(iDot);
    }

    void Undo() override {
        CFringeSegment& segment = pDigit->Fringes[iSegment];
        segment.InsertPoint(iDot, savedPoint);
    }

    std::string GetName() const override {
        return "Remove Dot";
    }
};

} // namespace DigitMode

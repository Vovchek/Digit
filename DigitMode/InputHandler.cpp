#include "stdafx.h"
#include "InputHandler.h"
#include "DigitMode/DigitInfo.h"  // Full include for implementation
#include "DigitMode/CFringeSegment.h"

namespace DigitMode {

void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        // Finalize any active segment when leaving Draw mode
        EndCurrentSegment();
    }
    currentMode = newMode;
    // Note: Cursor update is handled by caller (ImageView)
}

void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    
    // Get the next number for new segment
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    
    // Create new segment with incremented number
    CFringeSegment newSegment(newNumber, pDigit->Fringes.GetSize());
    pDigit->Fringes.Add(newSegment);
    
    // Update CurrentNumber for next segment
    pDigit->CurrentNumber = newNumber;
    
    // Set as active segment
    iActiveSegment = pDigit->Fringes.GetSize() - 1;
    
    // Add first dot to segment (done via AddDotCommand in caller)
    
    TRACE("InputHandler::StartNewSegment: number=%.1f, segment=%d at (%d, %d)\n", 
        newNumber, iActiveSegment, P.x, P.y);
}

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && iSegment < pDigit->Fringes.GetSize());
    
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    int dotCount = segment.GetPointCount();
    
    // Validate that we're continuing from an end dot
    ASSERT(iDot == 0 || iDot == dotCount - 1 && "Can only continue from segment ends");
    
    // Set as active segment
    iActiveSegment = iSegment;
    
    TRACE("InputHandler::ContinueSegment: segment=%d, dot=%d\n", iSegment, iDot);
}

void InputHandler::ConnectSegments(int iSegment, int iDot, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && iSegment < pDigit->Fringes.GetSize());
    
    // TODO: Phase 2.2 - Implement segment connection logic
    // For now, just transfer drawing to the free end of target segment
    
    CFringeSegment& targetSegment = pDigit->Fringes[iSegment];
    int dotCount = targetSegment.GetPointCount();
    
    // Find the free end (opposite of clicked end)
    int freeEndDot = (iDot == 0) ? (dotCount - 1) : 0;
    
    // Transfer active segment to this free end
    iActiveSegment = iSegment;
    
    TRACE("InputHandler::ConnectSegments: target segment=%d, clicked dot=%d, free end=%d\n", 
        iSegment, iDot, freeEndDot);
}

void InputHandler::EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        TRACE("InputHandler::EndCurrentSegment: segment=%d\n", iActiveSegment);
        iActiveSegment = -1;
    }
}

} // namespace DigitMode

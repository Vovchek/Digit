#include "stdafx.h"
#include "InputHandler.h"
#include "DigitMode/DigitInfo.h"  // Full include for implementation
#include "DigitMode/CFringeSegment.h"
#include "Commands/AddDotCommand.h"
#include "CommandDispatcher.h"
#include <memory>

namespace DigitMode {

void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        // Finalize any active segment when leaving Draw mode
        EndCurrentSegment();
    }
    currentMode = newMode;
    // Note: Cursor update is handled by caller (ImageView)
}

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    ContinueSegment(iSegment, iDot, pDigit);
    // Optionally emit commands via pCmdDisp
}

void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // Create model state
    StartNewSegment(P, pDigit);

    // If dispatcher provided, create and execute first AddDotCommand to add initial point
    if (pCmdDisp && pDigit && iActiveSegment >= 0) {
        CDPoint dp;
        dp.x = P.x;
        dp.y = P.y;
        auto cmd = std::make_unique<AddDotCommand>(pDigit, iActiveSegment, 0, dp);
        pCmdDisp->Execute(std::move(cmd));
    }
}

void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    
    // Get the next number for new segment
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    
    // Create new segment with incremented number
    CFringeSegment newSegment(newNumber, pDigit->Fringes.size());
    pDigit->Fringes.emplace_back(newSegment);
    
    // Update CurrentNumber for next segment
    pDigit->CurrentNumber = newNumber;
    
    // Set as active segment
    iActiveSegment = pDigit->Fringes.size() - 1;
    
    // Add first dot to segment (done via AddDotCommand in caller)
    
    TRACE("InputHandler::StartNewSegment: number=%.1f, segment=%d at (%d, %d)\n", 
        newNumber, iActiveSegment, P.x, P.y);
}

void InputHandler::ConnectSegments(int iSegment, int iDot, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    ConnectSegments(iSegment, iDot, pDigit);
    // Optionally emit a ConnectSegmentsCommand via pCmdDisp
}

void InputHandler::OnMouseMove(CPoint pt, const ModifierState& mods, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // Preview logic: update temporary preview point in active segment
    if (IsInDrawMode() && iActiveSegment >= 0 && pDigit) {
        // Update preview (not yet committed) - simplest: set last point to pt for visual feedback
        // Note: real implementation should track preview separately
        TRACE("InputHandler::OnMouseMove preview at (%d,%d)\n", pt.x, pt.y);
    }
}

void InputHandler::OnLButtonUp(CPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // Commit draw operation
    if (IsInDrawMode() && iActiveSegment >= 0 && pDigit) {
        // Finalize current segment (add final dot via command if dispatcher available)
        TRACE("InputHandler::OnLButtonUp commit at (%d,%d)\n", pt.x, pt.y);
        EndCurrentSegment();
    }
}

void InputHandler::CancelDraw(CDigitInfo* pDigit) {
    // Cancel active draw and revert any preview changes
    if (iActiveSegment >= 0) {
        TRACE("InputHandler::CancelDraw segment=%d\n", iActiveSegment);
        iActiveSegment = -1;
    }
}

void InputHandler::OnKeyDown(UINT nChar, CDigitInfo* pDigit) {
    if (!IsInDrawMode()) return;
    if (nChar == VK_BACK) {
        // Remove last dot behavior should create a command via CommandDispatcher in future
        TRACE("InputHandler::OnKeyDown Backspace in draw mode\n");
    } else if (nChar == VK_ESCAPE) {
        CancelDraw(pDigit);
    }
}

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && iSegment < pDigit->Fringes.size());
    
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
    ASSERT(iSegment >= 0 && iSegment < pDigit->Fringes.size());
    
    CFringeSegment& targetSegment = pDigit->Fringes[iSegment];
    int dotCount = targetSegment.GetPointCount();
    
    // Find the free end (opposite of clicked end)
    int freeEndDot = (iDot == 0) ? (dotCount - 1) : 0;

	// move dots from connected segment to active segment
    if (activeEnd == ActiveEnd::Start) {
		if (iDot == 0)
            pDigit->Fringes[iActiveSegment].InsertPointsAtStartReverse(targetSegment);
        else
            pDigit->Fringes[iActiveSegment].InsertPointsAtStart(targetSegment);
    }
    else {
        if (iDot == 0)
            pDigit->Fringes[iActiveSegment].AppendPointsReverse(targetSegment);
		else
			pDigit->Fringes[iActiveSegment].AppendPoints(targetSegment);
    }
    
    // Transfer active segment to this free end
    activeEnd = (iDot == 0) ? ActiveEnd::Start : ActiveEnd::End;

    TRACE("InputHandler::ConnectSegments: target segment=%d, clicked dot=%d, free end=%d\n",
        iSegment, iDot, freeEndDot);

}

void InputHandler::EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        TRACE("InputHandler::EndCurrentSegment: segment=%d\n", iActiveSegment);
        iActiveSegment = -1;
    }
}

void InputHandler::HandleBoxSelection(CPoint start, CPoint end, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);

    CRect box;
    box.SetRect(start, end);
    box.NormalizeRect();

    size_t count = pDigit->selectionManager.SelectBox(box, pDigit->Fringes);

    TRACE("InputHandler::HandleBoxSelection: Selected %zu objects\n", count);
}

void InputHandler::OnMouseDrag(CPoint start, CPoint end, CDigitInfo* pDigit) {
    if (currentMode == EditMode::Navigate) {
        HandleBoxSelection(start, end, pDigit);
    }
}

} // namespace DigitMode

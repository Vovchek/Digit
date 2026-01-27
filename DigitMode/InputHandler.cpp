#include "stdafx.h"
#include "InputHandler.h"

namespace DigitMode {

void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        // Finalize any active segment when leaving Draw mode
        EndCurrentSegment();
    }
    currentMode = newMode;
    // Note: Cursor update is handled by caller (ImageView)
}

void InputHandler::StartNewSegment(CPoint P) {
    // TODO: Implement in Phase 2
    // - Get next Number from DigitInfo
    // - Create new segment
    // - Set iActiveSegment
    // - Add first dot at P

    TRACE("InputHandler::StartNewSegment at (%d, %d)\n", P.x, P.y);
}

void InputHandler::ContinueSegment(int iSegment, int iDot) {
    // TODO: Implement in Phase 2
    // - Validate iSegment and iDot (must be end dot)
    // - Set as active segment
    // - Drawing continues from this end

    iActiveSegment = iSegment;
    TRACE("InputHandler::ContinueSegment: segment=%d, dot=%d\n", iSegment, iDot);
}

void InputHandler::ConnectSegments(int iSegment, int iDot) {
    // TODO: Implement in Phase 2
    // - Connect iActiveSegment to target segment
    // - Free end of target becomes new active end
    // - Handle topology (merge or link)

    TRACE("InputHandler::ConnectSegments: target segment=%d, dot=%d\n", iSegment, iDot);
    iActiveSegment = iSegment;
}

void InputHandler::EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        TRACE("InputHandler::EndCurrentSegment: segment=%d\n", iActiveSegment);
        iActiveSegment = -1;
    }
}

} // namespace DigitMode

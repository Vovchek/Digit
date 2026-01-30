#include "stdafx.h"
#include "InputHandler.h"
#include "DigitMode/DigitInfo.h"  // Full include for implementation
#include "DigitMode/CFringeSegment.h"
#include "Commands/AllCommands.h"
#include "CommandDispatcher.h"
#include <memory>
#include "HitTester.h"

namespace DigitMode {

void InputHandler::SetMode(EditMode newMode) {
    if (currentMode == EditMode::Draw && newMode != EditMode::Draw) {
        // Finalize any active segment when leaving Draw mode
        EndCurrentSegment();
    }
    currentMode = newMode;
    // Note: Cursor update is handled by caller (ImageView)
    }

void InputHandler::OnLButtonDown(UINT flags, CPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    m_cursorPos = pt;
    ModifierState mods = ModifierState::FromKeyboard();

    // Update hover via hit tester (ImageView passes hit test earlier, but keep local)
    int hitSeg=-1, hitDot=-1;
    SelectionLevel level = HitTester().HitTest(pt, hitSeg, hitDot, pDigit->Fringes);
    m_hoverLevel = level; m_hoverSeg = hitSeg; m_hoverDot = hitDot;

    if (currentMode == EditMode::Draw) {
        if (level == SelectionLevel::None) {
            StartNewSegment(pt, pDigit, pCmdDisp);
            return;
        }
        if (level == SelectionLevel::Dot) {
            ContinueSegment(hitSeg, hitDot, pDigit, pCmdDisp);
            return;
        }
    }

    // Navigate mode: begin box select if empty, else prepare drag
    if (currentMode == EditMode::Navigate) {
        if (level == SelectionLevel::None) {
            m_drag.active = true;
            m_drag.type = DragState::Type::BoxSelect;
            m_drag.start = pt;
            m_drag.current = pt;
        } else if (level == SelectionLevel::Dot) {
            m_drag.active = true;
            m_drag.type = DragState::Type::MoveDot;
            m_drag.segmentIndex = hitSeg;
            m_drag.dotIndex = hitDot;
            m_drag.start = pt;
            m_drag.current = pt;
        }
    }
}

void InputHandler::OnRButtonDown(UINT flags, CPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // Context menu typically handled by view; InputHandler does not implement
}

// (OnKeyUp implemented later)

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    ContinueSegment(iSegment, iDot, pDigit);
    // Optionally emit commands via pCmdDisp
}

void InputHandler::StartNewSegment(CPoint P, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    if (!pCmdDisp || !pDigit) {
        // Fallback to legacy behaviour
        StartNewSegment(P, pDigit);
        return;
    }

    // Use command to create the new segment (no points yet)
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    auto createCmd = std::make_unique<CreateSegmentCommand>(*pDigit, std::vector<CDPoint>{}, newNumber);
    pCmdDisp->Execute(std::move(createCmd));

    // Update CurrentNumber
    pDigit->CurrentNumber = newNumber;

    // Set active segment to newly created one
    iActiveSegment = static_cast<int>(pDigit->Fringes.size()) - 1;

    // Add initial dot via command
    CDPoint dp; dp.x = P.x; dp.y = P.y;
    auto addDot = std::make_unique<AddDotCommand>(pDigit, iActiveSegment, 0, dp);
    pCmdDisp->Execute(std::move(addDot));
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
    if (pCmdDisp && pDigit && iActiveSegment >= 0) {
        // Map activeEnd and clicked end into booleans expected by command
        bool endA = (activeEnd == ActiveEnd::Tail); // true => attach at A's end (append)
        int dotCount = pDigit->Fringes[static_cast<int>(iSegment)].GetPointCount();
        bool endB = (iDot == dotCount - 1); // true => clicked B's end

        auto cmd = std::make_unique<ConnectSegmentsCommand>(*pDigit, static_cast<size_t>(iActiveSegment), endA, static_cast<size_t>(iSegment), endB);
        pCmdDisp->Execute(std::move(cmd));

        // After connection, set active end to the free end of the target (as before)
        activeEnd = (iDot == 0) ? ActiveEnd::Head : ActiveEnd::Tail;
        return;
    }

    // Fallback: mutate directly (legacy behavior)
}

void InputHandler::OnMouseMove(CPoint pt, const ModifierState& mods, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    m_cursorPos = pt;
    // Update hover via hit tester
    if (pDigit) {
        int hitSeg=-1, hitDot=-1;
        SelectionLevel level = HitTester().HitTest(pt, hitSeg, hitDot, pDigit->Fringes);
        m_hoverLevel = level; m_hoverSeg = hitSeg; m_hoverDot = hitDot;
    }

    // Update drag if active
    if (m_drag.active) {
        m_drag.current = pt;
        // For box select, update selection preview (view will query state)
        if (m_drag.type == DragState::Type::BoxSelect) {
            // no geometry changes here
        }
    }

    // Draw-mode preview: no document mutation
    if (IsInDrawMode() && iActiveSegment >= 0 && pDigit) {
        TRACE("InputHandler::OnMouseMove preview at (%d,%d)\n", pt.x, pt.y);
    }
}

void InputHandler::OnLButtonUp(CPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    m_cursorPos = pt;
    // If a drag was active, commit appropriate command
    if (m_drag.active) {
        if (m_drag.type == DragState::Type::BoxSelect) {
            HandleBoxSelection(m_drag.start, m_drag.current, pDigit);
        } else if (m_drag.type == DragState::Type::MoveDot) {
            // Create MoveDotCommand if position changed
            if (pCmdDisp && pDigit) {
                CDPoint oldP = pDigit->Fringes[m_drag.segmentIndex].GetPoint(m_drag.dotIndex);
                CDPoint newP; newP.x = pt.x; newP.y = pt.y;
                if (!(oldP == newP)) {
                    auto cmd = std::make_unique<MoveDotCommand>(*pDigit, m_drag.segmentIndex, m_drag.dotIndex, oldP, newP);
                    pCmdDisp->Execute(std::move(cmd));
                }
            }
        }
        // Clear drag
        m_drag = DragState();
        return;
    }

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
        activeEnd = ActiveEnd::None;
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

void InputHandler::OnKeyUp(UINT nChar, CDigitInfo* pDigit) {
    // placeholder for future handling
}

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && static_cast<size_t>(iSegment) < pDigit->Fringes.size());
    
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    int dotCount = segment.GetPointCount();
    
    // Validate that we're continuing from an end dot
    ASSERT(iDot == 0 || iDot == dotCount - 1 && "Can only continue from segment ends");
    
    // Set as active segment & end
	activeEnd = (iDot == 0) ? ActiveEnd::Head : ActiveEnd::Tail;
    iActiveSegment = iSegment;
    
    TRACE("InputHandler::ContinueSegment: segment=%d, dot=%d\n", iSegment, iDot);
}

void InputHandler::ConnectSegments(int iSegment, int iDot, CDigitInfo* pDigit) {
    // Legacy fallback: this method mutates document directly. Prefer using overload with dispatcher.
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && static_cast<size_t>(iSegment) < pDigit->Fringes.size());

    CFringeSegment& targetSegment = pDigit->Fringes[iSegment];
    int dotCount = targetSegment.GetPointCount();
    int freeEndDot = (iDot == 0) ? (dotCount - 1) : 0;

    if (activeEnd == ActiveEnd::Head) {
        if (iDot == 0)
            pDigit->Fringes[iActiveSegment].InsertPointsAtStartReverse(targetSegment);
        else
            pDigit->Fringes[iActiveSegment].InsertPointsAtStart(targetSegment);
    } else {
        if (iDot == 0)
            pDigit->Fringes[iActiveSegment].AppendPointsReverse(targetSegment);
        else
            pDigit->Fringes[iActiveSegment].AppendPoints(targetSegment);
    }

    activeEnd = (iDot == 0) ? ActiveEnd::Head : ActiveEnd::Tail;
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

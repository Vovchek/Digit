#include "stdafx.h"
#include "InputHandler.h"
#include "DigitMode/DigitInfo.h"  // Full include for implementation
#include "DigitMode/CFringeSegment.h"
#include "Commands/AllCommands.h"
#include "CommandDispatcher.h"
#include <memory>
#include "HitTester.h"
#include "../ImageTempl/ViewTransform.h"

namespace DigitMode {

void InputHandler::SetMode(FringeEditMode newMode) {
    if (currentMode == FringeEditMode::Draw && newMode != FringeEditMode::Draw) {
        // Finalize any active segment when leaving Draw mode
        EndCurrentSegment();
    }
    currentMode = newMode;
    // Note: Cursor update is handled by caller (ImageView)
    }

void InputHandler::OnLButtonDown(UINT flags, CDPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp,
    const ::ViewTransform* view) {
    m_cursorPos = pt;
    ModifierState mods = ModifierState::FromKeyboard();

    // Update hover via hit tester (ImageView passes hit test earlier, but keep local)
    int hitSeg=-1, hitDot=-1;
    HitTester tester;
	double tolerance = std::max(1., (view ? 5.0 / view->GetScale() : 5.0)); // Adjust tolerance for zoom level
    SelectionLevel level = tester.HitTest(pt, hitSeg, hitDot, pDigit->Fringes, tolerance);
    m_hoverLevel = level; m_hoverSeg = hitSeg; m_hoverDot = hitDot;

    // Alt+L-Click destructive actions (delete dot / split edge)
    if (mods.alt) {
        if (level == SelectionLevel::Dot) {
            if (pCmdDisp && pDigit) {
                if (pDigit->Fringes[hitSeg].GetPointCount() == 1) {
					// delete entire segment if only one dot
                    std::vector<size_t> segs(1, hitSeg);
                    auto cmd = std::make_unique<DeleteSegmentsCommand>(*pDigit, segs);
					pCmdDisp->Execute(std::move(cmd));
                    if(hitSeg == iActiveSegment) {
						iActiveSegment = -1; // clear active segment
						activeEnd = ActiveEnd::None;
					}
                }
                else {
                    auto cmd = std::make_unique<DeleteDotCommand>(*pDigit, hitSeg, hitDot);
                    pCmdDisp->Execute(std::move(cmd));
                }
            }
            return;
        }
        else if (level == SelectionLevel::Edge) {
            if (pCmdDisp && pDigit) {
                // split at edge end: remove connection between hitDot and hitDot+1
                auto cmd = std::make_unique<SplitSegmentCommand>(*pDigit, hitSeg, hitDot + 1);
                pCmdDisp->Execute(std::move(cmd));
            }
            return;
        }
    }
    if (currentMode == FringeEditMode::DotEdit) {
        if (level != SelectionLevel::None && hitSeg >= 0) {
            iActiveSegment = hitSeg;
            activeEnd = ActiveEnd::Head;

            pDigit->CurrentNumber = pDigit->Fringes[iActiveSegment].GetNumber();
        }
        // -> Dragging dots and edges while editing
        if (level == SelectionLevel::Dot) {
            BeginDotDrag(hitSeg, hitDot, pt, nullptr); // nullptr to use pt for undo
            return;
        }
        // Edge drag or insert dot with Ctrl+Edge click
        if (level == SelectionLevel::Edge) {
            if (mods.ctrl) {
                // Insert dot at clicked edge position
                if (pCmdDisp && pDigit) {
                    CDPoint dp; dp.x = pt.x; dp.y = pt.y;
                    auto insertCmd = std::make_unique<AddDotCommand>(pDigit, hitSeg, hitDot + 1, dp);
                    pCmdDisp->Execute(std::move(insertCmd));
                }
                return; // TODO: try dragging newly inserted dot?
            }
            BeginEdgeDrag(hitSeg, hitDot, pt, pDigit);
            return;
        }

    }
    if (currentMode == FringeEditMode::Draw) {
        // If we already have an active segment, empty clicks should add a dot to its active end
        if (IsActiveSegmentValid(pDigit)) {
            if (level == SelectionLevel::None) {
				// End segment if emtpty click with Ctrl
				if (mods.ctrl) {
                    EndCurrentSegment();
                    return; // consumed
                }
                // Add dot to active segment at head or tail
                auto& seg = pDigit->Fringes[iActiveSegment];
                int insertIndex = (activeEnd == ActiveEnd::Head) ? 0 : seg.GetPointCount();
                if (pCmdDisp) {
                    CDPoint dp; dp.x = pt.x; dp.y = pt.y;
                    auto add = std::make_unique<AddDotCommand>(pDigit, iActiveSegment, insertIndex, dp);
                    pCmdDisp->Execute(std::move(add));
                }
                return; // consumed
            }

            if (level == SelectionLevel::Dot) {
                // Click on a dot while drawing: possibly connect or change active end
                if (mods.ctrl && hitSeg != iActiveSegment && pCmdDisp) {
                    // Connect active segment to clicked segment end
                    ConnectSegments(hitSeg, hitDot, pDigit, pCmdDisp);
                    return;
                }
                // Otherwise, adopt this segment as active (continue from that end) ->
            }
        }
        // -> Dragging is also allowed for dots and edges while editing
        if (level == SelectionLevel::Dot) {
            ContinueSegment(hitSeg, hitDot, pDigit, pCmdDisp);
            BeginDotDrag(hitSeg, hitDot, pt, nullptr); // nullptr to use pt for undo
            return;
        }
		// Edge drag or insert dot with Ctrl+Edge click
        if (level == SelectionLevel::Edge) {
            if(mods.ctrl) {
                // Insert dot at clicked edge position
                if (pCmdDisp && pDigit) {
                    CDPoint dp; dp.x = pt.x; dp.y = pt.y;
                    auto insertCmd = std::make_unique<AddDotCommand>(pDigit, hitSeg, hitDot + 1, dp);
                    pCmdDisp->Execute(std::move(insertCmd));
                }
				return; // TODO: try dragging newly inserted dot?
			}
            BeginEdgeDrag(hitSeg, hitDot, pt, pDigit);
            return;
        }
        // No active segment -> behave as start/continue as before
        if (level == SelectionLevel::None && mods.None()) {
            StartNewSegment(pt, pDigit, pCmdDisp);
            return;
        }
    }

    // Navigate mode: handle selection and box select
    if (currentMode == FringeEditMode::Navigate) {
        if (level == SelectionLevel::None) {
            // Empty click: start box select
            m_drag.active = true;
            m_drag.type = DragState::Type::BoxSelect;
            m_drag.start = pt;
            m_drag.current = pt;
        } 
        else if (level == SelectionLevel::Dot) {
            // Click on dot
            if (mods.ctrl) {
                // Ctrl+Click: Add/toggle to selection
                SelectionManager::SelectedObject obj;
                obj.level = SelectionLevel::Dot;
                obj.iSegment = hitSeg;
                obj.iDot = hitDot;
                pDigit->selectionManager.AddToSelection(obj);
            }
            else if (mods.shift) {
                // Shift+Click: Range select (TODO: implement range logic)
                // For now, just select the dot
                pDigit->selectionManager.SelectDot(hitSeg, hitDot);
            }
            else if (mods.alt) {
                // Alt+Click: Promote to Fringe
                pDigit->selectionManager.SelectDot(hitSeg, hitDot);
                pDigit->selectionManager.PromoteToFringe(pDigit->Fringes);
            }
            else {
                // Plain click: Select single
                pDigit->selectionManager.SelectDot(hitSeg, hitDot);
            }
            
            // Start drag if not Ctrl (Ctrl is just toggle)
            if (!mods.ctrl) {
                BeginDotDrag(hitSeg, hitDot, pt, pDigit);
            }
        } 
        else if (level == SelectionLevel::Edge) {
            // Click on edge
            if (mods.ctrl) {
                SelectionManager::SelectedObject obj;
                obj.level = SelectionLevel::Edge;
                obj.iSegment = hitSeg;
                obj.iEdge = hitDot; // HitTester returns edge start index in hitDot
                pDigit->selectionManager.AddToSelection(obj);
            }
            else if (mods.alt) {
                pDigit->selectionManager.SelectEdge(hitSeg, hitDot);
                pDigit->selectionManager.PromoteToFringe(pDigit->Fringes);
            }
            else {
                pDigit->selectionManager.SelectEdge(hitSeg, hitDot);
            }
            
            if (!mods.ctrl) {
                BeginEdgeDrag(hitSeg, hitDot, pt, pDigit);
            }
        }
        else if (level == SelectionLevel::Segment) {
            // Click on segment body
            if (mods.ctrl) {
                SelectionManager::SelectedObject obj;
                obj.level = SelectionLevel::Segment;
                obj.iSegment = hitSeg;
                pDigit->selectionManager.AddToSelection(obj);
            }
            else if (mods.alt) {
                pDigit->selectionManager.SelectSegment(hitSeg);
                pDigit->selectionManager.PromoteToFringe(pDigit->Fringes);
            }
            else {
                pDigit->selectionManager.SelectSegment(hitSeg);
            }
        }
    }
}

void InputHandler::OnRButtonDown(UINT flags, CDPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp,
    const ::ViewTransform* view) {
    // Context menu typically handled by view; InputHandler does not implement
}

// (OnKeyUp implemented later)

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    ContinueSegment(iSegment, iDot, pDigit);
    // Optionally emit commands via pCmdDisp
}

void InputHandler::StartNewSegment(CDPoint P, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // Use command to create the new segment, add initial dot
    double newNumber = pDigit->CurrentNumber + pDigit->numStep;
    auto createCmd = std::make_unique<CreateSegmentCommand>(*pDigit, std::vector<CDPoint>{P}, newNumber);
    pCmdDisp->Execute(std::move(createCmd));

    // Update CurrentNumber
    pDigit->CurrentNumber = newNumber;

    // Set active segment to newly created one
    iActiveSegment = static_cast<int>(pDigit->Fringes.size()) - 1;
	activeEnd = ActiveEnd::Tail; // Default to tail
}

void InputHandler::ConnectSegments(int iSegment, int iDot, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    if (pCmdDisp && IsActiveSegmentValid(pDigit)) {
        // Map activeEnd and clicked end into booleans expected by command
        bool endA = (activeEnd == ActiveEnd::Tail); // true => attach at A's end (append)
        int dotCount = pDigit->Fringes[static_cast<int>(iSegment)].GetPointCount();
        bool endB = (iDot >= dotCount / 2); // true => clicked close to B's end

        auto cmd = std::make_unique<ConnectSegmentsCommand>(*pDigit, static_cast<size_t>(iActiveSegment), endA, static_cast<size_t>(iSegment), endB);
        pCmdDisp->Execute(std::move(cmd));

        // After connection, iActiveSegment and activeEnd persits

        return;
    }

    // No legacy fallback: callers must pass a dispatcher
}

void InputHandler::OnMouseMove(CDPoint pt, const ModifierState& mods, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp,
    const ::ViewTransform* view) {
    m_cursorPos = pt;
    // Update hover via hit tester
    if (pDigit) {
        double tolerance = (view ? 5.0 / view->GetScale() : 5.0); // Adjust tolerance for zoom level
        int hitSeg=-1, hitDot=-1;
        SelectionLevel level = HitTester().HitTest(pt, hitSeg, hitDot, pDigit->Fringes, tolerance);
        m_hoverLevel = level; m_hoverSeg = hitSeg; m_hoverDot = hitDot;
    }

    // Update drag if active
    if (m_drag.active) {
        m_drag.current = pt;
        UpdateDragPreview(pt, pDigit);
    }

    // Draw-mode preview: no document mutation
    if (IsInDrawMode() && IsActiveSegmentValid(pDigit)) {
        //TRACE("InputHandler::OnMouseMove preview at (%d,%d)\n", pt.x, pt.y);
    }
}

void InputHandler::OnMouseWheel(const CPoint& pt, short zDelta, ::ViewTransform* view) {
    ModifierState mods = ModifierState::FromKeyboard();
    if (mods.ctrl && view) {
        double factor = (zDelta > 0) ? 1.15 : (1.0 / 1.15);
        view->ZoomAt(pt, factor);
    }
}

void InputHandler::BeginPan(const CPoint& pt) {
    m_isPanning = true;
    m_lastPanPoint = pt;
}

void InputHandler::ContinuePan(const CPoint& pt, ::ViewTransform* view) {
    if (!m_isPanning || !view) return;
    CPoint delta(pt.x - m_lastPanPoint.x, pt.y - m_lastPanPoint.y);
    view->PanBy(delta);
    m_lastPanPoint = pt;
}

void InputHandler::EndPan() {
    m_isPanning = false;
    m_lastPanPoint = CPoint(-1, -1);
}

void InputHandler::OnLButtonUp(CDPoint pt, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp, const ::ViewTransform* view) {
    m_cursorPos = pt;
    // If a drag was active, commit appropriate command
    if (m_drag.active) {
        CommitActiveDrag(pCmdDisp, pDigit);
        // Clear drag
        m_drag = DragState();
        return;
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

bool InputHandler::OnKeyDown(UINT nChar, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {

    ModifierState mods = ModifierState::FromKeyboard();

    if(nChar == VK_ESCAPE) {
		// always clear selection on Escape
        pDigit->selectionManager.Clear();
        // Always cancel drag or draw on Escape
        if (m_drag.active) {
            // Cancel active drag
            m_drag = DragState();
            return true;
        }
        else if (IsInDrawMode()) {
            CancelDraw(pDigit);
            return true;
        }
	}

    bool consumed = false;
    if (IsInDrawMode()) {
        if (nChar == VK_BACK) {
            // Delete last dot on active segment (if any)
            if (IsActiveSegmentValid(pDigit) && pCmdDisp) {
                int seg = iActiveSegment;
                auto& s = pDigit->Fringes[seg];
                int last = s.GetPointCount() - 1;
                if (last == 0) {
					std::vector<size_t> segs(1, seg);
                    auto cmd = std::make_unique<DeleteSegmentsCommand>(*pDigit, segs);
                    pCmdDisp->Execute(std::move(cmd));
                    iActiveSegment = -1;
                    activeEnd = ActiveEnd::None;
					return true;
                }
                else if (last > 0) {
                    auto cmd = std::make_unique<RemoveLastDotCommand>(pDigit, seg);
                    pCmdDisp->Execute(std::move(cmd));
                    consumed = true;
                }
            }
        }
        else if (nChar == VK_ESCAPE) {
            CancelDraw(pDigit);
            consumed = true;
        }
        else if (nChar == VK_RETURN) {
            // Finalize active segment
            EndCurrentSegment();
            consumed = true;
        }
        else if (nChar == 'b' || nChar == 'B') {
            // flip rubber band status
            m_rubberBand = !m_rubberBand;
            consumed = true;
        }
        else if((nChar == VK_ADD || nChar == VK_OEM_PLUS) && mods.None()) {
            // Increase current number
            if (pDigit) {
                pDigit->CurrentNumber += pDigit->numStep;
                if (IsActiveSegmentValid(pDigit)) {
                    std::vector<size_t> segv = { static_cast<size_t>(iActiveSegment) };
                    auto cmd = std::make_unique<RenumberSegmentsCommand>(*pDigit, segv, pDigit->CurrentNumber);
                    pCmdDisp->Execute(std::move(cmd));
                }
                consumed = true;
            }
        }
        else if ((nChar == VK_SUBTRACT || nChar == VK_OEM_MINUS) && mods.None()) {
            // Decrease current number
            if (pDigit) {
                pDigit->CurrentNumber -= pDigit->numStep;
				if (IsActiveSegmentValid(pDigit)) {
                    std::vector<size_t> segv = { static_cast<size_t>(iActiveSegment) };
                    auto cmd = std::make_unique<RenumberSegmentsCommand>(*pDigit, segv, pDigit->CurrentNumber);
                    pCmdDisp->Execute(std::move(cmd));
                }
                consumed = true;
            }
        }
        else if (nChar == VK_TAB && IsActiveSegmentValid(pDigit)) {
            if (mods.shift) {
                // switch to the previous segment if any
                if (iActiveSegment > 0) {
                    iActiveSegment--;
                    consumed = true;
                }
            } else {
                // progress to the next segment if any
                int num_fringes = static_cast<int>(pDigit->Fringes.size());
                if (num_fringes > 1 && iActiveSegment < num_fringes - 1) {
                    iActiveSegment++;
                    consumed = true;
                }
            }
            pDigit->CurrentNumber = pDigit->Fringes[iActiveSegment].GetNumber();
        }
    }
    else if (IsInNavigateMode()) {
        // Navigate mode: Handle keyboard shortcuts
        if ((nChar == 'n' || nChar == 'N') && !mods.shift && !mods.ctrl && !mods.alt) {
            // 'N' key: Auto-number fringes using selected segments as trusted
            if (pDigit && pCmdDisp) {
                std::vector<size_t> trustedIndices;
                
                // Collect segment indices from current selection
                for (size_t i = 0; i < pDigit->selectionManager.GetCount(); ++i) {
                    const auto& obj = pDigit->selectionManager.GetAt(i);
                    // Accept Segment and Fringe-level selections
                    if ((obj.level == SelectionLevel::Segment || obj.level == SelectionLevel::Fringe) 
                        && obj.iSegment >= 0) {
                        trustedIndices.push_back(static_cast<size_t>(obj.iSegment));
                    }
                }
                
                // If no selection, use 2 first segments as default
                if (trustedIndices.empty() && pDigit->Fringes.size() > 0) {
                    trustedIndices = {0};
                }
                
                // Execute auto-numbering command
                auto cmd = std::make_unique<AutoNumberingCommand>(
                    *pDigit,
                    trustedIndices,
                    pDigit->numStep,    // default step
                    0.7     // default confidence threshold
                );
                pCmdDisp->Execute(std::move(cmd));
                consumed = true;

                TRACE("InputHandler::OnKeyDown: Auto-number triggered with %zu trusted segments\n", 
                      trustedIndices.size());
            }
        }
        else if (nChar == VK_DELETE && !pDigit->selectionManager.IsEmpty()) {
            // Del key: Delete selected items
            if (pCmdDisp) {
                auto cmd = std::make_unique<DeleteSelectionCommand>(*pDigit, pDigit->selectionManager);
                pCmdDisp->Execute(std::move(cmd));
                consumed = true;
                
                TRACE("InputHandler::OnKeyDown: Delete selection executed\n");
            }
        }
    }
    else if (IsInEditMode()) {
        if ((nChar == VK_ADD || nChar == VK_OEM_PLUS) && mods.None()) {
            // Increase current number
            if (pDigit) {
                pDigit->CurrentNumber += pDigit->numStep;
                if (IsActiveSegmentValid(pDigit)) {
                    std::vector<size_t> segv = { static_cast<size_t>(iActiveSegment) };
                    auto cmd = std::make_unique<RenumberSegmentsCommand>(*pDigit, segv, pDigit->CurrentNumber);
                    pCmdDisp->Execute(std::move(cmd));
                }
                consumed = true;
            }
        }
        else if ((nChar == VK_SUBTRACT || nChar == VK_OEM_MINUS) && mods.None()) {
            // Decrease current number
            if (pDigit) {
                pDigit->CurrentNumber -= pDigit->numStep;
                if (IsActiveSegmentValid(pDigit)) {
                    std::vector<size_t> segv = { static_cast<size_t>(iActiveSegment) };
                    auto cmd = std::make_unique<RenumberSegmentsCommand>(*pDigit, segv, pDigit->CurrentNumber);
                    pCmdDisp->Execute(std::move(cmd));
                }
                consumed = true;
            }
        }
        else if (nChar == VK_TAB && IsActiveSegmentValid(pDigit)) {
            if (mods.shift) {
                // switch to the previous segment if any
                if (iActiveSegment > 0) {
                    iActiveSegment--;
                    consumed = true;
                }
            }
            else {
                // progress to the next segment if any
                int num_fringes = static_cast<int>(pDigit->Fringes.size());
                if (num_fringes > 1 && iActiveSegment < num_fringes - 1) {
                    iActiveSegment++;
                    consumed = true;
                }
            }
            pDigit->CurrentNumber = pDigit->Fringes[iActiveSegment].GetNumber();
        }
    }
	return consumed;
}

void InputHandler::OnKeyUp(UINT nChar, CDigitInfo* pDigit, CommandDispatcher* pCmdDisp) {
    // placeholder for future handling
}

void InputHandler::ContinueSegment(int iSegment, int iDot, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);
    ASSERT(iSegment >= 0 && static_cast<size_t>(iSegment) < pDigit->Fringes.size());
    
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    int dotCount = segment.GetPointCount();
    
    // Validate that we're continuing from an end dot
	if (iDot != 0 && iDot != dotCount - 1) {
        TRACE("InputHandler::ContinueSegment: Invalid dot index %d for segment %d with %d dots\n", iDot, iSegment, dotCount);
		return;
    }
    
    // Set as active segment & end
	activeEnd = (iDot == 0) ? ActiveEnd::Head : ActiveEnd::Tail;
    iActiveSegment = iSegment;
	pDigit->CurrentNumber = segment.GetNumber();
    
    TRACE("InputHandler::ContinueSegment: segment=%d, dot=%d\n", iSegment, iDot);
}

// legacy direct-mutation overload removed: callers must pass a CommandDispatcher

void InputHandler::EndCurrentSegment() {
    if (iActiveSegment >= 0) {
        TRACE("InputHandler::EndCurrentSegment: segment=%d\n", iActiveSegment);
        activeEnd = ActiveEnd::None;
    }
}

void InputHandler::HandleBoxSelection(CDPoint start, CDPoint end, CDigitInfo* pDigit) {
    ASSERT(pDigit != nullptr);

    CDRect box(start.x, start.y, end.x, end.y);
    box.NormalizeRect();

    // Determine mode from current modifiers
    ModifierState mods = ModifierState::FromKeyboard();
    BoxSelectionMode mode = BoxSelectionMode::Default;
    
    if (mods.alt) {
        mode = BoxSelectionMode::Fringe;  // Alt = Fringe select
    }
    else if (mods.shift) {
        mode = BoxSelectionMode::Segment;  // Shift = Segment select
    }
    else if (mods.ctrl) {
        mode = BoxSelectionMode::AddMode;  // Ctrl = Add to selection
    }

    size_t count = pDigit->selectionManager.SelectBox(box, pDigit->Fringes, mode);

    TRACE("InputHandler::HandleBoxSelection: Selected %zu objects (mode=%d)\n", count, static_cast<int>(mode));
}

void InputHandler::OnMouseDrag(CDPoint start, CDPoint end, CDigitInfo* pDigit) {
    if (currentMode == FringeEditMode::Navigate) {
        HandleBoxSelection(start, end, pDigit);
    }
}

bool InputHandler::IsActiveSegmentValid(const ::CDigitInfo* doc) const {
    return (doc != nullptr && iActiveSegment >= 0 
        && static_cast<size_t>(iActiveSegment) < doc->Fringes.size() 
		&& doc->Fringes[iActiveSegment].GetPointCount() > 0
        && activeEnd != ActiveEnd::None);
}

// ---- Drag helpers (implementation local) ----
void InputHandler::BeginDotDrag(int segIdx, int dotIdx, CDPoint start, CDigitInfo* pDigit) {
    m_drag.active = true;
    m_drag.type = DragState::Type::MoveDot;
    m_drag.segmentIndex = segIdx;
    m_drag.dotIndex = dotIdx;
    m_drag.start = start;
    m_drag.current = start;
    m_drag.dotOldPos = pDigit ? pDigit->Fringes[segIdx].GetPoint(dotIdx) : start;
}

void InputHandler::BeginEdgeDrag(int segIdx, int edgeStartIdx, CDPoint start, ::CDigitInfo* pDigit) {
    m_drag.active = true;
    m_drag.type = DragState::Type::MoveEdge;
    m_drag.segmentIndex = segIdx;
    m_drag.dotIndex = edgeStartIdx;
    m_drag.start = start;
    m_drag.current = start;
    if (pDigit) {
        auto& seg = pDigit->Fringes[segIdx];
        m_drag.edgeOldA = seg.GetPoint(edgeStartIdx);
        m_drag.edgeOldB = seg.GetPoint(edgeStartIdx + 1);
    }
}

void InputHandler::UpdateDragPreview(CDPoint pt, ::CDigitInfo* pDigit) {
    // For preview we perform immediate document updates
    if (!pDigit) return;
    if (m_drag.type == DragState::Type::MoveDot) {
        CDPoint newP; newP.x = pt.x; newP.y = pt.y;
        pDigit->Fringes[m_drag.segmentIndex].SetPoint(m_drag.dotIndex, newP);
    }
    else if (m_drag.type == DragState::Type::MoveEdge) {
        int segIdx = m_drag.segmentIndex;
        int eStart = m_drag.dotIndex;
        CDPoint delta; delta.x = pt.x - m_drag.start.x; delta.y = pt.y - m_drag.start.y;
        CDPoint a = m_drag.edgeOldA; a.x += delta.x; a.y += delta.y;
        CDPoint b = m_drag.edgeOldB; b.x += delta.x; b.y += delta.y;
        auto& seg = pDigit->Fringes[segIdx];
        seg.SetPoint(eStart, a);
        seg.SetPoint(eStart + 1, b);
    }
}

void InputHandler::CommitActiveDrag(CommandDispatcher* pCmdDisp, ::CDigitInfo* pDigit) {
    if (m_drag.type == DragState::Type::BoxSelect) {
        HandleBoxSelection(m_drag.start, m_cursorPos/*m_drag.current*/, pDigit);
        return;
    }
    if (!pCmdDisp || !pDigit) return;

    if (m_drag.type == DragState::Type::MoveDot) {
        CDPoint oldP = m_drag.dotOldPos;
        CDPoint newP = m_cursorPos; //pDigit->Fringes[m_drag.segmentIndex].GetPoint(m_drag.dotIndex);
        if (!(oldP == newP)) {
            auto cmd = std::make_unique<MoveDotCommand>(*pDigit, m_drag.segmentIndex, m_drag.dotIndex, oldP, newP);
            pCmdDisp->Execute(std::move(cmd));
        }
        return;
    }

    if (m_drag.type == DragState::Type::MoveEdge) {
        int segIdx = m_drag.segmentIndex;
        int edgeStart = m_drag.dotIndex;
        CDPoint oldA = m_drag.edgeOldA;
        CDPoint oldB = m_drag.edgeOldB;
        auto& seg = pDigit->Fringes[segIdx];
        CDPoint newA = seg.GetPoint(edgeStart);
        CDPoint newB = seg.GetPoint(edgeStart + 1);
        if (!(oldA == newA)) {
            auto cmdA = std::make_unique<MoveDotCommand>(*pDigit, segIdx, edgeStart, oldA, newA);
            pCmdDisp->Execute(std::move(cmdA));
        }
        if (!(oldB == newB)) {
            auto cmdB = std::make_unique<MoveDotCommand>(*pDigit, segIdx, edgeStart + 1, oldB, newB);
            pCmdDisp->Execute(std::move(cmdB));
        }
        return;
    }
}

CDPoint InputHandler::GetActiveDot(const ::CDigitInfo* doc) const
{
	if (!IsActiveSegmentValid(doc))
        return CPoint(-1, -1);
	const auto& seg = doc->Fringes[iActiveSegment];
	int dotIdx = (activeEnd == ActiveEnd::Head) ? 0 : seg.GetPointCount() - 1;
	CDPoint dp = seg.GetPoint(dotIdx);
	return CPoint(static_cast<int>(dp.x), static_cast<int>(dp.y));
}

// active dot to cursor rubber band status
// hides when mode is not Draw, Alt is pressed, dragging is active, or no valid active dot
bool InputHandler::GetRubberBand(const ::CDigitInfo* doc) const
{
	if (!m_rubberBand) return false;
    ModifierState mods = ModifierState::FromKeyboard();
	return IsInDrawMode() && !mods.alt && !m_drag.active && IsActiveSegmentValid(doc);
}

void InputHandler::DrawSelectionBox(CDC* pDC, const ViewTransform* viewTransform) const
{
    if (!pDC || !m_drag.active || m_drag.type != DragState::Type::BoxSelect) {
        return;
    }
    
    // Convert world coordinates to screen if transform provided
    auto start = m_drag.start;
    auto current = m_drag.current;
    
    if (viewTransform) {
        start = viewTransform->WorldToScreen(::CPoint2d{(double)m_drag.start.x, (double)m_drag.start.y});
        current = viewTransform->WorldToScreen(::CPoint2d{(double)m_drag.current.x, (double)m_drag.current.y});
    }
    
    // Create selection box rectangle
    CRect box(static_cast<int>(start.x), static_cast<int>(start.y), static_cast<int>(current.x), static_cast<int>(current.y));
    box.NormalizeRect();
    
    // Save DC state
    int savedDC = pDC->SaveDC();
    
    // Draw dashed rectangle for selection box
    CPen pen(PS_DASH, 1, RGB(0, 120, 215));  // Blue dashed line
    CPen* oldPen = pDC->SelectObject(&pen);
    
    // Set transparent brush (no fill)
    CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
    
    // Set ROP2 for XOR drawing (so we can undraw easily)
    int oldROP = pDC->SetROP2(R2_NOTXORPEN);
    
    // Draw the rectangle
    pDC->Rectangle(&box);
    
    // Restore DC state
    pDC->SetROP2(oldROP);
    pDC->SelectObject(oldBrush);
    pDC->SelectObject(oldPen);
    pDC->RestoreDC(savedDC);
}

} // namespace DigitMode

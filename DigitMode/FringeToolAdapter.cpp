/**
 * @file FringeToolAdapter.cpp
 * @brief Implementation of FringeToolAdapter
 */

#include "stdafx.h"
#include "FringeToolAdapter.h"
#include "FringeInputHandler.h"
#include "TooltipGenerator.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/HitTester.h"
#include "ImageTempl/ViewTransform.h"

namespace DigitMode {

FringeToolAdapter::FringeToolAdapter(FringeInputHandler* fringeInputHandler)
    : InputHandlerAdapter(fringeInputHandler, "FringeTool")
    , m_fringeInputHandler(fringeInputHandler)
{
    ASSERT(fringeInputHandler && "FringeInputHandler must not be null");
}

HitResult FringeToolAdapter::HitTest(CPoint screenPt, int tolerance)
{
    if (!m_fringeInputHandler) {
        return HitResult();
    }

    // Delegate to FringeInputHandler's hit-test capability
    CPoint worldPt = screenPt;
    if (m_fringeInputHandler->GetViewTransform()) {
        worldPt = m_fringeInputHandler->GetViewTransform()->ScreenToWorld(screenPt);
    }

    int hitSeg = -1, hitDot = -1;
    HitTester tester;
    SelectionLevel level = tester.HitTest(worldPt, hitSeg, hitDot, 
        m_fringeInputHandler->GetDigitInfo()->Fringes);

    // Convert FringeInputHandler::HitResult to DigitMode::HitResult
    HitResult result;
    result.hit = (level != SelectionLevel::None);
    result.distance = 0;  // Distance calculation not implemented
    result.toolContext = nullptr;  // FringeInputHandler doesn't provide context, set to null
    // Note: tool pointer is set by manager, not here

    return result;
}


// ========================================================================
// Mouse Event Handling (Hover Tracking)
// ========================================================================

void FringeToolAdapter::OnMouseMove(const ToolContext& ctx)
{
    // Forward to base implementation first (updates handler state including hover)
    InputHandlerAdapter::OnMouseMove(ctx);
    
    // Get hover state from InputHandler for tooltip generation
    if (m_fringeInputHandler) {
        m_hoveredObject = m_fringeInputHandler->GetInputHandler().GetHoveredObject();
    } else {
        m_hoveredObject.level = SelectionLevel::None;
    }
}

// ========================================================================
// View State (Visual Rendering)
// ========================================================================

IInteractionTool::ViewState FringeToolAdapter::GetViewState(bool isActive, bool isCapturing) const
{
    ViewState state;
    
    if (!m_fringeInputHandler) {
        return state;
    }
    
    // Note: FringeInputHandler visualization is complex and currently
    // handled via legacy drawing methods (DrawDigitInfo).
    // This adapter provides the interface for future integration.
    //
    // When FringeTool is active, the main visualization is handled by:
    // - CImageView::DrawDigitInfo() for dots and fringes
    // - CImageView::DrawSelectionBox() for navigate mode selection
    // 
    // This can be enhanced in Phase 2 if needed for unified rendering.
    
    // ================================================================
    // Tooltip (for hovered fringe element)
    // ================================================================
    
    if (m_hoveredObject.IsValid() && m_hoveredObject.level != SelectionLevel::None) {
        // Get CDigitInfo from FringeInputHandler to generate tooltip
        auto* pDigit = m_fringeInputHandler->GetDigitInfo();
        
        if (pDigit) {
            TooltipGenerator tooltipGen;
            std::string tooltip = tooltipGen.GetTooltip(m_hoveredObject, *pDigit);
            if (!tooltip.empty()) {
                state.tooltip = CString(tooltip.c_str());
            }
        }
    }
    
    return state;
}

}  // namespace DigitMode

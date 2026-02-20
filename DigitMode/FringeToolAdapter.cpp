/**
 * @file FringeToolAdapter.cpp
 * @brief Implementation of FringeToolAdapter
 */

#include "stdafx.h"
#include "FringeToolAdapter.h"
#include "FringeInputHandler.h"

namespace DigitMode {

FringeToolAdapter::FringeToolAdapter(FringeInputHandler* fringeInputHandler)
    : InputHandlerAdapter(fringeInputHandler, "FringeTool")
    , m_fringeInputHandler(fringeInputHandler)
{
    ASSERT(fringeInputHandler && "FringeInputHandler must not be null");
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
    
    return state;
}

}  // namespace DigitMode

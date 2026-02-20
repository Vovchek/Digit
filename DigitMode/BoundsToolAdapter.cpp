/**
 * @file BoundsToolAdapter.cpp
 * @brief Implementation of BoundsToolAdapter
 */

#include "stdafx.h"
#include "BoundsToolAdapter.h"
#include "BoundsInputHandler.h"
#include "BoundsHandler.h"
#include "DigitMode/Rendering/ShapeDrawStyle.h"

namespace DigitMode {

BoundsToolAdapter::BoundsToolAdapter(
    BoundsInputHandler* boundsInputHandler,
    BoundsHandler* boundsHandler)
    : InputHandlerAdapter(boundsInputHandler, "BoundsTool")
    , m_boundsHandler(boundsHandler)
{
    ASSERT(boundsInputHandler && "BoundsInputHandler must not be null");
    ASSERT(boundsHandler && "BoundsHandler must not be null");
}

// ========================================================================
// Hit-Testing
// ========================================================================

HitResult BoundsToolAdapter::HitTest(CPoint screenPt, int tolerance)
{
    if (!m_boundsHandler) {
        return HitResult();
    }
    
    // Delegate to BoundsHandler's hit-test capability
    auto boundsHit = m_boundsHandler->HitTest(screenPt, tolerance);
    
    // Convert BoundsHandler::HitResult to DigitMode::HitResult
    HitResult result;
    result.hit = boundsHit.hit;
    result.distance = boundsHit.distance;
    result.toolContext = nullptr;  // BoundsHandler doesn't provide context, set to null
    // Note: tool pointer is set by manager, not here
    
    return result;
}

// ========================================================================
// View State (Visual Rendering)
// ========================================================================

IInteractionTool::ViewState BoundsToolAdapter::GetViewState(bool isActive, bool isCapturing) const
{
    ViewState state;
    
    if (!m_boundsHandler) {
        return state;
    }
    
    // ================================================================
    // Draft Preview (Point-sequence creation)
    // ================================================================
    
    if (m_boundsHandler->IsDrafting()) {
        const auto* preview = m_boundsHandler->GetDraftPreview();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Draft;
            style.type = m_boundsHandler->GetShapeType();
            style.showHandles = false;
            state.shapes.push_back({preview, style});
        }
    }
    
    // ================================================================
    // Drag Preview (Move/resize handles, body drag)
    // ================================================================
    
    if (m_boundsHandler->IsDragging() && isCapturing) {
        const auto* preview = m_boundsHandler->GetPreviewShape();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Selected;
            style.type = m_boundsHandler->GetHoveredShapeType();
            style.showHandles = true;
            state.shapes.push_back({preview, style});
        }
    }
    
    return state;
}

}  // namespace DigitMode

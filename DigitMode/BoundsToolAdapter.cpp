/**
 * @file BoundsToolAdapter.cpp
 * @brief Implementation of BoundsToolAdapter
 */

#include "stdafx.h"
#include "BoundsToolAdapter.h"
#include "BoundsInputHandler.h"
#include "BoundsHandler.h"
#include "DigitMode/Rendering/ShapeDrawStyle.h"
#include "ApertureCore\include\aperturecore\geometry\Rectangle.h"
#include "ApertureCore\include\aperturecore\geometry\Ellipse.h"
#include "ApertureCore\include\aperturecore\geometry\Polygon.h"

namespace DigitMode {

BoundsToolAdapter::BoundsToolAdapter(
    BoundsInputHandler* boundsInputHandler)
    : InputHandlerAdapter(boundsInputHandler, "BoundsTool")
    , m_boundsInputHandler(boundsInputHandler)
{
    ASSERT(boundsInputHandler && "BoundsInputHandler must not be null");
}

// ========================================================================
// Mouse Event Handling (Hover Tracking)
// ========================================================================

void BoundsToolAdapter::OnMouseMove(const ToolContext& ctx)
{
    // Forward to base implementation first (updates handler state including hover)
    InputHandlerAdapter::OnMouseMove(ctx);

    // Get hover state from InputHandler for tooltip generation
    if (m_boundsInputHandler) {
		m_boundsInputHandler->GetBoundsHandler().UpdateHoveredHandle(ctx.screenPoint);
        m_boundsInputHandler->GetBoundsHandler().GetHoveredShape();
    }
    else {
        m_hoveredShape = nullptr;
    }
}


// ========================================================================
// Hit-Testing
// ========================================================================

HitResult BoundsToolAdapter::HitTest(CPoint screenPt, int tolerance)
{
    if (!m_boundsInputHandler) {
        return HitResult();
    }
    
    // Delegate to BoundsHandler's hit-test capability
    auto boundsHit = m_boundsInputHandler->GetBoundsHandler().HitTest(screenPt, tolerance);
    
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
    
    if (!m_boundsInputHandler) {
        return state;
    }
    
    // ================================================================
    // Draft Preview (Point-sequence creation)
    // ================================================================
    
    if (m_boundsInputHandler->GetBoundsHandler().IsDrafting()) {
        const auto* preview = m_boundsInputHandler->GetBoundsHandler().GetDraftPreview();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Draft;
            style.type = m_boundsInputHandler->GetBoundsHandler().GetShapeType();
            style.showHandles = false;
            state.shapes.push_back({preview, style});
        }
    }
    
    // ================================================================
    // Drag Preview (Move/resize handles, body drag)
    // ================================================================
    
    if (m_boundsInputHandler->GetBoundsHandler().IsDragging() && isCapturing) {
        const auto* preview = m_boundsInputHandler->GetBoundsHandler().GetPreviewShape();
        if (preview) {
            ShapeDrawStyle style;
            style.state = ShapeDrawStyle::State::Selected;
            style.type = m_boundsInputHandler->GetBoundsHandler().GetHoveredShapeType();
            style.showHandles = true;
            state.shapes.push_back({preview, style});
        }
    }
    
    // ================================================================
    // Tooltip (for hovered shape)
    // ================================================================
    
    const auto* hoveredShape = m_boundsInputHandler->GetBoundsHandler().GetHoveredShape();
    if (hoveredShape) {
        // Format: "[A|X|O] [E|R|P] WxH"
        CString tooltip;
        
        // Type: A=Aperture, X=External, O=Obstruction (INTERNAL)
        auto shapeType = m_boundsInputHandler->GetBoundsHandler().GetHoveredShapeType();
        char typeChar = 'X';  // default to External
        if (shapeType == aperture::TypeLimits::APERTURE) {
            typeChar = 'A';
        } else if (shapeType == aperture::TypeLimits::INTERNAL) {
            typeChar = 'O';
        }
        
        // Shape: E=Ellipse, R=Rectangle, P=Polygon
        char shapeChar = 'P';  // default to Polygon
        if (auto* rect = dynamic_cast<const aperture::Rectangle*>(hoveredShape)) {
            shapeChar = 'R';
        } else if (auto* ellipse = dynamic_cast<const aperture::Ellipse*>(hoveredShape)) {
            shapeChar = 'E';
        }
        
        // Dimensions: Width x Height of bounding box
        auto bounds = hoveredShape->getBounds();
        double width = bounds.width();
        double height = bounds.height();
        
        tooltip.Format(_T("%c %c %.0fx%.0f"), typeChar, shapeChar, width, height);
        state.tooltip = tooltip;
    }
    
    return state;
}

}  // namespace DigitMode

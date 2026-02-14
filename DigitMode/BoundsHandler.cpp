/**
 * @file BoundsHandler.cpp
 * @brief Implementation of BoundsHandler with strict Command pattern
 * 
 * IMPORTANT:
 * Shape geometry MUST NOT be modified outside Command::Execute/Undo.
 * BoundsHandler operates on preview copies only.
 */
#include "stdafx.h"
#include "BoundsHandler.h"
#include "Controls\CApertureCtrls.h"
#include "Commands\ReplaceShapeCommand.h"
#include "CommandDispatcher.h"
#include "ApertureCore\include\aperturecore\geometry\Handle.h"

namespace DigitMode {

BoundsHandler::BoundsHandler()
    : m_pApertureCtrls(nullptr)
    , m_pImage(nullptr)
    , m_pView(nullptr)
    , m_pDispatcher(nullptr)
    , m_isDragging(false)
    , m_dragShapeType(aperture::TypeLimits::EXTERNAL)
    , m_dragShapeIndex(0)
    , m_dragControlPointIndex(-1)
{
}

BoundsHandler::~BoundsHandler()
{
    // No cleanup needed (pointers are not owned)
}

// ========================================================================
// Initialization
// ========================================================================

void BoundsHandler::SetApertureCtrls(CApertureCtrls* pApertureCtrls, IImageData* pImage)
{
    m_pApertureCtrls = pApertureCtrls;
    m_pImage = pImage;
}

void BoundsHandler::SetViewTransform(ViewTransform* pView)
{
    m_pView = pView;
}

void BoundsHandler::SetCommandDispatcher(CommandDispatcher* pDispatcher)
{
    m_pDispatcher = pDispatcher;
}

// ========================================================================
// Coordinate Transforms
// ========================================================================

CPoint2d BoundsHandler::ScreenToWorldDouble(const CPoint& screenPt) const
{
    ASSERT(m_pView && "ViewTransform must be set before coordinate conversion");
    
    // Get current transform parameters
    double scale = m_pView->GetScale();
    CPoint2d offset = m_pView->GetOffset();
    
    // Convert to world coordinates with double precision
    CPoint2d worldPt;
    worldPt.x = (screenPt.x - offset.x) / scale;
    worldPt.y = (screenPt.y - offset.y) / scale;
    
    return worldPt;
}

// ========================================================================
// Hit-Testing (Delegates to CApertureCtrls)
// ========================================================================

BoundsHandler::HitResult BoundsHandler::HitTest(const CPoint& screenPt, int tolerance) const
{
    HitResult result;
    
    if (!m_pApertureCtrls || !m_pView) {
        return result;  // Not initialized
    }
    
    // Convert screen point to world coordinates
    aperture::Point worldPt = ScreenToAperturePoint(screenPt);
    
    // Convert screen tolerance to world tolerance
    double scale = m_pView->GetScale();
    double worldTolerance = tolerance / scale;
    
    // Delegate to CApertureCtrls hit-testing
    auto apertureResult = m_pApertureCtrls->HitTest(worldPt, worldTolerance);
    
    if (apertureResult.hitShape()) {
        result.hit = true;
        result.type = apertureResult.type;
        result.shapeIndex = static_cast<size_t>(m_pApertureCtrls->GetShapeIndex(apertureResult.shape));
        result.controlPointIndex = apertureResult.controlPointIndex;
        result.distance = apertureResult.distance;
    }
    
    return result;
}

// ========================================================================
// Drag Lifecycle (Preview + Command Pattern)
// ========================================================================

void BoundsHandler::BeginDrag(aperture::TypeLimits type, size_t index, int controlPointIndex, const CPoint& screenStart)
{
    ASSERT(m_pApertureCtrls && m_pView && "BoundsHandler must be initialized");
    
    if (m_isDragging) {
        return;  // Already dragging
    }

    // Get the shape at (type, index)
    const std::vector<std::unique_ptr<aperture::Shape>>* container = nullptr;
    
    switch (type) {
        case aperture::TypeLimits::EXTERNAL:
            container = &m_pApertureCtrls->GetShapes().getExternal();
            break;
        case aperture::TypeLimits::INTERNAL:
            container = &m_pApertureCtrls->GetShapes().getInternal();
            break;
        case aperture::TypeLimits::APERTURE:
            container = &m_pApertureCtrls->GetShapes().getApertures();
            break;
    }
    
    if (!container || index >= container->size()) {
        return;  // Invalid shape reference
    }
    
    const auto& shape = (*container)[index];
    
    // Clone shape for before-state and preview
    m_originalShape = shape->clone();
    m_previewShape = shape->clone();
    
    // Cache drag info
    m_dragShapeType = type;
    m_dragShapeIndex = index;
    m_dragControlPointIndex = controlPointIndex;
    m_dragStart = screenStart;
    m_dragCurrent = screenStart;
    m_isDragging = true;
}

void BoundsHandler::UpdateDrag(const CPoint& screenCurrent)
{
    if (!m_isDragging || !m_previewShape) {
        return;
    }

    // Compute world-space positions
    aperture::Point dragStartWorld = ScreenToAperturePoint(m_dragStart);
    aperture::Point dragCurrentWorld = ScreenToAperturePoint(screenCurrent);
    
    aperture::Point deltaWorld{
        dragCurrentWorld.x - dragStartWorld.x,
        dragCurrentWorld.y - dragStartWorld.y
    };
    
    // Reset preview to original state
    m_previewShape = m_originalShape->clone();
    
    // Apply drag to preview using shape's handle system
    // TODO: Enumerate handles and apply drag based on controlPointIndex
    // For now, this is a placeholder
    
    // Create DragContext
    aperture::DragContext dragContext;
    dragContext.dragStartWorld = dragStartWorld;
    dragContext.dragCurrentWorld = dragCurrentWorld;
    dragContext.deltaWorld = deltaWorld;
    dragContext.shiftKey = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    dragContext.altKey = (GetKeyState(VK_MENU) & 0x8000) != 0;
    
    // Enumerate handles to find the one being dragged
    std::vector<aperture::HandleDesc> handles;
    m_previewShape->EnumerateHandles(handles);
    
    if (m_dragControlPointIndex >= 0 && static_cast<size_t>(m_dragControlPointIndex) < handles.size()) {
        aperture::HandleDesc& handle = handles[m_dragControlPointIndex];
        dragContext.handle = handle;
        
        // Apply drag to preview
        m_previewShape->ApplyHandleDrag(handle, dragContext);
    }
    
    m_dragCurrent = screenCurrent;
    
    // Invalidate view to show preview (caller's responsibility to redraw)
}

void BoundsHandler::EndDrag(bool bCommit)
{
    if (!m_isDragging) {
        return;
    }

    if (bCommit && m_pDispatcher && m_originalShape && m_previewShape) {
        // Create ReplaceShapeCommand and dispatch
        auto cmd = std::make_unique<ReplaceShapeCommand>(
            *m_pApertureCtrls,
            m_dragShapeType,
            m_dragShapeIndex,
            std::move(m_originalShape),
            std::move(m_previewShape)
        );
        
        m_pDispatcher->Execute(std::move(cmd));
    }
    
    // Clear preview state
    m_previewShape.reset();
    m_originalShape.reset();
    m_isDragging = false;
    m_dragControlPointIndex = -1;
}

void BoundsHandler::CancelDrag()
{
    EndDrag(false);  // Discard preview
}

} // namespace DigitMode

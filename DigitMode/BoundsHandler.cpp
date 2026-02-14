#include "stdafx.h"
#include "BoundsHandler.h"
#include "Controls\CApertureCtrls.h"

namespace DigitMode {

BoundsHandler::BoundsHandler()
    : m_pApertureCtrls(nullptr), m_pImage(nullptr), m_pView(nullptr), m_isDragging(false)
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
        result.shapeIndex = apertureResult.shape.index;
        result.controlPointIndex = apertureResult.controlPointIndex;
        result.distance = apertureResult.distance;
    }
    
    return result;
}

// ========================================================================
// Drag Lifecycle (Delegates to CApertureCtrls)
// ========================================================================

void BoundsHandler::BeginDrag(size_t shapeIndex, int controlPointIndex, const CPoint& screenStart)
{
    ASSERT(m_pApertureCtrls && m_pView && "BoundsHandler must be initialized");
    if (m_isDragging) {
        return;
    }

    m_dragStart = screenStart;
    m_dragCurrent = screenStart;
    m_isDragging = true;
    
    // Delegate to CApertureCtrls
    ShapeHandle handle{shapeIndex, aperture::TypeLimits::EXTERNAL};  // TODO: get type from hit result
    m_pApertureCtrls->BeginEdit(handle, controlPointIndex);
}

void BoundsHandler::UpdateDrag(const CPoint& screenCurrent)
{
    if (!m_isDragging || !m_pApertureCtrls) {
        return;
    }

    // Compute world-space delta from drag start
    CPoint2d worldStart = ScreenToWorldDouble(m_dragStart);
    CPoint2d worldCurr = ScreenToWorldDouble(screenCurrent);
    
    aperture::Point worldDelta{
        worldCurr.x - worldStart.x,
        worldCurr.y - worldStart.y
    };
    
    // Delegate to CApertureCtrls
    m_pApertureCtrls->UpdateEdit(worldDelta);
    
    m_dragCurrent = screenCurrent;
}

void BoundsHandler::EndDrag(bool bCommit)
{
    if (!m_isDragging || !m_pApertureCtrls) {
        return;
    }

    // Delegate to CApertureCtrls
    if (bCommit) {
        m_pApertureCtrls->CommitEdit();
    } else {
        m_pApertureCtrls->CancelEdit();
    }

    m_isDragging = false;
}

void BoundsHandler::CancelDrag()
{
    EndDrag(false);
}

} // namespace DigitMode

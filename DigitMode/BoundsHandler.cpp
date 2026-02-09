#include "stdafx.h"
#include "BoundsHandler.h"
#include "Controls/BoundCtrls.h"
#include "Controls/ImageCtrls.h"

namespace DigitMode {

BoundsHandler::BoundsHandler()
    : m_pBounds(nullptr), m_pImage(nullptr), m_pView(nullptr)
{
}

BoundsHandler::~BoundsHandler()
{
    // No cleanup needed (pointers are not owned)
}

// ========================================================================
// Initialization
// ========================================================================

void BoundsHandler::SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage)
{
    m_pBounds = pBounds;
    m_pImage = pImage;
}

void BoundsHandler::SetViewTransform(ViewTransform* pView)
{
    m_pView = pView;
}

// ========================================================================
// Coordinate Transforms
// ========================================================================

CPoint BoundsHandler::ScreenToWorld(const CPoint& screenPt) const
{
    ASSERT(m_pView && "ViewTransform must be set before coordinate conversion");
    return m_pView->ScreenToWorld(screenPt);
}

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

CPoint BoundsHandler::WorldToScreen(const CPoint2d& worldPt) const
{
    ASSERT(m_pView && "ViewTransform must be set before coordinate conversion");
    return m_pView->WorldToScreen(worldPt);
}

} // namespace DigitMode

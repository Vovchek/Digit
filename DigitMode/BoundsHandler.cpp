#include "stdafx.h"
#include "BoundsHandler.h"
#include "AppDef.h"  // For BOUND_RECT, BOUND_ROUND, etc.
#include <cmath>

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

void BoundsHandler::SetBoundsData(IBoundsData* pBounds, IImageData* pImage)
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

// ========================================================================
// Hit-Testing
// ========================================================================

bool BoundsHandler::HitTestBoundHandle(const CPoint& screenPt,
                                       int& outBoundIdx,
                                       int& outHandleIdx) const
{
    ASSERT(m_pBounds && m_pImage && m_pView && "BoundsHandler must be initialized");
    
    outBoundIdx = -1;
    outHandleIdx = -1;
    
    // Get image dimensions via interface
    CSize imageSize = m_pImage->GetImageSize();
    int xDIB = imageSize.cx;
    int yDIB = imageSize.cy;
    
    if (xDIB == 0 || yDIB == 0) {
        return false;  // No image loaded
    }
    
    // Test external bound (boundIdx = 0) via interface
    int extBoundType = m_pBounds->GetExtBoundType();
    if (extBoundType != -1) {
        CRect bound;
        CArray<CPoint, CPoint> plgPoints;
        
        if (m_pBounds->GetExtRealBound(extBoundType, xDIB, yDIB, bound, plgPoints)) {
            // For RECT, ROUND, ELLIPSE: test 4 corner handles
            if (extBoundType == BOUND_RECT || 
                extBoundType == BOUND_ROUND || 
                extBoundType == BOUND_ELLIPSE) {
                
                // Test each corner handle
                for (int h = 0; h < 4; ++h) {
                    CPoint2d handleWorld = GetHandleWorldPos(bound, h);
                    CPoint handleScreen = WorldToScreen(handleWorld);
                    
                    if (IsNearHandle(screenPt, handleScreen, HANDLE_TOLERANCE)) {
                        outBoundIdx = 0;  // External bound
                        outHandleIdx = h;
                        return true;
                    }
                }
            }
            // TODO: For POLYGON, test each vertex as a handle
        }
    }
    
    // TODO: Test internal bounds (boundIdx = 1, 2, ...)
    
    return false;
}

// ========================================================================
// Drag Lifecycle
// ========================================================================

void BoundsHandler::BeginDrag(int boundIdx, int handleIdx, const CPoint& screenStart)
{
    ASSERT(m_pBounds && m_pImage && m_pView && "BoundsHandler must be initialized");
    if (m_isDragging) {
        return;
    }

    m_boundIndex = boundIdx;
    m_handleIndex = handleIdx;
    m_dragStart = screenStart;
    m_dragCurrent = screenStart;
    m_boundSnapshot = GetCurrentBound(boundIdx);
    m_previewBound = m_boundSnapshot;

    if (!m_boundSnapshot.IsRectEmpty()) {
        m_isDragging = true;
    }
}

void BoundsHandler::UpdateDrag(const CPoint& screenCurrent)
{
    if (!m_isDragging) {
        return;
    }

    m_dragCurrent = screenCurrent;

    CPoint2d worldStart = ScreenToWorldDouble(m_dragStart);
    CPoint2d worldCurr = ScreenToWorldDouble(screenCurrent);

    double worldDx = worldCurr.x - worldStart.x;
    double worldDy = worldCurr.y - worldStart.y;

    m_previewBound = ComputeNewBoundFromDrag(m_boundSnapshot, m_handleIndex, worldDx, worldDy);
}

void BoundsHandler::EndDrag(bool bCommit)
{
    if (!m_isDragging) {
        return;
    }

    if (bCommit) {
        m_boundSnapshot = m_previewBound;
    } else {
        m_previewBound = m_boundSnapshot;
    }

    m_isDragging = false;
    m_boundIndex = -1;
    m_handleIndex = -1;
}

void BoundsHandler::CancelDrag()
{
    EndDrag(false);
}

// ========================================================================
// Internal Helpers
// ========================================================================

CRect BoundsHandler::GetCurrentBound(int boundIdx) const
{
    if (!m_pBounds || !m_pImage || boundIdx != 0) {
        return CRect(0, 0, 0, 0);
    }

    CSize imageSize = m_pImage->GetImageSize();
    if (imageSize.cx == 0 || imageSize.cy == 0) {
        return CRect(0, 0, 0, 0);
    }

    int boundType = m_pBounds->GetExtBoundType();
    if (boundType == -1) {
        return CRect(0, 0, 0, 0);
    }

    CRect bound;
    CArray<CPoint, CPoint> plgPoints;
    if (!m_pBounds->GetExtRealBound(boundType, imageSize.cx, imageSize.cy, bound, plgPoints)) {
        return CRect(0, 0, 0, 0);
    }

    return bound;
}

CRect BoundsHandler::ComputeNewBoundFromDrag(const CRect& original,
                                             int handleIdx,
                                             double worldDx,
                                             double worldDy) const
{
    CRect newBound = original;
    int dx = static_cast<int>(std::lround(worldDx));
    int dy = static_cast<int>(std::lround(worldDy));

    switch (handleIdx) {
        case 0:
            newBound.left += dx;
            newBound.top += dy;
            break;
        case 1:
            newBound.right += dx;
            newBound.top += dy;
            break;
        case 2:
            newBound.right += dx;
            newBound.bottom += dy;
            break;
        case 3:
            newBound.left += dx;
            newBound.bottom += dy;
            break;
        default:
            break;
    }

    if (newBound.left >= newBound.right) {
        newBound.left = newBound.right - 1;
    }
    if (newBound.top >= newBound.bottom) {
        newBound.top = newBound.bottom - 1;
    }

    return newBound;
}

// ========================================================================
// Internal Helpers
// ========================================================================

CPoint2d BoundsHandler::GetHandleWorldPos(const CRect& bound, int handleIdx) const
{
    switch (handleIdx) {
        case 0: return CPoint2d{(double)bound.left, (double)bound.top};      // TL
        case 1: return CPoint2d{(double)bound.right, (double)bound.top};     // TR
        case 2: return CPoint2d{(double)bound.right, (double)bound.bottom};  // BR
        case 3: return CPoint2d{(double)bound.left, (double)bound.bottom};   // BL
        default: 
            ASSERT(false && "Invalid handle index");
            return CPoint2d{0, 0};
    }
}

bool BoundsHandler::IsNearHandle(const CPoint& screenPt,
                                 const CPoint& handleScreenPos,
                                 int tolerance) const
{
    int dx = screenPt.x - handleScreenPos.x;
    int dy = screenPt.y - handleScreenPos.y;
    int distSq = dx*dx + dy*dy;
    int toleranceSq = tolerance * tolerance;
    
    return distSq <= toleranceSq;
}

} // namespace DigitMode

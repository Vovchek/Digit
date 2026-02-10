#pragma once

#include <afxwin.h>
#include "ImageTempl/ViewTransform.h"

// Forward declarations
class CBoundCtrls;
class CImageCtrls;

namespace DigitMode {

/**
 * @brief Handles interactive editing of image bounds (apertures)
 * 
 * Phase 1 implementation:
 * - Step 1: ViewTransform coordinate conversion ✅
 * - Step 2: Hit-testing for bounds handles (in progress)
 * 
 * Future phases will add:
 * - Drag state management
 * - Rendering feedback
 */
class BoundsHandler {
public:
    BoundsHandler();
    ~BoundsHandler();
    
    // =========== Initialization ===========
    /**
     * Set pointers to required data/view objects
     * Must call before any interaction
     */
    void SetBoundsData(CBoundCtrls* pBounds, CImageCtrls* pImage);
    void SetViewTransform(ViewTransform* pView);
    
    // =========== Hit-Testing ===========
    /**
     * Check if screen point is near a bound handle
     * @param screenPt Point in client/screen coordinates
     * @param outBoundIdx Output: which bound (0-N, -1 if none)
     * @param outHandleIdx Output: which handle corner (0-3 for rect, -1 if none)
     *                     Handle indices: 0=TL, 1=TR, 2=BR, 3=BL
     * @return true if hit a handle; indices are valid
     */
    bool HitTestBoundHandle(const CPoint& screenPt, 
                            int& outBoundIdx, 
                            int& outHandleIdx) const;
    
    // =========== Coordinate Transforms ===========
    /**
     * Convert screen (client window) coordinates to world (image) coordinates
     * Returns integer world coordinates (matches ViewTransform::ScreenToWorld)
     */
    CPoint ScreenToWorld(const CPoint& screenPt) const;
    
    /**
     * Convert screen coordinates to world with double precision
     * Useful for precise calculations during drag operations
     */
    CPoint2d ScreenToWorldDouble(const CPoint& screenPt) const;
    
    /**
     * Convert world (image) coordinates to screen (client window) coordinates
     * Accounts for current zoom/pan in ViewTransform
     */
    CPoint WorldToScreen(const CPoint2d& worldPt) const;
    
    // =========== State Query ===========
    bool IsInitialized() const { return m_pView != nullptr; }

private:
    // =========== Internal Helpers ===========
    /**
     * Get handle position in world coordinates
     * @param bound Rectangle in world coordinates
     * @param handleIdx Corner index (0=TL, 1=TR, 2=BR, 3=BL)
     * @return Corner position in world coordinates
     */
    CPoint2d GetHandleWorldPos(const CRect& bound, int handleIdx) const;
    
    /**
     * Check if screen point is near a handle position
     * @param screenPt Screen point to test
     * @param handleScreenPos Screen position of handle
     * @param tolerance Pixel tolerance (default 5)
     */
    bool IsNearHandle(const CPoint& screenPt, 
                      const CPoint& handleScreenPos,
                      int tolerance = 5) const;
    
    // =========== State ===========
    CBoundCtrls* m_pBounds = nullptr;
    CImageCtrls* m_pImage = nullptr;
    ViewTransform* m_pView = nullptr;
    
    // Constants
    static constexpr int HANDLE_TOLERANCE = 5;  // Pixels for hit testing
};

} // namespace DigitMode

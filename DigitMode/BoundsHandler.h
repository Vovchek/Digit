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
 * Phase 1 implementation focuses on:
 * - ViewTransform coordinate conversion
 * - Basic structure for future expansion
 * 
 * Future phases will add:
 * - Hit-testing for handles/dots
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
    // =========== State ===========
    CBoundCtrls* m_pBounds = nullptr;
    CImageCtrls* m_pImage = nullptr;
    ViewTransform* m_pView = nullptr;
};

} // namespace DigitMode

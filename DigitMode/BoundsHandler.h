/**
 * @file BoundsHandler.h
 * @brief Handles interactive editing of aperture shapes using CApertureCtrls
 * 
 * Aligned with fringe editor pattern:
 * - Stateless hit-testing delegation to CApertureCtrls
 * - Drag lifecycle management (BeginDrag/UpdateDrag/EndDrag)
 * - Coordinate transforms (screen ↔ world)
 * 
 * @note Editing state lives in CApertureCtrls (BeginEdit/CommitEdit)
 */
#pragma once

#include <afxwin.h>
#include "ImageTempl/ViewTransform.h"
#include "ApertureCore\include\aperturecore\geometry\Point.h"

// Forward declarations
namespace DigitMode {
    class CApertureCtrls;
}
class IImageData;

namespace DigitMode {

/**
 * @brief Handles interactive editing of image bounds (apertures)
 * 
 * Responsibilities:
 * - Coordinate transformation (screen → world → aperture)
 * - Delegate hit-testing to CApertureCtrls
 * - Manage drag lifecycle (delegates to CApertureCtrls editing)
 * - NO bounds calculation logic (that's in CApertureCtrls)
 * 
 * Aligned with fringe editor pattern (HitTester + InputHandler)
 */
class BoundsHandler {
public:
    BoundsHandler();
    ~BoundsHandler();
    
    // ========================================================================
    // Initialization
    // ========================================================================
    
    /**
     * Set aperture controls and image data
     */
    void SetApertureCtrls(CApertureCtrls* pApertureCtrls, IImageData* pImage);
    
    /**
     * Set view transform for coordinate conversions
     */
    void SetViewTransform(ViewTransform* pView);
    
    // ========================================================================
    // Hit-Testing (delegates to CApertureCtrls)
    // ========================================================================
    
    /**
     * @brief Hit-test result structure
     */
    struct HitResult {
        bool hit = false;              ///< True if anything was hit
        size_t shapeIndex = 0;         ///< Index in type-specific container
        int controlPointIndex = -1;   ///< Control point index (-1 = body)
        double distance = 0.0;         ///< Distance from hit point
        
        bool isControlPoint() const { return hit && controlPointIndex >= 0; }
        bool isBody() const { return hit && controlPointIndex == -1; }
    };
    
    /**
     * @brief Hit-test using CApertureCtrls shape-based API
     * @param screenPt Point in screen coordinates
     * @param tolerance Hit tolerance in screen pixels
     * @return Hit result structure
     */
    HitResult HitTest(const CPoint& screenPt, int tolerance = HANDLE_TOLERANCE) const;

    // ========================================================================
    // Drag Lifecycle (delegates to CApertureCtrls editing)
    // ========================================================================
    
    /**
     * Begin dragging a shape control point
     */
    void BeginDrag(size_t shapeIndex, int controlPointIndex, const CPoint& screenStart);
    
    /**
     * Update drag position
     */
    void UpdateDrag(const CPoint& screenCurrent);
    
    /**
     * End drag - commit or cancel
     */
    void EndDrag(bool bCommit);
    
    /**
     * Cancel current drag operation
     */
    void CancelDrag();
    
    // ========================================================================
    // Coordinate Transforms
    // ========================================================================
    
    /**
     * Convert screen coordinates to world with double precision
     */
    CPoint2d ScreenToWorldDouble(const CPoint& screenPt) const;
    
    /**
     * Convert world coordinates to ApertureCore Point
     */
    aperture::Point WorldToAperturePoint(const CPoint2d& worldPt) const {
        return aperture::Point{worldPt.x, worldPt.y};
    }
    
    /**
     * Convert screen directly to ApertureCore Point
     */
    aperture::Point ScreenToAperturePoint(const CPoint& screenPt) const {
        CPoint2d world = ScreenToWorldDouble(screenPt);
        return WorldToAperturePoint(world);
    }
    
    // ========================================================================
    // State Query
    // ========================================================================
    
    bool IsInitialized() const { return m_pView != nullptr && m_pApertureCtrls != nullptr; }
    bool IsDragging() const { return m_isDragging; }

private:
    // Modern mode only
    CApertureCtrls* m_pApertureCtrls = nullptr;
    IImageData* m_pImage = nullptr;
    ViewTransform* m_pView = nullptr;

    // Drag state
    bool m_isDragging = false;
    CPoint m_dragStart;
    CPoint m_dragCurrent;
    
    // Constants
    static constexpr int HANDLE_TOLERANCE = 5;  // Pixels for hit testing
};

} // namespace DigitMode

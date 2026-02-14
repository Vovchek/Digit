/**
 * @file BoundsHandler.h
 * @brief Handles interactive editing of aperture shapes using Command pattern
 * 
 * STRICT COMMAND-BASED EDITING:
 * - Preview-only drag handling (no direct shape mutation)
 * - Commands dispatched on mouse-up only
 * - BoundsHandler owns preview state, not editing state
 * 
 * IMPORTANT:
 * Shape geometry MUST NOT be modified outside Command::Execute/Undo.
 * BoundsHandler operates on preview copies only.
 */
#pragma once

#include <afxwin.h>
#include "ImageTempl/ViewTransform.h"
#include "ApertureCore\include\aperturecore\geometry\Point.h"
#include "ApertureCore\include\aperturecore\geometry\Shape.h"
#include "ApertureCore\include\aperturecore\visibility\TypeLimits.h"
#include <memory>

// Forward declarations
namespace DigitMode {
    class CApertureCtrls;
    class CommandDispatcher;
}
class IImageData;

namespace DigitMode {

/**
 * @brief Handles interactive editing of image bounds (apertures)
 * 
 * Command-Based Architecture:
 * 1. MouseDown → cache (type, index), clone shape → preview
 * 2. MouseMove → apply drag to preview, invalidate (no mutation)
 * 3. MouseUp → create ReplaceShapeCommand(before, preview), dispatch
 * 4. Esc → discard preview
 * 
 * FORBIDDEN:
 * - Direct shape mutation
 * - Storing persistent shape pointers/handles
 * - Modifying ShapeCollection without commands
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
    
    /**
     * Set command dispatcher for undo/redo
     */
    void SetCommandDispatcher(CommandDispatcher* pDispatcher);
    
    // ========================================================================
    // Hit-Testing (delegates to CApertureCtrls)
    // ========================================================================
    
    /**
     * @brief Hit-test result structure
     */
    struct HitResult {
        bool hit = false;              ///< True if anything was hit
        aperture::TypeLimits type = aperture::TypeLimits::EXTERNAL;
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
    // Drag Lifecycle (Preview + Command Pattern)
    // ========================================================================
    
    /**
     * Begin dragging a shape control point
     * Creates preview clone, does NOT modify document
     */
    void BeginDrag(aperture::TypeLimits type, size_t index, int controlPointIndex, const CPoint& screenStart);
    
    /**
     * Update drag position
     * Modifies preview only, does NOT touch document
     */
    void UpdateDrag(const CPoint& screenCurrent);
    
    /**
     * End drag - commit or cancel
     * Commit: dispatches ReplaceShapeCommand
     * Cancel: discards preview
     */
    void EndDrag(bool bCommit);
    
    /**
     * Cancel current drag operation
     */
    void CancelDrag();
    
    /**
     * Get preview shape for rendering (nullptr if not dragging)
     */
    const aperture::Shape* GetPreviewShape() const { return m_previewShape.get(); }
    
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
    CommandDispatcher* m_pDispatcher = nullptr;

    // Preview state (Command pattern - NO direct mutation)
    bool m_isDragging = false;
    aperture::TypeLimits m_dragShapeType = aperture::TypeLimits::EXTERNAL;
    size_t m_dragShapeIndex = 0;
    int m_dragControlPointIndex = -1;
    std::unique_ptr<aperture::Shape> m_previewShape;     ///< Preview clone during drag
    std::unique_ptr<aperture::Shape> m_originalShape;    ///< Original before drag (for command)
    
    CPoint m_dragStart;
    CPoint m_dragCurrent;
    
    // Constants
    static constexpr int HANDLE_TOLERANCE = 5;  // Pixels for hit testing
};

} // namespace DigitMode

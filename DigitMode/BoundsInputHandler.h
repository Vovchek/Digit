/**
 * @file BoundsInputHandler.h
 * @brief Input handler for aperture bounds editing tool
 * 
 * ARCHITECTURAL ROLE:
 * BoundsInputHandler is a tool handler that implements IInputHandler.
 * It wraps BoundsHandler (the domain logic) and translates raw input
 * events into aperture editing gestures.
 * 
 * DESIGN PATTERN:
 * - BoundsInputHandler = IInputHandler adapter (input → gestures)
 * - BoundsHandler = Domain logic (hit-testing, drag, commands)
 * - CApertureCtrls = Document/collection owner
 * - CommandDispatcher = Command pattern coordinator
 * 
 * MODAL BEHAVIOR:
 * Internally manages edit modes (Select, AddRectangle, AddEllipse, etc.)
 * External callers switch modes via SetEditMode().
 * Navigation (pan/zoom) handled by NavigationInputHandler (fallback).
 */
#pragma once

#include "IInputHandler.h"
#include "BoundsHandler.h"
#include <afxwin.h>

// Forward declarations
class CApertureCtrls;
class CommandDispatcher;
class ViewTransform;
class IImageData;

namespace DigitMode {

/**
 * @brief Input handler for aperture bounds editing
 * 
 * Responsibilities:
 * - Translate raw input events to aperture editing operations
 * - Manage modal editing states (Select, Add modes)
 * - Delegate to BoundsHandler for domain logic
 * - Request view invalidation via callback
 * 
 * Constraints:
 * - MUST NOT call view methods directly
 * - MUST use BoundsHandler for all domain operations
 * - MUST NOT handle pan/zoom (navigation fallback handles that)
 * 
 * Input Consumption:
 * - Consumes events in Select mode (drag handles, click shapes)
 * - Consumes events in Add modes (click to add points)
 * - Returns false for unhandled gestures (allows navigation fallback)
 */
class BoundsInputHandler : public IInputHandler {
public:
    /**
     * @brief Construct bounds input handler
     * @param pView View for invalidation callback (not owned)
     */
    explicit BoundsInputHandler(CWnd* pView);
    ~BoundsInputHandler() override;
    
    // ========================================================================
    // Initialization (called by CImageView during setup)
    // ========================================================================
    
    /**
     * @brief Initialize with dependencies
     * @param pApertureCtrls Aperture controls (not owned)
     * @param pImage Image data provider (not owned)
     * @param pTransform View transform (not owned)
     * @param pDispatcher Command dispatcher (not owned)
     */
    void Initialize(
        CApertureCtrls* pApertureCtrls,
        IImageData* pImage,
        ViewTransform* pTransform,
        CommandDispatcher* pDispatcher
    );
    
    /**
     * @brief Check if handler is initialized
     */
    bool IsInitialized() const;
    
    // ========================================================================
    // Mode Management
    // ========================================================================
    
    /**
     * @brief Set current edit mode
     * @param mode New edit mode
     * 
     * Automatically cancels current operation when switching modes.
     */
    void SetEditMode(ShapeEditMode mode);
    
    /**
     * @brief Get current edit mode
     */
    ShapeEditMode GetEditMode() const;
    
    /**
     * @brief Get underlying BoundsHandler for rendering queries
     * 
     * Used by view to:
     * - Call RenderPreview() during OnDraw
     * - Query hover state for cursor updates
     */
    BoundsHandler& GetBoundsHandler() { return m_boundsHandler; }
    const BoundsHandler& GetBoundsHandler() const { return m_boundsHandler; }
    
    // ========================================================================
    // IInputHandler Implementation
    // ========================================================================
    
    bool OnMouseDown(UINT flags, CPoint pt) override;
    bool OnMouseMove(UINT flags, CPoint pt) override;
    bool OnMouseUp(UINT flags, CPoint pt) override;
    bool OnMouseWheel(UINT flags, short delta, CPoint pt) override;
    bool OnKeyDown(UINT nChar) override;
    bool OnKeyUp(UINT nChar) override;
    void Cancel() override;
    
private:
    // ========================================================================
    // Dependencies (not owned)
    // ========================================================================
    
    CWnd* m_pView;                    ///< View for invalidation
    
    // ========================================================================
    // Domain Logic
    // ========================================================================
    
    BoundsHandler m_boundsHandler;    ///< Underlying domain handler
    
    // ========================================================================
    // Helpers
    // ========================================================================
    
    /**
     * @brief Request view invalidation
     */
    void Invalidate();
    
    /**
     * @brief Handle mouse down in Select mode
     * @return true if event consumed
     */
    bool HandleSelectModeMouseDown(UINT flags, CPoint pt);
    
    /**
     * @brief Handle mouse down in Add modes (creation)
     * @return true if event consumed
     */
    bool HandleAddModeMouseDown(UINT flags, CPoint pt);
};

} // namespace DigitMode

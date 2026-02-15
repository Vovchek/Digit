/**
 * @file FringeInputHandler.h
 * @brief Input handler for fringe (contour) editing tool
 * 
 * ARCHITECTURAL ROLE:
 * FringeInputHandler is a tool handler that implements IInputHandler.
 * It wraps the existing InputHandler (fringe domain logic) and translates
 * raw input events into fringe editing gestures.
 * 
 * DESIGN PATTERN:
 * - FringeInputHandler = IInputHandler adapter (input → gestures)
 * - InputHandler = Domain logic (draw mode, dot edit, selection)
 * - CDigitInfo = Document owner (fringes, segments, dots)
 * - CommandDispatcher = Command pattern coordinator
 * 
 * MODAL BEHAVIOR:
 * Internally manages edit modes (Navigate, Draw, DotEdit).
 * External callers switch modes via SetMode().
 * Navigation (pan/zoom) returns false to allow NavigationInputHandler fallback.
 */
#pragma once

#include "IInputHandler.h"
#include "InputHandler.h"
#include <afxwin.h>

// Forward declarations
class CDigitInfo;
class CommandDispatcher;
class ViewTransform;

namespace DigitMode {

/**
 * @brief Input handler for fringe (contour) editing
 * 
 * Responsibilities:
 * - Translate raw input events to fringe editing operations
 * - Manage modal editing states (Navigate, Draw, DotEdit)
 * - Delegate to InputHandler for domain logic
 * - Request view invalidation via callback
 * 
 * Constraints:
 * - MUST NOT call view methods directly
 * - MUST use InputHandler for all domain operations
 * - MUST NOT handle pan/zoom (navigation fallback handles that)
 * 
 * Input Consumption:
 * - Consumes events in Draw mode (extend segments, add dots)
 * - Consumes events in DotEdit mode (drag dots, move edges)
 * - Consumes events in Navigate mode (box selection)
 * - Returns false for pan/zoom gestures (allows navigation fallback)
 */
class FringeInputHandler : public IInputHandler {
public:
    /**
     * @brief Construct fringe input handler
     * @param pView View for invalidation callback (not owned)
     */
    explicit FringeInputHandler(CWnd* pView);
    ~FringeInputHandler() override;
    
    // ========================================================================
    // Initialization (called by CImageView during setup)
    // ========================================================================
    
    /**
     * @brief Initialize with dependencies
     * @param pDigit Digit information (not owned)
     * @param pTransform View transform (not owned)
     * @param pDispatcher Command dispatcher (not owned)
     */
    void Initialize(
        CDigitInfo* pDigit,
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
     * Automatically finalizes pending operations when switching modes.
     */
    void SetMode(EditMode mode);
    
    /**
     * @brief Get current edit mode
     */
    EditMode GetMode() const;
    
    /**
     * @brief Get underlying InputHandler for legacy integration
     * 
     * Used by view for:
     * - Rendering active segment preview
     * - Querying active segment state
     */
    InputHandler& GetInputHandler() { return m_inputHandler; }
    const InputHandler& GetInputHandler() const { return m_inputHandler; }
    
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
    CDigitInfo* m_pDigit;             ///< Digit information
    ViewTransform* m_pTransform;      ///< View transform
    CommandDispatcher* m_pDispatcher; ///< Command dispatcher
    
    // ========================================================================
    // Domain Logic
    // ========================================================================
    
    InputHandler m_inputHandler;      ///< Underlying domain handler
    
    // ========================================================================
    // Helpers
    // ========================================================================
    
    /**
     * @brief Request view invalidation
     */
    void Invalidate();
    
    /**
     * @brief Check if Space key is pressed (pan gesture)
     */
    bool IsSpacePressed() const;
    
    /**
     * @brief Check if Ctrl key is pressed (zoom gesture)
     */
    bool IsCtrlPressed() const;
};

} // namespace DigitMode

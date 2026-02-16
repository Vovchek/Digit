/**
 * @file NavigationInputHandler.h
 * @brief Navigation handler for pan, zoom, and cancel operations
 * 
 * ARCHITECTURAL ROLE:
 * NavigationInputHandler is the FALLBACK handler in InputRouter.
 * It provides universal navigation that works with ANY tool.
 * 
 * DESIGN PRINCIPLE:
 * - No domain knowledge (bounds, fringes, shapes)
 * - No document mutation
 * - No commands
 * - Only viewport transformation (ViewTransform)
 * 
 * GESTURE BINDINGS:
 * - Pan:    Space + Left Mouse Drag
 * - Zoom:   Ctrl + Mouse Wheel
 * - Cancel: Escape (broadcasts to all handlers)
 */
#pragma once

#include "IInputHandler.h"
#include "ImageTempl/ViewTransform.h"  // For ViewTransform and CPoint2d
#include <afxwin.h>

// Forward declarations
class ViewTransform;

namespace DigitMode {

/**
 * @brief Navigation handler for viewport operations
 * 
 * Responsibilities:
 * - Pan viewport (Space + Drag)
 * - Zoom in/out (Ctrl + Wheel)
 * - Cancel operations (Escape)
 * 
 * Constraints:
 * - MUST NOT reference document data
 * - MUST NOT dispatch commands
 * - MUST ONLY modify ViewTransform
 * - MUST request invalidation via callback
 * 
 * State:
 * - Tracks pan drag state (start point, current offset)
 * - NO persistent state between gestures
 */
class NavigationInputHandler : public IInputHandler {
public:
    /**
     * @brief Construct navigation handler
     * @param pTransform ViewTransform to modify (not owned)
     */
    explicit NavigationInputHandler(ViewTransform* pTransform);
    ~NavigationInputHandler() override;
    
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
    
    ViewTransform* m_pTransform;  ///< Transform to modify
    
    // ========================================================================
    // Pan State
    // ========================================================================
    
    bool m_isPanning;             ///< True during pan drag
    CPoint m_panStart;            ///< Screen point where pan started
    CPoint2d m_panStartOffset;    ///< Transform offset at pan start
    
    // ========================================================================
    // Helpers
    // ========================================================================
    
    /**
     * @brief Check if Space key is pressed
     */
    bool IsSpacePressed() const;
    
    /**
     * @brief Check if Ctrl key is pressed
     */
    bool IsCtrlPressed() const;
};

} // namespace DigitMode

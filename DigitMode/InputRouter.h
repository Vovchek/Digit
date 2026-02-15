/**
 * @file InputRouter.h
 * @brief Central event routing for CAD-grade interaction model
 * 
 * ARCHITECTURAL ROLE:
 * InputRouter is the SINGLE point of delegation between views and handlers.
 * It implements the canonical routing rule:
 * 
 *   1. Try active tool
 *   2. If not consumed → try navigation handler
 * 
 * OWNERSHIP:
 * - Owned by CBaseImageView
 * - Configured by CImageView (tool switching)
 * 
 * DESIGN PRINCIPLE:
 * - No logic, only delegation
 * - Deterministic routing (tool → navigation)
 * - Tools can coexist with navigation (fallback model)
 */
#pragma once

#include "IInputHandler.h"

namespace DigitMode {

/**
 * @brief Central router for input events
 * 
 * Responsibilities:
 * - Route events to active tool or navigation handler
 * - Enforce canonical routing order (tool first, then navigation)
 * - Provide tool activation interface
 * 
 * NON-Responsibilities:
 * - Does NOT interpret events (handlers do that)
 * - Does NOT manage view state (views do that)
 * - Does NOT dispatch commands (handlers do that)
 * 
 * Thread Safety: Not thread-safe (single-threaded MFC GUI)
 */
class InputRouter {
public:
    InputRouter();
    ~InputRouter();
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    /**
     * @brief Set active tool handler
     * @param tool Pointer to tool handler (can be nullptr to disable tools)
     * 
     * The active tool receives events BEFORE navigation.
     * Call this when user switches tools (toolbar, menu, hotkey).
     * 
     * Passing nullptr disables tool handling (navigation-only mode).
     */
    void SetActiveTool(IInputHandler* tool);
    
    /**
     * @brief Set navigation handler
     * @param nav Pointer to navigation handler (can be nullptr)
     * 
     * The navigation handler is the FALLBACK for unconsumed events.
     * Typically set once during initialization and never changed.
     * 
     * Passing nullptr disables navigation fallback.
     */
    void SetNavigationHandler(IInputHandler* nav);
    
    /**
     * @brief Get current active tool
     * @return Pointer to active tool, or nullptr if none
     */
    IInputHandler* GetActiveTool() const { return m_activeTool; }
    
    /**
     * @brief Get navigation handler
     * @return Pointer to navigation handler, or nullptr if none
     */
    IInputHandler* GetNavigationHandler() const { return m_navigation; }
    
    // ========================================================================
    // Event Routing (canonical delegation)
    // ========================================================================
    
    /**
     * @brief Route mouse button down event
     * @param flags Mouse flags
     * @param pt Point in screen coordinates
     * @return true if consumed by any handler
     * 
     * Routing order:
     * 1. Active tool (if set)
     * 2. Navigation handler (if tool didn't consume)
     */
    bool OnMouseDown(UINT flags, CPoint pt);
    
    /**
     * @brief Route mouse move event
     * @param flags Mouse flags
     * @param pt Point in screen coordinates
     * @return true if consumed by any handler
     */
    bool OnMouseMove(UINT flags, CPoint pt);
    
    /**
     * @brief Route mouse button up event
     * @param flags Mouse flags
     * @param pt Point in screen coordinates
     * @return true if consumed by any handler
     */
    bool OnMouseUp(UINT flags, CPoint pt);
    
    /**
     * @brief Route mouse wheel event
     * @param flags Mouse flags
     * @param delta Wheel rotation delta
     * @param pt Point in screen coordinates
     * @return true if consumed by any handler
     */
    bool OnMouseWheel(UINT flags, short delta, CPoint pt);
    
    /**
     * @brief Route key down event
     * @param nChar Virtual key code
     * @return true if consumed by any handler
     */
    bool OnKeyDown(UINT nChar);
    
    /**
     * @brief Route key up event
     * @param nChar Virtual key code
     * @return true if consumed by any handler
     */
    bool OnKeyUp(UINT nChar);
    
    // ========================================================================
    // Lifecycle
    // ========================================================================
    
    /**
     * @brief Cancel current operation in all handlers
     * 
     * Called when:
     * - View loses focus
     * - Tool is switched
     * - User presses Escape (routed here explicitly)
     * 
     * Broadcasts Cancel() to:
     * 1. Active tool (if set)
     * 2. Navigation handler (if set)
     */
    void Cancel();

private:
    IInputHandler* m_activeTool;    ///< Current tool (can be nullptr)
    IInputHandler* m_navigation;    ///< Navigation handler (can be nullptr)
};

} // namespace DigitMode

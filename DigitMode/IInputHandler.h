/**
 * @file IInputHandler.h
 * @brief Common interface for all input handlers (tools and navigation)
 * 
 * ARCHITECTURAL PRINCIPLE:
 * All user input flows through handlers implementing this interface.
 * This creates a uniform, deterministic routing system where:
 * - Views are passive (forward events only)
 * - Tools express intent (interpret gestures)
 * - Routing is explicit (InputRouter delegates)
 * 
 * CAD-GRADE INTERACTION MODEL:
 * Views → InputRouter → [ActiveTool OR NavigationHandler]
 * 
 * RETURN VALUE CONTRACT:
 * - true  = Event consumed (stop propagation)
 * - false = Event not handled (allow fallback)
 */
#pragma once

#include <afxwin.h>  // For CPoint, UINT

namespace DigitMode {

/**
 * @brief Common interface for all input handlers
 * 
 * Implemented by:
 * - Tool handlers (BoundsInputHandler, FringeInputHandler)
 * - Navigation handler (NavigationInputHandler)
 * 
 * RESPONSIBILITIES:
 * - Interpret raw input events as domain-specific gestures
 * - Maintain internal state (drag, selection, drawing)
 * - Dispatch commands for document mutation
 * - Return true/false for event consumption
 * 
 * CONSTRAINTS:
 * - MUST NOT call Invalidate() (view responsibility)
 * - MUST NOT reference view classes directly
 * - MUST use Commands for all mutations
 * - MUST be stateless between Cancel() calls
 */
class IInputHandler {
public:
    /**
     * @brief Handle mouse button down event
     * @param flags Mouse flags (MK_LBUTTON, MK_CONTROL, etc.)
     * @param pt Point in screen coordinates
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - Begin drag operation
     * - Select object
     * - Start drawing
     * - Initiate pan (Space + LButton)
     */
    virtual bool OnMouseDown(UINT flags, CPoint pt) = 0;
    
    /**
     * @brief Handle mouse move event
     * @param flags Mouse flags
     * @param pt Point in screen coordinates
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - Update drag preview
     * - Hover feedback (cursor change)
     * - Pan viewport
     * - Draw rubber-band
     */
    virtual bool OnMouseMove(UINT flags, CPoint pt) = 0;
    
    /**
     * @brief Handle mouse button up event
     * @param flags Mouse flags
     * @param pt Point in screen coordinates
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - Commit drag operation (dispatch command)
     * - Finalize drawing
     * - End pan
     * - Complete selection
     */
    virtual bool OnMouseUp(UINT flags, CPoint pt) = 0;
    
    /**
     * @brief Handle mouse wheel event
     * @param flags Mouse flags
     * @param delta Wheel rotation delta (positive = forward, negative = backward)
     * @param pt Point in screen coordinates
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - Zoom in/out (Ctrl + Wheel)
     * - Scroll canvas
     * - Adjust tool parameters
     */
    virtual bool OnMouseWheel(UINT flags, short delta, CPoint pt) = 0;
    
    /**
     * @brief Handle key down event
     * @param nChar Virtual key code (VK_ESCAPE, VK_RETURN, etc.)
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - Escape to cancel operation
     * - Enter to commit
     * - Hotkeys for mode switching
     * - Navigation shortcuts
     */
    virtual bool OnKeyDown(UINT nChar) = 0;
    
    /**
     * @brief Handle key up event
     * @param nChar Virtual key code
     * @return true if event consumed, false to allow fallback
     * 
     * Typical uses:
     * - End modifier-based mode (release Space = end pan)
     * - Reset transient state
     */
    virtual bool OnKeyUp(UINT nChar) = 0;
    
    /**
     * @brief Cancel current operation
     * 
     * Called when:
     * - User presses Escape
     * - Tool is deactivated
     * - View loses focus
     * - Application needs to reset state
     * 
     * MUST:
     * - Discard all transient state (previews, selections)
     * - Release mouse capture (if applicable)
     * - Reset to idle state
     * 
     * MUST NOT:
     * - Dispatch commands
     * - Mutate document
     */
    virtual void Cancel() = 0;
    
    virtual ~IInputHandler() = default;
};

} // namespace DigitMode

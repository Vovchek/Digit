/**
 * @file ToolContext.h
 * @brief Event context passed to tools
 * 
 * Provides tools with all information they need without requiring them
 * to query global state or access view/document directly.
 */

#pragma once

#include <afxwin.h>
#include <limits>

namespace DigitMode {

// Forward declaration
class IInteractionTool;

/**
 * @brief Result of hit-testing a tool
 * 
 * Unified result structure used by manager to arbitrate between tools.
 */
struct HitResult {
    bool hit = false;
    
    /// Which tool reported this hit (set by manager, not tool)
    IInteractionTool* tool = nullptr;
    
    /// Tool-specific context (what was hit: shape index, handle, etc.)
    void* toolContext = nullptr;
    
    /// Distance from query point (for tie-breaking)
    double distance = 1000000.0;  // Use large value for "infinite" distance
    
    /// Priority for arbitration (from tool capabilities)
    int priority = 0;
    
    /// Comparison for sorting (higher priority first, closer distance second)
    bool operator<(const HitResult& other) const {
        if (priority != other.priority) return priority > other.priority;
        if (distance != other.distance) return distance < other.distance;
        return false;
    }
};

/**
 * @brief Event context passed to interaction tools
 * 
 * Contains all state relevant to handling an input event:
 * - Mouse position and buttons
 * - Keyboard modifiers
 * - What was hit (from manager's arbitration)
 * - Previous state (for computing deltas)
 * 
 * Tools receive this instead of raw MFC events, making them
 * decoupled from Windows message handling.
 */
struct ToolContext {
    // ========================================================================
    // Mouse State
    // ========================================================================
    
    /// Mouse button flags (MK_LBUTTON, MK_RBUTTON, etc.)
    UINT mouseFlags = 0;
    
    /// Current mouse position in screen coordinates
    CPoint screenPoint = {0, 0};
    
    // ========================================================================
    // Keyboard State (captured at event time)
    // ========================================================================
    
    bool shiftKey = false;
    bool altKey = false;
    bool ctrlKey = false;
    
    // ========================================================================
    // Hit Result (from manager's arbitration)
    // ========================================================================
    
    /// What was hit at screenPoint
    /// Filled by InteractionManager before calling tool
    /// Tools use this to determine what user is interacting with
    HitResult hit;
    
    // ========================================================================
    // Previous State (for computing deltas)
    // ========================================================================
    
    struct Previous {
        CPoint screenPoint = {0, 0};
        bool valid = false;
    };
    
    Previous previous;
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    /// Compute delta from previous position to current
    /// Only valid if previous.valid == true
    CSize GetDelta() const {
        if (!previous.valid) {
            return {0, 0};
        }
        return {screenPoint.x - previous.screenPoint.x, 
                screenPoint.y - previous.screenPoint.y};
    }
    
    /// Check if left button is pressed
    bool IsLeftButtonDown() const {
        return (mouseFlags & MK_LBUTTON) != 0;
    }
    
    /// Check if right button is pressed
    bool IsRightButtonDown() const {
        return (mouseFlags & MK_RBUTTON) != 0;
    }
    
    /// Check if middle button is pressed
    bool IsMiddleButtonDown() const {
        return (mouseFlags & MK_MBUTTON) != 0;
    }
    
    /// Check if any modifier is pressed
    bool HasModifier() const {
        return shiftKey || altKey || ctrlKey;
    }
};

}  // namespace DigitMode

#pragma once

#include <string>

// Forward declarations (global namespace)
class CDigitInfo;

namespace DigitMode {

/**
 * @brief Represents the current state of modifier keys
 * 
 * Design Principle: Modifiers have consistent global meaning:
 * - Ctrl  = Add / Extend / Connect
 * - Shift = Range / Constrain / Promote
 * - Alt   = Alternate / Destructive / Structural
 */
struct ModifierState {
    bool ctrl = false;
    bool shift = false;
    bool alt = false;

    /**
     * @brief Check if no modifiers are pressed
     */
    bool None() const { return !ctrl && !shift && !alt; }

    /**
     * @brief Get current modifier state from keyboard
     * @return ModifierState with current key states
     */
    static ModifierState FromKeyboard() {
        return {
            (GetKeyState(VK_CONTROL) & 0x8000) != 0,
            (GetKeyState(VK_SHIFT) & 0x8000) != 0,
            (GetKeyState(VK_MENU) & 0x8000) != 0  // Alt key
        };
    }

    /**
     * @brief Get debug string representation
     * @return String like "Ctrl+Shift+" or "None"
     */
    std::string Debug() const {
        std::string s;
        if (ctrl) s += "Ctrl+";
        if (shift) s += "Shift+";
        if (alt) s += "Alt+";
        return s.empty() ? "None" : s;
    }
};

/**
 * @brief Editing modes for the fringe editor
 * 
 * Modes change fundamental mouse behavior:
 * - Navigate: Selection, movement, multi-object operations
 * - Draw: Create and extend segments
 * - DotEdit: Fine geometry editing (move/insert/delete dots)
 */
enum class EditMode {
    Navigate,  ///< Default mode: selection and navigation
    Draw,      ///< Drawing mode: create/continue/connect segments
    DotEdit    ///< Dot edit mode: geometry-only editing
};

/**
 * @brief Manages mode switching and drawing state
 * 
 * Responsibilities:
 * - Track current editing mode
 * - Maintain active segment during drawing
 * - Finalize pending operations on mode switch
 * 
 * @note Selection is locked during Draw mode (handled by caller)
 */
class InputHandler {
private:
    EditMode currentMode = EditMode::Navigate;  ///< Initialize currentMode to Navigate

    // Draw mode state
    int iActiveSegment = -1;  ///< Index of segment being drawn (-1 = none)

public:
    /**
     * @brief Set the current editing mode
     * @param newMode Mode to switch to
     * 
     * Side effects:
     * - Finalizes active segment if leaving Draw mode
     * - Updates cursor (via caller)
     */
    void SetMode(EditMode newMode);

    /**
     * @brief Get the current editing mode
     */
    EditMode GetMode() const { return currentMode; }

    /**
     * @brief Check if currently in Draw mode
     */
    bool IsInDrawMode() const { return currentMode == EditMode::Draw; }

    // ===== Draw Mode Operations =====

    /**
     * @brief Start drawing a new segment at point P
     * @param P Starting point in image coordinates
     * @param pDigit Pointer to DigitInfo (for accessing Fringes)
     * 
     * Creates a new segment with incremented Number value
     */
    void StartNewSegment(CPoint P, ::CDigitInfo* pDigit);

    /**
     * @brief Continue drawing from an existing segment end
     * @param iSegment Index of segment to continue
     * @param iDot Index of end dot (must be 0 or last)
     * @param pDigit Pointer to DigitInfo
     */
    void ContinueSegment(int iSegment, int iDot, ::CDigitInfo* pDigit);

    /**
     * @brief Connect current segment to another segment's end
     * @param iSegment Index of segment to connect to
     * @param iDot Index of connection dot (end of segment)
     * @param pDigit Pointer to DigitInfo
     * 
     * After connection, the free end of the target segment becomes active
     */
    void ConnectSegments(int iSegment, int iDot, ::CDigitInfo* pDigit);

    /**
     * @brief End the current segment (finish drawing)
     * 
     * Called on right-click or when leaving Draw mode
     */
    void EndCurrentSegment();

    /**
     * @brief Get the index of the currently active segment
     * @return Segment index, or -1 if no active segment
     */
    int GetActiveSegment() const { return iActiveSegment; }

    /**
     * @brief Handle mouse drag for box selection in Navigate mode
     * @param start Start point of drag (screen coordinates)
     * @param end End point of drag (screen coordinates)
     * @param pDigit Pointer to DigitInfo (for accessing segments)
     */
    void HandleBoxSelection(CPoint start, CPoint end, class CDigitInfo* pDigit);

    /**
     * @brief Handle mouse drag events
     * @param start Starting point of the drag
     * @param end Ending point of the drag
     * @param pDigit Pointer to the digit information
     */
    void OnMouseDrag(CPoint start, CPoint end, CDigitInfo* pDigit);
};

} // namespace DigitMode

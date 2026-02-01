#pragma once

#include <string>

#include "SelectionManager.h"
// CDPoint definition
#include "MGTools/Include/Utils/BaseDataType.h"

// Forward declarations (global namespace)
class CDigitInfo;
class ViewTransform;

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
* @brief Active end of a segment during drawing
* 
* State used to track which end is being manipulated
**/
enum class ActiveEnd {
	None,   ///< Default state: no active end
	Head,  ///< Drawing extends/inserts at start of segment
	Tail     ///< Drawing extends/inserts at end of segment
};

/**
 * @brief Manages mode switching and drawing state
 * 
 * Responsibilities:
 * - Track current editing mode
 * - Maintain active segment and end dot during drawing
 * - Finalize pending operations on mode switch
 * 
 * @note Selection is locked during Draw mode (handled by caller)
 */
class InputHandler {
private:
    EditMode currentMode = EditMode::Navigate;  ///< Initialize currentMode to Navigate

    // Internal helpers for drag lifecycle
    void BeginDotDrag(int segIdx, int dotIdx, CPoint start, ::CDigitInfo* pDigit);
    void BeginEdgeDrag(int segIdx, int edgeStartIdx, CPoint start, ::CDigitInfo* pDigit);
    void UpdateDragPreview(CPoint pt, ::CDigitInfo* pDigit);
    void CommitActiveDrag(class CommandDispatcher* pCmdDisp, ::CDigitInfo* pDigit);

    // Draw mode state
    int iActiveSegment = -1;  ///< Index of segment being drawn (-1 = none)
	ActiveEnd activeEnd = ActiveEnd::None; ///< Currently active end during drawing
    // Transient interaction state
    CPoint m_cursorPos = CPoint(-1, -1);
	bool m_rubberBand = false;

    struct DragState {
        bool active = false;
        enum class Type { None, BoxSelect, MoveDot, MoveEdge, RubberBand } type = Type::None;
        CPoint start = CPoint(-1, -1);
        CPoint current = CPoint(-1, -1);
        int segmentIndex = -1;
        int dotIndex = -1;
        // For edge drags store original end-point positions
        CDPoint edgeOldA = CDPoint(0,0);
        CDPoint edgeOldB = CDPoint(0,0);
        // For dot drags store original position
        CDPoint dotOldPos = CDPoint(0,0);
    } m_drag;

    // Hover (simple representation)
    SelectionLevel m_hoverLevel = SelectionLevel::None;
    int m_hoverSeg = -1;
    int m_hoverDot = -1;

public:
    // Pan/zoom short-circuit state
    bool m_isPanning = false;
    CPoint m_lastPanPoint = CPoint(-1, -1);
    void OnMouseWheel(const CPoint& pt, short zDelta, ViewTransform* view);
    void BeginPan(const CPoint& pt);
    void ContinuePan(const CPoint& pt, ViewTransform* view);
    void EndPan();
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

    // Default constructor (keeps existing member-field usage in ImageView)
    InputHandler() = default;

    /**
     * @brief Check if currently in Draw mode
     */
    bool IsInDrawMode() const { return currentMode == EditMode::Draw; }

    // ===== Draw Mode Operations =====

    /**
     * @brief Start drawing a new segment at point P
     * @param P Starting point in image coordinates
     * @param pDigit Pointer to DigitInfo (for accessing Fringes)
     * @param pCmdDisp Pointer to CommandDispatcher (to create/execute Commands directly)
     *
     * Creates a new segment with incremented Number value
     */
    void StartNewSegment(CPoint P, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);

    /**
     * @brief Continue drawing from an existing segment end
     * @param iSegment Index of segment to continue
     * @param iDot Index of end dot (must be 0 or last)
     * @param pDigit Pointer to DigitInfo
     */
    void ContinueSegment(int iSegment, int iDot, ::CDigitInfo* pDigit);
    // Thin forwarding that accepts CommandDispatcher when continuation should emit commands
    void ContinueSegment(int iSegment, int iDot, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);

    /**
     * @brief Connect current segment to another segment's end
     * @param iSegment Index of segment to connect to
     * @param iDot Index of connection dot (end of segment)
     * @param pDigit Pointer to DigitInfo
     * 
     * After connection, the free end of the target segment becomes active
     */
    void ConnectSegments(int iSegment, int iDot, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);

    /**
     * @brief End the current segment (finish drawing)
     * 
     * Called on right-click or when leaving Draw mode
     */
    void EndCurrentSegment();

    // Preview & commit hooks used by ImageView adapter
    void OnMouseMove(CPoint pt, const ModifierState& mods, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);
    void OnLButtonUp(CPoint pt, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);

    // Full event handlers (higher-level adapter may call these)
    void OnLButtonDown(UINT flags, CPoint pt, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);
    void OnRButtonDown(UINT flags, CPoint pt, ::CDigitInfo* pDigit, class CommandDispatcher* pCmdDisp);

    void OnKeyUp(UINT nChar, ::CDigitInfo* pDigit, CommandDispatcher* pCmdDisp);

    // Cancel active draw without committing
    void CancelDraw(::CDigitInfo* pDigit);

    // Keyboard handling while in draw mode (arrows, backspace, escape)
    void OnKeyDown(UINT nChar, ::CDigitInfo* pDigit, CommandDispatcher* pCmdDisp);

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

    // Query helpers
    bool HasActiveSegment() const { return iActiveSegment >= 0; }
    bool IsActiveSegmentValid(const ::CDigitInfo* doc) const;
    bool GetRubberBand(const ::CDigitInfo* doc) const;
    ActiveEnd GetActiveEnd() const { return activeEnd; }
    CPoint GetCurrentCursorPos() const { return m_cursorPos; }
	CPoint GetActiveDot(const ::CDigitInfo* doc) const;
    
    /**
     * @brief Draw selection box (rubber band) during box select drag
     * @param pDC Device context to draw on
     */
    void DrawSelectionBox(CDC* pDC) const;
    
    /**
     * @brief Check if currently dragging a selection box
     */
    bool IsDraggingSelectionBox() const {
        return m_drag.active && m_drag.type == DragState::Type::BoxSelect;
    }
};

} // namespace DigitMode

/**
 * @file EditMode.h
 * @brief Edit mode enumeration for bounds editing
 * 
 * ## Overview
 * 
 * Defines mutually exclusive editing modes for aperture/bounds editing.
 * Each mode determines how mouse input is interpreted.
 * 
 * ## Modal Editing (UX Spec §1)
 * 
 * Bounds editing is **strictly modal**:
 * - Only one mode active at a time
 * - Mode persists until explicitly changed
 * - Switching modes cancels any in-progress operation
 * 
 * ## Mode Responsibilities
 * 
 * - **Select**: Select, move, resize, rotate existing shapes
 * - **Add Modes**: Create new shapes via point sequence
 * - **Delete**: Remove shape on click
 * 
 * ## Usage
 * 
 * ```cpp
 * // Set mode in UI toolbar/menu
 * boundsHandler.SetEditMode(EditMode::AddRectangle);
 * 
 * // Update status bar
 * const char* modeName = GetEditModeName(boundsHandler.GetEditMode());
 * statusBar.SetText(modeName);
 * 
 * // Update cursor
 * int cursorID = GetEditModeCursor(boundsHandler.GetEditMode());
 * SetCursor(LoadCursor(nullptr, MAKEINTRESOURCE(cursorID)));
 * ```
 * 
 * @see BoundsHandler::SetEditMode - Mode switching logic
 * @see UX Specification §1.1 - Modal editing rules
 */
#pragma once

namespace DigitMode {

/**
 * @brief Bounds editing modes (mutually exclusive)
 * 
 * Each mode defines how mouse clicks and drags are interpreted.
 * 
 * ## Mode Behavior Summary
 * 
 * | Mode          | LButton Down           | LButton Drag           | LButton Up        |
 * |---------------|------------------------|------------------------|-------------------|
 * | Select        | Hit-test & select      | Move/resize/rotate     | Commit change     |
 * | AddRectangle  | Add point 1            | (no drag)              | Add point 2/3     |
 * | AddEllipse    | Add perimeter point    | (no drag)              | Add point         |
 * | AddCircle     | Add perimeter point    | (no drag)              | Add point         |
 * | AddPolygon    | Add vertex             | (no drag)              | Add vertex        |
 * | Delete        | Hit-test               | (no drag)              | Delete shape      |
 * 
 * @note In Add modes, mouse drag is ignored (point-sequence only)
 * @note In Select mode, drag modifies preview shape only (Command pattern)
 */
enum class EditMode {
    Select,       ///< Select and edit existing shapes (default)
    AddRectangle, ///< Create rectangle via 3-point sequence (UX Spec §2.1)
    AddEllipse,   ///< Create ellipse via perimeter fitting (UX Spec §2.2)
    AddCircle,    ///< Create circle (equal-radii constraint) (UX Spec §2.3)
    AddPolygon,   ///< Create polygon via vertex clicks (UX Spec §2.4)
    Delete        ///< Remove shape on click
};

/**
 * @brief Get display name for edit mode
 * @param mode Edit mode
 * @return Human-readable mode name (for status bar)
 * 
 * ## Example Output
 * 
 * - Select → "Select/Edit"
 * - AddRectangle → "Add Rectangle"
 * - AddCircle → "Add Circle"
 * - etc.
 */
const char* GetEditModeName(EditMode mode);

/**
 * @brief Get cursor resource ID for edit mode
 * @param mode Edit mode
 * @return Windows cursor ID (IDC_* constant)
 * 
 * ## Cursor Mapping
 * 
 * - Select → IDC_ARROW (standard pointer)
 * - Add modes → IDC_CROSS (crosshair for precision)
 * - Delete → IDC_NO (slash cursor)
 * 
 * @note Returns standard Windows cursor IDs (can be extended with custom cursors)
 */
LPCTSTR GetEditModeCursor(EditMode mode);

} // namespace DigitMode

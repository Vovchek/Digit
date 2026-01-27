#pragma once

#include "InputHandler.h"
#include "SelectionManager.h"
#include <Windows.h>

namespace DigitMode {

/**
 * @brief Cursor types for different modes
 */
enum class CursorType {
    Arrow,      ///< Navigate mode
    Crosshair,  ///< Draw mode
    Vertex,     ///< Dot Edit mode
    Wait        ///< Processing
};

/**
 * @brief Cursor overlays for modifier feedback
 */
enum class CursorOverlay {
    None,   ///< No modifier
    Plus,   ///< Ctrl (add/connect)
    Range,  ///< Shift (range/constrain)
    Bang    ///< Alt (destructive/alternate)
};

/**
 * @brief Manages cursor appearance based on mode and modifiers
 * 
 * Design:
 * - Base cursor = mode icon
 * - Overlay = modifier feedback
 * 
 * @note Phase 1: Stub implementation (basic cursors only)
 * @todo Phase 6: Implement composite cursors with overlays
 */
class CursorManager {
private:
    HCURSOR hCursorArrow;
    HCURSOR hCursorCrosshair;
    HCURSOR hCursorVertex;
    HCURSOR hCursorWait;

public:
    /**
     * @brief Constructor - loads cursor resources
     */
    CursorManager();

    /**
     * @brief Update cursor based on current state
     * @param mode Current editing mode
     * @param mods Current modifier state
     * @param under Selection level under cursor (for context)
     * 
     * @note Phase 1: Overlays not yet implemented
     */
    void UpdateCursor(EditMode mode, ModifierState mods, SelectionLevel under);

private:
    /**
     * @brief Set cursor image (internal helper)
     */
    void SetCursorImage(CursorType base, CursorOverlay overlay);
};

} // namespace DigitMode

#include "stdafx.h"
#include "CursorManager.h"

namespace DigitMode {

CursorManager::CursorManager() {
    // Load standard Windows cursors
    hCursorArrow = ::LoadCursor(NULL, IDC_ARROW);
    hCursorCrosshair = ::LoadCursor(NULL, IDC_CROSS);
    hCursorWait = ::LoadCursor(NULL, IDC_WAIT);

    // TODO: Load custom vertex cursor in Phase 6
    hCursorVertex = hCursorArrow;  // Placeholder
}

void CursorManager::UpdateCursor(FringeEditMode mode, ModifierState mods, SelectionLevel under) {
    CursorType base;
    CursorOverlay overlay = CursorOverlay::None;

    // 1. Determine base cursor by mode
    switch (mode) {
        case FringeEditMode::Navigate:
            base = CursorType::Arrow;
            break;
        case FringeEditMode::Draw:
            base = CursorType::Crosshair;
            break;
        case FringeEditMode::DotEdit:
            base = CursorType::Vertex;
            break;
        default:
            base = CursorType::Arrow;
            break;
    }

    // 2. Determine overlay by modifiers
    // TODO: Phase 6 - Implement composite cursors
    if (mods.ctrl) {
        overlay = CursorOverlay::Plus;
    } else if (mods.shift) {
        overlay = CursorOverlay::Range;
    } else if (mods.alt) {
        overlay = CursorOverlay::Bang;
    }

    // 3. Set cursor
    SetCursorImage(base, overlay);
}

void CursorManager::SetCursorImage(CursorType base, CursorOverlay overlay) {
    // Phase 1: Simple cursor switch (no overlays yet)
    HCURSOR hCursor;

    switch (base) {
        case CursorType::Arrow:
            hCursor = hCursorArrow;
            break;
        case CursorType::Crosshair:
            hCursor = hCursorCrosshair;
            break;
        case CursorType::Vertex:
            hCursor = hCursorVertex;
            break;
        case CursorType::Wait:
            hCursor = hCursorWait;
            break;
        default:
            hCursor = hCursorArrow;
            break;
    }

    ::SetCursor(hCursor);
}

} // namespace DigitMode

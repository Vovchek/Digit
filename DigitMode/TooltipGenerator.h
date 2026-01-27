#pragma once

#include "SelectionManager.h"
#include <string>

// Forward declaration
class CDigitInfo;

namespace DigitMode {

/**
 * @brief Generates tooltips for selected objects
 * 
 * Tooltip Format (from UX spec):
 * - Dot: "#2.5 / 1(3) / 12"
 *   - Number = 2.5
 *   - Segment 1 of 3 segments in fringe
 *   - Dot 12 within segment
 * 
 * - Edge: "#2.5 Edge (5–6 of 15)"
 * - Segment: "Fringe #2.5 Segment — 34 dots"
 * - Fringe: "Fringe #2.5 (3 segments)"
 * 
 * @note Phase 1: Stub implementation (basic tooltips)
 * @todo Phase 6: Implement full query logic for segment counts
 */
class TooltipGenerator {
public:
    /**
     * @brief Generate tooltip for selected object
     * @param obj Selected object
     * @param digit Reference to DigitInfo for querying
     * @return Tooltip string
     * 
     * @note Phase 1: Returns placeholder strings
     */
    std::string GetTooltip(
        const SelectionManager::SelectedObject& obj,
        const CDigitInfo& digit
    ) const;

private:
    static constexpr int MAX_TOOLTIP_LEN = 256;
};

} // namespace DigitMode

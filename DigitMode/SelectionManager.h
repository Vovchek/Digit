#pragma once

#include <vector>
#include <set>

// Forward declaration (global namespace)
class CFringeSegment;
// Forward declare CDigitInfo to avoid header dependency
class CDigitInfo;
#include "MGTools/Include/Utils/BaseDataType.h" // CDPoint, CDRect

namespace DigitMode {

/**
 * @brief Selection hierarchy levels
 * 
 * Hierarchy: Dot → Edge → Segment → Fringe
 * - Fringe selection = all segments with same Number (logical group)
 */
enum class SelectionLevel {
    None,     ///< Nothing selected
    Dot,      ///< Single control point
    Edge,     ///< Line between two dots
    Segment,  ///< Entire polyline
    Fringe    ///< All segments with same Number
};

/**
 * @brief Box selection mode (affected by modifiers)
 * 
 * According to UX v1.0:
 * - Default: Dot inside, Edge intersects, Segment all edges included
 * - Segment mode (Shift): Segment if any part intersects
 * - Fringe mode (Alt): All segments with same Number if any intersects
 */
enum class BoxSelectionMode {
    Default,    ///< Standard box selection rules
    Segment,    ///< Select entire segment if any part intersects (Shift)
    Fringe,     ///< Select fringe if any segment intersects (Alt)
    AddMode     ///< Add to existing selection (Ctrl)
};

/**
 * @brief Manages persistent, hierarchical selection state
 * 
 * Key Principles:
 * - Selection persists across mode switches
 * - Selection is hierarchical: Dot → Edge → Segment → Fringe
 * - No implicit promotion (all changes are explicit)
 * - Multi-selection allowed only at same level
 * 
 * Segment-Primary Model:
 * - SelectFringe(number) selects ALL segments with that Number
 * - Fringe selection is a query operation, not container access
 */
class SelectionManager {
public:
    /**
     * @brief Represents a selected object in the hierarchy
     */
    struct SelectedObject {
        SelectionLevel level = SelectionLevel::None;
        int iSegment = -1;  ///< Index into segments array
        int iDot = -1;      ///< Dot index within segment
        int iEdge = -1;     ///< Edge index (pair of dots)
        double Number = 0.0; ///< For Fringe-level selection

        /**
         * @brief Check if this selection is valid
         */
        bool IsValid() const { return level != SelectionLevel::None; }

        /**
         * @brief Check if two selections are at the same level
         */
        bool IsSameLevel(const SelectedObject& other) const {
            return level == other.level;
        }
    };

private:
    std::vector<SelectedObject> selection;

public:
    // ===== Selection Primitives =====

    /**
     * @brief Select a single dot (clears existing selection)
     * @param iSegment Segment index
     * @param iDot Dot index within segment
     */
    void SelectDot(int iSegment, int iDot);

    /**
     * @brief Select a single edge (clears existing selection)
     * @param iSegment Segment index
     * @param iEdge Edge index (pair dot[i], dot[i+1])
     */
    void SelectEdge(int iSegment, int iEdge);

    /**
     * @brief Select a single segment (clears existing selection)
     * @param iSegment Segment index
     */
    void SelectSegment(int iSegment);

    /**
     * @brief Select all segments with given Number (clears existing selection)
     * @param number Number value to query
     * @param segments Reference to segment array for querying
     * 
     * This is a QUERY operation: finds all segments where segment.Number == number
     */
    void SelectFringe(double number, const std::vector<::CFringeSegment>& segments);

    /**
     * @brief Select all objects within a rectangular box
     * @param box Selection box in screen coordinates
     * @param segments Reference to segment array for querying
     * @param mode Box selection mode (Default/Segment/Fringe/AddMode)
     * @return Number of objects selected
     * 
     * Rules (UX v1.0):
     * - Default: Dot if inside, Edge if intersects, Segment if all edges included
     * - Segment mode: Segment if ANY part intersects
     * - Fringe mode: All segments with same Number if any segment intersects
     * - AddMode: Add to existing selection instead of replacing
     */
    size_t SelectBox(const CDRect& box, const std::vector<::CFringeSegment>& segments,
                     BoxSelectionMode mode = BoxSelectionMode::Default);

    // ===== Multi-Selection =====

    /**
     * @brief Add object to current selection (Ctrl+Click)
     * @param obj Object to add
     * @return true if added successfully, false if incompatible level
     * 
     * Rules:
     * - Only adds if same level as existing selection
     * - Toggles if object already selected
     */
    bool AddToSelection(const SelectedObject& obj);

    // ===== Promotion =====

    /**
     * @brief Promote selection to Fringe level (Alt+Click)
     * @param segments Reference to segment array for querying
     * 
     * Collects unique Number values from current selection,
     * then selects all segments with those Numbers
     */
    void PromoteToFringe(const std::vector<::CFringeSegment>& segments);

    // ===== Queries =====

    /**
     * @brief Get the current selection level
     * @return Selection level (None if empty)
     */
    SelectionLevel GetLevel() const {
        if (selection.empty()) return SelectionLevel::None;
        return selection[0].level;
    }

    /**
     * @brief Get number of selected objects
     */
    size_t GetCount() const { return selection.size(); }

    /**
     * @brief Get selected object at index
     */
    const SelectedObject& GetAt(size_t i) const { return selection[i]; }

    /**
     * @brief Clear all selection
     */
    void Clear() { selection.clear(); }

    /**
     * @brief Check if selection is empty
     */
    bool IsEmpty() const { return selection.empty(); }

    /**
     * @brief Draw visual feedback for current selection
     * @param pDC Pointer to device context
     * @param segments Reference to segment array for querying
     */
    void DrawSelection(CDC* pDC, const std::vector<::CFringeSegment>& segments);

    // Hover state used by ImageView to draw cursor/preview
    void SetHover(SelectionLevel level, int segId, int dotId);

    // Draw highlights for hover/selected object using DigitInfo draw helpers
    void DrawHighlights(CDC* pDC, const class CDigitInfo* pDigit, int dotSide);

private:
    /**
     * @brief Helper: Check if two selections refer to same object
     */
    bool IsSameObject(const SelectedObject& a, const SelectedObject& b) const;
};

} // namespace DigitMode

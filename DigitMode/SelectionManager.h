#pragma once

#include <vector>
#include <set>

// Forward declaration (global namespace)
class CFringeSegment;

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

private:
    /**
     * @brief Helper: Check if two selections refer to same object
     */
    bool IsSameObject(const SelectedObject& a, const SelectedObject& b) const;
};

} // namespace DigitMode

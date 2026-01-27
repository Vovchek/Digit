#include "stdafx.h"
#include "SelectionManager.h"
#include "CFringeSegment.h"  // Global namespace
#include <algorithm>
#include <cmath>

namespace DigitMode {

// ===== Selection Primitives =====

void SelectionManager::SelectDot(int iSegment, int iDot) {
    selection.clear();
    SelectedObject obj;
    obj.level = SelectionLevel::Dot;
    obj.iSegment = iSegment;
    obj.iDot = iDot;
    selection.push_back(obj);
}

void SelectionManager::SelectEdge(int iSegment, int iEdge) {
    selection.clear();
    SelectedObject obj;
    obj.level = SelectionLevel::Edge;
    obj.iSegment = iSegment;
    obj.iEdge = iEdge;  // Edge between dot[iEdge] and dot[iEdge+1]
    selection.push_back(obj);
}

void SelectionManager::SelectSegment(int iSegment) {
    selection.clear();
    SelectedObject obj;
    obj.level = SelectionLevel::Segment;
    obj.iSegment = iSegment;
    selection.push_back(obj);
}

void SelectionManager::SelectFringe(double number, const std::vector<CFringeSegment>& segments) {
    selection.clear();

    constexpr double EPSILON = 1e-6;

    // Query: Find all segments with this Number
    for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
        if (std::abs(segments[iSeg].GetNumber() - number) < EPSILON) {
            SelectedObject obj;
            obj.level = SelectionLevel::Fringe;
            obj.iSegment = static_cast<int>(iSeg);
            obj.Number = number;
            selection.push_back(obj);
        }
    }
}

// ===== Multi-Selection =====

bool SelectionManager::AddToSelection(const SelectedObject& obj) {
    // Only add if same level as existing selection
    if (!selection.empty()) {
        if (!selection[0].IsSameLevel(obj)) {
            return false;  // Incompatible level
        }

        // Check for duplicates (toggle if already selected)
        auto it = std::find_if(selection.begin(), selection.end(),
            [this, &obj](const SelectedObject& s) {
                return IsSameObject(s, obj);
            });

        if (it != selection.end()) {
            selection.erase(it);  // Toggle off
            return true;
        }
    }

    selection.push_back(obj);
    return true;
}

// ===== Promotion =====

void SelectionManager::PromoteToFringe(const std::vector<CFringeSegment>& segments) {
    // Collect unique Numbers from current selection
    std::set<double> numbers;
    for (const auto& obj : selection) {
        if (obj.iSegment >= 0 && obj.iSegment < static_cast<int>(segments.size())) {
            numbers.insert(segments[obj.iSegment].GetNumber());
        }
    }

    selection.clear();

    // For each unique Number, select all segments with that Number
    constexpr double EPSILON = 1e-6;
    for (double num : numbers) {
        for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
            if (std::abs(segments[iSeg].GetNumber() - num) < EPSILON) {
                SelectedObject obj;
                obj.level = SelectionLevel::Fringe;
                obj.iSegment = static_cast<int>(iSeg);
                obj.Number = num;
                selection.push_back(obj);
            }
        }
    }
}

// ===== Private Helpers =====

bool SelectionManager::IsSameObject(const SelectedObject& a, const SelectedObject& b) const {
    if (a.level != b.level) return false;
    if (a.iSegment != b.iSegment) return false;

    switch (a.level) {
        case SelectionLevel::Dot:
            return a.iDot == b.iDot;
        case SelectionLevel::Edge:
            return a.iEdge == b.iEdge;
        case SelectionLevel::Segment:
        case SelectionLevel::Fringe:
            return true;  // Segment index match is enough
        default:
            return false;
    }
}

} // namespace DigitMode

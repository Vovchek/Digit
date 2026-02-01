#include "stdafx.h"
#include "SelectionManager.h"
#include "CFringeSegment.h"  // Global namespace
#include "DigitMode/DigitInfo.h"
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

void SelectionManager::SetHover(SelectionLevel level, int segId, int dotId) {
    // Simple hover implementation: store as a single temporary selection (not persisted)
    selection.clear();
    if (level == SelectionLevel::None) return;

    SelectedObject obj;
    obj.level = level;
    obj.iSegment = segId;
    obj.iDot = dotId;
    selection.push_back(obj);
}

void SelectionManager::DrawHighlights(CDC* pDC, const CDigitInfo* pDigit, int /*dotSide*/) {
    // Use existing DrawSelection which expects segments; adapt from CDigitInfo
    if (!pDigit) return;
    DrawSelection(pDC, pDigit->Fringes);
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

size_t SelectionManager::SelectBox(const CRect& box, const std::vector<::CFringeSegment>& segments,
                                   BoxSelectionMode mode) {
    std::vector<SelectedObject> newSelection;
    
    // Helper: Check if edge intersects box
    auto EdgeIntersectsBox = [](const CDPoint& p1, const CDPoint& p2, const CRect& rect) -> bool {
        CPoint pt1(static_cast<int>(p1.x), static_cast<int>(p1.y));
        CPoint pt2(static_cast<int>(p2.x), static_cast<int>(p2.y));
        
        // If either point inside, edge intersects
        if (rect.PtInRect(pt1) || rect.PtInRect(pt2)) return true;
        
        // Cohen-Sutherland line-rectangle intersection
        // Simplified: check if line crosses any box edge
        int x1 = pt1.x, y1 = pt1.y, x2 = pt2.x, y2 = pt2.y;
        int xmin = rect.left, xmax = rect.right, ymin = rect.top, ymax = rect.bottom;
        
        // Line bounding box doesn't overlap rect bounding box
        int maxX1X2 = (x1 > x2) ? x1 : x2;
        int minX1X2 = (x1 < x2) ? x1 : x2;
        int maxY1Y2 = (y1 > y2) ? y1 : y2;
        int minY1Y2 = (y1 < y2) ? y1 : y2;
        
        if (maxX1X2 < xmin || minX1X2 > xmax ||
            maxY1Y2 < ymin || minY1Y2 > ymax) {
            return false;
        }
        
        // Detailed intersection check
        // Check if line crosses any of 4 box edges
        auto SegmentIntersect = [](int x1, int y1, int x2, int y2,
                                    int x3, int y3, int x4, int y4) -> bool {
            double d = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
            if (std::abs(d) < 1e-6) return false; // Parallel
            double ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / d;
            double ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / d;
            return (ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0);
        };
        
        // Check 4 box edges
        if (SegmentIntersect(x1, y1, x2, y2, xmin, ymin, xmax, ymin)) return true; // Top
        if (SegmentIntersect(x1, y1, x2, y2, xmax, ymin, xmax, ymax)) return true; // Right
        if (SegmentIntersect(x1, y1, x2, y2, xmin, ymax, xmax, ymax)) return true; // Bottom
        if (SegmentIntersect(x1, y1, x2, y2, xmin, ymin, xmin, ymax)) return true; // Left
        
        return false;
    };
    
    if (mode == BoxSelectionMode::Fringe) {
        // Fringe mode: Select all segments with same Number if any intersects
        std::set<double> numbersToSelect;
        constexpr double EPSILON = 1e-6;
        
        for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
            const auto& segment = segments[iSeg];
            bool segmentIntersects = false;
            
            // Check if any edge intersects
            for (int j = 0; j < segment.GetPointCount() - 1; j++) {
                CDPoint p1 = segment.GetPoint(j);
                CDPoint p2 = segment.GetPoint(j + 1);
                if (EdgeIntersectsBox(p1, p2, box)) {
                    segmentIntersects = true;
                    break;
                }
            }
            
            if (segmentIntersects) {
                numbersToSelect.insert(segment.GetNumber());
            }
        }
        
        // Select all segments with collected Numbers
        for (double num : numbersToSelect) {
            for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
                if (std::abs(segments[iSeg].GetNumber() - num) < EPSILON) {
                    SelectedObject obj;
                    obj.level = SelectionLevel::Fringe;
                    obj.iSegment = static_cast<int>(iSeg);
                    obj.Number = num;
                    newSelection.push_back(obj);
                }
            }
        }
    }
    else if (mode == BoxSelectionMode::Segment) {
        // Segment mode: Select segment if ANY part intersects
        for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
            const auto& segment = segments[iSeg];
            bool intersects = false;
            
            for (int j = 0; j < segment.GetPointCount() - 1; j++) {
                CDPoint p1 = segment.GetPoint(j);
                CDPoint p2 = segment.GetPoint(j + 1);
                if (EdgeIntersectsBox(p1, p2, box)) {
                    intersects = true;
                    break;
                }
            }
            
            if (intersects) {
                SelectedObject obj;
                obj.level = SelectionLevel::Segment;
                obj.iSegment = static_cast<int>(iSeg);
                newSelection.push_back(obj);
            }
        }
    }
    else {
        // Default mode: Dot inside, Edge intersects, Segment if all edges included
        for (size_t iSeg = 0; iSeg < segments.size(); iSeg++) {
            const auto& segment = segments[iSeg];
            int pointCount = segment.GetPointCount();
            
            std::vector<bool> dotsInBox(pointCount, false);
            int edgeCount = (pointCount > 1) ? (pointCount - 1) : 0;
            std::vector<bool> edgesIntersect(edgeCount, false);
            
            // Check dots
            for (int j = 0; j < pointCount; j++) {
                CDPoint point = segment.GetPoint(j);
                CPoint screenPoint(static_cast<int>(point.x), static_cast<int>(point.y));
                if (box.PtInRect(screenPoint)) {
                    dotsInBox[j] = true;
                }
            }
            
            // Check edges - edge is included if:
            // 1. Intersects box boundaries OR
            // 2. Both endpoints inside box (edge is fully inside)
            for (int j = 0; j < pointCount - 1; j++) {
                CDPoint p1 = segment.GetPoint(j);
                CDPoint p2 = segment.GetPoint(j + 1);
                
                bool edgeIntersects = EdgeIntersectsBox(p1, p2, box);
                bool bothDotsInside = dotsInBox[j] && dotsInBox[j + 1];
                
                if (edgeIntersects || bothDotsInside) {
                    edgesIntersect[j] = true;
                }
            }
            
            // Select dots that are inside
            for (int j = 0; j < pointCount; j++) {
                if (dotsInBox[j]) {
                    SelectedObject obj;
                    obj.level = SelectionLevel::Dot;
                    obj.iSegment = static_cast<int>(iSeg);
                    obj.iDot = j;
                    newSelection.push_back(obj);
                }
            }
            
            // Select edges that intersect
            for (int j = 0; j < pointCount - 1; j++) {
                if (edgesIntersect[j] && !dotsInBox[j] && !dotsInBox[j+1]) {
                    // Only add edge if dots not already selected
                    SelectedObject obj;
                    obj.level = SelectionLevel::Edge;
                    obj.iSegment = static_cast<int>(iSeg);
                    obj.iEdge = j;
                    newSelection.push_back(obj);
                }
            }
            
            // Select segment if ALL edges included
            if (pointCount > 1) {
                bool allEdgesIncluded = true;
                for (int j = 0; j < pointCount - 1; j++) {
                    if (!edgesIntersect[j]) {
                        allEdgesIncluded = false;
                        break;
                    }
                }
                
                if (allEdgesIncluded) {
                    // Remove individual dots/edges and add segment
                    newSelection.erase(
                        std::remove_if(newSelection.begin(), newSelection.end(),
                            [iSeg](const SelectedObject& obj) {
                                return obj.iSegment == static_cast<int>(iSeg);
                            }),
                        newSelection.end());
                    
                    SelectedObject obj;
                    obj.level = SelectionLevel::Segment;
                    obj.iSegment = static_cast<int>(iSeg);
                    newSelection.push_back(obj);
                }
            }
        }
    }
    
    if (mode == BoxSelectionMode::AddMode) {
        // Add to existing selection (Ctrl)
        for (auto& obj : newSelection) {
            AddToSelection(obj);
        }
    } else {
        // Replace selection
        selection = newSelection;
    }
    
    return selection.size();
}

// ===== Drawing =====

void SelectionManager::DrawSelection(CDC* pDC, const std::vector<::CFringeSegment>& segments) {
    if (!pDC || selection.empty()) return;
    
    // Save DC state
    int savedDC = pDC->SaveDC();
    
    // Set up glow effect colors
    COLORREF glowColor = RGB(255, 200, 0);  // Golden glow
    COLORREF highlightColor = RGB(255, 255, 0);  // Bright yellow
    
    for (const auto& obj : selection) {
        // Bounds check for segment index
        if (obj.iSegment < 0 || obj.iSegment >= static_cast<int>(segments.size())) {
            TRACE("DrawSelection: Invalid segment index %d (max %d)\n", obj.iSegment, segments.size());
            continue;
        }
        
        const auto& segment = segments[obj.iSegment];
        int pointCount = segment.GetPointCount();
        
        if (obj.level == SelectionLevel::Dot) {
            // Bounds check for dot index
            if (obj.iDot < 0 || obj.iDot >= pointCount) {
                TRACE("DrawSelection: Invalid dot index %d in segment %d (max %d)\n", 
                      obj.iDot, obj.iSegment, pointCount);
                continue;
            }
            
            // Draw dot with glow effect
            CDPoint point = segment.GetPoint(obj.iDot);
            CPoint screenPoint(static_cast<int>(point.x), static_cast<int>(point.y));
            
            // Outer glow (larger, semi-transparent effect)
            CPen glowPen(PS_SOLID, 3, glowColor);
            CPen* oldPen = pDC->SelectObject(&glowPen);
            CBrush glowBrush(glowColor);
            CBrush* oldBrush = pDC->SelectObject(&glowBrush);
            
            pDC->Ellipse(screenPoint.x - 6, screenPoint.y - 6, 
                         screenPoint.x + 6, screenPoint.y + 6);
            
            // Inner highlight (bright center)
            CBrush highlightBrush(highlightColor);
            pDC->SelectObject(&highlightBrush);
            pDC->Ellipse(screenPoint.x - 3, screenPoint.y - 3,
                         screenPoint.x + 3, screenPoint.y + 3);
            
            pDC->SelectObject(oldBrush);
            pDC->SelectObject(oldPen);
        }
        else if (obj.level == SelectionLevel::Edge) {
            // Bounds check for edge index
            if (obj.iEdge < 0 || obj.iEdge >= pointCount - 1) {
                TRACE("DrawSelection: Invalid edge index %d in segment %d (max %d)\n",
                      obj.iEdge, obj.iSegment, pointCount - 1);
                continue;
            }
            
            // Draw edge with glow
            CDPoint p1 = segment.GetPoint(obj.iEdge);
            CDPoint p2 = segment.GetPoint(obj.iEdge + 1);
            CPoint pt1(static_cast<int>(p1.x), static_cast<int>(p1.y));
            CPoint pt2(static_cast<int>(p2.x), static_cast<int>(p2.y));
            
            // Draw thicker glowing line
            CPen glowPen(PS_SOLID, 5, glowColor);
            CPen* oldPen = pDC->SelectObject(&glowPen);
            pDC->MoveTo(pt1);
            pDC->LineTo(pt2);
            
            // Draw bright center line
            CPen highlightPen(PS_SOLID, 2, highlightColor);
            pDC->SelectObject(&highlightPen);
            pDC->MoveTo(pt1);
            pDC->LineTo(pt2);
            
            pDC->SelectObject(oldPen);
        }
        else if (obj.level == SelectionLevel::Segment || obj.level == SelectionLevel::Fringe) {
            // Draw entire segment with glow
            if (pointCount < 2) continue;
            
            // Draw all edges with glow effect
            for (int j = 0; j < pointCount - 1; j++) {
                CDPoint p1 = segment.GetPoint(j);
                CDPoint p2 = segment.GetPoint(j + 1);
                CPoint pt1(static_cast<int>(p1.x), static_cast<int>(p1.y));
                CPoint pt2(static_cast<int>(p2.x), static_cast<int>(p2.y));
                
                // Outer glow
                CPen glowPen(PS_SOLID, 4, glowColor);
                CPen* oldPen = pDC->SelectObject(&glowPen);
                pDC->MoveTo(pt1);
                pDC->LineTo(pt2);
                
                // Inner highlight
                CPen highlightPen(PS_SOLID, 2, highlightColor);
                pDC->SelectObject(&highlightPen);
                pDC->MoveTo(pt1);
                pDC->LineTo(pt2);
                
                pDC->SelectObject(oldPen);
            }
            
            // Draw all dots with small highlights
            for (int j = 0; j < pointCount; j++) {
                CDPoint point = segment.GetPoint(j);
                CPoint screenPoint(static_cast<int>(point.x), static_cast<int>(point.y));
                
                CBrush highlightBrush(highlightColor);
                CBrush* oldBrush = pDC->SelectObject(&highlightBrush);
                CPen highlightPen(PS_SOLID, 1, highlightColor);
                CPen* oldPen = pDC->SelectObject(&highlightPen);
                
                pDC->Ellipse(screenPoint.x - 4, screenPoint.y - 4,
                             screenPoint.x + 4, screenPoint.y + 4);
                
                pDC->SelectObject(oldBrush);
                pDC->SelectObject(oldPen);
            }
        }
    }
    
    // Restore DC
    pDC->RestoreDC(savedDC);
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

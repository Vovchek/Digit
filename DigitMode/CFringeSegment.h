#pragma once
#include "MGTools/Include/Utils/BaseDataType.h"
#include <vector>
#include <afxwin.h>  // Include for MFC types like CDC, CPoint, CRect, BOOL

/// <summary>
/// Represents a single fringe as an ordered polyline of points.
/// A fringe is a continuous curve with a constant fringe number.
/// 
/// ARCHITECTURE NOTES:
/// - Points are stored in ORDERED sequence (spatial coherence preserved)
/// - NO section information stored here (sections are horizontal slices across ALL fringes)
/// - Multiple CFringeSegment objects can share the same Number (discontinuous segments)
/// - Sections can be reconstructed from Y-coordinates of all fringes when needed
/// - Uses std::vector (NOT CArray) for modern C++ compliance
/// </summary>
class CFringeSegment {
private:
    double m_Number;              ///< Fringe number (e.g., 0, 0.5, 1.0, ...)
    int m_Index;                  ///< Segment index for this fringe Number 
                                  ///< (to distinguish discontinuous parts)
    std::vector<CDPoint> m_Points; ///< ORDERED sequence of points forming the polyline
    BOOL m_bClosed;               ///< Is this a closed loop?
    
public:
    // ===== Construction =====
    CFringeSegment(double number = 0.0, int index = -1);
    CFringeSegment(const CFringeSegment& other);
    CFringeSegment& operator=(const CFringeSegment& other);
    ~CFringeSegment();
    
    // ===== Point Management =====
    /// Add point to end of polyline
    /// @return Index of newly added point
    int AddPoint(CDPoint p);
    
    /// Insert point at specific index (shifts subsequent points)
    /// @param idx Position to insert (0-based)
    /// @param p Point to insert
    void InsertPoint(int idx, CDPoint p);
    
    /// Remove point at index
    /// @param idx Index of point to remove (0-based)
    void RemovePoint(int idx);
    
    /// Move existing point to new position
    /// @param idx Index of point to move
    /// @param newP New position
    void MovePoint(int idx, CDPoint newP);
    
    /// Append all points from another fringe
    /// @param other Source fringe to append from
    void AppendPoints(const CFringeSegment& other);
    
    // ===== Queries =====
    /// Get total number of points in fringe
    int GetPointCount() const { return static_cast<int>(m_Points.size()); }
    
    /// Get point at specific index
    /// @param idx Point index (0-based)
    /// @return Point coordinates
    CDPoint GetPoint(int idx) const;
    
    /// Set point at specific index
    /// @param idx Point index (0-based)
    /// @param p New point coordinates
    void SetPoint(int idx, CDPoint p);

    /// Get fringe id
    int GetIndex() const { return m_Index; }

    /// Set fringe number
    /// @param n New fringe number
    void SetIndex(int index) { m_Index = index; }

    /// Get fringe number
    double GetNumber() const { return m_Number; }
    
    /// Set fringe number
    /// @param n New fringe number
    void SetNumber(double n) { m_Number = n; }
    
    /// Check if fringe is closed loop
    BOOL IsClosed() const { return m_bClosed; }
    
    /// Set closed loop state
    void SetClosed(BOOL bClosed) { m_bClosed = bClosed; }
    
    // ===== Hit Testing =====
    /// Find nearest point to screen position within tolerance
    /// @return Index of nearest point, or -1 if none within tolerance
    int FindNearestPoint(CPoint screenP, int tolerance = 5);
    
    /// Check if point is on polyline within tolerance
    BOOL IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx);
    
    /// Get bounding rectangle
    CRect GetBoundingRect() const;
    
    // ===== Drawing (UI dependency - to be removed later) =====
    void DrawDots(CDC* pDC, int dotSize, COLORREF color);
    void DrawPolyline(CDC* pDC, COLORREF color);
    void DrawFull(CDC* pDC, int dotSize, COLORREF lineColor, COLORREF dotColor);
    
    // ===== Advanced Operations (Phase 5+) =====
    /// Split fringe at given point index
    /// @return New fringe containing points from split point to end
    CFringeSegment Split(int atIndex);
    
    /// Get arc length of polyline
    double GetArcLength() const;
    
    /// Subdivide segments longer than maxGap
    void SubdivideSegments(double maxGap);
    
    /// Simplify polyline using Douglas-Peucker algorithm
    /// @param epsilon Maximum distance from original polyline
    void Simplify(double epsilon);
    
private:
    void SimplifyRecursive(int start, int end, double epsilon, std::vector<bool>& keep);
    double PointToLineDistance(CDPoint p, CDPoint lineStart, CDPoint lineEnd);
};

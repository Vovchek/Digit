#pragma once
#include "MGTools/Include/Utils/BaseDataType.h"

/// <summary>
/// Represents a single fringe as an ordered polyline of points.
/// A fringe is a continuous curve with a constant fringe number.
/// 
/// ARCHITECTURE NOTES:
/// - Points are stored in ORDERED sequence (spatial coherence preserved)
/// - NO section information stored here (sections are horizontal slices across ALL fringes)
/// - Multiple CFringeSegment objects can share the same Number (discontinuous segments)
/// - Sections can be reconstructed from Y-coordinates of all fringes when needed
/// </summary>
class CFringeSegment {
private:
    double m_Number;              ///< Fringe number (e.g., 0, 0.5, 1.0, ...)
    int m_Index;                  ///< Segment index for this fringe Number 
                                  ///< (to distinguish discontinuous parts)
    CArray<CDPoint> m_Points;     ///< ORDERED sequence of points forming the polyline
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
    int GetPointCount() const { return m_Points.GetSize(); }
    
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
    
    /// Set closed loop flag
    /// @param closed TRUE if closed loop
    void SetClosed(BOOL closed) { m_bClosed = closed; }
    
    // ===== Hit Testing =====
    /// Find nearest point to screen coordinate (within tolerance)
    /// @param screenP Screen coordinates to test
    /// @param tolerance Maximum distance in pixels
    /// @return Point index, or -1 if none within tolerance
    int FindNearestPoint(CPoint screenP, int tolerance);
    
    /// Check if screen coordinate is on the polyline
    /// @param P Screen coordinates to test
    /// @param tolerance Maximum distance in pixels
    /// @param nearestIdx [out] Index of nearest point (if found)
    /// @return TRUE if point is on polyline within tolerance
    BOOL IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx);
    
    /// Get bounding rectangle
    /// @return Bounding rectangle containing all points
    CRect GetBoundingRect() const;
    
    // ===== Drawing =====
    /// Draw only the dots (markers)
    /// @param pDC Device context
    /// @param dotSize Size of dot markers in pixels
    /// @param color Color for dots
    void DrawDots(CDC* pDC, int dotSize, COLORREF color);
    
    /// Draw only the connecting polyline
    /// @param pDC Device context
    /// @param color Color for line
    void DrawPolyline(CDC* pDC, COLORREF color);
    
    /// Draw both dots and connecting lines
    /// @param pDC Device context
    /// @param dotSize Size of dot markers in pixels
    /// @param lineColor Color for polyline
    /// @param dotColor Color for dots
    void DrawFull(CDC* pDC, int dotSize, COLORREF lineColor, COLORREF dotColor);
    
    // ===== Advanced Operations (Phase 5) =====
    /// Split this fringe at given point index (returns new fringe)
    /// @param atIndex Index where to split (point becomes first of new fringe)
    /// @return New fringe containing points from atIndex onward
    CFringeSegment Split(int atIndex);
    
    /// Compute total arc length
    /// @return Total length of polyline
    double GetArcLength() const;
    
    /// Subdivide long segments to max spacing
    /// @param maxGap Maximum allowed distance between consecutive points
    void SubdivideSegments(double maxGap);
    
    /// Simplify polyline (Douglas-Peucker algorithm)
    /// @param epsilon Maximum allowed distance from simplified line
    void Simplify(double epsilon);

private:
    /// Helper for Douglas-Peucker simplification
    void SimplifyRecursive(int start, int end, double epsilon, CArray<BOOL>& keep);
    
    /// Calculate distance from point to line segment
    double PointToLineDistance(CDPoint p, CDPoint lineStart, CDPoint lineEnd);
};

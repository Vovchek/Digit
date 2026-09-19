#pragma once
#include <aperturecore/geometry/Point.h>
#include <aperturecore/geometry/Bounds.h>
#include <vector>

/// <summary>
/// Represents a single fringe as an ordered polyline of points.
/// A fringe is a continuous curve with a constant fringe number.
/// 
/// ARCHITECTURE NOTES:
/// - Points are stored in ORDERED sequence (spatial coherence preserved)
/// - NO section information stored here (sections are horizontal slices across ALL fringes)
/// - Multiple FringeSegment objects can share the same Number (discontinuous segments)
/// - Sections can be reconstructed from Y-coordinates of all fringes when needed
/// - Uses std::vector (NOT CArray) for modern C++ compliance
/// </summary>
class FringeSegment {
private:
    double number_;              ///< Fringe number (e.g., 0, 0.5, 1.0, ...)
    int index_;                  ///< Segment index for this fringe Number 
                                  ///< (to distinguish discontinuous parts)
    std::vector<aperture::Point> points_; ///< ORDERED sequence of points forming the polyline
    bool isClosed_;               ///< Is this a closed loop?
    
public:
    // ===== Construction =====
    FringeSegment(double number = 0.0, int index = -1);
    FringeSegment(const FringeSegment& other);
    FringeSegment& operator=(const FringeSegment& other);
    ~FringeSegment();
    
    // ===== Point Management =====
    /// Add point to end of polyline
    /// @return Index of newly added point
    int addPoint(aperture::Point p);
    
    /// Insert point at specific index (shifts subsequent points)
    /// @param idx Position to insert (0-based)
    /// @param p Point to insert
    void insertPoint(int idx, aperture::Point p);
    
    /// Remove point at index
    /// @param idx Index of point to remove (0-based)
    void removePoint(int idx);
    
    /// Move existing point to new position
    /// @param idx Index of point to move
    /// @param newP New position
    void movePoint(int idx, aperture::Point newP);
    
    /// Append all points from another fringe
    /// @param other Source fringe to append from
    void appendPoints(const FringeSegment& other);
    void appendPointsReverse(const FringeSegment& other);
    void insertPointsAtStart(const FringeSegment& other);
    void insertPointsAtStartReverse(const FringeSegment& other);

	/// Reverse the order of points in the fringe
	void reversePoints();

    // ===== Queries =====
    /// Get total number of points in fringe
    int getPointCount() const { return static_cast<int>(points_.size()); }
    
    /// Get point at specific index
    /// @param idx Point index (0-based)
    /// @return Point coordinates
    aperture::Point getPoint(size_t idx) const;
    
    /// Set point at specific index
    /// @param idx Point index (0-based)
    /// @param p New point coordinates
    void setPoint(size_t idx, aperture::Point p);

    /// Get fringe id
    int getIndex() const { return index_; }

    /// Set fringe number
    /// @param n New fringe number
    void setIndex(int index) { index_ = index; }

    /// Get fringe number
    double getNumber() const { return number_; }
    
    /// Set fringe number
    /// @param n New fringe number
    void setNumber(double n) { number_ = n; }
    
    /// Check if fringe is closed loop
    bool isClosed() const { return isClosed_; }
    
    /// Set closed loop state
    void setClosed(bool bClosed) { isClosed_ = bClosed; }
    
    // ===== Hit Testing =====
    /// Find nearest point to position within tolerance
    /// @param worldP Point in screen coordinates
    /// @return Index of nearest point, or -1 if none within tolerance
    int findNearestPoint(aperture::Point worldP /* World */, double tolerance = 5.) const;
    
    /// Check if point is on polyline within tolerance
    /// @param P Point in world coordinates
    bool isPointOnPolyline(aperture::Point P /* World */, double tolerance, int& nearestIdx) const;
    
    /// Get bounding rectangle in world coordinates
    aperture::Bounds getBoundingRect() const;
    
    /// Split fringe at given point index
    /// @return New fringe containing points from split point to end
    FringeSegment split(int atIndex);
    
    /// Get arc length of polyline
    double getArcLength() const;
    
    /// Subdivide segments longer than maxGap
    void subdivideSegments(double maxGap);
    
    /// Simplify polyline using Douglas-Peucker algorithm
    /// @param epsilon Maximum distance from original polyline
    void simplify(double epsilon);
    
private:
    void simplifyRecursive(int start, int end, double epsilon, std::vector<bool>& keep);
    double pointToLineDistance(aperture::Point p, aperture::Point lineStart, aperture::Point lineEnd);
};

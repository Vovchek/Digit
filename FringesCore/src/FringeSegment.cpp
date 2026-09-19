/**
 * @file FringeSegment.cpp
 * @brief Implementation of FringeSegment class presenting numbered isoline
 * @author Vladimir N. Chekal
 * @see https://github.com/Vovchek
 */
#include "FringeSegment.h"

#undef max  // Windows.h defines max as a macro
#undef min

#include <cmath>
#include <limits>

// ===== Construction =====

FringeSegment::FringeSegment(double number, int index)
    : number_(number), index_(index), isClosed_(false)
{
}

FringeSegment::FringeSegment(const FringeSegment& other)
    : number_(other.number_), index_(other.index_), isClosed_(other.isClosed_)
{
    points_ = other.points_;  // std::vector copy
}

FringeSegment& FringeSegment::operator=(const FringeSegment& other)
{
    if (this != &other) {
        number_ = other.number_;
        index_ = other.index_;
        isClosed_ = other.isClosed_;
        points_ = other.points_;  // std::vector assignment
    }
    return *this;
}

FringeSegment::~FringeSegment()
{
}

// ===== Point Management =====

int FringeSegment::addPoint(aperture::Point p)
{
    points_.push_back(p);
    return static_cast<int>(points_.size()) - 1;
}

void FringeSegment::insertPoint(int idx, aperture::Point p)
{
    if (idx < 0 || idx > static_cast<int>(points_.size())) return;
    points_.insert(points_.begin() + idx, p);
}

void FringeSegment::removePoint(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(points_.size())) return;
    points_.erase(points_.begin() + idx);
}

void FringeSegment::movePoint(int idx, aperture::Point newP)
{
    if (idx < 0 || idx >= static_cast<int>(points_.size())) return;
    points_[idx] = newP;
}

void FringeSegment::appendPoints(const FringeSegment& other)
{
    points_.insert(points_.end(), other.points_.begin(), other.points_.end());
}

void FringeSegment::appendPointsReverse(const FringeSegment& other)
{
    points_.insert(points_.end(), other.points_.rbegin(), other.points_.rend());
}

void FringeSegment::insertPointsAtStart(const FringeSegment& other)
{
    points_.insert(points_.begin(), other.points_.begin(), other.points_.end());
}

void FringeSegment::insertPointsAtStartReverse(const FringeSegment& other)
{
    points_.insert(points_.begin(), other.points_.rbegin(), other.points_.rend());
}

void FringeSegment::reversePoints()
{
    std::reverse(points_.begin(), points_.end());
}

// ===== Queries =====

aperture::Point FringeSegment::getPoint(size_t idx) const
{
    if (idx < points_.size()) {
        return points_[idx];
    }
    return aperture::Point(0, 0);
}

void FringeSegment::setPoint(size_t idx, aperture::Point p)
{
    if (idx < points_.size()) {
        points_[idx] = p;
    }
}

// ===== Hit Testing =====

int FringeSegment::findNearestPoint(aperture::Point screenP, double tolerance) const
{
    int nearestIdx = -1;
    double minDist = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < points_.size(); i++) {
        double dx = points_[i].x - screenP.x;
        double dy = points_[i].y - screenP.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist < tolerance && dist < minDist) {
            minDist = dist;
            nearestIdx = static_cast<int>(i);
        }
    }
    
    return nearestIdx;
}

bool FringeSegment::isPointOnPolyline(aperture::Point P, double tolerance, int& nearestIdx) const
{
    nearestIdx = findNearestPoint(P, tolerance);
    return (nearestIdx >= 0);
}

aperture::Bounds FringeSegment::getBoundingRect() const
{
    if (points_.empty()) {
        return { 0., 0., 0., 0. };
    }
    
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    
    for (const auto& point : points_) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }
    
    return { minX, minY, maxX, maxY };
}

FringeSegment FringeSegment::split(int atIndex)
{
    FringeSegment newFringe(number_, index_+1);
    
    if (atIndex <= 0 || atIndex >= static_cast<int>(points_.size())) {
        return newFringe;  // Invalid, return empty
    }
    
    // Copy points [atIndex..end] to new fringe
    newFringe.points_.assign(points_.begin() + atIndex, points_.end());
    
    // Remove from original (keep [0..atIndex))
    points_.erase(points_.begin() + atIndex, points_.end());
    
    return newFringe;
}

double FringeSegment::getArcLength() const
{
    double length = 0.0;
    
    for (size_t i = 1; i < points_.size(); i++) {
        double dx = points_[i].x - points_[i - 1].x;
        double dy = points_[i].y - points_[i - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }
    
    if (isClosed() && points_.size() > 2) {
        double dx = points_[0].x - points_[points_.size() - 1].x;
        double dy = points_[0].y - points_[points_.size() - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }
    
    return length;
}

void FringeSegment::subdivideSegments(double maxGap)
{
    size_t originalCount = points_.size();
    
    for (size_t i = 0; i < originalCount - 1; /* increment in loop */) {
        aperture::Point p1 = points_[i];
        aperture::Point p2 = points_[i + 1];
        
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist > maxGap) {
            int nInsert = static_cast<int>(dist / maxGap);
            
            for (int j = 1; j <= nInsert; j++) {
                double t = static_cast<double>(j) / (nInsert + 1);
                aperture::Point pNew(p1.x + t * dx, p1.y + t * dy);
                insertPoint(static_cast<int>(i) + j, pNew);
            }
            
            i += nInsert + 1;  // Skip inserted points
            originalCount += nInsert;
        }
        else {
            i++;
        }
    }
}

void FringeSegment::simplify(double epsilon)
{
    if (points_.size() <= 2) return;
    
    std::vector<bool> keep(points_.size(), false);
    
    keep[0] = true;  // Always keep endpoints
    keep[keep.size() - 1] = true;
    
    simplifyRecursive(0, static_cast<int>(points_.size()) - 1, epsilon, keep);
    
    // Remove points not marked for keeping (reverse iteration)
    for (int i = static_cast<int>(points_.size()) - 1; i >= 0; i--) {
        if (!keep[i]) {
            points_.erase(points_.begin() + i);
        }
    }
}

void FringeSegment::simplifyRecursive(int start, int end, double epsilon, std::vector<bool>& keep)
{
    if (end - start <= 1) return;
    
    // Find point farthest from line segment
    double maxDist = 0;
    int maxIdx = start;
    
    aperture::Point p1 = points_[start];
    aperture::Point p2 = points_[end];
    
    for (int i = start + 1; i < end; i++) {
        double dist = pointToLineDistance(points_[i], p1, p2);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }
    
    if (maxDist > epsilon) {
        keep[maxIdx] = true;
        simplifyRecursive(start, maxIdx, epsilon, keep);
        simplifyRecursive(maxIdx, end, epsilon, keep);
    }
}

double FringeSegment::pointToLineDistance(aperture::Point p, aperture::Point lineStart, aperture::Point lineEnd)
{
    double A = p.x - lineStart.x;
    double B = p.y - lineStart.y;
    double C = lineEnd.x - lineStart.x;
    double D = lineEnd.y - lineStart.y;
    
    double dot = A * C + B * D;
    double len_sq = C * C + D * D;
    double param = (len_sq != 0) ? dot / len_sq : -1;
    
    double xx, yy;
    if (param < 0) {
        xx = lineStart.x;
        yy = lineStart.y;
    }
    else if (param > 1) {
        xx = lineEnd.x;
        yy = lineEnd.y;
    }
    else {
        xx = lineStart.x + param * C;
        yy = lineStart.y + param * D;
    }
    
    double dx = p.x - xx;
    double dy = p.y - yy;
    return std::sqrt(dx * dx + dy * dy);
}

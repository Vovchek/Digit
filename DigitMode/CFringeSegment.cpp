#include "stdafx.h"
#include "CFringeSegment.h"

#undef max  // Windows.h defines max as a macro
#undef min

#include <cmath>
#include <limits>

// ===== Construction =====

CFringeSegment::CFringeSegment(double number, int index)
    : m_Number(number), m_Index(index), m_bClosed(FALSE)
{
}

CFringeSegment::CFringeSegment(const CFringeSegment& other)
    : m_Number(other.m_Number), m_Index(other.m_Index), m_bClosed(other.m_bClosed)
{
    m_Points = other.m_Points;  // std::vector copy
}

CFringeSegment& CFringeSegment::operator=(const CFringeSegment& other)
{
    if (this != &other) {
        m_Number = other.m_Number;
        m_Index = other.m_Index;
        m_bClosed = other.m_bClosed;
        m_Points = other.m_Points;  // std::vector assignment
    }
    return *this;
}

CFringeSegment::~CFringeSegment()
{
}

// ===== Point Management =====

int CFringeSegment::AddPoint(CDPoint p)
{
    m_Points.push_back(p);
    return static_cast<int>(m_Points.size()) - 1;
}

void CFringeSegment::InsertPoint(int idx, CDPoint p)
{
    if (idx < 0 || idx > static_cast<int>(m_Points.size())) return;
    m_Points.insert(m_Points.begin() + idx, p);
}

void CFringeSegment::RemovePoint(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(m_Points.size())) return;
    m_Points.erase(m_Points.begin() + idx);
}

void CFringeSegment::MovePoint(int idx, CDPoint newP)
{
    if (idx < 0 || idx >= static_cast<int>(m_Points.size())) return;
    m_Points[idx] = newP;
}

void CFringeSegment::AppendPoints(const CFringeSegment& other)
{
    m_Points.insert(m_Points.end(), other.m_Points.begin(), other.m_Points.end());
}

void CFringeSegment::AppendPointsReverse(const CFringeSegment& other)
{
    m_Points.insert(m_Points.end(), other.m_Points.rbegin(), other.m_Points.rend());
}

void CFringeSegment::InsertPointsAtStart(const CFringeSegment& other)
{
    m_Points.insert(m_Points.begin(), other.m_Points.begin(), other.m_Points.end());
}

void CFringeSegment::InsertPointsAtStartReverse(const CFringeSegment& other)
{
    m_Points.insert(m_Points.begin(), other.m_Points.rbegin(), other.m_Points.rend());
}

// ===== Queries =====

CDPoint CFringeSegment::GetPoint(int idx) const
{
    if (idx >= 0 && idx < static_cast<int>(m_Points.size())) {
        return m_Points[idx];
    }
    return CDPoint(0, 0);
}

void CFringeSegment::SetPoint(int idx, CDPoint p)
{
    if (idx >= 0 && idx < static_cast<int>(m_Points.size())) {
        m_Points[idx] = p;
    }
}

// ===== Hit Testing =====

int CFringeSegment::FindNearestPoint(CPoint screenP, int tolerance)
{
    int nearestIdx = -1;
    double minDist = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < m_Points.size(); i++) {
        double dx = m_Points[i].x - screenP.x;
        double dy = m_Points[i].y - screenP.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist < tolerance && dist < minDist) {
            minDist = dist;
            nearestIdx = static_cast<int>(i);
        }
    }
    
    return nearestIdx;
}

BOOL CFringeSegment::IsPointOnPolyline(CPoint P, int tolerance, int& nearestIdx)
{
    nearestIdx = FindNearestPoint(P, tolerance);
    return (nearestIdx >= 0);
}

CRect CFringeSegment::GetBoundingRect() const
{
    if (m_Points.empty()) {
        return CRect(0, 0, 0, 0);
    }
    
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    
    for (const auto& point : m_Points) {
        if (point.x < minX) minX = point.x;
        if (point.x > maxX) maxX = point.x;
        if (point.y < minY) minY = point.y;
        if (point.y > maxY) maxY = point.y;
    }
    
    return CRect(static_cast<int>(minX), static_cast<int>(minY), 
                 static_cast<int>(maxX), static_cast<int>(maxY));
}

// ===== Drawing =====

void CFringeSegment::DrawDots(CDC* pDC, int dotSize, COLORREF color)
{
    int half = dotSize / 2;
    CBrush brush(color);
    CBrush* oldBrush = pDC->SelectObject(&brush);
    
    for (const auto& point : m_Points) {
        CPoint p(static_cast<int>(point.x), static_cast<int>(point.y));
        pDC->Ellipse(p.x - half, p.y - half, p.x + half, p.y + half);
    }
    
    pDC->SelectObject(oldBrush);
}

void CFringeSegment::DrawPolyline(CDC* pDC, COLORREF color)
{
    if (m_Points.size() < 2) return;
    
    CPen pen(PS_SOLID, 1, color);
    CPen* oldPen = pDC->SelectObject(&pen);
    
    CPoint p0(static_cast<int>(m_Points[0].x), static_cast<int>(m_Points[0].y));
    pDC->MoveTo(p0);
    
    for (size_t i = 1; i < m_Points.size(); i++) {
        CPoint p(static_cast<int>(m_Points[i].x), static_cast<int>(m_Points[i].y));
        pDC->LineTo(p);
    }
    
    if (m_bClosed && m_Points.size() > 2) {
        pDC->LineTo(p0);
    }
    
    pDC->SelectObject(oldPen);
}

void CFringeSegment::DrawFull(CDC* pDC, int dotSize, COLORREF lineColor, COLORREF dotColor)
{
    DrawPolyline(pDC, lineColor);
    DrawDots(pDC, dotSize, dotColor);
}

// ===== Advanced Operations (Phase 5) =====

CFringeSegment CFringeSegment::Split(int atIndex)
{
    CFringeSegment newFringe(m_Number);
    
    if (atIndex <= 0 || atIndex >= static_cast<int>(m_Points.size())) {
        return newFringe;  // Invalid, return empty
    }
    
    // Copy points [atIndex..end] to new fringe
    newFringe.m_Points.assign(m_Points.begin() + atIndex, m_Points.end());
    
    // Remove from original (keep [0..atIndex))
    m_Points.erase(m_Points.begin() + atIndex, m_Points.end());
    
    return newFringe;
}

double CFringeSegment::GetArcLength() const
{
    double length = 0.0;
    
    for (size_t i = 1; i < m_Points.size(); i++) {
        double dx = m_Points[i].x - m_Points[i - 1].x;
        double dy = m_Points[i].y - m_Points[i - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }
    
    if (m_bClosed && m_Points.size() > 2) {
        double dx = m_Points[0].x - m_Points[m_Points.size() - 1].x;
        double dy = m_Points[0].y - m_Points[m_Points.size() - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }
    
    return length;
}

void CFringeSegment::SubdivideSegments(double maxGap)
{
    size_t originalCount = m_Points.size();
    
    for (size_t i = 0; i < originalCount - 1; /* increment in loop */) {
        CDPoint p1 = m_Points[i];
        CDPoint p2 = m_Points[i + 1];
        
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist > maxGap) {
            int nInsert = static_cast<int>(dist / maxGap);
            
            for (int j = 1; j <= nInsert; j++) {
                double t = static_cast<double>(j) / (nInsert + 1);
                CDPoint pNew(p1.x + t * dx, p1.y + t * dy);
                InsertPoint(static_cast<int>(i) + j, pNew);
            }
            
            i += nInsert + 1;  // Skip inserted points
            originalCount += nInsert;
        }
        else {
            i++;
        }
    }
}

void CFringeSegment::Simplify(double epsilon)
{
    if (m_Points.size() <= 2) return;
    
    std::vector<bool> keep(m_Points.size(), false);
    
    keep[0] = true;  // Always keep endpoints
    keep[keep.size() - 1] = true;
    
    SimplifyRecursive(0, static_cast<int>(m_Points.size()) - 1, epsilon, keep);
    
    // Remove points not marked for keeping (reverse iteration)
    for (int i = static_cast<int>(m_Points.size()) - 1; i >= 0; i--) {
        if (!keep[i]) {
            m_Points.erase(m_Points.begin() + i);
        }
    }
}

void CFringeSegment::SimplifyRecursive(int start, int end, double epsilon, std::vector<bool>& keep)
{
    if (end - start <= 1) return;
    
    // Find point farthest from line segment
    double maxDist = 0;
    int maxIdx = start;
    
    CDPoint p1 = m_Points[start];
    CDPoint p2 = m_Points[end];
    
    for (int i = start + 1; i < end; i++) {
        double dist = PointToLineDistance(m_Points[i], p1, p2);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }
    
    if (maxDist > epsilon) {
        keep[maxIdx] = true;
        SimplifyRecursive(start, maxIdx, epsilon, keep);
        SimplifyRecursive(maxIdx, end, epsilon, keep);
    }
}

double CFringeSegment::PointToLineDistance(CDPoint p, CDPoint lineStart, CDPoint lineEnd)
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

#include "stdafx.h"
#include "CFringeSegment.h"
#include <cmath>
#include <float.h>

// ===== Construction =====

CFringeSegment::CFringeSegment(double number, int index)
    : m_Number(number), m_Index(index), m_bClosed(FALSE)
{
}

CFringeSegment::CFringeSegment(const CFringeSegment& other)
    : m_Number(other.m_Number), m_Index(other.m_Index), m_bClosed(other.m_bClosed)
{
    m_Points.Copy(other.m_Points);
}

CFringeSegment& CFringeSegment::operator=(const CFringeSegment& other)
{
    if (this != &other) {
        m_Number = other.m_Number;
		m_Index = other.m_Index;
        m_bClosed = other.m_bClosed;
        m_Points.RemoveAll();
        m_Points.Copy(other.m_Points);
    }
    return *this;
}

CFringeSegment::~CFringeSegment()
{
}

// ===== Point Management =====

int CFringeSegment::AddPoint(CDPoint p)
{
    m_Points.Add(p);
    return m_Points.GetSize() - 1;
}

void CFringeSegment::InsertPoint(int idx, CDPoint p)
{
    if (idx < 0 || idx > m_Points.GetSize()) return;
    m_Points.InsertAt(idx, p);
}

void CFringeSegment::RemovePoint(int idx)
{
    if (idx < 0 || idx >= m_Points.GetSize()) return;
    m_Points.RemoveAt(idx);
}

void CFringeSegment::MovePoint(int idx, CDPoint newP)
{
    if (idx < 0 || idx >= m_Points.GetSize()) return;
    m_Points[idx] = newP;
}

void CFringeSegment::AppendPoints(const CFringeSegment& other)
{
    for (int i = 0; i < other.m_Points.GetSize(); i++) {
        m_Points.Add(other.m_Points[i]);
    }
}

// ===== Queries =====

CDPoint CFringeSegment::GetPoint(int idx) const
{
    if (idx >= 0 && idx < m_Points.GetSize()) {
        return m_Points[idx];
    }
    return CDPoint(0, 0);
}

void CFringeSegment::SetPoint(int idx, CDPoint p)
{
    if (idx >= 0 && idx < m_Points.GetSize()) {
        m_Points[idx] = p;
    }
}

// ===== Hit Testing =====

int CFringeSegment::FindNearestPoint(CPoint screenP, int tolerance)
{
    int nearestIdx = -1;
    double minDist = DBL_MAX;
    
    for (int i = 0; i < m_Points.GetSize(); i++) {
        double dx = m_Points[i].x - screenP.x;
        double dy = m_Points[i].y - screenP.y;
        double dist = sqrt(dx * dx + dy * dy);
        
        if (dist < tolerance && dist < minDist) {
            minDist = dist;
            nearestIdx = i;
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
    if (m_Points.GetSize() == 0) {
        return CRect(0, 0, 0, 0);
    }
    
    double minX = DBL_MAX, minY = DBL_MAX;
    double maxX = -DBL_MAX, maxY = -DBL_MAX;
    
    for (int i = 0; i < m_Points.GetSize(); i++) {
        if (m_Points[i].x < minX) minX = m_Points[i].x;
        if (m_Points[i].x > maxX) maxX = m_Points[i].x;
        if (m_Points[i].y < minY) minY = m_Points[i].y;
        if (m_Points[i].y > maxY) maxY = m_Points[i].y;
    }
    
    return CRect((int)minX, (int)minY, (int)maxX, (int)maxY);
}

// ===== Drawing =====

void CFringeSegment::DrawDots(CDC* pDC, int dotSize, COLORREF color)
{
    int half = dotSize / 2;
    CBrush brush(color);
    CBrush* oldBrush = pDC->SelectObject(&brush);
    
    for (int i = 0; i < m_Points.GetSize(); i++) {
        CPoint p((int)m_Points[i].x, (int)m_Points[i].y);
        pDC->Ellipse(p.x - half, p.y - half, p.x + half, p.y + half);
    }
    
    pDC->SelectObject(oldBrush);
}

void CFringeSegment::DrawPolyline(CDC* pDC, COLORREF color)
{
    if (m_Points.GetSize() < 2) return;
    
    CPen pen(PS_SOLID, 1, color);
    CPen* oldPen = pDC->SelectObject(&pen);
    
    CPoint p0((int)m_Points[0].x, (int)m_Points[0].y);
    pDC->MoveTo(p0);
    
    for (int i = 1; i < m_Points.GetSize(); i++) {
        CPoint p((int)m_Points[i].x, (int)m_Points[i].y);
        pDC->LineTo(p);
    }
    
    if (m_bClosed && m_Points.GetSize() > 2) {
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
    
    if (atIndex <= 0 || atIndex >= m_Points.GetSize()) {
        return newFringe;  // Invalid, return empty
    }
    
    // Move points [atIndex..end] to new fringe
    for (int i = atIndex; i < m_Points.GetSize(); i++) {
        newFringe.AddPoint(m_Points[i]);
    }
    
    // Remove from original
    for (int i = m_Points.GetSize() - 1; i >= atIndex; i--) {
        m_Points.RemoveAt(i);
    }
    
    return newFringe;
}

double CFringeSegment::GetArcLength() const
{
    double length = 0.0;
    
    for (int i = 1; i < m_Points.GetSize(); i++) {
        double dx = m_Points[i].x - m_Points[i - 1].x;
        double dy = m_Points[i].y - m_Points[i - 1].y;
        length += sqrt(dx * dx + dy * dy);
    }
    
    if (m_bClosed && m_Points.GetSize() > 2) {
        double dx = m_Points[0].x - m_Points[m_Points.GetSize() - 1].x;
        double dy = m_Points[0].y - m_Points[m_Points.GetSize() - 1].y;
        length += sqrt(dx * dx + dy * dy);
    }
    
    return length;
}

void CFringeSegment::SubdivideSegments(double maxGap)
{
    int originalCount = m_Points.GetSize();
    
    for (int i = 0; i < originalCount - 1; /* increment in loop */) {
        CDPoint p1 = m_Points[i];
        CDPoint p2 = m_Points[i + 1];
        
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dist = sqrt(dx * dx + dy * dy);
        
        if (dist > maxGap) {
            int nInsert = (int)(dist / maxGap);
            
            for (int j = 1; j <= nInsert; j++) {
                double t = (double)j / (nInsert + 1);
                CDPoint pNew(p1.x + t * dx, p1.y + t * dy);
                InsertPoint(i + j, pNew);
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
    if (m_Points.GetSize() <= 2) return;
    
    CArray<BOOL> keep;
    keep.SetSize(m_Points.GetSize());
    for (int i = 0; i < keep.GetSize(); i++) {
        keep[i] = FALSE;
    }
    
    keep[0] = TRUE;  // Always keep endpoints
    keep[keep.GetSize() - 1] = TRUE;
    
    SimplifyRecursive(0, m_Points.GetSize() - 1, epsilon, keep);
    
    // Remove points not marked for keeping
    for (int i = m_Points.GetSize() - 1; i >= 0; i--) {
        if (!keep[i]) {
            m_Points.RemoveAt(i);
        }
    }
}

void CFringeSegment::SimplifyRecursive(int start, int end, double epsilon, CArray<BOOL>& keep)
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
        keep[maxIdx] = TRUE;
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
    return sqrt(dx * dx + dy * dy);
}

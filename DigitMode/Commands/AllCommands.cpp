#include "stdafx.h"
#include "AllCommands.h"
#include <algorithm>

namespace DigitMode {

// ---- AddDotCommand definitions ----
AddDotCommand::AddDotCommand(CDigitInfo* pD, int iSeg, int iD, CDPoint p)
    : pDigit(pD), iSegment(iSeg), iDot(iD), point(p) {}

void AddDotCommand::Execute() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.InsertPoint(iDot, point);
}

void AddDotCommand::Undo() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.RemovePoint(iDot);
}

// ---- RemoveLastDotCommand definitions ----
RemoveLastDotCommand::RemoveLastDotCommand(CDigitInfo* pD, int iSeg)
    : pDigit(pD), iSegment(iSeg) {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    iDot = segment.GetPointCount() - 1;
    savedPoint = segment.GetPoint(iDot);
}

void RemoveLastDotCommand::Execute() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.RemovePoint(iDot);
}

void RemoveLastDotCommand::Undo() {
    CFringeSegment& segment = pDigit->Fringes[iSegment];
    segment.InsertPoint(iDot, savedPoint);
}

// Minimal stubs for other commands (implementations later)

CreateSegmentCommand::CreateSegmentCommand(CDigitInfo& doc, const std::vector<CPoint2d>& points, double number)
    : m_doc(doc), m_points(points), m_number(number), m_createdIndex(static_cast<size_t>(-1)) {}

void CreateSegmentCommand::Execute() {
    CFringeSegment seg(m_number, static_cast<int>(m_doc.Fringes.size()));
    for (const auto& p : m_points) seg.AddPoint(p);
    m_doc.Fringes.push_back(seg);
    m_createdIndex = m_doc.Fringes.size() - 1;
}

void CreateSegmentCommand::Undo() {
    if (m_createdIndex < m_doc.Fringes.size())
        m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(m_createdIndex));
}

ExtendSegmentCommand::ExtendSegmentCommand(CDigitInfo& doc, size_t segmentIndex, bool atHead, const CPoint2d& point)
    : m_doc(doc), m_segmentIndex(segmentIndex), m_atHead(atHead), m_point(point) {}

void ExtendSegmentCommand::Execute() {
    auto& seg = m_doc.Fringes[static_cast<int>(m_segmentIndex)];
    if (m_atHead) seg.InsertPoint(0, m_point);
    else seg.AddPoint(m_point);
}

void ExtendSegmentCommand::Undo() {
    auto& seg = m_doc.Fringes[static_cast<int>(m_segmentIndex)];
    if (m_atHead) seg.RemovePoint(0);
    else seg.RemovePoint(seg.GetPointCount() - 1);
}

MoveDotCommand::MoveDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex, const CPoint2d& oldPos, const CPoint2d& newPos)
    : m_doc(doc), m_segmentIndex(segmentIndex), m_dotIndex(dotIndex), m_oldPos(oldPos), m_newPos(newPos) {}

void MoveDotCommand::Execute() { m_doc.Fringes[static_cast<int>(m_segmentIndex)].SetPoint(static_cast<int>(m_dotIndex), m_newPos); }
void MoveDotCommand::Undo() { m_doc.Fringes[static_cast<int>(m_segmentIndex)].SetPoint(static_cast<int>(m_dotIndex), m_oldPos); }

DeleteDotCommand::DeleteDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex)
    : m_doc(doc), m_segmentIndex(segmentIndex), m_dotIndex(dotIndex) {}

void DeleteDotCommand::Execute() {
    auto& seg = m_doc.Fringes[static_cast<int>(m_segmentIndex)];
    m_removedPoint = seg.GetPoint(static_cast<int>(m_dotIndex));
    seg.RemovePoint(static_cast<int>(m_dotIndex));
}

void DeleteDotCommand::Undo() { m_doc.Fringes[static_cast<int>(m_segmentIndex)].InsertPoint(static_cast<int>(m_dotIndex), m_removedPoint); }

SplitSegmentCommand::SplitSegmentCommand(CDigitInfo& doc, size_t segmentIndex, size_t splitDotIndex)
    : m_doc(doc), m_originalIndex(segmentIndex), m_newIndex(static_cast<size_t>(-1)), m_dotIndex(splitDotIndex){}

void SplitSegmentCommand::Execute() {
    auto& seg = m_doc.Fringes[static_cast<int>(m_originalIndex)];
    m_doc.Fringes.push_back(seg.Split(m_dotIndex));
	m_newIndex = m_doc.Fringes.size() - 1;
}

void SplitSegmentCommand::Undo() {
	m_doc.Fringes[static_cast<int>(m_originalIndex)].AppendPoints(m_doc.Fringes[static_cast<int>(m_newIndex)]);
	m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(m_newIndex));
}

ConnectSegmentsCommand::ConnectSegmentsCommand(CDigitInfo& doc, size_t segA, bool endA, size_t segB, bool endB)
    : m_doc(doc), m_segA(segA), m_segB(segB), m_endA(endA), m_endB(endB) {
	m_lenA = doc.Fringes[static_cast<int>(segA)].GetPointCount();
	m_numB = doc.Fringes[static_cast<int>(segB)].GetNumber();
    m_indexB = doc.Fringes[static_cast<int>(segB)].GetIndex();
}

void ConnectSegmentsCommand::Execute() {
    // Minimal implementation: append B into A and erase B
    auto& A = m_doc.Fringes[static_cast<int>(m_segA)];
    auto& B = m_doc.Fringes[static_cast<int>(m_segB)];
    if (m_endA) {
        if(m_endB)
            A.AppendPointsReverse(B);
        else
            A.AppendPoints(B);
    } else {
        if(m_endB)
            A.InsertPointsAtStartReverse(B);
        else
            A.InsertPointsAtStart(B);
	}
    m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(m_segB));
}

void ConnectSegmentsCommand::Undo() {
    // Restore A and reinsert B
    auto& A = m_doc.Fringes[static_cast<int>(m_segA)];
	CFringeSegment B;
    if(m_endA) {
        B = A.Split(m_lenA);
    } else {
		B = A.Split(A.GetPointCount() - m_lenA);
		std::swap(A, B);
    }
    if (m_endB)
        B.ReversePoints();
    B.SetNumber(static_cast<double>(m_numB));
    B.SetIndex(m_indexB);
    m_doc.Fringes.insert(m_doc.Fringes.begin() + static_cast<int>(m_segB), B);
}

RenumberSegmentsCommand::RenumberSegmentsCommand(CDigitInfo& doc, const std::vector<size_t>& segmentIndices, double newNumber)
    : m_doc(doc), m_indices(segmentIndices), m_newNumber(newNumber) {}

void RenumberSegmentsCommand::Execute() {
    m_oldNumbers.clear();
    for (size_t idx : m_indices) {
        m_oldNumbers.push_back(m_doc.Fringes[static_cast<int>(idx)].GetNumber());
        m_doc.Fringes[static_cast<int>(idx)].SetNumber(m_newNumber);
    }
}

void RenumberSegmentsCommand::Undo() {
    for (size_t i = 0; i < m_indices.size(); ++i) {
        m_doc.Fringes[static_cast<int>(m_indices[i])].SetNumber(m_oldNumbers[i]);
    }
}

DeleteSegmentsCommand::DeleteSegmentsCommand(CDigitInfo& doc, const std::vector<size_t>& segmentIndices)
    : m_doc(doc), m_indices(segmentIndices) {}

void DeleteSegmentsCommand::Execute() {
    std::vector<size_t> idxs = m_indices;
    std::sort(idxs.rbegin(), idxs.rend());
    for (size_t idx : idxs) {
        RemovedSegment rs; rs.index = idx; rs.segment = m_doc.Fringes[static_cast<int>(idx)];
        m_removed.push_back(rs);
        m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(idx));
    }
}

void DeleteSegmentsCommand::Undo() {
    std::sort(m_removed.begin(), m_removed.end(), [](const RemovedSegment& a, const RemovedSegment& b){ return a.index < b.index; });
    for (const auto& rs : m_removed) {
        m_doc.Fringes.insert(m_doc.Fringes.begin() + static_cast<int>(rs.index), rs.segment);
    }
}

} // namespace DigitMode



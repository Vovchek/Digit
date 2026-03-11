#include "stdafx.h"
#include "AllCommands.h"
#include "AutoNumberingAlgorithmSaddles.h"
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
CreateSegmentCommand::CreateSegmentCommand(CDigitInfo& doc, const std::vector<CDPoint>& points, double number)
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

ExtendSegmentCommand::ExtendSegmentCommand(CDigitInfo& doc, size_t segmentIndex, bool atHead, const CDPoint& point)
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

MoveDotCommand::MoveDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex, const CDPoint& oldPos, const CDPoint& newPos)
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
	m_segA_before = m_doc.Fringes[static_cast<int>(m_segA)];
	m_segB_before = m_doc.Fringes[static_cast<int>(m_segB)];
	m_segA_after = segA;
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
            A.InsertPointsAtStart(B);
        else
            A.InsertPointsAtStartReverse(B);
	}
    m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(m_segB));
	m_segA_after = (m_segB < m_segA) ? m_segA - 1 : m_segA;// Save data for undo
}
// TODO: fix segment index restoration
void ConnectSegmentsCommand::Undo() {
    // Remove merged A
    m_doc.Fringes.erase(m_doc.Fringes.begin() + m_segA_after);

    // Reinsert original segments in correct order
    if (m_segA < m_segB) {
        m_doc.Fringes.insert(m_doc.Fringes.begin() + m_segA, m_segA_before);
        m_doc.Fringes.insert(m_doc.Fringes.begin() + m_segB, m_segB_before);
    }
    else {
        m_doc.Fringes.insert(m_doc.Fringes.begin() + m_segB, m_segB_before);
        m_doc.Fringes.insert(m_doc.Fringes.begin() + m_segA, m_segA_before);
    }
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

// ---- AutoNumberingCommand definitions ----
AutoNumberingCommand::AutoNumberingCommand(
    CDigitInfo& doc,
    const std::vector<size_t>& trustedSegmentIndices,
    double step,
    double confidenceThreshold)
    : m_doc(doc), m_trustedIndices(trustedSegmentIndices), m_step(step), 
      m_confidenceThreshold(confidenceThreshold) {
    
    // If no trusted indices provided, algo will guess
    
    // Save original fringes for undo
    m_originalFringes = m_doc.Fringes;
}

void AutoNumberingCommand::Execute() {
    // Call the automatic numbering algorithm
    auto newTrusted = AutoNumberFringes(
        m_doc.Fringes,
        m_trustedIndices,
        m_step,
        m_confidenceThreshold
    );
    std::sort(m_doc.Fringes.begin(), m_doc.Fringes.end(), [](const auto& a, const auto& b) {return a.GetNumber() < b.GetNumber(); });
    TRACE("AutoNumberingCommand::Execute: %zu fringes numbered, %zu trusted\n",
          m_doc.Fringes.size(), newTrusted.size());
}

void AutoNumberingCommand::Undo() {
    // Restore original fringes
    m_doc.Fringes = m_originalFringes;
    
    TRACE("AutoNumberingCommand::Undo: Restored original fringe numbers\n");
}

// ---- DeleteSelectionCommand definitions ----
DeleteSelectionCommand::DeleteSelectionCommand(CDigitInfo& doc, const SelectionManager& selectionManager)
    : m_doc(doc) {
    // Save original fringes for undo
    m_originalFringes = m_doc.Fringes;
}

void DeleteSelectionCommand::Execute() {
    if (m_doc.selectionManager.IsEmpty()) {
        return; // Nothing to delete
    }

    SelectionLevel level = m_doc.selectionManager.GetLevel();
    
    if (level == SelectionLevel::Segment || level == SelectionLevel::Fringe) {
        // Collect all segment indices to delete
        std::vector<size_t> indicesToDelete;
        
        for (size_t i = 0; i < m_doc.selectionManager.GetCount(); ++i) {
            const auto& obj = m_doc.selectionManager.GetAt(i);
            if (obj.iSegment >= 0 && static_cast<size_t>(obj.iSegment) < m_doc.Fringes.size()) {
                indicesToDelete.push_back(static_cast<size_t>(obj.iSegment));
            }
        }
        
        // Sort in descending order and remove duplicates
        std::sort(indicesToDelete.rbegin(), indicesToDelete.rend());
        indicesToDelete.erase(std::unique(indicesToDelete.begin(), indicesToDelete.end()), 
                             indicesToDelete.end());
        
        // Delete segments in descending order
        for (size_t idx : indicesToDelete) {
            m_deletedSegmentIndices.push_back(idx);
            m_deletedSegmentPositions.push_back(m_doc.Fringes.size() - 1);
            m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(idx));
        }
    }
    else if (level == SelectionLevel::Dot) {
        // Delete individual dots
        std::vector<DeletedDot> dotsToDelete;
        
        for (size_t i = 0; i < m_doc.selectionManager.GetCount(); ++i) {
            const auto& obj = m_doc.selectionManager.GetAt(i);
            if (obj.iSegment >= 0 && obj.iDot >= 0) {
                auto& seg = m_doc.Fringes[static_cast<size_t>(obj.iSegment)];
                if (obj.iDot < seg.GetPointCount()) {
                    DeletedDot dd;
                    dd.segmentIndex = static_cast<size_t>(obj.iSegment);
                    dd.dotIndex = static_cast<size_t>(obj.iDot);
                    dd.point = seg.GetPoint(obj.iDot);
                    dotsToDelete.push_back(dd);
                }
            }
        }
        
        // Sort by segment and dot index in descending order
        std::sort(dotsToDelete.begin(), dotsToDelete.end(), 
                 [](const DeletedDot& a, const DeletedDot& b) {
                     if (a.segmentIndex != b.segmentIndex)
                         return a.segmentIndex > b.segmentIndex;
                     return a.dotIndex > b.dotIndex;
                 });
        
        // Delete dots and remove segments if they become empty
        for (const auto& dd : dotsToDelete) {
            auto& seg = m_doc.Fringes[dd.segmentIndex];
            seg.RemovePoint(static_cast<int>(dd.dotIndex));
            m_deletedDots.push_back(dd);
            
            // If segment is now empty, delete it
            if (seg.GetPointCount() == 0) {
                m_deletedSegmentIndices.push_back(dd.segmentIndex);
                m_doc.Fringes.erase(m_doc.Fringes.begin() + static_cast<int>(dd.segmentIndex));
            }
        }
    }
    else if (level == SelectionLevel::Edge) {
        // Delete edges (split segments)
        std::vector<DeletedEdge> edgesToDelete;
        
        for (size_t i = 0; i < m_doc.selectionManager.GetCount(); ++i) {
            const auto& obj = m_doc.selectionManager.GetAt(i);
            if (obj.iSegment >= 0 && obj.iEdge >= 0) {
                auto& seg = m_doc.Fringes[static_cast<size_t>(obj.iSegment)];
                if (obj.iEdge < seg.GetPointCount() - 1) {
                    DeletedEdge de;
                    de.segmentIndex = static_cast<size_t>(obj.iSegment);
                    de.edgeStartIndex = static_cast<size_t>(obj.iEdge);
                    edgesToDelete.push_back(de);
                }
            }
        }
        
        // Sort in descending order to avoid index invalidation
        std::sort(edgesToDelete.begin(), edgesToDelete.end(),
                 [](const DeletedEdge& a, const DeletedEdge& b) {
                     if (a.segmentIndex != b.segmentIndex)
                         return a.segmentIndex > b.segmentIndex;
                     return a.edgeStartIndex > b.edgeStartIndex;
                 });
        
        // Delete edges by splitting segments
        for (const auto& de : edgesToDelete) {
            auto& seg = m_doc.Fringes[de.segmentIndex];
            m_deletedEdges.push_back(de);
            // Split segment at edge (removes the edge and creates two segments)
            m_doc.Fringes.push_back(seg.Split(de.edgeStartIndex + 1));
        }
    }
    
    // Clear the selection after deletion
    m_doc.selectionManager.Clear();
    
    TRACE("DeleteSelectionCommand::Execute: Deleted %zu dots, %zu edges, %zu segments\n",
          m_deletedDots.size(), m_deletedEdges.size(), m_deletedSegmentIndices.size());
}

void DeleteSelectionCommand::Undo() {
    // Restore original fringes
    m_doc.Fringes = m_originalFringes;
    
    TRACE("DeleteSelectionCommand::Undo: Restored original state\n");
}

} // namespace DigitMode



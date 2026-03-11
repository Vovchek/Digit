#pragma once

#include "Command.h"
#include "ReplaceShapeCommand.h"
#include "AddShapeCommand.h"
#include "RemoveShapeCommand.h"
#include "../DigitInfo.h"
#include "../CFringeSegment.h"
#include "../../MGTools/Include/Utils/BaseDataType.h"
#include <vector>
#include <string>

namespace DigitMode {

// CreateSegmentCommand
class CreateSegmentCommand : public Command {
public:
    CreateSegmentCommand(CDigitInfo& doc, const std::vector<CDPoint>& points, double number);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Create Segment"; }
private:
    CDigitInfo& m_doc;
    std::vector<CDPoint> m_points;
    double m_number;
    size_t m_createdIndex;
};

// ExtendSegmentCommand
class ExtendSegmentCommand : public Command {
public:
    ExtendSegmentCommand(CDigitInfo& doc, size_t segmentIndex, bool atHead, const CDPoint& point);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Extend Segment"; }
private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    bool m_atHead;
    CDPoint m_point;
};

// MoveDotCommand
class MoveDotCommand : public Command {
public:
    MoveDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex, const CDPoint& oldPos, const CDPoint& newPos);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Move Dot"; }
private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    size_t m_dotIndex;
    CDPoint m_oldPos;
    CDPoint m_newPos;
};

// DeleteDotCommand
class DeleteDotCommand : public Command {
public:
    DeleteDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Dot"; }
private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    size_t m_dotIndex;
    CDPoint m_removedPoint;
};

// SplitSegmentCommand
class SplitSegmentCommand : public Command {
public:
    SplitSegmentCommand(CDigitInfo& doc, size_t segmentIndex, size_t splitDotIndex);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Split Segment"; }
private:
    CDigitInfo& m_doc;
    size_t m_originalIndex;
    size_t m_newIndex;
    size_t m_dotIndex;
};

// ConnectSegmentsCommand
class ConnectSegmentsCommand : public Command {
public:
    ConnectSegmentsCommand(CDigitInfo& doc, size_t segA, bool endA, size_t segB, bool endB);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Connect Segments"; }
private:
    CDigitInfo& m_doc;
    size_t m_segA;
    size_t m_segA_after;
    size_t m_segB;
    bool m_endA;
    bool m_endB;
    CFringeSegment m_segA_before;
    CFringeSegment m_segB_before;
};

// RenumberSegmentsCommand
class RenumberSegmentsCommand : public Command {
public:
    RenumberSegmentsCommand(CDigitInfo& doc, const std::vector<size_t>& segmentIndices, double newNumber);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Renumber Segments"; }
private:
    CDigitInfo& m_doc;
    std::vector<size_t> m_indices;
    std::vector<double> m_oldNumbers;
    double m_newNumber;
};

// DeleteSegmentsCommand
class DeleteSegmentsCommand : public Command {
public:
    DeleteSegmentsCommand(CDigitInfo& doc, const std::vector<size_t>& segmentIndices);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Segments"; }
private:
    CDigitInfo& m_doc;
    struct RemovedSegment { size_t index; CFringeSegment segment; };
    std::vector<RemovedSegment> m_removed;
    std::vector<size_t> m_indices;
};

// AddDotCommand (moved)
class AddDotCommand : public Command {
public:
    AddDotCommand(CDigitInfo* pD, int iSeg, int iD, CDPoint p);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Add Dot"; }
private:
    CDigitInfo* pDigit;
    int iSegment;
    int iDot;
    CDPoint point;
};

// RemoveLastDotCommand (moved)
class RemoveLastDotCommand : public Command {
public:
    RemoveLastDotCommand(CDigitInfo* pD, int iSeg);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Remove Dot"; }
private:
    CDigitInfo* pDigit;
    int iSegment;
    int iDot;
    CDPoint savedPoint;
};

// AutoNumberingCommand - Automatic fringe numbering
class AutoNumberingCommand : public Command {
public:
    /**
     * @brief Auto-number fringes using constraint-based solver
     * 
     * @param doc Reference to CDigitInfo document
     * @param trustedSegmentIndices Indices of segments to use as reference (trusted values)
     *                              If empty, uses first 2 segments as default
     * @param step Scalar increment between adjacent fringes (default 1.0)
     * @param confidenceThreshold Minimum confidence to accept inferred numbers (0-1, default 0.7)
     */
    AutoNumberingCommand(
        CDigitInfo& doc,
        const std::vector<size_t>& trustedSegmentIndices = {},
        double step = 1.0,
        double confidenceThreshold = 0.7
    );
    
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Auto-Number Fringes"; }
    
private:
    CDigitInfo& m_doc;
    std::vector<size_t> m_trustedIndices;
    double m_step;
    double m_confidenceThreshold;
    
    // Save original fringes for undo
    std::vector<CFringeSegment> m_originalFringes;
};

// DeleteSelectionCommand - Delete all selected items from selection manager
class DeleteSelectionCommand : public Command {
public:
    /**
     * @brief Delete all items in current selection
     * 
     * @param doc Reference to CDigitInfo document
     * @param selectionManager Reference to SelectionManager with selected items
     * 
     * Supports deletion of:
     * - Dots: Removes individual control points
     * - Edges: Splits segments at edge location
     * - Segments: Removes entire segments
     * - Fringes: Removes all segments with same Number
     */
    DeleteSelectionCommand(CDigitInfo& doc, const class SelectionManager& selectionManager);
    
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Selection"; }
    
private:
    CDigitInfo& m_doc;
    std::vector<CFringeSegment> m_originalFringes;
    std::vector<size_t> m_deletedSegmentIndices;
    std::vector<size_t> m_deletedSegmentPositions;
    
    struct DeletedDot {
        size_t segmentIndex;
        size_t dotIndex;
        CDPoint point;
    };
    std::vector<DeletedDot> m_deletedDots;
    
    struct DeletedEdge {
        size_t segmentIndex;
        size_t edgeStartIndex;
    };
    std::vector<DeletedEdge> m_deletedEdges;
};

} // namespace DigitMode

#pragma once

#include "Command.h"
#include "../DigitInfo.h"
#include "../CFringeSegment.h"
#include "../../MGTools/Include/Utils/BaseDataType.h"
#include <vector>
#include <string>

namespace DigitMode {

using CPoint2d = CDPoint;

// CreateSegmentCommand
class CreateSegmentCommand : public Command {
public:
    CreateSegmentCommand(CDigitInfo& doc, const std::vector<CPoint2d>& points, double number);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Create Segment"; }
private:
    CDigitInfo& m_doc;
    std::vector<CPoint2d> m_points;
    double m_number;
    size_t m_createdIndex;
};

// ExtendSegmentCommand
class ExtendSegmentCommand : public Command {
public:
    ExtendSegmentCommand(CDigitInfo& doc, size_t segmentIndex, bool atHead, const CPoint2d& point);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Extend Segment"; }
private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    bool m_atHead;
    CPoint2d m_point;
};

// MoveDotCommand
class MoveDotCommand : public Command {
public:
    MoveDotCommand(CDigitInfo& doc, size_t segmentIndex, size_t dotIndex, const CPoint2d& oldPos, const CPoint2d& newPos);
    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Move Dot"; }
private:
    CDigitInfo& m_doc;
    size_t m_segmentIndex;
    size_t m_dotIndex;
    CPoint2d m_oldPos;
    CPoint2d m_newPos;
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
    CPoint2d m_removedPoint;
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
    std::vector<CPoint2d> m_secondPart;
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
    size_t m_segB;
    bool m_endA;
    bool m_endB;

    std::vector<CPoint2d> m_segAPoints;
    std::vector<CPoint2d> m_segBPoints;
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

} // namespace DigitMode

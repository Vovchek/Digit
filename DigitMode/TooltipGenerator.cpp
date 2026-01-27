#include "stdafx.h"
#include "TooltipGenerator.h"
#include "DigitMode/DigitInfo.h"
#include "DigitMode/CFringeSegment.h"
#include <cstdio>

namespace DigitMode {

std::string TooltipGenerator::GetTooltip(
    const SelectionManager::SelectedObject& obj,
    const CDigitInfo& digit
) const {
    char buffer[MAX_TOOLTIP_LEN];

    switch (obj.level) {
        case SelectionLevel::Dot: {
            // TODO: Phase 6 - Implement full query logic
            // For now, return simplified tooltip
            sprintf_s(buffer, "Dot %d in Segment %d", obj.iDot + 1, obj.iSegment + 1);
            break;
        }

        case SelectionLevel::Edge: {
            sprintf_s(buffer, "Edge %d–%d in Segment %d",
                obj.iEdge, obj.iEdge + 1, obj.iSegment + 1);
            break;
        }

        case SelectionLevel::Segment: {
            sprintf_s(buffer, "Segment %d", obj.iSegment + 1);
            break;
        }

        case SelectionLevel::Fringe: {
            sprintf_s(buffer, "Fringe #%.1f", obj.Number);
            break;
        }

        default:
            strcpy_s(buffer, "Unknown");
            break;
    }

    return std::string(buffer);
}

} // namespace DigitMode

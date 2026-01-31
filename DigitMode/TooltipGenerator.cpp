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

    if(SelectionLevel::None == obj.level ||
		obj.iSegment < 0 || obj.iSegment >= static_cast<int>(digit.Fringes.size())) {
        return { "" };
    }

    auto seg = digit.Fringes[obj.iSegment];
    std::vector<int> segments;
    digit.FindFringesByNumber(seg.GetNumber(), segments);

    switch (obj.level) {
        case SelectionLevel::Dot: {
            sprintf_s(buffer, "# %.1lf | %d/%d | %d/%d", 
                seg.GetNumber(), 
				seg.GetIndex(), segments.size(),
                obj.iDot + 1, seg.GetPointCount());
            break;
        }

        case SelectionLevel::Segment:
			[[fallthrough]];
        case SelectionLevel::Edge: {
            sprintf_s(buffer, "# %.1lf | %d/%d ",
                seg.GetNumber(),
                seg.GetIndex(), segments.size());
            break;
        }

        case SelectionLevel::Fringe: {
            sprintf_s(buffer, "# %.1lf", seg.GetNumber());
            break;
        }

        default:
            return { "" };
    }

    return std::string(buffer);
}

} // namespace DigitMode

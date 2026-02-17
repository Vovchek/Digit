#include "FringeSegmentAdaptor.h"

namespace DigitMode::digitization {

    CFringeSegment FringeSegmentAdapter::AdaptFringe(const NumberedFringe& numbered) {
        CFringeSegment adapted(numbered.number, numbered.segmentIndex);
        for (const auto& point : numbered.points) {
            adapted.AddPoint({point.x, point.y});
        }
        return adapted;
    }

    std::vector<CFringeSegment> FringeSegmentAdapter::AdaptFringes(
        const std::vector<NumberedFringe>& fringes) {
        std::vector<CFringeSegment> result;
        for (const auto& fringeData : fringes) {
            result.push_back(AdaptFringe(fringeData));
        }
        return result;
    }

}

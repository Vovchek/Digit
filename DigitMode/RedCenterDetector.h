#pragma once

#include <functional>
#include <vector>

#include "DigitMode/DigitizationParams.h"

namespace DigitMode::digitization {

class RedCenterDetector {
public:
    static std::vector<Section> DetectExtrema(const DigitizationInput& input);

};

} // namespace DigitMode::digitization

#pragma once

#include <functional>
#include <vector>

#include "DigitMode/DigitizationParams.h"

namespace DigitMode::digitization {

class RedCenterDetector {
public:
    static std::vector<ExtremumPoint> DetectExtrema(const DigitizationInput& input);

private:
    static std::vector<ExtremumPoint> AnalyzeScanline(
        int scanlineY,
        const std::vector<unsigned char>& scanlineData,
        const std::function<bool(int, int)>& isVisible,
        int fringeCenterAs);
};

} // namespace DigitMode::digitization

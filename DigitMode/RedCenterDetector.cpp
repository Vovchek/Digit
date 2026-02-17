#include "DigitMode/RedCenterDetector.h"

#include <array>
#include <vector>
#include <algorithm>

// #include "Appdef.h" - MEMO: Appdef.h depends on MFC, do not use here!
// For now use local ad hoc defines
// TODO: delete when appropriate solution is found
#define FC_MAX    0
#define FC_MIN    1
#define FC_MINMAX 2

#include "Utils/middle.h"

namespace DigitMode::digitization {

std::vector<ExtremumPoint> RedCenterDetector::DetectExtrema(const DigitizationInput& input)
{
    std::vector<ExtremumPoint> results;
    if (!input.bitmapData || input.imageWidth <= 0 || input.imageHeight <= 0 || !input.isVisible) {
        return results;
    }

    const int width = input.imageWidth;
    const int height = input.imageHeight;

    std::vector<uint8_t> line(static_cast<size_t>(width));
    std::vector<uint8_t> invLine(static_cast<size_t>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (input.isVisible(x, y)) {
                const int row = (height - 1) - y;
                const int index = row * width + x;
                line[x] = input.bitmapData[index];
            }
            else {
                line[x] = 0;
            }
            invLine[x] = line[x];
        }

        std::vector<double> redXs;
        int nnpolos = 0;

        if (input.fringeCenterAs == FC_MAX) {
            fon_del_(line.data(), 0, width - 1);
            redXs = middle_(line.data(), width, y, input.isVisible);
        }
        else if (input.fringeCenterAs == FC_MIN) {
            invert_line(invLine.data(), 0, width - 1);
            fon_del_(invLine.data(), 0, width - 1);
            redXs = middle_(invLine.data(), width, y, input.isVisible);
        }
        else if (input.fringeCenterAs == FC_MINMAX) {
            fon_del(line.data(), 0, width - 1);
            redXs = middle_(line.data(), width, y, input.isVisible);
            invert_line(invLine.data(), 0, width - 1);
            fon_del(invLine.data(), 0, width - 1);
            auto tail = middle_(invLine.data(), width, y, input.isVisible);
			redXs.insert(redXs.end(), tail.begin(), tail.end());
            std::sort(redXs.begin(), redXs.end());
        }

        for (int i = 0; i < redXs.size(); ++i) {
            const double redX = redXs[i];
            const int sampleX = static_cast<int>(redX + 0.5);
            double intensity = 0.0;
            if (sampleX >= 0 && sampleX < width) {
                intensity = line[static_cast<size_t>(sampleX)];
            }

            ExtremumPoint point;
            point.position = { redX, static_cast<double>(y) };
            point.intensity = intensity;
            point.extremumType = input.fringeCenterAs;
            results.push_back(point);
        }
    }

    return results;
}

std::vector<ExtremumPoint> RedCenterDetector::AnalyzeScanline(
    int scanlineY,
    const std::vector<unsigned char>& scanlineData,
    const std::function<bool(int, int)>& isVisible,
    int fringeCenterAs)
{
    (void)scanlineY;
    (void)scanlineData;
    (void)isVisible;
    (void)fringeCenterAs;
    return {};
}

} // namespace DigitMode::digitization

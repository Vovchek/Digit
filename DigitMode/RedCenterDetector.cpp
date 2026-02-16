#include "DigitMode/RedCenterDetector.h"

#include <array>

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

    std::vector<std::array<int, 4>> bufLine(static_cast<size_t>(height));
    std::vector<int*> bufPtrs(static_cast<size_t>(height));
    for (int y = 0; y < height; ++y) {
        bufLine[static_cast<size_t>(y)] = { -1, -1, -1, -1 };
        bufPtrs[static_cast<size_t>(y)] = bufLine[static_cast<size_t>(y)].data();
    }

    std::vector<unsigned char> line(static_cast<size_t>(width));
    std::vector<unsigned char> invLine(static_cast<size_t>(width));

    for (int y = 0; y < height; ++y) {
        int left = -1;
        int right = -1;
        for (int x = 0; x < width; ++x) {
            if (input.isVisible(x, y)) {
                if (left < 0)
                    left = x;
                right = x;
            }
        }

        if (left < 0 || right < 0) {
            continue;
        }

        bufLine[static_cast<size_t>(y)][0] = left;
        bufLine[static_cast<size_t>(y)][1] = right;
        bufLine[static_cast<size_t>(y)][2] = -1;
        bufLine[static_cast<size_t>(y)][3] = -1;

        for (int x = 0; x < width; ++x) {
            if (input.isVisible(x, y)) {
                const int row = (height - 1) - y;
                const size_t index = static_cast<size_t>(row) * static_cast<size_t>(width) + static_cast<size_t>(x);
                line[static_cast<size_t>(x)] = input.bitmapData[index];
            }
            else {
                line[static_cast<size_t>(x)] = 0;
            }
            invLine[static_cast<size_t>(x)] = line[static_cast<size_t>(x)];
        }

        CArray<double, double> redXs;
        int nnpolos = 0;

        if (input.fringeCenterAs == FC_MAX) {
            fon_del(line.data(), 0, width - 1);
            middle(line.data(), width, height, y, bufPtrs.data(), redXs, nnpolos);
        }
        else if (input.fringeCenterAs == FC_MIN) {
            invert_line(invLine.data(), 0, width - 1);
            fon_del(invLine.data(), 0, width - 1);
            middle(invLine.data(), width, height, y, bufPtrs.data(), redXs, nnpolos);
        }
        else if (input.fringeCenterAs == FC_MINMAX) {
            fon_del(line.data(), 0, width - 1);
            middle(line.data(), width, height, y, bufPtrs.data(), redXs, nnpolos);
            invert_line(invLine.data(), 0, width - 1);
            fon_del(invLine.data(), 0, width - 1);
            middle(invLine.data(), width, height, y, bufPtrs.data(), redXs, nnpolos);
            SortDouble(redXs);
        }

        for (int i = 0; i < redXs.GetSize(); ++i) {
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

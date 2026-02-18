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

std::vector<Section> RedCenterDetector::DetectExtrema(const DigitizationInput& input)
{
    std::vector<Section> results;
    if (!input.bitmapData || input.imageWidth <= 0 || input.imageHeight <= 0 || !input.isVisible) {
        return results;
    }

    const int width = input.imageWidth;
    const int height = input.imageHeight;

    std::vector<uint8_t> line(static_cast<size_t>(width));
    std::vector<uint8_t> invLine(static_cast<size_t>(width));

    // TODO: need to sort out with fon_del - 
    // setting 0 to inv_line outside apertures worsens result a lot.
    // Should be quite oposite.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int row = (height - 1) - y;
            const int index = row * width + x;
            line[x] = input.bitmapData[index];
            invLine[x] = 255 - line[x];
        }

        std::vector<double> redXs;
        std::vector<double> blackXs;
        Section lineResults;

        int nnpolos = 0;

        if (input.fringeCenterAs == FC_MAX || input.fringeCenterAs == FC_MINMAX) {
            fon_del_(line.data(), 0, width - 1);
            redXs = middle_(line.data(), width, y, input.isVisible);
        }
        if (input.fringeCenterAs == FC_MIN || input.fringeCenterAs == FC_MINMAX) {
            fon_del_(invLine.data(), 0, width - 1);
            blackXs = middle_(invLine.data(), width, y, input.isVisible);
        }

        auto toResults = [y, width, &lineResults](const auto& Xs, const auto& line, auto extremumType) {
            for (const auto X : Xs) {
                const int sampleX = static_cast<int>(X + 0.5);
                double intensity = 0.0;
                if (sampleX >= 0 && sampleX < width) {
                    intensity = line[static_cast<size_t>(sampleX)];
                }
                ExtremumPoint point;
                point.position = { X, static_cast<double>(y) };
                point.intensity = intensity;
                point.extremumType = extremumType;

                lineResults.points.push_back(point);
            }

            };
        lineResults.points.clear();
        toResults(redXs, line, ExtremumType::Red);
        toResults(blackXs, invLine, ExtremumType::Black);
        std::sort(lineResults.points.begin(), lineResults.points.end(), [](const auto& a, const auto& b)
            {return a.position.x < b.position.x; });

        for(int x = 0; x < width; ++x)
            if (input.isVisible(x, y)) {
                lineResults.limits.leftEdge.x = x;
                break;
            }
        for (int x = width - 1; x > 0; --x)
            if (input.isVisible(x, y)) {
                lineResults.limits.rightEdge.x = x;
                break;
            }
        results.emplace_back(std::move(lineResults));
    }

    return results;
}


} // namespace DigitMode::digitization

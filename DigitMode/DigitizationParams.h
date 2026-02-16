#pragma once

#include <functional>
#include <vector>

namespace DigitMode::digitization {

struct Point2d {
    double x{0.0};
    double y{0.0};
};

struct ExtremumPoint {
    Point2d position;
    double intensity{0.0};
    int extremumType{0};
};

struct FringePolyline {
    std::vector<Point2d> points;
    int index{-1};
};

struct NumberedFringe {
    std::vector<Point2d> points;
    double number{0.0};
    int segmentIndex{-1};
};

struct DigitizationInput {
    const unsigned char* bitmapData{nullptr};
    int imageWidth{0};
    int imageHeight{0};

    std::function<bool(int, int)> isVisible;

    int fringeCenterAs{0};
    double contrastThreshold{0.0};
    int minFringeSpacing{0};
    int maxFringeSpacing{0};

    Point2d apertureCenter{};
};

struct DigitizationOutput {
    std::vector<ExtremumPoint> redCenters;
    std::vector<FringePolyline> polylines;
    std::vector<NumberedFringe> fringes;
    double averageFringeStep{0.0};
    int mainFringeNumber{0};
};

} // namespace DigitMode::digitization

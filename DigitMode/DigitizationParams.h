#pragma once

#include <functional>
#include <vector>

namespace DigitMode::digitization {

struct Point2d {
    double x{0.0};
    double y{0.0};
};

struct Limits {
    Point2d leftEdge;
    Point2d rightEdge;
};

enum class ExtremumType { Red = 0, Black = 1 };

struct ExtremumPoint {
    Point2d position;
    double intensity{0.0};
    ExtremumType extremumType{ ExtremumType::Red};
    bool isValid{false};
    double number{-1000.};
    Limits window{ {0.0,0.0},{0.0,0.0} };
};

struct Section {
    std::vector<ExtremumPoint> points;
    Limits limits{ {0.0,0.0},{0.0,0.0} };
    double aveStep{0.};
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

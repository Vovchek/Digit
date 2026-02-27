#pragma once

#include "aperturecore/include/aperturecore/geometry/Bounds.h"
#include "aperturecore/include/aperturecore/visibility/VisibilityMask.h"
#include "DigitMode/CFringeSegment.h"
#include <Eigen/sparse>

struct FringeSolverInput
{
    aperture::Bounds bounds;
    aperture::visibility::VisibilityMask visibilityMask;
    std::vector<CFringeSegment> fringeSegments;
    int Nx; // Number of grid points in x direction
    int Ny; // Number of grid points in y direction
};

class FringeSolverResult
{
public:
    bool SaveAsImage(const std::string& filename) const;
    bool SaveAsMTR(const std::string& filename) const;

private:
    Eigen::MatrixXd map;
    aperture::Bounds bounds;

};

class FringeSolverContext
{
public:
    explicit FringeSolverContext(const FringeSolverInput& input) : 
        input_(input) {}


private:
    FringeSolverInput input_;
};

class IFringeSolver
{
public:
    virtual ~IFringeSolver() = default;
    virtual FringeSolverResult Solve(const FringeSolverContext& ctx) = 0;
};


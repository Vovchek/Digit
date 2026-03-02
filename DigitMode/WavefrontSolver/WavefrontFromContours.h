#pragma once

#include "aperturecore/include/aperturecore/geometry/Bounds.h"
#include "aperturecore/include/aperturecore/visibility/VisibilityMask.h"
#include "aperturecore/include/aperturecore/geometry/CoordinateSystem.h"
#include "DigitMode/CFringeSegment.h"
#include <iomanip>
#include <limits>
#include <fstream>

// Save current macro state and undefine conflicting MFC macros for Eigen
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

// Restore original macro state
#pragma pop_macro("max")
#pragma pop_macro("min")

struct WavefrontFromContoursInput
{
    explicit WavefrontFromContoursInput(
        const aperture::Bounds& bounds,
        const aperture::visibility::VisibilityMask& visibilityMask,
        const std::vector<CFringeSegment>& fringeSegments,
        int outWidth = 0,
        int outHeight = 0,
        aperture::CoordinateSystemType inputCoordType = aperture::CoordinateSystemType::SCREEN,
        aperture::CoordinateSystemType outputCoordType = aperture::CoordinateSystemType::MATH)
        : bounds_(bounds)
        , visibilityMask_(visibilityMask)
        , fringeSegments_(fringeSegments)
        , outWidth_(outWidth > 0 ? outWidth : visibilityMask.width)
        , outHeight_(outHeight > 0 ? outHeight : 
            static_cast<int>(visibilityMask.height * outWidth_ / static_cast<double>(visibilityMask.width)))
        , inputCoordType_(inputCoordType)
		, outputCoordType_(outputCoordType) {
	}

    const aperture::Bounds bounds_;
    const aperture::visibility::VisibilityMask& visibilityMask_;
    const std::vector<CFringeSegment>& fringeSegments_;
    // Resolution parameters
    // outWidth: desired output matrix width (columns along x-axis)
    // outWidth = 0 means use aperture bounding box width (resolved in constructor)
    int outWidth_;
    // outHeight: desired output matrix height (rows along y-axis)
    // outHeight = 0 means calculate from outWidth to maintain aspect ratio (resolved in constructor)
    int outHeight_;
    aperture::CoordinateSystemType inputCoordType_{ aperture::CoordinateSystemType::SCREEN };
    aperture::CoordinateSystemType outputCoordType_{ aperture::CoordinateSystemType::MATH };
};

class WavefrontFromContoursContext
{
public:
    explicit WavefrontFromContoursContext(const WavefrontFromContoursInput& input) :
        input_(input) {}
	// solver helpers
	// rasterize to output resolution, considering visibility mask, bounds, input & output coordinate systems
	std::vector<char> buildMask() const
	{
		const auto& visibilityMask = input_.visibilityMask_;
		
		// Use pre-resolved output dimensions from constructor
		int outWidth = input_.outWidth_;
		int outHeight = input_.outHeight_;

		// Build output mask with output dimensions
		const int N = outHeight * outWidth;
		std::vector<char> visible(N, 0);

		// Create coordinate systems for conversion
		aperture::CoordinateSystem inputSys = (input_.inputCoordType_ == aperture::CoordinateSystemType::SCREEN) 
			? aperture::CoordinateSystem::screen(visibilityMask.height)
			: aperture::CoordinateSystem::math(visibilityMask.height);
		
		aperture::CoordinateSystem outputSys = (input_.outputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(outHeight)
			: aperture::CoordinateSystem::math(outHeight);

		for (int outY = 0; outY < outHeight; ++outY) {
			for (int outX = 0; outX < outWidth; ++outX) {
				// Map output pixel to visibility mask coordinate space
				double maskX = outX * visibilityMask.width / static_cast<double>(outWidth);
				double maskY = outY * visibilityMask.height / static_cast<double>(outHeight);

				// Handle coordinate system conversion
				double finalY = convertY(maskY);

				// Clamp to visibility mask bounds
				int x = static_cast<int>(maskX);
				int y = static_cast<int>(finalY);
				
				if (x >= 0 && x < visibilityMask.width && y >= 0 && y < visibilityMask.height) {
					int outIndex = outY * outWidth + outX;
					visible[outIndex] = visibilityMask.IsVisible(x, y) ? 1 : 0;
				}
			}
		}

		return visible;
	}

	std::pair<std::vector<char>, std::vector<double>> 
		rasterize(const std::vector<char>& mask) const;

	// Helper method to convert Y coordinate between reference systems
	double convertY(double y) const
	{
		// If coordinate systems match, no conversion needed
		if (input_.inputCoordType_ == input_.outputCoordType_) {
			return y;
		}

		// Create coordinate systems for conversion
		aperture::CoordinateSystem inputSys = (input_.inputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(input_.visibilityMask_.height)
			: aperture::CoordinateSystem::math(input_.visibilityMask_.height);
		
		aperture::CoordinateSystem outputSys = (input_.outputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(input_.outHeight_)
			: aperture::CoordinateSystem::math(input_.outHeight_);

		// Convert Y coordinate between coordinate systems
		return inputSys.convertY(y, outputSys);
	}

	// Helper method to convert bounds to output coordinate system
	aperture::Bounds convertBounds(const aperture::Bounds& bounds) const
	{
		// If coordinate systems match, no conversion needed
		if (input_.inputCoordType_ == input_.outputCoordType_) {
			return bounds;
		}

		// Need to flip Y bounds when converting between coordinate systems
		double minY = bounds.minY();
		double maxY = bounds.maxY();
		double height = bounds.height();

		// Create coordinate systems for conversion
		aperture::CoordinateSystem inputSys = (input_.inputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(height)
			: aperture::CoordinateSystem::math(height);
		
		aperture::CoordinateSystem outputSys = (input_.outputCoordType_ == aperture::CoordinateSystemType::SCREEN)
			? aperture::CoordinateSystem::screen(height)
			: aperture::CoordinateSystem::math(height);

		// Convert Y coordinates (note: max becomes min after flip)
		double convertedMinY = inputSys.convertY(maxY, outputSys);
		double convertedMaxY = inputSys.convertY(minY, outputSys);

		// Create new bounds with converted Y
		return aperture::Bounds(
			bounds.minX(),
			convertedMinY,
			bounds.width(),
			bounds.height()
		);
	}

	const WavefrontFromContoursInput input_;
};

class WavefrontFromContoursResult
{
public:

	// ============================================================================
	// Getters
	// ============================================================================

	const std::vector<double>& getData() const { return data_; }
	int getRows() const { return rows_; }
	int getCols() const { return cols_; }
	const aperture::Bounds& getBounds() const { return bounds_; }
	aperture::CoordinateSystemType getCoordinateSystem() const { return coordType_; }

	// ============================================================================
	// Setters
	// ============================================================================

	void setMatrixData(const double* data, int rows, int cols);
	void setBounds(const aperture::Bounds& bounds) { bounds_ = bounds; }
	void setCoordinateSystem(aperture::CoordinateSystemType coordType) { coordType_ = coordType; }

	// ============================================================================
	// Stream Serialization
	// ============================================================================

	friend std::ostream& operator<<(std::ostream& os, const WavefrontFromContoursResult& result);
	bool saveAsImage(const std::string& filename) const { return false; };
	bool saveMtrMatrix(std::ostream& os) const;

private:
    std::vector<double> data_;
    int rows_ = 0;
    int cols_ = 0;
    aperture::Bounds bounds_;
    aperture::CoordinateSystemType coordType_;

};

class IWavefrontFromContoursSolver
{
public:
    virtual ~IWavefrontFromContoursSolver() = default;
    virtual WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const = 0;
};

// High-Level Facade (User API)
class WavefrontFromContours
{
public:
	explicit WavefrontFromContours(WavefrontFromContoursInput input) :
        context_(input) {}
	WavefrontFromContoursResult run(const IWavefrontFromContoursSolver& solver)
    {
        return solver.solve(context_);
    }

private:
    WavefrontFromContoursContext context_;
};

// Helper function to access context parameters for solvers
inline void getContextDimensions(const WavefrontFromContoursContext& ctx, int& outWidth, int& outHeight)
{
	// Output dimensions are already resolved in constructor
	outWidth = ctx.input_.outWidth_;
	outHeight = ctx.input_.outHeight_;
}

// concreate solvers (e.g., Poisson, Bilinear, etc.) 
class WavefrontFromContoursSolver_Bilinear : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Bilinear interpolation to fill unknown values using known neighbors
	// Only interpolates pixels where knownZ[idx]==0 && mask[idx]!=0
	// Updates knownZ as values are interpolated
// Bilinear interpolation to fill unknown values using known neighbors
	static void performBilinearInterpolation(
		std::vector<double>& zk,
		std::vector<char>& knownZ,
		const std::vector<char>& mask,
		int rows, int cols)
	{
		const int maxPasses = 20;

		auto idx = [&](int x, int y) { return y * cols + x; };

		for (int pass = 0; pass < maxPasses; ++pass)
		{
			bool changed = false;

			std::vector<double> newZ = zk;
			std::vector<char>   newKnown = knownZ;

			for (int y = 0; y < rows; ++y)
			{
				for (int x = 0; x < cols; ++x)
				{
					int i = idx(x, y);

					if (knownZ[i] || !mask[i])
						continue;

					// --- Find horizontal bracket
					int xl = x - 1;
					while (xl >= 0 && (!knownZ[idx(xl, y)] || !mask[idx(xl, y)]))
						--xl;

					int xr = x + 1;
					while (xr < cols && (!knownZ[idx(xr, y)] || !mask[idx(xr, y)]))
						++xr;

					bool hasX = (xl >= 0 && xr < cols);

					// --- Find vertical bracket
					int yt = y - 1;
					while (yt >= 0 && (!knownZ[idx(x, yt)] || !mask[idx(x, yt)]))
						--yt;

					int yb = y + 1;
					while (yb < rows && (!knownZ[idx(x, yb)] || !mask[idx(x, yb)]))
						++yb;

					bool hasY = (yt >= 0 && yb < rows);

					if (!hasX && !hasY)
						continue;

					double val = 0.0;
					int    used = 0;

					if (hasX)
					{
						double zl = zk[idx(xl, y)];
						double zr = zk[idx(xr, y)];
						double t = double(x - xl) / double(xr - xl);
						val += zl * (1.0 - t) + zr * t;
						used++;
					}

					if (hasY)
					{
						double zt = zk[idx(x, yt)];
						double zb = zk[idx(x, yb)];
						double t = double(y - yt) / double(yb - yt);
						val += zt * (1.0 - t) + zb * t;
						used++;
					}

					newZ[i] = val / used;
					newKnown[i] = 1;
					changed = true;
				}
			}

			zk.swap(newZ);
			knownZ.swap(newKnown);

			if (!changed)
				break;
		}
	}
};

// True horizontal linear interpolation solver
// For each row, finds fringe intersections and linearly interpolates between them
class WavefrontFromContoursSolver_HorizontalLinear : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Helper structure to store fringe crossing point
	struct FringeCrossing
	{
		double x;           // X-coordinate where fringe crosses horizontal line
		double fringeValue; // Fringe number at this crossing
		
		bool operator<(const FringeCrossing& other) const
		{
			return x < other.x;
		}
	};
	
	// Find all points where fringes cross a horizontal line at given Y
	// Returns sorted vector of crossings
	std::vector<FringeCrossing> findFringeCrossings(
		const WavefrontFromContoursContext& ctx,
		double worldY) const;
	
	// Linearly interpolate Z value at worldX given sorted crossings
	// Returns NaN if outside fringe coverage or no valid interpolation
	double interpolateAtX(
		const std::vector<FringeCrossing>& crossings,
		double worldX) const;
};

// Horizontal cubic spline interpolation solver
// For each row, finds fringe intersections and uses cubic spline for smooth interpolation
class WavefrontFromContoursSolver_HorizontalSpline : public IWavefrontFromContoursSolver
{
public:
	WavefrontFromContoursResult solve(const WavefrontFromContoursContext& ctx) const override;

private:
	// Helper structure to store fringe crossing point
	struct FringeCrossing
	{
		double x;           // X-coordinate where fringe crosses horizontal line
		double fringeValue; // Fringe number at this crossing
		
		bool operator<(const FringeCrossing& other) const
		{
			return x < other.x;
		}
	};
	
	// Find all points where fringes cross a horizontal line at given Y
	// Returns sorted vector of crossings
	std::vector<FringeCrossing> findFringeCrossings(
		const WavefrontFromContoursContext& ctx,
		double worldY) const;
	
	// Cubic spline interpolation
	// Builds natural cubic spline coefficients for given crossings
	// Returns interpolated Z value at worldX
	double interpolateAtX(
		const std::vector<FringeCrossing>& crossings,
		double worldX) const;
	
	// Helper to compute cubic spline coefficients (natural spline)
	// Input: n points (x[i], y[i]) sorted by x
	// Output: coefficients for cubic polynomials between points
	struct SplineCoefficients
	{
		std::vector<double> a, b, c, d; // Coefficients for each segment
		std::vector<double> x;          // X coordinates of knots
		
		// Evaluate spline at given x
		double evaluate(double xi) const;
	};
	
	SplineCoefficients buildSpline(const std::vector<FringeCrossing>& crossings) const;
};
